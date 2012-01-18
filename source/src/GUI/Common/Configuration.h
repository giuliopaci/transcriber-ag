/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <glibmm.h>
#include "Common/Parameters.h"

#define TAG_CONFIG_NULL_VALUE "tag-config-null"

/**
 * @def 	TAG_PREFERENCES_PARAM_AUTOSAVE
 * @brief	Dynamic value code for autosave interval
 */
#define TAG_PREFERENCES_PARAM_AUTOSAVE						1

/**
 * @def 	TAG_PREFERENCES_PARAM_INACTIVITY
 * @brief	Dynamic value code for inactivity time
 */
#define TAG_PREFERENCES_PARAM_INACTIVITY					2

/**
 * @def 	TAG_PREFERENCES_PARAM_AUTOSETLANGUAGE
 * @brief	Dynamic value code for input language auto setting
 */
#define TAG_PREFERENCES_PARAM_AUTOSETLANGUAGE 				3

/**
 * @def 	TAG_PREFERENCES_PARAM_TIMESCALE
 * @brief	Dynamic value code for time scale visibility
 */
#define TAG_PREFERENCES_PARAM_TIMESCALE						4

/**
 * @def 	TAG_PREFERENCES_PARAM_STOPONCLICK
 * @brief	Dynamic value code for stop on click audio property
 */
#define TAG_PREFERENCES_PARAM_STOPONCLICK					5


/**
 * @deprecated	Not used anymore
 */
#define TAG_PREFERENCES_PARAM_FTPMACHINE					8

/**
 * @def 	TAG_PREFERENCES_PARAM_TOOLBAR
 * @brief	Dynamic value code for search toolbar property
 */
#define TAG_PREFERENCES_PARAM_TOOLBAR						9

/* SPELL */
///**
// * @def 	TAG_PREFERENCES_PARAM_SPELLER_PATH
// * @brief	Dynamic value code for speller path
// */
//#define TAG_PREFERENCES_PARAM_SPELLER_PATH					10
//
///**
// * @def 	TAG_PREFERENCES_PARAM_SPELLER_ALLOW
// * @brief	Dynamic value code for speller property
// */
//#define TAG_PREFERENCES_PARAM_SPELLER_ALLOW					11
//
///**
// * @def 	TAG_PREFERENCES_PARAM_SPELLER_IGNORE
// * @brief	Dynamic value code for speller ignore property
// */
//#define TAG_PREFERENCES_PARAM_SPELLER_IGNORE				12

/**
 * @def 	TAG_PREFERENCES_PARAM_GENERAL_TOOLBARSHOW
 * @brief	Dynamic value code for toolbar visibility
 */
#define TAG_PREFERENCES_PARAM_GENERAL_TOOLBARSHOW			13

/**
 * @def 	TAG_PREFERENCES_PARAM_GENERAL_STATUSBARSHOW
 * @brief	Dynamic value code for statusbar visibility
 */
#define TAG_PREFERENCES_PARAM_GENERAL_STATUSBARSHOW			14

/**
 * @def 	TAG_PREFERENCES_PARAM_EDITOR_ENTITYBG
 * @brief	Dynamic value code for nammed entity background color
 */
#define TAG_PREFERENCES_PARAM_EDITOR_ENTITYBG				15

/**
 * @def 	TAG_PREFERENCES_PARAM_EDITOR_SUPPRESSSEGMENT
 * @brief	Dynamic value code for segment suppression property
 */
#define TAG_PREFERENCES_PARAM_EDITOR_SUPPRESSSEGMENT		16

/**
 * @def 	TAG_PREFERENCES_PARAM_EDITOR_TEXTFONT
 * @brief	Dynamic value code for editor text font
 */
#define TAG_PREFERENCES_PARAM_EDITOR_TEXTFONT				17

/**
 * @def 	TAG_PREFERENCES_PARAM_EDITOR_LABELFONT
 * @brief	Dynamic value code for editor label font
 */
#define TAG_PREFERENCES_PARAM_EDITOR_LABELFONT				18

/**
 * @def 	TAG_PREFERENCES_PARAM_EDITOR_DEFAULTBROWSER
 * @brief	Dynamic value code for default web browser
 */
#define TAG_PREFERENCES_PARAM_EDITOR_DEFAULTBROWSER			19

/**
 * @def 	TAG_PREFERENCES_PARAM_AUDIO_RESOLUTIONZOOM
 * @brief	Dynamic value code for zoom resoluion
 */
#define TAG_PREFERENCES_PARAM_AUDIO_RESOLUTIONZOOM			20

