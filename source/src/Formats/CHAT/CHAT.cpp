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
 * CHAT.cpp: CHILDES CHAT native format : class definition
 */

#define _CHAT_IMPL

#include <iostream>
#include <string>
#include <fstream>
#include <ag/AGAPI.h>
#include "CHAT.h"
#include "CHATfile.h"
#include "CHATParser.h"
#include "CHATWriter.h"
#include "Common/FileInfo.h"
#include "Common/util/StringOps.h"

#include <ag/AGException.h>



#ifdef EXTERNAL_LOAD
list<AGId>
CHAT::plugin_load(const string& filename,
			const Id& id,
			map<string,string>* signalInfo,
			map<string,string>* options)
throw (agfio::LoadError)
{
	try {
		load(filename, id, signalInfo, options);
	}
	catch (const agfioError& e) {
		throw agfio::LoadError(string("CHAT:") + e.what());
	}
}


string
CHAT::plugin_store(const string& filename,
			const Id& id,
			map<string,string>* options)
throw (agfio::StoreError)
{
	try {
		return store(filename, id, options);
	}
	catch (const agfioError& e) {
		throw agfio::StoreError(string("CHAT:") + e.what());
	}
}

#endif

CHAT::CHAT()
{}

CHAT::~CHAT()
{}


/**
 * load CHAT file in DataModel
 * @param filename CHAT file to load
 * @param id graph Id
 * @param signalInfo some signal data
 * @param options load options : corpus name, data model adress, load mode (full or not)
 * @return loaded AG ids list
 */
list<AGId>
CHAT::load(const string& filename,
			const Id& id,
			map<string,string>* signalInfo,
			map<string,string>* options)
throw (agfio::LoadError)
{

	CHATfile B;

	if (!B.open(filename)) {
		throw agfio::LoadError("CHAT: can't open " + filename);
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
			TRACE << "CHAT --> RETRIEVING EXISTING DATAMODEL" << std::endl ;
			data = (tag::DataModel*)strtoul(addr.c_str(), NULL, 16);
		} else {

			TRACE << "CHAT --> NEW DATAMODEL\n" << std::endl ;
			data = new tag::DataModel();
			data->setKeepAG(true); // do not delete graph upon datamodel deletion
			do_del = true;
		}

		CHATParser parser(corpusName, options, fullmode);
		res = parser.parse(B, data);

		data->updateVersionInfo("CHAT2TAG", "1");
		res.push_back(data->getAGTrans() );
	} catch (agfio::LoadError& e) {
		throw e;
	}
	catch (const char* msg) {
		throw agfio::LoadError("\nCHAT format: " + string(msg));
	}
//
//	if (data)
//		data->setImportWarnings(import_warning) ;

	if ( do_del )
		delete data;

	return res;
}


/**
 * load DataModel to CHAT file
 * @param filename CHAT file to write
 * @param id AGSet Id
 * @param options save options : corpus name, data model adress, load mode (full or not)
 * @return ""
 */
string
CHAT::store(const string& filename,
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


		// -- Single track export --
		int nbtracks = data->getNbTracks();

		ostream* out = &cout;

		if (nbtracks == 1)
		{
			if ( filename != "" )
			{
				ofstream* fout = new ofstream(filename.c_str());
				if ( ! fout->good() )
					throw agfio::StoreError(string("Can't open file for writing: ")+filename);
				out = fout;
			}

			CHATWriter writer;

			writer.setTrackID(-1);
			writer.write(*out, data, FileInfo(filename).rootname());

			if ( filename != "" )
				((ofstream*)out)->close();
		}

		// -- Multiple tracks --
		if (nbtracks > 1)
		{
			string rootname = filename.substr(0, filename.find_last_of("."));

			for(int i=0; i<nbtracks; i++)
			{
				// -- Renaming : file.cha -> file_N.cha --
				string trackname = rootname + "_";
				trackname += i;
				trackname += ".cha";

				// -- Writing CHA file --
				ofstream* fout = new ofstream(trackname.c_str());
				if ( ! fout->good() )
					throw agfio::StoreError(string("Can't open file for writing: ") + trackname);
				out = fout;

				CHATWriter writer;

				writer.setTrackID(i);
				writer.write(*out, data, FileInfo(trackname).rootname());

				if ( trackname != "" )
					((ofstream*)out)->close();
			}
		}

	} catch (agfio::StoreError& e) {
		throw e;
	}
	catch (const char* msg) {
		throw agfio::StoreError("\nCHAT format: " + string(msg));
	}
	return "";
}
