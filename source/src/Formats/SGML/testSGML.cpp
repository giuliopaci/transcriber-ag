/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include <xercesc/parsers/SAXParser.hpp>
#include <fstream>
#include "SAX_SGMLHandler.h"
#include "SGMLobjects.h"
#include "agfXercesUtils.h"
#include "DataModel/DataModel.h"

int main(int argc, const char* argv[])
{
	try {
		xercesc_open();
	}
	catch (const char* msg) {
		Log::err() << "Caught error = " << msg << endl;
	}

	tag::DataModel* glou = new tag::DataModel() ;
	SGMLobjects SGML_objects ;
	SAX_SGMLHandler handler(*glou, &SGML_objects, "UTF-8");

	bool val_opt = false;
	string encoding = "iso-8859-1";
//	handler.set_localDTD("/homeLocal/LOCALDEVEL/dactilo/transcriber/LREP_devel_v1/LREP_devel_v1/source/etc/TransAG/TransAG-1.0.dtd");
	val_opt = false;

	try {
		SGMLagfSAXParse(&handler, argv[1], val_opt, encoding);
	}
	catch (const XMLException& e) {
		Log::err() << "TRS:loading failed due to the following error\n" << trans(e.getMessage()) << endl;
	}
//	catch (AGException& e) {
//		Log::err() << "TRS:loading failed due to the following error\n" <<
//		e.error() << endl;
//	}
//	catch (const char* msg) {
//		Log::err() << "Caught error = " << msg << endl;
//	}

//	list<AGId> result = handler.get_agids();
	xercesc_close() ;

//	cout << "AGID = " << *(result.begin()) << endl;
//	cout << "AGSETID = " << GetAGSetId(*(result.begin())) << endl;

//	toXML(GetAGSetId(*(result.begin())));

	SGML_objects.print() ;

	return 0 ;
}
