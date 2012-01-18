/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef ABOUTDIALOG_H_
#define ABOUTDIALOG_H_

#include <gtkmm.h>
#include "Common/globals.h"
#include "Common/icons/IcoPackImage.h"

namespace tag {
/**
 * @class 			AboutDialog
 * @ingroup			GUI
 * Basic dialog for displaying the about text of TranscriberAG
 *
 */
class AboutDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param image		Image to be displayed at the right of the dialog
		 * @param title		Title of the dialog
		 * @param icon		Icon of the dialog (top left presentation frame of the window)
		 * @param text		Content of the about dialog
		 * @return
		 */
		AboutDialog(const Glib::ustring& transcriber_image, const Glib::ustring& title, const Glib::ustring& transcriber_icon,
								const Glib::ustring& text, const Glib::ustring& under) ;
		virtual ~AboutDialog();

	private:
		Gtk::HBox hbox_main ;
		IcoPackImage image ;
		Gtk::TextView* tview ;
		Glib::RefPtr<Gtk::TextBuffer> buffer ;

		Gtk::HBox hbox_buttons ;
		Gtk::VBox vbox_right ;
		Gtk::Label BLANK1 ;
		Gtk::Label BLANK2 ;
		Gtk::Label under_label ;

		void set_text(Glib::ustring txt) ;
};

} //namespace

#endif /*ABOUTDIALOG_H_*/
