/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#define MEDIACOMPONENT_STANDALONE 1
#define GUESSER_TRACE 1

#include "MediaComponent/video/Thumbnailer.h"

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("Usage : ThumbnailerTest <filename>\n");
		return 1;
	}

	g_thread_init(NULL);

	// -- Output --
	Thumbnailer* thumbnailer = new Thumbnailer(argv[1], 60, 20, 5);

	// -- Caching --
	thumbnailer->process();
	thumbnailer->joinThread();

	// -- Loading --
	thumbnailer->process();
	thumbnailer->joinThread();

	return 0;
}

