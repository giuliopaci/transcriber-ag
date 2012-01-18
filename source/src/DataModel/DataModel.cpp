/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  $Id$
 *
 *  @file DataModel.cpp
 *  @brief data model for Transcriber application
 *    implements the interface between Transcriber Editor and AG data model,
 *    implements data model management rules.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <glib/gstdio.h>

#include <fstream>
#include <sstream>
#include <set>


#include <ag/AGException.h>
#include <ag/agfio.h>

#include "DataModel.h"

#include "Common/globals.h"
#include "Common/util/Utils.h"
#include "Common/util/StringOps.h"
#include "Common/FileInfo.h"
#include "Common/VersionInfo.h"
#include "Common/util/FormatTime.h"

#include "DataModel/conventions/ModelChecker.h"

#ifdef ASSERT
#undef ASSERT
#endif

#define ASSERT(a,b) if (!(a)) { MSGOUT << " : Assertion failed" << std::endl;  return (b); }

#define TIME_FMT "%.3f"

#ifdef WIN32
	#define PATH_DELIM  "\\"
#else
	#define PATH_DELIM "/"
#endif


using namespace std;



static void trim(Glib::ustring& txt2)
{
	while (g_unichar_isspace(txt2[0]) ) txt2.erase(0,1);
	guint l = txt2.length() -1;
	while (l >= 0 && g_unichar_isspace(txt2[l]) ) --l;
	++l;
	txt2.erase(l);
}

namespace tag
{

const float DataModel::EPSILON=0.002;
const float DataModel::USER_INPUT = 2.0;
bool DataModel::_initEnvironDone=false;


/*========================================================================
 *
 *  Constructor / Destructor
 *
 *========================================================================*/

/**
 * default DataModel constructor
 */
DataModel::DataModel()
{
	m_inhibUndoRedo = true ;
	init();
	m_inhibUndoRedo = false ;
}

/**
 * DataModel constructor
 * @param corpus_name default corpus name
 */
DataModel::DataModel(const string& corpus_name)
{
	m_inhibUndoRedo = true ;
	init();
	if (!corpus_name.empty()) {
		initAGSet(corpus_name);
	}
	m_inhibUndoRedo = false ;
}

/**
 *  destructor
 */
DataModel::~DataModel()
{
	if ( ! m_keepAG )
		deleteAGElements();
	if (m_qualifierMapping)
		delete(m_qualifierMapping) ;
	if (m_checker)
		delete(m_checker);
}


void DataModel::activateModelChecker(bool activate)
{
	if (m_checker)
		m_checker->clear() ;

	if (activate)
	{
		m_checker = new ModelChecker(this) ;
		Log::out() << "~~~ ModelChecker [ON]" << std::endl ;
	}
	else
	{
		if (m_checker)
			delete(m_checker) ;
		Log::out() << "~~~ ModelChecker [OFF]" << std::endl ;
	}
}

/**
 * Emission of signal signalModelUpdated upon data model modification
 * @param type		Updated type
 * @param id		Updated id
 * @param upd		Update type
 */
void DataModel::emitSignal(const std::string& type, const std::string& id, UpdateType upd)
{
	if ( ! m_inhibateSignals )
		signalElementModified().emit(type, id, upd);
}

void DataModel::emitSignal(const std::string& id, UpdateType upd)
{
	if ( ! m_inhibateSignals )
	{
		if (!id.empty())
			signalElementModified().emit(GetAnnotationType(id), id, upd);
		else
			signalElementModified().emit("", "", upd);
	}
}


/**
 *  delete AG elements related to current data model instance
 */
void DataModel::deleteAGElements()
{
	TRACE << "deleteAGelement... starting... m_agsetId=" << m_agsetId << " ExistsAGSet=" << ExistsAGSet(m_agsetId) <<  std::endl ;
	try
	{
		if(m_timelineId != "" && ExistsTimeline(m_timelineId)) {
			DeleteTimeline(m_timelineId); m_timelineId = "";
		}

		// delete annotation graphs
		map<string, string>::iterator it;
		for ( it = m_agIds.begin(); it !=m_agIds.end(); it++ ) {
			if (ExistsAG(it->second)) {
				TRACE << "  DeleteAG -> " << it->second << endl;
				DeleteAG(it->second);
			}
		}

		m_agIds.clear();
//		m_agTrans = "";

		// delete AGSet if empty
		if (m_agsetId != "" && ExistsAGSet(m_agsetId))
		{
			set<AGId> ids = GetAGIds(m_agsetId);
			if(ids.size() == 0)
			{
				TRACE << "  >>> DeleteAGSet.  " <<m_agsetId <<  std::endl ;
				DeleteAGSet(m_agsetId);
				m_agsetId = "";
			}
			else {
				TRACE << "  REMAINING AGIDS = " << ids.size()  << endl ;
				set<AGId>::iterator itr;
				for (itr=ids.begin(); itr!= ids.end(); ++itr ) {
					TRACE  << "\t" << *itr << " ";
				}
				TRACE << endl;
			}
		}
		closing = true ;
		setUpdated(true) ;
		closing = false ;
	}
	catch(AGException& ex)
	{
		MSGOUT << "Caught AGException " << ex.error() << endl;
		throw ex.error().c_str();
	}
	TRACE << "deleteAGelement... done." << std::endl ;
}

/*
 * get graph type for given annotation graph id
 * @param id annotation graph id
 * @param is_graph_id MUST be false if id is anchor or annotation id, MUST be true if id is a graph id
 * @return graph type
 */
const string& DataModel::getGraphType(const string& id, bool is_graph_id)
{
	if ( !m_graphType.empty() ) return m_graphType;  // forced

	map<string, string>::iterator it;
	if ( is_graph_id ) {
		for ( it=m_agIds.begin(); it != m_agIds.end(); ++it )
			if ( it->second == id ) return it->first;
	} else {
		const string& agid = GetAGId(id);
		for ( it=m_agIds.begin(); it != m_agIds.end(); ++it )
			if ( it->second == agid ) return it->first;
	}
	return _novalue;
}


/*========================================================================
 *
 * get general infos on data model
 *
 *========================================================================*/

string DataModel::getCorpusName()
{
	return (m_tmpId.empty() ? m_agsetId : m_savId);
}

void DataModel::setCorpusVersion(const string& version)
{
	setAGSetProperty("version", version) ;
}

string DataModel::getCorpusVersion()
{
	string res = "" ;
	try
	{
		if ( ExistsFeature(m_agsetId, "version") ) {
			res = GetFeature(m_agsetId, "version") ;
			return res ;
		}
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << m_agsetId << endl;
	}
}

void DataModel::setCorpusName(const string& name)
{
	m_savId = name ;
	//> give a value to tmpId for forcing the change of name when saving file
	if (m_tmpId.empty() && m_savId!=m_agsetId)
		m_tmpId = m_agsetId ;
}

map<string, string> DataModel::getAGSetProperties()
{
	ASSERT(m_agsetId != "", (map<string, string>()));
	return GetFeatures(m_agsetId);
}


string DataModel::getAGSetProperty(const string& item, const string& defval)
{
	ASSERT(m_agsetId != "", "");
	if (ExistsFeature(m_agsetId, item))
	{
		return GetFeature(m_agsetId, item);
	}
	return defval;
}

void DataModel::setAGSetProperty(const string& item, const string& value)
{
	ASSERT(m_agsetId != "", void());
	if ( value.empty() )
		unsetAGSetProperty(item);
	else
		agSetFeature(m_agsetId, item, value);
	if ( item == "lang" ) m_vlang.clear();
	setUpdated(true) ;
}

void DataModel::unsetAGSetProperty(const string& item)
{
	ASSERT(m_agsetId != "", void());
	if ( ExistsFeature(m_agsetId, item) ) {
		agDeleteFeature(m_agsetId, item);
		setUpdated(true) ;
	}
}


//
// Get/Set Transcription property
//
void DataModel::setGraphProperty(const string& graphtype, const string& item, string value)
{
	bool before_inhib = m_inhibUndoRedo ;
	m_inhibUndoRedo = true ;
	map<string, string>::iterator it = m_agIds.find(graphtype);
	ASSERT(( it != m_agIds.end()), void());
	agSetFeature(it->second, item, value);
	setUpdated(true) ;
	m_inhibUndoRedo = before_inhib ;
}

void DataModel::unsetGraphProperty(const string& graphtype, const string& item)
{
	bool before_inhib = m_inhibUndoRedo ;
	m_inhibUndoRedo = true ;
	map<string, string>::iterator it = m_agIds.find(graphtype);
	ASSERT(( it != m_agIds.end()), void());
	agDeleteFeature(it->second, item);
	setUpdated(true) ;
	m_inhibUndoRedo = before_inhib ;
}

map<string, string> DataModel::getGraphProperties(const string& graphtype)
{
	map<string, string>::iterator it = m_agIds.find(graphtype);
	ASSERT(( it != m_agIds.end()), (map<string, string>()));
	return GetFeatures(it->second);
}

string DataModel::getGraphProperty(const string& graphtype, string item, string defval)
{
	map<string, string>::iterator it = m_agIds.find(graphtype);
	ASSERT(( it != m_agIds.end()), "");
	if (it != m_agIds.end() && ExistsFeature(it->second, item))
	{
		return GetFeature(it->second, item);
	}
	return defval;
}

bool DataModel::existGraphProperty(const string& graphtype, string item)
{
	map<string, string>::iterator it = m_agIds.find(graphtype);
	ASSERT(( it != m_agIds.end()), "");
	if (it != m_agIds.end() && ExistsFeature(it->second, item))
		return true ;
	return false ;

}

/*========================================================================
 *
 *  Signal management & info
 *
 *========================================================================*/

/**
 *	associates one (or more) signal to data model from given file
 */
std::vector<string> DataModel::addSignal(const string& path, const string& sigclass,
		const string& sigtype, const string& encoding, int nbtracks, int notrack_hint, bool init_graphs)
{
	std::vector<string> addedSigs ;

	if (m_agsetId.empty() || m_timelineId.empty())
		return addedSigs ;

	Log::trace() << "DataModel::addSignal " << path << " nb tracks=" << nbtracks << " notrack_hint=" << notrack_hint << endl;

	string fname = FileInfo(path).Basename();
	string tail = FileInfo(path).tail();
	if (!tail.empty())
	{
		fname = fname.substr(0, fname.length() - tail.length());
	}

	int cur_nbtrack = getNbTracks();

	for (int i=0; i< nbtracks; ++i)
	{
		char trackid[8];
		int notrack =  getNbTracks()+1;
		//create signal
		sprintf(trackid, "%d", (notrack_hint != -1 && nbtracks == 1 ? notrack_hint : notrack)) ;
		string sigid =  m_timelineId + string(":") + trackid ;	// signal id preference
		sigid = CreateSignal(sigid, fname, sigclass, sigtype, encoding, TRANS_ANCHOR_UNIT, trackid) ;
		addedSigs.push_back(sigid);
		if (sigclass=="video")
			signalCfg.enterVideoSignal(sigid, path, notrack-1) ;
		else if (sigclass=="audio")
			signalCfg.enterAudioSignal(sigid, path, notrack-1) ;
	}

	if (cur_nbtrack > 0 && m_agIds.size() > 0 && init_graphs)
	{
		// if graph(s) already initialized for some tracks, initialize it for added tracks
		std::map<std::string, std::string>::iterator itg = m_agIds.begin();
		const string& aid = getAnchorAtOffset(itg->second, 0, 0.0);
		const set<AnnotationId>& ids = GetOutgoingAnnotationSet(aid, "");
		if (ids.size() > 0)
		{
			initGraphsMainstream(cur_nbtrack);
		}
	}

	setUpdated(true) ;
	return addedSigs ;
}

/**
 *  dissociates all tracks having the same signal file url from data model
 */
bool DataModel::removeSignalFile(const string& signalUrl)
{
	ASSERT( m_agsetId != "", false);
	ASSERT( m_timelineId != "", false);

	std::vector<std::string> ids = signalCfg.getIds("") ;
	for (unsigned int i=0; i < ids.size(); ++i)
	{
		if (GetSignalXlinkHref(ids[i]) == signalUrl)
		{
			removeSignal(ids[i], false);
			m_updated = true;
		}
	}

	return true;
}

/**
* dissociates given signal track from data model
*/
bool DataModel::removeSignal(const string& signalId, bool alltracks)
{
	if (alltracks)
	{
		return removeSignalFile(GetSignalXlinkHref(signalId));
	}
	else
	{
		try
		{
			std::vector<string> ids = signalCfg.getIds("") ;
			// ICI check if has associated nodes & annotations
			int notrack;
			for(notrack = 0; notrack < (int)ids.size(); ++notrack)
			{
				if( signalCfg.getSigid(notrack) == signalId )
				{
					break;
				}
			}
			if(notrack < (int)ids.size())
			{
				std::map<string, int>::iterator it;
				set<string> todel;
				for(it = m_anchorTrack.begin(); it != m_anchorTrack.end(); ++it)
				{
					if( it->second == notrack)
					{
						const set<AnnotationId>& ids = GetOutgoingAnnotationSet(it->first, "");
						set<AnnotationId>::const_iterator it2;
						for(it2 = ids.begin(); it2 != ids.end(); ++it2)
						{
							// TODO ici a faire sur tous les graphes liés au signal !!
							if(GetAnnotationType(*it2) == mainstreamBaseType())
							{
								// also delete unanchored elements linked to current annotation
								vector<string> v;
								vector<string>::reverse_iterator itr;
								getLinkedElements(*it2, v, "", true);
								for(itr = v.rbegin(); itr != v.rend(); ++itr)
								{
									deleteElement(*itr, false);
								}
							}
							else
							{
								agDeleteAnnotation(*it2);
							}
						}
						const set<AnnotationId>& id2s = GetIncomingAnnotationSet(it->first, "");
						for(it2 = id2s.begin(); it2 != id2s.end(); ++it2)
						{
							agDeleteAnnotation(*it2);
						}
						todel.insert(it->first);
					}
				}

				set<string>::iterator itd;
				for(itd = todel.begin(); itd != todel.end(); ++itd)
				{
					it = m_anchorTrack.find(*itd);
					if(it != m_anchorTrack.end())
					{
						m_anchorTrack.erase(it);
					}
					agDeleteAnchor(*itd);
				}

				DeleteSignal(signalId);
				signalCfg.deleteSignal(signalId) ;
			}
		}
		catch (AGException& ex)
		{
			MSGOUT << "Caught AGException " << ex.error() << endl;
			ostringstream msg;
			msg << "Caught exception In DataModel::removeSignal " << ex.error();
			throw msg.str().c_str();
		}
	}
	return true;
}

/**
 * store signal duration in data model (and accordingly adjust last anchor offset, if AGSet has been initialized)
 */
void DataModel::setSignalDuration(float dur_in_secs, bool recheck)
{
	m_signalLength = dur_in_secs;
	if ( ! m_timelineId.empty() )
	{
		agSetFeature(m_timelineId, "duration", FormatTime(dur_in_secs, false, true));

		if (recheck)
		{
			// hint : when creating new graph, it may be happen that last anchor offset
			// isn't correct if signal duration wasn't set at initGraphMainstream timeline
			// -> so we set it correctly here.
			map<string, string>::iterator itg;
			for ( itg = m_agIds.begin(); itg != m_agIds.end(); ++itg )
			{
				// if last anchor offset = UNDEFINED_LENGTH -> then wasn't set at graph creation, so set it now
				const list<AnchorId>& aids = GetAnchorSetByOffset(itg->second, UNDEFINED_LENGTH, EPSILON);
				if ( aids.size() == 0 ) break; //  was set -> nothing more to do
				list<AnchorId>::const_iterator ita;
				for ( ita = aids.begin(); ita != aids.end(); ++ita )
					agSetAnchorOffset(*ita, m_signalLength);
			}
		}
	}
	setUpdated(true) ;
}

/**
*  return signal duration in seconds
*/
float DataModel::getSignalDuration(bool fromAnchor)
{
	if (m_signalLength == UNDEFINED_LENGTH && m_timelineId != "")
	{

		if (ExistsFeature(m_timelineId, "duration") && !fromAnchor)
		{
			const string& dur = GetFeature(m_timelineId, "duration");
			int sec, min, hour;
			sscanf(dur.c_str(), "%02d:%02d:%02d", &hour, &min, &sec);
			m_signalLength = (hour * 3600) + (min * 60) + sec;
		}
		else
		{
			map<string, string>::iterator itg;
			for ( itg=m_agIds.begin(); itg != m_agIds.end(); ++itg )
			{
				if ( ! m_conventions.isContinuousGraph(itg->first) )
					continue;
				m_signalLength = 0.0;

				// retrieve highest anchor offset
				const list<AnchorId>& anchors = GetAnchorSet(itg->second);
				list<AnchorId>::const_iterator it;
				float f;
				for (it=anchors.begin(); it != anchors.end(); ++it)
				{
					if (GetAnchored(*it) )
					{
						f = GetAnchorOffset(*it);
						if (f > m_signalLength && f != UNDEFINED_LENGTH)
							m_signalLength = f;
					}
				}
				if (m_signalLength > 0.0)
					setSignalDuration(m_signalLength);
				else
					m_signalLength = UNDEFINED_LENGTH;
			}
		}
	}
	return m_signalLength;
}

/*
 *  get infos on associated signals
 */
int DataModel::getNbTracks()
{
	if ( signalCfg.isSingleSignal() )
		return 1;
	else
		return signalCfg.getNbSignals("audio") ;

}

string DataModel::getSignalFileURL(int notrack)
{
	ASSERT(notrack >= 0 && notrack < getNbTracks(), "");
	return GetSignalXlinkHref(signalCfg.getSigid(notrack));
}

std::map<string,string> DataModel::getSignalFileNames(string mimeclass)
{
	std::map<string,string> res ;
	std::vector<std::string>::const_iterator it ;
	const std::vector<string>& ids = signalCfg.getIds("") ;
	for (it=ids.begin(); it!=ids.end(); it++)
	{
		string mim = GetSignalMimeClass(*it) ;
		if (mim == mimeclass) {
			string path ;
			path = GetSignalXlinkHref(*it) ;
			TRACE_D << "DataModel::getSignalFileURLs " << path << std::endl ;
			res[*it] = path ;
		}
	}
	return res ;
}

std::map<string,string> DataModel::getSignalFilePaths()
{
	return signalCfg.getIdPaths() ;
}


string DataModel::getSignalId(int notrack)
{
	ASSERT(notrack >= 0 && notrack < getNbTracks(), "");
	return signalCfg.getSigid(notrack) ;
}

/**
 * get track no for given signal id
 */
int DataModel::getSignalNotrack(const string& signalId)
{
	try {
		Track t = signalId ;
		const string& track = GetSignalTrack(t) ;
		int i = atoi(track.c_str()) ;
		/* track numbered from 1 to n in AG, from 0 to n-1 in DataModel -> return i -1 */
		return (i - 1);
	}
  	catch (AGException& e)	{
		MSGOUT << "Caught AGException : " << e.error() << endl;
		return -1 ;
  	}
}


string DataModel::getSignalProperty(int notrack, const string& item)
{
	ASSERT(notrack >= 0 && notrack < getNbTracks(), "");
//	TRACE_D << "getSignalProperty " << item << " for " << m_signalIds[notrack]  << " = " << getSignalProperty(m_signalIds[notrack], item)<< endl;
	return getSignalProperty( signalCfg.getSigid(notrack), item );
}

string DataModel::getSignalProperty(const string& id, const string& item)
{
  	try {
		if ( ExistsSignal(id) )
			if ( ExistsFeature(id, item) )
			return GetFeature(id, item);
  	}
  	catch (AGException& e)	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << " item= " << item << endl;
	}
	return "";
}

string DataModel::getSignalFileClass(int notrack)
{
	ASSERT(notrack >= 0 && notrack < getNbTracks(), "");
	return GetSignalMimeClass( signalCfg.getSigid(notrack) );
}


/**
* set signal property for given track
* @param notrack track no
* @param propertyName property item
* @param propertyValue new property value
*/
void DataModel::setSignalProperty(int notrack, const std::string& item, const std::string& value)
{
	ASSERT(notrack >= 0 && notrack < getNbTracks(), void());
	setSignalProperty(signalCfg.getSigid(notrack), item, value);
}

/**
* set signal property for given track id
* @param id track id
* @param propertyName property item
* @param propertyValue new property value
*/
void DataModel::setSignalProperty(const string& id, const std::string& item,
		const std::string& value)
{
	if ( ExistsSignal(id) )
	{
		agSetFeature(id, item, value);
//		TRACE_D << "setSignalProperty " << item << " for " << id<< " =  " << value<<  endl;
		setUpdated(true) ;
	}
	else
	{
		MSGOUT << "Invalid signal ID : " << id << endl;
	}
}

/*
 * return signals data as list of maps - one map per signal file
 */
void DataModel::getSignalsProperties(
		std::map <string, std::map<std::string, std::string> >& signals)
{
	const set<SignalId>& ids = GetSignals(m_timelineId);
	set<SignalId>::const_iterator it;
	map<string, int > channels;

	for (it=ids.begin(); it != ids.end(); ++it)
	{
		const string& url=GetSignalXlinkHref(*it);
		if (channels.find(url) == channels.end() )
			channels[url] = 1;
		else
			channels[url]++;
	}

	for (it=ids.begin(); it != ids.end(); ++it)
	{
		const string& url=GetSignalXlinkHref(*it);
		if (channels[url] > 0)
		{
			signals[*it] = GetFeatures(*it);
			std::map<string, string>& info = signals[*it];
			// remove path_hint info
			std::map<string, string>::iterator todel = info.find("path_hint");
			if (todel != info.end() )
				info.erase(todel);
			todel = info.find("speaker_hint");
			if (todel != info.end() )
				info.erase(todel);
			info["file"] = url;
			info["format"] = GetSignalMimeType(*it);
			info["encoding"] = GetSignalEncoding(*it);
			info["channels"] = (channels[url] == 1 ? "Mono" : "Stereo");
			info["length"] = getElementProperty(m_timelineId, "duration");
			channels[url] = 0;
		}
	}
}

/*========================================================================
 *
 *  main transcription graph initialisation
 *
 *========================================================================*/

/**
 * create new AGSet and associated timeline
 * @param corpus_name corpus name
 */
void DataModel::initAGSet(const string& corpus_name)
{
	deleteAGElements();

	m_defaultCorpusName = corpus_name;

	bool before_inhib = m_inhibUndoRedo ;
	m_inhibUndoRedo = true ;

	try
	{
		if ( ExistsAGSet(m_defaultCorpusName) )
		{
			m_tmpId = DataModel::mktmpAGSetId();
			m_savId = m_defaultCorpusName;
			m_agsetId = CreateAGSet(m_tmpId);
		}
		else
			m_agsetId = CreateAGSet(m_defaultCorpusName);

		m_timelineId = CreateTimeline(m_agsetId);
		Log::trace() << "Create AGSet " << m_agsetId <<  " Timeline=" << m_timelineId  << endl;
		setAGSetProperty("TransAG_version", TRANSAG_VERSION_NO);

		m_inhibUndoRedo = before_inhib ;
	}
	catch (AGException& ex)
	{
		m_inhibUndoRedo = before_inhib ;
		MSGOUT << "Caught AGException " << ex.error() << endl;
		ostringstream msg;
		msg << "Caught exception In DataModel::initAGSet " << ex.error();
		throw msg.str().c_str();
	}
}

/**
 *	 init transcription graph for current conventions and transcription parameters
 *  @param lang transcription language
 *  @param scribe transcription author
 *  @return id of created annotation graph
 *
 *  @note
 *    graph properties are initialized from given parameters, current tool version
 *    and references to annotation conventions applicable for current language
 */

void DataModel::initAnnotationGraphs(const string& gtype, const string& lang, const string& scribe, bool reset_graphs)
{
	ASSERT( m_agsetId != "", void());

	if ( ! m_conventions.loaded() ) throw "No annotation conventions loaded !";

	TRACE_D << "DataModel::initAnnotationGraphs lang=" << lang << " scribe=" << scribe << endl;

	if ( gtype.empty() ) {
		Version version("1.0", "today", scribe, "", TRANSAG_VERSION_NO, "Created");
		m_versions.addVersion(version);
	}


	setAGSetProperty("TransAG_version", TRANSAG_VERSION_NO );
	setAGSetProperty("scribe", scribe );
	setAGSetProperty("lang", lang );

	if (reset_graphs)
		m_agIds.clear() ;

	Conventions::GraphDescriptorIter itg;
	bool ok=false;

	for (itg = m_conventions.getGraphDescriptors().begin(); itg != m_conventions.getGraphDescriptors().end(); ++itg )
	{
		if ( !gtype.empty() && itg->first != gtype )
			continue;

		TRACE_D << "************************************************* Initializing graph " << itg->first << std::endl ;

		ok=true;
		string agid = CreateAG(m_agsetId, m_timelineId);

		m_agIds[itg->first] =  agid;
		TRACE_D << "DataModel::> graph added for type " << itg->first << " : " << agid << std::endl ;

		setGraphProperty(itg->first, "type", itg->second.type);
		setGraphProperty(itg->first, "graphtype", itg->first);

//		if ( itg->first.compare(0, 10, "transcript") == 0 ) {
//			if ( !lang.empty() )
//				setGraphProperty(itg->first, "lang", lang);
//		}
	}
	if (ok)
		initGraphsMainstream(0, gtype);
	setUpdated(true) ;
}

/**
 * (internal) init annotation graph main stream elements
 * @param graph annotation graph
 * @param start_track signal track no from which graph is to be initialized
 */
void DataModel::initGraphsMainstream( int start_track, const string& gtype)
{
	map<string, string>::iterator itg;
	TRACE_D << "initGraphsMainstream  m_signalLength=" << m_signalLength << " / UNDEFINED_LENGTH=" << UNDEFINED_LENGTH << endl;

	for (itg = m_agIds.begin(); itg != m_agIds.end(); ++itg )
	{
		if ( !gtype.empty() && itg->first != gtype )
			continue;

		const string& graphId = itg->second;
		const string& graphtype = itg->first;
		const vector<string>& mainstream=m_conventions.getMainstreamTypes(graphtype);

		int notrack;

		//> For discret graph, don't create default annotation nor end/start anchor
		if (m_conventions.isContinuousGraph(graphtype))
		{
			//> Otherwise, initialize
			for (notrack = start_track; notrack < getNbTracks(); ++notrack)
			{
				string signaltype = getSignalFileClass(notrack) ;
				if (m_conventions.appliesToSignalClass(graphtype, signaltype))
				{
					//create first and last anchor
					const string& astart = createAnchorIfNeeded(graphId, notrack, 0.0, true);
					const string& aend = createAnchorIfNeeded(graphId, notrack, m_signalLength,	true);
				//	m_lastAnchor[graph] = aend; // store last anchor in case signal length undefined.
					setAnchorTrackHint(notrack);

					//create default annotations
					vector<string>::const_iterator it;
					for ( it = mainstream.begin(); it != mainstream.end(); ++it)
					{
						const string& type = *it ;

						// create element
						const string& id = createAnnotation(graphId, type, astart, aend) ;

						// if has subtype, set corresponding feature
						std::vector<string> subtypes ;
						if ( type==mainstreamBaseType(graphtype) && conventions().mainstreamHasSubtypes(type, graphtype, subtypes) )
						{
							setElementProperty(id, "subtype", *(subtypes.begin()), false) ;
						}
 					}
					if ( graphtype == "transcription_graph"	&& getSpeakerHint(notrack) == "" )
					{
						// create a default speaker for current track
						const string& lang = getAGSetProperty("lang");
						const Speaker& speaker = m_speakersDict.defaultSpeaker(lang);
						m_speakersDict.addSpeaker(speaker);
						setSpeakerHint(speaker.getId(), notrack);
					}
				}
			} //end all tracks
		}
	} //end all graphes
}

