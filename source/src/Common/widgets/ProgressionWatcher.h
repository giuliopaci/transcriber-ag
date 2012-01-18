/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/**
*  @file	ProgressionWatcher.h
*/

#ifndef PROGRESSIONWATCHER_H_
#define PROGRESSIONWATCHER_H_

#include <gtkmm.h>
#include "Common/globals.h"
#include "Common/icons/IcoPackImage.h"

namespace tag {

/**
 * @class 	ProgressionWatcher
 * @ingroup	GUI
 * Dialog displaying task progression (firefox like - but more basic;) for
 * file operation (download, remove, paste, ...)
 */

class ProgressionWatcher : public Gtk::Window
{
	public:

		/**
		 * @class Entry
		 *
		 * Represents a task in the progression watcher
		 * Display a Gtk::ProgressBar and some string information
		 */
		class Entry : public Gtk::Frame
		{
			public:
				/**
				 * Constructor
				 * @param slabel	text displayed into the progress bar
				 * @param fpath		path of the file being treated
				 * @param legend	text displayed once the task is finished
				 * @return
				 */
				Entry(Glib::ustring slabel, Glib::ustring fpath, Glib::ustring legend="")
				{
					set_name("download_frame") ;
					set_shadow_type(Gtk::SHADOW_IN) ;
					finished = false ;
					eventBox.set_name("download_frame") ;
					add(eventBox) ;
					eventBox.add(box) ;
					box.add(bar) ;
					box.set_name("download_frame") ;
					align.add(label) ;
					align.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_LEFT, 0, 15) ;
					if (slabel!="") {
						box.add(align) ;
					}
					file_path = fpath ;
					bar.set_text(slabel) ;
					bar.set_ellipsize(Pango::ELLIPSIZE_END) ;
					if (legend.empty())
						label.set_label(_("Processing...")) ;
					else
						label.set_label(legend) ;
					label.set_name("italic_label") ;
				}
				virtual ~Entry(){};

				/**
				 * Accessor to the Gtk::ProgressBar used
				 * @return		Gtk::ProgressBar of the entry
				 */
				GtkProgressBar* get_pbar() { return (&bar)->gobj() ; }

				/**
				 * Accessor to the Gtk::ProgressBar used
				 * @return		Gtk::ProgressBar of the entry
				 */
				Gtk::ProgressBar* get_bar() { return &bar ; }

				/**
				 * Force the complete status of the PogressBar
				 */
				void complete_bar() { bar.set_fraction(1.0) ; }

				/**
				 * Stupid accessor for display element
				 * @return		HSeparator automatically added after each entry
				 */
				Gtk::HSeparator* get_separator() {return &separator;}

				/**
				 * Change status of the entry (once task is over)
				 * @param isDone	success (true) or fail (false)
				 * @param msg		text displayed once the task is finished
				 */
				void set_status(bool isDone, Glib::ustring msg)
				{
					label.set_label(msg) ;
					if (isDone)
						label.set_name("bold_label_green") ;
					else
						label.set_name("bold_label_red") ;
					finished=true ;
				}

				/**
				 * Indicates wether the task is over
				 * @return		true if the task is finished, false otherwise
				 */
				bool is_finished() {return finished ;}

			private:
				Gtk::VBox box ;
				Gtk::ProgressBar bar ;
				Gtk::Label label ;
				Gtk::HSeparator separator ;
				Glib::ustring file_path ;
				bool finished ;
				Gtk::Alignment align ;
				Gtk::EventBox eventBox ;
		} ;

		/**
		 * Constructor
		 * @param title			Title of the dialog
		 * @param image			Image displayed in the dialog header
		 * @param splashmode	If true, displays as splashscreen (no window decoration)
		 * @param top			Top window
		 */
		ProgressionWatcher(Glib::ustring title, Glib::ustring image, bool splashmode, Gtk::Window* top);
		virtual ~ProgressionWatcher();

		/**
		 * Add a new entry to the progressManager
		 * @param label		text displayed in the progress bar
		 * @param file		file concerned by the task
		 * @param legend	text displayed under the progress bar
		 * @param present	true for presenting the dialog on top of GUI, false otherwise
		 * @return			newly created entry
		 */
		Entry* add_entry(Glib::ustring label, Glib::ustring file, Glib::ustring legend="", bool present=true) ;

	private:
		Gtk::VBox vbox_gen ;
		Gtk::Alignment align_gen ;

			//buttons
			Gtk::Alignment align_buttons ;
			Gtk::HBox hbox_buttons ;
			Gtk::Button* b_clean ;
			Gtk::Button* b_close ;
			//panel
			Gtk::HBox presentation_box ;
				Gtk::Alignment align_title;
					Gtk::Label label_title;
				Gtk::Alignment align_image ;
					IcoPackImage gen_image ;
			Gtk::ScrolledWindow scrollW ;
			Gtk::VBox vbox_data ;

			Glib::ustring progressionOK ;
			Glib::ustring progressionKO ;

			std::vector<Entry*> entries ;

			void clean() ;
};

} //namespace

#endif /*PROGRESSIONWATCHER_H_*/
