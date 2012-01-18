/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef SNDFILEHANDLER_H
#define SNDFILEHANDLER_H

#include <sndfile.h>
#include <string.h>

#include "IODevice.h"

/**
 * @class	SndFileHandler
 * @ingroup	MediaComponent
 *
 * IODevice based on libsndfile library
 */
class SndFileHandler : public IODevice
{
public:
	/**
	 * Constructor
	 */
	SndFileHandler();

	// -- IODevice API --
	bool			m_init();
	bool			m_open(const char *url, int mode = SFM_READ);
	bool			m_close();
	bool			m_write(MediumFrame*);
	bool			m_seek(double);
	bool			m_back();
	MediumFrame*	m_read();
	MediumInfo*		m_info();

	// ---------------
	// -- Internals --
	// ---------------

	/**
	 * Instantiates MediumInfo structure with provided parameters
	 * @param channels		Channels count
	 * @param samplerate	Sample rate
	 * @param format		Medium format
	 */
	void			m_set_sfinfo(int channels, int samplerate, int format);
	
	/**
	 * MediumInfo accessor
	 * @return MediumInfo instance (pointer)
	 */
	SF_INFO*		m_sfinfo();

	/**
	 * Formats codec information in a fancy way
	 * @param format_id	Format ID
	 */
	void			m_formatDescription(int format_id);

private:
	// -- SNDfile --
	SF_INFO			file_info;
	SF_VIRTUAL_IO	vio;
	SNDFILE*		s_file;
	MediumFrame*	f;
	MediumInfo		s_info;

	bool			info_retrieved;
	float			current_ts;
	float			step;
};

#endif

