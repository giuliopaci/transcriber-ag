/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Configuration.h"
#include "Common/util/FileHelper.h"
#include "Common/util/Utils.h"

namespace tag {

Configuration::Configuration(Parameters* parameters)
{
	users_param_on = false ;
	param = parameters ;
	if (parameters)
	{
		Glib::ustring rcUser = param->getParameterValue("General", "start,userFile") ;
		if (Glib::file_test(rcUser, Glib::FILE_TEST_EXISTS)) {
			try {
				users_param.load(rcUser) ;
				users_param_on = true ;
			}
			catch (const char* e) {
				Log::err() << "<!> configuration::error while loading local user file\n" << e << std::endl ;
			}
		}
		else
			Log::err() << "<!> configuration::error while loading local user file" << std::endl ;

	}
	else
		Log::err() << "<!> configuration::invalid intialisation parameters" << std::endl ;
}

Configuration::~Configuration()
{
}


//*******************************************************************************************************
//*******************************************************************************************************
//************************************************** GETTERS  *******************************************
//*******************************************************************************************************
//*******************************************************************************************************
// <!> PARAM is a virtual parameters list, done by merging SYSTEM configuration and USER configuration
// ALWAYS load from virtual list, not from SYSTEM or USER list


Glib::ustring Configuration::get_AG_DTD_path()
{
  	Glib::ustring value ;
  	Glib::ustring home_folder = param->getParameterValue("General", "start,homeFolder") ;
 	Glib::ustring home_dir = Glib::get_home_dir() ;
 	Glib::ustring home_folder_path = FileHelper::build_path(home_dir,home_folder) ;
	Glib::ustring dtd = param->getParameterValue("General", "start,dtd") ;
 	if (dtd!="" && dtd!= " ")
 		value = FileHelper::build_path(home_folder_path, dtd) ;
 	else
 		value = TAG_CONFIG_NULL_VALUE ;
 	return value;
}

Glib::ustring Configuration::get_global_dictionary()
{
	//get value from configuration file
	Glib::ustring value = param->getParameterValue("Data", "speaker,globalDictionary") ;
	//> if path is aboslute return path
	if (Glib::path_is_absolute(value) || value==" " || value== "")
		return value ;
	//> else compute local path
	else {
		Glib::ustring home_folder = param->getParameterValue("General", "start,homeFolder") ;
		Glib::ustring home_dir = Glib::get_home_dir() ;
		Glib::ustring home_folder_path = FileHelper::build_path(home_dir,home_folder) ;
		value = FileHelper::build_path(home_folder_path, value) ;
		return value;
	}
}

Glib::ustring Configuration::get_global_dictionary_url()
{
	Glib::ustring value = param->getParameterValue("Data", "speaker,remoteDictionary") ;
	return value ;
}

void Configuration::set_global_dictionary_url(Glib::ustring value, bool save)
{
	set_value("Data", "speaker,remoteDictionary", value, true, save) ;
}


Glib::ustring Configuration::get_FTP_received_path()
{
	Glib::ustring value = param->getParameterValue("FTP", "connection,receivedFTP");
	return value;
}

Glib::ustring Configuration::get_WORKDIR_path()
{
	Glib::ustring value = param->getParameterValue("General", "start,workdir") ;
	return value;
}


Glib::ustring Configuration::get_CONFIG_path()
{
	Glib::ustring value = param->getParameterValue("General", "start,config") ;
	return value;
}

Glib::ustring Configuration::get_DEMO_path()
{
	Glib::ustring name = param->getParameterValue("General", "start,demo") ;
	if (name!="" && name!= " ")
		/*** Video Demo ***/
		//		return FileHelper::build_path(param->getParameterValue("General", "start,config"),name) ;
		return name ;
	else
		return TAG_CONFIG_NULL_VALUE ;
}

Glib::ustring Configuration::get_NOTEBOOK_opened_path()
{
  	Glib::ustring value ;
  	Glib::ustring home_folder = param->getParameterValue("General", "start,homeFolder") ;
 	Glib::ustring home_dir = Glib::get_home_dir() ;
 	Glib::ustring home_folder_path = FileHelper::build_path(home_dir,home_folder) ;
	Glib::ustring opened_file = param->getParameterValue("GUI", "data,openedFile") ;
 	if (opened_file!="" && opened_file!=" ")
 		value = FileHelper::build_path(home_folder_path, opened_file) ;
 	else
 		value = TAG_CONFIG_NULL_VALUE ;
 	return value;
}

Glib::ustring Configuration::get_SHORTCUTS_path()
{
  	Glib::ustring value ;
  	Glib::ustring home_folder = param->getParameterValue("General", "start,homeFolder") ;
 	Glib::ustring home_dir = Glib::get_home_dir() ;
 	Glib::ustring home_folder_path = FileHelper::build_path(home_dir,home_folder) ;
	Glib::ustring shortcuts = param->getParameterValue("GUI", "data,shortcuts") ;
 	if (shortcuts!="" && shortcuts!=" ")
 		value = FileHelper::build_path(home_folder_path, shortcuts) ;
 	else
 		value = TAG_CONFIG_NULL_VALUE ;
 	return value;
}

Glib::ustring Configuration::get_SETTINGS_path()
{
  	Glib::ustring value ;
  	Glib::ustring home_folder = param->getParameterValue("General", "start,homeFolder") ;
 	Glib::ustring home_dir = Glib::get_home_dir() ;
 	Glib::ustring home_folder_path = FileHelper::build_path(home_dir,home_folder) ;
	Glib::ustring settings = param->getParameterValue("GUI", "data,settings") ;
 	if (settings!="" && settings!=" ")
 		value = FileHelper::build_path(home_folder_path, settings) ;
 	else
 		value = TAG_CONFIG_NULL_VALUE ;
 	return value;
}

Glib::ustring Configuration::get_MENU_recent_path()
{
  	Glib::ustring value ;
  	Glib::ustring home_folder = param->getParameterValue("General", "start,homeFolder") ;
 	Glib::ustring home_dir = Glib::get_home_dir() ;
 	Glib::ustring home_folder_path = FileHelper::build_path(home_dir,home_folder) ;
	Glib::ustring recent = param->getParameterValue("GUI", "data,recentFile") ;
 	if (recent!="" && recent!=" ")
 		value = FileHelper::build_path(home_folder_path, recent) ;
 	else
 		value = TAG_CONFIG_NULL_VALUE ;
 	return value;
}

Glib::ustring Configuration::get_FTP_toSend_path()
{
  	Glib::ustring value ;
  	Glib::ustring home_folder = param->getParameterValue("General", "start,homeFolder") ;
 	Glib::ustring home_dir = Glib::get_home_dir() ;
 	Glib::ustring home_folder_path = FileHelper::build_path(home_dir,home_folder) ;
	Glib::ustring toSend = param->getParameterValue("FTP", "connection,toSendFTP") ;
	if (toSend!="" && toSend!=" ")
		value = FileHelper::build_path(home_folder_path, toSend) ;
	else
		value = TAG_CONFIG_NULL_VALUE ;
	return value;
}

Glib::ustring Configuration::get_CLIPBOARD_path()
{
  	Glib::ustring value ;
  	Glib::ustring home_folder = param->getParameterValue("General", "start,homeFolder") ;
 	Glib::ustring home_dir = Glib::get_home_dir() ;
 	Glib::ustring home_folder_path = FileHelper::build_path(home_dir,home_folder) ;
	Glib::ustring clip = param->getParameterValue("GUI", "data,clipboard") ;
	if (clip!="" && clip!=" ")
		value = FileHelper::build_path(home_folder_path, clip) ;
	else
		value = TAG_CONFIG_NULL_VALUE ;
	return value;
}

Glib::ustring Configuration::get_languages_path()
{
	Glib::ustring lang = param->getParameterValue("Data", "languages,languagesFile") ;
	return lang;
}

Glib::ustring Configuration::get_TRANSAG_path()
{
  	Glib::ustring value ;
  	Glib::ustring home_folder = param->getParameterValue("General", "start,homeFolder") ;
 	Glib::ustring home_dir = Glib::get_home_dir() ;
 	Glib::ustring home_folder_path = FileHelper::build_path(home_dir,home_folder) ;
	return home_folder_path;
}

Glib::ustring Configuration::get_USER_acronym()
{
 	Glib::ustring value ;
	value = param->getParameterValue("General", "User,acronym");
	return value;
}

Glib::ustring Configuration::get_FTP_machine()
{
 	Glib::ustring value ;
	value = param->getParameterValue("FTP", "connection,machine");
	return value;
}

Glib::ustring Configuration::get_InputLanguages_path()
{
	Glib::ustring lang = param->getParameterValue("Data", "inputLanguage,inputFile") ;
	return lang ;
}

int Configuration::get_externalIME_mode()
{
	Glib::ustring mode = param->getParameterValue("Data", "inputLanguage,enableIME") ;
	return from_boolString_to_int(mode) ;
}

Glib::ustring Configuration::get_UserDoc_path()
{
	Glib::ustring doc = param->getParameterValue("General", "Doc,Docpath") ;
	return doc ;
}


//************************************ DATAMODEL*************************************************
//************************************************************************************************

Glib::ustring Configuration::get_DATAMODEL_conventions()
{
	Glib::ustring conventions = param->getParameterValue("DataModel", "Configuration,conventions");
	return conventions ;
}

Glib::ustring Configuration::get_DATAMODEL_defaultConvention()
{
	Glib::ustring conv_default = param->getParameterValue("DataModel", "Default,convention");
	return conv_default ;
}

Glib::ustring Configuration::get_DATAMODEL_languages()
{
	Glib::ustring languages = param->getParameterValue("DataModel", "Configuration,lang");
	return languages ;
}

Glib::ustring Configuration::get_DATAMODEL_defaultLanguage()
{
	Glib::ustring lang_default = param->getParameterValue("DataModel", "Default,lang");
	return lang_default ;
}

//************************************ AUDIO *************************************************
//******************************************************************************************


void Configuration::set_AUDIO_resolutionZoom(double value, bool save)
{
	set_dvalue("AnnotationEditor", "Signal,resolution", value, true, save) ;
}
double Configuration::get_AUDIO_resolutionZoom()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Signal,resolution");
	double res = string_to_number<double>(value) ;
	return res ;
}



