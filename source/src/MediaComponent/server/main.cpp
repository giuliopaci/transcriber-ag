/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include <BasicUsageEnvironment.hh>
#include "RTSPOverHTTPServer.hh"
#include "MultimediaRTSPServer.h"

#include <iostream>
#include <getopt.h>

using namespace std;

// ------------
// -   MAIN   -
// ------------
int main(int argc, char** argv)
{
	// --- Options ---
	int		option;
	string	document_root;
	int		custom_port = -1;

	while( (option = getopt(argc , argv, "d:p:h")) != -1 )
	{
		switch (option)
		{
			case 'd':
				document_root = optarg;
				break;

			case 'p':
				custom_port = atoi(optarg);
				break;

			case 'h':
				cout << "Usage : " << argv[0] << " [options]" << endl;
				cout << "Options are : " << endl;
				cout << "  -h : This help" << endl;
				cout << "  -d : Defines a custom document root for RTSP server" << endl;
				cout << "  -p : Defines a custom server port, in range [1..60000]" << endl;
				return 0;

			default:
				break;
		}
	}


	// -------------------
	// -   RTSP Server   -
	// -------------------
	TaskScheduler*				scheduler	= BasicTaskScheduler::createNew();
	UsageEnvironment*			env			= BasicUsageEnvironment::createNew(*scheduler);
	UserAuthenticationDatabase* auth_db		= NULL;

	// -- Authentication Mode --
#ifdef ACCESS_CONTROL
	auth_db = new UserAuthenticationDatabase;
	auth_db->addUserRecord("admin", "password");
#endif

	// -- New server instance --
	MultimediaRTSPServer* rtsp_server;
	portNumBits rtsp_server_port = RTSP_DEFAULT_PORT;

	if (custom_port > 0)
		rtsp_server_port = custom_port;

	rtsp_server = MultimediaRTSPServer::createNew(*env, rtsp_server_port, auth_db);

	// -- Console Header --
	*env << "\n";
	*env << "======================\n";
	*env << "Multimedia RTSP Server\n";
	*env << "======================\n";

	// -- On failure, trying next value --
	if (rtsp_server == NULL)
	{
		*env << "- Unable to bind server to port " << rtsp_server_port << "\n";
		rtsp_server_port++;
		*env << "- Trying the very next one : " << rtsp_server_port << "\n";

		rtsp_server	= MultimediaRTSPServer::createNew(*env, rtsp_server_port, auth_db);
	}

	if (rtsp_server == NULL)
	{
		*env << "- [EXIT] Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}

	*env << "- RTSP server bound to port " << rtsp_server_port << "\n";

	// -- Custom Lines --
	char*	url_prefix = rtsp_server->rtspURLPrefix();

	*env << "- 2009 (c) Bertin Technologies\n";
	*env << "- Based on Live555 Streaming Library v. " << LIVEMEDIA_LIBRARY_VERSION_STRING << "\n";

	// -- DocumentRoot --
	if (!document_root.empty())
	{
		rtsp_server->setDocumentRoot(document_root);
		*env << "- DocumentRoot set to : " << document_root.c_str() << "\n";
	}

	*env << "- Server is reachable at root URL : " << url_prefix << "\n\n";

	// -- Starting Event Loop --
	env->taskScheduler().doEventLoop();

	return 0;
}