/**
 * create annotation in graph and set its default features from convention specifications
 * @param graph annotation graph
 * @param type annotation type
 * @param start annotation start offset
 * @param end annotation end offset
 * @param emit_signal true if element modified signal to be emitted.
 */

string DataModel::createAnnotation(const string& graphId, const string& type,
		const string& start, const string& end, bool emit_signal)
{
	const string& id = agCreateAnnotation(graphId, start, end, type);
//	TRACE << "(DataModel::createAnnotation)  type=" << type << " id=" << id << " start=" << start << " end=" << end << endl;

	//TODO do it with better elegance
	vector<string> subtypes;
	const string& graphtype = getGraphType(graphId,true);
	if ( m_conventions.mainstreamHasSubtypes(type, graphtype, subtypes) )
			agSetFeature(id, "subtype", subtypes.front()) ;  // default subtype

	const map<string, string>& items = m_conventions.getTypeFeatures(type);
	if ( items.size() > 0 )
	{
		int notrack = getAnchorSignalTrack(start);
		map<string, string>::const_iterator it;
		map<string, string>::iterator it2;
		for (it= items.begin(); it != items.end(); ++it)
		{
			string hint = getHint(type, it->first, notrack);
			if ( hint.empty() )
				hint = it->second;
			if ( ! hint.empty() )
			{
				if (hint == "$speaker")
				{
					// add default speaker to speakers dictionary
					const string& lang = getAGSetProperty("lang") ;
					const Speaker& speaker = m_speakersDict.defaultSpeaker(lang) ;
					m_speakersDict.addSpeaker(speaker) ;
					hint = speaker.getId() ;
					setSpeakerHint(hint, notrack) ;
				}
//				TRACE << " --> set hint " << it->first << " = " << hint << endl;
				agSetFeature(id, it->first, hint) ;
			}
		}
	}
	setUpdated(true) ;
	if (emit_signal)
		emitSignal(id, INSERTED);
	return id;
}


/**
 * create annotation in graph with given features
 * @param graph annotation graph
 * @param type annotation type
 * @param start annotation start offset
 * @param end annotation end offset
 * @param props annotation features
 * @param emit_signal true if element modified signal to be emitted.
 */

string DataModel::createAnnotation(const string& graphId, const string& type,
		const string& start, const string& end, const map<string, string>& props, bool emit_signal)
{
	string id = agCreateAnnotation(graphId, start, end, type);
	map<string, string>::const_iterator it;

	for (it= props.begin(); it != props.end(); ++it)
	{
		agSetFeature(id, it->first, it->second);
	}
	setUpdated(true) ;
	if (emit_signal)
		emitSignal(id, INSERTED);
	return id;
}

/*========================================================================
 *
 *  Insert data to graph
 *
 *========================================================================*/

/**
 * check segment insertion rules
 * @param type segment type to be inserted
 * @param notrack corresponding signal track
 * @param start_offset inserted segment start offset in signal / -1 if not to be checked
 * @param stop_offset inserted segment end offset in signal / <= 0 if not to be checked
 * @param prevId id of existing annotation upon which new segment is to be aligned (type != mainstreamBaseType()) or after which it is to be inserted (type = mainstreamBaseType())
 * @param diagnosis (returned) eventual check failure diagnosis
 * @return true if segment can be deleted, else false
 *
 * return true if valid, else false
 *  diag contains eventual error messages or warnings
 */

bool DataModel::checkInsertionRules(const string& type, int notrack,
										float start_offset, float stop_offset,
										const string& prevId, int& order,
										string alignCandidate, string& diag)
{
	char msg[120];
	bool ok = true;

	//	TRACE_D << " IN checkInsertionRules   PREVID = " << prevId  << " (" << GetAnnotationType(prevId) << ")" << endl;
	msg[0] = 0;
	order = 0;
	diag = "";

	const string& graphtype = m_conventions.getGraphtypeFromType(type);
	if ( !m_conventions.isAnchoredType(type) && type!=mainstreamBaseType(graphtype) )
		return true;

	if (start_offset >= 0.0
			&& stop_offset > 0.0
			&& (stop_offset - start_offset ) < m_conventions.minSegmentSize(graphtype) )
	{
		sprintf(msg, _("segment size must be > %f secs"), m_conventions.minSegmentSize(graphtype));
		diag = msg ;
		return false ;
	}

	if (prevId.empty() )
	{
		// TODO
		// if previd == "" -> find prevId by offset
		TRACE_D << "NEED TO FIND PREVID BY OFFSET" << endl;
	}

	if (prevId != "")
	{
		// -- We want to create a parent on an existing element ? check for specific rules
		if ( ! alignCandidate.empty() && conventions().getAlignmentType(type) == getElementType(alignCandidate) )
		{
			int prevOrder = getOrder(alignCandidate);
			const string& realStartAnchor = GetStartAnchor(alignCandidate) ;
			// we need to be at the same offset of the existing element
			if ( !realStartAnchor.empty() && GetAnchored(realStartAnchor) && (GetAnchorOffset(realStartAnchor)==start_offset) )
			{
				bool canPlug = true ;
				set<AnnotationId> incomings = GetIncomingAnnotationSet(realStartAnchor, type) ;
				set<AnnotationId>::iterator it ;
				// check if no element of the type we want to create already exists
				for (it=incomings.begin(); canPlug && it!=incomings.end(); it++)
				{
					if ( getOrder(*it) == prevOrder )
						canPlug = false ;
				}
				if (canPlug)
				{
					order = prevOrder ;
					return true ;
				}
			}
		}

		const string& startAnchor = getStartAnchor(prevId);
		const string& endAnchor = getEndAnchor(prevId);
		int prevOrder = getOrder(prevId);

		if (startAnchor == "")
		{
			return true;
		}

		// -1 -> to be aligned on previd
		if (start_offset >= 0)
		{
			// Check that start & stop offset are coherent with prevID offset
			if (start_offset < GetAnchorOffset(startAnchor)
					|| (!endAnchor.empty() && start_offset
							> GetAnchorOffset(endAnchor)))
			{
				diag = _("signal offset doesn't match current insertion position");
				return false;
			}
			// -- Check new segment insertion would not cause neighbour segments to
			//> See segment before
			ok = checkRemainingMinsize(m_conventions.mainstreamBaseType(graphtype), prevOrder, endAnchor, start_offset, diag);
			//> See segment after
			if (ok && stop_offset <= 0.0)
				ok = checkRemainingMinsize(m_conventions.mainstreamBaseType(graphtype), prevOrder, startAnchor, start_offset, diag);
			else if (ok)
				ok = checkRemainingMinsize(m_conventions.mainstreamBaseType(graphtype), prevOrder, startAnchor, stop_offset, diag);

			if ( !ok)
				return false;
		}
		else if (type == m_conventions.mainstreamBaseType(graphtype) )
		{
			diag = _("No signal offset defined for segment");
			return false;
		}

//		if (m_conventions.getAlignmentType(type) != "")
		if ( type!=mainstreamBaseType(graphtype) && type!=segmentationBaseType(graphtype) )
		{
			//string alignId = findAlignementId(type, prevId);
			//order = getOrder(prevId) ;
			//m_conventions.getConfiguration("transcription_graph,overlap,alignment") == "true"

			// --> New element must be aligned on base segment type
			// --> We first check if another annotation of same type exists at startAnchor offset

			const string& alignType = conventions().getAlignmentType(type);
			const string& alignId = getParentElement(prevId, alignType, false, false );
			if ( alignId != "" )  {
				int max_overlap = m_conventions.maxOverlap(type);
				const set<AnnotationId>& previous = GetOutgoingAnnotationSet(GetStartAnchor(alignId), type);
				set<AnnotationId>::const_iterator it;
	//			const list<AnnotationId>& previous = GetAnnotationSetByOffset(graphId, GetAnchorOffset(startAnchor)+EPSILON, type);
	//			list<AnnotationId>::const_iterator it;
				int notrack = m_anchorTrack[startAnchor];
				int cnt;

				start_offset = GetStartOffset(prevId);
				if (previous.size() > 0)
				{
					order=0;
					cnt = 0;
					for (it = previous.begin(); it != previous.end(); ++it)
					{
						if (getElementSignalTrack(*it) == notrack)
						{
							++cnt;
							int o = getElementProperty(*it, "order", 0);
							if (o > order)
								order = o;
						}
					}

					cnt = order + 1;

					// -- Too many overlapping segments at offset
					if (cnt > max_overlap)
					{
						sprintf(msg, _("A %s is already set at current %s"), type.c_str(), alignType.c_str());
						ok = false;
					}
					else if (cnt > 0)
					{
	//					if ( order == 0 )
	//						order = 1; // 1st overlap
						sprintf(msg, _("Do you want to overlap a %s ?"), _(type.c_str()));
						++order; // next "available" order
					}
				} else {
					if ( max_overlap > 0 ) {
						// check if in the middle of an overlapped annot
						const string& p_id = getParentElement(prevId, type, false, false);
						if ( !p_id.empty() ) {
							const set<AnnotationId>& pids = GetOutgoingAnnotationSet(GetStartAnchor(p_id), type);
							if ( pids.size() > max_overlap ) {
								ok = false;
								// cannot add within overlapped branch unless it can terminate branch
								if ( endAnchor == GetEndAnchor(p_id) ) {
									const string& childtype = conventions().getChildType(type);
									const set<AnnotationId>& prev = GetIncomingAnnotationSet(endAnchor, childtype);
									if ( prev.size() > 1 ) {
										set<AnnotationId>::iterator itp;
										float poffset= getStartOffset(prevId);
										for (itp=prev.begin(); itp != prev.end(); ++itp) {
											if ( *itp != prevId ) {
												if ( getStartOffset(*itp) > poffset ) break;
											}
										}
										ok = ( itp == prev.end() );
									}
								}
								if ( ! ok )
									sprintf(msg, _("Can't add  %s-type segment within overlapped branch"), type.c_str());
							}
						}
					}
				}
			}
		}
	}

	diag = msg;
	return ok;
}


//
// TODO -> candidat a la suppression

bool DataModel::checkTimestampRules(float start_offset, float stop_offset, const string& id, string& diag)
{
	char msg[120] ;
	bool ok = true ;

	const string& type = getElementType(id) ;
	string graphtype = getGraphType(id, false) ;

	string align = id ;
	if ( !isMainstreamType(type, graphtype ) )
		align = getParentElement(id, mainstreamBaseType(graphtype), true) ;

	//> -- Check selection
	if (start_offset >= 0.0 && stop_offset > 0.0
			&& (stop_offset - start_offset ) < m_conventions.minSegmentSize(graphtype) )
	{
		diag = _("segment size must be > %f secs") + number_to_string(m_conventions.minSegmentSize(graphtype)) ;
		return false;
	}

	//> -- Check previous element
	if ( start_offset >= 0.0 )
	{
		int prevOrder = getOrder(align) ;
		const string& anchor = GetStartAnchor(align) ;
		// -- Check new segment insertion would not cause neighbour segments to
		//> See segment before
		ok = checkRemainingMinsize(m_conventions.mainstreamBaseType(graphtype), prevOrder, anchor, start_offset, diag);
	}

	if ( stop_offset > 0.0 && ok)
	{
		string nextId = getNextElementId(align) ;
		const string& nextStartAnchor = GetStartAnchor(nextId) ;
		float nextStartAnchorOffset = GetAnchorOffset(nextStartAnchor) ;
		int prevOrder = getOrder(nextId) ;

		if ( stop_offset > nextStartAnchorOffset )
		{
			diag = _("signal offset doesn't match current insertion position");
			return false;
		}

		//> See segment after
		ok = ok && checkRemainingMinsize(m_conventions.mainstreamBaseType(graphtype), prevOrder, nextStartAnchor, stop_offset, diag);
	}

	return ok ;
}

/**
 * check segment insertion rules
 * @param type segment type to be inserted
 * @param notrack corresponding signal track
 * @param start_offset inserted segment start offset in signal / -1 if not to be checked
 * @param stop_offset inserted segment end offset in signal / <= 0 if not to be checked
 * @param inactive_to_update inactive neighbour segment that would be modified by insertion
 * @param diagnosis (returned) eventual check failure diagnosis
 * @return true if segment can be deleted, else false
 *
 * return true if valid, else false
 *  diag contains eventual error messages or warnings
 */
bool DataModel::checkBackgroundInsertionRules(int notrack, float start_offset, float stop_offset, string& inactive_to_update, string& diag)
{
	char msg[120];
	bool ok = true;

	msg[0] = 0;
	diag = "";
	string type = "background";
	const string& graphtype = m_conventions.getGraphtypeFromType(type);

	if ( (start_offset >= 0.0 && stop_offset > 0.0 && (stop_offset - start_offset ) < m_conventions.minSegmentSize(graphtype))
			|| start_offset<=0.0 && stop_offset<0.0 )
	{
		sprintf(msg, _("background size must be > %f secs"), m_conventions.minSegmentSize(graphtype));
		diag = msg;
		return false;
	}

	inactive_to_update = "" ;

	if ( ! hasAnnotationGraph(graphtype) )
		return false;

	//> CHECK IF WE'RE NOT OVERLAPPING
	// get all background that have at least one of theirs frontiers inside selection
	std::vector<string> res ;
	getSegmentsInRange(m_agIds[graphtype], m_conventions.mainstreamBaseType(graphtype), notrack, start_offset, stop_offset, res, 2) ;
	vector<string>::iterator itv;
	int nb_inactive = 0;
	for (itv = res.begin(); itv != res.end(); )
	{
		// for inactive background
		if ( getElementProperty(*itv, "type", "none") == "none" ) {
			++nb_inactive;
			// if it exists keep id of background corresponding to selection
			if ( GetStartOffset(*itv) == start_offset && GetEndOffset(*itv) == stop_offset )
				inactive_to_update = *itv;
			// now let's erase it from vector because we only search active background
			itv = res.erase(itv);
		}
		else
			++itv;
	}

	// if we have active background overlapped (totally or partially) block
	if ( res.size() != 0 ) {
		diag = _("would overlap existing background") ;
		MSGOUT << "checkBackgroundInsertionRules:> " << diag << endl ;
		return false;
	}
	// else it's ok except if we have more than one inactive background inside selection
	// ==> means a o.O bug
	else {
		if (nb_inactive>1) {
			diag = _("Problem while inserting background: aborted") ;
			MSGOUT << "checkBackgroundInsertionRules:> " << diag << endl ;
			return false;
		}
	}

	// get current background at position
	AnnotationId prevId = getByOffset("background", start_offset, notrack) ;

	// except if we're updating an inactive background, let's proceed
	if (prevId != "" && prevId != inactive_to_update )
	{
		const string& startAnchor = GetStartAnchor(prevId);
		const string& endAnchor = GetEndAnchor(prevId);

		if (startAnchor == "")
			return true;

		if (start_offset >= 0)
		{
			//  Check new segment insertion would not cause neighbour segments to
			//  become too small
			const string& btype = getElementProperty(prevId, "type", "none");
			if ( ! (btype.empty() || btype == "none")  )
			{
				ok = checkRemainingMinsize("background", -1, endAnchor, start_offset, diag) ;
				if (ok && stop_offset < 0)
					ok = checkRemainingMinsize("background", -1, startAnchor, start_offset, diag);
				else if (ok)
					ok = checkRemainingMinsize("background", -1, startAnchor, stop_offset, diag);
			}
			if ( !ok)
				return false;
		}
	}

	diag = msg;
	return ok;
}



/**
 * after aligned overlapping turns have been terminated at startId start, the
 * remaining graph branch must be set to 0 order
 * @param startId id of base segment on which start anchor overlapping turns have been terminated
 * @param endAnchor previous end anchor of overlapping turns
 *
 */
bool DataModel::resetOrderOnBranch(const string& startId,
		const string& endAnchor, vector<UpdateAction>& updates)
{
	string anchor=GetStartAnchor(startId);
	set<AnnotationId>::const_iterator it;
	set<AnnotationId> upd;
	const string& graphtype = getGraphType(startId);

	while (anchor != "" && anchor != endAnchor)
	{
		const set<AnnotationId>& ids =GetOutgoingAnnotationSet(anchor);
		anchor="";
		for (it = ids.begin(); it != ids.end(); ++it)
		{
			upd.insert(*it);
			if (GetAnnotationType(*it) == m_conventions.mainstreamBaseType(graphtype) )
			{
				if (anchor.empty() )
				{
					anchor = GetEndAnchor(*it);
				}
				else
				{
					MSGOUT << "Warning : resetOrderOnBranch from " << startId
							<< ": more than one " << m_conventions.mainstreamBaseType(graphtype)
							<< " going out of " << anchor << " !!" << endl;
				}
			}
		}
		if (anchor == "")
		{
			MSGOUT << "Error : resetOrderOnBranch from " << startId
					<< ": endAnchor " << endAnchor << "never reached !!"
					<< endl;
			return false;
		}

	}
	for (it = upd.begin(); it != upd.end(); ++it)
	{
		if ( ExistsFeature(*it, "order") ) {
			if ( getOrder(*it) > 0 ) {
				const string& type = GetAnnotationType(*it);
				if ( isMainstreamType(type, graphtype) )
					updates.push_back(UpdateAction(*it, UPDATED));
			}
			agDeleteFeature(*it, "order");
		}
	}
	return true;
}

/**
 * set overlapping segments end to given anchor :
 * @param parent_id parent segment to terminate at align_id start
 * @param align_id  id of segment where overlapping turns must be terminated
 * @bool emit_signal emit "SEGMENT_RESIZED signal"
 * @return previous end anchor for resized parent_id.
 *
 * @note :
 *  eg: terminate overlapping turns on segment starting at a4 anchor
 *    -> will cause the following graph rearrangments
 *
 *      ___ a2___a3___                     ___ a2
 *   a1/              \ a5     ==>      a1/    \a4__a3__ a5
 *     \______a4______/                   \____/
 *
 *
 *  eg: terminate overlapping turns on segment starting at a3 anchor
 *    -> will cause the following graph rearrangments
 *
 *      ______________                     ______
 *   a1/              \ a5     ==>      a1/      \a3__a4__a5
 *     \__a2__a3__a4__/                   \__a2__/
 *
 *
 *  BUT: terminate overlapping turns on segment starting at a5 anchor
 *    -> will be refused because mixing the 2 graph branches may not
 *       be desirable.
 *
 *      ___ a2___a3___                     ___a2
 *   a1/              \ a7     ==>      a1/     \a5__a3__a6__a7
 *     \__a4__a5__a6__/                   \__a4_/
 *
 */

bool DataModel::terminateOverlappingBranches(const string& parent_id,
		const string& align_id, vector<UpdateAction>& updates)
{
	const string& ptype = GetAnnotationType(parent_id);
	int	order = getOrder(parent_id);
	const string& prev_end = GetEndAnchor(parent_id);
	const string& align_anchor = GetStartAnchor(align_id);
	float offset = GetAnchorOffset(align_anchor);
	set<AnnotationId>::const_iterator it;
	const string& graphtype = getGraphType(parent_id);
	bool ok = true;

	if ( GetStartAnchor(parent_id) == align_anchor )
	{
		MSGOUT << " terminateOverlapping would lead to self-loop segment for " << parent_id  << " align_id=" << align_id << endl;
//		string msg = _("Alignment error for ") + ptype;
//		throw msg.c_str();
		return false;
	}

	if (m_conventions.isHigherPrecedence(ptype, m_conventions.mainstreamBaseType(graphtype), graphtype) )
	{
		const set<AnnotationId>& ids =	GetIncomingAnnotationSet(prev_end, "");
		// check resize rules -> cannot terminate if one overlapped branch contains offsets greater than resize offset
//		cout << "incoming AT PREV = " << endl;
		for (it = ids.begin(); it != ids.end(); ++it)
		{
//			cout << "  " << GetAnnotationType(*it)<< "("<<getOrder(*it)<<")=" << *it << endl;
			if ( GetAnnotationType(*it) == ptype && *it != parent_id )
			{
				vector<string> v;
				vector<string>::iterator itv;
				// can't terminate before last anchored element in branch !!
				getLinkedElements(*it, v, m_conventions.mainstreamBaseType(graphtype), false, false);

				for (itv = v.begin(); itv != v.end(); ++itv)
				{
					if (*itv != *it ) {
						const string& r_anchor = GetEndAnchor(*itv);
						if ( r_anchor != prev_end && GetAnchored(r_anchor) && GetAnchorOffset(r_anchor)	> offset) {
//							throw _("Mixing 2 overlapping branches refused");
							MSGOUT << "terminateOverlappingBranches: cannot mix 2 overlapping branches" << endl;
							return false;
						}
					}
				}
			}
		}


		// now set overlapped branches elements ends to align_anchor
//		cout << "FIxed AT PREV = " << endl;
		for (it = ids.begin(); it != ids.end(); ++it)
		{
			if ( getOrder(*it) != order &&
					! m_conventions.isHigherPrecedence(GetAnnotationType(*it), ptype, graphtype) )  {
//				cout << "  " << GetAnnotationType(*it)<< "("<<getOrder(*it)<<")=" << *it << " -> " << align_anchor << endl;
				agSetEndAnchor(*it, align_anchor);
				if ( isMainstreamType(GetAnnotationType(*it), graphtype) )
					updates.push_back(UpdateAction(*it, RESIZED));
			}
		}
	}

	// set 0-order on all segments & attached qualifiers up to prev_end
	// in remaining branch
	resetOrderOnBranch(align_id, prev_end, updates);
	return true;
}

/**
 * set segment end anchor to new anchor
 *   @param id  segment id
 *   @param new_end new end anchor id for segment
 *   @return previous end anchor id for segment
 *
 *  @note
 *    will reattach all lower level segment ends & any segment qualifiers to new end
 */
string DataModel::setEndAnchor(const string& id, const string& new_end)
{
	const string& prev_end = GetEndAnchor(id);
	const string& ptype = GetAnnotationType(id);
	const string& graphtype = getGraphType(id);
	int order = getOrder(id) ;

	const set<AnnotationId>& ids = GetIncomingAnnotationSet(prev_end, "");
	set<AnnotationId>::const_iterator it;
	for (it = ids.begin(); it != ids.end(); ++it)
	{
		const string& type = GetAnnotationType(*it);
		if (isMainstreamType(type, graphtype) )
		{
			if (m_conventions.isHigherPrecedence(ptype, type, graphtype) && getParentElement(*it, ptype, false, false)
					== id)
			{
				agSetEndAnchor(*it, new_end);
			}
		}
		else
		{
			if (getOrder(*it) == order)
				agSetEndAnchor(*it, new_end);
		}
	}
	agSetEndAnchor(id, new_end);
	return prev_end;
}


/**
* Add new anchored element to a continuous graph; new element will split existing one at signal_offset
* @param type 				New element type
* @param prevId 			Existing element to split
* @param signal_offset 		New anchor offset in signal timeline
* @param emit_signal 		If true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
* @param forward_qual_attachments  if false, force reattaching qualifiers ending at prevId's end to inserted element start.
* @return new element 		id / "" if error occured
*/
std::string DataModel::insertMainstreamElement(const string& type, const string& prevId, float signal_offset, bool needSplit, bool emit_signal, bool forward_qual_attachments)
{
	string id("");

// 	TRACE << "(DataModel::insertMainstreamElement) type=" << type << " prevId=" << prevId << " signal_offset=" << signal_offset << endl;

	if (prevId == "" || !ExistsAnnotation(prevId) )
	{
		MSGOUT << "Error: empty or invalid prevId" << endl;
		return "";
	}

	const string& prevtype = GetAnnotationType(prevId);
	const string& graphtype = getGraphType(prevId);

	string baseType = m_conventions.mainstreamBaseType(graphtype);

	if ( signal_offset >= 0 && baseType != prevtype )
	{
		MSGOUT << "Error: non-basetype prevId : is " << baseType << endl;
		return "";
	}

	float dist = 0.;
	if ( signal_offset > 0 )
	{
		const string& prevStartAnchor = GetStartAnchor(prevId);
		dist = ( GetAnchored(prevStartAnchor) ? fabs(GetAnchorOffset(prevStartAnchor) - signal_offset) : (needSplit ? 99. : 0) );
	}

	//> -- We are inserting a mainstream type different to base mainstream type
	if ( type != baseType )
	{
		// create new base element
		if ( dist > EPSILON )
		{
			id = insertMainstreamBaseElement(prevId, signal_offset, forward_qual_attachments, emit_signal);

			do
			{
				baseType = m_conventions.getParentType(baseType);
				id = insertParentElement(id, emit_signal);
			}
			while ( baseType != type) ;
		}
		else
		{
			baseType = m_conventions.getAlignmentType(type);
			bool baseCase =  ( prevtype == baseType ) ;
			string pid = ( baseCase ? prevId : getParentElement(prevId, baseType) );

			// keep current base element: don't forget to set the time code
			const string& pidStartAnchor = GetStartAnchor(pid) ;
			if (baseCase)
				setAnchorOffset(pidStartAnchor, signal_offset, true, emit_signal) ;

			id = getParentElement(pid);
			if ( GetStartAnchor(id) != GetStartAnchor(pid) )
			{
				id = insertParentElement(pid, emit_signal);

				// when
				if (baseCase && emit_signal && !needSplit)
				{
					//TODO temporary UGLY
					//send UPDATED signal & adapt corresponding callback
					emitSignal(getElementType(prevId), prevId, DELETED) ;
					if ( mainstreamBaseElementHasText(prevId) )
						setElementProperty(prevId, "value", "") ;
					emitSignal(prevId, INSERTED) ;
				}
			}
			// segment of same type exists -> insert overlapping element.
			else
			{
				try
				{
					insertOverlappingElement(id, emit_signal);
				}
				catch (const char* msg)
				{
					MSGOUT << "can't insert overlapping " << type  << " : found existing element with id "<< id << endl;
					return "";
				}
			}
		}
	}
	//> -- We're inserting a base mainstream type
	else
	{
		id = insertMainstreamBaseElement(prevId, signal_offset, forward_qual_attachments, emit_signal);
	}
	return id;
}

std::string DataModel::insertEventMainstreamElement(const string& prevId, float start_offset, float end_offset,
														const string& value, const string& subvalue,
														bool fwd_qualifier, bool emit_signal)
{
	if (prevId == "" || !ExistsAnnotation(prevId) )
	{
		MSGOUT << "Error: empty or invalid prevId" << endl;
		return "";
	}

	// -> need to split prevId 2 times
	string id = insertMainstreamBaseElement(prevId, start_offset, fwd_qualifier, false) ; //true
	string nextid = insertMainstreamBaseElement(id, end_offset, fwd_qualifier, false) ; //true
	setElementProperty(id, "subtype", "unit_event", false) ;
	setElementProperty(id, "value", value, false) ;
	setElementProperty(id, "desc", subvalue, false) ;
	if ( emit_signal ) {
		emitSignal(prevId, RESIZED);
		emitSignal(id, INSERTED);
		emitSignal(nextid, INSERTED);
	}
	return id ;
}

