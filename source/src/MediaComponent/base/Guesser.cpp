/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/


#include "Guesser.h"

// --- Open (Static) ---
// - Always prefer SndFile, to play uncompressed & exotic supported formats
// - Returns an instantiated IODevice (NULL if the medium can't be opened)
IODevice* Guesser::open(const char *in_path, MediumCtx ctx)
{
	char* path = (char*)in_path;

	// -- Local/Remote Detection --
	if ( strstr(path, "rtsp://") != NULL )
	{
		#ifdef GUESSER_TRACE
		printf("Guesser --> RTSP Handler : %s --> PASS\n", path);
		#endif

		RTSPHandler* rtsp = new RTSPHandler();
		rtsp->set_av_ctx(ctx);
		rtsp->m_open(path);

		return rtsp;
	}
	else
	{
		#ifdef GUESSER_TRACE
		printf("Guesser --> RTSP Handler : %s --> FAIL\n", path);
		#endif
	}


	// -- LibSndfile --
	SndFileHandler* sf = new SndFileHandler();
	sf->set_av_ctx(ctx);

	if ( sf->m_open(path) )
	{
		#ifdef GUESSER_TRACE
		printf("Guesser --> Sndfile Handler : %s --> PASS\n", path);
		#endif
		return sf;
	}
	else
	{
		#ifdef GUESSER_TRACE
		printf("Guesser --> Sndfile Handler : %s --> FAIL\n", path);
		#endif
		delete sf;
	}


	// -- FFmpeg --
	FFMpegHandler* ff = new FFMpegHandler();
	ff->set_av_ctx(ctx);
	ff->m_init();

	if ( ff->m_open(path) )
	{
		#ifdef GUESSER_TRACE
		printf("Guesser --> FFMpeg Handler : %s --> PASS\n", path);
		#endif
		return ff;
	}
	else
	{
		#ifdef GUESSER_TRACE
		printf("Guesser --> FFMpeg Handler : %s --> FAIL\n", path);
		#endif
		delete ff;
	}

	// -- No device --
	#ifdef GUESSER_TRACE
	printf("Guesser --> All Handlers Failed --> No device instantiated.\n");
	#endif

	return NULL;
}

// Method for empty ssignal
IODevice* Guesser::open(int nbTracks, double length)
{
	SilentHandler* sh = new SilentHandler(nbTracks, length) ;
	return sh ;
}
