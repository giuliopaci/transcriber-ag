/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file 	IcoPackMenuToolButton.h
 */

#ifndef ICOPACKMENUTOOLBUTTON_H_
#define ICOPACKMENUTOOLBUTTON_H_

#include <gtkmm/menutoolbutton.h>
#include "Common/icons/IcoPackImage.h"
#include "IcoPackElement.h"

namespace tag {

/**
 * @class 		IcoPackMenuToolButton
 * @ingroup		Common
 *
 * Basic wrapper around Gtk::MenuToolButton for providing an easy set icon method.\n
 * Uses the TranscriberAG Gtk::Theme.
 */
class IcoPackMenuToolButton : public IcoPackElement, public Gtk::MenuToolButton
{
	public:
		/**
		 * Constructor
		 */
		IcoPackMenuToolButton();
		virtual ~IcoPackMenuToolButton();

		/*** Parent method ***/
		int set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip) { set_icon(icon, label, size, tooltip, "") ; }
		int set_icon(const Gtk::StockID& stock_id, const Glib::ustring& label, int size, const Glib::ustring& tooltip) { set_icon(stock_id, label, size, tooltip, "") ; }
		int change_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;
		void set_tip(const Glib::ustring& tip) ;

		/**
		 * Sets the button appearance.\n
		 * Set default values if no given ones.
		 * @param icon			Defined name of the icon that is displayed
		 * @param label			Button label
		 * @param size			Button icon size
		 * @param tooltip		Button tooltip
		 * @param a_tooltip		Arrow tooltip
		 * @return				True if successful, False if some theme error occurred
		 */
		int set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip, const Glib::ustring& a_tooltip) ;

		/**
		 * Sets the button appearance.\n
		 * Set default values if no given ones.
		 * @param stock_id			Gtk stock id
		 * @param label				Button label
		 * @param size				Button icon size
		 * @param tooltip			Button tooltip
		 * @param _arrow_tooltip 	Arrow tooltip
		 * @return					True if successful, False if some theme error occurred
		 */
		int set_icon(const Gtk::StockID& stock_id, const Glib::ustring& label, int size, const Glib::ustring& tooltip, const Glib::ustring& _arrow_tooltip)  ;

		/**
		 * @return 		Pointer on the menu used by the component
		 */
		Gtk::Menu* get_menu() { return &menu ; }

		/**
		 * Activates the menu
		 */
		void activate_menu() { set_menu(menu) ; menu.show_all() ; }

		/**
		 * Set sensitive the given menu options
		 * @param name			Name passed when the menu options had been appended.\n
		 * 						Empty string for all menu options.
		 * @param sensitive		True for sensitive, False otherwise.
		 */
		void setSensitiveItem(std::string name, bool sensitive) ;

		/**
		 * Set the given menu option as active
		 * @param name			Name passed when the menu options had been appended.\n
		 * 						Empty string for all menu options.
		 */
		void activateItem(std::string name) ;

		/**
		 * Select the given menu option.
		 * @param name			Name passed when the menu options had been appended.\n
		 * 						Empty string for all menu options.
		 */
		void selectItem(std::string name) ;

		/**
		 * Apprends a new menu option
		 * @param name			Internal name used for doing some action on the menu item
		 * @param display		Text to display
		 * @return				Pointer on the newly created menu item, NULL if fails or if an item
		 * 						with same internal name already exists
		 */
		Gtk::RadioMenuItem* appendItem(std::string name, std::string display) ;

	private:

		// image management
		IcoPackImage image ;
		Gtk::Tooltips arrow_tooltip ;

		// radio menu behavior
		Gtk::Menu menu ;
		std::map<std::string, Gtk::RadioMenuItem*> items ;
		Gtk::RadioButtonGroup group ;
};

} //namespace

#endif /*ICOPACKMENUTOOLBUTTON_H_*/
