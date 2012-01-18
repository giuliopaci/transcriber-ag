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
#include <ag/AGAPI.h>

#include <stdio.h>
#include <unistd.h>

#include "DataModel/DataModel.h"
#include "DataModel/DataModelMerge.h"
#include "Common/FileInfo.h"
#include "Common/VersionInfo.h"

using namespace tag;


void USAGE(const string& progname) {
	Log::err() << "USAGE: " << progname << "[-o <dest_file>] [-v]  <tag_file> <merged_tag_file>" << endl;
	exit(1);
}

int main(int argc, char* const argv[])
{

	string progname = FileInfo(argv[0]).Basename();

	int c;
	string format = "";
	string outpath = "";

	while ((c = getopt(argc, argv, "o:hv")) != -1) {
		switch (c) {
		case 'o': outpath=optarg; break;
		case 'v':
			cout << progname << " version " << TRANSAG_VERSION_NO << endl;
			return 0;
		default:
			USAGE(progname);
		}
	}

	if ( argc < optind+2  )
		USAGE(progname);

	const char* dest_file = argv[optind];
	const char* merged_file = argv[optind+1];


	DataModel::initEnviron(argv[0]);
	DataModel dest_data;
	DataModel merged_data;

	format = dest_data.guessFileFormat(dest_file);
	if (format == "") {
		Log::err() << "Unknown file format for file " << dest_file << endl;
		return 1;
	}

	if ( outpath.empty() ) {
		outpath = dest_file;
		if ( format != "TransAG" ) {// not a tag file -> will output in a tag file
			FileInfo info(outpath);
			outpath = FileInfo(info.dirname()).join(info.rootname());
			outpath += ".tag";
		}
	}

	string conventions = "transag_default";
	if ( format == "VRXML" || format == "CTM" ) {
		conventions="transag_tap";
	}

	try {
		dest_data.setConventions(conventions);
		dest_data.loadFromFile(dest_file,format, false);
	} catch (const char* msg) {
		Log::err() << dest_file << " : ERROR: " << msg << endl;
		return 1;
	}

	format = merged_data.guessFileFormat(merged_file);
	if (format == "") {
		Log::err() << "Unknown file format for file " << merged_file << endl;
		return 1;
	}

	try {
		merged_data.setConventions(conventions);
		merged_data.loadFromFile(merged_file,format, false);
	} catch (const char* msg) {
		Log::err() << merged_file << " : ERROR: " << msg << endl;
		return 1;
	}

	// do some checks
	// TODO -> check if compatible timelines
	//  if merge_mode == append_graph -> same timeline


	// now merge data
	DataModelMerge merger;

	if ( merger.merge(dest_data, merged_data, DataModelMerge::merge_append, 0.0) ) {
		// now save data
		dest_data.saveToFile(outpath);
	}

}
