/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

// Based on the blocking I/O API from PortAudio

#ifndef PORTAUDIOSTREAM_H
#define PORTAUDIOSTREAM_H

#define D_CHANNELS	2
#define D_SRATE		44100
#define D_FPS		2048

#include <stdint.h>
#include <portaudio.h>
#include <iostream>
#include "Common/portabilite.h"

/**
 * @class	PortAudioStream
 * @ingroup	MediaComponent
 *
 * This device writes audio samples to soundcards, using the PortAudio API
 */
class PortAudioStream
{
public:
	/**
	 * Default Constructor
	 */
	PortAudioStream();

	/**
	 * Constructor with PortAudio settings
	 * @param channels		Output channels
	 * @param sample_rate	Samplerate
	 * @param frame_size	Frame size
	 */
	PortAudioStream(int channels, int sample_rate, int frame_size);

	/**
	 * Destructor
	 */
	~PortAudioStream();

	/**
	 * Initializes a new PortAudio stream
	 * @param channels		Output channels
	 * @param sample_rate	Samplerate
	 * @return True on success, false otherwise;
	 */
	bool				init(int channels, int sample_rate);	// Channels, samplerate

	/**
	 * Initializes a new PortAudio stream
	 * @param channels		Output channels
	 * @param sample_rate	Samplerate
	 * @param frame_size	Frame size
	 * @return True on success, false otherwise;
	 */
	bool				init(int channels, int sample_rate, int frame_size);	// Channels, samplerate, frame_size

	/**
	 * @param samples	Audio samples
	 * @param len		Samples count
	 * @return PaError (PortAudio Error ID)
	 */
	PaError				write(int16_t* samples, int len);	// Samples buffer, samples count

	/**
	 * Terminates PortAudio stream
	 * @return True on success, false otherwise.
	 */
	bool				terminate();

	// --------------
	// -- Controls --
	// --------------
	/**
	 * Initialization status
	 * @return True if initialized, false otherwise.
	 */
	bool				isInitialized();

	/**
	 * Active status
	 * @return True if active, false otherwise.
	 */
	bool				isActive();

private:
	PaStream*			pa_stream;
	const PaDeviceInfo*	pa_info;
	PaError				pa_error;
	int					pa_frame_size;

	// -- State Variables --
	bool				initialized;
	bool				pa_device_ready;
	bool				mute;
};

#endif

