/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef SETTINGS_H_
#define SETTINGS_H_


#define SETTINGS_GENERAL_NAME	 	"general"
#define SETTINGS_SEARCH_NAME	  	"search"
#define SETTINGS_TSEARCH_NAME	  	"tag_search"
#define SETTINGS_DICOG_NAME		   	"dicog"
#define SETTINGS_DICOF_NAME		   	"dicof"
#define SETTINGS_CLIPBOARD_NAME		"clip"
#define SETTINGS_FPROPERTIES_NAME	"fproperties"
#define SETTINGS_PREFERENCES_NAME	"preferences"
#define SETTINGS_VIDEO_PLAYER		"video_player"
#define SETTINGS_VIDEO_BROWSER		"video_browser"
#define SETTINGS_STATUSREPORT_DLG 	"report_dialog"
#define SETTINGS_TOOL_LAUNCHER 		"tool_launcher"

#define SETTINGS_W	 				1
#define SETTINGS_H	  				2
#define SETTINGS_WPOS				3
#define SETTINGS_HPOS				4
#define SETTINGS_PANED				5

#include <gtkmm.h>
#include <glibmm.h>
#include "Common/Parameters.h"

namespace tag {

/**
 * @class 	Settings
 * @ingroup	GUI
 *	(Very) basic settings manager for widget geometry
 *	Available on some widgets only at the moment
 */

class Settings
{
	public:

		/**
		 * Returns the single instance of the settings object, or creates it
		 * if it hasn't been created yet.
		 * @return		Pointer on the Setting singleton
		 */
		static Settings* getInstance() ;

		/**
		 * Creates and configure the setting singleton.
		 * @param p_config		Pointer on the application parameter object
		 */
		static void configure(Parameters* p_config) ;

		/**
		 * Kills the setting instance
		 */
		static void kill() ;

		/**
		 * Load geometry settings from the file specified in configuration
		 */
		void load() ;

		/**
		 * Save settings into the file specified in configuration
		 */
		void save() ;

		/**
		 * Get geometry data of the widget associated to the given code_window
		 * @param code_window	Code identifying the widget for which the data are required\n
		 * 						SETTINGS_GENERAL_NAME : general window\n
		 * 						SETTINGS_SEARCH_NAME : search dialog\n
		 * 						SETTINGS_CLIPBOARD_NAME : clipboard\n
		 * 						SETTINGS_DICOG_NAME : global speaker dictionary\n
		 * 						SETTINGS_DICOF_NAME : local speaker dictionary\n
		 * 						SETTINGS_FPROPERTIES_NAME : preferences dialog
		 * @param[out] w		width
		 * @param[out] h		height
		 * @param[out] posx		x position
		 * @param[out] posy		y position
		 * @param[out] paned	panel separation position
		 */
		void get_datas(Glib::ustring code_window, int& w, int& h, int& posx, int& posy, int& paned) ;

		/**
		 * Set geometry data of the widget associated to the given code_window
		 * @param code_window	Code identifying the widget for which the data are required\n
		 * 						SETTINGS_GENERAL_NAME : general window\n
		 * 						SETTINGS_SEARCH_NAME : search dialog\n
		 * 						SETTINGS_CLIPBOARD_NAME : clipboard\n
		 * 						SETTINGS_DICOG_NAME : global speaker dictionary\n
		 * 						SETTINGS_DICOF_NAME : local speaker dictionary\n
		 * 						SETTINGS_FPROPERTIES_NAME : preferences dialog
		 * @param w				width
		 * @param h				height
		 * @param posx			x position
		 * @param posy			y position
		 * @param paned			panel separation position
		 */
		bool set_datas(Glib::ustring code_window, int w, int h, int posx, int posy, int paned) ;

	private:
		static Settings* m_instance;

		/**
		 * @param config	Pointer on the configuration instance
		 */
		Settings(Parameters* config) ;
		virtual ~Settings();

		std::map< Glib::ustring, std::vector<Glib::ustring> > datas ;
		Parameters* config ;
		bool loaded ;

		bool exist(Glib::ustring code_window) ;

		void create() ;

		/* When adding a new setting entry, use this method with the string id */
		void create_setting(Glib::ustring name) ;

		Glib::ustring get_SETTINGS_path() ;
};

} //namespace

#endif /*SETTINGS_H_*/
