/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "AudioTools.h"


// --- Constructor ---
AudioTools::AudioTools()
{
	audioIn			= NULL;
	monoFrame		= NULL;
	silenceInterval = false;
	trackID			= -1;

	// -- Silence Allocation --
	silenceBuf = new MediumFrame();
	
	silenceBuf->samples = new int16_t[AUDIO_BUFFER_SIZE];
	silenceBuf->len		= AUDIO_BUFFER_SIZE;
	memset(silenceBuf->samples, '0', silenceBuf->len);
}


// --- Destructor ---
AudioTools::~AudioTools()
{
	delete [] silenceBuf;

	if (monoFrame)
		delete [] monoFrame;

	// -- Handlers --
	for( map<int, SndFileHandler*>::iterator it = handlers.begin(); it != handlers.end(); it++ )
	{
		it->second->m_close();

		delete it->second;
	}
}


// --- GetUniqueID ---
int
AudioTools::getUniqueID()
{
	int id = handlers.size();

	while (handlers.find(id) != handlers.end())
		id++;

	return id;
}


// --- OpenExtractFile ---
int
AudioTools::openExtractFile(string filePath, int noTrack)
{
	trackID = noTrack;

	// -- TrackID checkers --
	if ( (trackID < -1) || (trackID > 1) )
	{
		printf("AudioTools --> Incorrect track number : %i\n", trackID);
		return -1;
	}

	// -- Initializing new handler --
	SndFileHandler* audioOut = new SndFileHandler();

	if (trackID == -1)
	{
		audioOut->m_set_sfinfo( audioIn->m_info()->audio_channels,
								audioIn->m_info()->audio_sample_rate,
								SF_FORMAT_WAV | SF_FORMAT_PCM_16);
	}
	else
	{
		audioOut->m_set_sfinfo( 1,
								audioIn->m_info()->audio_sample_rate,
								SF_FORMAT_WAV | SF_FORMAT_PCM_16);
	}

	if ( audioOut->m_open(filePath.c_str(), SFM_WRITE) )
	{
		// -- Adding new entry --
		handlers[ getUniqueID() ] = audioOut;
		return handlers.size() - 1;
	}
	else
	{
		printf("AudioTools --> Unable to open output file : %s\n", filePath.c_str());
		delete audioOut;
		return -1;
	}
}


// --- CloseExtractFile ---
bool
AudioTools::closeExtractFile(int fileID)
{
	SndFileHandler* audioOut = handlers[fileID];

	if (!audioOut)
		return false;

	audioOut->m_close();

	// -- Map removal --
	map<int, SndFileHandler*>::iterator it = handlers.find(fileID);
	
	if (it != handlers.end())
		handlers.erase(it);

	delete audioOut;

	return true;
}


// --- AddSegment ---
bool
AudioTools::addSegment(int fileID, float tsStart, float tsEnd)
{
	// -- Retrieving Handler --
	SndFileHandler* audioOut = handlers[fileID];

	if (!audioOut)
	{
		printf("AudioTools --> Output device is uninitialized!\n");
		return false;
	}

	if (!audioIn)
	{
		printf("AudioTools --> Input device is uninitialized!\n");
		return false;
	}

	// -- Inits --
	float	tsLoop	= tsStart;
	float	tsWatch	= 0.0;
	float	overLen	= 0.0;

	// -- Offsets Check --
	if (tsStart < 0.0)
		tsStart = 0.0;

	if (tsEnd > audioIn->m_info()->audio_duration)
		tsEnd = audioIn->m_info()->audio_duration;

	if (tsEnd <= tsStart)
	{
		printf("AudioTools --> End offset must be greater than start offset\n");
		return false;
	}


	// -- Extraction Loop --
	audioIn->m_seek(tsLoop);

	while (tsLoop < tsEnd)
	{
		MediumFrame* frame = audioIn->m_read();

		if (!frame)
			break;

		tsWatch	+= frame->ts - tsLoop;
		tsLoop	= frame->ts;

		// -- Last Buffer Adjustment --
		if (tsLoop > tsEnd)
		{
			overLen = (tsLoop - tsEnd) * (float)audioIn->m_info()->audio_sample_rate * (float)audioIn->m_info()->audio_channels;

			if ( (overLen - floor(overLen) > 0.5) )
				frame->len -= ceil(overLen);
			else
				frame->len -= floor(overLen);
		}

		if (trackID == -1)
		{
			audioOut->m_write(frame);
		}
		else
		{
			int frameLen = frame->len / audioIn->m_info()->audio_channels;

			// -- Mono frame (output) --
			if (!monoFrame)
			{
				monoFrame = new MediumFrame();

				monoFrame->samples	= new int16_t[frameLen];
				monoFrame->len		= frameLen;
				memset(monoFrame->samples, '0', monoFrame->len);
			}

			for(int i=0; i < frameLen; i++)
				monoFrame->samples[i] = frame->samples[ i*audioIn->m_info()->audio_channels + trackID ];

			audioOut->m_write(monoFrame);
		}
	}


	// -- Silence interval (if enabled) --
	if (silenceInterval)
		audioOut->m_write(silenceBuf);

	return true;
}

