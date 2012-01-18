// TRS.cc: TRS loader class implementation
// Paule Lecuyer
// Copyright (C) 2007 Bertin Technologies
// Web: http://www.bertin.fr/
// For license information, see the file `LICENSE' included
// with the distribution.


#include <fstream>
#include <ag/AGException.h>
#include <ag/RE.h>
#include <ag/AGAPI.h>
#include "TRS.h"
#include "SAX_TRSHandlers.h"
#include "agfXercesUtils.h"


list<AGId>
TRS::load(const string& filename,
	 const Id& id,
	 map<string,string>* signalInfo,
	 map<string,string>* options)
  throw (agfio::LoadError)
{
  try {
    xercesc_open();
  }
  catch (const agfioError& e) {
    throw agfio::LoadError(string("TRS:") + e.what());
  }
  
  SAX_TRSHandlers handler("UTF-8");

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
    throw agfio::LoadError(string("TRS:loading failed due to the following error\n") + trans(e.getMessage()));
  }
  catch (AGException& e) {
    throw agfio::LoadError("TRS:loading failed due to the following error\n" +
			   e.error());
  }

  list<AGId> result = handler.get_agids();
  xercesc_close();
  
  return result;
}

string
TRS::store(const string& filename,
	  const string& id,
	  map<string,string>* options)
  throw (agfio::StoreError)
{
  ofstream out(filename.c_str());
  if (! out.good())
    throw agfio::StoreError("TRS::store():can't open "+filename+"for writing");
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
  return "";
}
