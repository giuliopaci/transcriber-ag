/********************************************************************************/
/******************** (c) Bertin Technologies 2006 - 2101  **********************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* FERRY Guillaume - LECUYER Paule - MONTILLA Jmarc 	*/
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

using namespace tag;

void addNewGraph(DataModel& data);
void addNewTrack(DataModel& data);

void USAGE(const char* progname) {
	Log::err() << "USAGE: " << progname << " [-f <fmt>] [-c <conv>] [-T] [-v] <filename> [graph_id]" << endl;
	Log::err() << "\toptions :" << endl;
	Log::err() << "\t\v-v : print program version " << endl;
	Log::err() << "\t\t-f <fmt>: define file format" << endl;
	Log::err() << "\t\t-c <conv>: applicable conventions" << endl;
	Log::err() << "\t\t-o <file>: destination file (default will update input file)" << endl;
	Log::err() << "\t\t -T : add new Track to hold translations (default will add new graphtype)" << endl;
	exit(1);
}

int main(int argc, char* const argv[]) {

	const char* progname = argv[0];
	int c;
	string outfile("");
	string format("");
	string conventions("");
	string target_lang("fre");

	bool do_newTrack=false;

	while ((c = getopt(argc, argv, "c:l:o:Tv")) != -1) {
		switch (c) {
		case 'T': do_newTrack=true; break;
		case 'f': format=optarg; break;
		case 'c': conventions=optarg; break;
		case 'l': target_lang=optarg; break;
		case 'o': outfile=optarg; break;
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

	string graphtype= "transcription_graph";
	string desttype= "translation_graph";

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

	if ( do_newTrack ) {
		makeNewTrack(data, graphtype, target_lang);
	} else {
		addNewGraph(data, graphtype, desttype, target_lang);
	}

	cerr << endl << "=============================================== Print done in " << t.elapsed() << " s." << endl;

}


/**
 * generate a new graph from existing transcription graph
 */
void addNewTrack(DataModel& data, const string& graphtype, const string& target_lang)
{

}

/**
 * generate a new graph from existing transcription graph
 * will not copy any "decoration", only mainstream types
 * will also "clusterize" all units into valid "sentences"
 * clusterize criteria : end-of-sentence punctuation / end of segment
 */
void addNewGraph(DataModel& data, const string& graphtype, const string& desttype, const string& target_lang)
{
	data.initAnnotationGraphs(desttype, tgtlang, "(auto)");
	const vector<string>& mainstream_types = data.getMainstreamTypes(graphtype);

	vector<string>::const_iterator itm;

}
