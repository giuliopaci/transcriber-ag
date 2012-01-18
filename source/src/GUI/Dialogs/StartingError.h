/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef STARTINGERROR_H_
#define STARTINGERROR_H_

#include <gtkmm.h>


namespace tag {
/**
 * @class 		StartingError
 * @ingroup		GUI
 * Basic dialog for displaying launching error
 * @deprecated		Old class, shouldn't be used anymore (generic class could be used for that purpose)
 */
class StartingError : public Gtk::Window
{
	public:
		/**
		 * Constructor
		 * @param message	Error message to display
		 */
		StartingError(Glib::ustring message);
		virtual ~StartingError();

	private:

		void on_close() ;

		Gtk::VBox vbox ;
		Gtk::Image image ;
		Gtk::Label label ;
		Gtk::Button button ;

		Gtk::Label blank1 ;
		Gtk::Label blank2 ;
		Gtk::Label blank3 ;
		Gtk::HSeparator sep ;
};

} //namespace

#endif /*STARTINGERROR_H_*/
