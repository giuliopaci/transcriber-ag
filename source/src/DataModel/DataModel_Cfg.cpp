/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* @file DataModel_cfg.cpp
* configuration parameters for DataModel
*/

#include <ag/AGAPI.h>
#include <sstream>
#include <glib.h>
#include <glib/gstdio.h>

#include "DataModel/DataModel.h"

#include "Common/util/StringOps.h"
#include "Common/FileInfo.h"
#include "Common/Parameters.h"
#include "Common/iso639.h"
#include "Common/util/Utils.h"
#include "Common/util/Log.h"

#include "Common/Formats.h"

const utils::Encoding* utf8 = NULL;

#define DTD_VERSION "3.0"

#define QUOTEME(x) #x

#define PREFIX ""

using namespace std;

namespace tag {

/**
 * set file load/store options for AGLIB
 * @param format file format
 * @param savemode true if saving file, else false
 * @note options include dtd name, encoding, system
 */
void DataModel::setAGOptions(string format, bool savemode)
{
	if ( format == "TransAG" )
	{
		string configDir = m_conventions.getDirectory() ;

		string tmp = configDir ;
		configDir = Glib::path_get_dirname(tmp) ;

		// -- Use default TransAG-2.0 for dtd name.
		// -- Anyway, in most case DTD name will be mentioned in the xml file,
		// and the TransAG plugin will try to use the specified value before
		// using the default one

		m_localDTD = "" ;

		//> -- Try in configuration directory
		if ( Glib::file_test(configDir, Glib::FILE_TEST_EXISTS) )
		{
			m_agOptions["system"] = "TransAG-3.0.dtd";
			m_localDTD = Glib::build_filename(configDir,"TransAG-3.0.dtd") ;

			if ( !Glib::file_test(m_localDTD, Glib::FILE_TEST_EXISTS) )
			{
				m_agOptions["system"] = "TransAG-2.0.dtd";
				m_localDTD = Glib::build_filename(configDir,"TransAG-2.0.dtd") ;
			}

			if ( !Glib::file_test(m_localDTD, Glib::FILE_TEST_EXISTS) )
			{
				m_agOptions["system"] = "TransAG-1.0.dtd";
				m_localDTD = Glib::build_filename(configDir,"TransAG-1.0.dtd") ;
			}

		}

		//> -- Configuration directory does not exit, try in installation default directory
		if ( !Glib::file_test(m_localDTD, Glib::FILE_TEST_EXISTS) )
		{
			m_agOptions["system"] = "TransAG-3.0.dtd";
			m_localDTD = PREFIX "/etc/TransAG/TransAG-3.0.dtd";

			if ( !Glib::file_test(m_localDTD, Glib::FILE_TEST_EXISTS) )
			{
				m_agOptions["system"] = "TransAG-2.0.dtd";
				m_localDTD = PREFIX "/etc/TransAG/TransAG-2.0.dtd";
			}

			if ( !Glib::file_test(m_localDTD, Glib::FILE_TEST_EXISTS) )
			{
				m_agOptions["system"] = "TransAG-1.0.dtd";
				m_localDTD = PREFIX "/etc/TransAG/TransAG-1.0.dtd";
			}
		}

		m_agOptions["DTDvalidation"] = "true";
		m_agOptions["encoding"] = "utf-8";
	}
	else
	{
		m_agOptions["encoding"] = "";
		m_agOptions["DTDvalidation"] = "false";
	}

	if ( savemode )
		m_agOptions["dtd"] = m_agOptions["system"] ;
	else
		m_agOptions["dtd"] = m_localDTD;
}


/**
 *  set a specific option for file load/store of a given file format
 * @param key option name
 * @param value option value
 *
 * @note this allows to pass some config parameters to file format plugin.
 */
void DataModel::addAGOption(string key, string value, bool replaceIfExists)
{
	if ( replaceIfExists || m_agOptions.find(key) == m_agOptions.end() )
		m_agOptions[key] = value;
}


/* configure data model */
void DataModel::init()
{
	_novalue = "";
	_currentDTDVersion = DTD_VERSION;

	if ( utf8 == NULL ) {
		utf8 = utils::Encoding::getEncoding("utf-8");
		setlocale(LC_NUMERIC, "C");
	}

	closing = false ;
	m_updated = false;
	m_path = "";
	m_agsetId = "";
	m_tmpId = "";
	m_savId = "";
	m_timelineId = "";
	m_signalLength = UNDEFINED_LENGTH;
	m_lastVersion = "";
	m_defaultCorpusName = "";
	m_keepAG=false;
	m_inhibateSignals=false;
	m_qualifierMapping = NULL ;
	m_anchorTrackHint = 0;
	m_import_format = "" ;
	m_graphType = "";
	m_inhibateChecking = false;

	m_checker = NULL ;

	// speaker dictionary connect
	if (!m_speakerUpdatedConnection.connected())
		m_speakerUpdatedConnection = m_speakersDict.signalSpeakerUpdated().connect(sigc::mem_fun(*this, &DataModel::speakerUpdated)) ;

	setAGOptions();
}


/* initialize program execution environment (plugins path) */
void DataModel::initEnviron(string curpath)
{
	if ( ! _initEnvironDone )
	{
 		TRACE << "Initializing environment" << std::endl ;

		string configuration_path ;
		string lib_path("");
		string ld_library_path("");
		bool ld_ok = false;

		const gchar* varenv ;
		if ( (varenv = g_getenv("LD_LIBRARY_PATH")) != NULL ) {

			ld_library_path = varenv;
			string str_ag = FileInfo("lib").join("ag");
			// check if lib/ag already set in lib path
			unsigned long pos = ld_library_path.find(str_ag, 0);
			while ( ! ld_ok && pos != string::npos) {
				unsigned long pos2 = pos + str_ag.length();
				if ( pos2 == ld_library_path.length() || ld_library_path[pos2] == ':' ) {
					ld_ok = true;
					pos = ld_library_path.rfind(':', pos);
					if ( pos == string::npos ) pos =0;
					else ++pos;
					if ( curpath == "" ) {
						curpath = FileInfo(ld_library_path.substr(pos, pos2-pos)).dirname(2);
					}
				}
				pos = ld_library_path.find(str_ag, pos);
			}
		}

		string pdir = curpath;
		if ( pdir.empty() )
			pdir = PREFIX;
		else {
			if ( FileInfo(curpath).isFile() ) pdir = FileInfo(pdir).dirname();

			if ( !g_path_is_absolute (pdir.c_str()) ) {
				gchar* curdir = g_get_current_dir();
				g_chdir(pdir.c_str());
				gchar* newdir = g_get_current_dir();
				pdir = newdir;
				g_chdir(curdir);
				g_free(curdir);
				g_free(newdir);
			}
		}

		if ( !ld_ok ) {	// then set LD_LIBRARY_PATH relative to current path
			string root = FileInfo(pdir).rootname();
			if ( root == "bin"  || root == "etc" || root == "lib" ) // go up one stage
				pdir = FileInfo(pdir).dirname();

			string dir = FileInfo(pdir).join("lib");
			if ( FileInfo(dir).exists() ) {
				lib_path = dir; lib_path += ":";
				dir = FileInfo(dir).join("ag");
				if ( FileInfo(dir).exists() )
					lib_path += dir;
				else lib_path = "";
			}

			if ( lib_path.empty() && FileInfo(PREFIX "/lib/ag").exists() ) {
				lib_path = PREFIX "/lib:" PREFIX "/lib/ag";
			}
			if ( ! lib_path.empty() ) {
				if ( !ld_library_path.empty() ) ld_library_path += ":";
				ld_library_path += lib_path;
				g_setenv("LD_LIBRARY_PATH", ld_library_path.c_str(), 0);
			}
		}

		/** locate etc dir from current path */
		bool etc_ok = false;
		while ( ! etc_ok )
		{
			string dir = FileInfo(pdir).join("etc");
			if ( FileInfo(dir).exists() )
			{
				etc_ok = true;
				string dir2 = FileInfo(dir).join("TransAG");
				if ( FileInfo(dir2).exists() ) {
					dir = dir2;
					configuration_path = dir ;
				}
				dir2 = FileInfo(dir).join("conventions");
				if ( FileInfo(dir2).exists() )
					dir = dir2;
				Conventions::setConfigDir(dir);
			}
			dir = FileInfo(pdir).dirname();
			if ( dir.empty() || dir == pdir )
				break;
			pdir = dir;
		}

		if ( ! etc_ok )
		{
			// check if exists in in /usr/local/etc
			string dir = PREFIX "/etc/TransAG";
			if ( FileInfo(dir).exists() ) {
				etc_ok = true;
				configuration_path = dir ;
				string dir2 = FileInfo(dir).join("conventions");
				if ( FileInfo(dir2).exists() ) dir = dir2;
				Conventions::setConfigDir(dir);
			}
		}
 		_initEnvironDone = true;

 		//> Load format if it hasn't been done
 		TRACE << "configuration_path= " << configuration_path << std::endl ;
 		Formats::configure(configuration_path) ;
	}
}


/**
 * sets applicable annotation conventions for current DataModel instance
 * @param convention	convention name
 * @param lang	language for annotation labels display
 * @param fullmode if true (default), will load all files associated to given convention, including annotations label formats, topics list, ..
 * if false, load only convention specification file (faster).
 */
void DataModel::setConventions(string convention, string lang, bool fullmode)
{
	m_conventions.configure(convention, lang, fullmode);
	if ( ! m_conventions.name().empty() && !m_agsetId.empty())
	{
		// set convention id
		setAGSetProperty("convention_id", m_conventions.name());
		// set convention version
		string convention_version = m_conventions.getConfiguration("reference,version") ;
		setAGSetProperty("convention_version", convention_version);
		// set convention doc
		string convention_doc = m_conventions.getConfiguration("reference,doc,"+ string(ISO639::getLocale())) ;
		setAGSetProperty("convention_doc", convention_doc);
	}
	else
		Log::out() << "setting Conventions : name [" << !m_conventions.name().empty() << "] - agsetId [" << !m_agsetId.empty() << "]" << std::endl ;
}

void DataModel::setSingleSignal(bool single)
{
	string value ;
	if (single)
		value = "true" ;
	else
		value = "false" ;

	if (single)
		setAGSetProperty("single_signal", value) ;
	else
		unsetAGSetProperty("single_signal") ;

	signalCfg.setSingleSignal(single) ;
}

std::vector<string> DataModel::getAnnotationTypes(bool checkInModel)
{
	std::vector<string> types ;
	std::vector<string> tmp ;

	std::vector<string>::iterator it_graphes ;
	std::vector<std::string>::iterator it_tmp ;

	bool existInModel = true ;
	bool condition = true ;

	std::vector<string> graphes ;
	if (m_import_format=="SGML")
	{
		graphes.push_back("alignmentREF_graph") ;
		graphes.push_back("alignmentHYP_graph") ;
	}
	graphes.push_back("transcription_graph") ;
	graphes.push_back("background_graph") ;

/*
 * 	Would be sexier to determine graphes by interating on agIds map
 *  Only one problem remaining, find a way to indicate in argument
 *  the order of graph treatment, or adjust upper view to re-order
 *  the annotation type
 *
 *	for (it_graphes=m_agIds.begin(); it_graphes!=m_agIds.end(); it_graphes++)
 */

	for (it_graphes=graphes.begin(); it_graphes!=graphes.end(); it_graphes++)
	{
		tmp = getMainstreamTypes(*it_graphes) ;

		for (it_tmp=tmp.begin(); it_tmp!=tmp.end(); it_tmp++)
		{
			if ( checkInModel && m_agIds.find(*it_tmp) != m_agIds.end() )
				existInModel = hasElementsWithType(*it_tmp, m_agIds[*it_tmp]) ;

			if ( (find(types.begin(), types.end(), *it_tmp)==types.end()) && existInModel )
				types.push_back(*it_tmp) ;
		}
	}
	return types ;
}

} /*  namespace tag */