std::string DataModel::insertEventMainstreamElement(const string& prevId, float signal_offset,
														const string& value, const string& subvalue,
														bool fwd_qualifier, bool emit_signal)
{
	if (prevId == "" || !ExistsAnnotation(prevId) )
	{
		MSGOUT << "Error: empty or invalid prevId" << endl;
		return "";
	}
	string id = insertMainstreamBaseElement(prevId, signal_offset, fwd_qualifier, false) ; //false
	setElementProperty(id, "subtype", "unit_event", false) ;
	setElementProperty(id, "value", value, false) ;
	setElementProperty(id, "desc", subvalue, false) ;
	if ( emit_signal ) {
		emitSignal(prevId, RESIZED);
		emitSignal(id, INSERTED);
	}
	return id ;
}

bool DataModel::setEventMainstreamElement(const string& id, const string& value, const string& subvalue, bool emit_signal)
{
	bool updated = false ;

	//> 1 -- should be applied ?
	if ( !existsElement(id) || mainstreamBaseType("transcription_graph")!= getElementType(id) )
		return updated ;

	if ( getElementProperty(id, "subtype", "") != "unit_event" )
	{
		setElementProperty(id, "subtype", "unit_event", false) ;
		updated = true ;
	}

	//> 2 -- Apply
	if ( getElementProperty(id, "value", "") != value )
	{
		updated = true;
		setElementProperty(id, "value", value, false) ;
	}
	if ( getElementProperty(id, "desc", "") != subvalue )
	{
		updated = true;
		setElementProperty(id, "desc", subvalue, false) ;
	}

	//> 3 -- Tell fathers ?
	if (updated && emit_signal)
	{
		// emit signal for parent segment
		setUpdated(true) ;
		emitSignal(id, UPDATED);
	}

	return updated ;
}

/**
* add new anchored mainstream base-type element to a continuous graph; new element will split existing one at signal_offset
* @param prevId existing mainstream base-type element to split
* @param signal_offset new anchor offset in signal timeline
* @param forward_attached_qualifiers if true, any qualifier attachments will be forwarded to new element's end
* @param emit_signal if true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
* @return new element id / "" if error occured
*
*/
std::string DataModel::insertMainstreamBaseElement(const string& prevId, float signal_offset, bool forward_attached_qualifiers, bool emit_signal)
{
	string id("");

	if (prevId == "" || !ExistsAnnotation(prevId) ) {
		MSGOUT << "Error: empty or invalid prevId" << endl;
		return "";
	}
	const string& graphtype = getGraphType(prevId);
	const string& type = m_conventions.mainstreamBaseType(graphtype);
	bool do_check=false; // TODO a gerer..=?

	if ( do_check )
	{
		const string& p_type = GetAnnotationType(prevId);
		if ( p_type != m_conventions.mainstreamBaseType()) {
			MSGOUT << " prevId=" << prevId <<  " in not a mainstream base type element" << endl;
			return "";
		}
		// first check that split offset is coherent with current element anchoring
		float start = getStartOffset(prevId);
		float stop = getEndOffset(prevId);
		if ( signal_offset != -1 && start > signal_offset ) {
			MSGOUT << "Can't insert " << type << " element after prevId=" << prevId <<  ": signal_offset=" << signal_offset << " prevstart=" << start << endl;
			return "";
		}

		if ( signal_offset != -1 && stop < signal_offset ) {
			MSGOUT << "Can't insert segment after prevId=" << prevId << " : signal_offset=" << signal_offset << " > stop=" << stop << endl;
			return "";
		}
	}

	// anchor prevId's end to start and create new annotation
	const string& prev_end = GetEndAnchor(prevId);
	const string& order = getElementProperty(prevId, "order");

//	Log::trace() << "(DataModel::insertMainstreamBaseElement) offset=" << signal_offset << " prevId=" << prevId
//		<< " start=" << GetStartAnchor(prevId) << " end=" << prev_end << " order=" << order
//		<< " notrack=" << getAnchorSignalTrack(prev_end) << endl;

//	const string& new_end = agSplitAnchor(prev_end);
	AnchorId unused="";
	list<AnnotationId> split_ids = agSplitAnnotation(prevId, "", unused);
	id = split_ids.back();

	if ( ExistsFeature(id, "value") )
		agDeleteFeature(id, "value");

	//> -- Set base mainstream subtype
	//TODO set element property subtype dynamically by findind good one (?) in conventions
	int notrack = getElementSignalTrack(prevId) ;
	const string& newStartA = GetStartAnchor(id) ;
	addAnchorToTrackMap(newStartA, notrack) ;

	vector<string> subtypes;
	if ( m_conventions.mainstreamHasSubtypes(type, graphtype, subtypes) )
		setElementProperty(id, "subtype", subtypes.front(), false) ;  // default subtype

	const string& new_end = GetStartAnchor(id);
//	Log::trace() << " newId=" << id << " start=" << GetStartAnchor(id) << " end=" << GetEndAnchor(id) << " order=" << getOrder(id) << endl;
//	Log::trace() << "  ---- > prevId=" << prevId << " start=" << GetStartAnchor(prevId) << " end=" << GetEndAnchor(prevId) << " order=" << getOrder(id) << endl;

	if ( signal_offset != -1 )
		agSetAnchorOffset(new_end, signal_offset);

	if ( emit_signal ) {
		emitSignal(prevId, RESIZED);
		emitSignal(id, INSERTED);
	}


	if ( ! m_inhibateChecking )
		applyQualifiersSpanRules(id, forward_attached_qualifiers, emit_signal);

	setUpdated(true) ;
	return id;
}

/**
 * re-attach qualifier annotations to id start if cannot span over this type of element
 * @param id 	annotation id
 * @param forward_attached_qualifiers  if true and cannot span over this type of element,
 * 			then split annotation at inserted element start, else reattach end to id's start anchor
 * @param emit_signal if true emit signalUpdated
 */

void DataModel::applyQualifiersSpanRules(const string& id, bool forward_attached_qualifiers, bool emit_signal)
{

	const string& new_end = GetStartAnchor(id);
	const string& prev_end = GetEndAnchor(id);
	const string& type = GetAnnotationType(id);
	const string& graphtype = getGraphType(id);
	int order = getOrder(id);

	set<AnnotationId> qids;
	set<AnnotationId>::iterator itq;

//	TRACE << "applyQualifiersSpanRules id=" << id << " type=" << type << " order=" << order << endl;
	if ( type == mainstreamBaseType(graphtype)  )
	{
		const set<AnnotationId> ids = GetIncomingAnnotationSet(prev_end, "");
		set<AnnotationId>::const_iterator it;
		for (it = ids.begin(); it != ids.end(); ++it) {
			if ( getOrder(*it) == order && ! isMainstreamType(GetAnnotationType(*it), graphtype) )
				qids.insert(*it);
		}
	} else {
		// then check all qualifiers between previous and current id
		// -> get all candidate qualifiers, start-anchored between  previous and current id,
		//    check whether they are end-anchored  between  previous and current id,
		//		else add them to ids to be checked.
		const string& prevId = getPreviousElementId(id);
		if ( !prevId.empty() && getOrder(prevId) == order ) {
			string astart = GetStartAnchor(prevId);
			set<AnchorId> intermediate_anchors;
			set<AnnotationId> candidates;
			while ( !astart.empty() && astart != new_end ) {
				intermediate_anchors.insert(astart);
				const set<AnnotationId>& ids = GetOutgoingAnnotationSet(astart, "");
				set<AnnotationId>::const_iterator it;
				astart="";
				for (it = ids.begin(); it != ids.end(); ++it ) {
					if ( getOrder(*it) == order ) {
						const string& btype = GetAnnotationType(*it);
						if (btype == mainstreamBaseType())
							astart = GetEndAnchor(*it);
						else if ( ! conventions().isMainstreamType(btype, graphtype) )
							candidates.insert(*it);
					}
				}
			}
			intermediate_anchors.insert(new_end);
			// now add qualifiers which overlap new_end to ids
			for (itq = candidates.begin(); itq != candidates.end(); ++itq) {
				if ( intermediate_anchors.find(GetEndAnchor(*itq)) == intermediate_anchors.end() )
					qids.insert(*itq);
			}
		}
	}


	// now reattach qualifiers.
	for (itq = qids.begin(); itq != qids.end(); ++itq)
	{
		const string& btype = GetAnnotationType(*itq);
		if ( !forward_attached_qualifiers )
		{
//			TRACE << " reattach " << btype << " id=" << *itq << " to end=" << new_end << endl;
			agSetEndAnchor(*itq, new_end);
			if ( emit_signal )
				emitSignal(*itq, UPDATED);
		}
		else if ( ! m_conventions.canSpanOverType(btype, type) )
		{
			AnchorId unused = "";
			const list<AnnotationId>& ids = agSplitAnnotation(*itq, "", unused);
			string nid = ids.back();
			unused = GetStartAnchor(nid);
			agSetEndAnchor(*itq, new_end);
			agSetStartAnchor(nid, new_end);
			agDeleteAnchor(unused);
//			TRACE << " reattach " << btype << " id=" << *itq << " from=" << prev_end << " to=" << new_end << endl;
//			TRACE << " copy " << btype << " id=" << nid << " from=" << new_end << " to=" << GetEndAnchor(nid) << endl;
			if ( emit_signal )
			{
				emitSignal(*itq, UPDATED);
				emitSignal(nid, INSERTED);
			}
		}
	}
}


std::string DataModel::insertMainstreamBaseElement(const string& prevId, float start_offset, float end_offset, bool forward_attached_qualifiers, bool emit_signal)
{
	string id("");

	if (prevId == "" || !ExistsAnnotation(prevId) )
	{
		MSGOUT << "Error: empty or invalid prevId" << endl;
		return "";
	}
	const string& graphtype = getGraphType(prevId);
	const string& type = m_conventions.mainstreamBaseType(graphtype);


	//> -- Insert first border
	const string& inserted = insertMainstreamBaseElement(prevId, start_offset, forward_attached_qualifiers, emit_signal) ;
	if ( inserted.empty() )
	{
		MSGOUT << "Error while inserting in range a mainstream base element" << endl;
		return "" ;
	}

	//> -- insert second border
	const string& remaining_part = insertMainstreamBaseElement(inserted, end_offset, forward_attached_qualifiers, emit_signal) ;
	// deals with failure
	if ( remaining_part.empty() )
	{
		MSGOUT << "Error while inserting in range a mainstream base element" << endl;
		deleteMainstreamElement(inserted, emit_signal) ;
		return "" ;
	}

	//> -- As we have cut an annotation in two parts, let's propagate the feature
	//     to the second part (when splitting, they were stuck to the first part)
	setElementProperty(remaining_part, "subtype", getElementProperty(prevId, "subtype"), false) ;
	if ( ! mainstreamBaseElementHasText(prevId) )
		setElementProperty(remaining_part, "value", getElementProperty(prevId, "value"), false) ;

	if ( emit_signal )
	{
		signalElementModified().emit(mainstreamBaseType(graphtype), prevId, UPDATED);
		signalElementModified().emit(mainstreamBaseType(graphtype), remaining_part, UPDATED);
	}

	setUpdated(true) ;
	return inserted ;
}

/**
* add new overlapping element to a continuous graph upon existing one
* @param overId existing mainstream element to overlap
* @param emit_signal if true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
* @return new element id / "" if error occured
*
*/
std::string DataModel::insertOverlappingElement(const string& overId, bool emit_signal)
{
	string id("");
//	TRACE << "(DataModel::insertOverlappingElement)  overId=" << overId << endl;

	if (overId == "" || !ExistsAnnotation(overId) ) {
		MSGOUT << "Error: empty or invalid overId" << endl;
		return "";
	}
	const string& type = GetAnnotationType(overId);
	const string& graphtype = getGraphType(overId);

	// anchor prevId's end to start and create new annotation
	const string& new_start = GetStartAnchor(overId);
	const string& new_end = GetEndAnchor(overId);
	set<AnnotationId>::const_iterator it;
	int new_order = 1;

	//  qualifier annotations : if may span over this type of element, nothing to do,
	//  else duplicate annotation for new inserted element.

	const set<AnnotationId>& ids = GetOutgoingAnnotationSet(new_start, type);
	if ( ids.size() > 1 )
	{
		for (it = ids.begin(); it != ids.end(); ++it)
		{
			int order = getOrder(*it);
			if ( order >= new_order ) ++new_order;
		}
	}
	if ( new_order > m_conventions.maxOverlap(type) ) {
		throw "Too many overlapping elements";
	}

	const vector<string>& types = m_conventions.getMainstreamTypes(graphtype);
	vector<string>::const_iterator itt;
	bool doit = false;
	string childId;
	vector < pair<string, UpdateType> > updates;

	for (itt = types.begin(); itt != types.end(); ++itt ) {
		if ( *itt == type ) doit = true;
		if ( doit ) {
			childId = createAnnotation(m_agIds[graphtype], *itt, new_start, new_end, false);
			if ( childId != "" ) {
				if ( *itt == type ) id = childId;
				setElementProperty(childId, "order", new_order, false);
				if ( emit_signal )
					updates.insert(updates.begin(), pair<string, UpdateType>(childId, INSERTED));
			}
		}
	}

	vector< pair<string, UpdateType> >::iterator itv;
	for (itv = updates.begin(); itv != updates.end(); ++itv) {
		emitSignal((*itv).first, (*itv).second);
	}

	setUpdated(true) ;
	return id;
}

/**
* add new mainstream text-type element to a continuous graph; new element will split existing one, its text contents will be split at given offset.
* @param prevId existing mainstream text-type element to split
* @param text_offset text offset where to split
* @param signal_offset 			New anchor offset in signal timeline / -1 if unanchored
* @param forward_attached_qualifiers 		If true, any qualifier attachments will be attached to new element's end
* @param emit_signal if true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
* @return new element id / "" if error occured
*/

std::string DataModel::splitTextMainstreamElement(const string& prevId, float text_offset, float signal_offset, bool forward_attached_qualifiers, bool emit_signal)
{
		const string& newId = insertMainstreamBaseElement(prevId, signal_offset, forward_attached_qualifiers, false);
		splitTextContent(prevId, text_offset, false);
		if ( emit_signal ) {
//			emitSignal(prevId, UPDATED);  // TODO -> check if required.
			if ( signal_offset != -1 ) {
				// emit resized for previous anchored element
				string prevA = prevId;
				while ( !prevA.empty() && ! isAnchoredElement(prevA, true) )
					prevA = getPreviousElementId(prevA);
				if ( !prevA.empty() ) emitSignal(prevA, RESIZED);
			}
			emitSignal(newId, SPLITTED);
		}
		return newId;
}

/**
* add new mainstream parent element to a continuous graph; new element will split existing one at child element start
* @param childId existing mainstream child type element where to split
* @param emit_signal if true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
* @return new element id / "" if error occured
*
*/

std::string DataModel::insertParentElement(const string& childId, bool emit_signal)
{
	string id("");
//	TRACE << "(DataModel::insertParentElement)  childId=" << childId << endl;

	if (childId == "" || !ExistsAnnotation(childId) ) {
		MSGOUT << "Error: empty or invalid childId" << endl;
		return "";
	}

	const string& childtype = GetAnnotationType(childId);
	const string& graphtype = getGraphType(childId);
	const string& type = m_conventions.getParentType(childtype, graphtype);
	const string& prevParent = getParentElement(childId);

	// anchor prevParent end to start and create new annotation
	const string& prev_end = GetEndAnchor(prevParent);
	const string& new_end = GetStartAnchor(childId);
	string order = getElementProperty(prevParent, "order");
	set<AnnotationId>::const_iterator it;

//	Log::trace() << "DataModel::insertParentElement type=" << type <<  " prevParent=" << prevParent << " childId=" << childId << " order=" << order
//		<< " notrack=" << getAnchorSignalTrack(prev_end) << endl;

	vector<UpdateAction> resized;
	if ( m_conventions.canOverlap(type) ) {
		// then new element will terminate eventual overlapping branch.
		terminateOverlappingBranches(prevParent, childId, resized);
		order = "";
	}

	id = createAnnotation(m_agIds[graphtype], type, new_end, prev_end, false);

	if ( !order.empty() )
		setElementProperty(id, "order", order, false);
	agSetEndAnchor(prevParent, new_end);

	if (id != "" && emit_signal)
	{
		TRACE_D << "signal segment modified: 1" << endl;
		emitSignal(prevParent, RESIZED);
		emitSignal(id, INSERTED);

	vector<UpdateAction>::iterator itm;
		for ( itm=resized.begin(); itm != resized.end(); ++itm )
			emitSignal(itm->first, itm->second);
	}

	if ( ! m_inhibateChecking && m_conventions.isSpannableType(childtype) )
		applyQualifiersSpanRules(id, true, emit_signal);

	setUpdated(true) ;
	return id;
}


/**
 * split text annotation contents between given annotation id and following element.
 * @param prevId current text id
 * @param text_offset offset where to split (in chars)
 * @param emit_signal true if element modified signal to be emitted.
 */
void DataModel::splitTextContent(const string& textId, int text_offset, bool emit_signal)
{
	if ( !mainstreamBaseElementHasText(textId) )
		return ;

	Glib::ustring txt = getElementProperty(textId, "value");
//	TRACE << "DataModel::splitTextContent id=" << textId << " txt=" << txt << "//  text_offset=" << text_offset << endl;
	if ( text_offset < txt.length() )
	{
		const string& nid = getNextElementId(textId);
		// -- Total replacement
		if ( text_offset <= 0 )
		{
			setElementProperty(textId, "value", "" , emit_signal);
			setElementProperty(nid, "value", txt , emit_signal);
		}
		// -- Cut
		else
		{
			Glib::ustring txt2 = txt.substr(text_offset);
			trim(txt2);
			txt.erase(text_offset);
			trim(txt);
			// TODO ici remettre emit_signal=true ??
			setElementProperty(textId, "value", txt , emit_signal);
			setElementProperty(nid, "value", txt2, emit_signal);
		}

		// update qualifiers coming at textId end
		vector<string> quals ;
		getQualifiers(textId, quals, "", false, false) ;
		vector<string>::iterator it ;
		for (it=quals.begin(); it!=quals.end(); it++)
			emitSignal(*it, UPDATED) ;

		// update qualifiers outgoing from nid start
		getQualifiers(nid, quals, "", false, false) ;
		for (it=quals.begin(); it!=quals.end(); it++)
			emitSignal(*it, UPDATED) ;
	}
}


//
// 	add  background-type segment
std::string DataModel::addBackgroundSegment(int notrack, float start_offset,
		float stop_offset, const string& subtype, const string& level,
		bool emit_signal)
{
	string graphtype = "background_graph";
	if ( ! hasAnnotationGraph(graphtype) ) return "";

	//> CHECK CONSTRAINSTS
	std::string id ;
	bool active = (!subtype.empty()) ;

	if (stop_offset==-1)
		id = addBackground("", subtype, level, notrack, start_offset, 0, true, emit_signal, active, true) ;
	else
		id = addBackgroundBySelection(subtype, level, notrack, start_offset, stop_offset, emit_signal, active);

	//const std::string& id = agCreateAnnotation(graphId, astart, astop, type);

	if (!id.empty())
		setUpdated(true) ;

	return id;
}



std::string DataModel::addBackground(string prevId, const string & subtype, const string& level, int notrack, float signal_offset,
		int text_offset, bool at_end, bool emit_signal, bool active, bool check_active)
{
	Glib::ustring type = "background" ;
	string graphtype = "background_graph";
	string id = "" ;

	if (prevId.empty())
		prevId = getByOffset("background", signal_offset, notrack);

	if (prevId == "" || !ExistsAnnotation(prevId) ) {
		MSGOUT << "Error: empty or invalid prevId" << endl;
		return "";
	}

	// if we're about to create an inactive bg and if the previous one
	// is an inactive bg => do nothing, it's the same
	if (check_active && (!isActiveBackground(prevId) && !active) )
		return "" ;

	// first check that split signal offset is coherent with current segment anchoring
	float start = GetStartOffset(prevId);
	float stop = GetEndOffset(prevId);
	if ( start > signal_offset ) {
			MSGOUT << "Can't insert segment after prevId=" << prevId <<  " with start > signal_offset : " << start << " - " << signal_offset << endl;
			return "";
	}

	if ( stop < signal_offset ) {
			MSGOUT << "Can't insert segment after prevId=" << prevId << " with stop < signal_offset : " << stop << " - " << signal_offset << endl;
			return "";
	}

	// Will anchor prevId's end to start and create new annotation
	const string& prev_end = GetEndAnchor(prevId);
	set<AnnotationId>::const_iterator it;

	// 2 cas ->  ajout d'un nouveau segment / split d'un segment existant -
	//
	// cas 1 - ajout d'un nouveau segment  (en fin de tour ou de transcription)
	// - we set previous segment end anchor time and add a new segment

	if (at_end)
	{
		// fix "at_end" criteria -> if unanchored prev_end then we are
		// definitely not positionned on a segment end
		at_end = GetAnchored(prev_end) ;
	}

	TRACE_D << "DataModel PrevId=" << prevId << " prev_end=" << prev_end
			<< " notrack=" << getAnchorSignalTrack(prev_end) << " at_end="
			<< at_end << endl;

	if (at_end)
	{
		if ( fabs(start - signal_offset) < EPSILON ) {
			// no need to insert at seg start
			id = prevId;
		} else
			id = insertMainstreamBaseElement(prevId, signal_offset, false, false) ;

		// update last anchor info
		// if ( m_lastAnchor == prev_end ) m_lastAnchor = new_end;

		if (id != "")
		{
				//> merge padding background if next and id are padding bgS
			AnnotationId next = getNextElementId(id) ;
			if (!next.empty() && getElementType(next)=="background" && check_active)
			{
				if ( !isActiveBackground(next) && !active)
				{
					AnchorId end_current = GetEndAnchor(id) ;
					AnchorId end_next = GetEndAnchor(next) ;
					if (emit_signal)
						emitSignal(next, DELETED) ;
					agDeleteAnnotation(next) ;
					TRACE_D << "----> delete annotation " << next << endl ;
					agSetEndAnchor(id,end_next) ;
					TRACE_D << "----> set end anchor of " << id << " at " << end_next << endl ;
					agDeleteAnchor(end_current) ;
					TRACE_D << "----> delete anchor " << end_current << endl ;
				}
			}


			if ( !subtype.empty() )
				agSetFeature(id, "type", subtype);
			if ( !level.empty() )
				agSetFeature(id, "level", level);

			//> propagate to display if needed
			if (emit_signal) {
				emitSignal(prevId, RESIZED);
				if ( id != prevId )
					emitSignal(id, INSERTED) ;
			}
		}
	}
	else
	{
		MSGOUT << " SHOULD NOT HAPPEN ! at " << __FILE__ << ":" << __LINE__ << endl;
		return "";
	}

	setUpdated(true) ;
	return id;
}


std::string DataModel::addBackgroundBySelection(const string& subtype, const string& level, int notrack, float start_offset, float stop_offset,
		bool emit_signal, bool active)
{
	string type = "background" ;
	string graphtype = "background_graph" ;

	if ( m_agIds.find(graphtype) == m_agIds.end() ) {
		MSGOUT << "No " <<  graphtype << " graph in datamodel" << endl;
		return "";
	}
	const string& graphId = m_agIds[graphtype];

	string id = "";

	if (stop_offset == 0.0) {
		stop_offset = getNextSegmentStartOffset(type, notrack, start_offset+EPSILON);
	}

	if ( start_offset > stop_offset ) {
		MSGOUT << "Can't insert " << type << " notrack=" << notrack << " with start > stop_offset : " << start_offset << " - " << stop_offset << endl;
		return "";
	}

	string	prevId = getByOffset("background", start_offset, notrack);

	// TODO -> ICI VOIR USAGE DE SPLIT ANNOTATION !!

	if ( !prevId.empty() )
	{
		string second_part = "" ;
		const string& prev_end = GetEndAnchor(prevId);
		float prev_end_offset = GetAnchorOffset(prev_end);

		string prevType = getElementProperty(prevId, "type") ;
		string prevLevel = getElementProperty(prevId, "level") ;

		id = addBackground(prevId, subtype, level, notrack, start_offset, 0, true, false, active, true);
		//if creation OK
		//if we"re creating a background inside another (inside prevId)
		//the last bg created is the second part of the initial cut background
		//-> we need to restore the values
		if (id != "" && fabs(prev_end_offset - stop_offset) > EPSILON )
		{
			// note: we don't need to check the compatibility of types of "id" and "second_part"
			// 		 that we're about to create because the "second_part" has same bg
			//  	 type than "prev_id"
			// 		 ==> means if "prev_id" and "id" are compatible, then "id" and "second_part" are too
			second_part = addBackground(id, "", "", notrack, stop_offset, 0, true, false, active, false) ;
			if ( !prevType.empty() )
				setElementProperty(second_part, "type", prevType, false) ;
			else
				deleteElementProperty(second_part, "type", false) ;

			if ( !prevLevel.empty())
				setElementProperty(second_part, "level", prevLevel, false) ;
			else
				deleteElementProperty(second_part, "level", false) ;

			if ( !subtype.empty() )
				agSetFeature(id, "type", subtype);
			if ( !level.empty() )
				agSetFeature(id, "level", level);
		}

		if (emit_signal && !id.empty())
		{
			emitSignal(prevId, UPDATED) ;
			if ( id != prevId )
				emitSignal(id, INSERTED) ;
			if (!second_part.empty())
				emitSignal(second_part, INSERTED) ;
		}

	}
	return id;
}


/**
 *  add qualifier annotation to segments
 *  @param qtype 		Qualifier type
 *  @param start_id 	Id of segment on which start anchor the qualifier start is attached
 *  @param end_id 		Id of segment on which start anchor the qualifier end is to be attached; if left blank, then qualifier end will be attached to start_id's end anchor.
 *  @param desc 		Qualifier desc
 *  @param emit_signal 	True if element modified signal to be emitted.
 */
std::string DataModel::addQualifier(const string& qtype, const string& start_id, const string& end_id, const string& desc, bool emit_signal)
{
	bool is_instantaneous = m_conventions.isInstantaneous(qtype);
//	TRACE << "(DataModel::addSegmentQualifier) qtype=" << qtype << " start_id=" << start_id << " end_id=" << end_id << " instantaneous_type" << is_instantaneous << endl;

	const AnchorId& sid = GetStartAnchor(start_id);
	const AnchorId& eid = (end_id.empty() ? GetEndAnchor(start_id) : GetStartAnchor(end_id));
	const string& graphId = GetAGId(start_id);
	const string& qid = createAnnotation(graphId, qtype, sid, eid, false);

	if ( !desc.empty() )
		agSetFeature(qid, "desc", desc);

	const string& order = getElementProperty(start_id, "order");

	if ( !order.empty() )
		agSetFeature(qid, "order", order);

	if ( emit_signal )
		emitSignal(qid, INSERTED);

	return qid;
}