void Configuration::set_AUDIO_absoluteNormalisation(bool value, bool save)
{
	if (value)
		set_value("AnnotationEditor", "Signal,peaks_norm", "absolute", true, save) ;
	else
		set_value("AnnotationEditor", "Signal,peaks_norm", "relative", true, save) ;
}
int Configuration::get_AUDIO_absoluteNormalisation()
{
	Glib::ustring load = param->getParameterValue("AnnotationEditor", "Signal,peaks_norm");
	if (load.compare("relative")==0)
		return false ;
	else
		return true ;
}

int Configuration::get_AUDIO_showTimeScale()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Signal,scale,show");
	return from_boolString_to_int(value) ;
}
void Configuration::set_AUDIO_showTimeScale(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "Signal,scale,show", value, true, save) ;
}

int Configuration::get_AUDIO_rewindAtEnd()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Signal,rewind_at_end");
	return from_boolString_to_int(value) ;
}
void Configuration::set_AUDIO_rewindAtEnd(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "Signal,rewind_at_end", value, true, save) ;
}

int Configuration::get_AUDIO_rewindAtSelectionEnd()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Signal,rewind_at_selection_end");
	return from_boolString_to_int(value) ;
}
void Configuration::set_AUDIO_rewindAtSelectionEnd(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "Signal,rewind_at_selection_end", value, true, save) ;
}