/**
 * @def 	TAG_PREFERENCES_PARAM_COLORS_EDITOR
 * @brief	Dynamic value code for editor colors
 */
#define TAG_PREFERENCES_PARAM_COLORS_EDITOR					21

/**
 * @def 	TAG_PREFERENCES_PARAM_COLORS_AUDIO
 * @brief	Dynamic value code for audio colors
 */
#define TAG_PREFERENCES_PARAM_COLORS_AUDIO					22

/**
 * @def 	TAG_PREFERENCES_PARAM_EDITOR_ALLOWBROWSEONTAGS
 * @brief	Dynamic value code for tag browsing property
 */
#define TAG_PREFERENCES_PARAM_EDITOR_ALLOWBROWSEONTAGS		23

/**
 * @def 	TAG_PREFERENCES_PARAM_EDITOR_TOOLTIP
 * @brief	Dynamic value code for editor tooltip visibility
 */
#define TAG_PREFERENCES_PARAM_EDITOR_TOOLTIP				24

/**
 * @def 	TAG_PREFERENCES_PARAM_EDITOR_HIGHLIGHT
 * @brief	Dynamic value code for editor highlight property
 */
#define TAG_PREFERENCES_PARAM_EDITOR_HIGHLIGHT				25


/**
 * @def 	TAG_PREFERENCES_PARAM_FBROWSER_RESOLUTION
 * @brief	Dynamic value code for editor highlight property
 */
#define TAG_PREFERENCES_PARAM_FBROWSER_RESOLUTION			26


namespace tag {
/**
 * @class 		Configuration
 * @ingroup		GUI
 * Wrapper for TranscriberAG configuration, based on Parameters class
 *
 * All the values returned are got from the two configuration file\n
 * - transcriberAG.rc\n
 * - userAG.rc
 */

class Configuration
{
	public:
		/**
		 * Constructor
		 * @param parameters	Pointer on application parameters object
		 */
		Configuration(Parameters* parameters);
		virtual ~Configuration();

		//> TRANSCRIPTION
		Glib::ustring get_DATAMODEL_conventions() ;
		Glib::ustring get_DATAMODEL_defaultConvention() ;
		Glib::ustring get_DATAMODEL_languages() ;
		Glib::ustring get_DATAMODEL_defaultLanguage() ;
		Glib::ustring get_AUDIO_defaultRep() ;
		void set_DATAMODEL_defaultConvention(Glib::ustring value, bool save)  ;
		void set_DATAMODEL_defaultLanguage(Glib::ustring value, bool save)  ;
		void set_AUDIO_defaultRep(Glib::ustring value, bool save) ;

		//> FTP
		Glib::ustring get_FTP_received_path() ;
		Glib::ustring get_FTP_toSend_path() ;
		Glib::ustring get_FTP_machine() ;
		void set_FTP_received_path(Glib::ustring value, bool save) ;
		void set_FTP_machine(Glib::ustring value, bool save) ;

		//> SPEAKERS
		Glib::ustring get_global_dictionary() ;
		Glib::ustring get_global_dictionary_url() ;
		void set_global_dictionary(Glib::ustring value, bool save) ;
		void set_global_dictionary_url(Glib::ustring value, bool save) ;

		//> START
		Glib::ustring get_TRANSAG_path() ;
		Glib::ustring get_CONFIG_path() ;
		Glib::ustring get_DEMO_path() ;
		Glib::ustring get_AG_DTD_path() ;

		//> GENERAL GUI
		Glib::ustring get_UserDoc_path() ;
		Glib::ustring get_USER_acronym() ;
		Glib::ustring get_InputLanguages_path() ;
		Glib::ustring get_languages_path() ;
		Glib::ustring get_WORKDIR_path() ;
		Glib::ustring get_SETTINGS_path() ;
		Glib::ustring get_SHORTCUTS_path() ;
		Glib::ustring get_CLIPBOARD_path() ;
		Glib::ustring get_NOTEBOOK_opened_path() ;
		Glib::ustring get_MENU_recent_path() ;
		int get_GUI_loadOpenedFiles() ;
		int get_GUI_searchToolbarMode() ;
		Glib::ustring get_TOOLBAR_displayLabel() ;
		void set_TOOLBAR_displayLabel(Glib::ustring value, bool save) ;
		int get_GUI_statusbarShow() ;
		int get_GUI_toolbarShow() ;
		void set_GUI_statusbarShow(bool value, bool save) ;
		void set_GUI_toolbarShow(bool value, bool save) ;
		void set_USER_acronym(Glib::ustring value, bool save) ;
		void set_GUI_loadOpenedFiles(bool value, bool save) ;
		void set_GUI_searchToolbarMode(bool value, bool save) ;
		void set_InputLanguages_path(Glib::ustring value, bool save) ;
		void set_WORKDIR_path(Glib::ustring value, bool save) ;
		Glib::ustring get_GUI_defaultBrowser() ;
		void set_GUI_defaultBrowser(Glib::ustring, bool save) ;