/*========================================================================
 *
 *  delete data from graph
 *
 *========================================================================*/

bool DataModel::deleteElement(const string& id, bool emit_signal, bool force_wtext)
{
	string unused ;
	return deleteElement(id, emit_signal, unused, force_wtext) ;
}

bool DataModel::deleteElement(const string& id, bool emit_signal, string& err, bool force_wtext)
{
	if ( !ExistsAnnotation(id) )
	{
		err = string(_("Invalid annotation:")) + " " + id ;
		return false;
	}

	try
	{
		const string& type = GetAnnotationType(id);
		const string& graphtype = getGraphType(id);

		if ( isMainstreamType(type, graphtype) )
		{
			bool with_children =false;
			bool ok = checkDeletionRules(id, with_children, err);
			if ( ! ok )
				throw err.c_str();
			else
			{
				if (graphtype == "transcription_graph")
				{
					// specific treatment for mainstream event
					if ( isEventMainstreamElement(id, graphtype) )
						deleteEventMainstreamElement(id, emit_signal) ;
					// otherwise, go !
					else
						deleteMainstreamElement(id, emit_signal, force_wtext);
				}
				else if (type=="background")
					deleteBackground(id, emit_signal) ;
			}
		}
		else
		{
			AnchorId start = GetStartAnchor(id);
			AnchorId stop = GetEndAnchor(id);
			string pstartid = getMainstreamStartElement(id);
			string pstopid = getMainstreamNextElement(id);

			// <!> emit signal BEFORE deleting because we need
			// some current element information for updating view
			// TODO get information before and send them with signal
			// after
			if ( emit_signal )
				emitSignal(id, DELETED);
			agDeleteAnnotation(id);

			if ( GetAnchored(start) == false && !pstartid.empty() )
			{
				// if no other "qualifier-only" annotation than deleted one attached to start anchor
				// then unsplit parent element
				if ( ! anchorHasOtherAttachedElements(start, pstartid) )
					deleteMainstreamElement(pstartid, emit_signal, force_wtext);
			}
			if ( stop != start && GetAnchored(stop) == false && !pstopid.empty() )
			{
				// if no other "qualifier-only" annotation than deleted one attached to stop anchor
				// then unsplit parent element
				if ( ! anchorHasOtherAttachedElements(stop, pstopid) )
					deleteMainstreamElement(pstopid, emit_signal, force_wtext);
			}
		}
		setUpdated(true) ;
		return true ;
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
		err = e.error() ;
		return false ;
	}
	catch (const char* exception)
	{
		Log::out() << "~~~ deleteElement : unknown exception : " << exception << std::endl ;
		return false ;
	}
}


/**
 *  delete mainstream element with given id
 * @param id id of mainstream element to be deleted
 * @param with_children also delete children annotations (lower level mainstream segments)
 * @param emit_signal if true, emit signalElementModified() signal for both deleted element and eventually resized element
 *
 * @note
 *   When a segment is deleted, previous segment of same type is extended to deleted's end.
 *	If deleted segment is a "text" segment, deleted segment data is "attached" to previous segment :
 *   - if no "event" annotation is attached to deleted segment start node, then segment text is added to previous segment, previous segment end is set to deleted segment end and start node is deleted,
 *   - if any "event" annotation is attached to deleted segment start node, then this node is kept but "unanchored", ie. its signal offset is unset.
 *
 *  If any "mainstream" upper level annotation (eg. a turn) is attached at deleted segment start node, it will also be deleted
 *
 *  if "overlapping" branches terminate at deleted segment start, then all branches are extended to deleted segment end, as shown:
 *
 *      ___ a2___a3___                     ___ a2___a3______
 *   a1/              \a5_a6   ==>      a1/                 \a6
 *     \______a4______/                   \______a4_________/
 *
 *
 *  if deleted segment is the first segment of an "overlapping" branch :
 *	  - if its order is > 0 : it is attached to the end of main branch first segment, and eventual following segments are also attached to main branch first segment.
 *
 *      ___ a2___a3___                     ___ a2___a3
 *   a1/              \a5_a6   ==>      a1/         /      a5-a6
 *     \______a4______/                            a4_____/
 *
 *
 *	  - if its order is = 0 : ??? -> deletion not allowed.
 *		TODO -> allow deletion if "empty" (ie no text, no qualifiers) -> then overlapping branch becomes main branch.
 *
 */
void DataModel::deleteMainstreamElement(const string& id, bool emit_signal, bool force_wtext)
{
	if ( !ExistsAnnotation(id) )
	{
		return;
	}
	const string& type = GetAnnotationType(id);
	const string& graphtype = getGraphType(id);
	const string& basetype = m_conventions.mainstreamBaseType(graphtype);
	const string& graphId = GetAGId(id);
	int curtrack = getElementSignalTrack(id);

	if ( !isMainstreamType(type, graphtype) )
	{
		MSGOUT << "Can delete non mainstream-type element " << type << " id="
		<< id << ", use deleteElement method" << endl;
		return;
	}

	try
	{
		Log::trace() << "--------  deleteMainstreamElement : " << type << " id=" << id << endl ;

		set<AnnotationId> resized ; // contains ids of segments resized during deletion
		set<AnnotationId> updated ;
		set<AnnotationId>::const_iterator it;
		bool	force_whole_display = false;
		string 	firstChildToDelete ;
		int 	order = getOrder(id) ;

		string previd("");
		AnchorId startAnchor = GetStartAnchor(id);
		//		const set<AnnotationId>& cids = GetOutgoingAnnotationSet(startAnchor, "");

		// -- When deleting a segmentation base type, take care with the first child
		// 	 ==> If the first child is a text element and the previous is a text element too,
		//       delete the child (text will be merged in view)
		//       In all other cases, keep it !
		if ( type==segmentationBaseType(graphtype) )
		{
			firstChildToDelete = getElementStartingAtAnchor( startAnchor, mainstreamBaseType(), order) ;
			const string& prevId = getPreviousElementId(firstChildToDelete) ;
		//	bool firstOverlap = (isFirstChild(id, "") && getOrder(id)>=1) ;

//			if  ( !firstOverlap )
			if ( !prevId.empty() ) {  // no previous if deleting overlapping branch eg.
				if ( !mainstreamBaseElementHasText(firstChildToDelete, graphtype) || !mainstreamBaseElementHasText(prevId) )
					firstChildToDelete = "" ;
			}
		}

		const string& parent_id = getParentElement(id);
		//		string order = getElementProperty(id, "order");
		int cur_order = getOrder(id);
		const string& del_nextid = getBaseTypeNextId(id);
		const string& del_end = getEndAnchor(id);
		vector<string> to_del; // will store elements to be deleted

		// del_parent is true if "parent" annotation must also be deleted (eg parent turn when deleting 1st segment)
		bool del_parent = ( ! parent_id.empty()
				&& ( GetStartAnchor(parent_id) == startAnchor)
				&& (getOrder(parent_id) == cur_order ) );

		bool with_overlap=false;
		string attach_previd_to("");

		// define previous element to which deleted segment data is to be re-attached
		//  - previous of same type in graph if non-overlapping segment (in case 2 overlapping elements are attached
		//     at segment start, then 0-order element will be taken into account)
		//  - 0-order element of same type attached at same end node if overlapping branch
		//
		// note that deletion forbidden if deleted element has more that one anchored child segment
		previd = getPreviousElementId(id);
		const vector<string>& overid = getElementsWithSameStart(id, type);
		if ( ! overid.empty() )
		{
			if ( cur_order == 0 )
			{ // deletion of "main branch" segment forbidden
				// TODO allow deletion if "empty", ie no text and no qualifiers
				throw _("Can't delete main branch of overlapping segments !");
				//return false ;
			}

			// can't delete first segment of a multi-segmented overlapping turn
			if ( del_parent )
			{
				TRACE_D << " del_nextid = " << del_nextid << " parent_id =" << parent_id << " get parent=" << getParentElement(del_nextid) << endl ;
				if ( !del_nextid.empty()
						&& getParentElement(del_nextid) == parent_id )
					throw _("Can't delete first segment of a multi-segmented overlapping branch");
			}

	// ICI BIZARRE
			// deletion forbidden if more than one base segment in deleted branch
			// if only one child, then delete child
			if ( m_conventions.isHigherPrecedence(type, basetype, graphtype) ) {
				const vector<string>& overid2 = getElementsWithSameStart(id, basetype);
				if ( overid2.size() > 0 ) {
					vector<string>::const_iterator ito;
					string del_child("");
					for ( ito = overid2.begin(); ito != overid2.end(); ++ito )
						if ( getOrder(*ito) == cur_order ) {
							del_child = *ito;
						}
					if ( ! del_child.empty() ) {
//						if ( getEndAnchor(del_child) != del_end ) {
							// then more than one segment in overlaping branch -> deletion refused
//							throw _("Can't delete a multi-segmented overlapping turn, delete segments first");
//							TRACE << "  @@@@@@ DEL CHILD⁼ " << del_child << "SHOULD NO delete a multi-segmented overlapping turn" << endl;
//						}
						// ok, only one segment -> then call deleteMainstreamElement(child_segment) that will do the job
//						return deleteMainstreamElement(del_child, true, emit_signal);
						return deleteMainstreamElement(del_child, emit_signal);
					}
				}
			}
			// ICI FIN BIZARRE

			const vector<string>& overid2 = getElementsWithSameEnd(id, type);
			if ( overid2.size() > 0 )
				previd = overid2.front();
			else
			{
				Log::trace() << "--------  deleteMainstreamElement : No segment with same end node found : id=" << id << " ... Aborted."<< endl ;
				throw _("No segment with same end node found !");
			}

			with_overlap =true;
		}

		// if deleted type == mainstreamBaseType :
		// if qualifiers are attached to segment start, keep
		// unanchored node, else concatenate segment text to previd text

		//string prev_order = (previd.empty() ? "" : getElementProperty(previd, "order"));
		int prev_order = (previd.empty() ? 0 : getOrder(previd));
		vector<string> v_set_order; // will contain ids of elems for which "order" property is to be eventually set
		vector<string>::iterator itv;
		string overlap_to_extend("");
		string prev_end = (previd.empty() ? "" : GetEndAnchor(previd));


		getLinkedElements(id, v_set_order, "", false, false);

		if ( type == m_conventions.mainstreamBaseType(graphtype) && ! previd.empty() )
		{
			// 4 cases
			//  - non overlapping branch and segment qualifiers starting or ending at id start anchor
			//     -> unanchor start anchor
			//  - non overlapping branch and no segment qualifiers starting or ending at id start anchor
			//      -> add id text to previd and attach previd to id end, delete id and id start anchor
			//  - overlapping branch and segment qualifiers starting at id start anchor or ending at previd end anchor
			//		- reattach id+qual start and previd+qual end to new unanchored node
			//  - overlapping branch and no segment qualifiers starting at id start anchor or ending at previd end anchor
			//      -> add id text to previd and attach previd to id end, delete id
			//  for the 2 first cases, also checks if an overlapping segment ends at previd end -> will be extended up to new end
			//
			vector<string> q_id, q_prev;
			bool previd_has_qual = false;
			bool id_has_qual = false;
			getQualifiers(id, q_id, "", true, false);
			id_has_qual = ( q_id.size() > 0 );
			getQualifiers(previd, q_prev, "", false, false);
			previd_has_qual = ( q_prev.size() > 0 ||  !mainstreamBaseElementHasText(previd) );

			//> -- NO OVERLAP
			if ( !with_overlap )
			{
				//> -- Keep qualifier unanchored node
				if ( previd_has_qual || id_has_qual )
				{
					//						TRACE_D << " NO OVERLAP, WITH QUAL prev=" <<  previd_has_qual << " id=" << id_has_qual << " -> unanchor " << start << endl;
					agUnsetAnchorOffset(startAnchor);
					bool is_text= mainstreamBaseElementHasText(id, graphtype);

					// -- for empty text segment, delete segment
					const string& txt = getElementProperty(id, "value", "") ;

					// -- Always delete non-text unit
					// -- Always delete text unit that has no text data
					// -- Never delete text unit that has text data, except in forced mode
					//		=> because at this point we don't know if all text is erased at same time
					//		   If not, the text unit is still needed and we should not remove it
					//		=> forced mode is set to true when all text is erased at same time
					//TODO: the 3d point is more a merging than a deletion, we should split this
					// 		specific case in a merge method and remove the force case
					if ( (!is_text || txt.empty() || force_wtext) && !id_has_qual )
					{
						attach_previd_to=GetEndAnchor(id) ;
						to_del.push_back(id) ;
					}
				}
				//> -- No qualifier  -> merge text
				else
				{
					// TODO -> use mergeAnnotations !!!
					// concatenate segment text to previd
					string text  ;
					if ( mainstreamBaseElementHasText(id, graphtype) )
						text = getElementProperty(id, "value", "") ;

					if ( ! text.empty() )
					{
						const string& ptext = getElementProperty(previd, "value", "");
						setElementProperty(previd, "value", ptext + " " + text, false);
					}

					//						TRACE_D << " NO OVERLAP, NO QUAL  -> append text and del " <<id << endl;
					attach_previd_to=GetEndAnchor(id);
					to_del.push_back(id);
					to_del.push_back(GetStartAnchor(id));
				}
			}
			//> -- OVERLAP
			else
			{
				//> -- create new anchor and reattach qualifier id start and previd end to new anchor
				if ( previd_has_qual || id_has_qual )
				{
					attach_previd_to=agCreateAnchor(graphId);
					agSetStartAnchor(id,attach_previd_to);
					for (itv=q_id.begin(); itv != q_id.end(); ++itv)
					{
						if ( GetStartAnchor(*itv) == GetEndAnchor(*itv) )
							agSetEndAnchor(*itv, attach_previd_to);
						agSetStartAnchor(*itv, attach_previd_to);
					}

					// -- for empty text segment, delete segment
					const string& txt = getElementProperty(id, "value", "") ;
					if (txt.empty() && !id_has_qual)
						to_del.push_back(id) ;
					//						TRACE_D << " WITH OVERLAP, WITH QUAL  -> attaching to new anchor " << attach_previd_to << endl;
				}
				//> -- No qualifier
				else
				{
					// TODO -> use mergeAnnotations !!!
					// concatenate segment text to previd
					string text ;
					if ( mainstreamBaseElementHasText(id, graphtype) )
						text = getElementProperty(id, "value", "");

					if ( ! text.empty() )
					{
						const string& ptext = getElementProperty(previd, "value", "");
						setElementProperty(previd, "value", ptext + " " + text, false);
					}

					attach_previd_to=GetEndAnchor(id) ;
					to_del.push_back(id);
					//						TRACE_D << " WITH OVERLAP, NO QUAL  -> append text ,del " <<id << "attach prev to " << attach_previd_to << endl;
				}
			}
		}
		//> -- Not a base segment or no previd
		else
		{
			if ( ! with_overlap )
				attach_previd_to=getEndAnchor(id);
			to_del.push_back(id);
			//				TRACE_D << " OTHER  case = del " << id  << " attach_prev=" <<  attach_previd_to << endl;
		}

		// if id not to be deleted -> add it to "set_order" vector
		for (itv=to_del.begin(); itv != to_del.end() && *itv != id; ++itv) ;
		if ( itv == to_del.end() )
			v_set_order.push_back(id);

		// check if overlapping segment ends at previd's end
		if ( ! with_overlap )
		{
			const vector<string>& overid2 = getElementsWithSameEnd(previd, type);
			if ( overid2.size() > 0 )
			{
				overlap_to_extend = overid2.front();
//				TRACE_D << " found overlap extension required for " << overlap_to_extend << endl;
			}
		}

		// set end anchor of previous element to attachment anchor
		if ( ! attach_previd_to.empty() )
		{
			const set<AnnotationId>& ids = GetIncomingAnnotationSet(prev_end, "");
			set<AnnotationId>::const_iterator it;
			for ( it = ids.begin(); it != ids.end(); ++it )
			{
				//if ( getElementProperty(*it, "order") == prev_order )
				if ( getOrder(*it) == prev_order )
				{
					const string& ptype = GetAnnotationType(*it);
					if ( (type == m_conventions.mainstreamBaseType(graphtype) && isQualifierType(ptype) )
							|| (ptype == type) )
					{
						if ( GetStartAnchor(*it) == GetEndAnchor(*it) ) {
							//								TRACE_D << "reattaching " << ptype << " " << *it << " start to " <<  attach_previd_to << endl;
							agSetStartAnchor(*it, attach_previd_to);
						}
						agSetEndAnchor(*it, attach_previd_to);
						TRACE_D << "reattaching " << ptype << " " << *it << " end to " <<  attach_previd_to << endl;
						if ( emit_signal
								&& isMainstreamType(ptype, graphtype) && GetAnchored(GetStartAnchor(*it)) )
							resized.insert(*it);
					}
				}
			}
			// if prev_end anchor remains unused, delete it
			to_del.push_back(prev_end);
		}

		// if "overlapping" branch was ending on deleted segment start, then
		//  extend last overlap segment up to deleted segment end,
		// or up to deleted's parent's end.

		if ( ! overlap_to_extend.empty() )
		{
			const set<AnnotationId>& inc_over = GetIncomingAnnotationSet(startAnchor, "");
			int over_order = getOrder(overlap_to_extend);
			string over_end = del_end;
			if ( del_parent ) over_end = GetEndAnchor(parent_id);

			if ( inc_over.size() > 0 )
			{
				set<AnnotationId>::const_iterator it;
				for ( it = inc_over.begin(); it != inc_over.end(); ++it )
				{
					if ( *it != previd && getOrder(*it) == over_order )
					{
						setEndAnchor(*it, over_end);
//						if ( isMainstreamType(GetAnnotationType(*it)) )
//							resized.insert(*it);
						force_whole_display=true ;
					}
				}
			}

//			if ( getElementType(overlap_to_extend) == mainstreamBaseType(graphtype)
//					|| getElementType(overlap_to_extend) == segmentationBaseType(graphtype) )
//			{
//				const string& targetype = conventions().getParentType(segmentationBaseType(graphtype), graphtype) ;
//				const string& parent = getParentElement(overlap_to_extend, targetype, false) ;
//				updated.insert(parent) ;
//			}
		}

		// now set elements order to previd order in attached branch,
		//	 and eventually emit "element updated" signal
		if ( v_set_order.size() > 0 ) {
//T			RACE << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  SET ORDER !!" << endl;
			for ( itv= v_set_order.begin(); itv != v_set_order.end(); ++itv )
			{
				if ( getOrder(*itv) != prev_order ) {
					if ( prev_order == 0 )
						deleteElementProperty(*itv, "order", false);
					else
						setElementProperty(*itv, "order", prev_order, false);
					TRACE_D << " set order for " << GetAnnotationType(*itv) << " " << *itv << " to " << prev_order << " -" << endl;

					if ( emit_signal && ! force_whole_display && isMainstreamType(GetAnnotationType(*itv), graphtype) )
						updated.insert(*itv) ;
				}
			}
		}

		// delete to_del annots
		for (itv=to_del.begin(); itv != to_del.end(); ++itv )
		{
			if ( ExistsAnnotation(*itv) ) {
				//					TRACE_D << " do delete " << type << " " << *itv << endl ;
				agDeleteAnnotation(*itv);
			}
			else if (ExistsAnchor(*itv)) {
				//					TRACE_D << " delete if unused anchor " << *itv << endl ;
				deleteUnusedAnchor(*itv);
			}
		}

		// removing 0-order element -> set order=0 on whole alternate branch
		if ( !previd.empty() && cur_order == 0  && m_conventions.canOverlap(type) )
		{
			vector<string>::iterator itv;
			v_set_order.clear();
			TRACE_D << "Set 0-order on " << previd << " element" << endl;
			getLinkedElements(previd, v_set_order, "", false, false);
			v_set_order.push_back(previd); // also set 0-order on previd
			for ( itv= v_set_order.begin(); itv != v_set_order.end(); ++itv )
				deleteElementProperty(*itv, "order");
		}
		//
		if ( del_parent )
			deleteMainstreamElement(parent_id, emit_signal);

		//> If needed, delete first children
		//  (espcially when deleting segmentationBaseType, delete first mainstreamBaseType)
		if ( !firstChildToDelete.empty() )
			deleteMainstreamElement(firstChildToDelete, emit_signal) ;

		// if start anchor remains unused, delete it
		deleteUnusedAnchor(startAnchor);

		setUpdated(true) ;

		if ( emit_signal )
		{
			if ( ! force_whole_display )
			{
				if ( ! ExistsAnnotation(id) )
					emitSignal(type, id, DELETED);

				// send signalElementModified for all resized segments
				for ( it = resized.begin(); it != resized.end(); ++it )
					emitSignal(*it, RESIZED);

				for ( it = updated.begin(); it != updated.end(); ++it )
					emitSignal(*it, UPDATED);

				if ( !previd.empty() )
					emitSignal(previd, RESIZED);
			}
			else
			{
				// overlapping segments extended may have invalidated overall display, force whole redisplay
				emitSignal("", "", UPDATED);
			}
		}
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
		throw e.error().c_str() ;
	}
	catch (const char* ex)
	{
		Log::out() << "~~~~ DeleteMainstreamElement : " << ex << std::endl ;
		throw ex ;
	}
}

bool DataModel::deleteEventMainstreamElement(const string& id, bool emit_signal)
{
	bool updated = false ;

	if ( !ExistsAnnotation(id) )
		return updated ;

	const string& type = GetAnnotationType(id);
	const string& graphtype = getGraphType(id);
	const string& graphId = GetAGId(id);
	int curtrack = getElementSignalTrack(id);

	if ( type != mainstreamBaseType(graphtype) )
		return false ;

	if ( mainstreamBaseElementHasText(id, graphtype) )
		return false ;

	const string& parent = getParentElement(id, segmentationBaseType(graphtype)) ;

	//> -- Unique child of segment ? can"t delete it, erase data and set as text
	if ( isUniqueChild(id, parent) )
	{
		// reset all element features to default values
		const map<string, string> props = GetFeatures(id);
		map<string, string>::const_iterator itp;
		for ( itp = props.begin(); itp != props.end(); ++itp)
			agDeleteFeature(id, itp->first);

		vector<string> subtypes;
		if ( m_conventions.mainstreamHasSubtypes(type, graphtype, subtypes) )
				agSetFeature(id, "subtype", subtypes.front()) ;  // default subtype

		if (emit_signal)
			signalElementModified().emit("unit", id, UPDATED) ;
	}
	//> -- First child of segment ? Just change
	else if ( isFirstChild(id, parent) )
	{
		const string& next = getNextElementId(id) ;
		bool nextHasText = mainstreamBaseElementHasText(next, graphtype) ;
		// value feature should only be got from buffer, don't merge it !
		// TODO do it in a more elegant way
		vector<string> except ;
		if (nextHasText)
			except.push_back("value") ;
		mergeAnnotations(id, next, except, false, true, true, false) ;
		if (nextHasText)
			setElementProperty(id, "value", "", false) ;

		// -- Use specific signal to tell gui to delete text too
		//TODO don't use specific signal (always delete text when deleting unit ?)
		if (nextHasText)
		{
			deleteMainstreamElement(next, false) ;
		}
		else
			deleteEventMainstreamElement(next, true) ;

		if (nextHasText && ExistsFeature(id, "desc"))
			deleteElementProperty(id, "desc", false) ;

		if (emit_signal)
		{
			signalElementModified().emit("unit", next, DELETED) ;
			signalElementModified().emit("unit", id, UPDATED) ;
		}
	}
	//> -- Last child ? just delete
	else if ( isLastChild(id, parent))
	{
		deleteMainstreamElement(id, true) ;
	}
	//> -- Other case : delete and merge the surrounders
	else
	{
		const string& prev = getPreviousElementId(id) ;
		const string& next = getNextElementId(id) ;

		// -- 1: delete
		deleteMainstreamElement(id, emit_signal) ;

		// -- 2: merge the remaining text annotations
		if ( mainstreamBaseElementHasText(prev) && mainstreamBaseElementHasText(next))
		{
//			vector<string> except ;
//			except.push_back("value") ;
			mergeAnnotations(prev, next, true, false, true, false) ;
			// value feature should only be got from buffer, don't merge it !
			// TODO do it in a more elegant way
//			setElementProperty(prev, "value", "", false) ;

			// this property must not be merged, we apply it after merge to be sure
			// -> (better than excluding it from merge because it depends of annotation type)
			if (ExistsFeature(prev, "desc"))
				deleteElementProperty(prev, "desc", false) ;
			if ( emit_signal )
			{
				signalElementModified().emit("unit", next, DELETED);
				signalElementModified().emit("unit", prev, UPDATED);
			}
		}
	}
}