int Configuration::get_AUDIO_stopOnClick()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Signal,stop_on_click");
	return from_boolString_to_int(value) ;
}
void Configuration::set_AUDIO_stopOnClick(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "Signal,stop_on_click", value, true, save) ;
}

double Configuration::get_AUDIO_restartDelay()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Signal,restart_delay");
	double res = string_to_number<double>(value) ;
	return res ;
}
void Configuration::set_AUDIO_restartDelay(double value, bool save)
{
	set_dvalue("AnnotationEditor", "Signal,restart_delay", value, true, save) ;
}

double Configuration::get_AUDIO_signalVScaleSize()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Signal,vertical_scale_size");
	double res = string_to_number<double>(value) ;
	return res ;
}
void Configuration::set_AUDIO_signalVScaleSize(double value, bool save)
{
	set_dvalue("AnnotationEditor", "Signal,vertical_scale_size", value, true, save) ;
}

//************************************ VIDEO ***********************************************
//******************************************************************************************

int Configuration::get_VIDEO_frameBrowserResolution()
{
	Glib::ustring value = param->getParameterValue("Video", "frameBrowser,resolution");
	int res = string_to_number<int>(value) ;
	return res ;
}
void Configuration::set_VIDEO_frameBrowserResolution(int value, bool save)
{
	set_ivalue("Video", "frameBrowser,resolution", value, true, save) ;
}

//************************************ GUI *************************************************
//******************************************************************************************

