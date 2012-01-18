/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * $Id*
 *
 * MDTM.cpp: CHILDES MDTM native format : class definition
 */

#define _MDTM_IMPL

#include <iostream>
#include <string>
#include <fstream>
#include <ag/AGAPI.h>
#include "MDTM.h"
#include "MDTMfile.h"
#include "MDTMParser.h"
//#include "MDTMWriter.h"
#include "Common/FileInfo.h"

#include <ag/AGException.h>



#ifdef EXTERNAL_LOAD
list<AGId>
MDTM::plugin_load(const string& filename,
			const Id& id,
			map<string,string>* signalInfo,
			map<string,string>* options)
throw (agfio::LoadError)
{
	try {
		load(filename, id, signalInfo, options);
	}
	catch (const agfioError& e) {
		throw agfio::LoadError(string("MDTM:") + e.what());
	}
}


/*
string
MDTM::plugin_store(const string& filename,
			const Id& id,
			map<string,string>* options)
throw (agfio::StoreError)
{
	try {
		return store(filename, id, options);
	}
	catch (const agfioError& e) {
		throw agfio::StoreError(string("MDTM:") + e.what());
	}
}
*/

#endif

MDTM::MDTM()
{}

MDTM::~MDTM()
{}


/**
 * load MDTM file in DataModel
 * @param filename MDTM file to load
 * @param id graph Id
 * @param signalInfo some signal data
 * @param options load options : corpus name, data model adress, load mode (full or not)
 * @return loaded AG ids list
 */
list<AGId>
MDTM::load(const string& filename,
			const Id& id,
			map<string,string>* signalInfo,
			map<string,string>* options)
throw (agfio::LoadError)
{

	MDTMfile B;

	if (!B.open(filename)) {
		throw agfio::LoadError("MDTM: can't open " + filename);
	}

	list<AGId> res;
	tag::DataModel* data = NULL;
	bool do_del = false;

	try {
		string addr= "";
		string corpusName = "";
		map<string,string>::iterator it;

		bool fullmode = true;

		if ( (it = options->find("&datamodel")) != options->end() )  addr = it->second;
		if ( (it = options->find("corpusName")) != options->end() )  corpusName = it->second;
		if ( (it = options->find("fullmode")) != options->end() )  fullmode = (it->second == "true" || it->second == "1");

		if ( corpusName.empty() ) corpusName="TransAG";

		if (  addr != "" ) {
			// data model adress passed by caller -> use it
			TRACE << "MDTM --> RETRIEVING EXISTING DATAMODEL" << std::endl ;
			data = (tag::DataModel*)strtoul(addr.c_str(), NULL, 16);
		} else {

			TRACE << "MDTM --> NEW DATAMODEL" << std::endl ;
			data = new tag::DataModel();
			data->setKeepAG(true); // do not delete graph upon datamodel deletion
			do_del = true;
		}

		MDTMParser parser(corpusName, options, fullmode);
		res = parser.parse(B, data);

		data->updateVersionInfo("MDTM2TAG", "1");
		res.push_back(data->getAGTrans() );
	} catch (agfio::LoadError& e) {
		throw e;
	}
	catch (const char* msg) {
		throw agfio::LoadError("\nMDTM format: " + string(msg));
	}
//
//	if (data)
//		data->setImportWarnings(import_warning) ;

	if ( do_del )
		delete data;

	return res;
}


/**
 * load DataModel to MDTM file
 * @param filename MDTM file to write
 * @param id AGSet Id
 * @param options save options : corpus name, data model adress, load mode (full or not)
 * @return ""
 */
/*
string
MDTM::store(const string& filename,
			const string& id,
			map<string,string>* options)     throw (agfio::StoreError)
{
	try {
		string addr= "";
		string corpusName = "";
		map<string,string>::iterator it;
		tag::DataModel* data = NULL;

		if ( (it = options->find("&datamodel")) != options->end() )  addr = it->second;
		if (  addr != "" ) {
			// data model adress passed by caller -> use it
			data = (tag::DataModel*)strtoul(addr.c_str(), NULL, 16);
		} else {
			// TODO : initialize data model from agset Id
//			data = new tag::DataModel(id);
			throw agfio::StoreError("No datamodel address provided");
		}

		ostream* out = &cout;
		if ( filename != "" ) {
			ofstream* fout = new ofstream(filename.c_str());
			if ( ! fout->good() ) throw agfio::StoreError(string("Can't open file for writing: ")+filename);
			out = fout;
		}

		MDTMWriter writer;
		writer.write(*out, data, FileInfo(filename).rootname());
		if ( filename != "" ) ((ofstream*)out)->close();

	} catch (agfio::StoreError& e) {
		throw e;
	}
	catch (const char* msg) {
		throw agfio::StoreError("\nMDTM format: " + string(msg));
	}
	return "";
}
*/