void DataModel::deleteBackground(const string& id, bool emit_signal)
{
	bool with_children =false;
	string diag;
	if (!checkDeletionRules(id, with_children, diag))
	{
		MSGOUT << "deleteBackground:> can't delete " << id << endl ;
		return ;
	}

	//const string& type = GetAnnotationType(id);

	try
	{
		//> set background as inactive
		if (hasElementProperty(id,"type"))
			agDeleteFeature(id,"type") ;
		if (hasElementProperty(id,"level"))
			agDeleteFeature(id,"level") ;

		AnnotationId previous = getPreviousElementId(id) ;
		AnnotationId next = getNextElementId(id) ;

		std::vector<string> resized ;
		std::vector<string> deleted ;
		bool resized_prev = false ;

		AnchorId end = GetEndAnchor(id) ;

		//> if previous is a padding background, as we're deleting the current
		// (in real making the current to be a padding one - i.e empty one)
		// we'll have to join them in a single padding one
		if (!previous.empty() && getElementType(previous)=="background") {
			if ( getElementProperty(previous, "type", "none") == "none" ) {
				AnchorId end_previous = GetEndAnchor(previous) ;
				agDeleteAnnotation(id) ;
				TRACE_D << "----> delete annotation id" << id << endl ;
				agSetEndAnchor(previous,end) ;
				TRACE_D << "----> set end anchor of " << previous << " at " << end << endl ;
				agDeleteAnchor(end_previous) ;
				TRACE_D << "----> delete anchor " << end_previous << endl ;
				deleted.push_back(id) ;
				resized_prev = true ;
				resized.push_back(previous) ;
			}
		}

		AnchorId new_current ;
		if (resized_prev)
			new_current = previous ;
		else
			new_current = id ;

		//> do same action with the next one if exists
		if (!next.empty() && getElementType(next)=="background") {
			if ( getElementProperty(next, "type", "none") == "none" ) {
				AnchorId start_current = GetStartAnchor(new_current) ;
				AnchorId start_next = GetStartAnchor(next) ;
				agDeleteAnnotation(new_current) ;
				TRACE_D << "----> delete annotation " << new_current << endl ;
				agSetStartAnchor(next,start_current) ;
				TRACE_D << "----> set start anchor of " << next << " at " << start_current << endl ;
				agDeleteAnchor(start_next) ;
				TRACE_D << "----> delete anchor " << start_next << endl ;
				deleted.push_back(new_current) ;
				resized.push_back(next) ;
			}
		}

		setUpdated(true) ;

		if ( emit_signal ) {
			std::vector<string>::iterator it ;
			for ( it = deleted.begin(); it != deleted.end(); it++ )
				emitSignal("background", *it, DELETED);
			for ( it = resized.begin(); it != resized.end(); ++it )
				emitSignal("background", *it, RESIZED);
			//> If no merge done with any inactive background around id,
			// that means that we just "deleted" id, means make it as an
			// inactive background -> we have to update its display
			if (deleted.size()==0 && resized.size()==0)
				emitSignal("background", id, UPDATED);
		}
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
}

/**
 * check deletion rules for given segment id:
 *   - can't delete start segment of main branch
 *   - if deleting an overlapping turn, then also delete "empty" childs
 *   - if fast transcription, also delete childs
 * @param id segment id
 * @param with_children (returned) children annotations also to be deleted
 * @param diagnosis (returned) eventual check failure diagnosis
 * @return true if segment can be deleted, else false
 *
 */
bool DataModel::checkDeletionRules(const std::string& id, bool& with_children, string& diagnosis, bool forceFirst)
{
	bool first_of_overlap = false;

	diagnosis="";
	if ( !ExistsAnnotation(id) )
	{
		return false;
	}
	const string& type = GetAnnotationType(id);
	const string& graphtype = getGraphType(id);

	if ( ! isMainstreamType(type, graphtype) )
		return true;


	if ( m_conventions.fastAnnotationMode() )
		with_children = true;
	else
		with_children = false;

	if (getPreviousElementId(id) == "")
	{
		// if first in graph and no overlapping element -> can't delete (except for background),
		//  else allow delete (overlapped will become 0-order element)
		if (getOrder(id) == 0)
		{
			const vector<string>& over_ids = getElementsWithSameStart(id, type);
			if  ( over_ids.size() == 0 && graphtype!="background_graph")
			{
				if (forceFirst)
					return true ;

				char msg[120];
				sprintf(msg, _("can't delete first %s"), m_conventions.getLocalizedLabel(GetAnnotationType(id)).c_str());
				diagnosis = msg;
				return false;
			}
			else if (over_ids.size() != 0)
			{
				if ( m_conventions.maxOverlap(type)  == 0 ) {}
				with_children = true;
			}
		}
	}

	return true;
}


//
// delete anchor if no more incoming or outgoing annotations attached to it
void DataModel::deleteUnusedAnchor(const std::string& anchor)
{
	if (ExistsAnchor(anchor) )
	{
		const set<AnnotationId>& ids = GetIncomingAnnotationSet(anchor, "");
		if (ids.size() == 0)
		{
			const set<AnnotationId>& ids2 =
					GetOutgoingAnnotationSet(anchor, "");
			if (ids2.size() == 0)
			{
				std::map<string, int>::iterator it = m_anchorTrack.find(anchor);
				if (it != m_anchorTrack.end() )
					m_anchorTrack.erase(it);
				agDeleteAnchor(anchor);
				setUpdated(true) ;
			}
		}
	}
}

//
// check if anchor has incoming / outgoing arcs other than given id
bool DataModel::anchorHasOtherAttachedElements(const string& anchor,
		const string& id)
{
	const set<AnnotationId>& cids = GetOutgoingAnnotationSet(anchor, "");
	set<AnnotationId>::const_iterator it;
	for (it=cids.begin(); it!= cids.end(); ++it)
		if ( *it != id)
			return true;

	const string& previd = getPreviousElementId(id); // normally attached element

	const set<AnnotationId>& cids2 = GetIncomingAnnotationSet(anchor, "");
	for (it=cids2.begin(); it!= cids2.end(); ++it)
		if ( *it != previd)
			return true;

	return false;
}

/*========================================================================
 *
 *  Merge Annotations
 *
 *========================================================================*/

string DataModel::mergeAnnotations(const string& baseId, const string& mergedId, bool append, bool delayed_del, bool replace_anchor, bool emit_signal)
{
	vector<string> lock_features ;
	return mergeAnnotations(baseId, mergedId, lock_features, append, delayed_del, replace_anchor, emit_signal) ;
}

/**
 * merge 2 annotations with same type by deleting one and overwritting / merging properties
 * @param  baseId 						Id of annotation to be kept
 * @param  mergedId 					Id of annotation to be merged
 * @param  append  						If false, merged properties replace base properties values, else they are appended to base properties values
 * @param  delayed_del 					If true, merged annotation will not be deleted
 * @param  keep_attachments				If true, will adjust attachments
 * @param  emit_signal					If true, will emit an UPDATED signal for baseId
 * @return 								deleted Annotation id
 *
 * @note if base annotation properties that do not exists in merged annotation are left unchanged
 * @note merged annotation is deleted, as well as its start anchor if unused, and base annotation end is set to merged end.
 */
string DataModel::mergeAnnotations(const string& baseId, const string& mergedId, const vector<string>& lock_features, bool append, bool delayed_del, bool keep_attachments, bool emit_signal)
{
	if ( GetAnnotationType(baseId) != GetAnnotationType(mergedId) )
		throw "(DataModel::mergeAnnotations) merged annotations must have the same type !!";

	TRACE_D << " >mergeAnnotations  baseId=" << baseId << " (" << GetAnnotationInfo(baseId)  << ") and "
			<<   mergedId << " ("<< GetAnnotationInfo(mergedId) <<")" << endl;

	// merge all annotations properties
	const map<string, string> props = GetFeatures(mergedId);
	map<string, string>::const_iterator itp;
	for ( itp = props.begin(); itp != props.end(); ++itp)
	{
		if ( find(lock_features.begin(), lock_features.end(), itp->first) == lock_features.end() )
		{
			string prop = getElementProperty(baseId, itp->first, "");
			if ( append && itp->first != "subtype" )
			{
				if ( ! (prop.empty() || itp->second.empty()) )
					prop += " ";
				prop += itp->second;
			}
			else
				prop = itp->second;
			agSetFeature(baseId, itp->first, prop);
		}
	}

	if ( ! delayed_del )
	{
		if (emit_signal)
			signalElementModified().emit( getElementType(mergedId), mergedId, DELETED) ;

		// delete merged annotation
		string mergedEnd = GetEndAnchor(mergedId);
		string baseEnd = GetEndAnchor(baseId);

		// - 1 - Adjust attachments of mutual anchor (baseId end anchor / mergedId start anchor)
		if (keep_attachments)
			mergeAnnotationAnchors(baseId, mergedId, false) ;

		// - 2 - Merge by deletion
		agSetEndAnchor(baseId, mergedEnd);
		agDeleteAnnotation(mergedId);

		// - 3 - Clean anchor
		deleteUnusedAnchor(baseEnd);

		if (emit_signal)
			signalElementModified().emit( getElementType(baseId), baseId, UPDATED) ;

		return mergedId;
	}
	return "";
}

/*
 *	To be called before changing anchors of annotation.
 *	Will adjust incoming and outgoing anchors
 *
 *	<!> WARNING <!> : This method WON'T CHANGE baseId & mergedId ATTACHMENT
 */
void DataModel::mergeAnnotationAnchors(const string& baseId, const string& mergedId, bool withOffset)
{
	const string& baseStart = GetStartAnchor(baseId) ;
	const string& baseEnd = GetEndAnchor(baseId) ;

	const string& mergedStart = GetStartAnchor(mergedId) ;
	const string& mergedEnd = GetEndAnchor(mergedId) ;

	// All incoming at merging point (base end/ merged start) will become incoming at furture merged end
	std::set<string> incoming = GetIncomingAnnotationSet(baseEnd, "") ;
	std::set<string>::iterator it_in ;
	for (it_in=incoming.begin(); it_in!=incoming.end(); it_in++)
	{
		if (*it_in !=baseId && *it_in!=mergedId)
			setAnchor(*it_in, mergedEnd, false) ;
	}

	// All outgoing of of merging point (base end/ merged start) will becore outgoing at future merged start
	std::set<string> outgoings = GetOutgoingAnnotationSet(baseEnd, "") ;
	std::set<string>::iterator it_out ;
	for (it_out=outgoings.begin(); it_out!=outgoings.end(); it_out++)
	{
		if (*it_out !=baseId && *it_out!=mergedId)
			setAnchor(*it_out, baseStart, true);
	}

	if ( withOffset && GetAnchored(baseEnd) )
		setAnchorOffset(mergedEnd, GetAnchorOffset(baseEnd)) ;
}


/*========================================================================
 *
 *  Update data in graph
 *
 *========================================================================*/

//
// update given segment qualifier (type + desc
//
std::string DataModel::setQualifier(string qid, const string& qtype,
		const string& desc, bool emit_signal)
{
	bool updated = false;

	const string& graphtype = getGraphType(qid);

	string parent_id = getParentElement(qid, m_conventions.mainstreamBaseType(graphtype));
	const string& checkId= getQualifierId(parent_id, qtype, desc);

	if ( !checkId.empty() && checkId != qid)
		throw _("Same qualifier already exists for current segment");

	AnchorId start = GetStartAnchor(qid);
	AnchorId stop = GetEndAnchor(qid);
	const string& current_type = GetAnnotationType(qid);

	// -- Different type ?
	if (current_type != qtype)
	{
		// -- We must delete & recreate qualifier annotation with proper type
		// because AGAPI doesn't allow to change annotation type
		const string& order = getElementProperty(qid, "order");
		if (ExistsAnnotation(qid) )
			agDeleteAnnotation(qid);
		qid = agCreateAnnotation(qid, start, stop, qtype);
		if ( !order.empty() )
			setElementProperty(qid, "order", order);
		updated = true;
	}

	if (desc != getElementProperty(qid, "desc", "") )
	{
		updated = true;
		if (desc.empty() )
		{
			if (ExistsFeature(qid, "desc") )
				agDeleteFeature(qid, "desc");
		}
		else
			agSetFeature(qid, "desc", desc);
	}

	if (updated && emit_signal)
	{
		// emit signal for parent segment
		setUpdated(true) ;
		emitSignal(qid, UPDATED);
	}

	return qid;
}

std::string DataModel::setQualifier(string qid, const string& qtype, const string& desc, const string& norm, bool emit_signal)
{
	bool updated = false;

	const string& graphtype = getGraphType(qid);

	string parent_id = getParentElement(qid, m_conventions.mainstreamBaseType(graphtype));
	const string& checkId= getQualifierId(parent_id, qtype, desc);

	if ( !checkId.empty() && checkId != qid)
		throw _("Same qualifier already exists for current segment");

	AnchorId start = GetStartAnchor(qid);
	AnchorId stop = GetEndAnchor(qid);
	const string& current_type = GetAnnotationType(qid);

	if (current_type != qtype)
	{
		const string& order = getElementProperty(qid, "order");
		// we must then delete & recreate qualifier annotation with proper type
		if (ExistsAnnotation(qid) )
			agDeleteAnnotation(qid);
		qid = agCreateAnnotation(qid, start, stop, qtype);
		if ( !order.empty() )
			setElementProperty(qid, "order", order);
		updated = true;
	}

	if (desc != getElementProperty(qid, "desc", "") )
	{
		updated = true;
		if (desc.empty() )
		{
			if (ExistsFeature(qid, "desc") )
				agDeleteFeature(qid, "desc");
		}
		else
			agSetFeature(qid, "desc", desc);
	}

	if (norm != getElementProperty(qid, "norm", "") )
	{
		updated = true;
		if (norm.empty() )
		{
			if (ExistsFeature(qid, "norm") )
				agDeleteFeature(qid, "norm");
		}
		else
			agSetFeature(qid, "norm", norm);
	}

	if (updated && emit_signal)
	{
		// emit signal for parent segment
		setUpdated(true) ;
		emitSignal(qid, UPDATED);
	}

	return qid;
}


//
// retrieve qualifier annot id from parent segment id, qualifier type & qualifier desc
string DataModel::getQualifierId(const string& pid, const string& qtype,
		const string& qdesc)
{
	vector<string> v;

	getQualifiers(pid, v, qtype, true, false);
	if (v.size() > 0)
	{
		vector<string>::iterator it;
		for (it = v.begin(); it != v.end(); ++it) {
			if (getElementProperty(*it, "desc", "") == qdesc)
				return *it;
		}
	}
	return "";
}


/* update segment anchors */
void DataModel::setSegmentOffsets(const string& id, float start_offset, float stop_offset, bool use_epsilon, bool emit_signal)
{
	try
	{
		const std::string& start = GetStartAnchor(id);
		const std::string& stop = GetEndAnchor(id);

		// we just need to set anchors offset to new offset
		setAnchorOffset(stop, stop_offset, use_epsilon, emit_signal) ;
		setAnchorOffset(start, start_offset, use_epsilon, emit_signal) ;
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
}

/**
 * check that resizing given segment will not make other segments
 *   anchoring inconsistent
 * @param id id of segment to be resized
 * @param start_offset new start offset for segment (in secs.)
 * @param end_offset new end offset for segment (in secs.)
 * @param diagnosis (return) diagnosis if check failed
 * @param check_over if true also check eventual overlapping segments resizing
 * @return true if valid resizing request, else false, stating in diagnosis why resizing was refused.
 *
 * @note
 *    Overlapping may be constrained by annotation conventions.
 *
 *  if resized segment has overlapping segments and conventions specify that
 *   overlapping segments must be aligned on overlapped ones, then resize rules are also checked on overlapping segments.
 *
 */

bool DataModel::checkResizeRules(const string& id, float start_offset, float end_offset, string& diag, bool check_over, bool fromLink, bool allowUnanchored)
{
	Log::out() << " ~~~~~~~~~ CheckResizeRule for id=" << id << " fromLink=" << fromLink << " start=" << start_offset << " end=" << end_offset << std::endl ;

	bool ok = true;
	const string& graphtype = getGraphType(id);

	ok = checkElementFrontiersResizable(id, start_offset, end_offset, diag) ;
	if (!ok)
		return false ;

	// -- Check if the offset changement beeing done is more than min value
	if ( fabs(start_offset - GetStartOffset(id)) >= EPSILON)
	{
		// if offset is negative, check if we allow or not
		if (allowUnanchored && start_offset<0)
			ok = true ;
		else
			ok = checkRemainingMinsize(m_conventions.mainstreamBaseType(graphtype), -1,	GetStartAnchor(id), start_offset, diag);
	}

	if (ok)
	{
		if ( (fabs(end_offset - GetEndOffset(id)) < EPSILON) || (allowUnanchored && end_offset<0) )
			ok = true;
		else
			ok = checkRemainingMinsize(m_conventions.mainstreamBaseType(graphtype), -1, GetEndAnchor(id), end_offset, diag);
	}

	string alignmentKey = graphtype + ",overlap,alignment" ;
	if (ok && check_over && m_conventions.getConfiguration(alignmentKey) == "true")
	{
//		TRACE_D << "NEED for check_over in CheckResizeRules" << endl;
		const string& type = GetAnnotationType(id);
		if ( m_conventions.maxOverlap(type) > 0)
		{
			const vector<string>& over = getElementsWithSameStart(id, type);
			vector<string>::const_iterator it_over;
			for (it_over = over.begin(); ok && it_over != over.end(); ++it_over)
				ok = checkResizeRules(*it_over, start_offset, end_offset, diag, false, false, allowUnanchored);
		}
	}

	if (ok)
	{
		if (!fromLink)
			m_anchorLinks.tmpClearBuffer() ;

		ok = checkResizeRulesForLinks(id, start_offset, end_offset, diag, check_over, allowUnanchored) ;
	}

	return ok;
}

bool DataModel::checkResizeRulesForLinks(const string& id, float start_offset, float end_offset, string& diag, bool check_over, bool allowUnanchored)
{
	Log::trace() << "\t~~~~~~~~~ CheckResizeRuleForLinks for id=" << id << " start=" << start_offset << " end=" << end_offset << std::endl ;

	bool allOk = true ;

	//> -- Determine which anchor is being resized
	bool start = (getElementOffset(id, true) != start_offset) ;
	AnchorId toBeChecked ;
	float modifiedOffset ;
	if (start)
	{
		toBeChecked = getStartAnchor(id) ;
		modifiedOffset = start_offset ;
	}
	else
	{
		toBeChecked = getEndAnchor(id) ;
		modifiedOffset = end_offset ;
	}

	//> -- Flag as checked for avoiding a checking loop
	if ( ! m_anchorLinks.tmpFlagAnchor(toBeChecked) )
	{
		Log::trace() << "\t\t Links have been already checked for anchor " << toBeChecked << ", exit." << std::endl ;
		return allOk ;
	}

	//> -- Checks whether any links exist
	if ( !m_anchorLinks.hasLinks(toBeChecked) )
		return allOk ;

	Log::trace() << "\tChecking links for anchor " << toBeChecked << std::endl ;

	//> -- For all linked anchors, let's proceed
	std::set<std::string> linksSet = m_anchorLinks.getLinks(toBeChecked) ;
	std::set<std::string>::iterator it = linksSet.begin() ;
	while ( it!=linksSet.end() && allOk )
	{
		Log::trace() << "\tCandidate link " << *it << std::endl ;
		//> -- Flag as checked for avoiding a checking loop
		if ( ! m_anchorLinks.tmpIsFlaggedAnchor(*it) )
		{
			//> -- Proceed only with anchors from different graph
			const string& graphtype = getGraphType(*it);

			const string& main = getMainstreamStartingAtAnchor(*it, graphtype) ;
			if (!main.empty())
				allOk = checkResizeRules(main, modifiedOffset, getEndOffset(main), diag, check_over, true, allowUnanchored) ;
			else
			{
				allOk = false ;
				diag = "unable to find mainstream element for anchor " + toBeChecked ;
			}
		}
		else
			Log::trace() << "\t\t Linked anchor " << *it << " has been already checked, see next." << std::endl ;

		it++ ;
	}
	return allOk ;
}

std::string DataModel::getMainstreamStartingAtAnchor(const string& anchorid, const string& graphtype)
{
	if (graphtype.empty())
	{
		Log::err() << "getMainstreamStartingAtAnchor ~> unable to find corresponding mainstream" << std::endl ;
		return "" ;
	}

	std::set<std::string> annot = GetOutgoingAnnotationSet(anchorid, mainstreamBaseType(graphtype) ) ;
	std::set<std::string>::iterator it ;
	for (it=annot.begin(); it!=annot.end(); it++)
	{
		if (getOrder(*it)==0)
			return (*it) ;
	}

	return "" ;
}

std::string DataModel::getElementStartingAtAnchor(const string& anchorid, const string& type, int order)
{
	std::set<std::string> annot = GetOutgoingAnnotationSet(anchorid, type ) ;
	std::set<std::string>::iterator it ;
	for (it=annot.begin(); it!=annot.end(); it++)
	{
		if (getOrder(*it)==order)
			return (*it) ;
	}
	return "" ;
}

bool DataModel::checkElementFrontiersResizable(const string& id, float new_start, float new_end, string& err)
{
	if (id.empty() || !existsElement(id)) {
		TRACE << "checkElementFrontiersResizable:> can't find element " << id << std::endl ;
		return false ;
	}
	else {
		float start = getElementOffset(id, true) ;
		float end = getElementOffset(id, false) ;
		bool resizing_first = (new_start>0 && start==0 && (fabs(start - new_start)>=EPSILON ) )  ;
		bool resizing_last = (new_end>0 && end==m_signalLength && (fabs(new_end-end)>=EPSILON ) ) ;
		if (resizing_first)
		{
			err = _("You can't change the start time") ;
			return false ;
		}
		else if (resizing_last)
		{
			err = _("You can't change the end time") ;
			return false ;
		}
		else
			return true ;
	}
}

/**
 *  check that neighbour segment would still be greater than m_conventions.minSegmentSize(graphtype) if
 *    given anchor was moved to given signal offset.
 *  @param type checked segment type
 *  @param order checked overlapping order, or -1 if any order checked
 *  @param anchor moved anchor
 *  @param signal_offset target signal offset
 *  @param diagnosis (return) check failure diagnosis
 *  @return true if move valid, else false
 *
 */
bool DataModel::checkRemainingMinsize(const string& type, int order,
		const string& anchor, float signal_offset, string& diag)
{
	char msg[120];
	bool ok_forw = true;
	bool ok_down = true;

	set<AnnotationId>::const_iterator it;
	float anchor_offset = GetAnchorOffset(anchor);
	msg[0] = 0;

	bool newly_anchored = !GetAnchored(anchor) ;

	//> -- Move enough important ?
	if (fabs(anchor_offset - signal_offset) > EPSILON || newly_anchored )
	{
		const string& graphtype = m_conventions.getGraphtypeFromType(type);

		//> -- Resizing in forward time
		// --> check with next anchor time (the previous is ok_forw since we're already anchored)
		if ( (anchor_offset - signal_offset) < 0 || newly_anchored )
		{
			const set<AnnotationId>& ids = GetOutgoingAnnotationSet(anchor,	type);
			set<AnnotationId>::const_iterator it = ids.begin();

			for (; ok_forw && it != ids.end(); ++it)
			{
				if (order >= 0 && getOrder(*it) != order)
					continue;
				float next_offset = getEndOffset(*it);

/*
				 // it may happen due to overlapping turns that cursor position
				 // is after 1st segment found, if overlaping turn has been
				 // divided in several segments. In such a case, check with
				 //  next segment id
				 while ( next_offset >= 0.0 && next_offset < signal_offset ) {
				 // check if next segment belongs to same parent
				 const string& parent = getParentElement(id);
				 const string& nextid = getNextAnchoredElementId(id);
				 if ( nextid == "" ) break;
				 if ( getParentElement(nextid) == parent ) {
				 id = nextid;
				 next_offset = getBaseSegmentEndOffset(id);
				 } else break;
				 }
*/

				// forbid that new segment frontier goes after corresponding base segment end
				float diff = (next_offset - signal_offset) ;
				if (diff < m_conventions.minSegmentSize(graphtype))
				{
					if (diff < 0)
						sprintf(msg, _("Can't set segment end beyond previous/next segment"));
					else
						sprintf(msg, _("segment size must be > %f secs"), m_conventions.minSegmentSize(graphtype)) ;
					ok_forw = false;
				}
			}
		}
		//> -- Resizing in backward time
		if ( (anchor_offset - signal_offset) >= 0 || newly_anchored )
		{
			const set<AnnotationId>& ids = GetIncomingAnnotationSet(anchor, type);
			set<AnnotationId>::const_iterator it = ids.begin();
			/*			if ( ids.size() > 0 ) {  */
			for (; ok_down && it != ids.end(); ++it)
			{
				if (order >= 0 && getOrder(*it) != order)
					continue;
//				TRACE_D << " RSZ  UPWARD = " << *it << endl;
				float next = getStartOffset(*it);
				if (next >= 0.0)
				{
					float diff = (signal_offset - next);
					if (diff < m_conventions.minSegmentSize(graphtype))
					{
						if (diff < 0)
							sprintf(msg, _("Can't set segment end beyond previous/next segment")) ;
						else
							sprintf(msg, _("segment size must be > %f secs"), m_conventions.minSegmentSize(graphtype));
						ok_down = false;
					}
				}
			}
		}
	}

	diag = msg;

	return (ok_forw && ok_down) ;
}

/**
 *  set anchor offset in signal
 */
bool DataModel::setAnchorOffset(const std::string& anchorid, float offset,
		bool use_epsilon, bool emit_signal)
{
	try
	{
		bool was_anchored = GetAnchored(anchorid) ;
		if ( ! was_anchored
				|| fabs(GetAnchorOffset(anchorid) - offset) > (use_epsilon ? EPSILON : 0.0) )
		{
			agSetAnchorOffset(anchorid, offset);
			if ( !was_anchored && emit_signal)
				emitSignal("anchor", anchorid, ANCHORED);

			if ( emit_signal )
			{
				set<string> ids;
				set<string>::iterator it;
				getAnchorAttachments(anchorid, ids, false);
				setUpdated(true) ;
				for ( it = ids.begin(); it != ids.end(); ++it )
					emitSignal(*it, RESIZED);
			}
			return true;
		}
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << endl;
	}
	return false;
}

bool DataModel::unsetAnchorOffset(const std::string& anchorid, bool emit_signal)
{
	try
	{
		agUnsetAnchorOffset(anchorid) ;
		if ( emit_signal )
			emitSignal("anchor", anchorid, UNANCHORED);
		return true;
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << endl;
	}
	return false;
}

//
// retrieve all attachmnents on given anchor, both incoming and outgoing.
//
void DataModel::getAnchorAttachments(const string& anchorid, set<string>& att,
		bool with_qualifiers, bool with_incoming)
{
	set<AnnotationId>::const_iterator it;
	const set<AnnotationId>& ids = GetOutgoingAnnotationSet(anchorid, "");
	for (it = ids.begin(); it != ids.end(); ++it)
	{
		if (with_qualifiers || !isQualifierType(GetAnnotationType(*it)) )
			att.insert(*it);
	}

	if (with_incoming)
	{
		const set<AnnotationId>& ids2 = GetIncomingAnnotationSet(anchorid, "");
		for (it = ids2.begin(); it != ids2.end(); ++it)
		{
			if (with_qualifiers || !isQualifierType(GetAnnotationType(*it)) )
				att.insert(*it);
		}
	}
}

/*========================================================================
 *
 *  browse through graph - links and alignment, parent elements
 *
 *========================================================================*/

//
// return alignment element id for given element id
//  (which is an element of lower precedence in mainstream)
string DataModel::getAlignmentId(const string& id)
{
	try
	{
		const string& type = m_conventions.getAlignmentType(GetAnnotationType(id)) ;
		if ( ! type.empty() )
		{
			const string& astart = GetStartAnchor(id);
			const string& order = getElementProperty(id, "order");
//			TRACE_D << "getAlignmentId for " << GetAnnotationType(id) << " id=" << id << " -> " << type << " at anchor = " << astart << endl;
			const set<AnnotationId>& ids = GetOutgoingAnnotationSet(astart, type);
			set<AnnotationId>::const_iterator it;
			for ( it = ids.begin(); it != ids.end(); ++it )
			{
				if ( getElementProperty(*it, "order") == order )
					return *it;
			}
		}
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl ;
	}
	return "";
}

/**
 * get next (signal-anchored) element id with same type for given element id
 * @param id element id
 * @return next element id
 */
string DataModel::getNextAnchoredElementId(const string& id)
{
	try
	{
		const string& type = GetAnnotationType(id);
		const string& graphtype = getGraphType(id);

		if ( ! isMainstreamType(type, graphtype) )
			return getNextAnchoredElementId(getMainstreamStartElement(id));

		// browse through graph
		string start_id = id;
		const string& order = getElementProperty(id, "order");
		while ( !start_id.empty() )
		{
			const AnchorId& anchor = GetEndAnchor(start_id);
			const set<AnnotationId>& next = GetOutgoingAnnotationSet(anchor, type);
			set<AnnotationId>::const_iterator it;
			start_id = "";
			for ( it=next.begin(); it!= next.end(); ++it)
			{
				if (getElementProperty(*it, "order") == order )
				{
					if ( GetAnchored(anchor) ) return (*it);
					else start_id = (*it);
				}
			}
		}

		//}
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return "";
}

/**
 *  return background segement  starting / ending during speech segment of given id
 *  @param id speech segment id
 *  @param v (return) vector holding background segment ids
 *  @param mode 0:starting in id range  1: ending in id range 2: starting or ending in id range
 *
 */

void DataModel::getBackgroundsInSegment(const string& id, vector<string>& v, int mode)
{
	string graphtype = "background_graph";
	if ( ! hasAnnotationGraph(graphtype) ) return ;

	// get start and end offset of element
	const string& start_anchor = GetStartAnchor(id);
	const string& end_anchor = GetEndAnchor(id);
	bool seg_start_anchored = GetAnchored(start_anchor) ;
	bool seg_end_anchored = GetAnchored(end_anchor) ;

	float start_offset, end_offset ;

	//> If we're searching start of background, we must be in a segment
	// anchored at start (because we place background symbol at beginning
	// of the segment)
	//> Then we need to have anchored range in which searching start of background,
	// so check if current id is end-anchored
	if (mode==0) {
		if (!seg_start_anchored)
			return ;
		else {
			start_offset = GetAnchorOffset(start_anchor) ;
			if (seg_end_anchored)
				end_offset = GetAnchorOffset(end_anchor) ;
			else
				end_offset = getEndOffset(id) ;
		}
	}
	//> End background symbols are displayed at end of segment, so
	//  we need to have end anchored
	else {
		if (!seg_end_anchored)
			return ;
		else {
				end_offset = GetAnchorOffset(end_anchor) ;
			if (seg_start_anchored)
				start_offset = GetAnchorOffset(start_anchor) ;
			else
				start_offset = getStartOffset(id) ;
		}
	}

	// get showId of researched background
	int notrack = getElementSignalTrack(id) ;

	// get all background segments of given graph between start and end
	getSegmentsInRange(m_agIds[graphtype], mainstreamBaseType(graphtype), notrack, start_offset, end_offset, v, mode) ;

	// now filter "inactive" backgrounds, ie. those with no "type" or "type=none" feature
	vector<string>::iterator itv;
	for (itv = v.begin(); itv != v.end(); ) {
		if ( getElementProperty(*itv, "type", "none") == "none" ) itv = v.erase(itv);
		else ++itv;
	}
}

// return background of given type incoming / outgoing at segment of given id
// mode: 0:starting in id range  1: ending in id range 2: starting or ending in id range, 3: included in range
void DataModel::getSegmentsInRange(const string& graphId, const string& type,
		int notrack, float start_offset, float end_offset,
		vector<string>& v, int mode)
{
	if (start_offset<0 || end_offset <0)
		return ;

	try
	{
		std::set<AnnotationId> bgs = GetAnnotationSet(graphId, type) ;
		std::set<AnnotationId>::iterator it ;
		for (it=bgs.begin(); it!=bgs.end(); it++)
		{
			if ( getElementSignalTrack(*it) == notrack ) {
				bool to_add = false;
				if ( mode==0 || mode>=2 ) {
					float start = GetStartOffset(*it) ;
					to_add = (start_offset<=start && start<end_offset) ;
				}
				if ( (mode == 3 && to_add) || (!to_add && (mode==1 || mode==2)) ) {
					float end = GetEndOffset(*it) ;
					to_add = ( start_offset<end && end<=end_offset);
				}
				if ( to_add ) v.push_back(*it) ;
			}
		}
	}
	catch (AGException& e) 	{
		MSGOUT << "Caught AGException : " << e.error() << endl;
	}
}

bool DataModel::hasQualifiers(const string& id)
{
	vector<string> unused_v ;
	return getQualifiers(id, unused_v, "", true, true) ;
}

// return qualifiers of given type incoming / outgoing at segment of given id
bool DataModel::getQualifiers(const string& id, vector<string>& v,
		const string& type, bool starting_at, bool onlyCheckExistence)
{
	v.clear();

	try
	{
		set<AnnotationId>::const_iterator it;
		const string& order = getElementProperty(id, "order");
		const string& graphtype = getGraphType(id);

		//> -- Get qualifiers at STARTING anchor
		if ( starting_at )
		{
			const string& anchor = GetStartAnchor(id);
			const set<AnnotationId>& ids = GetOutgoingAnnotationSet(anchor, type);
			// -- Use order
			if ( m_conventions.sortQualifiersForLayout() )
			{
				vector<string> v2;
				const string& endanchor = GetEndAnchor(id);
				for ( it = ids.begin(); it != ids.end(); ++it )
				{
					// If no type specified, take all except mainstream
					// If type specified, we're sure that the set only contains element of
					// that type
					if ( getElementProperty(*it,"order") == order
							&& ( !type.empty()
									|| (!isMainstreamType(GetAnnotationType(*it), graphtype) )))
					{
						const string& aend = GetEndAnchor(*it);
						// -- instantaneous
						if ( aend == anchor )
							v2.push_back(*it);
						// -- Complete overlap
						else if ( aend == endanchor )
							v2.insert(v2.begin(), *it);
						// -- Only start
						else
							v.push_back(*it);

						if ( onlyCheckExistence && ( !v.empty() || !v2.empty() ) )
							return true ;
					}
				}
				copy(v2.begin(), v2.end(), back_inserter(v));
			}
			// -- Whatever order
			else
			{
				for ( it = ids.begin(); it != ids.end(); ++it )
				{
					if ( getElementProperty(*it,"order") == order
							&& (!type.empty() || (!isMainstreamType(GetAnnotationType(*it), graphtype)) ) )
					{
						v.push_back(*it);
						if ( onlyCheckExistence)
							return true ;
					}
				}
			}
		} // END of start anchor case

		//> -- Get qualifier at ENDING anchor
		else
		{
			const string& anchor = GetEndAnchor(id);
			const set<AnnotationId>& ids = GetIncomingAnnotationSet(anchor, type);
			// -- Use order
			if ( m_conventions.sortQualifiersForLayout() )
			{
				const string& startanchor = GetStartAnchor(id);
				vector<string> v2;
				for ( it = ids.begin(); it != ids.end(); ++it )
				{
					if ( getElementProperty(*it,"order") == order
							&& ( !type.empty() || (!isMainstreamType(GetAnnotationType(*it), graphtype))) )
					{
						const string& astart = GetStartAnchor(*it);
						// -- Skip instantaneous events
						if ( astart != anchor )
						{
							// -- Complete overlap
							if ( astart == startanchor )
								v.insert(v.begin(), *it);
							else
								v2.insert(v2.begin(), *it);

							if ( onlyCheckExistence && ( !v.empty() || !v2.empty() ) )
								return true ;
						}
					}
				}
				copy(v2.begin(), v2.end(), back_inserter(v));
			}
			//> -- Whatever order
			else
			{
				for ( it = ids.begin(); it != ids.end(); ++it )
				{
					if ( getElementProperty(*it,"order") == order
							&& ( !type.empty() || (!isMainstreamType(GetAnnotationType(*it), graphtype) ) ) )
					{
						// -- Skip instantaneous events
						if ( GetStartAnchor(*it) != anchor )
						{
							v.push_back(*it);
							if ( onlyCheckExistence )
								return true ;
						}
					}
				}
			}
		}// END of end anchor case

		return (!v.empty()) ;
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
		return false ;
	}
}

/**
 * return "parent" element in graph
 * @param id annotation id
 * @param with_same_start if true, parent must be attached to element start anchor, else returns ""
 * @return parent element id / "" if no parent found
 *
 */
string DataModel::getParentElement(const string& id, bool with_same_start)
{
	if ( id.empty() )
		return _novalue;
	const string& atype = GetAnnotationType(id);
	const string& graphtype = getGraphType(id);
	const string& ptype = m_conventions.getParentType(atype, graphtype);

	if ( ptype.empty() )
		return _novalue;

	return getParentElement(id, ptype, with_same_start, false);
}

/**
 * return "parent" element in graph
 * @param id annotation id
 * @param type target parent type / ""
 * @param with_same_start if true, parent must be attached to element start anchor, else returns ""
 * @return parent element id / "" if no parent found
 *
 */
string DataModel::getParentElement(const string& id, const string& type, bool with_same_start, bool check)
{
	try {

		if ( check ) {
			if ( id.empty() || type.empty() )
				return _novalue;
			const string& atype = GetAnnotationType(id);
			if ( type == atype ) {
				const string& agid = GetAGId(id);

				if ( type == m_agBaseType[agid] )
				{
					const string& pid = getAnchoredBaseTypeStartId(id);
					if ( !with_same_start || GetStartAnchor(pid) == GetStartAnchor(id) )
						return pid ;
					else
						return _novalue ;
				}
				else
					return id; // is it's own parent
			}
		}

		set<AnnotationId>::const_iterator it;
		int max_order = m_conventions.maxOrder(type);
		int order = getOrder(id);

		if ( with_same_start ) {
			const string& start = GetStartAnchor(id);
			const set<AnnotationId>& ids = GetOutgoingAnnotationSet(start, type);
			if ( ids.size() > 0 ) {
				for (it = ids.begin(); it != ids.end(); ++it ) {
					if ( max_order == 0 || getOrder(*it) == order )
						return *it;
				}
			}
			return _novalue;

		} else {

			string start = GetStartAnchor(id);
			const string& graphtype = getGraphType(id);
			string atype = GetAnnotationType(id);
			if ( ! m_conventions.isMainstreamType(atype, graphtype))
				atype = m_conventions.mainstreamBaseType(graphtype);

			while ( 1 )
			{
				// -- Let's see all outgoing element of wanted type
				// ==> if order match, it's good
				// Specific case when wanted type can't overlap but have orders
				// We keep the first 0 order we find as default candidate
				const set<AnnotationId>& ids = GetOutgoingAnnotationSet(start, type);
				if ( ids.size() != 0 )
				{
					for ( it = ids.begin(); it != ids.end(); ++it )
					{
						if ( max_order == 0 || getOrder(*it)==order )
							return *it;
					}
				}

				// -- Nothing found, let's go back a unit more and search again
				const set<AnnotationId>& prev = GetIncomingAnnotationSet(start, atype);
				if ( prev.size() == 0 )
				{
					//don't display, sometimes with undo/redo we can get here whereas parent isn't re-created yet
					//				MSGOUT << " Warn: couldn't find parent id for " << type << " id=" << id << endl;
					return _novalue;
				}
				start = GetStartAnchor(*(prev.begin()));
			}
		}

	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}

	return _novalue;
}


/**
 * return "parent" property value
 * @param id annotation id
 * @param name target property name
 * @param with_same_start if true, parent must be attached to element start anchor, else returns ""
 * @return parent property value / "" if not found
 * @note if current element doesn't own given property, try to get its value from parent elements in graph
 *
 */
string DataModel::getParentProperty(const string& id, const string& name, bool with_same_start)
{
	if ( id.empty() )
		return "";

	if ( hasElementProperty(id, name) ) return getElementProperty(id, name);
	return getParentProperty(getParentElement(id, "", with_same_start), name);
}

/**
 * returns true if id is first child of parent (have the same start anchor)
 */
bool DataModel::isFirstChild(const string& id, const string& parent)
{
	try
	{
		bool ok;
		if ( ! parent.empty() ) {
			ok  = (id != parent
						&&  GetStartAnchor(id) == GetStartAnchor(parent) ) ;
		} else {
			const string& type = GetAnnotationType(id);
			const string& graphtype = getGraphType(id);
			const string& ptype = m_conventions.getParentType(type, graphtype);
			const set<AnnotationId>& ids = GetOutgoingAnnotationSet(GetStartAnchor(id), ptype);
			ok = (ids.size() > 0);
		}
		return ok ;
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return false;
}

/**
 * returns true if id is the last child of parent (have the same end anchor)
 */
bool DataModel::isLastChild(const string& id, const string& parent)
{
	try
	{

		bool ok;
		if ( ! parent.empty() ) {
			ok  = (id != parent
						&&  GetEndAnchor(id) == GetEndAnchor(parent) ) ;
		} else {
			const string& type = GetAnnotationType(id);
			const string& graphtype = getGraphType(id);
			const string& ptype = m_conventions.getParentType(type, graphtype);
			const set<AnnotationId>& ids = GetIncomingAnnotationSet(GetEndAnchor(id), ptype);
			ok = (ids.size() > 0);
		}
		return ok ;
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return false;
}

bool DataModel::isUniqueChild(const string& id, const string&  parent)
{
	try
	{
		return isFirstChild(id, parent) && isLastChild(id, parent) ;
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return false;
}

/**
 * return ids of any element of given type with same start anchor as given id
 * @param id reference id
 * @param type searched type (defaults to reference element's type)
 * @return vector holding ids of elements found
 *
 * @note
 *   if id identifies an "overlapping" element (order > 0 ), then 0-order element
 *    is returned as first entry in vector
 *
 */

vector<string> DataModel::getElementsWithSameStart(const string& id, string type)
{
	vector<string> v;
	const string& order = getElementProperty(id, "order");
	const set<AnnotationId>& ids = GetOutgoingAnnotationSet(GetStartAnchor(id), type);
	set<AnnotationId>::const_iterator it;
	for (it = ids.begin(); it != ids.end(); ++it)
	{
		if ( *it == id)
			continue;
		if (order.empty() )
		{
			if (getElementProperty(*it, "order") != order)
				v.push_back(*it);
		}
		else
		{
			if (getElementProperty(*it, "order").empty() )
				v.insert(v.begin(), *it);
			else
				v.push_back(*it);
		}
	}
	return v;
}

/**
 * return ids of any element of given type with same end anchor as given id
 * @param id reference id
 * @param type searched type (defaults to reference element's type)
 * @return vector holding ids of elements found
 *
 * @note
 *   if id identifies an "overlapping" element (order > 0 ), then 0-order element
 *    is returned as first entry in vector
 */

vector<string> DataModel::getElementsWithSameEnd(const string& id, string type)
{
	vector<string> v;
	const string& order = getElementProperty(id, "order");
	const string& graphtype = getGraphType(id);

	string endAnchor ;
//if ( type == m_conventions.mainstreamBaseType(graphtype) )
//	endAnchor = getEndAnchor(id);
//else
//	endAnchor = GetEndAnchor(id);

	endAnchor = GetEndAnchor(id);

	const set<AnnotationId>& ids = GetIncomingAnnotationSet(endAnchor, type);
	set<AnnotationId>::const_iterator it;
	for (it = ids.begin(); it != ids.end(); ++it)
	{
		if ( *it == id)
			continue;
		if (order.empty() )
		{
			if (getElementProperty(*it, "order") != order)
				v.push_back(*it);
		}
		else
		{
			if (getElementProperty(*it, "order").empty() )
				v.insert(v.begin(), *it);
			else
				v.push_back(*it);
		}
	}
	return v;
}

/**
 *  return next segment with start offset greater than or equal to given offset for given type and track
 */
float DataModel::getNextSegmentStartOffset(const std::string& type,
		int notrack, float offset)
{
	float next = m_signalLength;

	const string& graphtype = m_conventions.getGraphtypeFromType(type);
	const string& graphId = m_agIds[graphtype];

	const list<std::string>& ids = GetAnnotationSeqByOffset(graphId, offset,
			0.0, type);
	list<std::string>::const_iterator it;
	for (it=ids.begin(); it != ids.end(); ++it)
		if (notrack == -1 || getElementSignalTrack(*it) == notrack)
		{
			next = GetStartOffset(ids.front());
			break;
		}
	return next;
}


/*========================================================================
 *
 *  browse through graph - segments
 *
 *========================================================================*/

/**
 * retrieve all segments of given type with a start time within given time interval
 * @param type searched type
 * @param v (returned) signal segments found within time interval
 * @param start time range start
 * @param stop time range end
 * @param notrack  if -1, retrieve segments for all signal tracks, else retrieve segments only for specified signal track no
 * @param anchored_only if true, retrieve only segments with signal-anchored start node (default)
 * @param follow_graph if true, follow graph links to retrieve end-anchored signal segments (default)
 * @return true if ok, else false
 *
 * @note
 *   if stop == 0.0, will retrieve all segments up to signal end.
 *   returned segments are ordered by (start_offset / order / notrack)
 */

bool DataModel::getSegments(string type, vector<SignalSegment>& v, float start,
		float stop, int notrack, bool anchored_only, bool follow_graph)
{
	const string& graphtype = m_conventions.getGraphtypeFromType(type);
	const string& graphId = m_agIds[graphtype];

	try
	{
		if ( stop > 0.0 )
			stop -= EPSILON;
		const list<std::string>& ids = GetAnnotationSeqByOffset(graphId, start, stop, type);
		list<std::string>::const_iterator it;
		v.push_back(SignalSegment(""));
		float last_start = start;
		for ( it = ids.begin(); it != ids.end(); ++it )
		{
			if ( stop == 0.0 || (last_start < (stop - EPSILON) ) )
			{
				if ( getSegment(*it, v.back(), notrack, anchored_only, follow_graph) )
				{
					if ( v.back().getStartOffset() >= 0 )
						last_start = v.back().getStartOffset();
			  	   	v.push_back(SignalSegment(""));
				}
			}
		}
		v.resize(v.size() - 1);
		sort(v.begin(), v.end());
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ",  type=" << type << endl;
		return false;
	}
	return true;
}

/**
 * retrieve all segments of given type with a start time within parent segment time interval
 * @param type searched type
 * @param v (returned) signal segments found within parent time range
 * @param parent parent signal segment
 * @return true if ok, else false
 *
 * @note
 *   returned segments are ordered by (start_offset / order / notrack)
 */

bool DataModel::getSegments(string type, vector<SignalSegment>& v,
		const SignalSegment& parent)
{
	return getSegments(type, v, parent.getStartOffset(), parent.getEndOffset(),
			parent.getTrack());
}

/**
 * get all segments of given type, ordered by graph links, starting at parent start node, ending at parent end node
 *
 *  @param type searched type
 *  @param v returned vector holding all elements found
 *  @param parent parent signal segment
 *  @return true if any child found, else false.
 *
 * @note
 *  the retrieval is done by browsing though graph links.
 *  If searched type is a mainstream or background type, then all type-arcs
 *  belonging to the subgraph starting at parent start node and ending at
 *  parent start node are retrieved
 *  if searched type is an "event" type
 *
 *   other arc types, like events, are anchored  on mainstreamBaseType()-typed arcs.
 *  thus, to retrieve arcs attached to a given graph, we must first retrieve corresponding
 *   "segment" arcs
 *  if parent end node is unanchored -> retrieve until end-anchored segment is found
 *
 */
bool DataModel::getChildSegments(string type, vector<SignalSegment>& v,
		const SignalSegment& parent)
{
	string id = parent.getId();
	AnchorId end = GetEndAnchor(id);
	float end_offset = (GetAnchored(end) ? GetAnchorOffset(end)
			: parent.getEndOffset());
	string seltype = type;
	const string& order = getElementProperty(id, "order");
	bool can_overlap = m_conventions.canOverlap(type);
	const string& graphtype = getGraphType(id);


	// if non-end-anchored segment but end offset given in parent -> set end to empty
	//  to force checking against parent end offset
	if ( !GetAnchored(end) && parent.getEndOffset() != -1.0)
		end = "";

	//Log::err() << "IN   GetChildSegments " << type  << " parent_id=" << parent.getId() << "  end=" << end << endl;
	//
	if ( !isMainstreamType(type, graphtype) )
		seltype = "";

	try
	{
		AnchorId start = GetStartAnchor(id);
		string next_start = "";

		/*
		while ( !start.empty() && start != end
				&& ( ! GetAnchored(start) || GetAnchorOffset(start) < end_offset) ) */

		while ( !start.empty()
				&& ( start != end
						|| ( end.empty() && GetAnchored(start) && GetAnchorOffset(start) < end_offset) ) )
		{
			const set<AnnotationId>& ids = GetOutgoingAnnotationSet(start, seltype);
			set<AnnotationId>::const_iterator it = ids.begin();
			next_start = "";
			for ( it = ids.begin(); it != ids.end(); ++it )
			{
				if ( (can_overlap || getElementProperty(*it, "order") == order) )
				{
					if ( GetEndAnchor(*it) != start )
					{
						v.push_back(SignalSegment(""));
						getSegment(*it, v.back(), parent.getTrack());
						if ( next_start.empty() ) next_start = GetEndAnchor(*it);
					}
				}
			}
			start = next_start;
		}
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ",  type=" << type << endl;
		return false;
	}
	//		TRACE << "OUT   GetChildSegments " << type  << " parent_id=" << parent.getId() << endl;
	return true;
}


/**
 * get all segments of given type, ordered by graph links, starting at parent start node, ending at parent end node
 *
 *  @param type searched type
 *  @param v returned vector holding all elements found
 *  @param parent parent signal segment
 *  @return true if any child found, else false.
 *
 * @note
 *  the retrieval is done by browsing though graph links.
 *  If searched type is a mainstream or background type, then all type-arcs
 *  belonging to the subgraph starting at parent start node and ending at
 *  parent start node are retrieved
 *  if searched type is an "event" type
 *
 *   other arc types, like events, are anchored  on mainstreamBaseType()-typed arcs.
 *  thus, to retrieve arcs attached to a given graph, we must first retrieve corresponding
 *   "segment" arcs
 *  if parent end node is unanchored -> retrieve until end-anchored segment is found
 *
 */
bool DataModel::getChilds(vector<string>& dest, const string& type, const string& parent, int notrack)
{
	const string& graphtype = m_conventions.getGraphtypeFromType(type);
	if ( graphtype.empty() ) return false;

	if ( !isMainstreamType(type, graphtype) )
		throw "Cannot use DataModel::getChilds to retrieve non mainstream elements";

	const string& graphId = m_agIds[graphtype];

	bool can_overlap = m_conventions.canOverlap(type);

	/*** single signal ***/
	bool singleSignal = signalCfg.isSingleSignal() ;
	bool video = conventions().usesVideoSignal() ;
	int no = 0 ;
	if (video)
		no = 2 ;

	AnchorId astart = "";
	AnchorId aend = "" ;
	int order = 0;
	if ( parent.empty() )
	{
		const set<AnchorId>& anchors = GetAnchorSetNearestOffset(graphId, 0.0);
		set<AnchorId>::const_iterator it;
		for (it = anchors.begin(); it != anchors.end(); ++it )
		{
			int elementTrack = getAnchorSignalTrack(*it) ;
			if (singleSignal && elementTrack != no)
				continue ;

			if ( notrack==-1 || elementTrack == notrack)
 			{
				astart = *it;
				getElementsBetweenAnchors(dest, type, astart, aend, (can_overlap ? -1 : order));
			}
		}
		if ( notrack == -1 && getNbTracks() > 1 )
		{
			// now sort elements by offset
			DataModel::CompareIdsByOffset cmp(*this);
			sort(dest.begin(), dest.end(), cmp);
		}
	}
	else
	{
		astart = GetStartAnchor(parent);
		aend = GetEndAnchor(parent);
		order = getOrder(parent);
		getElementsBetweenAnchors(dest, type, astart, aend, (can_overlap ? -1 : order));
	}
	return true;
}

void DataModel::getElementsBetweenAnchors(vector<string>& dest, const string& type, AnchorId astart, AnchorId aend, int order)
{

	string prev_start = "";

	while ( astart != aend ) {
		const set<AnnotationId>& ids = GetOutgoingAnnotationSet(astart, type);

		if ( ids.size() == 0 ) break;

		set<AnnotationId>::const_iterator it = ids.begin();
		prev_start = astart;
		int prev_order = -1;
		for ( it = ids.begin(); it != ids.end(); ++it ) {
			int cur_order = getOrder(*it);
			if ( (order == -1 || cur_order == order) ) 	{
				if ( prev_order != -1 && cur_order < prev_order ) {
					string sav = dest.back();
					dest.back() = *it;
					dest.push_back(sav);
				} else {
					dest.push_back(*it);
					prev_order = cur_order;
				}
				astart = GetEndAnchor(*it);
			}
		}
		if ( astart == prev_start ) {
			Log::trace() << "(DataModel::getChilds) " << "Loop detected for " << type << " at anchor " << astart << endl;
			break;
		}
	}
}

/**
 * 	retrieve signal segment properties for given (mainstream or background) annotation id;
 *
 * @param id annotation id
 * @param s (returned) signal segment
 * @param checktrack if != -1, check that annotation corresponds to given track
 * @param anchored_only return false if annotation start not anchored in signal timeline
 * @param follow_graph if annotation end not anchored, browse through graph to retrieve actual signal segment anchored end (may happen for splitted "value" segments)
 * @return true if signal segment retrieved, else false.
 */
bool DataModel::getSegment(const string& id, SignalSegment& s, int checktrack,
		bool anchored_only, bool follow_graph, int parent_order)
{
	int notrack;
	try
	{
		if ( !ExistsAnnotation(id) ) return false;

		const AnchorId& start = GetStartAnchor(id);
		if ( GetAnchored(start) ) {
			s.setStartOffset(GetAnchorOffset(start));
			notrack = getAnchorSignalTrack(start);
		} else 	{
			if ( anchored_only ) return false;
			s.setStartOffset(-1.);
			const string& astart = getStartAnchor(id);
			notrack = getAnchorSignalTrack(astart);
		}

		/*** single signal ***/
		bool singleSignal = signalCfg.isSingleSignal() ;
		bool video = conventions().usesVideoSignal() ;
		int no = 0 ;
		if (video)
			no = 2 ;
		if (singleSignal && notrack != no)
			return false ;

		if ( checktrack >= 0 && notrack != checktrack )
			return false;

		s.setId(id);

		// ICI VErifier regle order / notrack
		/*
		 if ( notrack == 0 )
		 if ( parent_order == -1 )
		 s.setOrder(getOrder(id));
		 else s.setOrder(getElementProperty(id, "order", parent_order));
		 else s.setOrder(0);  // s.setOrder(notrack);
		 */
		s.setOrder(getElementProperty(id, "order", 0));
		s.setTrack(notrack);

		const AnchorId& end = GetEndAnchor(id);
		if ( GetAnchored(end) )
			s.setEndOffset(GetAnchorOffset(end));
		else
		{
			s.setEndOffset(-1.);
			if (follow_graph )
			{
				s.setEndId(getAnchoredBaseTypeEndId(id));
				s.setEndOffset(GetEndOffset(s.getEndId()));
			}
		}
		return true;
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ",  id=" << id << endl;
	}
	return false;
}

/**
* get all elements of given type linked to start element,
*    ending when new anchored element with same type as start element is found.
*    (resulting vector includes start element if corresponds to seltype)
*
* @param start_id id of start element for graph browse
* @param v (OUT) vector in which annotation ids will be stored
* @param type target annotation type / "" for all types
* @param qualifiers_only if true return only qualifier-type annotations
* @param only_at_start if true return only elements attached to same start anchor as start_id
* @return end anchor or last segment-type element browsed
*
*/
string DataModel::getLinkedElements(const string& start_id, vector<string>& v,
		const string& type, bool qualifiers_only, bool only_at_start)
{
	bool more = true;
	string id = getAnchoredBaseTypeStartId(start_id);
	const string& parent_type = GetAnnotationType(start_id);
	const string& graphtype = getGraphType(start_id);

	const string& maintype = m_conventions.mainstreamBaseType(graphtype);
	bool add_start = (type == parent_type);
	const string& order = getElementProperty(start_id, "order");
	set<AnnotationId>::const_iterator it;
	string end("");

	try
	{
		while ( more )
		{
			const AnchorId& start = GetStartAnchor(id);
			const set<AnnotationId>& ids = GetOutgoingAnnotationSet(start, type);
			for ( it = ids.begin(); it != ids.end(); ++it )
			{
				const string& t = GetAnnotationType(*it);
				bool is_mainstream = isMainstreamType(t, graphtype);
				if ( ! (is_mainstream && (qualifiers_only || m_conventions.isHigherPrecedence(t, parent_type, graphtype))) )
				if ( (add_start || *it != start_id)
						&& ( getElementProperty(*it, "order") == order ) )
				v.push_back(*it);
			}

			end = GetEndAnchor(id);
			if ( only_at_start ) break;

			if ( GetAnchored(end) )
			{
				if ( parent_type == maintype ) more = false;
				else
				{
					const set<AnnotationId>& next = GetOutgoingAnnotationSet(end, parent_type);
					more = ( next.size() == 0 );
				}
			}
			if ( more )
			{
				const set<AnnotationId>& next = GetOutgoingAnnotationSet(end, maintype);
				id = "";
				for ( it =next.begin(); it != next.end(); ++it )
				if ( getElementProperty(*it, "order") == order )
				id = *it;
				if ( id.empty() ) more = false;
			}
		}
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << "id=" << id << ",  type=" << type << endl;
		return "";
	}
	return end;
}

/**
 * return next element in graph with same type
 * @param id current element id
 * @return previous element id, or empty string if no previous element found
 *
 * @note
 *  if current element type can overlap and overlapping elements exists, function returns 0-order previous element, else returns element with same order
 */
string DataModel::getNextElementId(const string& start_id)
{
	if ( !ExistsAnnotation(start_id) )
		return "";

	const AnchorId& anchor = GetEndAnchor(start_id);
	const set<AnnotationId>& next = GetOutgoingAnnotationSet(anchor, GetAnnotationType(start_id)) ;
	set<AnnotationId>::const_iterator it;
	const string& order = getElementProperty(start_id, "order");

	for (it=next.begin(); it != next.end(); ++it)
		if (getElementProperty(*it, "order") == order)
			return *it;
	return "";
}

/**
 * return previous element in graph with same type
 * @param id current element id
 * @return previous element id, or empty string if no previous element found
 *
 * @note
 *  in case 2 overlapping elements are attached at id start, then 0-order element will be returned
 */
string DataModel::getPreviousElementId(const string& id)
{
	if ( !ExistsAnnotation(id) )
		return "";

	const AnchorId& anchor = GetStartAnchor(id);
	const string& type = GetAnnotationType(id);
	const string& graphtype = getGraphType(id);

	const set<AnnotationId>& prev = GetIncomingAnnotationSet(anchor, type) ;
	set<AnnotationId>::const_iterator it;
	int order = getElementProperty(id, "order", 0);
	bool is_mainstream = isMainstreamType(type, graphtype);
	string sid = (is_mainstream ? id : getMainstreamStartElement(id));
	const vector<string>& overid = getElementsWithSameStart(sid, GetAnnotationType(sid));

	// if we are at start of overlapping branch -> check for 0-order previous element
	// else check for same order element
	if (m_conventions.canOverlap(type) && !overid.empty() )
		order = 0;

	for (it=prev.begin(); it != prev.end(); ++it) {
		if (getElementProperty(*it, "order", 0) == order)
			return *it;
	}

	return "";
}


/**
 * return id of mainstream start element for given annotation id
 * @param id annotation id
 * @return id of mainstream start element
 *
 * @note
 *   if id identifies a mainstream annotation, function will return id,
 *   else function will return mainstream annotation starting at id's start anchor
 */
string DataModel::getMainstreamStartElement(const string& id)
{
	if ( !ExistsAnnotation(id) )
		return "";

	const string& type = GetAnnotationType(id);
	const string& graphtype = getGraphType(id);

	if (isMainstreamType(type,graphtype) )
		return id;

	const AnchorId& start = GetStartAnchor(id);
	const string& order = getElementProperty(id, "order");

	const vector<string>&mainstream=m_conventions.getMainstreamTypes(graphtype);

	const set<AnnotationId>& ids = GetOutgoingAnnotationSet(start,
			mainstream.back());
	set<AnnotationId>::const_iterator it;
	for (it = ids.begin(); it != ids.end(); ++it)
		if (getElementProperty(*it, "order") == order)
			return *it;

	return "";
}

/**
 * return id of mainstream end element for given annotation id
 * @param id annotation id
 * @return id of mainstream end element
 *
 * @note
 *   if id identifies a mainstream annotation, function will return id,
 *   else function will return mainstream annotation ending at id's end anchor
 */
string DataModel::getMainstreamEndElement(const string& id)
{
	if ( !ExistsAnnotation(id) )
		return "";
	const string& type = GetAnnotationType(id);
	const string& graphtype = getGraphType(id);
	const vector<string>& mainstream=m_conventions.getMainstreamTypes(graphtype);

	const AnchorId& stop = GetEndAnchor(id);
 	const set<AnnotationId>& ids = GetIncomingAnnotationSet(stop,
			mainstream.back());
	set<AnnotationId>::const_iterator it;
	const string& order = getElementProperty(id, "order");

	for (it=ids.begin(); it != ids.end(); ++it)
	{
		if (getElementProperty(*it, "order") == order)
			return *it;
	}

	return "";
}

/**
 * For given annotation id, returns the  id of next mainstream element.
 * @param id 		Annotation id
 * @return 			Id of next mainstream element
 *
 * @note
 *   If id identifies a mainstream annotation, function will return the next element of,
 *   same type, otherwise it will return the base mainstream annotation starting at id's
 *   end anchor.\n
 *   If element belongs to an overlapping branch terminating at element's end,
 *   will return "" (no next mainstream element).
 */
string DataModel::getMainstreamNextElement(const string& id)
{
	if ( !ExistsAnnotation(id) )
		return "";

	string searched_type = GetAnnotationType(id);
	const string& graphtype = getGraphType(id);

	//> -- Id is mainstream ? searching same type
	//     Otherwise, searching for baseSegmentationType
	if ( !isMainstreamType(searched_type, graphtype) )
		searched_type = mainstreamBaseType(graphtype) ;

	//> -- Get the end anchor: we'll search for the annotation at this timestamp
	const AnchorId& stop = GetEndAnchor(id);

	//> -- Get all annotation of identified type starting at end anchor
	const set<AnnotationId>& ids = GetOutgoingAnnotationSet(stop, searched_type);
	set<AnnotationId>::const_iterator it;

	//> -- Find the one corresponding to same order
	const string& order = getElementProperty(id, "order");
	for (it=ids.begin(); it != ids.end(); ++it)
	{
		if (getElementProperty(*it, "order") == order)
			return *it;
	}

	return "";
}

//
// get segment start element with type = mainstreamBaseType for given annotation id
//     if annotation is qualifier type, or
//     if annotation start is unanchored, browse back in
//     annotation graph to retrieve anchored segment
//
string DataModel::getAnchoredBaseTypeStartId(const std::string& p_id)
{
	if (p_id.empty() || !ExistsAnnotation(p_id) )
		return "";

	string id("");
	const string& graphtype = getGraphType(p_id);

	try
	{
		const string& b_type = m_conventions.mainstreamBaseType(graphtype);
		const string& p_type = GetAnnotationType(p_id);
		set<AnnotationId>::const_iterator it;

		// - not a mainstream ? search for attached one
		if ( ! isMainstreamType(p_type, graphtype) )
			id = getMainstreamStartElement(p_id);
		else
		{
			// - not a mainstream basetype ? get the mainstream base type
			if ( p_type != b_type )
			{
				const set<AnnotationId>& ids = GetOutgoingAnnotationSet(GetStartAnchor(p_id), b_type);
				const string& order = getElementProperty(p_id, "order");
				for ( it=ids.begin(); it != ids.end(); ++ it )
				{
					if ( getElementProperty(*it, "order") == order )
					{
						id = *it;
						break;
					}
				}
			}
			// - id is a mainstream base type ? start from it
			else
				id = p_id;
		}

		// - no matching minstream element for starting research, stop
		if ( id == "" )
			return "";

		while ( 1 )
		{
			//- is start node time anchored ? found the good one
			const string& start = GetStartAnchor(id);
			if ( GetAnchored(start) )
				return id ;

			//- unanchored annot -> browse backward through graph to retrieve anchored node
			const set<AnnotationId>& ids = GetIncomingAnnotationSet(start, b_type);
			if ( ids.size() == 0 )
				return "";
			bool found_pred = false ;
			int order = getOrder(id) ;
			for ( it = ids.begin(); it != ids.end(); ++it )
			{
				if ( getOrder(*it) == order )
				{
					id = *it;
					found_pred = true ;
					break;
				}
			}

			//- get there ? none of same order could be found, we're stuck here, nobody found
			if (!found_pred)
				return "" ;
		}
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << p_id << endl;
	}
	return "";
}

//
// get end element with type = mainstreamBaseType for given annotation id
//     if annotation is qualifier type, or
//     if annotation end is unanchored, browse forth in
//     annotation graph to retrieve start-anchored segment
//
string DataModel::getAnchoredBaseTypeEndId(const std::string& p_id)
{
	if (p_id.empty() || !ExistsAnnotation(p_id) )
		return "";


	const string& p_type = GetAnnotationType(p_id);
	const string& graphtype = getGraphType(p_id);

	const string& endAnchor = GetEndAnchor(p_id);
	if (p_type == m_conventions.mainstreamBaseType(graphtype) && GetAnchored(endAnchor) )
		return p_id;

	const string& id = getMainstreamEndElement(p_id);

	if (id.empty() )
	{
		MSGOUT << "no mainstream end element for id=" << p_id
				<< " (wrong end anchor ?)" << endl;
	}
	else
	{
		vector<string> childs;
		getLinkedElements(id, childs, m_conventions.mainstreamBaseType(graphtype));
		if (childs.size() > 0)
			return childs.back();
	}
	return "";
}

//
// get next "anchored" element with type = mainstreamBaseType for given annotation id
//     if annotation is qualifier type, or
//     if annotation end is unanchored, browse forth in
//     annotation graph to retrieve anchored segment
//

string DataModel::getBaseTypeNextId(const std::string& p_id)
{
	if (p_id.empty() || !ExistsAnnotation(p_id) )
		return "";
	string id = getAnchoredBaseTypeEndId(p_id);

	if (id != "")
	{
		const string& graphtype = getGraphType(p_id);
		const set<AnnotationId>& ids = GetOutgoingAnnotationSet(
				GetEndAnchor(id), m_conventions.mainstreamBaseType(graphtype));
		if (ids.size() > 0)
			id = *(ids.begin());
		else
			id = "";
	}
	return id;
}


bool DataModel::mainstreamBaseElementHasText(const string& id, const string& graphtype)
{
	try
	{
		const string& submain = getElementProperty(id, "subtype") ;
		if ( ! submain.empty() )
		{
			if ( graphtype.empty() )
				return conventions().submainHasText(GetAnnotationType(id), submain, getGraphType(id)) ;
			else
				return conventions().submainHasText(GetAnnotationType(id), submain, graphtype) ;
		}
		return false ;
	}
	catch(AGException& ex)
	{
		return false;
	}	
}

bool DataModel::isEventMainstreamElement(const string& id, const string& graphtype)
{
	try
	{
		// -- Mainstream base type & no text data ? OK
		if ( getElementType(id)==conventions().mainstreamBaseType(graphtype)
				&& !mainstreamBaseElementHasText(id, graphtype) )
			return true ;
		// -- Otherwise, no !
		else
			return false ;
	}
	catch(AGException& ex)
	{
		return false;
	}
}

bool DataModel::mainstreamBaseElementIsValid(const string& id, const string& graphtype)
{
	if ( getElementType(id) != mainstreamBaseType(graphtype) )
		return false ;

	const string& submain = getElementProperty(id, "subtype") ;
	const string& subtype =  getElementProperty(id, "value") ;
	const string& desc =  getElementProperty(id, "desc") ;
	int valid = conventions().isValidMainstreamBaseType(graphtype, submain, subtype, desc) ;

	return (valid > 0) ;
}


/*========================================================================
 *
 *  query / set graph element properties
 *
 *========================================================================*/

//
// check if graph has annotations with given type
//
bool DataModel::hasElementsWithType(const std::string& type, string graphId)
{
	try
	{
		if ( graphId.empty () ) {
			const string& graphtype = m_conventions.getGraphtypeFromType(type);
			if ( graphtype.empty() ) return false;

			graphId = m_agIds[graphtype];
		}
		if (!graphId.empty()) {
			const set<AnnotationType>& l = GetAnnotationTypes(graphId);
			return (l.find(type) != l.end());
		}
		else {
			MSGOUT << "hasElementsWithType: graphId empty <!>" << endl;
			return false ;
		}
	}
	catch (AGException& e) 	{
		MSGOUT << "Caught AGException : " << e.error() << ", type=" << type << " - graphId=" << graphId << endl;
		return false;
	}
	return false;
}


/**
 * get elements ids with given type
 * @param dest	(out) vector in which returned ids will be stored;
 * @param type	target type
 * @param notrack target track no (-1 for both)
 * @param graphId	target AG id - let empty if id to be guessed from annotation type, with respect to current annotation conventions
 * @return true if ok, false if no element of given type
 */
bool DataModel::getElementsWithType(vector<string>& dest, const std::string& type, int notrack, string graphId)
{
	if ( graphId.empty () )
	{
		const string& graphtype = m_conventions.getGraphtypeFromType(type);
		if ( graphtype.empty() )
			return false;
		graphId = m_agIds[graphtype];
	}
	if ( notrack == -1 || getNbTracks() == 1 )
	{
		const list<AnnotationId>& ids = GetAnnotationSeqByOffset(graphId, 0.0, 0.0, type);
		copy(ids.begin(), ids.end(), back_inserter(dest));
	}
	else
	{
		// TODO temporaire -> gerer parcours de graphe
		const list<AnnotationId>& ids = GetAnnotationSeqByOffset(graphId, 0.0, 0.0, type);
		list<AnnotationId>::const_iterator it;
		for (it = ids.begin(); it != ids.end(); ++it )
			if ( getAnchorSignalTrack(GetStartAnchor(*it)) == notrack )
				dest.push_back(*it);
	}
	return true ;
}


//
// check element existence for given id
//
bool DataModel::existsElement(const std::string& id)
{
	try
	{
		return ExistsAnnotation(id);
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return false;
}
//
// get element type for given id
//
string DataModel::getElementType(const std::string& id)
{
	try
	{
		if ( ExistsAnnotation(id) )
		return GetAnnotationType(id);
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return "";
}

//
// has annotation property for given id
//
bool DataModel::hasElementProperty(const std::string& id, const std::string& name)
{
	try
	{
		return ExistsFeature(id, name);
	}
	catch (AGException& e)
	{
		MSGOUT << "DataModel::hasElementProperty: caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return false;
}

//
// get annotation property with given name for given id;
//  will return default value if not defined
//
// 1st version returns element as a string, 2nd as an int, 3rd as a float
string DataModel::getElementProperty(const std::string& id,
		const std::string& name, const std::string& defaut)
{
	try
	{
		if ( ExistsFeature(id, name) )
			return GetFeature(id, name);
	}
	catch (AGException& e)
	{
//		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << ", name=" << name << endl;
	}
	return defaut;
}

int DataModel::getElementProperty(const std::string& id,
		const std::string& name, int defaut)
{
	const string& value = getElementProperty(id, name, "");
	if ( !value.empty() )
		return atoi(value.c_str()) ;
	return defaut;
}

float DataModel::getElementProperty(const std::string& id,
		const std::string& name, float defaut)
{
	const string& value = getElementProperty(id, name, "");
	if ( !value.empty() )
		return atof(value.c_str()) ;
	return defaut;
}

/**
* set element property for given id
* @param id annotation id
* @param item property item
* @param value new property value
* @param emit_signal if true emit signalElementModified(UPDATED) signal
*
* @note: 1st version sets property as a string, 2nd as an int, 3rd as a float
*/

bool DataModel::setElementProperty(const std::string& id,
		const std::string& item, const std::string& value, bool emit_signal)
{
	try
	{
		if ( ExistsAnnotation(id) )
		{
			agSetFeature(id, item, value);
			if ( emit_signal )
					emitSignal(id, UPDATED);
			return true ;
		}
		else
			return false ;
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
		return false ;
	}
}

bool DataModel::setElementProperty(const std::string& id,
		const std::string& item, float value, bool emit_signal)
{
	string s;
	StringOps(s).fromFloat(value);
	return setElementProperty(id, item, s, emit_signal);
}


bool DataModel::setElementProperty(const std::string& id,
		const std::string& item, int value, bool emit_signal)
{
	string s;
	StringOps(s).fromInt(value);
	return setElementProperty(id, item, s, emit_signal);
}

/**
* return segment language for given segment id
* @param id segment id
* @param use_default if true and no segment language defined, will return transcription language
* @return segment language
*
* @note : will return
*  - segment language property if set
*  - else parent segment language property if set
*  - else document transcription language
*/
const string& DataModel::getSegmentLanguage(const string& id, bool use_default)
{
	if ( ! id.empty() ) {
		const string& lang = getElementProperty(id, "lang");
		if ( !lang.empty() ) return lang;

		const string& pid = getParentElement(id);
		if ( !pid.empty() )
			return getSegmentLanguage(pid, use_default);
	}
	if ( use_default ) {
		int notrack = (id.empty() ? 0 : getElementSignalTrack(id));
		return getTranscriptionLanguage(notrack);
	}
	return _novalue;
}


/**
* delete element property for given id
* @param id annotation id
* @param name property name
* @param emit_signal if true and property deleted, emits signalElementModified(UPDATED)
* @return true if property deleted, else false
*/
bool DataModel::deleteElementProperty(const std::string& id,
		const std::string& name, bool emit_signal)
{
	try
	{
		if ( ExistsFeature(id, name) )
		{
			agDeleteFeature(id, name);
			if ( emit_signal )
				emitSignal(id, UPDATED);
			return true;
		}
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return false;
}

/**
* check if given annotation is instantaneous
* @ param id annotation id
* @return  true if element start anchor = element end anchor, else false
*/
bool DataModel::isInstantaneous(const std::string& id)
{
	if (ExistsAnnotation(id) )
		return (GetStartAnchor(id) == GetEndAnchor(id));
	return false;
}


/**
* check if given segment is a speech segment
* @ param id segment id
* @param checkSpeaker if false and id is  mainstream base type, just check id has text subtype, else check speaker property in id's hierarchy (default),
* @return  true if true if speech segment , else false
*/

bool DataModel::isSpeechSegment(const std::string& id, bool checkSpeaker)
{
	bool isSpeechSegment = false;
	try
	{
		const string& type = GetAnnotationType(id);
		const string& graphtype = getGraphType(id);

		if ( checkSpeaker || type != m_conventions.mainstreamBaseType(graphtype) ) {
			const string& spktype = m_conventions.getSpeakerType(graphtype);
			if ( type == spktype ) {
				if ( (isSpeechSegment = ExistsFeature(id, "speaker") ) )
					isSpeechSegment = (GetFeature(id, "speaker") != Speaker::NO_SPEECH );
			}
			else {
				if ( m_conventions.isHigherPrecedence(spktype, type, graphtype) ) {
					const string& pid = getParentElement(id, spktype) ;
					if (!pid.empty()) {
						if ( (isSpeechSegment = ExistsFeature(pid, "speaker") ) )
							isSpeechSegment = (GetFeature(pid, "speaker") != Speaker::NO_SPEECH );
					}
				}
			}
		}
		else
		{
			return mainstreamBaseElementHasText(id);
		}
	}
	catch (AGException& e)
	{
		//			MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return isSpeechSegment;
}

/**
* return annotation order (overlapping annotations)
* @param id annotation id
* @return annotation order (from 0 to n)
*/
int DataModel::getOrder(const string& id)
{
	return getElementProperty(id, "order", 0);
}

/**
 * check if some elements with given property exist in graph
 * @param agid graph id
 * @param name feature name
 * @param type element type
 * @return true if have some elements, else false.
 */
bool DataModel::hasElementsWithProperty(const string& agid, const string& name, const string& type)
{
	bool b = false;
	if ( ! ExistsAG(agid)  ) return false;
	const set<FeatureName>& features = GetAnnotationFeatureNames(agid, type);
	b = (features.find(name) != features.end());
	return b;
}

/**
* return possible overlapping type for given annotation id
* @param id annotation id
* @return overlapping annotation type / "" if no overlap allowed
*
* @notes : check in mainstream type at and above given annotation type if max_overlap > 0;
*/
string DataModel::getOverlapType(const string& id)
{
	const string& type = GetAnnotationType(id);
	bool check=false;
	vector<std::string>::const_reverse_iterator it;
	const vector<string>& mainstream = m_conventions.getMainstreamTypes(getGraphType(id));

	for (it = mainstream.rbegin(); it != mainstream.rend(); ++it)
	{
		if ( *it == type)
			check = true;
		if (check)
			if ( m_conventions.maxOverlap(*it) > 0)
				return *it;
	}
	return "";
}

/*========================================================================
 *
 *  query / set graph element anchors
 *
 *========================================================================*/

/**
* create new anchor at given offset if no other anchor exists at same offset for same track or insertion policy = INSERT,
* else return existing anchor id
 * @param graphId checked graph id
 * @param notrack checked signal track no
 * @param offset signal offset
 * @param force if true, force creation even if another anchor exists at given offset
 * @return new anchor id / existing anchor id
 */

string DataModel::createAnchorIfNeeded(const string& graphId, int notrack, float offset, bool force)
{
	string id = "";
	try
	{
		id = getAnchorAtOffset(graphId, notrack, offset);
		if ( id == "" || force)
		{
			set<SignalId> sigIds;
			sigIds.insert(signalCfg.getSigid(notrack));
			id = agCreateAnchor(graphId, offset, TRANS_ANCHOR_UNIT, sigIds);
			m_anchorTrack[id] = notrack;
		}
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return id;
}


//
// wrapper for SplitAnchor method
//   also updates m_anchorTrack map
string DataModel::splitAnchor(const string& anchorid)
{
	const string& newid = agSplitAnchor(anchorid);
	m_anchorTrack[newid] = m_anchorTrack[anchorid];
	return newid;
}

/**
 * get signal-anchored start node for given annotation id.
 * If current annotation start is unanchored, browse back in mainstream base type graph to retrieve anchored node.
 * @param id annotation id
 * @return start anchor id / "" if not found
 */
string DataModel::getStartAnchor(const std::string& p_id)
{
	const string& id = getAnchoredBaseTypeStartId(p_id);
	if (id != "")
		return GetStartAnchor(id);
	return _novalue;
}

/**
 * get signal-anchored end node for given annotation id.
 * If current annotation start is unanchored, browse forth in mainstream base type graph to retrieve anchored node.
 * @param id annotation id
 * @return end anchor id / "" if not found
 */
string DataModel::getEndAnchor(const std::string& p_id)
{
	try
	{
		string id = p_id;
		if ( isQualifierType(GetAnnotationType(p_id)) )
		id = getMainstreamStartElement(p_id);
		const string& type = GetAnnotationType(id);
		string stop = GetEndAnchor(id);
		while ( ! GetAnchored(stop) )
		{
			const set<AnnotationId>& ids = GetOutgoingAnnotationSet(stop, type);
			if ( ids.size() > 0 ) {
				id=*(ids.begin());
				stop = GetEndAnchor(id);
			}
			else return _novalue;
		}
		return GetEndAnchor(id);
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << p_id << endl;
	}
	return _novalue;
}


float DataModel::isAnchoredElement(string id, bool start)
{
	float ret = -1.0;

	if ( ! id.empty() ) {
		try {
			if (start) {
				const string& an = GetStartAnchor(id);
				if ( GetAnchored(an) )
					ret = GetAnchorOffset(an);
			} else {
				const string& an = GetEndAnchor(id);
				if ( GetAnchored(an) )
					ret = GetAnchorOffset(an);
			}
		} catch (AGException& e) {}
	}
	return ret;
}

/*
 *  mode
 *  		0: checks if start anchor has time value\n
* 			1: checks if end anchor has time value\n
* 			2: checks if both anchors have time value\n
* 			3: checks if at least one anchor have time value\n
*/
bool DataModel::isAnchoredElement(const std::string& id, int mode)
{
	try {
		const string& startan = GetStartAnchor(id) ;
		bool start = GetAnchored(startan) ;
		if ( mode==0 || (mode==3 && start) )
			return start ;

		const string& endan = GetEndAnchor(id) ;
		bool end = GetAnchored(endan) ;
		if ( mode==1 || (mode==3 && end) )
			return end ;

		return (start && end) ;
	} catch (AGException& e) {}
	return false;
}

/**
 * get signal-anchored start node offset for given annotation id.
 * If current annotation start is unanchored, browse back in mainstream base type graph following to retrieve anchored node.
 * @param id annotation id
 * @return start anchor  signal offset / -1 if not found
 */
float DataModel::getStartOffset(const std::string& id)
{
	const string& start = getStartAnchor(id);
	if (start == "")
		return -1.0;
	return GetAnchorOffset(start);
}

/**
 * get signal-anchored end node offset for given annotation id.
 * If current annotation end is unanchored, browse forth in mainstream base type graph to retrieve anchored node.
 * @param id annotation id
 * @return end anchor signal offset / -1 if not found
 */
float DataModel::getEndOffset(const std::string& id)
{
	const string& stop = getEndAnchor(id);
	if (stop == "")
		return -1.0;
	return GetAnchorOffset(stop);
}


/**
 *  returns signal track no corresponding to given annotation start anchor
 *  @param id annotation id
 *  @return signal track no  / -1 if annotation not found
 *
 * @note
 *   element may be anchored or not. If not, then the signal track no of its anchored parent
 *   element will be returned.
 *
 */
int DataModel::getElementSignalTrack(const std::string& id)
{
	if ( getNbTracks() == 1 ) return 0;  // only one track;

	int notrack = 0;
	try
	{
//		if ( !ExistsAnnotation(id) )
//			MSGOUT << "Non existing annotation id=" << id << endl;
//		else
//		{
//			const string& type = getElementType(id) ;
//			if (type=="background") {
//				const string& anchorid = GetStartAnchor(id);
//				notrack = getAnchorSignalTrack(anchorid);
//			} else {
				const string& anchorid = getStartAnchor(id);
				if ( ! anchorid.empty() )
					notrack = getAnchorSignalTrack(anchorid);
				else
					MSGOUT << "Base segment start anchor not found for " << GetAnnotationType(id) << " id=" << id << endl;
//			}
//		}
	}
	catch (AGException& e)	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return notrack;
}

const string& DataModel::getElementSignalId(const std::string& id)
{
	bool background = (getElementType(id)=="background") ;
	try
	{
		if ( !ExistsAnnotation(id) )
			MSGOUT << "Non existing annotation id=" << id << endl;
		else {
			string anchorid;
			if (!background)
				anchorid = getStartAnchor(id);
			else
				anchorid = GetStartAnchor(id);
			if ( ! anchorid.empty() ) {
				std::set<SignalId> signals = GetAnchorSignalIds(anchorid) ;
				std::set<SignalId>::iterator it ;
				if (signals.size()==1) {
					it = signals.begin() ;
					return *it ;
				}
				else {
					MSGOUT << "Several signals for the element " << GetAnnotationType(id) << " id=" << id << endl;
				}
			}
			else
				MSGOUT << "Base segment start anchor not found for " << GetAnnotationType(id) << " id=" << id << endl;
		}
	}
	catch (AGException& e)	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return _novalue ;
}

/**
 * returns signal track no corresponding to given anchor (where 0 identifies first signal track)
 *
 * @param id anchor id
 * @return signal track no / 0 if anchor not found
 */
int DataModel::getAnchorSignalTrack(const std::string& id)
{
	if ( getNbTracks() == 1 && !signalCfg.isSingleSignal())
		return 0 ;

	std::map<string, int>::iterator it = m_anchorTrack.find(id) ;
	if (it == m_anchorTrack.end() )
		return 0 ;
	else
		return it->second ;
}

/**
 * returns given anchor offset,
 *
 * @param id anchor id
 * @return anchor offset if anchored, else -1
 */
float DataModel::getAnchorOffset(const std::string& id)
{
	try
	{
		if ( GetAnchored(id) ) return GetAnchorOffset(id);
	}
	catch (AGException& e)
	{
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return -1.;
}


/**
 * set speaker hint for new turns creation
 * @param notrack signal track no
 * @param spkid speaker id
 */
void DataModel::setSpeakerHint(const string& spkid, int notrack)
{
	setSignalProperty(notrack, "speaker_hint", spkid);
	setHint("", "speaker", spkid, notrack);
}

/**
 * set annotation hint
 * @param type annotation type
 * @param name feature name
 * @param value feature value
 * @param notrack signal track no
 */
void DataModel::setHint(const string& type, const string& name, const string& value, int notrack)
{
	if ( name == "speaker" )
		m_featureHints[notrack][name] = value;
	else
		m_featureHints[notrack][type + "," + name] = value;
}

void DataModel::setHint(const string& label, const string& value, int notrack)
{
	m_featureHints[notrack][label] = value;
}


/**
 * get annotation hint
 * @param type annotation type
 * @param name feature name
 * @param notrack signal track no
 * @return hint value / "" if no hint
 */
const string& DataModel::getHint(const string& type, const string& name, int notrack)
{
	if ( m_featureHints.find(notrack) != m_featureHints.end() ) {
		const map<string, string>& hints = m_featureHints[notrack];
		map<string, string>::const_iterator it;
		if ( name == "speaker" ) it = hints.find(name);
		else {
			string key = type + "," + name;
			it = hints.find(key);
		}
		if ( it != hints.end() ) return it->second;
	}
	return _novalue;
}

const string& DataModel::getHint(const string& label, int notrack)
{
	if ( m_featureHints.find(notrack) != m_featureHints.end() )
	{
		const map<string, string>& hints = m_featureHints[notrack];
		map<string, string>::const_iterator it = hints.find(label) ;
		if ( it != hints.end() )
			return it->second;
	}
	return _novalue;
}

void DataModel::cleanHint(const string& label, int notrack)
{
	if ( m_featureHints.find(notrack) == m_featureHints.end() )
		return ;
	m_featureHints[notrack].erase(label) ;
}


/**
 * find segments of given type overlapping signal segment s in same track
 *
 *	@param s checked signal segment
 * @param v returned set of overlapping ids
 * @param type search overlapping type (defaults to checked segment type)
 * @param strict if true, return only ids of segments which totally overlapp checked segment
 *
 */

void DataModel::getOverlappingSegmentsIds(const SignalSegment& s,
		set<string>& v, const string& type, bool strict)
{
	const string& graphtype = m_conventions.getGraphtypeFromType(type);
	if ( graphtype.empty() ) // no element with this type
		return;
	const string& graphId = m_agIds[graphtype];

	if (s.getStartOffset() >= 0)
	{
		const list<AnnotationId>& ids = GetAnnotationSetByOffset(graphId,
				s.getStartOffset()+EPSILON, type);
		list<AnnotationId>::const_iterator it;
		for (it = ids.begin(); it != ids.end(); ++it)
		{
			if (getAnchorSignalTrack(*it) != s.getTrack() )
				continue;
			if ( *it != s.getId() )
			{
				float end_offset = GetEndOffset(*it);
				if (end_offset == 0 && type == m_conventions.mainstreamBaseType(graphtype) )
					end_offset = getEndOffset(*it);
				if (end_offset > s.getStartOffset() )
					if ( !strict || end_offset >= s.getEndOffset() )
						v.insert(*it);
			}
		}
	}

	if ( !strict && s.getEndOffset() >= 0)
	{
		const list<AnnotationId>& ids = GetAnnotationSetByOffset(graphId,
				s.getEndOffset()-EPSILON, type);
		list<AnnotationId>::const_iterator it;
		for (it = ids.begin(); it != ids.end(); ++it)
		{
			if (getAnchorSignalTrack(*it) != s.getTrack() )
				continue;

			if ( *it != s.getId() && GetStartOffset(*it) < s.getEndOffset() )
				v.insert(*it);
		}
	}
}



/*========================================================================
 *
 *  Various info functions
 *
 *========================================================================*/

string DataModel::getAnchorAtOffset(const string& graphId, int notrack, float offset)
{
	const list<AnchorId>& l = GetAnchorSetByOffset(graphId, offset, EPSILON);
	list<AnchorId>::const_iterator it;
	for (it=l.begin(); it!=l.end() ; ++it)
		if (GetAnchored(*it) )
			if (m_anchorTrack[*it] == notrack)
			{
				return *it;
			}

	return "";
}


//
// locate given type annotation which overlaps given time for given track
//
string DataModel::getByOffset(const string& type, float startTime,
		int notrack, string graphtype)
{
	string ptype = type;
	if (graphtype.empty())
		graphtype = m_conventions.getGraphtypeFromType(type);
	if ( !hasAnnotationGraph(graphtype))
		return "";

	const string& agId = m_agIds[graphtype];

	if (type == mainstreamBaseType(graphtype) )
	{
		// base segment may be splitted and thus not found (missing anchor offsets)
		// to avoid this, we use parent type in search, and we'll refine segment
		// search afterwards.
		ptype = m_conventions.getParentType(type,graphtype);
		if ( ptype.empty() ) ptype = type ;  // no parent type
	}

	list<std::string> l ;
	// AGAPI doesn't give back annotation if given time is end of signal ??
	// reduce time to get annotation
	if (startTime==m_signalLength)
		l = GetAnnotationSetByOffset(agId, startTime-EPSILON, ptype) ;
	else
		l = GetAnnotationSetByOffset(agId, startTime, ptype) ;

	// Log::err() << " getByOffset FOUND " << l.size() << " ANNOTS" << endl;
	list<std::string>::const_iterator it;
	for (it = l.begin(); it != l.end(); ++it)
	{
		//  Check correct track
		if ( (notrack < 0 || getElementSignalTrack(*it) == notrack) )
		{
			//> Adjust comparison for end signal
			float startTimeCompare = startTime+EPSILON ;
			while (startTimeCompare >= m_signalLength)
				startTimeCompare = startTimeCompare - EPSILON ;

			if (GetEndOffset(*it) > startTimeCompare)
			{
				if (type == ptype)
					return *it;
				else
				{
					//> Adjust comparison if we're at signal start
					float startTimeCompare2 = startTime-EPSILON ;
					if (startTimeCompare2 <= 0)
						startTimeCompare2 = 0 + EPSILON ;

					string id = getAnchoredBaseTypeStartId(*it);
					string nextid = getBaseTypeNextId(id);
					while (nextid != "" && GetStartOffset(nextid) < startTimeCompare2)
					{
						id = nextid;
						nextid = getBaseTypeNextId(nextid);
					}
					return id;
				}
			}
		}
	}
	return "";
}

//
// TODO ici verifier ajustement de récuperation des bg aux frontières de segments
//  -> cf positionnement curseur ??
string DataModel::checkBackgroundParallelSegmentAtStart(const string& segment, const string& background)
{
	float segment_start = GetStartOffset(segment) ;
	float segment_end = GetEndOffset(segment) ;

	float background_start = GetStartOffset(background) ;

	string next = "" ;

	//> We're one much more segment behind, try to find next anchored element
	if (segment_end <= background_start)
		next = getNextAnchoredElementId(segment) ;

	//> if some found, check conditions
	if (!next.empty())
	{
		bool end_is_anchored = GetAnchored(GetEndAnchor(next)) ;
		bool start_is_anchored = GetAnchored(GetStartAnchor(next)) ;
		if (start_is_anchored)
		segment_start = GetStartOffset(next) ;
		else
			segment_start = -1 ;
		if (end_is_anchored)
			segment_end = GetEndOffset(next) ;
		else
			segment_end = -1 ;

		//> For being valid candidate, segment must be anchored (at start)
		//  and bgstart must be timed upper than segment start
		//  and bgstart must be timed lower than segment end
		if (segment_start==-1)
			next = "" ;
		else if (segment_end!=-1) {
			if (background_start<segment_start || background_start>segment_end)
			next = "" ;
	    }
	}

	if (next.empty())
		return segment ;
	else
		return next ;
}

string DataModel::checkBackgroundParallelSegmentAtEnd(const string& segment, const string& background)
{
	float segment_start = GetStartOffset(segment) ;
	float segment_end = GetEndOffset(segment) ;

	float background_end = GetEndOffset(background) ;

	string previous = "" ;

	//> We're one much more segment behind, try to find previous anchored element
	if (background_end<=segment_start)
		previous = getPreviousElementId(segment) ;

	//> if some found, check conditions
	if (!previous.empty())
	{
		bool end_is_anchored = GetAnchored(GetEndAnchor(previous)) ;
		bool start_is_anchored = GetAnchored(GetStartAnchor(previous)) ;
		if (start_is_anchored)
			segment_start = GetStartOffset(previous) ;
		else
			segment_start = -1 ;
		if (end_is_anchored)
			segment_end = GetEndOffset(previous) ;
		else
			segment_end = -1 ;

		//> For being valid candidate, segment must be anchored (at start)
		//  and bgstart must be timed upper than segment start
		//  and bgstart must be timed lower than segment end
		if (segment_start==-1 && segment_end!=-1)
			previous = "" ;
		else {
			if (segment_end!=-1) {
				if (background_end>segment_end)
					previous = "" ;
			}
			if (segment_start!=-1) {
				if (background_end<segment_start)
					previous = "" ;
			}
		}
	}

	if (previous.empty())
		return segment ;
	else
		return previous ;
}


//
// locate given type annotation which overlaps given time for given track
//
string DataModel::getNextByOffset(const string& type, float startTime,
		int notrack)
{
	string ptype = type;
	string nearest = "";
	const string& graphtype = m_conventions.getGraphtypeFromType(type);
	const string& graphId = m_agIds[graphtype];

	const list<AnnotationId>& l = GetAnnotationSeqByOffset(graphId, startTime
			+EPSILON, 0.0, type);

	if (l.size() > 0)
	{
		list<AnnotationId>::const_iterator it = l.begin();
		float startoff = GetStartOffset(*it);
		int nt = getElementSignalTrack(*it);
		nearest = *it;
		int order = getOrder(*it);
		if ( ! (nt == 0 && order == 0))
		{
			++it;
			while (it != l.end() && GetStartOffset(*it) == startoff)
			{
				if (getOrder(*it) != 0)
					++it;
				else
				{
					nearest = *it;
					break;
				}
			}
		}
	}

	return nearest;
}

/*========================================================================
 *
 *  SPEAKERS MANAGEMENT
 *
 *========================================================================*/

//
// if speaker updated, emit signalElementModified for all related turns
void DataModel::speakerUpdated(const string& id)
{
	//
	//	TRACE_D << " IN    DataModel::speakerUpdated  " << id << endl;

	if (id.empty() )
		return;
	string graphtype = "transcription_graph";
	const string& type = m_conventions.getTypeFromFeature("speaker", graphtype);
	if ( type.empty() || m_agIds.find(graphtype) == m_agIds.end() ) return ;

	const set<AnnotationId>& ids = GetAnnotationSetByFeature(m_agIds[graphtype], "speaker", id, type);
	set<AnnotationId>::const_iterator it;
	for (it = ids.begin(); it != ids.end(); ++it)
		emitSignal(*it, UPDATED);
}

//
// returns true if given speaker id associated to a turn
bool DataModel::speakerInUse(const string& id)
{
	if (id.empty() )
		return false;
	string graphtype = "transcription_graph";
	const string& type = m_conventions.getTypeFromFeature("speaker", graphtype);
	if ( type.empty() || m_agIds.find(graphtype) == m_agIds.end() ) return false;

	const set<AnnotationId>& ids = GetAnnotationSetByFeature(m_agIds[graphtype],
			"speaker", id, type);
	return (ids.size() > 0);
}

/** returns true if some speakers in dictionnary are not associated to any turn */
bool DataModel::hasUnusedSpeakers()
{
	SpeakerDictionary::iterator it;

	for (it = m_speakersDict.begin(); it != m_speakersDict.end(); ++it)
		if (it->second.getId() != Speaker::NO_SPEECH
				&& speakerInUse(it->second.getId()) == false)
			return true;

	return false;
}



/**
 replace given speaker by another speaker, for given turn if turnid != "", else for all turns associated to old_spkid
 */

bool DataModel::replaceSpeaker(const string& old_spkid,
		const string& new_spkid, const string& turnid, bool emit_signal)
{
	bool done = true;

	// check new speaker id is valid
	if ( !getSpeakerDictionary().existsSpeaker(new_spkid) )
		return false;

	if (turnid != "")
	{
		if (ExistsAnnotation(turnid) )
		{
			if (getElementProperty(turnid, "speaker") == old_spkid)
				setElementProperty(turnid, "speaker", new_spkid, emit_signal);
		}
	}
	else
	{
		string graphtype = "transcription_graph";
		const string& type = m_conventions.getTypeFromFeature("speaker", graphtype);
		if ( type.empty() || m_agIds.find(graphtype) == m_agIds.end() ) return false;

		const set<AnnotationId>& ids = GetAnnotationSetByFeature(m_agIds[graphtype],
				"speaker", old_spkid, type);
		set<AnnotationId>::const_iterator it;
		for (it = ids.begin(); it != ids.end(); ++it)
			setElementProperty(*it, "speaker", new_spkid, emit_signal);
	}
	return done;
}

bool DataModel::existsAnchor(const string& id)
{
	return ExistsAnchor(id) ;
}

void DataModel::getInAndOutAnnotationAtAnchor(const string anchor, std::set<string>& incoming, std::set<string>& outgoing)
{
	if (!existsAnchor(anchor))
		return ;

	outgoing = GetOutgoingAnnotationSet(anchor, "") ;
	incoming = GetIncomingAnnotationSet(anchor, "") ;
}

string DataModel::print_annotation(const string& id, bool with_offset)
{
	if (id.empty())
		return "no-id" ;

	if (!ExistsAnnotation(id))
		return "element no longer exist" ;

	ostringstream os;
	os << GetAnnotationType(id) << " " << id;

	AnchorId start_a = GetStartAnchor(id) ;
	AnchorId end_a = GetEndAnchor(id) ;
	bool start_isAnchored = GetAnchored(start_a) ;
	bool end_isAnchored = GetAnchored(end_a) ;

	os << " (" << start_a <<" , " << end_a << ")" ;
	if (with_offset)
	{
		os << " {" ;
		if (start_isAnchored)
			os << GetAnchorOffset(start_a) ;
		else
			os << "noanchor" ;

		os <<" - " ;

		if (end_isAnchored)
			os << GetAnchorOffset(end_a) ;
		else
			os << "noanchor" ;
		os << "}" ;
	}

	return os.str() ;
}

string DataModel::print_anchor(const string& id)
{
	if (id.empty())
		return "no-id" ;

	if (!ExistsAnchor(id))
		return "element no longer exist" ;

	ostringstream os;
	bool isAnchored = GetAnchored(id) ;

	os << id << " {" ;
	if (isAnchored)
		os <<  GetAnchorOffset(id);
	else
		os << "noanchor" ;
	return os.str() ;
}


float DataModel::getElementOffset(const string& id, bool start)
{
	if (!existsElement(id))
		return -1 ;

	AnchorId anchor ;
	if (start)
		anchor = GetStartAnchor(id) ;
	else
		anchor = GetEndAnchor(id) ;

	if (!anchor.empty() && GetAnchored(anchor))
		return GetAnchorOffset(anchor) ;
	else
		return -1 ;
}

bool DataModel::setElementOffset(const string& id, float off, bool start, bool emitSignal)
{
	if (!existsElement(id) || off < 0 )
		return false ;

	AnchorId anchor ;
	if (start)
		anchor = GetStartAnchor(id) ;
	else
		anchor = GetEndAnchor(id) ;

	if (!anchor.empty() /*&& GetAnchored(anchor)*/)
	{
		setAnchorOffset(anchor, off, false, emitSignal) ;
		return true ;
	}
	else
		return false ;
}

bool DataModel::unsetElementOffset(const string& id, bool start, bool emitSignal)
{
	if ( !existsElement(id) )
		return false ;

	string idToCheck = id ;
	// qualifier ? find corresponding unit
	if (!isMainstreamType(getElementType(id)))
	{
		idToCheck = getParentElement(id, mainstreamBaseType(), true) ;
		if ( !existsElement(idToCheck) )
			return false ;
	}

	AnchorId anchor ;
	// Never remove timestamp for first anchor of segment
	if (start && !isFirstChild(idToCheck, ""))
		anchor = GetStartAnchor(idToCheck) ;
	// Never remove timestamp for last anchor of segment
	else if (!isLastChild(idToCheck, ""))
		anchor = GetEndAnchor(idToCheck) ;

	if ( !anchor.empty() && GetAnchored(anchor) )
	{
		unsetAnchorOffset(anchor, emitSignal) ;
		return true ;
	}
	else
		return false ;
}

std::set<string> DataModel::getIncomingAnnotations(const string& id, const string& type)
{
	return GetIncomingAnnotationSet(id, type) ;
}

bool DataModel::isActiveBackground(const string& id)
{
	const string& type = getElementType(id) ;
	if (type=="background")
	{
		const string& test = getElementProperty(id, "type") ;
		if (test.empty() || test.compare("none")==0)
			return false ;
		else
			return true ;
	}
	return true ;
}

void DataModel::addAnchorToTrackMap(const string& anchorId, int notrack)
{
	if (anchorId.empty() || notrack<0)
		return ;
	m_anchorTrack[anchorId] = notrack ;
}

void DataModel::setUpdated(bool b)
{
	m_updated = b ;
	if (!closing)
		m_signalModelUpdated.emit(b) ;
}

//******************************************************************************
//******************************* Discret Graph ********************************
//******************************************************************************

std::string DataModel::createParseElement(const string& prevId, const string& graphtype, const string& annottype, int notrack, float start, float end)
{
	if (prevId.empty())
	{
		string startanchor = createAnchor(getAG(graphtype), notrack, start) ;
		string endanchor = "" ;
		if (start<0 || end <0)
			return "" ;
		if (start!=end)
		{
				endanchor = createAnchor(getAG(graphtype), notrack, end) ;
			if (!startanchor.empty() && !endanchor.empty())
				return agCreateAnnotation(getAG(graphtype), startanchor, endanchor, annottype) ;
			else
				return "" ;
		}
		else
			return agCreateAnnotation(getAG(graphtype), startanchor, startanchor, annottype) ;
	}
	else
	{
		string prev_end_endanchor = GetEndAnchor(prevId) ;
		if (!GetAnchored(prev_end_endanchor)) {
			TRACE << "createDiscretElement ERROR: prevId end-anchor not anchored!" << std::endl ;
			return "" ;
		}
		float prev_end = GetAnchorOffset(prev_end_endanchor) ;
		if (prev_end!=start) {
			TRACE << "createDiscretElement ERROR: prevId end-anchor <> new start !" << std::endl ;
			fprintf(stderr, "\tprev=%f\n", prev_end) ;
			fprintf(stderr, "\tstart=%f\n\n", start) ;
			return "" ;
		}
		if (start==end)
			return agCreateAnnotation(getAG(graphtype), prev_end_endanchor, prev_end_endanchor, annottype) ;
		else  {
			string end_anchor = createAnchor(getAG(graphtype), notrack, end) ;
			return agCreateAnnotation(getAG(graphtype), prev_end_endanchor, end_anchor, annottype) ;
		}
	}
}

/*
 * Create a parse element in given graphtype, on given track, from first_element to last_element
 * included
 */
std::string DataModel::createParseElement(const string& graphtype, const string& annottype, int notrack,
												const string& first_element, const string& last_element)
{
	if (graphtype.empty() || annottype.empty() || first_element.empty() || last_element.empty()) {
		TRACE << "DataModel::createDiscretElement with anchor:> Invalid Data IN" << std::endl ;
		return "" ;
	}

	AnchorId start = GetStartAnchor(first_element) ;
	AnchorId end = GetEndAnchor(last_element) ;

	return agCreateAnnotation(getAG(graphtype), start, end, annottype) ;
}

/**
* create new anchor at given offset even if an anchor at same place already exist
* @param notrack signal track no
* @param offset offset in signal
* @param policy insertion policy (defaults to ADJUST_PREVIOUS)
* @return created anchor id / existing anchor id
*/
string DataModel::createAnchor(const string& graphId, int notrack, float offset)
{
	string id = "";
	try {
		set<SignalId> sigIds;
		sigIds.insert( signalCfg.getSigid(notrack) ) ;
		id = agCreateAnchor(graphId, offset, TRANS_ANCHOR_UNIT, sigIds);
		m_anchorTrack[id] = notrack;
	}
	catch (AGException& e) {
		MSGOUT << "Caught AGException : " << e.error() << ", id=" << id << endl;
	}
	return id;
}

string DataModel::getAnnotationByOffset(const string& graphtype, float offset, const string& type)
{
	if (graphtype.empty() || offset<0)
		return "" ;

	std::map<string,string>::iterator it = m_agIds.find(graphtype) ;
	if (it!=m_agIds.end())
		return GetAnnotationByOffset(it->second, offset, type) ;
	else
		return "" ;
}


/*
 * get transcription language for given track
 */
const string& DataModel::getTranscriptionLanguage(int notrack)
{
	if ( m_vlang.size() == 0 ) {
		string lang=getAGSetProperty("lang","");
		if ( ! lang.empty() ) {
			StringOps(lang).split(m_vlang, "/;,", false);
			int i;
			for (i=0; i < m_vlang.size(); ++i )
				if ( m_vlang[i] == "" ) m_vlang[i] = "unk";
		} else m_vlang.push_back("unk");
		while ( m_vlang.size() < getNbTracks() ) m_vlang.push_back(m_vlang[0]);
	}
	if ( notrack < 0 ) notrack = 0;
	else if (notrack >= m_vlang.size()) notrack = m_vlang.size()-1;
	return m_vlang[notrack];

}



// to optimize searches for annotations related to a given
//  signal track -> store track associated to anchor id
//
//  HINT : in TransAG format we consider that an anchor
//    may be associated to a single track only
//
void DataModel::updateAnchorTrackMap(const string& graphId, std::map<string, int>& anchorTrack)
{
	list<AnchorId> anchorIds = GetAnchorSet(graphId) ;
	list<AnchorId>::iterator it2;
	int notrack;
	// For each graph, let's loop over all its anchors
	for ( it2=anchorIds.begin(); it2 != anchorIds.end(); ++it2 )
	{
		// Treat only time coded anchors
		if ( GetAnchored(*it2) )
		{
			// For each anchor, get the associated signals (should be only one)
			set<SignalId> sigs = GetAnchorSignalIds(*it2);
			set<SignalId>::iterator it3;
			for ( it3 = sigs.begin(); it3 != sigs.end(); ++it3 )
			{
				if ( (*it3).empty() )
					Log::err() << "updating anchor map: empty signal found for anchor " << *it2 << ", stop." << std::endl ;

//				std::string signal_s = GetSignalTrack(*it3) ;
//				int signal = string_to_number<int>(signal_s) ;

				// in file, notrack ranges from 1 to n
				// in memory it ranges from 0 to n-1;
//				notrack = signal - 1 ;
				if ( signalCfg.getNbSignals("") > 0)
					notrack = signalCfg.getNotrack(*it3) ;
				else
				{
					int signal = string_to_number<int>(GetSignalTrack(*it3)) ;
					notrack = signal - 1 ;
				}
				anchorTrack[*it2] = notrack;
			}
		}
	}
}

string DataModel::toString(const string& id)
{
	string res = "" ;
	if (!existsElement(id))
		return res ;

	const string& astart = GetStartAnchor(id) ;
	const string& aend = GetEndAnchor(id) ;
	const string& fstart = float_to_string(isAnchoredElement(id, true)) ;
	const string& fend = float_to_string(isAnchoredElement(id, false)) ;
	const string& type = GetAnnotationType(id) ;
	const string& graphtype = getGraphType(id);

	res = res + StringOps(type).toUpper() +  " " + id
			+  " { " + astart + "(" + fstart + ")"
			+ " - " + aend + "(" + fend + ")" + " }" ;
	res = res + "\n graph: " + graphtype ;

	const map<string,string>& features = GetFeatures(id) ;
	map<string,string>::const_iterator it ;
	for (it=features.begin(); it!=features.end(); it++)
		res = res + "\n\t" + it->first + " = " + it->second ;

	return res ;
}


/**
 * make unique AGSetId
 * @return unique AGSet id
 */
string DataModel::mktmpAGSetId()
{
	char buf[10];
	unsigned int t = ((unsigned int)time(0) & 0x00FFFFFF);
	do {
		sprintf(buf, "A%06X", t);
		++t;
	} while ( ExistsAGSet(buf) );
	return buf;
}

string DataModel::getAnchor(const string& id, bool start)
{
	if (start)
		return GetStartAnchor(id) ;
	else
		return GetEndAnchor(id) ;
}

void DataModel::setAnchor(const string& id, const string& anchorId, bool start)
{
	if (start)
		return SetStartAnchor(id, anchorId) ;
	else
		return SetEndAnchor(id, anchorId) ;
}

/*! return true if segment id1 and id2 overlap each other */
bool DataModel::overlaps(const string& id1, const string& id2, float min_over)
{
	float so = getStartOffset(id1);
	float eo = getEndOffset(id1);
	return overlaps(id2, so, eo, min_over);
}

/*! return true if segment id1 and id2 overlap each other */
bool DataModel::overlaps(const string& id, float so, float eo, float min_over)
{
	float sb = getStartOffset(id);
	float eb = getEndOffset(id);

	if ( eb < so || sb > eo ) return false;
	if ( eo < sb || so > eb ) return false;
	if ( so <= sb ) {
		if ( eo < eb ) return ((eo-sb) >= min_over);
		else return ((eb-sb) >= min_over);
	} else {
		if ( eb < eo ) return ((eb-so) >= min_over);
		else return ((eo-so) >= min_over);
	}
}


} /*  namespace tag */