int Configuration::get_GUI_statusbarShow()
{
	Glib::ustring value = param->getParameterValue("GUI", "statusbar,show");
	return from_boolString_to_int(value) ;
}

int Configuration::get_GUI_toolbarShow()
{
	Glib::ustring value = param->getParameterValue("GUI", "toolbar,show");
	return from_boolString_to_int(value) ;
}

int Configuration::get_GUI_loadOpenedFiles()
{
	Glib::ustring value = param->getParameterValue("GUI", "data,openLastFiles");
	return from_boolString_to_int(value) ;
}

Glib::ustring Configuration::get_TOOLBAR_displayLabel()
{
	Glib::ustring value = param->getParameterValue("GUI", "toolbar,display_label") ;
	return value ;
}

int Configuration::get_GUI_searchToolbarMode()
{
	Glib::ustring value = param->getParameterValue("GUI", "search,toolbarMode") ;
	return from_boolString_to_int(value) ;
}

Glib::ustring Configuration::get_EDITOR_defaultFont()
{
	Glib::ustring font = param->getParameterValue("AnnotationEditor", "style,defaultfont") ;
	return font ;
}

Glib::ustring Configuration::get_AUDIO_defaultRep()
{
	Glib::ustring value = param->getParameterValue("General", "start,audiodir") ;
	return value;
}

//************************************ EDITOR **********************************************
//******************************************************************************************


int Configuration::get_EDITOR_EditAllowBrowseOnTags()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "EditMode,allow_browse_on_tags") ;
	if ( value.compare("true")==0 )
		return true ;
	else
		return false ;
}

Glib::ustring Configuration::get_EDITOR_EditAllowKeyDelete()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "EditMode,allow_key_delete") ;
	return value;
}

Glib::ustring Configuration::get_EDITOR_BrowseStereoViewMode()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "BrowseMode,stereo,viewmode") ;
	return value;
}

int Configuration::get_EDITOR_BrowseSynchroStT()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "BrowseMode,synchro_signal_to_text") ;
	return from_boolString_to_int(value) ;
}

int Configuration::get_EDITOR_BrowseSynchroTtS()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "BrowseMode,synchro_text_to_signal") ;
	return from_boolString_to_int(value) ;
}

Glib::ustring Configuration::get_EDITOR_BrowseHighlight()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "BrowseMode,highlight_current") ;
	return value ;
}

Glib::ustring Configuration::get_EDITOR_EditStereoViewMode()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "EditMode,stereo,viewmode") ;
	return value;
}

int Configuration::get_EDITOR_EditSynchroStT()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "EditMode,synchro_signal_to_text") ;
	return from_boolString_to_int(value) ;
}

int Configuration::get_EDITOR_EditSynchroTtS()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "EditMode,synchro_text_to_signal") ;
	return from_boolString_to_int(value) ;
}

Glib::ustring Configuration::get_EDITOR_EditHighlight()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "EditMode,highlight_current") ;
	return value ;
}

int Configuration::get_EDITOR_autosave()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Autosave,period") ;
	if (value.compare("")!=0 && value.compare(" ")!=0) {
		int i = atoi(value.c_str()) ;
		if ( i>=0 )
			return i ;
		else return -1 ;
	}
	else
		return -1 ;
}

int Configuration::get_EDITOR_activity()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Activity,delay") ;
	if (value.compare("")!=0 && value.compare(" ")!=0) {
		int i = atoi(value.c_str()) ;
		if ( i>=0 )
			return i ;
		else
			return -1 ;
	}
	else
		return -1 ;
}

int Configuration::get_EDITOR_newLineAfterSectionLabel()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "AnnotationLayout,section,newline_after_label") ;
	return from_boolString_to_int(value) ;
}

int Configuration::get_EDITOR_newLineAfterTurnLabelMono()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "AnnotationLayout,turn,newline_after_label") ;
	return from_boolString_to_int(value) ;
}

int Configuration::get_EDITOR_newLineAfterTurnLabelStereo()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "AnnotationLayout,turn,newline_after_label,stereo") ;
	return from_boolString_to_int(value) ;
}

int Configuration::get_EDITOR_autosetLanguage()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "EditMode,autoset_language") ;
	return from_boolString_to_int(value) ;
}

int Configuration::get_EDITOR_entitiesBg()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "AnnotationLayout,qualifiers,entity_bg") ;
	return from_boolString_to_int(value) ;
}

