/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// VRXML.cc: VRXML loader class implementation
// Paule Lecuyer
// Copyright (C) 2007 Bertin Technologies
// Web: http://www.bertin.fr/


#include <sstream>
#include <ag/AGException.h>
#include <ag/RE.h>
#include <ag/AGAPI.h>
#include "VRXML.h"
#include "SAX_VRXMLHandler.h"
#include "agfXercesUtils.h"
#include "Common/iso639.h"

#include "DataModel/DataModel.h"


#define DEFAULT_CONVENTIONS "transag_default"
#define DEFAULT_LANGUAGE "eng"

list<AGId>
VRXML::load(const string& filename,
		const Id& id,
		map<string,string>* signalInfo,
		map<string,string>* options)
throw (agfio::LoadError)
{
	try {
		xercesc_open();
	}
	catch (const agfioError& e) {
		throw agfio::LoadError(string("VRXML:") + e.what());
	}


	bool val_opt = true;
	string encoding("");
	string localDTD("");
	string addr("");
	string agsetId ("");
	string conventions=DEFAULT_CONVENTIONS;
	string lang = DEFAULT_LANGUAGE;
	bool fullmode = true;
	float highlightThreshold = 100.0;
	bool keep_segmentation = true;

	if (options != NULL) {
		if ((*options)["dtd"] != "")
			localDTD = (*options)["dtd"];
		if ((*options)["encoding"] != "")
			encoding = (*options)["encoding"];
		if ((*options)["DTDvalidation"] == "false")
			val_opt = false;

		map<string,string>::iterator it;
		string item;

		if ( (it = options->find("&datamodel")) != options->end() )  addr = it->second;
		if ( (it = options->find("corpusName")) != options->end() )  agsetId = it->second;
		if ( (it = options->find("fullmode")) != options->end() )
			fullmode = (it->second == "true" || it->second == "1");
		if ( (it = options->find("do_score_highlight")) != options->end() ) {
			istringstream is(it->second);
			is >> highlightThreshold ;
		}
		if ( (it = options->find("keep_segmentation")) != options->end() ) {
			keep_segmentation= (it->second == "true" || it->second == "yes" || it->second == "1");
		}

		if ( (it = options->find("lang")) != options->end() )  item = it->second;
		if ( item.length() == 2 ) {
			const char* pl = ISO639::get3LetterCode(item.c_str());
			if ( pl != NULL ) lang = pl;
		} else if (item.length() == 3) { // check validity
			const char* pl = ISO639::get2LetterCode(item.c_str());
			if ( pl != NULL ) lang=item;
		}

		if ( (it = options->find("conventions")) != options->end() )
			conventions = it->second;
	}

	tag::DataModel* data = NULL;
	bool do_del = false;

//	if ( agsetId.empty() ) agsetId="TransAG";


	if (  addr != "" ) {
		// data model adress passed by caller -> use it
		Log::err() << "data model adress passed by caller -> use it" << endl;
		data = (tag::DataModel*)strtoul(addr.c_str(), NULL, 16);
	}
	else {
		// make a unique Set ID
		string prefix = "vrxml_";
		int no = 0;
		do {
			++no;
			ostringstream os;
			os << prefix << no;
			agsetId = os.str();
		} while ( ExistsAGSet(agsetId) );

		data = new tag::DataModel();
		data->setKeepAG(true); // do not delete graph upon datamodel deletion
		do_del = true;
	}

	// TODO -> ajouter mode avec sauvegarde des timecodes de mots

	if ( data->getAGPrefix() == "" )
		data->initAGSet(agsetId);

//	Log::err() << "BEFORE CONFIGURE=" << data->getAGPrefix() << " agtrans="<< data->getAGTrans() << endl;

	// configure data model
	try {
		data->setConventions(conventions, lang, fullmode);
	} catch ( const char* msg ) {
		Log::err() << msg << endl;
		data->setConventions("", "", fullmode);
	}

	SAX_VRXMLHandler handler(*data, "UTF-8");
	handler.setScoreThreshold(highlightThreshold);
	handler.setKeepSegmentation(keep_segmentation);

	data->setInhibateChecking(true);
	data->setGraphType("transcription_graph");
	try {
//		Log::err() << "BEFORE PARSE AGSET=" << data->getAGPrefix() << " agtrans="<< data->getAGTrans() << endl;
		VRXMLagfSAXParse(&handler, filename, val_opt, encoding);
	}
	catch (const XMLException& e) {
		throw agfio::LoadError(string("VRXML:loading failed due to the following error\n") + trans(e.getMessage()));
	}
	catch (AGException& e) {
		throw agfio::LoadError("VRXML:loading failed due to the following error\n" +
				e.error());
	}
	data->setInhibateChecking(false);
	data->setGraphType("");

	list<AGId> result ;
	if ( data != NULL  ) result.push_back(data->getAGTrans());
	xercesc_close();

	return result;
}

string
VRXML::store(const string& filename,
		const string& id,
		map<string,string>* options)
throw (agfio::StoreError)
{
	ofstream out(filename.c_str());
	if (! out.good())
		throw agfio::StoreError("VRXML::store():can't open "+filename+"for writing");
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
