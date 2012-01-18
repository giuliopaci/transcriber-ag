/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
*  @file DataModel.cpp
*  @brief data model for Transcriber application
*    implements the interface between Transcriber Editor and AG data model,
*    implements data model management rules.
*/
/*========================================================================
*
*  AG File management
*
*========================================================================*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib/gstdio.h>

#include <fstream>
#include <sstream>
#include <set>

#include <ag/AGAPI.h>
#include <ag/AGException.h>
#include <ag/agfio.h>

#include "Common/util/StringOps.h"
#include "DataModel/DataModel.h"
#include "Common/FileInfo.h"
#include "Common/VersionInfo.h"
#include "Common/iso639.h"
#include "Common/util/Utils.h"

#include "Common/Formats.h"
#include "Common/globals.h"
#include "Common/util/Log.h"

using namespace std;

namespace tag {


//------------------------------------------------------------------------------
//------------------------------------ LOADING ---------------------------------
//------------------------------------------------------------------------------

/**
 *  load data model from annotation file
 * @param path 		Annotation file path
 * @param format 	Annotation file format - must be valid and can be checked first with guessFileFormat
 * @param fullmode 	If true, will fully load conventions (including related items such as topics and qualifiers labels layout), else will only load required items) (default to true)
 * @throw errmsg 	Error message if loading failed
 *
 * @note 			Fullmode is generally set to true when loading occurs from annotation editor, else set to false
 */
/*
 *   <*> TRICK <*>
 *   AGAPI doesn't allow loading two files with same AGSet Id
 *   If exception caught due to problem of duplicate AGSet id
 *   create temporary copy from file, "rename" AGSet Id in it
 * 	and load temporary file
 */
