/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "FFMpegHandler.h"
#include <iostream>
#include <math.h>

// -- USLEEP Local definition --
#ifdef WIN32
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#define USLEEP(a) SleepEx((a <= 1000 ? 1 : a/1000), false)
#define strcasecmp stricmp
#else
#include <unistd.h>
#define USLEEP(a) usleep(a)
#endif


// --- FFMpegHandler ---
FFMpegHandler::FFMpegHandler()
{
	info_retrieved		= stream_seek = false;
	handler_ctx			= DecoderCtx;
	av_ctx				= AudioCtx;
	formatCtx			= NULL;
	time_base			= frame_rate = 0.0;
	scaler_rgb			= NULL;
	current_ts			= 0.0;
	current_pts			= 0.0;
	current_dts			= 0.0;
	step				= 0.0;
	key_frame_interval	= 0.5;		// Default Dummy Interval
	key_frame_counter	= 0.0;

	mutex	= g_mutex_new();
	m_init();
}


// --- ~FFMpegHandler ---
FFMpegHandler::~FFMpegHandler()
{}


/**********************/
/** I/O base methods **/
/**********************/

// --- Init ---
bool FFMpegHandler::m_init()
{
	// -- Mutex Lock --
	bool locked = false;

	while (!locked)
	{
		locked = g_mutex_trylock(mutex);
		USLEEP(10*1000);
	}

	av_register_all();
	av_log_set_level(AV_LOG_QUIET);

	g_mutex_unlock(mutex);
	return true;
}


// --- Open ---
bool FFMpegHandler::m_open(const char *inMedium, int)
{
	int err;
	MediumFrame* tFrame = NULL;
	medium = (char*)inMedium;


	// Step 1 - Opening
	err = avformat_open_input(&formatCtx, medium, NULL, NULL);

	if (err)
	{
		formatCtx = NULL;
		fprintf(stderr, "FFMpegHandler --> Unable to open file\n");
		return false;
	}

	avformat_find_stream_info(formatCtx, NULL);

	// Step 2 - Storing Streams Indexes
	for(unsigned int i=0; i<formatCtx->nb_streams; i++)
	{
		switch( formatCtx->streams[i]->codec->codec_type )
		{
			case AVMEDIA_TYPE_VIDEO:
				v_streams.push_back(i);
				video_time_base = av_q2d(formatCtx->streams[v_streams[0]]->time_base);
				break;

			case AVMEDIA_TYPE_AUDIO:
				a_streams.push_back(i);
				audio_time_base = av_q2d(formatCtx->streams[a_streams[0]]->time_base);
				break;

			/** Ignoring non A/V streams */
			default:
				break;
		}
	}

	// Step 3 - Inits
	initFrame();
	
	switch(av_ctx)
	{
		case AudioCtx:
			if (a_streams.empty())
				return false;

			decoderCtx = formatCtx->streams[a_streams.at(0)]->codec;
			initDecoder(decoderCtx->codec_id);
			
			tFrame = m_read();

			av_seek_frame(formatCtx, a_streams.at(0), 0, AVSEEK_FLAG_BACKWARD);
			break;

		case VideoCtx:
			if (v_streams.empty())
				return false;

			decoderCtx = formatCtx->streams[v_streams.at(0)]->codec;
			initDecoder(decoderCtx->codec_id);
			
			while(time_base == 0.0)
				m_next_frame();

			av_seek_frame(formatCtx, v_streams.at(0), 0, AVSEEK_FLAG_BACKWARD);
			break;

		case AudioVideoCtx:
			break;

		default:
			return false;
	}


	// Step 4 - 1st Frame (Detect) & Rewind
	m_info();

	/** Frame size bug workaround */
	if (m_info()->audio_frame_size == 0)
	{
		if (tFrame != NULL)
		{
			m_info()->audio_frame_size = tFrame->len / 2;
			fprintf(stderr, "FFMpegHandler --> Auto-adjusting frame size to : %i\n", tFrame->len);
		}
	}


	// Step 5 - Initializing Scalers
	if (av_ctx == VideoCtx)
	{
		m_set_dimensions(s_info.video_width, s_info.video_height);
		initScalers();
	}

	return !err;
}


