/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

// -> Standalone file that provides executable 'MediumGuesser'
// -> Returns a string containing format informations extracted from input (local) media
// -> Informations	: Codec / Frequency / Sample Resolution / Number of Channels / Duration
// -> String form	: format=WAV;frequency=44100;sample_res=16;channels=2;duration=443.00

#include "MediaComponent/base/Types.h"
#include "MediaComponent/base/Guesser.h"
#include "Common/util/Log.h"
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>

using namespace std;

bool extractInfo(string);


// -------------------
// --- ExtractInfo ---
// -------------------
bool extractInfo(char *path, string &info)
{
	IODevice *io = Guesser::open(path);

	if (io == NULL)
	{
		Log::err() << "IODevice::open_failure\n" << std::endl ;
		return false;
	}

	// -- Building output string --
	ostringstream oss;

	oss << "format="		<< io->m_info()->audio_codec << ";";
	oss << "samplerate="	<< io->m_info()->audio_sample_rate << ";";
	oss << "sampleres="		<< io->m_info()->audio_sample_resolution << ";";
	oss << "channels="		<< io->m_info()->audio_channels << ";";
	oss << "duration="		<< io->m_info()->audio_duration;

	info = oss.str();

	return true;
}


// --------------------------
// --- MediumGuesser Main ---
// --------------------------
int main(int argc, char**argv)
{
	// -- Args Check --
	if (argc <= 1)
	{
		cerr << "No input specified!" << endl;
		cerr << "Usage : MediumGuesser <input_file>" << endl;
		return 0;
	}

	string info;

	// -- Glib init --
	g_thread_init(NULL);

	if ( extractInfo(argv[1], info) )
	{
		cout << info << endl;
		return 0;
	}
	else
	{
		cerr << "Unable to parse input file!" << endl;
		return 1;
	}
}