bool DataModel::loadFromFile(const string& path, const string& format, bool fullmode, bool reset_agId) throw (const char*)
{
//	Log::setTraceLevel(Log::MAJOR);
	bool ret = false;

	if ( m_agsetId != "") {
		deleteAGElements();
	}

	setAGOptions(format, false);

	// also pass current object adress to loader
	// coz some loaders may directly fill in a DataModel
	char addr [10];
	sprintf(addr, "%lx", (unsigned long)this);
	addAGOption("&datamodel", addr); // -> will store data in current model
	addAGOption("fullmode", (fullmode ? "true" : "false")); // -> will store data in current model
	string default_id = format;

	if ( format != "TransAG" && ExistsAGSet(default_id) ) {
		m_savId = default_id;
		default_id = DataModel::mktmpAGSetId();
		m_tmpId = default_id;
	}
	addAGOption("corpusName", default_id);

	//> TRY TO LOAD FORMAT QUALIFIER MAPPING
	string import_convention ;
	bool convention_done = false ;
	if ( !isTAGformat(format) )
		m_qualifierMapping = loadQualifierMapping(format) ;
	else
		m_qualifierMapping = NULL;

	if (m_qualifierMapping != NULL)
	{
		char mapp [10];
		sprintf(mapp, "%lx", (unsigned long)m_qualifierMapping);
		addAGOption("&qualifierMapping", mapp);

		import_convention = getConversionConventionFile(format, m_qualifierMapping) ;
		if (!import_convention.empty())
		{
			addAGOption("conventions", import_convention) ;
			import_convention = Glib::path_get_basename(import_convention) ;
			convention_done = true ;
		}
	}

	//> No convention mapping was found, let's apply default convention
	if (!convention_done)
		addAGOption("conventions", getConventionsName(), false); // -> will use current conventions by default

	list<AGId> loaded_agids;

	Log::trace() << "DataModel::loadFromFile path=" << path << " format=" << format << " - conventions=" << m_conventions.path() << endl;
	Log::trace() << "> using check module [" << getModelCheckerStatus() << "]" << endl;
	setInhibateSignals(true);

	if (reset_agId)
		m_tmpId = "" ;


	// Keep old status because following action can modify file
	// but all should be transparent for the user
	bool updated_before = m_updated ;

	//> LOAD FROM FILE
	try
	{
		loaded_agids = Load(format, path, "", NULL, &m_agOptions);
		m_path = path;
	}
	catch (AGException& ex) {
			const string& err = ex.error();
			TRACE_D<<"DataModel::loadFromFile:  caught AGException "<<err<<endl;
			return false ;
	}
	catch (agfio::LoadError& ex) {

		const string& err = ex.what();

		if ( m_tmpId == "" ) { // first try -> create tmp AGSetId
			if ( StringOps(err).contains("exists", false) ) {

				m_savId = (m_defaultCorpusName.empty() ? "TransAG" : m_defaultCorpusName);

				unsigned long pos = err.find_first_of("`'");

				if ( pos != string::npos ) {
					m_savId = err.substr(pos+1, err.find_first_of("`'", pos+1)-pos-1);
				}
				TRACE_D << "Aglib duplicate id " << m_savId  << " -> renaming AG" << endl;
				string tmpfic = fixAGSetId(path, m_savId, m_tmpId);
				bool ok = loadFromFile(tmpfic, format, fullmode, reset_agId);
				g_remove(tmpfic.c_str());
				setInhibateSignals(false);
//				Log::resetTraceLevel();
				return ok ;
			}
		}

		TRACE_D <<"DataModel::loadFromFile:  caught agfio::LoadError "<<err<<endl;
		ostringstream msg;
		msg	<< _("Error loading file") << " "<<  path << " :\n " << err << endl;
		setInhibateSignals(false);
//		Log::resetTraceLevel();
		throw msg.str().c_str();
	}
	catch (...) {
		TRACE_D<<"DataModel::loadFromFile:  caught unknown exception "<<endl;
		setInhibateSignals(false);
		return false ;
	}

	try
	{
		ret = initFromLoadedAG(format, loaded_agids, fullmode, import_convention);

		if (!updated_before && m_updated)
			m_updated = false ;

		// Warning: ALWAYS DO IT AFTER GRAPH INITIALIZATION because we need some graphs information
		fixLangBackwardCompatibility();

		// CHECK REMAINING CONVENTION ANNOTATION GRAPH
		actualizeGraphForConvention() ;
	}
	catch (const char* msg) {
		setInhibateSignals(false);
		ostringstream os;
		os	<< _("Error loading file") << " "<<  path << " : " <<  msg;
			throw os.str().c_str();
	}
	//	Log::resetTraceLevel();
	setInhibateSignals(false);

	setUpdated(m_updated) ;

	const list<AnnotationId>& lsect =  GetAnnotationSeqByOffset(getAG("transcription_graph"),0.0, 0.0, "section");
//	cout << "°°°°°°°°°°°°°°°°°°°°°°°°°°° found " << lsect.size() << " sections" << endl;

	return ret ;
}

