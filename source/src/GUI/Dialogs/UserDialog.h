/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef USERDIALOG_H_
#define USERDIALOG_H_

#include <gtkmm.h>
#include "Configuration.h"

namespace tag {
/**
 * @class 		UserDialog
 * @ingroup		GUI
 *
 * Dialog used for getting a user name when launching application for the
 * first time.
 *
 */
class UserDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param config	Pointer on Configuration object
		 * @param save		True for saving name in configuration, false otherwise
		 */
		UserDialog(Configuration* config, bool save);
		virtual ~UserDialog();

		/**
		 * Accessor to the selected identifier
		 * @return			The selected identifier
		 */
		Glib::ustring get_identifier() ;

		/**
		 *	Emit signalIdentified signal.
		 *	This signal is emitted when the identification has been successful
		 * @note		When the user cancel the operation, the main application close.
		 * @return		Corresponding signal
		 */
		sigc::signal<void,bool>& signalIdentified() { return m_signalIdentified; }


	private :
		Configuration* config ;
		/* wether or not should save username in file parameters, or only set it on map */
		/* (false for external saving */
		bool save ;

		Gtk::Label blank1 ;
		Gtk::Label blank2 ;
		Gtk::Label blank3 ;

		Gtk::Alignment general_align ;
		Gtk::HBox general_hbox ;
		Gtk::Image general_image ;
		Gtk::Label	general_label ;
		Gtk::HSeparator sep ;

		Gtk::Alignment table_align ;
		Gtk::Table* table ;
			Gtk::Label login_label ;
			Gtk::Entry login_entry ;
			Gtk::Alignment login_align ;

		void set_widget() ;

		sigc::signal<void,bool> m_signalIdentified ;
		void on_response(int response) ;
		bool on_key_press_event(GdkEventKey* event) ;
};

} //namespace

#endif /*USERDIALOG_H_*/
