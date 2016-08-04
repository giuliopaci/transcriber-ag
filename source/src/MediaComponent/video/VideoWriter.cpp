/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "VideoWriter.h"

// -- Main constructor --
VideoWriter::VideoWriter(string inPath, int inWidth, int inHeight, int inStep)
	: sourcePath(inPath), outWidth(inWidth), outHeight(inHeight), frameStep(inStep)
{
	av_register_all();
	avcodec_register_all();
}


// -- GenerateVideo --
VideoResult*
VideoWriter::generateVideo()
{
	// -- Opening source file --
	device = Guesser::open(sourcePath.c_str(), VideoCtx);


	// -- Dimensions check --
	if (device != NULL)
	{
		double base_ratio	= (double)device->m_info()->video_width / (double)device->m_info()->video_height;
		double aspect_ratio	= device->m_info()->video_aspect_ratio;

		if (aspect_ratio == 0.0)
			aspect_ratio = 1.0;

		// -- New algorithm (assuming video ratio > 1) --
		outHeight = (int)(outWidth / base_ratio);
	}

	device->m_set_dimensions(outWidth, outHeight);


	// -- Format guesser & allocation --
	outFormat = av_guess_format(NULL, sourcePath.c_str(), NULL);

	if (!outFormat)
	{
		printf("Format Guess Fallback : mpeg\n");
		outFormat = av_guess_format("mpeg", NULL, NULL);
	}
    
	formatCtx = avformat_alloc_context();

	if (!formatCtx)
	{
		printf("Memory error\n");
		return NULL;
	}

	formatCtx->oformat = outFormat;

	// -- Output filename --
	snprintf(formatCtx->filename, sizeof(formatCtx->filename), "%s", "/tmp/output.mpg");


	// -- Single output video stream --
	if (outFormat->video_codec != CODEC_ID_NONE)
		video_st = addVideoStream(formatCtx, outFormat->video_codec);
    
	// -- Format Log --
	av_dump_format(formatCtx, 0, formatCtx->filename, 1);


	// -- Opening video stream --
	if (video_st)
	{
		printf("OpenVideo\n");
		openVideo(formatCtx, video_st);
	}


	// -- Opening output file --
	if ( !(outFormat->flags & AVFMT_NOFILE) )
	{
		printf("Opening file : %s\n", formatCtx->filename);
		if (avio_open(&formatCtx->pb, formatCtx->filename, AVIO_FLAG_WRITE) < 0)
		{
			printf("Could not open '%s'\n", formatCtx->filename);
			exit(1);
		}
	}

	// -- File Header --
	avformat_write_header(formatCtx, NULL);

	// -- Frame copy --
	int nbframes = device->m_info()->video_duration / frameStep;

	for(int i=0; i<nbframes; i++)
		writeVideoFrame(formatCtx, video_st, i);


	// -- File trailer --
	av_write_trailer(formatCtx);
	closeVideo(formatCtx, video_st);

	// -- Freeing all objects --
	for(int i = 0; i < formatCtx->nb_streams; i++)
	{
		av_freep(&formatCtx->streams[i]->codec);
		av_freep(&formatCtx->streams[i]);
	}

	if (!(outFormat->flags & AVFMT_NOFILE))
		avio_close(formatCtx->pb);

	av_free(formatCtx);
}


