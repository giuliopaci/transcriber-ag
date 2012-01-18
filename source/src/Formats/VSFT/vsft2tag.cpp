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
#include "MediaComponent/base/Guesser.h"

using namespace tag;


/**
 * print out help message on program usage and exit
 * @param progname current program name
 */

void USAGE(const char* progname)
{
	Log::err() << "USAGE: " << progname << " [-h] [-v] [-d <time>] [-f <time>] [-t <time>] [-l <lang>] [-c <conv>] [-PI] [-s audiofilename] <filename>" << endl;
	Log::err() << "options :" << endl;
	Log::err() << "\t-d <time> : transcripted file duration" << endl;
	Log::err() << "\t-f <time> : transcription start time'"  << endl;
	Log::err() << "\t-t <time> : transcription end time " << endl;
	Log::err() << "\t-c <conv> : applicable conventions for transcription conversion" << endl;
	Log::err() << "\t-l <lang> : iso639-2 language code for transcription (overrides any language information found in file)" << endl;
	Log::err() << "\t-PI       : \"&\" turns converted as [pron=pi] annot" << endl;
	Log::err() << "\t-v : print program version " << endl;
	Log::err() << "\t-h : print this message " << endl;
	Log::err() << "Where <time> can be noted in one of the following forms : " << endl;
	Log::err() << "\twithout millisecond precision : 'hh:mm:ss' 'mm:ss' 'sss'" << endl;
	Log::err() << "\twith millisecond precision : 'hh:mm:ss.ddd' 'mm:ss.ddd' 'sss.ddd'" << endl;
	exit(1);
}


int main(int argc, char* const argv[])
{
	string format("VSFT");
	string signalFilename ("");
	map<string, string> options;
	map<string, string>::iterator it;
	bool ok_duration = false;

	const char* progname = argv[0];

	#ifdef WIN32
	DataModel::initEnviron(progname);
	#else
	DataModel::initEnviron("");
	#endif
	
	Log::setTraceLevel(Log::MAJOR);

	int c;

	while ((c = getopt(argc , argv, "c:d:f:l:t:vhP:s:")) != -1) {
		switch (c) {
		case 'd':
			options["duration"]=optarg;
			ok_duration=true;
			break;
		case 'f':
			options["startTime"]=optarg;
			break;
		case 't':
			options["endTime"]=optarg;
			break;
		case 'l':
			options["lang"]=optarg;
			break;
		case 'c':
			options["conventions"]=optarg;
			break;
		case 's' :
			signalFilename = optarg;
			break;
		case 'P':
			if ( strcmp(optarg, "I") == 0 )
				options["with_pi"] = "1";
			else  USAGE(progname);
			break;
		case 'v':
			cout << progname << " version " << TRANSAG_VERSION_NO << endl;
			return 0;
		default: USAGE(progname);
		}
	}


	if ( optind == argc ) USAGE(progname);
	const char* filename = argv[optind];

	options["signalFormat"]="WAV";
	options["signalNbTracks"]= "1";

	if ( !ok_duration ) {
		 // guessing time from audio file
        if ( signalFilename.empty() ) {
            FileInfo info(filename);
            info.setTail("wav");
            if ( !info.exists() )   info.setTail("WAV");
            if ( info.exists() )
                signalFilename = info.path();
        } else {
            if ( ! FileInfo(signalFilename).exists() ) {
                Log::err() << "Audio file not found : " << signalFilename << endl;
                exit(1);
            }
        }
        if ( ! signalFilename.empty() ) {
            IODevice* device = Guesser::open((char*)(signalFilename.c_str())) ;
            float duration = 0 ;
            if (device)
            {
                float duration = device->m_info()->audio_duration ;
                options["duration"] = StringOps().fromFloat(duration);
                options["signalEncoding"] = device->m_info()->audio_encoding;
                options["signalNbTracks"]= StringOps().fromInt(device->m_info()->audio_channels);

                device->m_close() ;
                delete(device) ;
            }
            else {
                options["duration"]= "unspecified" ;
                options["signalEncoding"] = "unspecified" ;
                options["signalNbTracks"]= "unspecified" ;
            }
        }
	}

	if ( signalFilename.empty() ) {
		FileInfo info(filename);
		info.setTail("wav");
		signalFilename = info.path();
	}
	options["signalFilename"] = signalFilename;

	try {

		DataModel data("TransAG");
		if ( format != data.guessFileFormat(filename) ) {
			Log::err() << "Input file is not in VSFT format" << endl;
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