// --- Close ---
bool FFMpegHandler::m_close()
{
	if (formatCtx != NULL)
		av_close_input_file(formatCtx);

	av_free(frame->samples);
	delete frame;

	return true;
}


/*********************/
/** INITIALIZATIONS **/
/*********************/

// --- SetHandlerContext ---
void FFMpegHandler::setHandlerContext(HandlerCtx in_mode)
{
	handler_ctx = in_mode;
}


// --- InitEncoder ---
bool FFMpegHandler::initEncoder(AVCodecID codec_id)
{
	encoderCtx	= avcodec_alloc_context3(NULL);
	encoder		= avcodec_find_encoder(codec_id);

	if (avcodec_open2(encoderCtx, encoder, NULL) < 0)
	{
		fprintf(stderr, "FFMpegHandler --> Unable to initialize encoder\n");
		return false;
	}

	return true;
}


// --- InitDecoder ---
bool FFMpegHandler::initDecoder(AVCodecID codec_id, bool allocate)
{
	if (allocate)
		decoderCtx = avcodec_alloc_context3(NULL) ;

	decoder	= avcodec_find_decoder(codec_id);

	if (avcodec_open2(decoderCtx, decoder, NULL) < 0)
	{
		fprintf(stderr, "FFMpegHandler --> Unable to initialize decoder\n");
		return false;
	}

	i_frame = avcodec_alloc_frame();
	o_frame	= avcodec_alloc_frame();
	s_frame	= avcodec_alloc_frame();
	o_size	= -1;

	return true;
}


// --- SetDecoderParams ---
void FFMpegHandler::setDecoderParams(int channels, int sample_rate)
{
	decoderCtx->channels	= channels;
	decoderCtx->sample_rate	= sample_rate;
}


// --- InitFrame ---
void FFMpegHandler::initFrame()
{
	frame = new MediumFrame;

	/** av_malloc aligns data for us! */
	frame->samples = (int16_t*)av_malloc( MAX_AUDIO_FRAME_SIZE * sizeof(int16_t) );
}


// --- Decode ---
MediumFrame* FFMpegHandler::decode(uint8_t *buf, int buf_len)
{
	int bytes = -1;

	switch(av_ctx)
	{
		case AudioCtx:
			frame->len	= MAX_AUDIO_FRAME_SIZE;
			AVPacket avpacket;
			avpacket.size = buf_len;
			avpacket.data = buf;
			bytes		= avcodec_decode_audio3(decoderCtx, frame->samples, &frame->len, &avpacket);

			if (m_info()->audio_channels)
				frame->len /= m_info()->audio_channels;

			break;

		case VideoCtx:
			break;

		default:
			break;
	}

	if (bytes <= 0)
	{
		fprintf(stderr, "FFMpegHandler --> Decoding Failed, NULL Frame Returned\n");
		return NULL;
	}

	return frame;
}


// --- DecodeMulti ---
vector<MediumFrame*> FFMpegHandler::decodeMulti(uint8_t* buf, int buf_len)
{
	int bytes = 0;
	int toDecode = buf_len;
	vector<MediumFrame*> frames;

	switch(av_ctx)
	{
		case AudioCtx:
			while(toDecode > 0)
			{
				MediumFrame* f = new MediumFrame;

				f->samples	= new int16_t[MAX_AUDIO_FRAME_SIZE];
				f->len		= MAX_AUDIO_FRAME_SIZE;
				AVPacket avpacket;
				avpacket.data = buf;
				avpacket.size = toDecode;
				bytes		= avcodec_decode_audio3(decoderCtx, f->samples, &f->len, &avpacket);

				buf += bytes;
				toDecode -= bytes;
					
				if (bytes < 0)
				{
					fprintf(stderr, "FFMpegHandler --> Decoding Failed, NULL Frame Returned\n");
					return frames;
				}

				frames.push_back(f);
			}

			break;

		case VideoCtx:
			break;

		default:
			break;
	}
	
	return frames;
}



// --- Encode ---
MediumFrame* FFMpegHandler::encode(uint8_t* buf, int buf_len)
{
	frame->len = FF_MIN_BUFFER_SIZE;
	int err = avcodec_encode_audio(encoderCtx, encBuf, buf_len, (const short int*)buf);

	if (err < 0)
		return NULL;

	frame->samples = (int16_t*)encBuf;
	return frame;
}


