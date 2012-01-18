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
#include <getopt.h>
#include <vector>
#include <list>
#include <ag/AGAPI.h>

#include "DataModel/DataModel.h"
#include "DataModel/DataModel_CPHelper.h"
#include "Common/VersionInfo.h"
#include "Common/FileInfo.h"

using namespace tag;

//void printData(DataModel& data);
void printSpeechSegments(DataModel& data);

#ifdef AUTRE_TEST
int main(int argc, const char* argv[])
{
	if ( argc < 2 ) {
	cout << "Usage : " << argv[0] << " <filename>" << endl;
	return 1;
	}
	try {
	list<WordList> meslistes;
	list<WordList>::iterator it;
	WordList::loadLists(argv[1], meslistes);

	for ( it = meslistes.begin(); it != meslistes.end(); ++it )
		cout << " Lu = " << it->getLabel() << endl;
	} catch (const char* msg ) {
		cout << "ERREUR " << msg << endl;
	}
}
#endif

void USAGE(const char* progname)
{
	Log::err() << "USAGE: " << progname << " [-v] [-f <fmt>] <filename>" << endl;
	Log::err() << "\toptions :" << endl;
	Log::err() << "\t\t-v : print program version " << endl;
	Log::err() << "\t\t-f <fmt>: define file format" << endl;
	Log::err() << "\t\t-c <conv>: applicable conventions" << endl;
	exit(1);
}


void run_test(string filename, string format, string conventions);
void printData(DataModel& data);

int main(int argc, char* const argv[])
{
	string format = "";
	string conventions = "";
	const char* progname = argv[0];
	int c;
	string rep;

	while ((c = getopt(argc , argv, "vf:c:")) != -1) {
		switch (c) {
		case 'f': format=optarg; break;
		case 'c': conventions=optarg; break;
		case 'v':
			cout << "clean_tag version " << TRANSAG_VERSION_NO << endl;
			return 0;
		default: USAGE(progname);
		}
	}

	if ( optind == argc ) USAGE(progname);
	const char* filename = argv[optind];
 	FileInfo(filename).exists();

	DataModel::initEnviron(progname);

	run_test(filename, format, conventions);

//	cout << "===================" << endl;
//	cout << "======FIN FIN 1" << endl;
//	cout << "======[press ret]" ;
//	cin >> rep;
//
//	run_test(filename, format, conventions);

}

void run_test(string filename, string format, string conventions)
{

	try {

	DataModel data("TransAG");

	if ( format == "" )
		format = data.guessFileFormat(filename);
	if ( format == "" ) {
			 Log::err() << "Unknown file format" << endl;
				return ;
	}

	if ( !conventions.empty() ) {
		data.addAGOption("conventions", conventions);
		data.addAGOption("duration", "600.0");
	}
	Glib::Timer t;
	cout << endl << "================================================" << endl;
	data.loadFromFile(filename,format);

	cerr << endl << "=============================================== Load done in " << t.elapsed() << " s." << endl;
	t.start();
//	printData(data);
	cerr << endl << "=============================================== Print done in " << t.elapsed() << " s." << endl;
//
//
//	cout << endl << "================================================" << endl << endl;
//	string start_id, end_id;
//	int start_offset, end_offset;
////	cout << "start_id: " ; cin >> start_id;
////	cout << "start_offset: " ; cin >> start_offset;
////	cout << "end_id: " ; cin >> end_id;
////	cout << "end_offset: " ; cin >> end_offset;
//
//	start_id = 	"trs_1:1:E7"; start_offset = 0;
//	end_id = "trs_1:1:ED"; end_offset = 1;
//
//	DataModel_CPHelper helper(&data);
//	string buffer = helper.getSubgraphTAGBuffer(start_id, start_offset, end_id, end_offset);
//	cout << endl << "================================================" << endl ;
//	cout << " TAG Buffer =" << endl << buffer << endl;
//	cout << endl << "================================================" << endl << endl;
//	cout << "GET TEXT = " << endl << helper.getTextFromTAGBuffer(buffer, "segment", "value") << endl<< endl<< endl;

	} catch (const char* msg ) {
		Log::err() << " Caught exception : " << msg << endl;
	}
}

void printData(DataModel& data)
{
		string aid = data.getAnchorAtOffset(data.getAGTrans(), 0, 0.0);
string btype = data.mainstreamBaseType("transcription_graph");
//cout << "Starting at anchor " << aid << endl;
	 	while ( aid != "" ) {
			const set<AnnotationId>& ids = GetOutgoingAnnotationSet(aid, "");
			set<AnnotationId>::const_iterator it;
			aid = "";
		for ( it = ids.begin(); it != ids.end(); ++it ) {
//			if ( data.getOrder(*it) > 0 ) cout << "  >> " << endl;
//			if ( !GetAnchored(GetStartAnchor(*it)) ) cout << "\t";
			const string& type = GetAnnotationType(*it) ;
			int order = data.getOrder(*it);

			cout << "@@@  type=" << type << " id=" << *it  << " order=" << order
				<< " parent=" << data.getParentElement(*it) << " basetype parent="<<data.getParentElement(*it, btype) << endl;
			if ( order == 0 && type == btype )
				aid = GetEndAnchor(*it);
		}
	}
}
