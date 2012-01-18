/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		SaveAudioDialog.h
 */

#ifndef SAVEAUDIODIALOG
#define SAVEAUDIODIALOG

#include <gtkmm.h>

#include "Common/globals.h"
#include "Common/util/Utils.h"

namespace tag {
/**
 * @class SaveAudioDialog
 *
 * Dialogs used for extracting audio signal.\n
 * Called from a signal selection, this dialog enables to choose the path
 * of the file into which the selected signal portion will be saved.
 *
 */
class SaveAudioDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param audio_path		Default audio file path
		 * @param start				Extraction range start
		 * @param end				Extraction range end
		 */
		SaveAudioDialog(Glib::ustring audio_path, float start, float end) ;
		virtual ~SaveAudioDialog() ;

		/**
		 * Accessor to the path of the extracted file
		 * @return		Source audio file path
		 */
		Glib::ustring getAudioPath() { return m_audio_path ; }

		/**
		 * Accessor to the chosen destination audio file path
		 * @return		Destination audio file path
		 */
		Glib::ustring getSelectionPath() { return m_newPath ; }

	private:
		Glib::ustring m_audio_path  ;
		Glib::ustring m_newName ;
		Glib::ustring m_newPath ;
		float m_start ;
		float m_end ;

		Gtk::HBox hbox_title ;
			Gtk::Label label_title ;
			Gtk::Image* image ;

		Gtk::ScrolledWindow scrolledW_file_infos ;
			Gtk::HBox hbox_file_infos ;
				Gtk::Label label_file_infos ;
				Gtk::Label label_file_infos_value ;

		Gtk::HBox hbox_offsets_infos ;
			Gtk::Label label_offsets_infos ;
			Gtk::Label label_offsets_infos_value ;

		Gtk::HBox hbox_folder ;
			Gtk::Label label_folder ;
			Gtk::FileChooserButton chooser ;

		Gtk::HBox hbox_name ;
			Gtk::Label label_name ;
			Gtk::Entry entry_name ;

		Gtk::Alignment align_button ;
			Gtk::HBox hbox_button ;
				Gtk::Button* save_button ;
				Gtk::Button* cancel_button ;

		void on_button_clicked(bool save) ;
} ;

} //namespace

#endif /*SAVEAUDIODIALOG*/
