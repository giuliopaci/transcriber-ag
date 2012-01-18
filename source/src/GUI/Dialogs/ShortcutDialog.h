/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef SHORTCUTDIALOG_H_
#define SHORTCUTDIALOG_H_

#include <gtkmm.h>
#include "Configuration.h"
#include "Common/globals.h"
#include "TreeManager.h"

namespace tag {
/**
 * @class 		ShortcutDialog
 * @ingroup		GUI
 * Dialog used for creating a new explorer shortcut.
 * Enables user to choose a directory target and a shortcut name.
 */
class ShortcutDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param number_tree	Tree indice defined and used in the TreeManager class
		 * @param lock_name		If set to true, it is not possible to set the shortcut name
		 */
		ShortcutDialog(int number_tree, bool lock_name);
		virtual ~ShortcutDialog();

		/**
		 * Accessor to the chosen shortcut path
		 * @return		Target directory path selected by the user
		 */
		Glib::ustring get_chosen_path() { return chosen_path ; }

		/**
		 * Accessor to the chosen name
		 * @return		Shortcut name selected by the user
		 */
		Glib::ustring get_chosen_display() { return chosen_display ; }

		/**
		 * Define the TreeManager the shortcut dialog is Pointerd to
		 * @param treeManager	Pointer on the current TreeManager
		 */
		void set_treeManager(TreeManager* treeManager) { manager = treeManager ;}

		/**
		 * Set default value for shortcut
		 * Used when the shortcut is created by right click on a Explorer_tree row
		 * @param path		Default target path
		 * @param display	Default name
		 */
		void set_default(Glib::ustring path, Glib::ustring display) ;

	private:
		bool lock_name ;
		int number_tree ;
		Glib::ustring chosen_path ;
		Glib::ustring chosen_display ;
		TreeManager* manager ;

		//WIDGETS
		Gtk::Table* table ;
		Gtk::Alignment table_align ;

		Gtk::Label BLANK1 ;
		Gtk::Label BLANK2 ;

		Gtk::HSeparator separ ;

		Gtk::FileChooserButton* button_chooser ;

		Gtk::Label path_label ;
		Gtk::Label display_label ;

		Gtk::Entry path_entry ;
		Gtk::Alignment path_align ;
		Gtk::Entry display_entry ;
		Gtk::Alignment display_align ;

		Gtk::Button* validate ;
		Gtk::Button* cancel ;

		Gtk::Alignment buttons_align ;
		Gtk::HBox buttons_box ;

		int check_values() ;
		void on_selection_changed() ;
		void on_cancel() ;
		void on_validate() ;
		void on_path_entry_signal_changed() ;
};

} //namespace

#endif /*SHORTCUTDIALOG_H_*/