// --- AddVideoStream ---
AVStream*
VideoWriter::addVideoStream(AVFormatContext *formatCtx, AVCodecID codecID)
{
	AVCodecContext*	codecCtx;
	AVStream*		stream;

	stream = avformat_new_stream(formatCtx, NULL);

	if (!stream)
	{
		printf("Could not alloc stream\n");
		return NULL;
	}

	codecCtx				= stream->codec;
	codecCtx->codec_id		= CODEC_ID_BMP;
	codecCtx->codec_type	= AVMEDIA_TYPE_VIDEO;

	// -- Sample parameters --
	codecCtx->bit_rate		= device->m_info()->video_bit_rate;
	codecCtx->width			= outWidth;
	codecCtx->height		= outHeight;
	codecCtx->time_base.den = STREAM_FRAME_RATE;
	codecCtx->time_base.num = 1;
	codecCtx->gop_size		= 12; /* emit one intra frame every twelve frames at most */
	codecCtx->pix_fmt		= STREAM_PIX_FMT;

	if (codecCtx->codec_id == CODEC_ID_MPEG2VIDEO)
		codecCtx->max_b_frames = 2;

	if (codecCtx->codec_id == CODEC_ID_MPEG1VIDEO)
		codecCtx->mb_decision=2;

	//  -- Separate stream headers --
	if(!strcmp(formatCtx->oformat->name, "mp4") || !strcmp(formatCtx->oformat->name, "mov") || !strcmp(formatCtx->oformat->name, "3gp"))
	{
		printf("Global header!\n");
		codecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

	return stream;
}


// -- WriteVideoFrame --
void
VideoWriter::writeVideoFrame(AVFormatContext* formatCtx, AVStream* stream, int index)
{
	int out_size, ret;
	AVCodecContext*	codecCtx;
	static struct SwsContext *img_convert_ctx;

	codecCtx = stream->codec;
  
	AVPacket pkt;

	double ts = (double)index * (double)frameStep;

	MediumFrame* frame = device->m_get_rgb_frame(ts);
	
	av_init_packet(&pkt);

	// -- Packet settings --
	pkt.stream_index= stream->index;
	pkt.data = frame->v_samples[0];
	pkt.size = frame->vlen;

	// -- Writing packet --
	ret = av_write_frame(formatCtx, &pkt);
}


// -- OpenVideo --
void
VideoWriter::openVideo(AVFormatContext *formatCtx, AVStream *stream)
{
	AVCodec *codec;
	AVCodecContext *codecCtx;

	codecCtx = stream->codec;

	/* find the video encoder */
	codec = avcodec_find_encoder(codecCtx->codec_id);

	if (!codec)
	{
		printf("codec not found\n");
		exit(1);
	}
	else
		printf("codec found\n");

	/* open the codec */
	if (avcodec_open2(codecCtx, codec, NULL) < 0)
		printf("could not open codec\n");
	else
		printf("codec opened successfully\n");

	video_outbuf = NULL;
	
	if (!(formatCtx->oformat->flags & AVFMT_RAWPICTURE))
	{
		/* allocate output buffer */
		/* XXX: API change will be done */
		/* buffers passed into lav* can be allocated any way you prefer,
			as long as they're aligned enough for the architecture, and
			they're freed appropriately (such as using av_free for buffers
			allocated with av_malloc) */
		video_outbuf_size = 200000;
		video_outbuf = (uint8_t*)av_malloc(video_outbuf_size);
	}

	/* allocate the encoded raw picture */
	picture = alloc_picture(codecCtx->pix_fmt, codecCtx->width, codecCtx->height);

	if (!picture)
		printf("Could not allocate picture\n");
	
	/* if the output format is not YUV420P, then a temporary YUV420P
	picture is needed too. It is then converted to the required
	output format */
	tmp_picture = NULL;

	if (codecCtx->pix_fmt != PIX_FMT_YUV420P)
	{
		tmp_picture = alloc_picture(PIX_FMT_YUV420P, codecCtx->width, codecCtx->height);
		if (!tmp_picture)
		{
			fprintf(stderr, "Could not allocate temporary picture\n");
		}
	}
}


AVFrame*
VideoWriter::alloc_picture(int pix_fmt, int width, int height)
{
	AVFrame *picture;
	uint8_t *picture_buf;
	int size;

	picture = avcodec_alloc_frame();
	if (!picture)
		return NULL;

	size = 0; // PLER avpicture_get_size(pix_fmt, width, height);
	picture_buf = (uint8_t*)av_malloc(size);

	if (!picture_buf)
	{
		av_free(picture);
		return NULL;
	}

//  TODO COMMENTAIRE A VIRER
//	avpicture_fill((AVPicture *)picture, picture_buf, pix_fmt, width, height);

	return picture;
}


void
VideoWriter::closeVideo(AVFormatContext *formatCtx, AVStream *stream)
{
	avcodec_close(stream->codec);
	av_free(picture->data[0]);
	av_free(picture);

	if (tmp_picture)
	{
		av_free(tmp_picture->data[0]);
		av_free(tmp_picture);
	}
	av_free(video_outbuf);
}

