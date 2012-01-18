/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef EXPLORER_DIALOG_H_
#define EXPLORER_DIALOG_H_

#include <gtkmm.h>

namespace tag {
/**
 * @class 	Explorer_dialog
 * @ingroup GUI
 * Basic wrappers for dialog easy using
 */
class Explorer_dialog
{
	public:
		Explorer_dialog();
		virtual ~Explorer_dialog();

		/**
		 * Information dialog
		 * @param text		Text to be displayed
		 * @param w			Parent window
		 * @param modal		True for modal, false otherwise
		 */
		static void msg_dialog_info(Glib::ustring text, Gtk::Window* w, bool modal) ;

		/**
		 * Warning dialog
		 * @param text		Text to be displayed
		 * @param w			Parent window
		 * @param modal		True for modal, false otherwise
		 */
		static void msg_dialog_warning(Glib::ustring text, Gtk::Window* w, bool modal) ;

		/**
		 * Error dialog
		 * @param text		Text to be displayed
		 * @param w			Parent window
		 * @param modal		True for modal, false otherwise
		 */
		static void msg_dialog_error(Glib::ustring text, Gtk::Window* w, bool modal) ;

		/**
		 * Dialog for file renaming.
		 * Check some invalid character using <em>FileHelper::check_rename_filename</em> method.
		 * Loop until successful end or cancellation
		 * @param win				Parent window
		 * @param old_name_wt_ext	Old name (without extension)
		 * @return					Newly selected name or empty string if canceled
		 */
		static Glib::ustring msg_rename(Gtk::Window* win, Glib::ustring old_name_wt_ext);

		/**
		 * Yes / No dialog
		 * @param text			Text to be display (bold)
		 * @param win			Parent window
		 * @param modal			True for modal, false otherwise
		 * @param second_text	Secondary text displayed under main text
		 * @return				Gtk::RESPONSE_YES or Gtk::RESPONSE_NO
		 */
		static int msg_dialog_question(Glib::ustring text, Gtk::Window* win, bool modal, Glib::ustring second_text) ;

		/**
		 * Dialog used when opening an audio file.
		 * If a transcription file associated to the audio exists, the user
		 * can choose to open it or not.
		 * @param annot_exists	True if a transcription file associated to audio exists
		 * @param path			Path of the audio file
		 * @param win			Parent window
		 * @return				Gtk::RESPONSE_YES or Gtk::RESPONSE_NO
		 */
		static int msg_dialog_open_audio(bool annot_exists, Glib::ustring path, Gtk::Window* win) ;


	private:
		static int msg_dialog_cancel(Glib::ustring text, Gtk::Window* win, bool modal, Glib::ustring second_text, Gtk::Entry* entry) ;

};

} //namespace

#endif /*EXPLORER_DIALOG_H_*/
