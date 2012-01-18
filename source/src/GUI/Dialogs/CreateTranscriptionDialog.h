/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef CREATETRANSCRIPTIONDIALOG_H_
#define CREATETRANSCRIPTIONDIALOG_H_

#include "Common/globals.h"
#include <gtkmm.h>
#include "Common/util/Utils.h"
#include "Common/icons/IcoPackButton.h"
#include "Common/icons/Icons.h"

namespace tag {
/**
 * @class 		CreateTranscriptionDialog
 * @ingroup		GUI
 * Basic dialog for creating a new transcription
 *
 * @note could be used too for file selection at opening
 */
class CreateTranscriptionDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param audio_paths	vector withh all audio files already selected for the
		 * 						new transcription
		 * @param audio_ext		audio file extension allowed
		 * @return
		 */
		CreateTranscriptionDialog(std::vector<Glib::ustring>& audio_paths, const std::vector<Glib::ustring>& audio_ext) ;
		virtual ~CreateTranscriptionDialog() ;

		/**
		 * Get all chosen files
		 * @return		vector containing all audio selected files
		 */
		std::vector<Glib::ustring> get_result() {return audio_files; }

	private:
		int current_number ;

		class AudioLine : public Gtk::HBox
		{
			public:
				AudioLine(Glib::ustring path, int number,
							const std::vector<Glib::ustring>& extensions, Glib::ustring& _last_selected) ;
				virtual ~AudioLine() {} ;

				void on_file_chosen() ;

				Gtk::Label number_label ;
				Gtk::Entry entry ;
				Gtk::FileChooserButton* chooser ;
				Glib::ustring& last_selected ;
		} ;

		Gtk::HBox title_box ;
			Gtk::Label title_label ;

		//> Dialog Action Area
		IcoPackButton button_add ;
		Gtk::Button* button_cancel ;
		Gtk::Button* button_apply ;

		std::vector<AudioLine*> lines ;
		const std::vector<Glib::ustring>& audio_extensions ;
		std::vector<Glib::ustring>&	 audio_files ;
		Glib::ustring last_selected ;

		void add_audio(Glib::ustring path) ;
		void add_audio_file() ;
		virtual void on_response(int id) ;
} ;

} //namespace

#endif /*CREATETRANSCRIPTIONDIALOG_H_*/
