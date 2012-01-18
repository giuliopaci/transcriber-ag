/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef RTSPSESSION_H
#define RTSPSESSION_H

// -- LiveMedia ---
#include "liveMedia.hh"
#include "GroupsockHelper.hh"
#include "UsageEnvironment.hh"

// -- MediaComponent --
#include "MediaComponent/rtsp/BufferSink.h"

/**
 * @class	RTSPSession
 * @ingroup	MediaComponent
 *
 * This class implements a RTSP session system, based on Live555 root classes.
 * Multiple subsessions are allowed.
 */
class RTSPSession
{
public:
	/**
	 * Default Constructor
	 */
	RTSPSession();

	/**
	 * Default Destructor
	 */
	~RTSPSession();

	/**
	 * Starts RTSP stream playback
	 * @return True on success, false otherwise.
	 */
	bool				play();

	/**
	 * Stops RTSP stream playback
	 * @return True on success, false otherwise.
	 */
	bool				stop();

	/**
	 * Seeks into RTSP stream
	 * @param offset	Seek position (in seconds)
	 */
	void				seek(double offset);

	/**
	 * Initialize RTSP streams
	 */
	void				setupStreams();
	
	/**
	 * TearDown RTSP streams
	 */
	void				tearDownStreams();
	
	/**
	 * Closes Media Sinks (BufferSink)
	 */
	void				closeMediaSinks();

	/**
	 * Initializes new session instance
	 * @return True on success, false otherwise.
	 */
	bool				initSession();

	/**
	 * Initializes all new subsessions instances
	 */
	void				initSubsessions();

	/**
	 * Shutdowns & destroys current session
	 */
	void				shutdownSession();

	/**
	 * Sets remote medium URL
	 * @param rtsp_url	Medium URL
	 */
	void				setMedium(char*	rtsp_url)	{ url = rtsp_url; }

	/**
	 * Stops session eventloop by setting control variable to 0xff (quit)
	 */
	void				stopPlayback()				{ event = 0xff; }

	/**
	 * Buffering Status
     * @return True if device is still buffering, false otherwise.
	 */
	bool				isBuffering()				{ return sink->isBuffering(); }

	/**
	 * Playback Status
     * @return True if playback is on, false otherwise.
	 */
	bool				isPlaying()					{ return playing; }

	/**
	 * Access to current medium frame, via internal sink object
     * @return A MediumFrame instance (pointer), or NULL
	 */
	MediumFrame*		getFrame()					{ return sink->getFrame(); }

	/**
	 * Access to MediumInfo structure
     * @return A MediumInfo instance (pointer)
	 */
	MediumInfo*			getInfo()					{ return sink->getInfo(); }

private:
	RTSPClient*			client;
	FramedSource*		source;
	BufferSink*			sink;
	RTCPInstance*		rtcpInstance;
	MediaSession*		session;
	UsageEnvironment*	env;
	TaskScheduler*		scheduler;

	// -- Medium --
	char*				url;
	char				event;
	double				offset;
	bool				playing;
};

#endif
