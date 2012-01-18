/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include <iostream>
#include <iterator>
#include <algorithm>
#include <set>
#include <vector>
#include <list>

#include <stdio.h>
#include <unistd.h>

#include "DataModel/speakers/SpeakerDictionary.h"
#include "Common/VersionInfo.h"

using namespace tag;
using namespace std;

void USAGE(const char* progname) {
	Log::err() << "USAGE: " << progname << " [-v] <spk_dic> " << endl;
	Log::err() << "\toptions :" << endl;
	Log::err() << "\t\v-v : print program version " << endl;
	exit(1);
}

int main(int argc, char* const argv[]) {

	const char* progname = argv[0];
	int c;
	string annot_id("");
	string format = "";
	string conventions = "";

	while ((c = getopt(argc, argv, "v")) != -1) {
		switch (c) {

		case 'v':
			cout << progname << " version " << TRANSAG_VERSION_NO << endl;
			return 0;
		default:
			USAGE(progname);
		}
	}

	if (optind == argc)
		USAGE(progname);
	const char* filename = argv[optind];


	SpeakerDictionary dico;
	SpeakerDictionary::iterator it;

	try {
		dico.loadDictionary(filename);
		for (it=dico.begin(); it != dico.end(); ++it)
			cout << it->first << " = " << it->second.getFullName() << endl;

	} catch (const char* msg) {
		Log::err() << "ERROR: " << msg << endl;
	}


	cout << endl << "================================================" << endl;

}

