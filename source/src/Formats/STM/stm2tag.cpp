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
#include "Common/VersionInfo.h"
#include "Common/FileInfo.h"
#include "Common/util/StringOps.h"
#include "Common/iso639.h"

using namespace tag;


/**
 * print out help message on program usage and exit
 * @param progname current program name
 */

void USAGE(const char* progname)
{
	Log::err() << "USAGE: " << progname << " [-h] [-v] [-l <lang>] [-c <conv>] <filename>" << endl;
	Log::err() << "options :" << endl;
	Log::err() << "\t-c <conv> : applicable conventions for transcription conversion" << endl;
	Log::err() << "\t-l <lang> : iso639-2 language code for transcription (overrides any language information found in file)" << endl;
	Log::err() << "\t-v : print program version " << endl;
	Log::err() << "\t-h : print this message " << endl;
	exit(1);
}


int main(int argc, char* const argv[])
{
	string format("STM");
	string signalFilename ("");
	map<string, string> options;
	map<string, string>::iterator it;

	const char* progname = argv[0];

	#ifdef WIN32
	DataModel::initEnviron(progname);
	#else
	DataModel::initEnviron("");
	#endif
	Log::setTraceLevel(Log::MAJOR);

	int c;

	while ((c = getopt(argc , argv, "c:l:vh")) != -1) {
		switch (c) {
		case 'l':
			options["lang"]=optarg;
			break;
		case 'c':
			options["conventions"]=optarg;
			break;
		case 'v':
			cout << progname << " version " << TRANSAG_VERSION_NO << endl;
			return 0;
		default: USAGE(progname);
		}
	}


	if ( optind == argc ) USAGE(progname);
	const char* filename = argv[optind];


	try {

		DataModel data("TransAG");
		if ( format != data.guessFileFormat(filename) ) {
			Log::err() << "Input file is not in STM format" << endl;
			return 0;
		}
		for ( it=options.begin(); it != options.end(); ++it ) {
			if ( ! it->second.empty() )
				data.addAGOption(it->first, it->second);
		}
		data.loadFromFile(filename,format);
		FileInfo info(filename);
		info.setTail("tag");
		data.saveToFile(info.path(), "TransAG");

	} catch (const char* msg ) {
		Log::err() << " Caught exception : " << msg << endl;
	}

	return 0;
}