Glib::ustring Configuration::get_EDITOR_textFontStyle()
{
	Glib::ustring value = param->getParameterValue("Display", "Fonts-editor,text") ;
	return value ;
}

Glib::ustring Configuration::get_EDITOR_labelFontStyle()
{
	Glib::ustring value = param->getParameterValue("Display", "Fonts-editor,label") ;
	return value ;
}

int Configuration::get_EDITOR_displayQualifierTooltips()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "AnnotationLayout,qualifiers,tooltip") ;
	if ( value.compare("true")==0 )
		return true ;
	else
		return false ;
}
void Configuration::set_EDITOR_displayQualifierTooltips(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "AnnotationLayout,qualifiers,tooltip", value, true, save) ;
}

//************************************ SPELLER **********************************************
//******************************************************************************************

int Configuration::get_SPELLER_state()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Speller,enabled") ;
	return from_boolString_to_int(value) ;
}

Glib::ustring Configuration::get_SPELLER_dicoPath()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Speller,directory") ;
	return value;
}

int Configuration::get_SPELLER_allowUserDico()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Speller,allow_user_dic") ;
	return from_boolString_to_int(value) ;
}

int Configuration::get_SPELLER_allowIgnoreWord()
{
	Glib::ustring value = param->getParameterValue("AnnotationEditor", "Speller,allow_ignore_word") ;
	return from_boolString_to_int(value) ;
}

Glib::ustring Configuration::get_GUI_defaultBrowser()
{
	Glib::ustring value = param->getParameterValue("General", "Browser,default") ;
	return value ;
}

void Configuration::set_SPELLER_dicoPath(Glib::ustring value, bool save)
{
	set_value("AnnotationEditor", "Speller,directory", value, true, save) ;
}

void Configuration::set_SPELLER_allowIgnoreWord(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "Speller,allow_ignore_word", value, true, save) ;
}

void Configuration::set_SPELLER_allowUserDico(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "Speller,allow_user_dic", value, true, save) ;
}

void Configuration::set_SPELLER_state(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "Speller,enabled", value, true, save) ;
}

//******************************* EDITOR COLORS *******************************************
//******************************************************************************************

Glib::ustring Configuration::get_COLORS_EDITOR_event(Glib::ustring g_type)
{
	Glib::ustring value ;
	if (g_type=="bg")
		value = param->getParameterValue("Display", "Colors-editor,qualifier_event_bg") ;
	else
		value = param->getParameterValue("Display", "Colors-editor,qualifier_event_fg") ;
	return value ;
}
void Configuration::set_COLORS_EDITOR_event(Glib::ustring value, bool save, Glib::ustring g_type)
{
	if (g_type=="bg")
		set_value("Display", "Colors-editor,qualifier_event_bg", value, true, save) ;
	else
		set_value("Display", "Colors-editor,qualifier_event_fg", value, true, save) ;
}

Glib::ustring Configuration::get_COLORS_EDITOR_background(Glib::ustring g_type)
{
	Glib::ustring value ;
	if (g_type=="bg")
		value = param->getParameterValue("Display", "Colors-editor,background_bg") ;
	else
		value = param->getParameterValue("Display", "Colors-editor,background_fg") ;
	return value ;
}
void Configuration::set_COLORS_EDITOR_background(Glib::ustring value, bool save, Glib::ustring g_type)
{
	if (g_type=="bg")
		set_value("Display", "Colors-editor,background_bg", value, true, save) ;
	else
		set_value("Display", "Colors-editor,background_fg", value, true, save) ;
}

Glib::ustring Configuration::get_COLORS_EDITOR_entity(Glib::ustring g_type)
{
	Glib::ustring value ;
	if (g_type=="bg")
		value = param->getParameterValue("Display", "Colors-editor,qualifier_entity_bg") ;
	else
		value = param->getParameterValue("Display", "Colors-editor,qualifier_entity_fg") ;
	return value ;
}
void Configuration::set_COLORS_EDITOR_entity(Glib::ustring value, bool save, Glib::ustring g_type)
{
	if (g_type=="bg")
		set_value("Display", "Colors-editor,qualifier_entity_bg", value, true, save) ;
	else
		set_value("Display", "Colors-editor,qualifier_entity_fg", value, true, save) ;
}

