/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		FileDialogs.h
 *  @brief 		Basic specific file selection dialogs
 */

#ifndef _HAVE_FILEDIALOGS_H
#define _HAVE_FILEDIALOGS_H

#include <gtkmm.h>


namespace dlg {

/**
* Calls file chooser to select tag-format annotation file
* @param 		 filename 				Default selected filename
* @param 		 load  					True if select for opening, else false
* @param[in,out] copyMediaFile 			1 or 2 for enabling user to copy media file
* 										(2 for default value to yes, 1 for default value to false),
* 										-1 for disabling the option.
* 										If set to 1 or 2, will receive the user choice.
* @param 		 top 					Parent toplevel window  / NULL ; if not null, dialog will be transient for parent toplevel
*/
Glib::ustring selectTAGFile(const Glib::ustring& filename, bool load, int& copyMediaFile, Gtk::Window* top);
//Glib::ustring selectTAGFile(const Glib::ustring& filename="", bool load=true, Gtk::Window* top=NULL);


/**
 * Calls file chooser to select path and format for export
 * @param 		filename 		Default selected filename
 * @param 		top parent 		Toplevel window  / NULL ; if not null, dialog will be transient for parent toplevel
 * @param[out] 	format			Chosen export format
 * @return
 */
Glib::ustring selectExportFile(const Glib::ustring& filename, Gtk::Window* top, Glib::ustring& format) ;

/**
*  Calls file chooser to select audio file for the given annotation file
 * @param[out] 	close			Will received the True value if user cancels action or close the dialog
 * @param 		filename		Default selected filename
 * @param 		title			Dialog title
 * @param 		top				Parent toplevel window  / NULL ; if not null, dialog will be transient for parent toplevel
 * @param 		force_mono		True for only enabling mono audio files, False otherwise
 * @param 		default_rep		Indicates a default path on which the dialog will be located to
 * @return						The selected audio file path, or empty value if none was selected
 */
Glib::ustring selectAudioFile(bool& close, const Glib::ustring& filename="", const Glib::ustring& title="", Gtk::Window* top=NULL, bool force_mono= false, const std::string& default_rep="");

/**
 * Calls file chooser to select video file for the given annotation file
 * @param[out] 	close			Will received the True value if user cancels action or close the dialog
 * @param 		filename		Default selected filename
 * @param 		title			Dialog title
 * @param 		top				Parent toplevel window  / NULL ; if not null, dialog will be transient for parent toplevel
 * @return						The selected video file path, or empty value if none was selected
 */
Glib::ustring selectVideoFile(bool& close, const Glib::ustring& filename, const Glib::ustring& title, Gtk::Window* top) ;

}

#endif /* _HAVE_FILEDIALOGS_H */
