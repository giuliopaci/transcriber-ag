/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "BufferSink.h"
#define sink_buffer_size 20000

BufferSink::BufferSink(UsageEnvironment& env)
	: MediaSink(env)
{
	initialized = false;
	buffering	= false;

	// -- PortAudio --
	ffmpeg			= NULL;
	m_prev_frame	= NULL;
	fBuffer			= new unsigned char[sink_buffer_size];
	fBufferSize		= sink_buffer_size;
}


BufferSink::~BufferSink()
{
	if (ffmpeg)
		delete ffmpeg;

	delete[] fBuffer;
}


// --- InitAudio ---
void BufferSink::initAudio(unsigned len)
{
	if (initialized)
		return;

	ffmpeg	= new FFMpegHandler();
	ffmpeg->m_init();
	ffmpeg->initFrame();
	ffmpeg->setHandlerContext(DecoderCtx);
	ffmpeg->initDecoder(ffmpeg_codec_id, true);

	// - Frame Size Adjustments -
	if (sink_rtp_payload != 14)
	{
		sink_frame_size = len;
		sink_frame_size /= 2 * sink_channels;
	}

	info->audio_frame_size = sink_frame_size;

	// -- FFmpeg Settings --
	ffmpeg->setDecoderParams(sink_channels, sink_sample_rate);

	MediumInfo *f_info = ffmpeg->m_info();
	f_info = info;
	initialized = true;
}


// --- Write ---
void BufferSink::write(unsigned len, struct timeval ts)
{
	vector<MediumFrame*> inFrames;

	inFrames = ffmpeg->decodeMulti( buffer(), len);

	// -- MP3 adjustments --
	if (sink_rtp_payload == 14)
	{
		info->audio_sample_rate	= ffmpeg->getDecoderCtx()->sample_rate;
		info->audio_channels	= ffmpeg->getDecoderCtx()->channels;
		info->audio_frame_size	= ffmpeg->getDecoderCtx()->frame_size;
	}

	// -- Filling buffer --
	if (inFrames.size() > 0)
		for(int i=0; i<inFrames.size(); i++)
			frames.push( inFrames.at(i) );
}


// --- GetFrame ---
MediumFrame* BufferSink::getFrame()
{
	if (m_prev_frame)
	{
		delete [] m_prev_frame->samples;
		delete m_prev_frame;
	}

	if (frames.size() > MIN_RTSP_BUFFER)
	{
		buffering = false;
		MediumFrame* f = frames.front();
		frames.pop();


		// -- Deleting previous frame, we're assured it's unused now --
		m_prev_frame = f;

		return f;
	}

	// -- Fallback (if NULL frame) --
	if (!buffering && (frames.size() > 0) )
	{
		buffering = false;
		MediumFrame* f = frames.front();
		frames.pop();

		// -- Deleting previous frame, we're assured it's unused now --
		m_prev_frame = f;
		
		return f;
	}

	// -- Reset --
	m_prev_frame = NULL;
	
	return NULL;
}


// --- FlushQueue ---
void BufferSink::flushQueue()
{
	MediumFrame* f;

	while( !frames.empty() )
	{
		f = frames.front();
		frames.pop();
		delete [] f->samples;
		delete f;
	}
}


// -- CreateNew --
BufferSink* BufferSink::createNew(UsageEnvironment& env)
{
	return new BufferSink(env);
}


// --- AfterGettingFrame ---
void BufferSink::afterGettingFrame(void* clientData,
								unsigned frameSize,
								unsigned truncatedBytes,
								struct timeval presentationTime,
								unsigned durationInMicroseconds)
{
	BufferSink* sink = (BufferSink*)clientData;

	//double ts = double(presentationTime.tv_sec) + double(presentationTime.tv_usec / (1000*1000));

	sink->initAudio(frameSize);
	sink->write(frameSize, presentationTime);
	sink->afterGettingFrame1(frameSize, presentationTime);
}



// --- AfterGettingFrame1 ---
void BufferSink::afterGettingFrame1(unsigned frameSize, struct timeval presentationTime)
{
	continuePlaying();
}


// --- ContinuePlaying ---
Boolean BufferSink::continuePlaying()
{
	if (fSource == NULL)
		return False;

	fSource->getNextFrame(fBuffer, fBufferSize,
						  afterGettingFrame, this,
						  onSourceClosure, this);

  return True;
}



// --------------------
// -   AUDIO HANDLE   -
// --------------------

// --- SetAudioSettings ---
void BufferSink::setAudioSettings(int i_payload, char* i_codec, int i_channels, int i_sample_rate, double i_duration)
{
	info = new MediumInfo;

	sink_rtp_payload	= i_payload;
	sink_codec_name		= i_codec;
	sink_channels		= i_channels;
	sink_sample_rate	= i_sample_rate;
	sink_duration		= i_duration;

	// -- Channels / FrameSize / Codec Adjustments --
	switch(sink_rtp_payload)
	{
		case 0:		// PCM-Mulaw
			ffmpeg_codec_id = CODEC_ID_PCM_MULAW;
			break;

		case 10:	// Uncompressed PCM-Stereo
			ffmpeg_codec_id	= CODEC_ID_PCM_S16BE;
			break;

		case 11:	// Uncompressed PCM-Mono
			ffmpeg_codec_id	= CODEC_ID_PCM_S16BE;
			break;

		case 14:	// Mpeg-Audio (default : stereo@44.1Khz)
			ffmpeg_codec_id		= CODEC_ID_MP3;

			// -- Other parameters will be guessed by FFmpeg decoder itself,
			// -- parsing MP3 frames header	

			break;

		default:
			ffmpeg_codec_id	= CODEC_ID_PCM_S16BE;
	}

	// -- Filling MediumInfo Structure --
	info->audio_frame_size 	= sink_frame_size;
	info->audio_sample_rate	= sink_sample_rate;
	info->audio_channels	= sink_channels;
	info->audio_duration	= sink_duration;
	info->audio_codec		= sink_codec_name;
}

