/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "RTSPHandler.h"

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


// --- Thread Caller ---
gpointer RTSPHandler_play(gpointer data)
{
	RTSPHandler* handler = (RTSPHandler*)data;
	handler->getSession()->play();

	g_thread_exit(0);

	return NULL;
}


// --- RTSPHandler ---
RTSPHandler::RTSPHandler()
{
	play_thread	= NULL;
	is_playing	= false;
	initialized	= false;
	thread_lock	= false;

	session		= new RTSPSession();
	rtsp_mutex	= g_mutex_new();
}


// --- ~RTSPHandler ---
RTSPHandler::~RTSPHandler()
{
	delete session;
}


// --- Init ---
bool
RTSPHandler::m_init()
{
	return true;
}


// --- Open ---
bool
RTSPHandler::m_open(const char* in_medium, int)
{
	medium = (char*)in_medium;

	session->setMedium(medium);
	initialized = session->initSession();

	return initialized;
}


// --- Close ---
bool
RTSPHandler::m_close()
{
	m_stop();
	session->shutdownSession();
	return true;
}


// --- Read ---
MediumFrame*
RTSPHandler::m_read()
{
	rtsp_timer.start();

	MediumFrame *f = session->getFrame();

	while( (f == NULL) &&
			session->isBuffering() &&
			(rtsp_timer.elapsed() <= 1.500) )
	{
		f = session->getFrame();
	}

	//printf("[RTSPHandler] - Frame Read\n");

	rtsp_timer.stop();
	rtsp_timer.reset();
	
	return f;
}


// --- Write ---
bool
RTSPHandler::m_write(MediumFrame*)
{
	return true;
}

// --- StreamInfo ---
MediumInfo*
RTSPHandler::m_info()
{
	return session->getInfo();
}


// --- Play ---
bool
RTSPHandler::m_play()
{
	if (!is_playing)
	{
		play_thread	= g_thread_create(RTSPHandler_play, this, true, NULL);
		is_playing	= true;
	}

	return is_playing;
}


// --- Pause ---
bool
RTSPHandler::m_pause()
{
	return true;
}


// --- Stop ---
bool
RTSPHandler::m_stop()
{
	// -- Mutex Lock --
	bool locked = false;

	while (!locked)
	{
		locked = g_mutex_trylock(rtsp_mutex);
		USLEEP(10*1000);
	}

	if (!locked)
	{
		fprintf(stderr, "RTSPHandler --> Already Stopped by another call\n");
		return true;
	}

	if (is_playing)
	{
		if (play_thread != NULL)
		{
			session->stopPlayback();

			g_thread_join(play_thread);
			play_thread = NULL;
		}

		is_playing	= false;
	}

	// -- Mutex unlock --
	g_mutex_unlock(rtsp_mutex);
	return true;
}


// --- Seek ---
bool
RTSPHandler::m_seek(double offset)
{
	session->seek(offset);
	return true;
}

