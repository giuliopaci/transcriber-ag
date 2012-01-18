/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file 	IcoPackToolButton.h
 */

#ifndef ICOPACKTOOLBUTTON_H_
#define ICOPACKTOOLBUTTON_H_

#include "IcoPackElement.h"

namespace tag {

/**
 * @class 		IcoPackToolButton
 * @ingroup		Common
 *
 * Basic wrapper around Gtk::ToolButton for providing an easy set icon method.\n
 * Uses the TranscriberAG Gtk::Theme.
 */
class IcoPackToolButton : public IcoPackElement, public Gtk::ToolButton
{
	public:
		IcoPackToolButton();
		virtual ~IcoPackToolButton();

		/**
		 * Changes icon
		 * @param stock_id	Gtk stock
		 * @param label		Label
		 * @param size		Icon size
		 * @param tooltip	Icon tooltip
		 * @return			1 for success, 0 for failure
		 */
		int change_icon(const Gtk::StockID& stock_id, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;

		/*** Parent method ***/
		int set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;
		int set_icon(const Gtk::StockID& stock_id, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;
		int change_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;
		void set_tip(const Glib::ustring& tip) ;
} ;

} //namespace

#endif /*ICOPACKTOOLBUTTON_H_*/
