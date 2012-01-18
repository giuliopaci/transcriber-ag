/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#define MEDIACOMPONENT_STANDALONE 1
#define GUESSER_TRACE 1

#ifdef WIN32
	#include <getopt.h>
#endif

#include <string>

#include "MediaComponent/tools/AudioTools.h"

char*		input	= NULL;
char*		output	= NULL;
int			fileID;

IODevice*	device = NULL;
AudioTools	aTool;

// -- Prototypes --
bool audio_cut(char*);


// --- Main ---
int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("Usage : audioTools <input_file> <output_file> [options]\n");
		return 1;
	}

	input	= argv[1];
	output	= argv[2];

	// -- Glib Init --
	g_thread_init(NULL);

	int option;
	int rc;

	// -- Options Parser --
	while( (option = getopt(argc , argv, "c:")) != -1 )
	{
		switch (option)
		{
			case 'c':
				rc = audio_cut(optarg);
				break;

			default:
				break;
		}
	}

	return rc;
}


// --- Audio::Cut ---
bool audio_cut(char* inArg)
{
	double t_start, t_end;
	
	string args = inArg;
	int pos = args.find_first_of(":");

	if (pos != string::npos)
	{
		t_start = atof( args.substr(0, pos).c_str() );
		t_end	= atof( args.substr(pos + 1).c_str() );
	}
	else
	{
		printf("Audio::Cut --> Wrong arguments!\n");
		return false;
	}

	device = Guesser::open(input);

	if (!device)
		return false;

	// -- Offset check --
	if (t_end <= t_start)
		t_end = device->m_info()->audio_duration;


	printf("Audio::Cut --> Clip [%f, %f]\n", t_start, t_end);

	aTool.setInputDevice(device);

	fileID = aTool.openExtractFile(output);

	aTool.addSegment(fileID, t_start, t_end);
	aTool.closeExtractFile(fileID);	

	device->m_close();

	return true;
}