bool DataModel::initFromLoadedAG(const string& import_format, const list<AGId>& loaded_agids, bool fullmode, const string& convention_name)
{
	ostringstream warns;
	// get AG data
	try
	{
		m_import_format = import_format;

		if ( loaded_agids.size() > 0 )
		{
			list<AGId>::const_iterator it;
			AGId agid = loaded_agids.front();

			m_timelineId = GetTimelineId(agid);
			getSignalDuration(true);
			m_agsetId = GetAGSetId(agid);

			//> get convention data from file
			string convname ;
			string version ="" ;
			if ( !convention_name.empty() )
				convname = convention_name ;
			else
			{
				convname = getAGSetProperty("convention_id", "");
				version = getAGSetProperty("convention_version", "");
			}

			//> CHECK CONVENTIONS
			//  (for imported format, it has  been done by corresponding plugin)
			if (isTAGformat(import_format) && m_checker)
				m_checker->checkConventions(convname, version) ;

			//> SET CONVENTION (will take default one by default)
			if (!convention_name.empty())
				convname = convention_name ;

//			if ( /*getConventionsName() != convname*/ convention_path.empty() )
				setConventions(convname, "", fullmode);

			// SIGNAL CONFIGURATION IS EMPTY
			// ---> means that io plugin doesn't rely on DataModel
			// ---> use graph data and fill signal configuration
			if ( signalCfg.getNbSignals("") == 0 )
			{
				const set<SignalId>& sigIds = GetSignals(m_timelineId);
				loadSignalsFromFile(sigIds) ;
			}

			signalCfg.checkTracks() ;

			m_anchorLinks.setModel(this) ;

			//> CHECK GRAPH
			string graphtype = "";
			for ( it=loaded_agids.begin(); it != loaded_agids.end() ; ++it )
			{
				graphtype = guessGraphType(*it) ;

				if ( ! graphtype.empty() )
				{
					if ( m_conventions.hasGraphType(graphtype) ) {
						int checkgraph ;
						if (m_checker) {
							checkgraph = m_checker->checkGraph(*it, graphtype) ;
							TRACE << "> checking graph " << graphtype << " [" << checkgraph << "]" << std::endl ;
							if ( m_checker->getCheckResult(graphtype) )
								TRACE << "> checking graph " << graphtype << " [" << checkgraph << "] - nb errors = " << m_checker->getCheckResult(graphtype)->get_errorCodes().size() << std::endl ;
						}
						else
							checkgraph = 1 ;

						if (checkgraph != -1)
						{
							m_agIds[graphtype] = *it;
							m_agBaseType[*it] = m_conventions.mainstreamBaseType(graphtype);
							updateAnchorTrackMap(*it, m_anchorTrack);
							graphSpecificTreatment(graphtype) ;
						}
					}
					else
						Log::err() << "File contains " << graphtype << " graph  which is not defined in " << m_conventions.name() << " conventions, will be ignored." << endl;
				}
				else
					Log::err() << "checking graph: TYPE COULDN'T BE DEFINED" << std::endl ;
			}


			// no graphs in file correspond to convention: STOP there
			if ( m_agIds.size()==0 )
				return false ;


			// FILE VERSION
			loadVersionData() ;

			//> ADJUST SIGNAL LENGHT IF NOT PREVIOUSLY DONE
			//  When creating new graph, it may be happen that
			//  signal durationisn't correct if it wasn't set when initGraphMainstream
			//  was called.
			//  Therefore don't set to updated: we'll set the timeline value in AGgraph
			//  but it will be the same value than in file
			if ( m_signalLength == UNDEFINED_LENGTH || m_signalLength==0 )
				getSignalDuration(true) ;

			// CHECK SIGNAL DISPLAY
			string forced = getAGSetProperty("single_signal") ;
			if (!forced.empty())
				signalCfg.setSingleSignal(true) ;
		}

		if ( ! warns.str().empty() )
			TRACE <<  warns.str() << endl;
	}
	catch (AGException& ex) {
//		Log::resetTraceLevel();
		throw ex.error().c_str();
	}
	return true;
}

std::string DataModel::guessGraphType(const string& agId)
{
	string graphtype("") ;
	if (ExistsFeature(agId, "graphtype"))
		graphtype = GetFeature(agId, "graphtype");
	else if (ExistsFeature(agId, "type"))
	{
			string type = GetFeature(agId, "type");
			if ( strstr(type.c_str(), "transcription" ) != NULL ) {
				graphtype = "transcription_graph"; // normalize type
			}
	}

	// try to guess type from mainstream types
	if ( graphtype.empty() ) {
		const set<string>& types = GetAnnotationTypes(agId);
		if ( types.size() > 0 )
			graphtype = conventions().getGraphtypeFromTypes(types);
	}

	return graphtype ;
}


