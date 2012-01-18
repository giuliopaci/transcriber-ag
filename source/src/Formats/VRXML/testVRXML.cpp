/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include <ag/agfio_plugin.h>
#include <xercesc/parsers/SAXParser.hpp>
#include <fstream>
#include <ag/AGException.h>
#include <ag/RE.h>
#include <ag/AGAPI.h>
#include "SAX_VRXMLHandler.h"
#include "agfXercesUtils.h"
#include "Common/FileInfo.h"


int main(int argc, const char* argv[])
{
	if ( argc != 2 ) {
		Log::err() << "Usage: " << FileInfo(argv[0]).Basename() << " <filename>" << endl;
		return 1;
	}
  try {
    xercesc_open();
  }
  catch (const agfioError& e) {
    throw agfio::LoadError(string("VRXML:") + e.what());
  }


	DataModel::initEnviron(argv[0]);

  tag::DataModel data("VRXML");

	// configure data model
	try {
		data.setConventions("transag_default", "fre");
	} catch ( const char* msg ) {
		Log::err() << msg << endl;
		data.setConventions("");
	}

  SAX_VRXMLHandler handler(data, "UTF-8");

  bool val_opt = false;
  string encoding = "";
      val_opt = false;

  try {
	  VRXMLagfSAXParse(&handler, argv[1], val_opt, encoding);
   }
  catch (const XMLException& e) {
    Log::err() << "VRXML:loading failed due to the following error\n" << trans(e.getMessage()) << endl;
  }
  catch (AGException& e) {
    Log::err() << "VRXML:loading failed due to the following error\n" <<
			   e.error() << endl;
  }
  catch (const char* msg) {
	Log::err() << "Caught error = " << msg << endl;
	}

  xercesc_close();

Log::err() << "AGID = " << data.getAGTrans() << endl;
Log::err() << "AGSETID = " << GetAGSetId(data.getAGTrans()) << endl;

	cout << toXML(GetAGSetId(data.getAGTrans()));

  return 0;

}
