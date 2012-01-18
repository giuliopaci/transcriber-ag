/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "RTSPSession.h"
#include "BasicUsageEnvironment.hh"
#include <iostream>

using namespace std;

// --- Callbacks ---
void	subsessionAfterPlaying(void* clientData);

// --- RTSPSession ---
RTSPSession::RTSPSession()
{
	offset		= 0.0;
	event		= 0;
	playing		= false;
}


// --- ~RTSPSession ---
RTSPSession::~RTSPSession()
{}


// --- InitSession ---
bool RTSPSession::initSession()
{
	// -- Env --
	scheduler	= BasicTaskScheduler::createNew();
	env			= BasicUsageEnvironment::createNew(*scheduler);
  	client		= RTSPClient::createNew(*env, 0, "TranscriberAG", 0);
	sink		= BufferSink::createNew(*env);

	// -- SDP Retrieval --
	char *sdp	= client->describeURL(url);
	session		= MediaSession::createNew(*env, sdp);

	//cout << "SDP : " << sdp << endl;

	delete[] sdp;

	if (session == NULL)
	{
		cout << "Failed to create a MediaSession object from the SDP description: " << env->getResultMsg() << endl;
		shutdownSession();
		return false;
	}
	else
	if (!session->hasSubsessions())
	{
		cout << "This session has no media subsessions (i.e., \"m=\" lines)" << endl;
		shutdownSession();
		return false;
	}
	else
	{
		initSubsessions();
		setupStreams();
		return true;
	}
}


// --- InitSubsessions ---
void RTSPSession::initSubsessions()
{
	MediaSubsessionIterator iter(*session);
	MediaSubsession*		subsession;

	int	desiredPortNum	= 32768;

	while( (subsession = iter.next()) != NULL )
	{
		char* singleMedium	= NULL;

		// -- Single Medium ? --
		if (singleMedium != NULL)
		{
			if ( strcmp(subsession->mediumName(), singleMedium) != 0 )
			{
				cout << "Ignoring " << subsession->mediumName()
					 << "/" << subsession->codecName()
					 << "subsession, because we've asked to receive a single "
					 << singleMedium << " session only"
					 << endl;
				continue;
			}
			else
			{
				// Receive this subsession only
				singleMedium = (char*)"xxxxx";
			}
		}

		// -- Subsession Initiate --
		if ( !subsession->initiate(-1) )
		{
			cout << "Unable to create receiver for " << subsession->mediumName()
				 << "/" << subsession->codecName()
				 << "\" subsession: " << env->getResultMsg() << endl;
		}
		else
		{
			cout << "Created receiver for " << subsession->mediumName()
				 << "/" << subsession->codecName()
				 << "\" subsession (client ports " << subsession->clientPortNum()
				 << "-" << subsession->clientPortNum()+1 << ")" << endl;
		}

		// -- Some Packet Reorder --
		if (subsession->rtpSource() != NULL)
		{
			printf("Setting Packet Reordering Threshold to 1s\n");
			subsession->rtpSource()->setPacketReorderingThresholdTime(1000000);
		}

		// -- Subsession Port Number --
		if (desiredPortNum != 0)
		{
			subsession->setClientPortNum(desiredPortNum);
			desiredPortNum += 2;
		}
	}
}


// --- SetupStreams ---
void RTSPSession::setupStreams()
{
	MediaSubsessionIterator iter(*session);
	MediaSubsession*		subsession;

	while ( (subsession = iter.next()) != NULL )
	{
		if (subsession->clientPortNum() == 0)
		{
			cout << "No client port set!" << endl;
			continue; // port # was not set
		}

  		if ( !client->setupMediaSubsession(*subsession, False, False) )
		{
			cout << "Failed to setup " << subsession->mediumName()
				 << "/" << subsession->codecName()
				 << "\" subsession: " << env->getResultMsg() << endl;
		}
		else
		{
			cout << "Setup " << subsession->mediumName()
				 << "/" << subsession->codecName()
				 << " subsession (client ports " << subsession->clientPortNum()
				 << "-" << subsession->clientPortNum()+1 << ")" << endl;

			sink->setAudioSettings( subsession->rtpPayloadFormat(),
									(char*)subsession->codecName(),
									subsession->numChannels(),
									subsession->rtpTimestampFrequency(),
									session->playEndTime());

			subsession->sink = sink;
		}
	}
}


// --- TearDownStreams ---
void RTSPSession::tearDownStreams()
{
	if (client == NULL || session == NULL)
		return;

	client->teardownMediaSession(*session);
}


// --- Shutdown ---
void RTSPSession::shutdownSession()
{
	// -- Closing Sinks --
	closeMediaSinks();

	// -- Closing streams --
	tearDownStreams();

	Medium::close(session);
	Medium::close(client);
}


// --- Seek ---
void RTSPSession::seek(double off)
{
	offset = off;
}



// --- Play (Threaded) ---
bool RTSPSession::play()
{
	event	= 0;
	playing	= true;

	// -- Starting sink(s) --
	sink->setBuffering(true);

	MediaSubsessionIterator iter(*session);
	MediaSubsession*		subsession;

	while ( (subsession = iter.next()) != NULL )
	{
		if (subsession->readSource() == NULL)
		{
			printf("Subsession Source unitialized!\n");
			continue;
		}

		subsession->sink->startPlaying(*(subsession->readSource()),
										subsessionAfterPlaying,
										subsession);
	}

	extern double initialSeekTime, duration, scale;

	// -- Starting Client --
	if ( !client->playMediaSession(*session, offset, -1, 1.0) )	// seek, duration, scale
		cout << "RTSPSession --> Unable to start playback..." << endl;

	// -- Event Loop (ruled by 'event' variable) --
	env->taskScheduler().doEventLoop(&event);

	stop();
}


// --- Stop ---
bool RTSPSession::stop()
{
	playing = false;

	// -- Stopping Client --
	if ( !client->pauseMediaSession(*session) )
		cout << "RTSPSession -> Unable to pause" << endl;

	// -- Stopping Sink(s) --
	MediaSubsessionIterator iter(*session);
	MediaSubsession*		subsession;

	while( (subsession = iter.next()) != NULL )
	{
		if (subsession->readSource() == NULL)
		{
			printf("Subsession Source Unitialized!\n");
			continue;
		}

		subsession->sink->stopPlaying();
		((BufferSink*)subsession->sink)->flushQueue();
		((BufferSink*)subsession->sink)->setBuffering(false);
	}
}


// --- CloseMediaSinks ---
void RTSPSession::closeMediaSinks()
{
	if (session == NULL)
		return;

	MediaSubsessionIterator iter(*session);
	MediaSubsession*		subsession;

	while( (subsession = iter.next()) != NULL )
	{
		Medium::close(subsession->sink);
	}
}


// --- SubsessionAfterPlaying ---
void subsessionAfterPlaying(void* clientData)
{
	cout << "RTSPSession --> Callback : Subsession after playing" << endl;

	// Begin by closing this media subsession's stream:
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	// Next, check whether *all* subsessions' streams have now been closed:
	MediaSession& session = subsession->parentSession();
	MediaSubsessionIterator iter(session);

	while ((subsession = iter.next()) != NULL)
	{
		if (subsession->sink != NULL)
			return;
	}
}