void DataModel::graphSpecificTreatment(const string& graphtype)
{
	std::map<string,string>::iterator it =  m_agIds.find(graphtype) ;

	if (it == m_agIds.end())
		return ;

	string agId = it->second ;

	if (graphtype.compare("transcription_graph")==0)
	{
		string dtdpath = Glib::path_get_dirname(m_conventions.getDirectory()) ;
		dtdpath = Glib::build_filename(dtdpath, m_agOptions["system"]) ;
		if ( !Glib::file_test(dtdpath, Glib::FILE_TEST_EXISTS) )
			dtdpath="" ;

		//> -- Load speaker dictionary (is associated to agId)
		if ( ExistsFeature(agId, "speakers") )
		{
			const string& spkbuf = GetFeature(agId, "speakers") ;
			if ( !spkbuf.empty() && spkbuf.find_first_not_of(" \n") != string::npos )
			{
				m_speakersDict.fromXML(spkbuf, dtdpath);
			}
			Log::trace() << "Nb speakers = " << m_speakersDict.size() << endl;

			//> -- Initialize speaker hint for each track
			string spk ;
			if (  m_speakersDict.size() > 0 )
			{
				SpeakerDictionary::iterator its = m_speakersDict.begin();
				int i;
				for (i=0; i < getNbTracks(); ++i )
				{
					if ( getSpeakerHint(i).empty() )
					{
						//> if not enough speaker use last one
						if ( its != m_speakersDict.end() )
							spk = its->first ;
						setSpeakerHint(spk, i);
						++its;
					}
				}
			}
		}

		//> -- Load anchor Links (is associated to agSet)
		if ( ExistsFeature(getAGPrefix(), "anchor_links") )
		{
			const string& buf = GetFeature(getAGPrefix(), "anchor_links") ;
			if ( !buf.empty() && buf.find_first_not_of(" \n") != string::npos )
			{
				m_anchorLinks.fromXML(buf, dtdpath);
			}
			Log::out() << "Anchor list size = " << m_anchorLinks.getSize() << endl;
		}
	}
}

void DataModel::loadSignalsFromFile(const set<SignalId>& sigIds)
{
	set<SignalId>::const_iterator its;

	bool automatic_numbering = true ;
	// track number is given for signal, don't do numbering
	if ( StringOps(GetSignalTrack(*(sigIds.begin()))).toInt() > 0 )
		automatic_numbering = false ;

	for (its = sigIds.begin(); its != sigIds.end(); ++its )
	{
		int notrack ;
		if (automatic_numbering)
			notrack = -1 ;
		else
			notrack = StringOps(GetSignalTrack(*its)).toInt() -1 ;

		string classtype = GetSignalMimeClass(*its) ;
		string name = GetSignalXlinkHref(*its) ;

		if (classtype == "video")
			signalCfg.enterVideoSignal(*its, name, notrack) ;
		else
			signalCfg.enterAudioSignal(*its, name, notrack) ;
	}
}

void DataModel::loadVersionData()
{
	// load versions history (associated to agSet)
	if ( ExistsFeature(m_agsetId, "versions") )
	{
		const string& versbuf = GetFeature(m_agsetId, "versions");
		if ( !versbuf.empty() && versbuf.find_first_not_of(" \n") != string::npos )
				m_versions.fromXML(versbuf);
		Log::trace() << "Nb versions = " << m_versions.size() << endl;
		if ( m_versions.size() > 0 )
				m_lastVersion = m_versions.back().getId();
	}
}

int DataModel::actualizeGraphForConvention()
{
	int cpt = 0 ;
	ostringstream warns;
	const string& lang = getTranscriptionLanguage();
	TRACE << ">>> checking annotation graphs defined in conventions agId.size=" << m_agIds.size() << " conv.size=" << m_conventions.getGraphTypes().size() << endl;
	if ( m_agIds.size() <  m_conventions.getGraphTypes().size() )
	{
		vector<string>::const_iterator itt ;
		for ( itt = m_conventions.getGraphTypes().begin(); itt != m_conventions.getGraphTypes().end(); ++itt ) {
			if ( m_agIds.find(*itt) == m_agIds.end() ) {
				warns << "INFO : adding  " << *itt << " annotation graph" << endl;
				initAnnotationGraphs(*itt, lang, "(auto)");
				cpt ++ ;
				if (m_checker)
					m_checker->addAddedGraph(*itt) ;
			}
		}
	}
	if (cpt > 0 )
		m_updated = true ;
}



//------------------------------------------------------------------------------
//------------------------------------- SAVING ---------------------------------
//------------------------------------------------------------------------------

/*
*  save data model to file
*/
void DataModel::saveToFile(const string& path, const string& format, bool cleanup_unused)  throw (const char*)
{
	std::map<string,string> options ;
	saveToFileWithOptions(options, path, format, cleanup_unused) ;
}