// --- Write ---
bool FFMpegHandler::m_write(MediumFrame*)
{
	return true;
}


// --- Read ---
MediumFrame* FFMpegHandler::m_read()
{
	AVPacket		pkt;
	MediumFrame*	frame = NULL;
	int				err;
	bool			frame_ok = false;

	// 0 - Initializations
	av_init_packet(&pkt);

	// 1 - Reading Frame
	switch(av_ctx)
	{
		case AudioCtx:
			while(!frame_ok)
			{
				err = av_read_frame(formatCtx, &pkt);

				// 2 - Decoding
				if (!err)
				{
					if (pkt.stream_index == a_streams.at(0))
					{
						current_ts += step;
						frame 		= decode(pkt.data, pkt.size);

						if (frame)
						{
							frame->ts	= pkt.pts * audio_time_base;
							frame->vlen	= pkt.size;
							frame_ok	= true;
						}

						av_free_packet(&pkt);
					}
				}
				else
				{
					return NULL;
				}
			}
			break;

		case VideoCtx:
			frame = m_next_frame();
			break;

		default:
			break;
	}

	return frame;
}


// --- CurrentFrame ---
MediumFrame* FFMpegHandler::m_current_frame()
{
	return convertToRGB();
}


// --- PrevFrame ---
MediumFrame* FFMpegHandler::m_previous_frame()
{
	AVPacket	pkt;
	double		target_ts, previous_ts;

	// 0 - Initializing
	stream_seek = true;
	target_ts	= current_ts - time_base;
	previous_ts	= target_ts - key_frame_interval * 2;

	if (previous_ts < 0.0)
		previous_ts = 0.0;

	// 1 - Seeking to previous keyframe (N-2)
	av_init_packet(&pkt);
	avcodec_flush_buffers(decoderCtx);

	// -- Precise Seek --
	m_seek(target_ts);


	// 2 - Incremental Seek, Frame by Frame
	long diff;

	fprintf(stderr, "FFMpegHandler --> DTS / Target (previous) : %f / %f\n", current_dts, target_ts);
	diff = (long)( (target_ts - current_dts) * 1000.0 );

	while(diff > 0)
	{
		m_next_frame();
		diff = (long)( (target_ts - current_dts) * 1000.0 );
	}

	fprintf(stderr, "FFMpegHandler --> Frame Found at %f\n", current_dts);

	current_ts	= current_dts;
	stream_seek	= false;

	return convertToRGB();
}


// --- NextFrame ---
MediumFrame* FFMpegHandler::m_next_frame()
{
	AVPacket	pkt;
	int			err;
	int			bytes;
	int			frame_ok = 0;
	double		dts_shift;
	double		pts;

	// -- Inits --
	av_init_packet(&pkt);

	// -- Calcul Framerate --
	if (time_base == 0.0)
	{
		time_base	= av_q2d(decoderCtx->time_base);
		
		// -- Prevent High-Incorrect FrameRates --
		if (time_base < 0.03)	// ~33 fps
			time_base 	*= 2.0;

		frame_rate	= 1.0 / time_base;
	}

	
	// -- Frame decoding --
	frame->vlen = 0;

	while(!frame_ok)
	{
		err = av_read_frame(formatCtx, &pkt);

		if (!err)
		{
			if (pkt.stream_index == v_streams.at(0))
			{
        		decoderCtx->reordered_opaque= pkt.pts;
		        bytes = avcodec_decode_video2(decoderCtx,
                                       		 i_frame, &frame_ok,
		                                     &pkt);

				if (bytes <= 0)
				{
					fprintf(stderr, "FFMpegHandler --> Unable to decode packet\n");
					return NULL;
				}

        		if( pkt.dts == AV_NOPTS_VALUE && i_frame->reordered_opaque != AV_NOPTS_VALUE)
        		    pts= i_frame->reordered_opaque;
				else
					if(pkt.dts != AV_NOPTS_VALUE)
            			pts= pkt.dts;
			        else
            			pts= 0;

				pts *= video_time_base;

				current_pts = pts;
				current_dts = pts;

				av_free_packet(&pkt);
			}
		}
		else
		{
			fprintf(stderr, "FFMpegHandler --> Error while decoding frame\n");
			return NULL;
		}
	}

	// -- Storing Key-frame default interval --
	if (!stream_seek)
	{
		if (i_frame->key_frame)
		{
			key_frame_interval	= key_frame_counter;
			key_frame_counter	= 0.0;
		}
		else
			key_frame_counter += time_base;


		if (key_frame_interval == 0.0)
			key_frame_interval = 0.5;	// Default KeyFrame Interval (could be in conf)
	}


	if (current_dts > 0.0)
		current_ts = current_dts;

	if (!stream_seek)
		return convertToRGB();
	else
		return NULL;
}