Glib::ustring Configuration::get_COLORS_EDITOR_highlight1(Glib::ustring g_type)
{
	Glib::ustring value = param->getParameterValue("Display", "Colors-editor,highlight1_segment_bg") ;
	return value ;
}
void Configuration::set_COLORS_EDITOR_highlight1(Glib::ustring value, bool save, Glib::ustring g_type)
{
	set_value("Display", "Colors-editor,highlight1_segment_bg", value, true, save) ;
}

Glib::ustring Configuration::get_COLORS_EDITOR_highlight2(Glib::ustring g_type)
{
	Glib::ustring value = param->getParameterValue("Display", "Colors-editor,highlight2_segment_bg") ;
	return value ;
}
void Configuration::set_COLORS_EDITOR_highlight2(Glib::ustring value, bool save, Glib::ustring g_type)
{
	set_value("Display", "Colors-editor,highlight2_segment_bg", value, true, save) ;
}

Glib::ustring Configuration::get_COLORS_EDITOR_text(Glib::ustring g_type)
{
	Glib::ustring value ;
	if (g_type=="bg")
		value = param->getParameterValue("Display", "Colors-editor,text_bg") ;
	else
		value = param->getParameterValue("Display", "Colors-editor,text_fg") ;
	return value ;
}
void Configuration::set_COLORS_EDITOR_text(Glib::ustring value, bool save, Glib::ustring g_type)
{
	if (g_type=="bg")
		set_value("Display", "Colors-editor,text_bg", value, true, save) ;
	else
		set_value("Display", "Colors-editor,text_fg", value, true, save) ;
}


//******************************* AUDIO COLORS *********************************************
//******************************************************************************************

Glib::ustring Configuration::get_COLORS_AUDIO_signal(Glib::ustring g_type)
{
	Glib::ustring value ;
	if (g_type=="bg")
		value = param->getParameterValue("Display", "Colors-audio,signal_bg") ;
	else
		value = param->getParameterValue("Display", "Colors-audio,signal_fg") ;
	return value ;
}
void Configuration::set_COLORS_AUDIO_signal(Glib::ustring value, bool save, Glib::ustring g_type)
{
	if (g_type=="bg")
		set_value("Display", "Colors-audio,signal_bg", value, true, save) ;
	else
		set_value("Display", "Colors-audio,signal_fg", value, true, save) ;
}

Glib::ustring Configuration::get_COLORS_AUDIO_signalActivated(Glib::ustring g_type)
{
	Glib::ustring value ;
	value = param->getParameterValue("Display", "Colors-audio,signal_bg_active") ;
	return value ;
}
void Configuration::set_COLORS_AUDIO_signalActivated(Glib::ustring value, bool save, Glib::ustring g_type)
{
	set_value("Display", "Colors-audio,signal_bg_active", value, true, save) ;
}

Glib::ustring Configuration::get_COLORS_AUDIO_signalDisabled(Glib::ustring g_type)
{
	Glib::ustring value ;
	value = param->getParameterValue("Display", "Colors-audio,signal_disabled") ;
	return value ;
}
void Configuration::set_COLORS_AUDIO_signalDisabled(Glib::ustring value, bool save, Glib::ustring g_type)
{
	set_value("Display", "Colors-audio,signal_disabled", value, true, save) ;
}

Glib::ustring Configuration::get_COLORS_AUDIO_signalSelection(Glib::ustring g_type)
{
	Glib::ustring value ;
	value = param->getParameterValue("Display", "Colors-audio,signal_selection") ;
	return value ;
}
void Configuration::set_COLORS_AUDIO_signalSelection(Glib::ustring value, bool save, Glib::ustring g_type)
{
	set_value("Display", "Colors-audio,signal_selection", value, true, save) ;
}

Glib::ustring Configuration::get_COLORS_AUDIO_signalTooltip(Glib::ustring g_type)
{
	Glib::ustring value ;
	if (g_type=="bg")
		value = param->getParameterValue("Display", "Colors-audio,signal_tip_bg") ;
	else
		value = param->getParameterValue("Display", "Colors-audio,signal_tip_fg") ;
	return value ;
}
void Configuration::set_COLORS_AUDIO_signalTooltip(Glib::ustring value, bool save, Glib::ustring g_type)
{
	if (g_type=="bg")
		set_value("Display", "Colors-audio,signal_tip_bg", value, true, save) ;
	else
		set_value("Display", "Colors-audio,signal_tip_fg", value, true, save) ;
}

