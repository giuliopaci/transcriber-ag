/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef EXPLORER_POPUP_H_
#define EXPLORER_POPUP_H_

#include "Common/globals.h"
#include <gtkmm.h>

namespace tag {

#define POPUP_TREE_PLAY _("Play")
#define POPUP_TREE_NEW_DIR _("New Folder")
#define POPUP_TREE_RENAME _("Rename")
#define POPUP_TREE_SUPPRESS _("Delete")
#define POPUP_TREE_IMPORT_IN _("Import in current file")
#define POPUP_TREE_IMPORT _("Import")
#define POPUP_TREE_OPEN _("Open")
#define POPUP_TREE_PROPERTY _("Properties")
#define POPUP_TREE_COPY _("Copy")
#define POPUP_TREE_PASTE _("Paste")
#define POPUP_TREE_CUT _("Cut")
#define POPUP_TREE_DEFINESHORTCUT _("Add to shortcut")

#define POPUP_TREE_CREATE_SINGLETRANSCRIPTION _("Create new transcription")
#define POPUP_TREE_CREATE_MULTITRANSCRIPTION _("Create new multi-chanel transcription")

#define POPUP_FTP_DOWNLOAD _("Download")
#define POPUP_FTP_UPLOAD _("Upload")

#define POPUP_TREE_CHANGESHORTCUT _("Modify shortcut")
#define POPUP_TREE_DELETESHORTCUT _("Delete shortcut")
#define POPUP_TREE_REFRESHDIR _("Refresh")
#define POPUP_TREE_REFRESHFTPDIR _("Actualize")

#define POPUP_TREATMENT_UP _("Increase priority")
#define POPUP_TREATMENT_DOWN _("Decrease priority")
#define POPUP_TREATMENT_PAUSE _("Interrupt all treatments")
#define POPUP_TREATMENT_CANCEL _("Cancel all treatments")

#define POPUP_TREATMENT_CANCEL _("Cancel all treatments")

/**
 * @class 		Explorer_popup
 * @ingroup		GUI
 * Static methods for some file operation specific to TranscriberAG GUI
 *
 */
class Explorer_popup : public Gtk::Menu
{
	public:
		/**
		 * Constructor
		 */
		Explorer_popup();
		virtual ~Explorer_popup();

		/**
		 * Prepare popup corresponding to given mode
		 * @param mode		Popup mode:\n
		 * 					1: not supported file\n
		 * 					1: directory\n
		 * 					2: system root directory\n
		 * 					3: personnal root directory\n
		 * 					4: for media file\n
		 * 					5: for annotation file\n
		 */
		void prepare_explorer_popup(int mode) ;

		/**
		 * @param mode		<em>deprecated</em>
		 * @deprecated		Feature not used anymore
		 */
		void prepare_treatment_popup(int mode) ;

		/**
		 * Activate paste action in menu (sensitive/unsensitive)
		 * @param value		True for activate, false for deactivate
		 */
		void enable_popup_paste(bool value) ;

		/**
		 * Insert an entry in popup menu
		 * @param item		Gtk::MenuItem to be inserted
		 * @param image		Gtk::Image to be displayed (or NULL if none)
		 * @param name		Name of the new entry
		 */
		void insert_item(Gtk::MenuItem* item, Gtk::Image* image, Glib::ustring name) ;

		/**
		 * Wrapper for easy creating image
		 * @param ico_path		TranscriberAG defined icon name
		 * @return				Newly created image
		 * @note				See Icons class for defined icon names
		 */
		Gtk::Image* create_image(Glib::ustring ico_path) ;

	private:
		int indice ;
		std::vector<Gtk::MenuItem*> itemList ;
		std::map<Glib::ustring, int> map_indice ;
		std::map<int,Gtk::Image*> map_image ;
} ;

} //namespace

#endif /*EXPLORER_POPUP_H_*/