// --- PrevFrame (N-frames interval) ---
MediumFrame* FFMpegHandler::m_previous_frame(int steps)
{
	// -- Updating current timestamp to N-1 --
	if (steps > 1)
		current_ts -= ( (steps-1) * time_base );

	if (current_ts <= 0.0)
	{
		current_ts = 0.0;
		return NULL;
	}

	return m_previous_frame();
}


// --- NextFrame (N-frames interval) ---
MediumFrame* FFMpegHandler::m_next_frame(int steps)
{
	MediumFrame *f;

	for(int i=0; i<steps; i++)
		f = m_next_frame();

	return f;
}


// --- Seek ---
bool FFMpegHandler::m_seek(double ts)
{
	if (ts <= 0.0)
		ts = 0.0;

	double	loop_ts		= ts;
	int		seek_flags	= 0;
	int		idx			= -1;
	int		max_tries	= 0;
	bool	seek_ok		= false;
	float	ref_dts;

	stream_seek = true;

	if (ts < current_ts)
		seek_flags = AVSEEK_FLAG_BACKWARD;

	switch(av_ctx)
	{
		case AudioCtx:
			avcodec_flush_buffers(decoderCtx);

			if (ts >= m_info()->audio_duration)
				ts = m_info()->audio_duration;

			idx			= av_seek_frame(formatCtx, a_streams[0], ts / audio_time_base, seek_flags);
			current_ts	= ts;
			break;

		case VideoCtx:
			if (ts >= m_info()->video_duration)
			{
				current_ts = m_info()->video_duration;

				return true;
			}

			loop_ts -= key_frame_interval * 2.0;

			while(!seek_ok && max_tries < 5)
			{
				if (loop_ts < 0.0)
					loop_ts = 0.0;

				avcodec_flush_buffers(decoderCtx);

				idx	= av_seek_frame(formatCtx, v_streams[0], loop_ts / video_time_base, AVSEEK_FLAG_BACKWARD);

				// -- Checkin' --
				m_next_frame();
				ref_dts = current_dts;

				// -- Seeking to previous keyframe zone --
				if ( (current_dts > ts) && (ts > 0.0) )
				{
					max_tries++;
					loop_ts -= key_frame_interval * 2.0;
				}
				else
				if (idx >= 0)
					seek_ok = true;
			}

			if (!seek_ok)
			{
				fprintf(stderr, "FFMpegHandler --> Seek Failed\n");
				stream_seek = false;
				
				return false;
			}

			// -- Steppin' forward 'til closest previous frame --
			while (current_dts - (ts - time_base) < 0.001)
			{
				m_next_frame();

				if (current_dts - ref_dts < 0.001)
				{
					fprintf(stderr, "FFMpegHandler --> NULL Frame encountered while seeking : %f / %f\n", current_dts, ref_dts);
					stream_seek = false;

					current_ts = current_dts;
					
					return false;
				}
			}

			current_ts = current_dts;
			break;

		default:
			break;
	}

	stream_seek = false;

	return (idx >= 0);
}


// --- Rewind ---
bool FFMpegHandler::m_back()
{
	int idx = -1;

	avcodec_flush_buffers(decoderCtx);

	switch(av_ctx)
	{
		case AudioCtx:
			idx	= av_seek_frame(formatCtx, a_streams[0], 0.0, AVSEEK_FLAG_BACKWARD);
			break;

		case VideoCtx:
			idx	= av_seek_frame(formatCtx, v_streams[0], 0.0, AVSEEK_FLAG_BACKWARD);
			break;

		default:
			break;
	}

	return (idx >= 0);
}


