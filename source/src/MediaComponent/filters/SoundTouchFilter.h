/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef SOUNDTOUCHFILTER_H
#define SOUNDTOUCHFILTER_H

#include "base/Types.h"
#include <string.h>
#include <vector>
#include "SoundTouch.h"
#include "AbstractFilter.h"

using namespace soundtouch;
using namespace std;

/**
 * @class	SoundTouchFilter
 * @ingroup	MediaComponent
 *
 * Role : offers real-time processing effects for an input audio stream\n
 * Effects : pitch control / tempo control\n
 * Based on SoundTouch LGPL library
 */
class SoundTouchFilter : public AbstractFilter
{
public:
	/**
	 * Constructor
	 */
	SoundTouchFilter();

	/**
	 * Destructor
	 */
	~SoundTouchFilter();

	/**
	 * Initializes filter with a MediumInfo structure
	 * @param info	MediumInfo structure (pointer)
	 */
	void		init(MediumInfo* info);

	/**
	 * Puts some audio samples into SoundTouch processor
	 * @return True on success, false otherwise.
	 */
	bool		feed();
	void		filter(MediumFrame*& frame);
	/**
	 * Returns processed samples available in SoundTouch Processor
	 * @return Integer : Available samples
	 */
	int			samplesAvailable();

	// -- Controls --
	/**
	 * Seeks with internal IODevice
	 * @param timestamp Requested timestamp
	 */
	void		seek(float timestamp);
	
	/**
	 * Stops playback
	 */
	void		stop();
	
	/**
	 * Starts playback
	 */
	void		play();

	/**
	 * Sets pitch
	 * @param pitch Pitch factor (from 0.40 to 2.50)
	 */
	void		setPitch(float pitch);

	/**
	 * Sets tempo
	 * @param tempo Tempo factor (from 0.25 to 4.00)
	 */
	void		setTempo(float tempo);

	/**
	 * Sets rate
	 * @param rate Rate factor
	 */
	void		setRate(float rate);

	/**
	 * Sets audio offset (for a specific track)
	 * @param offset Track offset
	 */
	void		setOffset(float offset)	{ f_offset = offset; }

	/**
	 * Returns current pitch
	 * @return Pitch(float)
	 */
	float		getPitch()				{ return f_pitch; }

	/**
	 * Returns current tempo
	 * @return Tempo(float)
	 */
	float		getTempo()				{ return f_tempo; }

	/**
	 * Returns current rate
	 * @return Rate(float)
	 */
	float		getRate()				{ return f_rate; }

private:
	int			fSize;
	int			nSamples;
	int			nChannels;
	int			samplesReceived;

	float		f_pitch;
	float		f_tempo;
	float		f_rate;
	float		f_offset;
	float		f_silence;
	float		f_step;
	double		f_ts;
	int			f_channels;

	// -- SoundTouch --
	SoundTouch*	st;
	int			st_size;
	SAMPLETYPE*	st_output;
	SAMPLETYPE*	st_input;
	SAMPLETYPE*	st_recv;

	// -- IODevice --
	IODevice*	device;
	bool		f_ready;
	bool		f_available;
	bool		stream_end;
};

#endif

