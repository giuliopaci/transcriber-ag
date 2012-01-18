/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file 	IcoPackElement.h
 */

#ifndef ICOPACKELEMENT_H_
#define ICOPACKELEMENT_H_

#include <gtkmm.h>
#include <gtkmm/icontheme.h>
#include "Common/icons/IcoPackImage.h"
namespace tag {

/**
 * @class 		IcoPackElement
 * @ingroup		Common
 *
 * Basis abstract class for all IcoPackElement
 */
class IcoPackElement
{
	public:
		IcoPackElement();
		virtual ~IcoPackElement();

		/**
		 * Sets the button appearance.\n
		 * Set default values if no given ones.
		 * @param icon		Defined name of the icon that is displayed
		 * @param label		Button label
		 * @param size		Button icon size
		 * @param tooltip	Button tooltip
		 * @return			True if successful, False if some theme error occurred
		 */
		virtual int set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip) = 0 ;

		/**
		 * Sets the button appearance.\n
		 * Set default values if no given ones.
		 * @param stock_id	Stock_id to be used
		 * @param label		Button label
		 * @param size		Button icon size
		 * @param tooltip	Button tooltip
		 * @return			True if successful, False if some theme error occurred
		 */
		virtual int set_icon(const Gtk::StockID& stock_id, const Glib::ustring& label, int size, const Glib::ustring& tooltip) = 0 ;

		/**
		 * Resets the button appearance.
		 * @param icon		Defined name of the icon that is displayed
		 * @param label		Button label
		 * @param size		Button icon size
		 * @param tooltip	Button tooltip
		 * @return			True if successful, False if some theme error occurred
		 */
		virtual int change_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip) = 0 ;

		/**
		 * Sets the image tooltip
		 * @param tip		Text to display in the tooltip
		 */
		virtual void set_tip(const Glib::ustring& tip) = 0 ;

	protected :
		IcoPackImage image ;			/**< Displayed image */
		Gtk::Tooltips tooltip ;		/**< Tooltip */
};

} //namespace

#endif /* ICOPACKELEMENT_H_ */
