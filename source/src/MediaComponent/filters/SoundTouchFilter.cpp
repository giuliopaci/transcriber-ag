/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "SoundTouchFilter.h"
#include "base/Guesser.h"
#include <iostream>

// --- SoundTouchFilter ---
SoundTouchFilter::SoundTouchFilter()
{
	enabled		= true;
	stream_end	= false;
	f_available	= false;
	f_offset	= 0.0;
	f_silence	= 0.0;
	f_tempo		= 1.0;
	f_pitch		= 1.0;
	f_rate		= 1.0;
	f_ts		= 0.0;
	st_input	= NULL;
	st_output	= NULL;
	st_recv		= NULL;
}


// --- ~SoundTouchFilter ---
SoundTouchFilter::~SoundTouchFilter()
{
	if (device)
	{
		device->m_close();
	
		delete device;
		delete st;
	}
	
	// -- Sound buffers --
	if (st_input)
	{
		delete[] st_input;
		delete[] st_output;
		delete[] st_recv;
	}
}


// --- Init ---
void SoundTouchFilter::init(MediumInfo *info)
{
	// -- Initializing --
	device		= Guesser::open(medium_path);

	if (!device)
		return;

	fSize		= info->audio_frame_size;
	nChannels	= info->audio_channels;
	nSamples	= fSize * nChannels;
	f_step		= (float)fSize / (float)info->audio_sample_rate;
	st			= new SoundTouch();
	st_size		= 0;
	st_input	= new SAMPLETYPE[nSamples];
	st_output	= new SAMPLETYPE[nSamples];
	st_recv		= new SAMPLETYPE[nSamples];

	// -- SoundTouch Processor Settings --
	st->setSampleRate(info->audio_sample_rate);
	st->setChannels(nChannels);
	st->setTempo(1.0);
	st->setPitch(1.0);
	st->setRate(1.0);

	st->setSetting(SETTING_SEQUENCE_MS,		40);
	st->setSetting(SETTING_SEEKWINDOW_MS,	10);
	st->setSetting(SETTING_OVERLAP_MS,		12);
}


// --- Filter ---
void SoundTouchFilter::filter(MediumFrame *&f)
{
	if (!enabled || !f)
		return;

	// -- Silencing --
	if (f_silence > 0.0)
	{
		for(int i=0; i<fSize; i++)
			f->samples[nChannels*i + channelID] = 0.0;

		f_silence -= f_step;
		return;
	}
	else
	{
		f_silence = 0.0;
	}

	// -- 1 -> Initializations --
	st_size			= 0;
	samplesReceived	= 0;

	// -- 2 -> Retrieving Processed Samples --
	while (st_size < fSize)
	{
		samplesReceived = st->receiveSamples(st_recv, fSize - st_size);

		if (samplesReceived == 0)
		{
			if (!feed())
			{
				int tChannels = nChannels;
					
				if (nChannels != fChannels)
					tChannels = fChannels;
				
				// -- Offset mode (wait for the other track) --
				for(int i=0; i<fSize; i++)
					f->samples[tChannels*i + channelID] = 0;
				
				return;
			}
		}
		else
		{
			for(int i=0; i<samplesReceived; i++)
				for(int j=0; j<nChannels; j++)
					st_output[nChannels *(st_size + i) + j] = st_recv[nChannels*i + j];

			st_size += samplesReceived;
		}
	}

	if (nChannels != fChannels)
	{
		f->len = fSize * fChannels;

		for(int i=0; i<fSize; i++)
			if (!stream_end)
				f->samples[fChannels*i + channelID] = st_output[nChannels*i];
			else
				f->samples[fChannels*i + channelID] = 0;
	}
	else
	{
		for(int i=0; i<fSize; i++)
			if (!stream_end)
				f->samples[nChannels*i + channelID] = st_output[nChannels*i + channelID];
			else
				f->samples[nChannels*i + channelID] = 0;
	}

	// -- Applying audio timestamp --
    f->ts = f_ts + f_offset;
}


// --- Feed ---
// -- 1 -> Demuxes the input samples for a single channel
// -- 2 -> Feeds the SoundTouch processor
bool SoundTouchFilter::feed()
{
	MediumFrame* inFrame = device->m_read();

	if (!f_available)
	{	
		while(inFrame == NULL)
			inFrame = device->m_read();
	
		f_available = true;
	}
	else
	{
		if (inFrame == NULL)
			return false;
	}

	f_ts = inFrame->ts;

	for(int i=0; i<nSamples; i++)
		st_input[i] = inFrame->samples[i];

	st->putSamples(st_input, fSize);

	return true;
}


// --- SamplesAvailable ---
int SoundTouchFilter::samplesAvailable()
{
	return st->numSamples();
}


// --- Play ---
void SoundTouchFilter::play()
{
	device->m_play();
}


// --- Stop ---
void SoundTouchFilter::stop()
{
	device->m_stop();
	f_available = false;
}


// --- Seek ---
void SoundTouchFilter::seek(float p_seek)
{
	f_available	= false;
	stream_end	= false;

	if (p_seek - f_offset >= 0.0)
	{
		f_silence	= 0.0;

		// -- End of stream ? --
		if (p_seek - f_offset > device->m_info()->audio_duration)
		{
			p_seek = device->m_info()->audio_duration;
			f_available	= true;
			stream_end	= true;
		}
		else
			p_seek -= f_offset;

		device->m_seek(p_seek);
	}
	else
	{
		// -- We have to play 'silence' to initialize offset correctly --
		f_silence = -(p_seek - f_offset);
		device->m_seek(0.0);
	}
	st->clear();
}


// --- SetTempo ---
void SoundTouchFilter::setTempo(float in_tempo)
{
	f_tempo = in_tempo;

	st->setTempo(f_tempo);
}


// --- SetPitch ---
void SoundTouchFilter::setPitch(float in_pitch)
{
	f_pitch = in_pitch;

	st->setPitch(f_pitch);
}


// --- SetTempo ---
void SoundTouchFilter::setRate(float in_rate)
{
	f_rate = in_rate;

	st->setRate(f_rate);
}