// --- GetStreamInfo ---
MediumInfo* FFMpegHandler::m_info()
{
	if (!info_retrieved)
	{
		m_formatDescription();

		switch (av_ctx)
		{
			case AudioCtx:
				s_info.audio_channels		= decoderCtx->channels;
				s_info.audio_frame_size		= decoderCtx->frame_size;
				s_info.audio_sample_rate	= decoderCtx->sample_rate;
				s_info.audio_codec			= decoder->name;

				if (formatCtx != NULL)
				{
					s_info.audio_duration	= (double)formatCtx->duration / (double)AV_TIME_BASE;
					s_info.audio_samples	= (long) s_info.audio_duration * s_info.audio_frame_size;
				}

				/** Debug display */
				/*
				printf("AudioStream::channels		= %i\n", s_info.audio_channels);
				printf("AudioStream::frame_size		= %i\n", s_info.audio_frame_size);
				printf("AudioStream::sample_rate	= %i\n", s_info.audio_sample_rate);
				printf("AudioStream::codec			= %s\n", s_info.audio_codec.c_str());
				printf("AudioStream::duration		= %f\n", s_info.audio_duration);
				printf("AudioStream::samples		= %i\n", s_info.audio_samples);
				*/

				break;

			case VideoCtx:
				s_info.video_channels		= decoderCtx->channels;
				s_info.video_frame_size		= decoderCtx->frame_size;
				s_info.video_width			= decoderCtx->width;
				s_info.video_height			= decoderCtx->height;
				s_info.video_codec			= decoder->name;
				s_info.video_fps			= 1.0 / time_base;
				s_info.video_aspect_ratio	= av_q2d( decoderCtx->sample_aspect_ratio );

				if (formatCtx != NULL)
				{
					s_info.video_duration	= (double)formatCtx->duration / (double)AV_TIME_BASE;
					s_info.video_samples	= (long) s_info.video_duration * s_info.video_frame_size;
				}
				break;

			default:
				break;
		}

		step			= (double)decoderCtx->frame_size / (double)decoderCtx->sample_rate;
		info_retrieved	= true;
	}

	return &s_info;
}


// --- FormatDescription ---
void FFMpegHandler::m_formatDescription()
{
	int sample_resolution = -1;

	// -- Data Encoding --
	switch (decoderCtx->sample_fmt)
	{
		case AV_SAMPLE_FMT_U8		: sample_resolution = 8;	break;
		case AV_SAMPLE_FMT_S16		: sample_resolution = 16;	break;
		case AV_SAMPLE_FMT_S32		: sample_resolution = 32;	break;
		default					: sample_resolution = -1;	break;
	}

	switch (av_ctx)
	{
		case AudioCtx:
			s_info.audio_sample_resolution = sample_resolution;
			break;

		case VideoCtx:
			s_info.video_sample_resolution = sample_resolution;
			break;

		default:
			break;
	}
}

// --- GetFrameTimeStamp ---
double FFMpegHandler::m_get_frame_ts(int f_nbr)
{
	return (f_nbr * time_base);
}

// --- GetAbsoluteTime ---
double FFMpegHandler::m_get_absolute_time(int time_s, int frame_shift)
{
	double abs_time = (double)time_s + (double)frame_shift * time_base;

	return abs_time;
}


// --- GetRelativeTime ---
void FFMpegHandler::m_get_relative_time(double time_f, int& time_s, int& frame_shift)
{
	time_s		= floor(time_f);
	frame_shift = m_get_frame_shift(time_f);
}


// --- GetFrameShift ---
int FFMpegHandler::m_get_frame_shift(double time_f)
{
	double decs		= time_f - floor(time_f);
	double shift	= decs / time_base;

	if ( (shift - floor(shift)) < 0.001 )
 		return floor(shift);
	else
		return ceil(shift);
}


// --- GetFrameNumber ---
int FFMpegHandler::m_get_frame_number(double time_f)
{

	double nbr = time_f / time_base;

	if ( (nbr - floor(nbr)) < 0.001 )
		return floor(nbr);
	else
		return ceil(nbr);
}