/**
*  save data model to annotation file
* @param path annotation file path
* @param format annotation file format (default to "TransAG")
* @param cleanup_unused if true, cleanup unused speakers from speakers dictionnary before saving file (default to true)
* @throw errmsg error message if save operation failed
*/
void DataModel::saveToFileWithOptions(const std::map<string,string>& options, const string& path, const string& format, bool cleanup_unused)  throw (const char*)
{
	if (m_agsetId.empty())
	{
		Log::err() << "Unexpected error : empty AGSetId. Aborted" << std::endl;
		string msg = _("Error while saving file...") ;
		throw msg.c_str() ;
	}

	// check that target path is writable
	string check = path;
	if ( ! FileInfo(check).exists() ) check = FileInfo(check).dirname();
	if ( ! FileInfo(check).canWrite() ) {
		string msg = _("No write access on "); msg += check;
		throw msg.c_str() ;
	}

	if ( m_agIds.size() > 0 ) {
		// update speakers dictionary in graph before storing
		storeSpeakersDict( cleanup_unused && (m_conventions.getConfiguration("speakers,auto_cleanup") == "true") );
		// add used topics information
		storeTopicsList() ;
		// add anchor links
		storeAnchorLinks() ;
	}

	setAGOptions(format, true);
	char addr [10];
	sprintf(addr, "%lx", (unsigned long)this);
	addAGOption("&datamodel", addr); // -> will store data in current model

	try {
		string outpath = path;
		if ( ! m_tmpId.empty() && (format == "TransAG" ) ) outpath = path + "." + m_tmpId;

		Store(format, outpath, m_agsetId, &m_agOptions);


		if ( ! m_tmpId.empty()  && (format == "TransAG" ) ) {
			// restore original AGSetId
			fixAGSetId(outpath, m_tmpId, m_savId, path);
			if ( g_remove(outpath.c_str()) != 0 ) {
				string msg = "g_remove " + outpath + " :";
				msg += strerror(errno);
				TRACE_D << msg << endl;
			}
		}
		m_path = path;

		//> set model status to false for real saving (not backup one)
		if ( outpath[outpath.length()-1] != '~' )
			m_updated = false;
	}
	catch (agfio::StoreError &ex) {
		throw ex.what();
	}
/*	catch (agfio::WriteError &ex) {
		throw ex.what();
	}*/
	catch (...) {
		Glib::ustring msg = Glib::ustring(_("Error saving file")) + ": " + path ;
		throw msg.c_str();
	}

}

/* storeSpeakersDict */
void DataModel::storeSpeakersDict(bool cleanup_unused)
{

	const string& type = m_conventions.getTypeFromFeature("speaker");

	if ( type.empty() ) {
		MSGOUT << "**Warn : no type with feature 'speaker' defined in conventions " << m_conventions.name() << endl;
		return;
	}

	const string& graphtype = m_conventions.getGraphtypeFromType(type);
	const string& graphId = m_agIds[graphtype];

	if(cleanup_unused)
        {
		// remove "unused" speakers from dictionary
		SpeakerDictionary::iterator it;
		list<string> todel;
		list<string>::iterator it2;

		for(it = m_speakersDict.begin(); it != m_speakersDict.end(); ++it)
                {
                        const set<AnnotationId>& ids = GetAnnotationSetByFeature(graphId, "speaker", it->second.getId(), type);
                        if(ids.size() == 0)
                        {
                                todel.push_back(it->second.getId());
                        }
		}
		for(it2 = todel.begin(); it2 != todel.end(); ++it2)
                {
			m_speakersDict.deleteSpeaker(*it2);
                }
	}
	ostringstream os;
	m_speakersDict.toXML(os);
	SetFeature(graphId, "speakers", os.str());
}

/* storeAnnotTopics */
void DataModel::storeTopicsList() throw (const char*)
{
	const char* delim = "\n" ;
	std::set<Glib::ustring> used_topic ;

	const string& type = m_conventions.getTypeFromFeature("topic");

	if ( !type.empty() ) {
		const string& graphtype = m_conventions.getGraphtypeFromType(type);
		const string& graphId = m_agIds[graphtype];

		std::set<AnnotationId> sections = GetAnnotationSet(graphId, type) ;
		std::set<AnnotationId>::iterator it ;
		for (it=sections.begin(); it!=sections.end(); it++) {
			if (ExistsFeature(*it, "topic")) {
				string id = GetFeature(*it, "topic") ;
				if (!id.empty()) {
					used_topic.insert(id) ;
				}
			}
		}

		if (used_topic.size()>0) {
			ostringstream os ;
			Topics::toXML(os, m_conventions.getTopics(), used_topic, delim);
			SetFeature(graphId, "topics", os.str());
		}
		else
			if (ExistsFeature(graphId,"topics"))
				DeleteFeature(graphId, "topics") ;
	}
}


