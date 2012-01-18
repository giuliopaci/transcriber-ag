/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include <ag/AGException.h>
#include "CTM.h"
#include "Common/FileInfo.h"
#include <list>
using namespace std;


int main(int argc, const char* argv[])
{
	CTM loader;

	  map<string,string> options;

		const char* progname = argv[0];
	
		string convpath = FileInfo(progname).dirname(4);
		convpath += "/etc/TransAG/conventions/transag_default.rc";

	options["signalNbTracks"] = "1";
	options["duration"] = "600.0";
	options["conventions"] = convpath ;
	
	try {
		const list<AGId>& ids=loader.pubload(argv[1], "TransAG", NULL, &options);		
		list<AGId>::const_iterator it;
		for ( it=ids.begin(); it != ids.end(); ++it ) {
			cout << " AGId = " << *it << endl;
		}
	} catch (agfio::LoadError& e) {
		Log::err() << " ERROR " << e.what() << endl;
	}
	catch (...) {
		Log::err() << " Unknown exception" << endl;
	}
	return 0;
}
