/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

// ----------------------------
// -   Simple Stream Player   -
// ----------------------------

// - Currently, only audio streams are supported

#include <string>
#include <iostream>

#include "base/Guesser.h"
#include "io/PortAudioStream.h"

using namespace std;

int main(int argc, char** argv)
{
	string usage = "Usage : MMPlayer <url>";

	// -- No arguments --
	if (argc <= 1)
	{
		cout << usage << endl;
		return 1;
	}

	// -- GLib Thread Init --
	g_thread_init(NULL);

	// -- Variables (Decl) --
	IODevice*			device;
	MediumInfo*			info;
	PortAudioStream*	pa;
	MediumFrame*		frame;
	string				url;
	double				current_ts;

	// -- Variables (Init) --
	url			= argv[1];
	current_ts	= 0.0;


	// -- Opening file --
	device = Guesser::open(argv[1]);

	if (!device)
	{
		cout << "Unable to open url : " << url << endl;
		return 1;
	}

	// -- RTSP Handler --
	if (url.find("rtsp://") != string::npos)
	{
		device->m_play();

		while(frame == NULL)
			frame = device->m_read();

		for(int i=0; i<frame->len; i++)
			frame->samples[i] = '0';

		device->m_stop();
	}


	// -- Playback Loop --
	info = device->m_info();

	pa = new PortAudioStream(info->audio_channels, info->audio_sample_rate, info->audio_frame_size);

	device->m_play();

	// -- Infinite Loop (temp) --
	while (true)
	{
		frame = device->m_read();

		if (frame)
		{
			pa->write(frame->samples, frame->len);
			current_ts = frame->ts;
		}
	}

	cout << "Playback finished." << endl;

	return 0;
}