void DataModel::storeAnchorLinks() throw (const char*)
{
	if ( m_anchorLinks.getSize()==0 && ExistsFeature( getAGPrefix(), "anchor_links") )
		DeleteFeature(getAGPrefix(), "anchor_links") ;
	else
	{
		const char* delim = "\n" ;
		ostringstream os ;
		m_anchorLinks.toXML(os, delim) ;
		SetFeature(getAGPrefix(), "anchor_links", os.str());
	}
}

void DataModel::initVersionInfo(const string& scribe, const string& date, const string& comment)
{
	m_versions.clear();
	Version nv(m_versions.newVersionId(), date, scribe, "", TRANSAG_VERSION_NO, comment);
	m_versions.addVersion(nv);
	m_lastVersion = m_versions.back().getId();

	ostringstream os;
	m_versions.toXML(os);
	SetFeature(m_agsetId, "versions", os.str());

}


void DataModel::updateVersionInfo(const string& scribe, const string& wid)
{
	if ( m_lastVersion != "" || m_versions.size() == 0 ) {
		Version nv( m_versions.newVersionId(), "", scribe, wid, TRANSAG_VERSION_NO, "");
		m_versions.addVersion(nv);
		m_lastVersion = "";
	} else {
		if ( !scribe.empty() )
			m_versions.back().setAuthor(scribe);
		m_versions.back().setWid(wid);
	}

	ostringstream os;
	m_versions.toXML(os);
	SetFeature(m_agsetId, "versions", os.str());

}


/**
*  guess annotation file
* @param path annotation file path
* @param file_dtd (out) address of string in which dtd name will be returned if file requires one.
* @return	guessed file format, that can be later on passed to loadFromFile.
* @throw errmsg error message if save operation failed
*/

string DataModel::guessFileFormat(const string& path, string* file_dtd) throw (const char*)
{
/*
*  for the time beeing, has a bit rude approach (ie. it is "hard-coded")
*  TODO  get file format from XML file based on suffix rule + basic signature
*/
	std::map <std::string, std::string>::iterator it;

	// read file firsts lines to figure out file format
	ifstream fi(path.c_str());
	if ( fi.bad() ) {
		string msg = _("File open failed");
		msg += " : " ;
		msg += path;
		throw msg.c_str();
	}
	string tag, dummy, dtd;
	string format="";

//	Explorer_filter* filter = Explorer_filter::getInstance(false) ;
	Formats* formats = Formats::getInstance() ;

	fi >> tag;

	//> XML format: look for DOCTYPE value in order to determinate transcriberAG format
	if ( fi.good() && tag == "<?xml" )
	{
		getline(fi, dummy);
		fi >> tag;
		if ( fi.good() )
		{
			unsigned int pos=0;
			if ( tag == "<!DOCTYPE" )
			{
				fi >> tag >> dummy >> dtd;
			}
			//TODO deals here other XML format instead of using format class ?
//			else if ( tag == "<AudioDoc" )
//			{
//				tag = "AudioDoc" ;
//			}
			else if (tag[0] == '<' && tag[1] != '!')
			{
				StringOps(tag).trim("<>");
			}

			if (formats )
				format = formats->getFormatFromType(tag);

			if ( (format == "AG" || format == "TransAG")  && dummy == "SYSTEM" )
			{
				// get AG Subtype -> std AG / TransAG
				StringOps(dtd).getToken(pos, dummy, "\"");
				if ( file_dtd != NULL ) *file_dtd = dummy;

				string dtd_version ("") ;

				pos=dummy.find_first_of("-._");
				format = dummy.substr(0, pos);
				if ( pos != string::npos ) {
					pos = dummy.find_first_not_of("-._", pos);
					string dtd_version = dummy.substr(pos);
					pos = dtd_version.find(".dtd");
					if ( pos != string::npos )
						dtd_version = dtd_version.substr(0, pos);
					if ( dtd_version != _currentDTDVersion ) {
						format = "TransAG_compat";
					}
				}
			}
		}
	}
	// TranscriberAG text format
	else if ( (fi.good()) && (tag == "#") &&
				(fi >> tag) && (tag == "Generated") &&
				(fi >> tag) && (tag == "by") &&
				(fi >> tag) && (tag == "TranscriberAG") )
	{
		format = "" ; // cannot import pure text file

	}
	//> For other file, let's see the extension
	else
	{
		Glib::ustring tail = FileInfo(path).tail();
		tail = tail.substr(1, tail.size()-1) ;
		tail = tail.lowercase() ;
		if (formats )
			format = formats->getFormatFromType(tail);
	}

	fi.close();
	return format;
}

