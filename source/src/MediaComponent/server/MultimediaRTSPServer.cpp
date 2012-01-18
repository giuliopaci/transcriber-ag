/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "MultimediaRTSPServer.h"
#include <liveMedia.hh>

// --- CreateNew (Static) ---
MultimediaRTSPServer*
MultimediaRTSPServer::createNew(UsageEnvironment&			rtsp_env,
								Port						rtsp_port,
								UserAuthenticationDatabase* rtsp_database)
{
	int rtsp_socket = -1;

	do
	{
		rtsp_socket = setUpOurSocket(rtsp_env, rtsp_port);

		if (rtsp_socket == -1)
			break;

		return new MultimediaRTSPServer(rtsp_env, rtsp_socket, rtsp_port, rtsp_database);
	}
	while(0);

	if (rtsp_socket != -1)
		::closeSocket(rtsp_socket);

	return NULL;
}


// --- MultimediaRTSPServer ---
MultimediaRTSPServer::MultimediaRTSPServer( UsageEnvironment& 			rtsp_env,
											int							rtsp_socket,
											Port						rtsp_port,
											UserAuthenticationDatabase* rtsp_database)
	: RTSPServer(rtsp_env, rtsp_socket, rtsp_port, rtsp_database, RTSP_ALIVE_TIME)
{
	// -- Default path (UNIX) --
	document_root = "/";
}


// --- ~MultimediaRTSPServer ---
MultimediaRTSPServer::~MultimediaRTSPServer()
{}


// --- CreateNewSMS (Static forward) ---
static ServerMediaSession* createNewSMS(UsageEnvironment&	rtsp_env,
										char const*			file_name,
										char const*			local_url,
										FILE*				fh);


// --- LookupServerMediaSession ---
ServerMediaSession*
MultimediaRTSPServer::lookupServerMediaSession(char const* streamName)
{
	string	url = streamName;
	int		idx = 0;

	// -- URL processing --
	while( (idx = url.find_first_of(":")) != string::npos )
		url = url.replace(idx, 1, "/");

	url = document_root + "/" + url;

	// -- Local file check --
	FILE* fid = fopen(url.c_str(), "rb");
	Boolean fileExists = fid != NULL;

	// -- Existing server media session(SMS) ? --
	ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName);
	Boolean smsExists = sms != NULL;

	// -- SMS handler --
	if (!fileExists)
	{
		if (smsExists)
		{
			printf("- MultimediaRTSPServer : Session removal for file -> %s\n", url.c_str());

			// -- Removing old SMS --
			removeServerMediaSession(sms);
		}
		return NULL;
	}
	else
	{
		if (!smsExists)
		{
			// -- Creating new SMS --
			sms = createNewSMS(envir(), streamName, url.c_str(), fid);
			addServerMediaSession(sms);
		}

		fclose(fid);
		return sms;
	}
}


// --- Macro : NEW_SMS ---
#define NEW_SMS(description) do {\
char* descStr = (char*)description\
    ", streamed by MultimediaRTSPServer";\
sms = ServerMediaSession::createNew(env, fileName, fileName, descStr);\
} while(0)


// --- CreateNewSMS (Implementation) ---
static ServerMediaSession* createNewSMS(UsageEnvironment& env,
										char const* fileName,
										char const* localFile,
										FILE*)
{
	// -- Extension Check --
	char const* extension = strrchr(fileName, '.');

	if (extension == NULL)
		return NULL;

	// -- Inits --
	ServerMediaSession* sms		= NULL;
	Boolean const reuseSource	= False;


	// -- Subsession Search --
	if (strcmp(extension, ".aac") == 0)
	{
		// -- AAC Audio (ADTS Format) --
		NEW_SMS("AAC Audio");
		sms->addSubsession(ADTSAudioFileServerMediaSubsession::createNew(env, localFile, reuseSource));
	}
	else
	if (strcmp(extension, ".amr") == 0)
	{
		// -- AMR Audio --
		NEW_SMS("AMR Audio");
		sms->addSubsession(AMRAudioFileServerMediaSubsession::createNew(env, localFile, reuseSource));
	}
	else
	if (strcmp(extension, ".m4e") == 0)
	{
		// -- MPEG-4 Video Elementary Stream --
		NEW_SMS("MPEG-4 Video");
		sms->addSubsession(MPEG4VideoFileServerMediaSubsession::createNew(env, localFile, reuseSource));
	}
	else
	if (strcmp(extension, ".mp3") == 0)
	{
		// -- MPEG-1 Or 2 Audio --
		NEW_SMS("MPEG-1 or 2 Audio");

		Boolean useADUs = False;
		Interleaving* interleaving = NULL;

		// --------------------
		// -   ADUs SECTION   -
		// --------------------

		// -- Uncomment for ADUs --
		//#define STREAM_USING_ADUS 1

		// -- Uncomment for ADUs reorder (before streaming) --
		//#define INTERLEAVE_ADUS 1

		#ifdef STREAM_USING_ADUS
		useADUs = True;
			#ifdef INTERLEAVE_ADUS
				unsigned char	interleaveCycle[] = {0,2,1,3}; // or choose your own...
				unsigned const	interleaveCycleSize = (sizeof interleaveCycle) / (sizeof (unsigned char));
				interleaving = new Interleaving(interleaveCycleSize, interleaveCycle);
			#endif
		#endif

		// --------------------------
		// -    END ADUs SECTION    -
		// --------------------------
		sms->addSubsession(MP3AudioFileServerMediaSubsession::createNew(env, localFile, reuseSource, useADUs, interleaving));
	}
	else
	if (strcmp(extension, ".mpg") == 0)
	{
		// -- MPEG-1 Or 2 Program Stream --
		NEW_SMS("MPEG-1 or 2 Program Stream");
		MPEG1or2FileServerDemux* demux = MPEG1or2FileServerDemux::createNew(env, localFile, reuseSource);

		sms->addSubsession(demux->newVideoServerMediaSubsession());
		sms->addSubsession(demux->newAudioServerMediaSubsession());
	}
	else
	if (strcmp(extension, ".ts") == 0)
	{
		// -- MPEG Transport Stream --
		NEW_SMS("MPEG Transport Stream");

		unsigned	indexFileNameLen	= strlen(fileName) + 2;
		char*		indexFileName		= new char[indexFileNameLen];
		sprintf(indexFileName, "%sx", fileName);

		sms->addSubsession(MPEG2TransportFileServerMediaSubsession::createNew(env, localFile, indexFileName, reuseSource));
		delete[] indexFileName;
	}
	else
	if (strcmp(extension, ".wav") == 0)
	{
		// -- WAV Audio Stream --
		NEW_SMS("WAV Audio Stream");

		// -- U-law conversion ? --
		Boolean convertToULaw = False;
		sms->addSubsession(WAVAudioFileServerMediaSubsession::createNew(env, localFile, reuseSource, convertToULaw));
	}

	if (sms == NULL)
		printf("- MultimediaRTSPServer : Unable to stream file -> %s\n", localFile);
	else
		printf("- MultimediaRTSPServer : Streaming file -> %s\n", localFile);

	return sms;
}