		//> TEXT EDITOR
		Glib::ustring get_EDITOR_defaultFont() ;
		void set_EDITOR_defaultFont(Glib::ustring value, bool save) ;
		Glib::ustring get_EDITOR_BrowseStereoViewMode() ;
		int get_EDITOR_BrowseSynchroStT() ;
		int get_EDITOR_BrowseSynchroTtS() ;
		Glib::ustring get_EDITOR_BrowseHighlight() ;
		int get_EDITOR_autosave() ;
		int get_EDITOR_activity() ;
		void set_EDITOR_autosave(int value, bool save) ;
		void set_EDITOR_activity(int value, bool save) ;
		void set_EDITOR_BrowseStereoViewMode(Glib::ustring value, bool save) ;
		void set_EDITOR_BrowseHighlight(Glib::ustring value, bool save) ;
		void set_EDITOR_BrowseSynchroStT(bool value, bool save) ;
		void set_EDITOR_BrowseSynchroTtS(bool value, bool save) ;
		Glib::ustring get_EDITOR_EditStereoViewMode() ;
		int get_EDITOR_EditSynchroStT() ;
		int get_EDITOR_EditSynchroTtS() ;
		Glib::ustring get_EDITOR_EditHighlight() ;
		void set_EDITOR_EditHighlight(Glib::ustring value, bool save) ;
		void set_EDITOR_EditStereoViewMode(Glib::ustring value, bool save) ;
		void set_EDITOR_EditSynchroStT(bool value, bool save) ;
		void set_EDITOR_EditSynchroTtS(bool value, bool save) ;
		int get_EDITOR_EditAllowBrowseOnTags() ;
		void set_EDITOR_EditAllowBrowseOnTags(bool value, bool save) ;
		Glib::ustring get_EDITOR_EditAllowKeyDelete() ;
		void set_EDITOR_EditAllowKeyDelete(Glib::ustring value, bool save) ;
		int get_EDITOR_newLineAfterSectionLabel() ;
		int get_EDITOR_newLineAfterTurnLabelMono() ;
		int get_EDITOR_newLineAfterTurnLabelStereo() ;
		void set_EDITOR_newLineAfterSectionLabel(bool value, bool save) ;
		void set_EDITOR_newLineAfterTurnLabelMono(bool value, bool save) ;
		void set_EDITOR_newLineAfterTurnLabelStereo(bool value, bool save) ;
		int get_EDITOR_autosetLanguage() ;
		void set_EDITOR_autosetLanguage(bool value, bool save) ;
		int get_EDITOR_entitiesBg() ;
		void set_EDITOR_entitiesBg(bool value, bool save) ;

		void set_EDITOR_textFontStyle(Glib::ustring value, bool save) ;
		void set_EDITOR_labelFontStyle(Glib::ustring value, bool save) ;
		Glib::ustring get_EDITOR_textFontStyle() ;
		Glib::ustring get_EDITOR_labelFontStyle() ;

		int get_EDITOR_displayQualifierTooltips() ;
		void set_EDITOR_displayQualifierTooltips(bool value, bool save) ;


		//> AUDIO
		void set_AUDIO_absoluteNormalisation(bool value, bool save)  ;
		int get_AUDIO_absoluteNormalisation() ;
		void set_AUDIO_showTimeScale(bool value, bool save) ;
		int get_AUDIO_showTimeScale() ;
		void set_AUDIO_rewindAtEnd(bool value, bool save) ;
		int get_AUDIO_rewindAtEnd() ;
		void set_AUDIO_rewindAtSelectionEnd(bool value, bool save) ;
		int get_AUDIO_rewindAtSelectionEnd() ;
		void set_AUDIO_stopOnClick(bool value, bool save) ;
		int get_AUDIO_stopOnClick() ;
		void set_AUDIO_resolutionZoom(double value, bool save) ;
		double get_AUDIO_resolutionZoom() ;
		void set_AUDIO_restartDelay(double value, bool save) ;
		double get_AUDIO_restartDelay() ;
		void set_AUDIO_signalVScaleSize(double value, bool save) ;
		double get_AUDIO_signalVScaleSize() ;