bool DataModel::isTAGformat(const string& format)
{
	return (format == "TransAG") || (format == "TransAG_compat") ;
}

Parameters* DataModel::getConventionsFromFormat(const string& format)
{
	Parameters* param = loadQualifierMapping(format) ;
	string convention_path = getConversionConventionFile(format, param) ;
	delete param ;

	if (convention_path.empty() || !Glib::file_test(convention_path, Glib::FILE_TEST_EXISTS))
		return NULL ;

	try {
		Parameters* p = new Parameters() ;
		p->load(convention_path) ;
		return p ;

	} catch ( const char* msg ) {
		MSGOUT << "getConventionsFromFormat error = " << msg << endl;
		return NULL ;
	} catch (...) {
		string msg = "getConventionsFromFormat:> Exception caught when loading conventions  file ";
		MSGOUT << msg << endl;
		return NULL ;
	}
}

/*
* fix temporary AGSetId
* return tmpfic path
*/
string DataModel::fixAGSetId(const string& path, const string& old_id, string& new_id, const string& outpath) throw (const char*)
{
	char tmpname[1024];

	if ( outpath.empty() ) {
		// get a unique temporary name
		strcpy(tmpname, g_get_tmp_dir());

		if ( ! G_IS_DIR_SEPARATOR(tmpname[strlen(tmpname)-1]) ) strcat(tmpname, G_DIR_SEPARATOR_S);
		strcat(tmpname,"TransAG.AXXXXXX");

		int fd = g_mkstemp(tmpname);
		if ( fd == -1 ) {
			string err = "fixAGSetId : " ;
			err += strerror(errno);
			err += " : " ; err += tmpname;
			throw err.c_str();
		}
		close(fd);
	} else strcpy(tmpname, outpath.c_str());


	if ( new_id.empty() ) {
		// set temporary new id
		new_id = (const char*)(FileInfo(tmpname).tail()+1);
	}

	//TRACE_D << "FIXING AGSETID  " << old_id << "  -> " << new_id << " for " << path << endl;

	ofstream fo(tmpname);
	ifstream fi(path.c_str());

	if ( ! (fi.good() && fo.good()) ) {
		string err = "fixAGSetId : " ;
		err += strerror(errno);
		err += " : " ; err += (fi.good() ? (const char*)tmpname : path.c_str());
		throw err.c_str();
	}

	string buf;
	unsigned long pos;
	int stage = 0;
	string id_marker = "id=\"" + old_id + "\"";
	string id_prefix = old_id + ":";
	while ( fi.good() && fo.good() ) {
		getline(fi, buf);
		switch ( stage ) {
		case 0:  // look for agsetId
			if ( (pos = buf.find(id_marker)) != string::npos ) {
				buf.replace(pos+4, old_id.length(), new_id);
				++stage;
			}
			break;
		case 1:
			pos = 0;
			while( (pos = buf.find(id_prefix, pos)) != string::npos ) {
				buf.replace(pos, old_id.length(), new_id);
			}
		}
		fo << buf << endl;
	}
	fi.close();
	fo.close();
	return string(tmpname);
}

/*
 * store meta-data from "info" file associated to signal file
 */
