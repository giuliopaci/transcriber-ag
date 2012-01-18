/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// --- TEMP MAIN ---

#include "RTSPSession.h"

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Usage : myOpenRTSP <rtsp_url>\n");
		return 0;
	}

	printf("About to open medium : %s\n", argv[1]);

	RTSPSession *session = new RTSPSession();

	session->setMedium( argv[1] );
	session->initSession();
	session->play();
}

