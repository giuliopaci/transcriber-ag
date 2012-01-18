/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef RTSPHANDLER_H
#define RTSPHANDLER_H

// -- Glib --
#include <glib.h>
#include <glib/gstdio.h>
#include <glibmm/timer.h>

#include "MediaComponent/rtsp/RTSPSession.h"

/**
 * @class	RTSPHandler
 * @ingroup	MediaComponent
 *
 * IODevice based on live555 libraries
 * This class is a RTSP client, dedicated to remote media.
 */
class RTSPHandler : public IODevice
{
public:
	/**
	 * Constructor
	 */
	RTSPHandler();
	
	/**
	 * Destructor
	 */
	~RTSPHandler();

	// ------------------
	// -- IODevice API --
	// ------------------
	bool			m_init();
	bool			m_initialized()	{ return initialized; }
	bool			m_open(const char *url, int mode = 0);
	bool			m_close();
	bool			m_write(MediumFrame*);
	MediumFrame*	m_read();
	bool			m_play();
	bool			m_pause();
	bool			m_stop();
	bool			m_seek(double);
	MediumInfo*		m_info();

	/**
	 * Retrieves RTSP session instance
	 * @return Session instance (pointer)
	 */
	RTSPSession*	getSession()	{ return session; }

protected:

private:
	RTSPSession*	session;
	MediumInfo		s_info;
	GThread*		play_thread;
	GMutex*			rtsp_mutex;
	Glib::Timer		rtsp_timer;
	bool			is_playing;
	bool			initialized;
	bool			thread_lock;
};

#endif