Glib::ustring Configuration::get_COLORS_AUDIO_signalCursor(Glib::ustring g_type)
{
	Glib::ustring value ;
	value = param->getParameterValue("Display", "Colors-audio,signal_cursor") ;
	return value ;
}
void Configuration::set_COLORS_AUDIO_signalCursor(Glib::ustring value, bool save, Glib::ustring g_type)
{
	set_value("Display", "Colors-audio,signal_cursor", value, true, save) ;
}

Glib::ustring Configuration::get_COLORS_AUDIO_signalSegmentEnd(Glib::ustring g_type)
{
	Glib::ustring value ;
	value = param->getParameterValue("Display", "Colors-audio,signal_segment_end") ;
	return value ;
}
void Configuration::set_COLORS_AUDIO_signalSegmentEnd(Glib::ustring value, bool save, Glib::ustring g_type)
{
	set_value("Display", "Colors-audio,signal_segment_end", value, true, save) ;
}


//**********************************************************************************
//**********************************************************************************
//********************************** SETTERS ***************************************
//**********************************************************************************
//**********************************************************************************


void Configuration::set_global_dictionary(Glib::ustring value, bool save)
{
	set_value("Data", "speaker,globalDictionary", value, true, save) ;
}

void Configuration::set_InputLanguages_path(Glib::ustring value, bool save)
{
	set_value("Data", "inputLanguage,inputFile", value, true, save) ;
}

void Configuration::set_FTP_machine(Glib::ustring value, bool save)
{
	set_value("FTP", "connection,machine", value, true, save) ;
}

void Configuration::set_USER_acronym(Glib::ustring value, bool save)
{
	set_value("General", "User,acronym", value, true, save) ;
}
void Configuration::set_FTP_received_path(Glib::ustring value, bool save)
{
	set_value("FTP", "connection,receivedFTP", value, false, save);
}
void Configuration::set_WORKDIR_path(Glib::ustring value, bool save)
{
	set_value("General", "start,workdir", value, false, save);
}

void Configuration::set_DATAMODEL_defaultConvention(Glib::ustring value, bool save)
{
	set_value("DataModel", "Default,convention", value, true, save) ;
}

void Configuration::set_DATAMODEL_defaultLanguage(Glib::ustring value, bool save)
{
	set_value("DataModel", "Default,lang", value, true, save) ;
}

void Configuration::set_EDITOR_defaultFont(Glib::ustring value, bool save)
{
	set_value("AnnotationEditor", "style,defaultfont", value, true, save) ;
}

void Configuration::set_GUI_loadOpenedFiles(bool value, bool save)
{
	set_bvalue("GUI", "data,openLastFiles", value, true, save) ;
}

void Configuration::set_GUI_statusbarShow(bool value, bool save)
{
	set_bvalue("GUI", "statusbar,show", value, true, save) ;
}

void Configuration::set_GUI_toolbarShow(bool value, bool save)
{
	set_bvalue("GUI", "toolbar,show", value, true, save) ;
}


void Configuration::set_AUDIO_defaultRep(Glib::ustring value, bool save)
{
	set_value("General", "start,audiodir", value, true, save) ;
}



void Configuration::set_GUI_searchToolbarMode(bool value, bool save)
{
	set_bvalue("GUI", "search,toolbarMode", value, true, save) ;
}

void Configuration::set_EDITOR_BrowseStereoViewMode(Glib::ustring value, bool save)
{
	set_value("AnnotationEditor", "BrowseMode,stereo,viewmode", value, true, save) ;
}

void Configuration::set_EDITOR_BrowseSynchroStT(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "BrowseMode,synchro_signal_to_text", value, true, save) ;
}

void Configuration::set_EDITOR_BrowseSynchroTtS(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "BrowseMode,synchro_text_to_signal", value, true, save) ;
}

void Configuration::set_EDITOR_BrowseHighlight(Glib::ustring value, bool save)
{
	set_value("AnnotationEditor", "BrowseMode,highlight_current", value, true, save) ;
}

void Configuration::set_EDITOR_EditStereoViewMode(Glib::ustring value, bool save)
{
	set_value("AnnotationEditor", "EditMode,stereo,viewmode", value, true, save) ;
}

void Configuration::set_EDITOR_EditSynchroStT(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "EditMode,synchro_signal_to_text", value, true, save) ;
}

void Configuration::set_EDITOR_EditSynchroTtS(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "EditMode,synchro_text_to_signal", value, true, save) ;
}

void Configuration::set_EDITOR_EditHighlight(Glib::ustring value, bool save)
{
	set_value("AnnotationEditor", "EditMode,highlight_current", value, true, save) ;
}