// --- GetNearestFrame ---
double FFMpegHandler::m_get_nearest_ts(double time_f)
{
	// -- New Algorithm --
	double ts_base, ts_shift, ts_near;

	ts_base		= floor(time_f);
	ts_shift	= m_get_frame_shift(time_f);
	ts_near		= ts_base + time_base * ts_shift;

	return ts_near;
}


// --- SetDimensions ---
bool FFMpegHandler::m_set_dimensions(int in_w, int in_h)
{
    if (v_width == in_w && v_height == in_h)
        return false;

    v_width     = in_w;
    v_height    = in_h;

    // -- Scaling Context Init --
    if (scaler_rgb)
        sws_freeContext(scaler_rgb);

    scaler_rgb = sws_getContext(m_info()->video_width,
                                 m_info()->video_height,
                                 decoderCtx->pix_fmt,
                                 v_width,
                                 v_height,
                                 PIX_FMT_RGB24,
                                 SWS_FAST_BILINEAR,
                                 NULL, NULL, NULL);

    if (scaler_rgb == NULL)
    {
        fprintf(stderr, "FFMpegHandler --> Cannot instantiate Swscale context\n");
        return false;
    }


    // -- Reallocating Buffers --
    if (o_size != -1)
    {
        o_size = -1;

        delete[] o_data;
        av_free(o_frame);
    }

    // -- Buffers Allocation --
    o_size = avpicture_get_size(PIX_FMT_RGB24,
                                v_width,
                                v_height);

    o_frame = avcodec_alloc_frame();
    o_data  = new uint8_t[o_size];

    // -- Output Frame Allocation --
    avpicture_fill( reinterpret_cast<AVPicture*>(o_frame),
                    o_data,
                    PIX_FMT_RGB24,
                    v_width,
                    v_height);

	return true;
}


// --- GetRBGFrame ---
MediumFrame*
FFMpegHandler::m_get_rgb_frame(double ts)
{
	// -- Precision Hack --
	m_seek( m_get_nearest_ts(ts) );

	return convertToRGB();
}


// --- GetRawFrame ---
AVFrame*
FFMpegHandler::m_get_raw_frame(double ts)
{
	m_seek( m_get_nearest_ts(ts) );

	return i_frame;
}


// -----------------------
// --- SCALING METHODS ---
// -----------------------

// --- InitScalers ---
bool FFMpegHandler::initScalers()
{
	// -- RGB Scaler --
	if (scaler_rgb)
		sws_freeContext(scaler_rgb);

	scaler_rgb = sws_getContext( m_info()->video_width,
								 m_info()->video_height,
								 decoderCtx->pix_fmt,
								 m_info()->video_width,
								 m_info()->video_height,
								 PIX_FMT_RGB24,
								 SWS_FAST_BILINEAR,
								 NULL, NULL, NULL );

	if (scaler_rgb == NULL)
	{
		fprintf(stderr, "FFMpegHandler --> Cannot instantiate RGB Scaler\n");
		return false;
	}

	
	// -- Buffers Allocation --
	o_size = avpicture_get_size(PIX_FMT_RGB24,
								m_info()->video_width,
								m_info()->video_height);

	o_frame	= avcodec_alloc_frame();
	o_data	= new uint8_t[o_size];

	// -- RGB Frame Allocation --
	avpicture_fill( reinterpret_cast<AVPicture*>(o_frame),
					o_data,
					PIX_FMT_RGB24,
					m_info()->video_width,
					m_info()->video_height);

	return true;
}


// --- ConvertToRGB ---
MediumFrame*
FFMpegHandler::convertToRGB()
{
	if (!scaler_rgb)
		return NULL;

	// -- Scaling --
	sws_scale(scaler_rgb,
				i_frame->data,
				i_frame->linesize,
				0,
				m_info()->video_height,
				o_frame->data,
				o_frame->linesize);

	frame->v_samples	= o_frame->data;
	frame->v_linesize	= o_frame->linesize;
	frame->ts			= current_ts;
	frame->v_width		= v_width;
	frame->v_height		= v_height;
	frame->vlen			= avpicture_get_size(PIX_FMT_RGB24, v_width, v_height);

	return frame;
}

