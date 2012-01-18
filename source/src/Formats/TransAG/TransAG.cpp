/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// TransAG.cc: TransAG loader class implementation
// based on AG loader class definition by Haejoong Lee, Xiaoyi Ma, Steven Bird
// ( Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
//   Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
//   For license information, see the file `LICENSE' included with aglib  distribution)


#include <errno.h>
#include <string.h>
#include <fstream>
#include <ag/AGException.h>
#include <ag/RE.h>
#include <ag/AGAPI.h>
#include "TransAG.h"
#include "SAX_TransAGHandler.h"
#include "agfXercesUtils.h"

#ifdef EXTERNAL_LOAD
list<AGId>
TransAG::plugin_load(const string& filename,
	 const Id& id,
	 map<string,string>* signalInfo,
	 map<string,string>* options)
  throw (agfio::LoadError)
{
  try {
    load(filename, id, signalInfo, options);
  }
  catch (const agfioError& e) {
    throw agfio::LoadError(string("TransAG:") + e.what());
  }
}

string
TransAG::plugin_store(const string& filename,
	  const string& id,
	  map<string,string>* options)
  throw (agfio::StoreError)
{
  try {
    store(filename, id, options);
  }
  catch (const agfioError& e) {
    throw agfio::StoreError(string("TransAG:") + e.what());
  }
}

#endif

list<AGId>
TransAG::load(const string& filename,
	 const Id& id,
	 map<string,string>* signalInfo,
	 map<string,string>* options)
  throw (agfio::LoadError)
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
  if (options != NULL) {
    if ((*options)["dtd"] != "")
      handler.set_localDTD((*options)["dtd"]);
    if ((*options)["encoding"] != "")
      encoding = (*options)["encoding"];
    if ((*options)["DTDvalidation"] == "false")
      val_opt = false;
  }

  try {
    tagSAXParse(&handler, filename, val_opt, encoding);
   }
  catch (const XMLException& e) {
    throw agfio::LoadError(string("TransAG:loading failed due to the following error\n") + trans(e.getMessage()));
  }
  catch (AGException& e) {
    throw agfio::LoadError("TransAG:loading failed due to the following error\n" +
			   e.error());
  }

  list<AGId> result = handler.get_agids();
  xercesc_close();

  return result;
}


string
TransAG::store(const string& filename,
	  const string& id,
	  map<string,string>* options)
  throw (agfio::StoreError)
{
  ofstream out(filename.c_str());
  if (! out.good())
    throw agfio::StoreError("TransAG::store():can't open "+filename+"for writing");
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
  if ( !out.good() ) {
	  throw agfio::StoreError(strerror(errno));
  }
  out.close();
  return "";
}


