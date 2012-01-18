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
#include "Common/util/FormatTime.h"
#include "Common/util/FormatToUTF8.h"
#include "Common/VersionInfo.h"
#include "Formats/TXT/TXTWriter.h"

using namespace tag;

void printData(DataModel& data);
void printGraph(DataModel& data, string annot_id);

void USAGE(const char* progname) {
	Log::err() << "USAGE: " << progname << " [-f <fmt>] [-c <conv>] [-T] [-v] <filename> [graph_id]" << endl;
	Log::err() << "\toptions :" << endl;
	Log::err() << "\t\v-v : print program version " << endl;
	Log::err() << "\t\t-f <fmt>: define file format" << endl;
	Log::err() << "\t\t-c <conv>: applicable conventions" << endl;
	Log::err() << "\t\t -T : print formatted timecodes" << endl;
	exit(1);
}

bool do_format=false;

int main(int argc, char* const argv[]) {

	const char* progname = argv[0];
	int c;
	string annot_id("");
	string format = "";
	string conventions = "";

	while ((c = getopt(argc, argv, "f:c:Tv")) != -1) {
		switch (c) {
		case 'T': do_format=true; break;
		case 'f': format=optarg; break;
		case 'c': conventions=optarg; break;
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

	optind++;
	if (optind < argc) {
		const char* pt = strchr(argv[optind], ':');
		if (pt != NULL) {
			annot_id = string("TransAG") + pt;
		} else {
			Log::err() << "Invalid annotation id" << endl;
			return -1;
		}
	}

	DataModel::initEnviron(progname);
	//DataModel data("TransAG");
	DataModel data;

	if (format == "")
		format = data.guessFileFormat(filename);
	if (format == "") {
		Log::err() << "Unknown file format" << endl;
		return 1;
	}


	if ( !conventions.empty() ) {
		data.addAGOption("conventions", conventions);
		data.addAGOption("duration", "600.0");
	}
	Glib::Timer t;
	try {
		data.loadFromFile(filename,format);
	} catch (const char* msg) {
		Log::err() << "ERROR: " << msg << endl;
	}
	cerr << endl << "=============================================== Load done in " << t.elapsed() << " s." << endl;
	t.start();

	if (annot_id.empty() ) {
		TXTWriter writer;
		writer.setPrintDetailed(true);
		writer.write(cout, &data);
	} else {
		printGraph(data, annot_id);
	}

	cerr << endl << "=============================================== Print done in " << t.elapsed() << " s." << endl;

}


/**
 * print a graph part
 */
void printGraph(DataModel& data, string annot_id)
{

		if ( ! ExistsAnnotation(annot_id) ) {
			// is it an anchor ? if yes, then take into account highest level annotation starting at anchor
			if ( ExistsAnchor(annot_id) ) {
				const string& graphtype = data.getGraphType(annot_id);
				const set<AnnotationId>& ids = GetOutgoingAnnotationSet(annot_id, "");
				set<AnnotationId>::const_iterator it;
				string reftype="";
				for (it = ids.begin(); it != ids.end(); ++it) {
					const string& type = GetAnnotationType(*it);
					if ( data.isMainstreamType(type, graphtype) ) {
						if ( reftype == "" || data.conventions().isHigherPrecedence(type, reftype, graphtype) )
							annot_id = *it;
					}
				}
			} else {
				Log::err()		<< " non-existent annotation id" << endl;
				return;
			}
		}

		TXTWriter writer;
		writer.setPrintDetailed(true);

		const string& curtype = data.getElementType(annot_id);
		int renderer = 0;
		if ( curtype == "section") renderer = 1;
		else if ( curtype == "turn" ) renderer = 2;
		else if (curtype == "segment" ) renderer = 3;
		else if (curtype == "unit" ) renderer = 4;

		switch (renderer) {
		case 0:
			cerr << "INVALID annot_id " << annot_id << ", .. exiting" << endl;
			exit(1);
		case 1:		writer.renderSectionStart(cout, annot_id); break;
		case 2:		writer.renderTurnStart(cout, annot_id); break;
		case 3:		writer.renderSegmentStart(cout, annot_id); break;
		case 4: 	writer.renderBaseElement(cout, annot_id); break;
		}

		int notrack = data.getElementSignalTrack(annot_id);

		writer.renderAll(cout, data.getMainstreamTypes(), renderer-1, annot_id, notrack);
	}
