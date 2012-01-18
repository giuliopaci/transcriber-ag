/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include <ag/AGException.h>
#include <ag/AGAPI.h>
#include "STM.h"
#include <list>
using namespace std;


int main(int argc, const char* argv[])
{
	map<string,string> options;

	options["signalNbTracks"] = "1";
	options["duration"] = "600.0";
	options["conventions"] = "../../../etc/TransAG/mono_h4_detailed.rc";

	string filename = "glou" ;
	try {
		const list<AGId>& ids= Load("STM", filename, "TransAG", NULL, &options);
		list<AGId>::const_iterator it;
		for ( it=ids.begin(); it != ids.end(); ++it ) {
			TRACE << " AGId = " << *it << endl;
		}
		//string newfic=argv[1];
		//newfic += ".copy";
		//loader.pubstore(argv[1], GetAGSetId(ids.front()), NULL);
	} catch (agfio::LoadError& e) {
		Log::err() << " ERROR " << e.what() << endl;
	}
	catch (...) {
		Log::err() << " Unknown exception" << endl;
	}
	return 0;
}
