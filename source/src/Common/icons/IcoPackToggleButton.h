/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file 	IcoPackToggleButton.h
 */

#ifndef ICOPACKTOGGLEBUTTON_H_
#define ICOPACKTOGGLEBUTTON_H_

#include "IcoPackElement.h"

namespace tag {

/**
 * @class 		IcoPackToggleButton
 * @ingroup		Common
 *
 * Basic wrapper around Gtk::ToggleButton for providing an easy set icon method.\n
 * Uses the TranscriberAG Gtk::Theme.
 */
class IcoPackToggleButton : public IcoPackElement, public Gtk::ToggleButton
{
	public:
		/**
		 * Constructor
		 */
		IcoPackToggleButton();
		virtual ~IcoPackToggleButton();

		/**
		 * Changes icon
		 * @param stock_id	Gtk stock
		 * @param label		Label
		 * @param size		Icon size
		 * @param tooltip	Icon tooltip
		 * @return			1 for success, 0 for failure
		 */
		int change_icon(const Gtk::StockID& stock_id, const Glib::ustring& label, int size, const Glib::ustring& tooltip) { return 1 ; }

		/*** Parent method ***/
		int set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;
		int set_icon(const Gtk::StockID& stock_id, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;
		int change_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip) { return 1 ; }
		void set_tip(const Glib::ustring& tip) ;

};

} //namespace

#endif /*ICOPACKTOGGLEBUTTON_H_*/
