/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef SILENTHANDLER_H
#define SILENTHANDLER_H

#include "IODevice.h"

/**
 * @class	SilentHandler
 * @ingroup	MediaComponent
 *
 * IODevice based on FFmpeg libraries (libavcodec / libavformat) for empty handler
 */
class SilentHandler : public IODevice
{
public:
	/**
	 * Constructor
	 */
	SilentHandler(int inTracks, double inDuration);
	~SilentHandler();


	// -- IODevice API --
	bool			m_init();
	bool			m_open(const char *url, int mode = 0) {};	// mode = 0/1 : mono/stereo
	bool			m_close() {};
	bool			m_write(MediumFrame*) {};
	bool			m_seek(double);
	MediumFrame*	m_read();
	MediumInfo*		m_info();

private:
	MediumFrame*	silence;
	MediumInfo		s_info;
		
	int				nbTracks;
	double			duration;
	double			current_ts;
	double			step;
	bool			info_retrieved;
};

#endif

