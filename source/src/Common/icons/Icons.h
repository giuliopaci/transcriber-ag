/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file 	Icons.h
 * @brief	File providing defined names for icons used in TranscriberAG
 */

#ifndef ICONS_H_
#define ICONS_H_

#include <glibmm.h>
#include <gtkmm/icontheme.h>
#include <gtkmm.h>
#include <stdio.h>
#include <iostream>


namespace tag {

#define ICO_TRANSCRIBER 					"Transcriber"
#define ICO_ERROR 							"errorAG2"
#define ICO_TRANSCRIBER_GEN 				"TranscriberGen"

/***************************************** PREFERENCES ************************************/

#define ICO_PREFERENCES_APPLY 				"validate"
#define ICO_PREFERENCES_CANCEL 				"cancel"
#define ICO_PREFERENCES 					"preferences"

#define ICO_PREFERENCES_VIDEO 				"preferences_video"
#define ICO_PREFERENCES_AUDIO 				"preferences_audio"
#define ICO_PREFERENCES_GENERAL 			"preferences_general"
#define ICO_PREFERENCES_TRANSCRIPT			"preferences_transcript"
#define ICO_PREFERENCES_EDITOR 				"preferences_editor"
#define ICO_PREFERENCES_SPELLER 			"preferences_speller"
#define ICO_PREFERENCES_LOOK 				"preferences_look"
#define ICO_PREFERENCES_CONNECTION 			"preferences_connection"
#define ICO_PREFERENCES_SPEAKERS 			"preferences_speakers"

#define ICO_PREFERENCES_RELOAD1 			"preferences_reload_level1"
#define ICO_PREFERENCES_RELOAD1_OVER 		"preferences_reload_level1_over"
#define ICO_PREFERENCES_RELOAD1_PRESSED 	"preferences_reload_level1_pressed"
#define ICO_PREFERENCES_RELOAD2 			"preferences_reload_level2"

#define ICO_PREFERENCES_LOOK_FONTS 			"preferences_look_fonts"
#define ICO_PREFERENCES_LOOK_COLORS 		"preferences_look_colors"

/***************************************** TREE ************************************/

#define ICO_TREE_SHORTCUT 					"bookmark_ag_add"
#define ICO_TREE_EXPLORER 					"tree"

/***************************************** SPEAKER ************************************/

#define ICO_SPEAKER_DICO_GLOBAL 			"dicoAG1"
#define ICO_SPEAKER_DICO_LOCAL 				"dicoAG2"
#define ICO_SPEAKER_ADDLANGUAGE 			"add"
#define ICO_SPEAKER_REMOVELANGUAGE			"remove"
#define ICO_SPEAKER_ADDSPEAKER				"add"
#define ICO_SPEAKER_REMOVESPEAKER 			"remove"
#define ICO_SPEAKER_SPEAKER		 			"speaker"
#define ICO_SPEAKER_DETAILS					"speaker_dico_details"

/***************************************** SEARCH_PANEL ************************************/

#define ICO_SEARCH_DIALOG "search_dialog"
#define ICO_SEARCH_NEXT "search_next"
#define ICO_SEARCH_PREVIOUS "search_previous"
#define ICO_SEARCH_CLOSE "search_close_black"
#define ICO_SEARCH_PANEL "search_toPanel"
#define ICO_SEARCH_TOOLBAR "search_toToolbar"

#define ICO_SEARCH_IN_PROGRESS 			"searching.gif"
#define ICO_SEARCH_IN_PROGRESS_STATIC 	"searching_static"

/***************************************** IMAGE DIALOG ************************************/

#define ICO_ID "id"
#define ICO_NETWORK "networkAG"
#define ICO_NETWORK_DISABLED "networkAG_disabled"

/***************************************** TREE FILE************************************/

#define ICO_TREE_INFO "file-info"
#define ICO_TREE_DIR "gnome_directory1"
#define ICO_TREE_DIR2 "gnome_directory2"
#define ICO_TREE_DEFAULT "file-default"
#define ICO_TREE_WAV "file-wave"
#define ICO_TREE_TAG "file-tag"
#define ICO_TREE_TRS "file-trs"
#define ICO_TREE_PDF "file-pdf"
#define ICO_TREE_EXCEL "file-excel"
#define ICO_TREE_WORD "file-word"
#define ICO_TREE_TXT "file-txt"

#define ICO_TREE_HTML "file-html"
#define ICO_TREE_XML "file-xml"

#define ICO_TREE_BAK "file-bak"
#define ICO_TREE_PPT "file-ppt"
#define ICO_TREE_SH "file-sh"

#define ICO_TREE_JPG "file-jpg"
#define ICO_TREE_PNG "file-png"
#define ICO_TREE_GIF "file-gif"

#define ICO_TREE_RPM "file-rpm"
#define ICO_TREE_TAR "file-tar"
#define ICO_TREE_TGZ "file-tgz"
#define ICO_TREE_ZIP "file-tar"
#define ICO_TREE_RAR "file-rar"
#define ICO_TREE_LIBA "file-liba"

#define ICO_TREE_H "file-h"
#define ICO_TREE_C "file-c"
#define ICO_TREE_CO "file-o"
#define ICO_TREE_CPP "file-cpp"
#define ICO_TREE_BINARY "file-binary"

#define ICO_TREE_JAVA "file-java"
#define ICO_TREE_JAVACLASS "file-javaclass"
#define ICO_TREE_JAR "file-jar"
#define ICO_TREE_PHP "file-php"

#define ICO_TREE_MPG "file-mpeg"
#define ICO_TREE_AVI "file-avi"
#define ICO_TREE_WMV "file-wmv"

#define ICO_ROOT_PROJECT "project_winter"
#define ICO_ROOT_FS "fs"
#define ICO_ROOT_FTP "distant"
#define ICO_ROOT_PERSONNAL "bookmark_ag"

/***************************************** NOTEBOOK ************************************/

#define ICO_NOTEBOOK_CLOSE "tab_close"
#define ICO_NOTEBOOK_CLOSE_2 "tab_close_2"
#define ICO_NOTEBOOK_CLOSE_3 "tab_close_3"
#define ICO_NOTEBOOK_PROGRESS "load_indicator.gif"
#define ICO_NOTEBOOK_CLOCK "clock"


/*************************************** TREE POPUP ************************************/

#define ICO_POPUP_DIR "popup_dir"
#define ICO_POPUP_DOWNLOAD "popup_upload"
#define ICO_POPUP_UPLOAD "popup_download"
#define ICO_POPUP_REFRESH "popup_refresh"
#define ICO_POPUP_DELETESHORTCUT "popup_delete_shortcut"
#define ICO_POPUP_MODIFYSHORTCUT "popup_modify_shortcut"
#define ICO_POPUP_PLAY "popup_play"
#define ICO_POPUP_DEFINESHORTCUT "bookmark_ag"
#define ICO_POPUP_PROPERTIES "file_properties"

/************************************** FILE PROPERTY DIALOG ************************************/

#define ICO_FPROPERTY_AUDIO "property_audio"


/*************************************** CLIPBOARD ************************************/

#define ICO_CLIPBOARD "clipboard"
#define ICO_CLIPBOARD_IN "clipboard_in"
#define ICO_CLIPBOARD_OUT "clipboard_out"
#define ICO_CLIPBOARD_SORT_ASC "sort_increase"
#define ICO_CLIPBOARD_SORT_DESC "sort_decrease"


/*************************************** TEXT UP TOOLBAR ************************************/

#define ICO_TAG_DISPLAY "display_tag"
#define ICO_TAG_DISPLAY_DISABLED "display_tag_disabled"
#define ICO_SYNC_TWS "sync_tws"
#define ICO_SYNC_SWT "sync_swt"
#define ICO_SYNC_TWS_DISABLED "sync_tws_disabled"
#define ICO_SYNC_SWT_DISABLED "sync_swt_disabled"
#define ICO_HIGHLIGHT "hilight"
#define ICO_HIGHLIGHT_DISABLED "hilight_disabled"
#define ICO_FILEMODE_BROWSE "browse_mode"
#define ICO_FILEMODE_EDIT "edit_mode"
#define ICO_DISPLAY_UNIQUESCREEN "screen_one"
#define ICO_DISPLAY_TWOSCREEN "screen_two"

#define ICO_COMBOLANGUAGE_ICON "kbd"
#define ICO_EDITOR_BACKGROUND_ENABLE "background_enable"
#define ICO_EDITOR_BACKGROUND_DISABLE "background_disable"

/*************************************** MEDIA ICON ************************************/
#define ICO_MEDIA_CLIPBOARD "clipboard"

#define ICO_ARROW_UP "arrow_up"
#define ICO_METADATA_STATS "stats"
#define ICO_METADATA_LARGETEXT "add"

#define ICO_KEYFRAME_DELETE "checkedbox"

#define ICO_FRAMEBROWSER_FRAMEERROR "frame_error"

/*************************************** CHECKER ICON ************************************/

#define ICO_CHK_INFO 			"chk_info"
#define ICO_CHK_WARN 			"chk_warning"
#define ICO_CHK_ERROR 			"errorAG2"


/*************************************** TOOL LAUNCHER ICON ************************************/

#define ICO_TOOL_LAUNCHER 		"external_tools"
#define ICO_TOOL_LAUNCHER_ON 	"led_green"
#define ICO_TOOL_LAUNCHER_OFF 	"led_grey"
#define ICO_TOOL_LAUNCHER_KO 	"led_red"

#define ICO_EXPAND				"expand"
#define ICO_EXPAND_LEFT			"expand_left"
#define ICO_EXPAND_LEFT_OVER	"expand_left_over"
#define ICO_EXPAND_LEFT_PRESS	"expand_left_press"
#define ICO_EXPAND_RIGHT		"expand_right"
#define ICO_EXPAND_RIGHT_OVER	"expand_right_over"
#define ICO_EXPAND_RIGHT_PRESS	"expand_right_press"


/**
 * @class 		Icons
 * @ingroup		Common
 *
 * Useful method for icons.\n
 * @remarks		Only single static method, should be moved into Common/widgets
 */
class Icons
{
	public:
		/**
		 * Set icon in the presentation bar of the window
		 * @param win		Impacted window
		 * @param icon		Icon defined name
		 * @param size		Icon size
		 */
		static void set_window_icon(Gtk::Window* win, const Glib::ustring& icon, int size) ;

		/**
		 * Gets a pixbuf with the corresponding icon
		 * @param 		icon		Icon string code
		 * @param 		size		Size needed
		 * @param[out]	result		True for success, false for failure
		 * @return					Reference on the corresponding icon (NULL if failure)
		 */
		static Glib::RefPtr<Gdk::Pixbuf> create_pixbuf(const Glib::ustring& icon, int size, bool& result) ;
};


} //namespace

#endif
