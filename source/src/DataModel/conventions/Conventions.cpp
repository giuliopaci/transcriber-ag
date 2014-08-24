/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* @file Conventions.cpp
* 	annotation conventions manager for transcriber DataModel
*/

#include <ag/AGAPI.h>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <glib.h>
#include <glib/gstdio.h>

#include "Conventions.h"

#include "DataModel/DataModel.h"
#include "Common/util/Log.h"
#include "Common/util/StringOps.h"
#include "Common/FileInfo.h"
#include "Common/Parameters.h"
#include "Common/util/Log.h"
#include "Common/util/Utils.h"
#include "Common/iso639.h"


using namespace std;

#ifdef WIN32
#define DEFAULT_CFG_DIR "c:\\Program Files\\TranscriberAG\\etc\\conventions"
#else
#define DEFAULT_CFG_DIR "/etc/TransAG/conventions"
#endif

#define DEFAULT_CONVENTIONS "transag_default"

namespace tag {

string Conventions::_configDir(DEFAULT_CFG_DIR);



//------------------------------------------------------------------------------
//									CONSTRUCTOR
//------------------------------------------------------------------------------

Conventions::Conventions()
: m_configDir(_configDir), m_currentFile("")
{
	_undef="";
}


//------------------------------------------------------------------------------
//									LOADING
//------------------------------------------------------------------------------

/* configure data model */
void Conventions::configure(string conventions, string lang, bool fullmode)
{
	if ( conventions.empty() )
	{
		if (! m_currentFile.empty() ) // already done
			return;
		conventions = DEFAULT_CONVENTIONS;
	}

	m_configDir = _configDir;
	

	if ( ! FileInfo(m_configDir).exists() )
		m_configDir = "";

//	else {
//		if ( *(m_configDir.rbegin()) != '/' )
//		m_configDir += "/";
//	}

	TRACE_D << "CONVENTIONS DIR= " <<  m_configDir << std::endl ;

	string tail = FileInfo(conventions).tail();
	if ( tail == "" )
		conventions += ".rc";

	if ( ! g_path_is_absolute(conventions.c_str()) && !m_configDir.empty() )
	{
			// lookup in cfgdir
		conventions = Glib::build_filename(m_configDir, conventions) ;
//				FileInfo(m_configDir).join(conventions);
	}
	if ( ! FileInfo(conventions).exists() )
	{
		string msg = "Convention file not found : " + conventions;
		//throw msg.c_str();
		TRACE << "(Conventions::configure) " << msg << endl;
		return;
	}

	// already loaded
	if ( conventions == m_currentFile && lang == m_currentLang)
		return;

	try
	{
		Parameters param;
		TRACE << "++++ Load conventions from " << conventions << endl;
		param.load(conventions);
		string component = "Conventions";
		if ( ! param.existsParametersMap(component) )
			component = "conventions";
		m_config.clear();
		m_config = param.getParametersMap(component);
		m_configDir = FileInfo(conventions).dirname();

		m_currentFile = conventions;
		m_currentLang = lang;
		if (param.existsParameter(component, "reference,version"))
			m_version = m_config["reference,version"] ;
		else
			m_version = "-1" ;

		m_conventions = FileInfo(conventions).Basename();
		string tail = FileInfo(m_conventions).tail();
		m_conventions = m_conventions.substr(0, m_conventions.length()-tail.length());
		TRACE << "++++ done load conventions from " << conventions << endl;
	}
	catch ( const char* msg )
	{
		MSGOUT << "Sax error = " << msg << endl;
		throw msg;
	}
	catch (...)
	{
		string msg = "Exception caught when loading conventions  file ";
		msg += conventions;
		throw msg.c_str();
	}

	m_usesVideoSignal = false;

	// for each annotation graph type, create or update GraphDescriptor object
	m_graphDesc.clear() ;
	map<string, string>::iterator itm = m_config.begin();
	Parameters::KeyLookup is_graph("graphs,");
	while ( (itm = find_if(itm, m_config.end(), is_graph)) != m_config.end() )
	{
		const string& val = itm->second;
		if (  val == "true" || val == "1" || val =="yes") {
			vector<string> names;
			StringOps(itm->first).split(names, ",");
			updateGraphDescriptor(names[1], NULL, false);
		}
		++itm;
	}

	//> actualize available graph types
	m_graphTypes.clear();
	GraphDescriptorIter it;
	for ( it=m_graphDesc.begin(); it != m_graphDesc.end(); ++it ) {
		TRACE << "conventions -> graph " << it->first << endl;
		m_graphTypes.push_back(it->first);
	}

	//> add extra graphs
	addExtraGraphs() ;

	//> load annotation labels
	m_config["layout,labels"] = loadLocalizedLayout(m_config["layout,labels"]);

	//> set qualifier sorting
	m_sortQualifiersForLayout = (m_config["layout,sort_qualifiers"] == "true");

	//> load corpus version - DEPRECATED - could be used
	m_corpusName = m_config["corpus,name"] ;
	m_corpusVersion = m_config["corpus,version"] ;

	//> set input Handling
	m_spaceHandling = (m_config["space_handling,single_space"] == "true");
	Glib::ustring list = m_config["space_handling,bordering_spaces"] ;
	Glib::ustring::iterator its ;
	for (its=list.begin(); its!=list.end(); its++)
 		m_borderedChars.insert( m_borderedChars.end(), *its ) ;

	if ( fullmode )
	{
		//> LOAD EXTRA PROPERTIES AND LABELS
		string extra = getConfiguration("extra,properties");
		if (extra != "" )
		{
			string tail =FileInfo(extra).tail();
			if ( tail == "" ) extra += ".rc";
			extra = FileInfo(m_configDir).join(extra);
			if ( ! FileInfo(extra).exists() )
				setConfiguration("extra,properties", "");
			else
				setConfiguration("extra,properties", extra);
		}

		std::map <std::string, std::string>::iterator it ;

		//> Configure extra words lists
		it = m_config.find("extra,wordlists") ;
		if ( it != m_config.end() && it->second != "" )
			loadWordLists(it->second, lang) ;

		//> Configure topic list
		it = m_config.find("topics,topiclist") ;
		if ( it != m_config.end() && it->second != "" )
			loadTopics(it->second, lang) ;
	}
}

void Conventions::updateGraphDescriptor(const string& graphtype, Parameters* param, bool reset_mode)
{
	std::map<std::string, std::string> configuration ;
	if (!param)
		configuration = m_config ;
	else
		configuration = param->getParametersMap("Conventions");

	m_graphDesc[graphtype].maxOverlap = 0;

	updMainstreamTypes(graphtype, param, reset_mode);
	updQualifierTypes(graphtype, param);
	updSignalTypes(graphtype, param);
#ifdef TODEL
	updMainstreamBaseSubtype(graphtype, param) ;
#endif

	m_graphDesc[graphtype].type = configuration[graphtype+",type"];

	string val = configuration[graphtype+",continuous"];
	StringOps(val).toLower();
	m_graphDesc[graphtype].isContinuous = (  val == "true" || val == "1" || val =="yes");
	val = configuration[graphtype+",resize,keep_attachments"];
	StringOps(val).toLower();
	m_graphDesc[graphtype].resizeKeepAttachments = (  val == "true" || val == "1" || val =="yes");
	val = configuration[graphtype+",overlap,alignment"];
	StringOps(val).toLower();
	m_graphDesc[graphtype].overlapAlignment = (  val == "true" || val == "1" || val =="yes");
}


int Conventions::addExtraGraphs()
{
	int cpt = 0 ;
	std::map<string,Parameters*>::iterator it ;
	for (it=m_graphToAdd.begin(); it!=m_graphToAdd.end(); it++)
	{
		string gtype = it->first ;
		Parameters* param = it->second ;
//TODO check if already exists ?
		if (param)
			updateGraphDescriptor(gtype, param, false) ;
		cpt ++ ;
	}
	return cpt ;
}

bool Conventions::addGraphDescription(const string& graphtype, Parameters* description)
{
	if (!description)
		return false ;

	if (hasGraphType(graphtype))
		return false ;

	m_graphToAdd[graphtype] = description ;
	updateGraphDescriptor(graphtype, description, false) ;

	return true ;
}

string Conventions::loadLocalizedLayout(const string& path)
{
	m_labels.clear();

	string cfgfic = FileInfo(m_configDir).join(path);

	string tail = FileInfo(cfgfic).tail();
	if ( tail != "" ) cfgfic = cfgfic.substr(0, cfgfic.length() - tail.length());
	else tail = ".rc";

	// check if localized annotations layout configuration
	string lang = ISO639::getLocale();

	string localized_cfg = cfgfic + string("_") + lang;
	localized_cfg += tail;
	if ( FileInfo(localized_cfg).exists() ) cfgfic = localized_cfg;
	else cfgfic += tail;


	if ( FileInfo(cfgfic).exists() ) {

		try {
			TRACE << "load annotation layout from file " << cfgfic << endl;
			m_layout.load(cfgfic);
			vector<string>::const_iterator it;
			vector<string>::iterator it2, ittyp;

			//> For all types of graph
			for ( ittyp = m_graphTypes.begin(); ittyp != m_graphTypes.end(); ++ittyp )
			{
				//> GET ALL LAYOUTS FOR MAINSTREAM TYPES
				const vector<string>& mainstream = getMainstreamTypes(*ittyp);
				for ( it = mainstream.begin(); it != mainstream.end(); ++it )
				{
					// get labels for mainstreams label
					m_labels[*it] = m_layout.getParameterValue("Labels", "label", *it);
					if ( m_labels[*it].empty() )
						m_labels[*it] = *it;

					// get all subtypes of mainstream and find their labels
					const string& subs = getConfiguration(*it+",subtypes");
					if ( ! subs.empty() )
					{
						vector<string> v;
						StringOps(subs).split(v,";,");
						for (it2 = v.begin(); it2 != v.end(); ++it2)
						{
							// don't change if already in map or if no traduction found
							if (m_labels.find(*it2) == m_labels.end() || m_labels[*it2] == *it2) {
								m_labels[*it2] = m_layout.getParameterValue("Menu", *it, *it2);
								if ( m_labels[*it2].empty() )
									m_labels[*it2] = *it2;
								else {
									unsigned long pos = m_labels[*it2].find("_");
									if ( pos != string::npos ) m_labels[*it2].erase(pos, 1);
								}
							}
						}
					}

					// get all levels of mainstream if exists and find their labels
					// TODO: do it dynamically searching for all additional parameters
					const string& levels = getConfiguration(*it+",levels");
					if ( !levels.empty() ) {
						vector<string> v;
						StringOps(levels).split(v,";,");
						for (it2 = v.begin(); it2 != v.end(); ++it2)
						{
							// don't change if already in map or if no traduction found
							if (m_labels.find(*it2) == m_labels.end()  || m_labels[*it2] == *it2) {
								m_labels[*it2] = m_layout.getParameterValue("Menu", *it, *it2);
								if ( m_labels[*it2].empty() )
									m_labels[*it2] = *it2;
								else {
									unsigned long pos = m_labels[*it2].find("_");
									if ( pos != string::npos ) m_labels[*it2].erase(pos, 1);
								}
							}
						}
					}
				}

				//> GET ALL LAYOUTS FOR QUALIFIER
				const vector<string>& qualifiers = getQualifierTypes("", *ittyp);
				for ( it = qualifiers.begin(); it != qualifiers.end(); ++it )
				{
					// don't change if already in map or if no traduction found
					if (m_labels.find(*it) == m_labels.end() || m_labels[*it] == *it) {
						m_labels[*it] = m_layout.getParameterValue("Menu", *it, "label");
						if ( m_labels[*it].empty() )
							m_labels[*it] = *it;
						else {
							unsigned long pos = m_labels[*it].find("_");
							if ( pos != string::npos )
								m_labels[*it].erase(pos, 1);
						}
					}
				}
			} //end for each graph type
		}
		catch ( const char* msg) {
			TRACE << " ERROR LOAD " << cfgfic << endl << msg << endl;
			cfgfic = "";
		}
	} else {
		TRACE << "Warning : file not found : " << cfgfic << endl;
		cfgfic = "";
	}

	return cfgfic;
}

/**
* load predefined word lists for specified transcription language
* @param path base path for wordlist file
* @param lang current transcription language (optional, defaults to current locale)
* @note: wordlist file lookup is done in following way :
*   - look for "<path>.<lang>"
*   - if not found look for <path>
*/
void Conventions::loadWordLists(const string& path, string lang)
{
	m_wordLists.clear();

	string cfgfic = FileInfo(m_configDir).join(path);

	if ( lang.empty() )  lang = ISO639::getLocale();

	string localized_cfg = cfgfic + string(".") + lang;
	if ( FileInfo(localized_cfg).exists() ) cfgfic = localized_cfg;

	if ( FileInfo(cfgfic).exists() ) {

		try {
			TRACE << "loading words lists from file " << cfgfic << endl;
			WordList::loadLists(cfgfic, m_wordLists);
		} catch ( const char* msg) {
			TRACE << " ERROR LOADING " << cfgfic << endl << msg << endl;
			cfgfic = "";
		}
	} else {
		TRACE << "Warning : word list file not found : " << cfgfic << endl;
		cfgfic = "";
	}
}

/**
* load predefined topics for annotation
* @param path base path for wordlist file
* @param lang current transcription language (optional, defaults to current locale)
* @note: topic file lookup is done in following way :
*   - look for "<path>.<lang>"
*   - if not found look for <path>
*/
void Conventions::loadTopics(const string& path, string lang)
{
	m_Topics.clear();

	string cfgfic = FileInfo(m_configDir).join(path);


	if ( lang.empty() )  lang = ISO639::getLocale();

	string localized_cfg = cfgfic + string(".") + lang;
	if ( FileInfo(localized_cfg).exists() ) 
		cfgfic = localized_cfg;

	if ( FileInfo(cfgfic).exists() ) {
		try {
			TRACE << "loading topics from file " << cfgfic << endl;
			Topics::loadTopics(cfgfic, m_Topics);
		} catch ( const char* msg) {
			TRACE << " ERROR LOADING " << cfgfic << endl << msg << endl;
			cfgfic = "";
		}
	} else {
		TRACE << "Warning : topics not found : " << cfgfic << endl;
		cfgfic = "";
	}
}

/**
 * get configuration option value
 * @param key option key
 * @return option value
 */
const std::string& Conventions::getConfiguration(const std::string& key, std::map<std::string, std::string> configuration) const
{
	map<string, string>::const_iterator it = configuration.find(key);
	if ( it != configuration.end() )
		return it->second;
	return _undef;
}

const std::string& Conventions::getConfiguration(const std::string& key) const
{
	return getConfiguration(key, m_config) ;
}


const std::string& Conventions::getLayoutItem(const string& component, const string& item, const string& suffix)
{
	if ( suffix.empty() )
		return 	m_layout.getParameterValue(component, item);
	else {
		string key = item +"_"+suffix;
		return m_layout.getParameterValue(component, key);
	}
}

/**
 * return base mainstream annotation type for current graph type
 * @param graphtype
 * @return base segment type
 */
const string& Conventions::mainstreamBaseType(const string& graphtype)
{
	GraphDescriptorIter itg = m_graphDesc.find(graphtype);
	if ( itg != m_graphDesc.end() && itg->second.mainstream.size() > 0  )
		return itg->second.mainstream.back();
	return _undef;
}


bool Conventions::isQualifierType(const string& type, const string& graphtype)
{
	map<string, vector<string> >::iterator itc;
	for ( itc=m_graphDesc[graphtype].qualifierClass.begin(); itc !=  m_graphDesc[graphtype].qualifierClass.end(); ++itc ) {
		vector<string>::iterator itv;
		for ( itv=itc->second.begin(); itv != itc->second.end() && (*itv) != type; ++itv );
		if ( itv != itc->second.end() ) return true;
	}
	return false;
}

bool Conventions::isQualifier(const string& type, const string& desc, const string& graphtype, bool& typeOK, bool& descOK)
{
	typeOK = isQualifierType(type, graphtype) ;
	descOK = false ;

	if (!typeOK)
		return false ;
	else
	{
		string key = type + ",subtypes" ;
		std::map<string,string>::iterator it = m_config.find(key) ;
		if (it!=m_config.end())
		{
			string subtypes = it->second ;
			vector<string> v_subtypes ;
			StringOps ops(subtypes) ;
			ops.split(v_subtypes, ";") ;
			descOK = (find(v_subtypes.begin(), v_subtypes.end(), desc) != v_subtypes.end());
		}
		return descOK ;
	}
}

/**
 * get mainstream annotation types for given graph type for current conventions
 * @param graphtype	 graph type
 * @return vector of mainstream annotation types
 */
const vector<string>& Conventions::getMainstreamTypes(const string& graphtype)
{
	GraphDescriptorIter it = m_graphDesc.find(graphtype) ;
	if ( it!=m_graphDesc.end() )
		return it->second.mainstream ;
	else
		return _no_type_v ;
}

/**
 * return base segmentation annotation type (ie defining a timeline segmentation) for current graph type
 * @param graphtype
 * @return base segment type
 */
const string& Conventions::segmentationBaseType(const string& graphtype)
{
	GraphDescriptorIter it = m_graphDesc.find(graphtype) ;
	if ( it!=m_graphDesc.end() )
		return it->second.segmentationBaseType ;
	else
		return _no_type_s ;
}

/**
 * check if graph type is defined as continuous (all annotations must link together) or discrete
 * @param graphtype
 * @return true if continuous, else false
 */
bool Conventions::isContinuousGraph(const std::string& graphtype)
{
	GraphDescriptorIter it = m_graphDesc.find(graphtype) ;
	if ( it!=m_graphDesc.end() )
		return it->second.isContinuous ;
	else
		return false ;
}


/** return true if given qualifier type belongs to given qualifier class */
bool Conventions::isQualifierClassType(const string& qclass, const string& type, const string& graphtype)
{
	map<string, vector<string> >::iterator it = m_graphDesc[graphtype].qualifierClass.find(qclass);
	if ( it != m_graphDesc[graphtype].qualifierClass.end() ) {
		vector<string>::iterator itv;
		for ( itv=it->second.begin(); itv != it->second.end() && (*itv) != type; ++itv );
		return ( itv != it->second.end() );
	}
	return false;
}

const vector<string>& Conventions::getQualifierTypes(const string& qclass, const string& graphtype)
{
	if ( ! ( qclass.empty() || m_graphDesc[graphtype].qualifierClass.find(qclass) ==  m_graphDesc[graphtype].qualifierClass.end()) )
		return m_graphDesc[graphtype].qualifierClass[qclass];
	return m_graphDesc[graphtype].qualifiers;
}

const std::string& Conventions::getQualifierClass(const string& type, const string& graphtype)
{
	map<string, vector<string> >::iterator it;
	vector<string>::iterator itv;

	for ( it= m_graphDesc[graphtype].qualifierClass.begin();  it != m_graphDesc[graphtype].qualifierClass.end(); ++it ) {
		for ( itv=it->second.begin(); itv != it->second.end() && (*itv) != type; ++itv );
		if ( itv != it->second.end() ) return it->first;
	}
	return _undef;
}

bool Conventions::isMainstreamType(const string& type, const string& graphtype)
{
	if (graphtype.empty())
	{
		bool found = false ;
		GraphDescriptorIter it_graph ;
		for ( it_graph = m_graphDesc.begin(); it_graph != m_graphDesc.end() && !found; ++it_graph )
		{
			const vector<string>& mainstream = getMainstreamTypes(it_graph->first);
			vector<string>::const_iterator it;
			for ( it=mainstream.begin(); it != mainstream.end() ; ++it) {
				if (type==*it)
					found = true ;
			}
		}
		return found ;
	}
	else
	{
		const vector<string>& mainstream = getMainstreamTypes(graphtype);
		vector<string>::const_iterator it;
		for ( it=mainstream.begin(); it != mainstream.end() && *it != type; ++it);
		return (it != mainstream.end());
	}
}


/**
 * Checks if some qualifier annotations may span over this type
 * @param type 			Annotation type
 * @param graphtype 	Graph type, or empty for automatic guess
 * @return 				True if can be spanned over, else false
 */
bool Conventions::isSpannableType(const string& type, const string& graphtype)
{
	GraphDescriptorIter itg;
	for (itg = m_graphDesc.begin(); itg != m_graphDesc.end(); ++itg ) {
		if ( graphtype.empty() || itg->first == graphtype )
			if (itg->second.spannables.find(type) != itg->second.spannables.end() )
				return true;
	}
	return false;
}

bool Conventions::isValidType(const string& type, const string& graphtype)
{
	GraphDescriptorIter itg;
	for (itg = m_graphDesc.begin();itg != m_graphDesc.end(); ++itg ) {
		if ( graphtype.empty() || itg->first == graphtype )
			if (itg->second.types.find(type) != itg->second.types.end() )
				return true;
	}
	return false;
}

bool Conventions::isValidSubtype(const string& subtype, const string& type, const string& graphtype)
{
	if ( isMainstreamType(type, graphtype) ) {
		vector<string> subtypes ;
		if ( mainstreamHasSubtypes(type, graphtype, subtypes) )
			return (find(subtypes.begin(), subtypes.end(), subtype) != subtypes.end());
		return false;
	} else {
		bool is_type_editable = typeCanBeEdited(type) ;

		//> editable, each sub-type is valid
		if (is_type_editable)
			return true ;
		//> no subtype, of course it's ok
		else if (subtype.empty() || subtype.compare(" ")==0 )
			return true ;
		//> otherwise, check in sub-type list
		else
		{
			vector<string> subtypes ;
			getAnnotationItems(subtypes, type, "subtypes") ;
			return 	(find(subtypes.begin(), subtypes.end(), subtype) != subtypes.end());
		}
	}
}

bool Conventions::mainstreamHasSubtypes(const string& type, const string& graphtype, std::vector<std::string>& subtypes)
{
	GraphDescriptorIter itg;
	for (itg = m_graphDesc.begin(); itg != m_graphDesc.end(); ++itg )
	{
		if ( graphtype.empty() || itg->first == graphtype )
		{
			TypeDescriptorIter it = itg->second.types.find(type);
			if ( it != itg->second.types.end() )
			{
				map<string, string>::const_iterator its;
				string text_type = type + "_text";
				for (its = it->second.submain.begin(); its != it->second.submain.end(); ++its ) {
					int l = its->first.length();
					if ( ( l > 5 && its->first.compare( l-5, 5, "_text") == 0) || its->first == "none" )
						subtypes.insert(subtypes.begin(), its->first);
					else subtypes.push_back(its->first);
				}
			}
		}
	}
	return (subtypes.size() > 0) ;
}

/*
 *  1:  annotation is valid.\n
 * -1:	submain is not valid for given type\n
 * -2:	value is not valid for given submain\n
 * -3:	desc is not valid for given value\n
 */
int Conventions::isValidMainstreamBaseType(const string& graphtype, const string& submain, const string& value, const string& desc)
{
	std::vector<string> vectmp ;
	std::vector<string>::iterator itmp ;
	GraphDescriptorIter itg = m_graphDesc.find(graphtype);
	if ( itg != m_graphDesc.end() )
	{
		TypeDescriptorIter it = itg->second.types.find(itg->second.mainstream.back()) ;
		map<string, string>::const_iterator its;
		for (its = it->second.submain.begin(); its != it->second.submain.end(); ++its )
		{
			if ( its->first == submain )
			{
				// submain is empty : text element, OK
				if ( its->second.empty() )
					return 1;

				// submain has pre-defined value: find "value" in list
				if ( its->second.find(value) == string::npos )
					return -2;

				// submain has "value" in pre-defined list : now check desc
				if ( !isValidSubtype(desc, value, graphtype) )
					return -3 ;

				// all traps successed ? congratulations :)
				return 1;
			}
		}
		// no submain ? no problem :)
		return 1;
	}
	return -1;
}


const std::map< std::string, std::string> & Conventions::getTypeFeatures(const std::string& type, const std::string& graphtype)
{
	GraphDescriptorIter itg;
	for (itg = m_graphDesc.begin(); itg != m_graphDesc.end(); ++itg ) {
		if ( graphtype.empty() || itg->first == graphtype ) {
			TypeDescriptorIter it = itg->second.types.find(type);
			if ( it != itg->second.types.end() )
				return it->second.features;
		}
	}
	return _emptyFeat;
}

bool Conventions::isValidFeature(const std::string& feature, const std::string& type, const std::string& graphtype)
{
	if (type.empty() || feature.empty())
		return false ;

	const std::map< std::string, std::string>& features = getTypeFeatures(type, graphtype);
	 std::map<std::string, std::string>::const_iterator it = features.find(feature) ;
	 return ( it != features.end() ) ;
}

string Conventions::getSubtypeFeatureName(const string& type, const string& graphtype)
{
	if (isQualifierType(type, graphtype))
		return "desc" ;
	else
		return "" ;
}

bool Conventions::typeCanBeNormalized(const string& type)
{
	string key = type + ",allow_norm" ;
	if ( getConfiguration(key) == "true" )
		return true ;
	else
		return false ;
}

bool Conventions::typeCanBeEdited(const string& type)
{
	string key = type + ",editable" ;
	if (getConfiguration(key) == "true" )
		return true ;
	else
		return false ;
}
/**
 * get parent type for given annotation type :
 * - mainstream type : upper level mainstream type
 * - qualifier type : base segment type for graph type.
 * @param type item type
 * @param graphtype  graph type
 * @return  parent type
 */
const string& Conventions::getParentType(const string& type, const string& graphtype)
{
	GraphDescriptorIter itg;
	for ( itg = m_graphDesc.begin(); itg != m_graphDesc.end(); ++itg ) {
		if ( graphtype.empty() || itg->first == graphtype ) {
			TypeDescriptorIter it = itg->second.types.find(type);
			if ( it != itg->second.types.end() )
				return it->second.parentType;
		}
	}
	return _undef;
}

/**
 * get direct child type for given annotation type :
 * - mainstream type : lower level mainstream type
 * - qualifier type : ""
 * @param type item type
 * @param graphtype  graph type
 * @return  child type
 */
const string& Conventions::getChildType(const string& type, const string& graphtype)
{
	GraphDescriptorIter itg;
	for ( itg = m_graphDesc.begin(); itg != m_graphDesc.end(); ++itg ) {
		if ( graphtype.empty() || itg->first == graphtype ) {
			TypeDescriptorIter it = itg->second.types.find(type);
			if ( it != itg->second.types.end() )
				return it->second.childType;
		}
	}
	return _undef;
}


float  Conventions::minSegmentSize(const string& type, float defval)
{
	std::map <std::string, float>::iterator it = m_minSegSize.find(type);
	if ( it != m_minSegSize.end() ) return it->second;
	return defval;
}

// set mainstream type vector
void Conventions::updMainstreamTypes(const string& graphtype, Parameters* param, bool reset_mode)
{
	std::map<std::string, std::string> configuration ;
	string key;
	if (!param)
		configuration = m_config ;
	else
		configuration = param->getParametersMap("Conventions");

	vector<string> mainstream;
	StringOps(getConfiguration(graphtype+",mainstream", configuration)).split(mainstream, ";,");

	vector<string>::iterator it;

	// absolute minimal size property for graph
	key = graphtype+",min_seg_size";
	if ( configuration.find(key) != configuration.end() )
		m_minSegSize[graphtype] = myatof(configuration[key].c_str());

	m_graphDesc[graphtype].speakerType = "";

	string parent("");
	for ( it = mainstream.begin(); it != mainstream.end(); ++it )
	{
		m_graphDesc[graphtype].types[*it] = *it;
		TypeDescriptor& t = m_graphDesc[graphtype].types[*it];

		t.isMainstream = true;
		t.typeClass = "mainstream";

		// hierarchy alignement property
		t.parentType = parent;
		if ( parent != "" ) {
			m_graphDesc[graphtype].types[parent].childType = *it;
		}
		parent = *it;

		// anchoring property
		const string& an = configuration[*it+",is_anchored"];
		if ( an == "true" ) {
			m_graphDesc[graphtype].segmentationBaseType = *it;
			t.isAnchored = true;
		}

		// minimal size property
		key = *it+",min_size_hint";
		if ( configuration.find(key) != configuration.end() )
			m_minSegSize[*it] = myatof(configuration[key].c_str());


		// overlapping property
		key = *it+",max_overlap";
		t.maxOverlap = atoi(configuration[key].c_str());
		if ( t.maxOverlap > m_graphDesc[graphtype].maxOverlap )
			m_graphDesc[graphtype].maxOverlap = t.maxOverlap;
		if ( m_graphDesc[graphtype].maxOverlap > 0 )
			t.maxOrder = m_graphDesc[graphtype].maxOverlap;

		// eventual subtypes
		key = *it+",subtypes" ;
		if ( configuration.find(key) != configuration.end() ) {
			std::vector<string> submains ;
			std::vector<string>::iterator itsub ;
			StringOps(configuration[key].c_str()).split(submains, ";,");
			for (itsub=submains.begin(); itsub!=submains.end(); itsub++)
				t.submain[*itsub] = getConfiguration(*itsub+",input") ;
		}

		// annotation features
		const string& feats = getConfiguration(*it+",features");
		vector<string> v;
		StringOps(feats).split(v, ";,");
		if ( v.size() > 0 )
		{
			vector<string>::iterator itv;
			std::map<string, string> items;
			StringOps(getConfiguration(*it+",default_features")).getDCSVItems(items);
			for (itv=v.begin(); itv != v.end(); ++itv )
			{
				if ( items.find(*itv) == items.end() )
					items[*itv] = "";
			}
			t.features = items;
			if ( items.find("speaker") != items.end() )
				m_graphDesc[graphtype].speakerType = *it;
		}
	}

	m_graphDesc[graphtype].mainstream = mainstream;
}

#ifdef TODEL

void Conventions::updMainstreamBaseSubtype(const string& graphtype, Parameters* param)
{
	m_graphDesc[graphtype].submain.clear() ;

	string baseType = m_graphDesc[graphtype].mainstream.back() ;

	const string& mainstreamBaseSubtypes = getConfiguration(baseType+",subtypes") ;
	if (mainstreamBaseSubtypes.empty())
		return ;

	std::vector<string> submains ;
	std::vector<string>::iterator itsub ;
	StringOps(mainstreamBaseSubtypes).split(submains, ";,");
	for (itsub=submains.begin(); itsub!=submains.end(); itsub++)
		m_graphDesc[graphtype].submain[*itsub] = getConfiguration(*itsub+",input") ;
}
#endif

//
// set mainstream type vector
void Conventions::updSignalTypes(const string& graphtype, Parameters* param)
{
	std::map<std::string, std::string> configuration ;
	if (!param)
		configuration = m_config ;
	else
		configuration = param->getParametersMap("Conventions");

	vector<string> signalClass;
	StringOps(configuration[graphtype+",signals"]).split(signalClass, ";,");
	m_graphDesc[graphtype].signalClass = signalClass;
	if ( !m_usesVideoSignal )
		m_usesVideoSignal = (find(signalClass.begin(), signalClass.end(),"video") != signalClass.end() );
}

//
// set mainstream type vector
void Conventions::updQualifierTypes(const string& graphtype, Parameters* param)
{
	string buf("");

	std::map<std::string, std::string> configuration ;
	if (!param)
		configuration = m_config ;
	else
		configuration = param->getParametersMap("Conventions");

	// get all qualifiers for graph type
	map<string, string>::iterator itm = configuration.begin();
	Parameters::KeyLookup is_qual(graphtype+",qualifier");
	GraphDescriptor& gDesc = m_graphDesc[graphtype];

	while ( (itm = find_if(itm, configuration.end(), is_qual)) != configuration.end() ) {
//		if ( !buf.empty() ) buf += ";";
//		buf += itm->second;
		string name= itm->first.substr(graphtype.size()+1);
		unsigned long pos = name.find_first_of( "_,");

		if ( pos  != string::npos  ) {
			name = name.substr(pos+1);
			if ( !name.empty() ) {
				vector<string> v;
				StringOps(itm->second).split(v, ";,");
				gDesc.qualifierClass[name] = v;
			}
		}

		vector<string> qualifiers;
		vector<string>::iterator itq;
		StringOps(itm->second).split(qualifiers, ";,");

		for (itq=qualifiers.begin(); itq != qualifiers.end(); ++itq ) {
			gDesc.types[*itq] = *itq;
			TypeDescriptor& t = gDesc.types[*itq];
			t.isMainstream = false;
			t.typeClass = name;
			t.parentType = gDesc.mainstream.back();
			t.canSpanOver = getConfiguration(*itq+",can_span_over");
			gDesc.spannables.insert(t.canSpanOver);
		}
		++itm;
	}
}


//
// compare mainstream level for given types
// return  value is positive if higher level; else negative
int Conventions::compareMainstreamLevel(const string& type, const string& ref, const string& graphtype)
{
	GraphDescriptorIter itg = m_graphDesc.find(graphtype);
	if ( itg != m_graphDesc.end() ) {
		const vector<string>& mainstream = itg->second.mainstream;
		int i, j;
		for ( i=0; i < mainstream.size() && mainstream[i] != type; ++i );
		for ( j=0; j < mainstream.size() && mainstream[j] != ref; ++j );
		return ( j - i ) ;
	}
	return 0;
}


//
// compare mainstream level for given types
// return  value is positive if higher level; else negative
bool Conventions::isHigherPrecedence(const string& type, const string& ref, const string& graphtype)
{
//	if ( ! isMainstreamType(type, graphtype ) ) return false;
	return (compareMainstreamLevel(type, ref, graphtype) > 0);
}

string Conventions::getLocalizedLabel(const string& type)
{
	std::map<string, string>::iterator it;
	it = m_labels.find(type);
	if ( it == m_labels.end() )
		return type;
	return it->second;
}

const string& Conventions::getAlignmentType(const string& type)
{
	return getAlignmentType(type, getGraphtypeFromType(type));
}
const string& Conventions::getAlignmentType(const string& type, const string& graphtype)
{
	GraphDescriptorIter itg = m_graphDesc.find(graphtype);
	if ( itg != m_graphDesc.end() ) {
		TypeDescriptorIter it = itg->second.types.find(type);
		if ( it != itg->second.types.end() )
			return it->second.childType;
	}
	return _undef;
}

/**
*  check if overlapping allowed for given annotation type
* @param type annotation type
* @return true if overlapping allowed, else false
*/
bool Conventions::canOverlap(const string& type, const string& graphtype)
{
	return (maxOverlap(type, graphtype) > 0 );
}

/**
*  max overlapping layers allowed for given annotation type
* @param type annotation type
* @return max overlapping layers
*/
int Conventions::maxOverlap(const string& type, const string& graphtype)
{
	GraphDescriptorIter itg;
	for (itg = m_graphDesc.begin(); itg != m_graphDesc.end(); ++itg ) {
		if ( graphtype.empty() || itg->first == graphtype ) {
			TypeDescriptorIter it = itg->second.types.find(type);
			if ( it != itg->second.types.end() )
				return it->second.maxOverlap;
		}
	}
	return 0;
}

/**
*  max possible order for given annotation type
* @param type annotation type
* @return max overlapping layers
*/
int Conventions::maxOrder(const string& type, const string& graphtype)
{
	GraphDescriptorIter itg;
	for (itg = m_graphDesc.begin(); itg != m_graphDesc.end(); ++itg ) {
		if ( graphtype.empty() || itg->first == graphtype ) {
			TypeDescriptorIter it = itg->second.types.find(type);
			if ( it != itg->second.types.end() )
				return it->second.maxOrder;
		}
	}
	return 0;
}

/**
*  check if span over signal-anchored  node allowed for given annotation type
* @param type annotation type
* @return true if span allowed, else false
*/
bool Conventions::canSpanOverType(const string& qtype, const string& mtype)
{
	bool ok=false;
	const string& graphtype = getGraphtypeFromType(mtype);
	if ( ! graphtype.empty() ) {
		GraphDescriptorIter itg = m_graphDesc.find(graphtype);
		TypeDescriptorIter it = itg->second.types.find(qtype);
		if ( it != itg->second.types.end() ) {
			const string& spanned = it->second.canSpanOver;
			if ( !spanned.empty() ) {
				if ( mtype == spanned ) ok = true;
				else ok=(isHigherPrecedence(spanned, mtype));
			}
		}
	}
	return ok;
}


bool Conventions::isAnchoredType(const std::string& type)
{
	return isAnchoredType(type, getGraphtypeFromType(type));
}

bool Conventions::isAnchoredType(const std::string& type, const string& graphtype)
{
	GraphDescriptorIter itg;
	for (itg = m_graphDesc.begin(); itg != m_graphDesc.end(); ++itg ) {
		if ( graphtype.empty() || itg->first == graphtype ) {
			TypeDescriptorIter it = itg->second.types.find(type);
			if ( it != itg->second.types.end() )
				return ( it->second.isAnchored );
		}
	}
	return false;
}


// IS THIS STILL USED ???
bool Conventions::isInstantaneous(const string& qtype)
{
	return (  getConfiguration(qtype+",is_instantaneous") == "true" );
}

#ifdef TODEL
void Conventions::appendMainstreamType(string type, string graphtype)
{

	string tmp = getConfiguration(graphtype+",mainstreams");
	if (tmp.find(WORD_TYPE) == string::npos)
	{
		tmp += (string(";") + WORD_TYPE);
		setConfiguration("transcription_graph,mainstreams", tmp);
		setConfiguration("segment,alignment", WORD_TYPE);
		updMainstreamTypes("transcription_graph", NULL, false);
	}
}
#endif

void Conventions::getAnnotationItems(vector<string>& items, const string& type, const string& itemtype)
{
	items.clear();
	string key=type+","+itemtype;
	StringOps(getConfiguration(key)).split(items, ";,");
}

bool Conventions::appliesToSignalClass(const string& graphtype, const string& signaltype)
{
	std::vector<string> signalClass = m_graphDesc[graphtype].signalClass ;
	std::vector<string>::iterator it ;
	bool found = false ;
	for (it=signalClass.begin(); it!=signalClass.end() && !found; it++) {
		if (*it==signaltype)
			found = true ;
	}
	return found ;
}

bool Conventions::checkValue(const string& id, const string& parameter, const string& value)
{
	string key = id + "," + parameter ;
	std::vector<std::string> values ;
	StringOps(getConfiguration(key)).split(values, ";,");
	std::vector<std::string>::iterator it ;
	bool found = false ;
	for (it=values.begin(); it!=values.end()&&!found; it++) {
		if (*it==value)
			found = true ;
	}
	return found ;
}

string Conventions::normalizeSubmain(const string& subtype, const string& graphtype)
{
	TypeDescriptorIter it;
	GraphDescriptor& gDesc = m_graphDesc[graphtype];
	for ( it = gDesc.types.begin(); it != gDesc.types.end(); ++it )
	{
		if ( it->first == subtype )
			return subtype;
		if ( it->second.isMainstream )
			if ( it->second.submain.find(subtype) != it->second.submain.end())
				return it->first;
	}
	return "";
}

bool Conventions::submainHasText(const string& type, const string& subtype, const string& graphtype)
{
	GraphDescriptorIter itg = m_graphDesc.find(graphtype);
	// Search for graphe
	if ( itg != m_graphDesc.end() ) 
	{
		TypeDescriptorIter it ;
		// Search for matching mainstream type
		it = itg->second.types.find(type) ;
		if ( it != itg->second.types.end() && it->second.isMainstream ) 
		{
			std::map<string, string>::const_iterator it2 ;
			// Search for matching subtype 
			it2 = it->second.submain.find(subtype);
			if ( it2 != it->second.submain.end() )
				return it2->second.empty();
		}
		// Otherwise, let's say no
		return false ;
	}
	return false;
}

/*
 * UNUSED FOR THE MOMENT, SEE FOLLOWING METHOD COMMENTS
 */
//std::string Conventions::getGraphtypeFromTypes(const set<string>& types)
//{
//	std::map<string,int> matches ;
//
//	std::vector<string> graphtypes = getGraphTypes() ;
//	std::vector<string>::iterator it ;
//	for ( it=graphtypes.begin(); it!=graphtypes.end(); it++ )
//	{
//		matches[*it] = 0 ;
//		std::vector<string> mainstreams = getMainstreamTypes(*it) ;
//		set<string>::const_iterator it_types ;
//		for (it_types=types.begin(); it_types!=types.end(); it_types++)
//		{
//			if (is_in_svect(mainstreams, *it_types))
//				matches[*it] = matches[*it] + 1 ;
//		}
//	}
//
//	std::map<string,int>::iterator it_matches ;
//	int max = 0 ;
//	string candidate = "" ;
//	for (it_matches=matches.begin(); it_matches!=matches.end(); it_matches++)
//	{
//		if (it_matches->second > max) {
//			max = it_matches->second ;
//			candidate = it_matches->first ;
//		}
//	}
//
//	return candidate ;
//}

/*
 * Checks with mainstream.
 * <!>
 * CONSIDER THAT A TYPE IS UNIQUE SO ONLY BELONGS TO 1 GRAPH
 * IF THIS ASSERTION CHANGES, CHANGE WITH THE COMMENTED METHOD ABOVE
 * <!>
 */
const string& Conventions::getGraphtypeFromTypes(const set<string>& types)
{
	GraphDescriptorIter itg ;
	for (itg = m_graphDesc.begin(); itg != m_graphDesc.end(); ++itg ) {
		set<string>::const_iterator it_types ;
		for (it_types=types.begin(); it_types!=types.end(); it_types++)
		{
			if (find(itg->second.mainstream.begin(), itg->second.mainstream.end(), *it_types) != itg->second.mainstream.end())
				return itg->first ;
		}
	}

	return _undef ;
}

const string& Conventions::getGraphtypeFromType(const string& type)
{
	GraphDescriptorIter itg;
	TypeDescriptorIter it;
	for ( itg=m_graphDesc.begin(); itg != m_graphDesc.end(); ++itg ) {
		it = itg->second.types.find(type);
		if ( it != itg->second.types.end() )
			return itg->first;
	}
	return _undef ;
}

/**
 * @param feature feature name
 * @return type which contains feature (if many types, returns the first one found
 */
const string& Conventions::getTypeFromFeature(const std::string& feature, const string& graphtype)
{
	GraphDescriptorIter itg;
	for ( itg=m_graphDesc.begin(); itg != m_graphDesc.end(); ++itg ) {
		if ( graphtype.empty() || itg->first == graphtype ) {
			TypeDescriptorIter it;
			for ( it = itg->second.types.begin(); it != itg->second.types.end(); ++it ) {
				if ( it->second.features.find(feature) != it->second.features.end() ) {
					return it->first;
				}
			}
		}
	}
	return _undef;
}


/**
 * getSpeakerType -> returns annotation type to which "speaker" feature is attached
 */
const string& Conventions::getSpeakerType(const string& graphtype)
{
	GraphDescriptorIter itg = m_graphDesc.find(graphtype);
	if ( itg != m_graphDesc.end() )
		return itg->second.speakerType;
	return _undef;
}

/********************* space handling accessors *******************************/

bool Conventions::needSpaceBorders(char c)
{
	std::set<gunichar>::iterator it = m_borderedChars.find(c) ;
	return (it!=m_borderedChars.end());
}

bool Conventions::spaceBorderingForced()
{
	return ( !m_borderedChars.empty() );
}

bool Conventions::automaticSpaceHandling()
{
	return m_spaceHandling ;
}




/**
 * @return true if conventions set for fast annotation mode
 */
bool Conventions::fastAnnotationMode()
{
	if ( hasGraphType("transcription_graph") )
		return (m_config["transcription_graph,type"] == "true");
}

// DEBUG
//void Conventions::printGraphDescriptors()
//{
//	GraphDescriptorIter it ;
//	for (it=m_graphDesc.begin(); it!=m_graphDesc.end(); it++) {
//		TRACE_D << ">>>> \n" << GraphDescriptor::toString(it->second, it->first) << std::endl ;
//	}
//}

} /*  namespace tag */
