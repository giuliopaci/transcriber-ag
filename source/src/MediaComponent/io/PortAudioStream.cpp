/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include <stdio.h>
#include "PortAudioStream.h"

// --- PortAudioStream() ---
PortAudioStream::PortAudioStream()
	: initialized(false), mute(false), pa_device_ready(false)
{}


// --- PortAudioStream(3 params) ---
PortAudioStream::PortAudioStream(int channels, int sample_rate, int frame_size)
	: initialized(false), mute(false), pa_device_ready(false)
{
	init(channels, sample_rate, frame_size);
}


// --- ~PortAudioStream ---
PortAudioStream::~PortAudioStream()
{}


// --- IsInitialized ---
bool PortAudioStream::isInitialized()
{
	return initialized;
}


// --- Init(2 params) ---
bool PortAudioStream::init(int channels, int sample_rate)
{
	return init(channels, sample_rate, D_FPS);
}


// --- Init(3 params) ---
bool PortAudioStream::init(int channels, int sample_rate, int frame_size)
{
	pa_frame_size = frame_size;

	int pa_tries = 0;

	while ( (pa_tries < 10) && (!initialized) )
	{
		if (pa_tries > 0)
			USLEEP(100 * 1000); // sleepin' 100ms

		pa_tries++;


		// -- PortAudio Internal Init --
		pa_error = Pa_Initialize();

		if (pa_error != paNoError)
		{
			fprintf(stderr, "PortAudio --> Internal Init Failed @Iteration %i\n", pa_tries);
			fprintf(stderr, "PortAudio --> Error Description : %s\n", Pa_GetErrorText(pa_error));
			Pa_Terminate();
			
			continue;
		 }


		// -- Device Allocation --
		pa_info = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());

		if (pa_info == NULL)
		{
			fprintf(stderr, "PortAudio --> Device Unavailable @Iteration %i\n", pa_tries);
			Pa_Terminate();

			continue;
		}


		// -- Device Configuration --
		PaStreamParameters* params = new PaStreamParameters();

		params->device			= Pa_GetDefaultOutputDevice();
		params->channelCount	= channels;
		params->sampleFormat	= paInt16;


		// -- Opening Stream --
		pa_error = Pa_OpenDefaultStream(&pa_stream, 0, channels, paInt16, (double)sample_rate, paFramesPerBufferUnspecified, NULL, NULL);

		if (pa_error != paNoError)
		{
			fprintf(stderr, "PortAudio --> Output Stream Not Available @Iteration %i\n", pa_tries);
			fprintf(stderr, "PortAudio --> Error Description : %s\n", Pa_GetErrorText(pa_error));
			terminate();

			continue;
		}


		// -- Starting Stream --
		pa_error = Pa_StartStream(pa_stream);

		if (pa_error == paNoError)
			initialized = true;
		else
		{
			fprintf(stderr, "PortAudio --> Output Stream Not Started @Iteration %i\n", pa_tries);
			fprintf(stderr, "PortAudio --> Error Description : %s\n", Pa_GetErrorText(pa_error));
			continue;
		}
	}

	if (initialized)
		fprintf(stderr, "PortAudio --> Initialization OK : Channels : %i / SampleRate : %i / FrameSize : %i\n", channels, sample_rate, pa_frame_size);
	return initialized;
}


// --- Write ---
PaError PortAudioStream::write(int16_t *samples, int samples_len)
{
	if (initialized)
	{
		Pa_WriteStream(pa_stream, samples, pa_frame_size);

		return paNoError;
	}

	return paNotInitialized;
}


// --- IsActive ---
bool PortAudioStream::isActive()
{
	return Pa_IsStreamActive(&pa_stream);
}


// --- Terminate ---
bool PortAudioStream::terminate()
{
	fprintf(stderr, "PortAudio --> Termination\n");
	Pa_AbortStream(pa_stream);
	Pa_CloseStream(pa_stream);
	Pa_Terminate();

	return true;
}

