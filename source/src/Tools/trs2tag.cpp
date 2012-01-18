/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// agdemo.cc: A demo of using AG API
// Steven Bird & Xiaoyi Ma
// Copyright (C) 2001-2003 Linguistic Data Consortium
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <set>
#include <ag/AGAPI.h>
#include <ag/AGException.h>
#include <ag/agfio.h>
#include <glib.h>
#include "Common/VersionInfo.h"

void USAGE(const char* progname)
{
	Log::err() << "USAGE: " << g_path_get_basename (progname) << " [-v] [-d <trans-dtd>] <filename>" << endl;
	Log::err() << "\toptions :" << endl;
	Log::err() << "\t\v-v : print program version " << endl;
	exit(1);
}

using namespace std;

int main(int argc, char* const argv[])
{
	string trans_dtd = "trans-14.dtd";

	const char* progname = argv[0];
	int c;

	while ((c = getopt(argc , argv, "vd:")) != -1) {
		switch (c) {
		case 'd':
		{ int novers;
			trans_dtd = optarg;
			if ( strstr(optarg,".dtd") == NULL ) trans_dtd += ".dtd";
			if ( sscanf(trans_dtd.c_str(), "trans-%d.dtd", &novers) != 1 ) {
				Log::err() << "Invalid transcriber DTD :" << trans_dtd << endl;
				return 1;
			}
		}
		case 'v':
			cout << "trs2tag version " << TRANSAG_VERSION_NO << endl;
			return 0;
		default: USAGE(progname);
		}
	}

	if ( optind == argc ) USAGE(progname);
	const char* filename = argv[optind];

	map < string, string > m_agOptions;
	string m_localDTD;

	m_localDTD = getenv("HOME");
	m_localDTD += "/.TransAG/";
	m_agOptions["system"] = trans_dtd;
	m_localDTD += m_agOptions["system"];

	m_agOptions["dtd"] = m_localDTD;
    m_agOptions["DTDvalidation"] = "false";

	try {
		list < AGId > ids = Load("TRS", filename, "", NULL, &m_agOptions);
		list < AGId >::iterator it;
		AGSetId loadId = "";
		loadId = GetAGSetId(*(ids.begin()));
		char buf[1024], *pt;
		strcpy(buf, filename);
		pt = strrchr(buf, '.');
		if (pt != NULL)
			*pt = 0;
		strcat(buf, ".tag");
		m_agOptions["system"] = "TransAG-1.0.dtd";
		m_agOptions["dtd"] = m_agOptions["system"];

		Store("TransAG", buf, loadId, &m_agOptions);

	}
	catch(agfio::LoadError & ex) {

		Log::err() << "ERROR : " << ex.what() << endl;

	}
	catch(AGException & ex) {
		Log::err() << "ERROR : " << ex.error();
	}
}
