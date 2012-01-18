/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef MULTIMEDIA_RTSP_SERVER_H
#define MULTIMEDIA_RTSP_SERVER_H

#ifndef _RTSP_SERVER_HH
#include "RTSPServer.hh"
#endif

#include <string>

using namespace std;

// -- RTSP Default Settings --
#define RTSP_ALIVE_TIME		1200
#define RTSP_DEFAULT_PORT	6666

/**
 * @class	MultimediaRTSPServer
 * @ingroup	MediaComponent
 *
 * RTSP Server based on live555 libraries\n
 * This class is a simple RTSP server, listening to default port 6666\n
 * All compatible media located in the server directory are directly streamable\n
 * Url Example : rtsp://server:6666/example.mp3
 */
class MultimediaRTSPServer : public RTSPServer
{
public:
	/**
	 * Static method providing a newly instantiated server instance, with parameters
	 * @param env	Current environment
	 * @param port	Listening port
	 * @param db	Optional authentication database (can be NULL)
	 */
	static	MultimediaRTSPServer* createNew(UsageEnvironment& env, Port port, UserAuthenticationDatabase* db);

	/**
	 * Sets a new prefix path
	 * @param prefix	New prefix path
	 */
	void	setDocumentRoot(const string& prefix)	{ document_root = prefix; }

private:
	MultimediaRTSPServer(UsageEnvironment&, int, Port, UserAuthenticationDatabase*);

	virtual	~MultimediaRTSPServer();
	virtual	ServerMediaSession* lookupServerMediaSession(char const*);

	string	document_root;
};

#endif

