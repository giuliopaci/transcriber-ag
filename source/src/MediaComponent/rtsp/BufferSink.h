/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef BUFFERSINK_H
#define BUFFERSINK_H

#include <queue>
#include "MediaSink.hh"
#include "MediaComponent/io/FFMpegHandler.h"

#define MIN_RTSP_BUFFER	 10

/**
 * @class	BufferSink
 * @ingroup	MediaComponent
 *
 * This class implements a RTSP sink system, based on Live555 root classes.
 * An RTSP sink simply receives RTSP packets, and agregate them into a complete frame.
 * Once a frame is complete, it is immediately available.
 */
class BufferSink : public MediaSink
{
public:
	/**
	 * Default Constructor (with inherited UsageEnvironment instance)
	 * @param env Environment instance(provided by parent object, RTSPSession)
	 */
	BufferSink(UsageEnvironment& env);

	/**
	 * Virtual Default Destructor
	 */
	virtual				~BufferSink();

	/**
	 * Instantiates a new BufferSink instance (static)
	 * @param env Environment instance
	 */
	static BufferSink*	createNew(UsageEnvironment& env);

	/**
	 * Gets current buffer
	 * @return Current buffer
	 */
	unsigned char*		buffer()				{ return fBuffer; }

	/**
	 * Provides MediumInfo structure
	 * @return MediumInfo (pointer)
	 */
	MediumInfo*			getInfo()				{ return info; }

	/**
	 * Buffering status
	 * @return True if sink is still buffering, false otherwise.
	 */
	bool				isBuffering()			{ return buffering; }

	/** Buffering status setter
	 * @param b buffering flag (bool)
	 */
	void				setBuffering(bool b)	{ buffering = b; }

	/**
	 * Access to currently decoded frame
	 * @return Current decoded frame
	 */
	MediumFrame*		getFrame();

	/**
	 * Empties frame queue 
	 */
	void				flushQueue();

	/**
	 * Writes a complete RTSP frame into our MediumFrame instance
	 * @param len Input frame length
	 * @param ts
	 */
	void				write(unsigned len, struct timeval ts);

	/**
	 * Configures IODevice decoder, according to RTP payload
	 * @param rtpPayloadFormat	RTP payload format
	 * @param codecName			Codec name
	 * @param numChannels		Channels count
	 * @param sampleRate		Samplerate
	 * @param duration			Stream duration (in seconds)
	 */
	void				setAudioSettings(int	rtpPayloadFormat,
										 char*	codecName,
										 int	numChannels,
										 int	sampleRate,
										 double	duration);

private:
	void				initAudio(unsigned f_len);
	virtual 			Boolean continuePlaying();
	void				afterGettingFrame1(unsigned			frameSize,
										   struct timeval	presentationTime);
	static void			afterGettingFrame(void*		clientData,
										  unsigned	frameSize,
										  unsigned	truncatedBytes,
										  struct	timeval presentationTime,
										  unsigned	durationInMicroseconds);


	// -- MediaComponent Objects --
	FFMpegHandler*		ffmpeg;
	MediumFrame*		m_frame;
	MediumFrame*		m_prev_frame;
	MediumInfo			*info;
	queue<MediumFrame*>	frames;
	bool				initialized;
	bool				buffering;

	// -- Internals --
	unsigned char*		fBuffer;
	unsigned			fBufferSize;
	AVCodecID			ffmpeg_codec_id;
	int					sink_sample_rate;
	int					sink_frame_size;
	int					sink_channels;
	int					sink_rtp_payload;
	double				sink_duration;
	char*				sink_codec_name;
};

#endif
