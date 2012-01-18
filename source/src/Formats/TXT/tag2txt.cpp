/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* fast transcription to tag converter
*/

#include <iostream>
#include <iterator>
#include <algorithm>
#include <set>
#include <vector>
#include <list>
#include <getopt.h>
#include <ag/AGAPI.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "DataModel/DataModel.h"
#include "Common/util/ExpRegul.h"
#include "Common/util/StringOps.h"
#include "Common/VersionInfo.h"
#include "Common/FileInfo.h"
#include "Common/iso639.h"

using namespace tag;


/**
 * print out help message on program usage and exit
 * @param progname current program name
 */

void USAGE(const char* progname)
{
	Log::err() << "USAGE: " << progname << " [-t on|off] [-h] [-v] <filename>" << endl;
	Log::err() << "options :" << endl;
	Log::err() << "\t-t on|off: print speech segments timecode or not (on by default)" << endl;
	Log::err() << "\t-v : print program version " << endl;
	Log::err() << "\t-h : print this message " << endl;
	exit(1);
}


int main(int argc, char* const argv[])
{
	string format("TransAG");
	string signalFilename ("");
	map<string, string> options;
	map<string, string>::iterator it;
	bool print_timecodes = true;

	const char* progname = argv[0];

	#ifdef WIN32
	DataModel::initEnviron(progname);
	#else
	DataModel::initEnviron("");
	#endif
	Log::setTraceLevel(Log::MAJOR);

	int c;

	while ((c = getopt(argc , argv, "vht:")) != -1) {
		switch (c) {
		case 'v':
			cout << progname << " version " << TRANSAG_VERSION_NO << endl;
			return 0;
		case 't':
		{
			string s = optarg;
			print_timecodes = ! ( s == "0" || s == "off" || s == "no" );
		}
		default: USAGE(progname);
		}
	}


	if ( optind == argc ) USAGE(progname);
	const char* filename = argv[optind];


	try {

		DataModel data("TransAG");
		string guessed = data.guessFileFormat(filename) ;
		if ( format != guessed ) {
			Log::err() << "Input file is not in " << format << " format : guessed=" << guessed << endl;
			return 0;
		}
		data.loadFromFile(filename,format);
		FileInfo info(filename);
		info.setTail("txt");
		data.saveToFile(info.path(), "TXT");
		Log::err() << "Saved file " << info.path() << flush << endl;

	} catch (const char* msg ) {
		Log::err() << " Caught exception : " << msg << endl;
	}

	return 0;
}


