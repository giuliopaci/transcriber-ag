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
 * VS Fast Transcription file loader class definition
 */

#define _TXT_IMPL

#include <iostream>
#include <string>
#include <fstream>
#include <ag/AGAPI.h>
#include "TXT.h"
#include "TXTParser.h"
#include "TXTWriter.h"
#include "TXTfile.h"
#include "Common/FileInfo.h"

#include <ag/AGException.h>



#ifdef EXTERNAL_LOAD
list<AGId>
TXT::plugin_load(const string& filename,
			const Id& id,
			map<string,string>* signalInfo,
			map<string,string>* options)
throw (agfio::LoadError)
{
	try {
		load(filename, id, signalInfo, options);
	}
	catch (const agfioError& e) {
		throw agfio::LoadError(string("TXT:") + e.what());
	}
}


string
TXT::plugin_store(const string& filename,
			const Id& id,
			map<string,string>* options)
throw (agfio::StoreError)
{
	try {
		return store(filename, id, options);
	}
	catch (const agfioError& e) {
		throw agfio::StoreError(string("TXT:") + e.what());
	}
}

#endif

TXT::TXT()
{
}

TXT::~TXT()
{
}


/**
 * load TXT file in DataModel
 * @param filename TXT file to load
 * @param id graph Id
 * @param signalInfo some signal data
 * @param options load options : corpus name, data model adress, load mode (full or not)
 * @return loaded AG ids list
 */
list<AGId>
TXT::load(const string& filename,
			const Id& id,
			map<string,string>* signalInfo,
			map<string,string>* options)
throw (agfio::LoadError)
{
	throw agfio::LoadError("No loader for plain text files");
}


/**
 * load DataModel to TXT file
 * @param filename TXT file to write
 * @param id AGSet Id
 * @param options save options : corpus name, data model adress, load mode (full or not)
 * @return ""
 */
string
TXT::store(const string& filename,
			const string& id,
			map<string,string>* options)     throw (agfio::StoreError)
{
	try {
		string addr= "";
		string corpusName = "";
		map<string,string>::iterator it;
		tag::DataModel* data = NULL;
		int nbtracks = 1 ;		

		if ( (it = options->find("&datamodel")) != options->end() )  
			addr = it->second;
		if (  addr != "" ) {
			// data model adress passed by caller -> use it
			data = (tag::DataModel*)strtoul(addr.c_str(), NULL, 16);
		} 
		else {
			// TODO : initialize data model from agset Id
//			data = new tag::DataModel(id);
			throw agfio::StoreError("No datamodel address provided");
		}

		if ( (it = options->find("nbtracks")) != options->end() )  {
			string track_s = it->second ;
			nbtracks = atoi(track_s.c_str()) ;
		}


		ostream* out = &cout;
		if ( filename != "" ) {
			ofstream* fout = new ofstream(filename.c_str());
			if ( ! fout->good() ) throw agfio::StoreError(string("Can't open file for writing: ")+filename);
			out = fout;
		}

		TXTWriter writer;
		writer.setPrintTimecode(true);
		writer.write(*out, data);
		if ( filename != "" ) ((ofstream*)out)->close();

	} catch (agfio::StoreError& e) {
		throw e;
	}
	catch (const char* msg) {
		throw agfio::StoreError("\nTXT format: " + string(msg));
	}
	return "";
}
