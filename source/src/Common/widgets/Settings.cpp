/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Settings.h"
#include "Common/util/Utils.h"
#include "Common/util/FileHelper.h"
#include "Common/Explorer_utils.h"

namespace tag {

//------------------------------------------------------------------------------
//									 STATIC BUSINESS
//------------------------------------------------------------------------------

// the Languages instance
Settings* Settings::m_instance = NULL;

Settings* Settings::getInstance()
{
	return m_instance;
}

void Settings::configure(Parameters* p_config)
{
	if ( m_instance != NULL ) delete m_instance;
	m_instance = new Settings(p_config);
}

void Settings::kill()
{
	if ( m_instance != NULL ) {
		delete m_instance;
		m_instance = NULL ;
	}
}


//------------------------------------------------------------------------------
//									INSTANCE BUSINESS
//------------------------------------------------------------------------------


Settings::Settings(Parameters* p_config)
{
	loaded = false;
	config = p_config ;
	load() ;
}

Settings::~Settings()
{
}

void Settings::load()
{
	Glib::ustring file = get_SETTINGS_path() ;
	if (Glib::file_test(file, Glib::FILE_TEST_EXISTS) && file.compare("none")!=0)
	{
		std::vector<Glib::ustring> vect ;
		Explorer_utils::read_lines(file, &vect) ;
		for (int i=0; i< vect.size(); i++) {
			std::vector<Glib::ustring> v ;
			mini_parser('|', vect[i], &v) ;
			datas[v[0]] = v ;
		}
	}
	else {
		create() ;
 	}
	loaded = true ;
}

void Settings::create()
{
	create_setting(SETTINGS_GENERAL_NAME) ;
	create_setting(SETTINGS_SEARCH_NAME) ;
	create_setting(SETTINGS_TSEARCH_NAME) ;
	create_setting(SETTINGS_CLIPBOARD_NAME) ;
	create_setting(SETTINGS_PREFERENCES_NAME) ;
	create_setting(SETTINGS_DICOG_NAME) ;
	create_setting(SETTINGS_DICOF_NAME) ;
	create_setting(SETTINGS_FPROPERTIES_NAME) ;
	create_setting(SETTINGS_VIDEO_BROWSER) ;
	create_setting(SETTINGS_VIDEO_PLAYER) ;
	create_setting(SETTINGS_STATUSREPORT_DLG) ;
	create_setting(SETTINGS_TOOL_LAUNCHER) ;
	save() ;
}


void Settings::create_setting(Glib::ustring name)
{
	Glib::ustring s = number_to_string(-1) ;
	std::vector<Glib::ustring> v1 ;
	v1.insert(v1.end(), name) ;
	v1.insert(v1.end(), s) ;
	v1.insert(v1.end(), s) ;
	v1.insert(v1.end(), s) ;
	v1.insert(v1.end(), s) ;
	v1.insert(v1.end(), s) ;
	datas[name] = v1 ;
}


void Settings::save()
{
	std::map<Glib::ustring, std::vector<Glib::ustring> >::iterator it = datas.begin() ;
	std::vector<Glib::ustring> save ;
	while(it!=datas.end()) {
		Glib::ustring line = "" ;
		for (int i=0; i<(it->second).size(); i++) {
			if (i!=(it->second).size()-1)
				line =  line + (it->second)[i] + "|"   ;
			else
				line =  line + (it->second)[i] ;
		}
		it++ ;
		save.insert(save.end(), line) ;
	}
	Glib::ustring file = get_SETTINGS_path() ;
	Explorer_utils::write_lines(file, save, "w") ;
}


void Settings::get_datas(Glib::ustring code_window, int& w, int& h, int& posx, int& posy, int& paned)
{
	if (loaded && exist(code_window)) {
		Glib::ustring s = datas[code_window][SETTINGS_W] ;
		w = string_to_number<int>(s) ;
		s = datas[code_window][SETTINGS_H] ;
		h = string_to_number<int>(s) ;
		s = datas[code_window][SETTINGS_WPOS] ;
		posx = string_to_number<int>(s) ;
		s = datas[code_window][SETTINGS_HPOS] ;
		posy = string_to_number<int>(s) ;
		s = datas[code_window][SETTINGS_PANED] ;
		paned = string_to_number<int>(s) ;
	}
	else {
		w = -1 ;
		h = -1 ;
		posx = -1 ;
		posy = -1 ;
		paned = -1 ;
	}
}

bool Settings::set_datas(Glib::ustring code_window, int w, int h, int posx, int posy, int paned)
{
	if (!exist(code_window) || !loaded)
		return false ;

	Glib::ustring s = number_to_string(w) ;
	datas[code_window][SETTINGS_W] = s ;
	s = number_to_string(h) ;
	datas[code_window][SETTINGS_H] = s ;
	s = number_to_string(posx) ;
	datas[code_window][SETTINGS_WPOS] = s ;
	s = number_to_string(posy) ;
	datas[code_window][SETTINGS_HPOS] = s ;
	if (paned!=-1) {
		s = number_to_string(paned) ;
		datas[code_window][SETTINGS_PANED] = s ;
	}

	return true ;
}

bool Settings::exist(Glib::ustring code_window)
{
	std::map<Glib::ustring, std::vector<Glib::ustring> >::iterator it = datas.begin() ;
	if ( datas.find(code_window) == datas.end() )
		return false ;
	else
		return true ;
}

Glib::ustring Settings::get_SETTINGS_path()
{
  	Glib::ustring value ;
  	Glib::ustring home_folder = config->getParameterValue("General", "start,homeFolder") ;
 	Glib::ustring home_dir = Glib::get_home_dir() ;
 	Glib::ustring home_folder_path = FileHelper::build_path(home_dir,home_folder) ;
	Glib::ustring settings = config->getParameterValue("GUI", "data,settings") ;
 	if (settings!="" && settings!=" ")
 		value = FileHelper::build_path(home_folder_path, settings) ;
 	else
 		value = "" ;
 	return value;
}

} //namespace

