// AG.cc: AG loader class implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <fstream>
#include <ag/AGException.h>
#include <ag/RE.h>
#include <ag/AGAPI.h>
#include "AG.h"
#include "SAX_AGHandlers.h"
#include "agfXercesUtils.h"
	/* (( BT Patch -- */
#include <errno.h>
	/* -- BT Patch )) */

namespace ns1 {

list<AGId>
AG::load(const string& filename,
	 const Id& id,
	 map<string,string>* signalInfo,
	 map<string,string>* options)
  throw (agfio::LoadError)
{
  try {
    xercesc_open();
  }
  catch (const agfioError& e) {
    throw agfio::LoadError(string("AG:") + e.what());
  }
  
  SAX_AGHandlers handler("UTF-8");

  bool val_opt = true;
  string encoding = "";
  if (options != NULL) {
    if ((*options)["dtd"] != "")
      handler.set_localDTD((*options)["dtd"]);
    if ((*options)["encoding"] != "")
      encoding = (*options)["encoding"];
    if ((*options)["DTDvalidation"] == "false")
      val_opt = false;
  }

  try {
    agfSAXParse(&handler, filename, val_opt, encoding);
  }
  catch (const XMLException& e) {
    throw agfio::LoadError(string("AG:loading failed due to the following error\n") + trans(e.getMessage()));
  }
  catch (AGException& e) {
    throw agfio::LoadError("AG:loading failed due to the following error\n" +
			   e.error());
  }

  list<AGId> result = handler.get_agids();
  xercesc_close();
  
  return result;
}

string
AG::store(const string& filename,
	  const string& id,
	  map<string,string>* options)
  throw (agfio::StoreError)
{
  ofstream out(filename.c_str());
  if (! out.good())
    throw agfio::StoreError("AG::store():can't open "+filename+"for writing");
  string encoding, dtd;
  if (options) {
    encoding = (*options)["encoding"];
    dtd = (*options)["dtd"];
  }
  if (encoding.empty())
    out << "<?xml version=\"1.0\"?>" << endl;
  else
    out << "<?xml version=\"1.0\" encoding=\""
	<< encoding << "\"?>" << endl;
  if (dtd.empty())
    out << "<!DOCTYPE AGSet SYSTEM \"http://agtk.sf.net/doc/xml/ag-1.1.dtd\">" << endl;
  else
    out << "<!DOCTYPE AGSet SYSTEM \""
	<< dtd << "\">" << endl;
  out << toXML(id).substr(55);
/* (( BT Patch -- */
  if ( !out.good() ) {
	  string msg = "AG::store(): write error on "+filename;
	  msg += strerror(errno);
	    throw agfio::StoreError(msg);
  }
 /* -- BT Patch )) */ 
  return "";
}

}