		//> VIDEO
		int get_VIDEO_frameBrowserResolution() ;
		void set_VIDEO_frameBrowserResolution(int value, bool save) ;

		//> LANGUAGE
		int get_externalIME_mode() ;

		//> SPELLER
		Glib::ustring get_SPELLER_dicoPath() ;
		int get_SPELLER_allowIgnoreWord() ;
		int get_SPELLER_allowUserDico() ;
		int get_SPELLER_state() ;
		void set_SPELLER_dicoPath(Glib::ustring value, bool save) ;
		void set_SPELLER_allowIgnoreWord(bool value, bool save) ;
		void set_SPELLER_allowUserDico(bool value, bool save) ;
		void set_SPELLER_state(bool value, bool save) ;

		//> COLORS
		Glib::ustring get_COLORS_EDITOR_event(Glib::ustring g_type) ;
		void set_COLORS_EDITOR_event(Glib::ustring value, bool save, Glib::ustring g_type) ;
		Glib::ustring get_COLORS_EDITOR_background(Glib::ustring g_type) ;
		void set_COLORS_EDITOR_background(Glib::ustring value, bool save, Glib::ustring g_type) ;
		Glib::ustring get_COLORS_EDITOR_entity(Glib::ustring g_type) ;
		void set_COLORS_EDITOR_entity(Glib::ustring value, bool save, Glib::ustring g_type) ;
		Glib::ustring get_COLORS_EDITOR_highlight1(Glib::ustring g_type) ;
		Glib::ustring get_COLORS_EDITOR_highlight2(Glib::ustring g_type) ;
		void set_COLORS_EDITOR_highlight1(Glib::ustring value, bool save, Glib::ustring g_type) ;
		void set_COLORS_EDITOR_highlight2(Glib::ustring value, bool save, Glib::ustring g_type) ;
		Glib::ustring get_COLORS_EDITOR_text(Glib::ustring g_type) ;
		void set_COLORS_EDITOR_text(Glib::ustring value, bool save, Glib::ustring g_type) ;

		Glib::ustring get_COLORS_AUDIO_signal(Glib::ustring g_type) ;
		void set_COLORS_AUDIO_signal(Glib::ustring value, bool save, Glib::ustring g_type) ;
		Glib::ustring get_COLORS_AUDIO_signalActivated(Glib::ustring g_type) ;
		void set_COLORS_AUDIO_signalActivated(Glib::ustring value, bool save, Glib::ustring g_type) ;
		Glib::ustring get_COLORS_AUDIO_signalDisabled(Glib::ustring g_type) ;
		void set_COLORS_AUDIO_signalDisabled(Glib::ustring value, bool save, Glib::ustring g_type) ;
		Glib::ustring get_COLORS_AUDIO_signalSelection(Glib::ustring g_type) ;
		void set_COLORS_AUDIO_signalSelection(Glib::ustring value, bool save, Glib::ustring g_type) ;
		Glib::ustring get_COLORS_AUDIO_signalTooltip(Glib::ustring g_type) ;
		void set_COLORS_AUDIO_signalTooltip(Glib::ustring value, bool save, Glib::ustring g_type) ;
		Glib::ustring get_COLORS_AUDIO_signalCursor(Glib::ustring g_type) ;
		void set_COLORS_AUDIO_signalCursor(Glib::ustring value, bool save, Glib::ustring g_type) ;
		Glib::ustring get_COLORS_AUDIO_signalSegmentEnd(Glib::ustring g_type) ;
		void set_COLORS_AUDIO_signalSegmentEnd(Glib::ustring value, bool save, Glib::ustring g_type) ;

		/**
		 * Reload user configuration from parameter file
		 * @return	True for success, False for failure
		 */
		bool reload_user_config() ;

		/**
		 * Save user configuration from parameter file
		 * @return	True for success, False for failure
		 */
		bool save_user_config() ;

	private:
		Parameters* param ;
		Parameters users_param ;
		bool users_param_on ;

		void set_value(Glib::ustring id_comp, Glib::ustring id_secParam, Glib::ustring value, bool create, bool save) ;
		void set_bvalue(Glib::ustring id_comp, Glib::ustring id_secParam, bool value, bool create, bool save) ;
		void set_ivalue(Glib::ustring id_comp, Glib::ustring id_secParam, int value, bool create, bool save) ;
		void set_dvalue(Glib::ustring id_comp, Glib::ustring id_secParam, double value, bool create, bool save) ;
		int from_boolString_to_int(Glib::ustring value) ;
};

} //namespace

#endif /*CONFIGURATION_H_*/
