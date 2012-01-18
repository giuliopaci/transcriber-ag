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
#include "SAX_TransAGHandler.h"
#include "agfXercesUtils.h"


int TransAG_load(const char* path)
{
  try {
    xercesc_open();
  }
  catch (const agfioError& e) {
    throw agfio::LoadError(string("TransAG:") + e.what());
  }

  SAX_TransAGHandler handler("UTF-8");

  bool val_opt = true;
  string encoding = "";
      handler.set_localDTD("/home/lecuyer/.TransAG/TransAG-1.0.dtd");
      val_opt = false;

  try {
    tagSAXParse(&handler, path, val_opt, encoding);
   }
  catch (const XMLException& e) {
    cerr << "TransAG:loading failed due to the following error\n" << trans(e.getMessage()) << endl;
  }
  catch (AGException& e) {
    cerr << "TransAG:loading failed due to the following error\n" <<
			   e.error() << endl;
  }
  catch (const char* msg) {
	cerr << "Caught error = " << msg << endl;
	}

  list<AGId> result = handler.get_agids();
  xercesc_close();

cout << "AGID = " << *(result.begin()) << endl;
cout << "AGSETID = " << GetAGSetId(*(result.begin())) << endl;

//	toXML(GetAGSetId(*(result.begin())));

  return 0;

}

int main(int argc, const char* argv[])
{
	if ( argc <= 1 ) {
		cerr << "usage: testTransAG <fichier_tag>" << endl;
		return 1;
	}
		return TransAG_load(argv[1]);
}