void Configuration::set_EDITOR_EditAllowBrowseOnTags(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "EditMode,allow_browse_on_tags", value, true, save) ;
}


void Configuration::set_EDITOR_EditAllowKeyDelete(Glib::ustring value, bool save)
{
	set_value("AnnotationEditor", "EditMode,allow_key_delete", value, true, save) ;
}

void Configuration::set_EDITOR_autosave(int value, bool save)
{
	set_ivalue("AnnotationEditor", "Autosave,period", value, true, save) ;
}


void Configuration::set_EDITOR_newLineAfterSectionLabel(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "AnnotationLayout,section,newline_after_label", value, true, save) ;
}

void Configuration::set_EDITOR_newLineAfterTurnLabelMono(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "AnnotationLayout,turn,newline_after_label", value, true, save) ;
}

void Configuration::set_EDITOR_newLineAfterTurnLabelStereo(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "AnnotationLayout,turn,newline_after_label,stereo", value, true, save) ;
}

void Configuration::set_EDITOR_activity(int value, bool save)
{
	set_ivalue("AnnotationEditor", "Activity,delay", value, true, save) ;
}

void Configuration::set_EDITOR_autosetLanguage(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "EditMode,autoset_language", value, true, save) ;
}

void Configuration::set_EDITOR_entitiesBg(bool value, bool save)
{
	set_bvalue("AnnotationEditor", "AnnotationLayout,qualifiers,entity_bg", value, true, save) ;
}

void Configuration::set_EDITOR_textFontStyle(Glib::ustring value, bool save)
{
	set_value("Display", "Fonts-editor,text", value, true, save) ;
}

void Configuration::set_EDITOR_labelFontStyle(Glib::ustring value, bool save)
{
	set_value("Display", "Fonts-editor,label", value, true, save) ;
}

void Configuration::set_TOOLBAR_displayLabel(Glib::ustring value, bool save)
{
	set_value("GUI", "toolbar,display_label", value, true, save) ;
}

void Configuration::set_GUI_defaultBrowser(Glib::ustring value, bool save)
{
	set_value("General", "Browser,default", value, true, save) ;
}

//**********************************************************************************
//********************************** ACTIONS ***************************************
//**********************************************************************************
// <!> CAUTION
// parameters is a merged view of USER PARAMETERS and SYSTEM PARAMETERS
// For saving configuration, we need to use USER PARAMETERS

void Configuration::set_value(Glib::ustring id_comp, Glib::ustring id_secParam, Glib::ustring value, bool create, bool save)
{
	if (users_param_on) {
		users_param.setParameterValue(id_comp, id_secParam, value, create) ;
		if (save)
			users_param.save() ;
		Parameters::mergeUserParameters(&users_param, param, false) ;
	}
}

void Configuration::set_bvalue(Glib::ustring id_comp, Glib::ustring id_secParam, bool value, bool create, bool save)
{
	Glib::ustring v ;
	if (value)
		v = "true" ;
	else
		v = "false" ;
	set_value(id_comp, id_secParam, v, create, save) ;
}

void Configuration::set_ivalue(Glib::ustring id_comp, Glib::ustring id_secParam, int value, bool create, bool save)
{
	Glib::ustring v = number_to_string(value) ;
	set_value(id_comp, id_secParam, v, create, save) ;
}

void Configuration::set_dvalue(Glib::ustring id_comp, Glib::ustring id_secParam, double value, bool create, bool save)
{
	Glib::ustring v = number_to_string(value) ;
	set_value(id_comp, id_secParam, v, create, save) ;
}


bool Configuration::reload_user_config()
{
	if (users_param_on)
	{
		try
		{
			users_param.reload() ;
			Parameters::mergeUserParameters(&users_param, param, false) ;
			return true ;
		}
		catch (const char* e) {
			return false ;
		}
	}
	return false ;
}

bool Configuration::save_user_config()
{
	if (users_param_on)
	{
		bool success = users_param.save() ;
		if (success)
			Parameters::mergeUserParameters(&users_param, param, false) ;
		return success ;
	}
	return false ;
}

int Configuration::from_boolString_to_int(Glib::ustring value)
{
	if (value.compare("true")==0 || (value.compare("yes")==0))
		return 1 ;
	else if (value.compare("false")==0 || value.compare("no")==0 )
		return 0 ;
	else
		return -1 ;
}

} //namespace
