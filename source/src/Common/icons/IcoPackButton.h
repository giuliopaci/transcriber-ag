/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file 	IcoPackButton.h
 */

#ifndef ICOPACKBUTTON_H_
#define ICOPACKBUTTON_H_

#include "IcoPackElement.h"

namespace tag {

/**
 * @class 		IcoPackButton
 * @ingroup		Common
 *
 * Basic wrapper around Gtk::Button for providing an easy set icon method.\n
 * Uses the TranscriberAG Gtk::Theme.
 */
class IcoPackButton : public IcoPackElement, public Gtk::Button
{
	public:
		/**
		 * Constructor
		 */
		IcoPackButton();
		virtual ~IcoPackButton();

		/*** Parent method ***/
		int set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;
		int set_icon(const Gtk::StockID& stock_id, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;
		int change_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;
		void set_tip(const Glib::ustring& tip) ;
} ;

} //namespace

#endif /*ICOPACK_H_*/
