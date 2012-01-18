/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "SilentHandler.h"

// --- Constructor ---
SilentHandler::SilentHandler(int inTracks, double inDuration)
	: nbTracks(inTracks), duration(inDuration)
{
	// -- Inits --
	info_retrieved = false;
	m_init();

	// -- Building fake informations --
	m_info();
}


// --- Destructor ---
SilentHandler::~SilentHandler()
{
	delete[] silence->samples;
	delete silence;
}


// --- Initialization ---
bool
SilentHandler::m_init()
{
	silence = new MediumFrame();

	silence->samples	= new int16_t[ nbTracks * AUDIO_BUFFER_SIZE ];
	silence->len		= AUDIO_BUFFER_SIZE;

	for(int i=0; i<silence->len * nbTracks; i++)
		silence->samples[i] = '\0';
}


// --- Seek ---
bool
SilentHandler::m_seek(double ts)
{
	current_ts = ts;

	if (current_ts < 0.0)
		current_ts = 0.0;

	if (current_ts > m_info()->audio_duration)
		current_ts = m_info()->audio_duration;

	return true;
}


// --- Read ---
MediumFrame*
SilentHandler::m_read()
{
	current_ts += step;

	if (current_ts >= m_info()->audio_duration)
		return NULL;

	return silence;
}


// --- Information ---
MediumInfo*
SilentHandler::m_info()
{
	if (!info_retrieved)
	{
		// -- Filling --
		s_info.audio_channels			= nbTracks;
		s_info.audio_frame_size			= AUDIO_BUFFER_SIZE;
		s_info.audio_sample_rate		= AUDIO_SAMPLE_RATE;
		s_info.audio_duration			= duration;
		s_info.audio_samples			= (long)duration * s_info.audio_sample_rate;
		s_info.audio_codec				= "unknown";
		s_info.audio_encoding			= "unknown";
		s_info.audio_sample_resolution	= 16;

		step = (float)s_info.audio_frame_size / (float)s_info.audio_sample_rate;
		info_retrieved = true;
	}

	return &s_info;
}

