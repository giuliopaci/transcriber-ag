/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef EXPLORER_MENU_H_
#define EXPLORER_MENU_H_

#include <gtkmm.h>

namespace tag {

/**
 * @class 		Explorer_menu
 * @ingroup		GUI
 *
 *	Wrapper for TranscriberAG
 */

class Explorer_menu
{
	public:

		/**
		 * Instantiate a new explorer_menu
		 */
		Explorer_menu();
		virtual ~Explorer_menu();

		/**
		 * Get UIManager corresponding to menu
		 * @return pointer on Gtk::UIManager
		 */
		Glib::RefPtr<Gtk::UIManager> get_mainMenu() ;

		/**
		 * Get action group corresponding to menu
		 * @return pointer on Gtk::ActionGroup
		 */
  		Glib::RefPtr<Gtk::ActionGroup> get_actionGroup() ;

  		/**
  		 * Get a menu widget
  		 * Wrapper for get_widget(s_menu) method of Gtk::UIManager
  		 * @param s_menu 	ui name of menu
  		 * @return 			corresponding widget
  		 */
		Gtk::Widget* get_widget_mainMenu(Glib::ustring s_menu) ;


	  /**
	   * Add a recent file action in recent file menu
	   * @param path 		path of file
	   * @param id 			Gtk::UIManager ui_merge_id of the new action
	   * @param ui_path 	Gtk::UIManager ui of the entry being created
	   * @param action 		Gtk::Action for the new entry
	   * @param slot 		callback for the action
	   * @return			false if same path already existed in menu, true if successfull
	   * @note 				Doesn't update order use
	   */
		bool add_recent_file(Glib::ustring path, Gtk::UIManager::ui_merge_id id, Glib::ustring ui_path,  Glib::RefPtr<Gtk::Action> action, const Gtk::Action::SlotActivate& slot) ;

		/**
		 * Add default action group to menu
	   	 * @param 	window window whose accelerate group will be linked with menu
	     */
		void set_menu_with_action(Gtk::Window* window) ;

		/**
		 * Set ui string to menu
	   	 * @param ui_info		ui string to be used by menu
	   	 * @param agName		"general" for general menu\n
	   	 * 						"signal" for signalMenu\n
	   	 * 						"annotate" for annotate menu\n
	   	 * 						"display" for display menu
	   	 * @return 				newly created Gtk::UIManager::ui_merge_id Id
	     */
		Gtk::UIManager::ui_merge_id add_ui(Glib::ustring ui_info, Glib::ustring agName) ;

		/**
		 * Remove an action from the ui string
		 * @param agName 		name of ui to be removed:\n
		 * 						"general" for general menu\n
	   	 * 						"signal" for signalMenu\n
	   	 * 						"annotate" for annotate menu\n
	   	 * 						"display" for display menu
		 */
		void remove_ui(Glib::ustring agName) ;

		/**
		 * Remove action group passed as parameter from menu
	   	 * @param actionG	action grou pto be removed
	   	 * @param check		check if the action group exists in action group list
	   	 * @note			using check parameter set to true is recommended (slower but more secure)
	     */
		void remove_actionGroup(Glib::RefPtr<Gtk::ActionGroup> actionG, bool check) ;

		/**
		 * Add action group passed as parameter in menu
		 * @param actionG	action group to be inserted
	   	 * @param win 		window for which accelerate group must be set. (NULL if no need)
	   	 * @param check 	check if action group doesn't already exist in action group list
	   	 * @note			using check parameter set to true is recommended (slower but more secure)
	   	 */
		void insert_actionGroup(Glib::RefPtr<Gtk::ActionGroup> actionG, Gtk::Window* win, bool check) ;

		/**
		 *	Save recent files information in file set in TranscriberAG configuration
		 * @param path		path of recent entries files
		 */
		void save(Glib::ustring path) ;

		/**
		 * Configure ui for no-opened-file mode or opened-file mode
		 * @param ui		menu ui string to use
		 * @param type		"default" for no-opened-file mode, "general" for opened-file
		 */
		void set_ui(Glib::ustring ui, Glib::ustring type) ;

		/**
		 * Get the menu ui string for no-opened-file or opened-file mode
		 * @param type		"default" for no-opened-file mode, "general" for opened-file
		 * @return			menu ui string for selected mode
		 */
		Glib::ustring get_ui(Glib::ustring type) ;

		/**
		 * Reset ui for first file mode or last file mode
		 * (remove old one(s) and set corresponding new one)
		 * @param mode		"first" when the first file is being opened in notebook,
		 * 					"last" when the last file is being closed in notebook
		 */
		void switch_ui(Glib::ustring mode) ;


	private:
		/* basic menu */
		Glib::RefPtr<Gtk::UIManager> mainMenu ;
  		Glib::RefPtr<Gtk::ActionGroup>  actionGroup ;
  		Gtk::Toolbar toolbar ;

		/* list to stock recent files uis and actions */
		std::vector< Glib::RefPtr<Gtk::Action> > recent_actions ;
		std::vector<Gtk::UIManager::ui_merge_id> recent_uis ;
		std::vector<Glib::ustring> recent_files ;

		/* All dynamic action group */
		bool active ;
		Glib::ustring default_ui_info ;
		Glib::ustring general_ui_info ;

		Gtk::UIManager::ui_merge_id default_ui_id ;
		Gtk::UIManager::ui_merge_id general_ui_id ;
		Gtk::UIManager::ui_merge_id signal_ui_id ;
		Gtk::UIManager::ui_merge_id annotate_ui_id ;
		Gtk::UIManager::ui_merge_id display_ui_id ;
		Gtk::UIManager::ui_merge_id video_ui_id ;
};

} //namespace


#endif /*EXPLORER_MENU_H_*/