void DataModel::loadSignalInfo(const string& sigId, const string& path, const string& itemlist)
{
	if ( !FileInfo(path).exists() ) return;

	map<string, string> items;
	map<string, string>::iterator it;
	StringOps(itemlist).getDCSVItems(items) ;

	if ( items.size() > 0 ) {

		ifstream fi (path.c_str());
		string buf, key, val;
		if ( fi.good() ) {
			while ( fi.good() ) {
				getline(fi, buf);
				if ( buf[0] == '#' ) continue;
				if ( buf.empty() ) continue;
				istringstream is(buf);
				is >> key;
				StringOps op(key);
				getline(is, buf);
				for (it=items.begin(); it != items.end(); ++it ) {
					if ( op.contains(it->first) ) {
						SetFeature(sigId, (it->second.empty() ? it->first : it->second),
									 StringOps(buf).trim());
						break;
					}
				}
				/*
				if ( op.contains("type") ) SetFeature(sigId, "type", StringOps(buf).trim());
				if ( op.contains("category") ) SetFeature(sigId, "category", StringOps(buf).trim());
				if ( op.contains("source") ) SetFeature(sigId, "source", StringOps(buf).trim());
				if ( op.contains("date") && op.contains("start"))
						SetFeature(sigId, "date", StringOps(buf).trim());
				*/

			}
			fi.close();
		}
	}
}

/*******************************************************************************
 *								  IMPORT METHODS
 ******************************************************************************/


void DataModel::actualizeQualifierMapping(string format)
{
	m_qualifierMapping = loadQualifierMapping(format) ;
}

Parameters* DataModel::loadQualifierMapping(string format)
{
	if ( format == "TransAG" )
		return NULL;

	string name = StringOps(format).toLower() + ".xml" ;
	string folder = m_conventions.getDirectory() ;
//	Log::err() << "loadQualifierMapping: CONV-DIR=" << folder << endl ;
	FileInfo fold(folder) ;
	string path = fold.join("import_mapping") ;
	FileInfo pat(path) ;
	path = pat.join(name.c_str()) ;

	if ( !Glib::file_test(path, Glib::FILE_TEST_EXISTS) ) {
		TRACE << "# No qualifier mapping found for format " << format  << " - [" << path << "]" << std::endl ;
		return NULL ;
	}
	else {
		try {
			TRACE << "# Loading qualifier mapping from " << path << std::endl ;
			Parameters* param = new Parameters() ;
			param->load(path);
			return param ;
		}
		catch ( const char* msg ) {
			MSGOUT << "Sax error = " << msg << endl;
			return NULL ;
		}
		catch (...) {
			string msg = "Exception caught when qualifiers mapping file";
			return NULL ;
		}
	}
}

string DataModel::getConversionConventionFile(const string& format)
{
	return getConversionConventionFile(format, m_qualifierMapping) ;
}

string DataModel::getConversionConventionFile(const string& format, Parameters* qualifierMapping)
{
	if (!qualifierMapping)
		return "" ;

	//> get convention file specified in the mapping conversion file
	string name = qualifierMapping->getParameterValue("load", "convention,file") ;
	//> get the folder of transcriberAG convention
	string convDir =  conventions().getDirectory() ;
	FileInfo info(convDir) ;
	string path = info.join(name) ;

	//> if the file exists, OK
	if ( Glib::file_test(path, Glib::FILE_TEST_EXISTS) && Glib::file_test(path, Glib::FILE_TEST_IS_REGULAR) )
		return path ;
	else
		return "" ;
}

/*
 * Get language feature set in AGSet.
 * for backward compatibility with previous TranscriberAG versions, we also check
 * if AG graph feature is set for transcription_graph. if set, fix it in AGSet.
 */
string DataModel::fixLangBackwardCompatibility()
{
	string transcription_language = getTranscriptionProperty("lang") ;
	string agset_language = getAGSetProperty("lang") ;
	if (!transcription_language.empty())
	{
		setAGSetProperty("lang", transcription_language) ;
		unsetTranscriptionProperty("lang") ;
		return transcription_language ;
	}
	else {
		if (existGraphProperty("transcription_graph", "lang"))
			unsetTranscriptionProperty("lang") ;
		return agset_language ;
	}
}


void DataModel::setImportWarnings(vector<string> warnings)
{
	if (m_checker)
		m_checker->setImportWarnings(warnings) ;
	else
		TRACE << "import warning: checker module not initialized." << std::endl ;
}

} // namespace
