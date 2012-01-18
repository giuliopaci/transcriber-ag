/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file	ComboEntry_mod.h
 */

#ifndef COMBOENTRY_MOD_H_
#define COMBOENTRY_MOD_H_

#include <iostream>
#include <sstream>
#include <string>
#include <gtkmm.h>
#include <gtkmm/icontheme.h>
#include "Common/InputLanguage.h"

#define COMBO_ENTRY_MOD_ICON "myCombo_arrow"
#define COMBO_ENTRY_MOD_ICON_SIZE 12

namespace tag {
/**
* @class 		ComboEntry_mod
* @ingroup		Common
*
* Basic widget providing both Entry and ComboBoxText features.\n
* Enables the use of internal InputLanguage for writing in specific language.\n\n
*
* @note 	We could have used a ComboxBoxEntry but we needed to overwrite some entry's
* 			behavior in order to do some text processing.
*/
class ComboEntry_mod : public Gtk::Entry
{
	public:
		/**
		 * Constructor
		 */
		ComboEntry_mod();
		virtual ~ComboEntry_mod();

		/**
		 * Accessor to the arrow GUI element.\n
		 * The arrow enables the access to the combo list.
		 * @return		A pointer on the arrow button
		 */
		Gtk::Button* get_arrow() { return &arrow ; }

		/**
		 * Accessor to the menu GUI element.\n
		 * The menu simulates the combo list.
		 * @return
		 */
		Gtk::Menu* get_menu() { return &menu ; }

		/**
		 * Defines the InputLanguage that will be used for specific language.
		 * @param _iLang	Pointer on an InputLanguage
		 */
		void set_input_language(InputLanguage* _iLang) { iLang = _iLang ; }

		/**
		 * Adds the given string in the combo list
		 * @param	s 		New entry in the combo list
		 */
		void add_in_list(Glib::ustring s) ;

		/**
		 * Checks the existence of a string in the combo list.
		 * @param word		Word to be looked for in the combo list
		 * @return			True if the words is already in, False otherwise
		 */
		bool is_in_list(Glib::ustring word) ;

		/**
		 * Activates (raises and uses) or not the external IME (if some is used)
		 * @param activate		True for activating the external IME, False otherwise.
		 */
		void externalIMEcontrol(bool activate) ;

		/**
		 * Set the external IME statue
		 * @param value		True if the last InputLanguage set is an external IME.
		 */
		void setIMEstatus(bool value) { IME_on=value ; }

		/**
		 * Force the cursor display
		 * @bug		With our implementation the cursor doesn't seems to be visible when
		 * 			the entry gets the focus, so we did this (ugly ?) workaround
		 */
		void force_cursor() ;

		/**
		 * Removes all menu items
		 */
		void clearMenu() ;

	private:
		Gtk::Image image ;
		bool IME_on ;

		int m_x ;
		int m_y ;
		Gtk::Menu menu ;
		Gtk::Button arrow ;
		InputLanguage* iLang ;

		std::map<Glib::ustring, int> list ;

		void my_insert_text(const Glib::ustring& c) ;
		void onPopupMenuPosition(int& x, int& y, bool& push_in) ;
		void myPopulate() ;

		void on_list(Glib::ustring word) ;

		bool externalIMEcontrol_afterIdle(bool activate) ;

		virtual bool on_focus_in_event(GdkEventFocus* event) ;
		bool on_button_press_event(GdkEventButton* event) ;


		/**
		 * Redefinition of the on_key_press_event method of the Gtk::Entry
		 * @param event		GdkEventKey
		 * @return			True or False as all Gtk event callback
		 */
		bool on_key_press_event(GdkEventKey* event) ;
} ;

} //NAMESPACE

#endif /*COMBOENTRY_MOD_H_*/
