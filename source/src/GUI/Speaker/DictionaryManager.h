/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef DICTIONARYMANAGER_H_
#define DICTIONARYMANAGER_H_

#include "SpeakerDico_dialog.h"

#include "DataModel/speakers/Speaker.h"

#include "Common/Parameters.h"

#include "Editors/CommonEditor/AGEditor.h"

#include "Configuration.h"


namespace tag {

/**
 * @class 	DictionaryManager
 * @ingroup	GUI
 *
 * Dictionaries manager.
 */
class DictionaryManager
{
	public:
		/**
		 * Constructor
		 * @param win		Parent window
		 * @param config	Application configuration object
		 * @return
		 */
		DictionaryManager(Gtk::Window* win, Configuration* config);
		virtual ~DictionaryManager();

		/**
		 * editor: annotation editor
		 * forcedEditability: don't use default behaviour, and precise if dictionary must be editable or not
		 * 					0: non editable
		 * 					1: editable
		 * 					x: default behaviour
		 */
		SpeakerDico_dialog* new_dialog(AnnotationEditor* editor, int forcedEditability, bool modal, Gtk::Window* win);

		/**
		 * url: absolute path to dictionary file
		 * forcedEditability: don't use default behaviour, and precise if dictionary must be editable or not
		 * 					0: non editable
		 * 					1: editable
		 * 					x: default behaviour
		 */
		SpeakerDico_dialog* new_dialog(Glib::ustring url, int forcedEditability, bool modal, Gtk::Window* win) ;

		/**
		 * Checks whether the given local dictionary is already opened in a dialog
		 * @param filepath		Path of the file containing the local dictionary
		 * @return				True if already opened.
		 */
		bool is_opened(Glib::ustring filepath) ;

		/**
		 * Checks whether the global dictionary is already opened.
		 * @return				True if already opened.
		 */
		bool is_opened_global() ;

		/**
		 * Closes all opened dictionaries
		 * @return	True if all opened dictionaries were closed, false if at least
		 * 			one has to stay opened (modifications not saved)
		 */
		bool close_all() ;

		/**
		 * Displays dictionary on foreground
		 * @param path				Path to the dictionary
		 * @param id_to_select		Speaker identifiant to select (empty for none)
		 */
		void raiseDictionary(Glib::ustring path, Glib::ustring id_to_select) ;

		/**
		 * Checks whether an URL is configured for the dictionary, and downloads it
		 * if exits.
		 */
		int checkServerDictionary() ;

		/**
		 * Displays the speaker dictionary corresponding to the given editor.\n
		 * If already opened, raises it.
		 * @param edit		Editor
		 * @param id		If not empty, the dialog will select the speaker corresponding to the id.
		 * @param modal		True for modal, false otherwise
		 */
		void showLocalDictionary(AGEditor* edit, Glib::ustring id, bool modal) ;

		/**
		 * Displays the global speaker dictionary.\n
		 * If already opened, raises it.
		 */
		void showGlobalDictionary() ;

		/**
		 * Get drag and drop target
		 * @return		Drag and drop target
		 */
		Gtk::TargetEntry getDragAndDropTarget() ;

		/**
		 *  Signal sent when the user wants to raise other dictionary
		 */
		sigc::signal<void>& signalRaiseCurrentDictionary() { return m_signalRaiseCurrentDictionary ; }

	private:
		Configuration* configuration ;

		/** internal management **/
		std::vector<SpeakerDico_dialog*> dicos ;
		SpeakerDico_dialog* global_dictionary ;
		Gtk::Window* window ;

		/** DRAG'N DROP **/
		/* list of targetEntry to define drag and source identity */
		std::vector<Gtk::TargetEntry> dragdropTargetList ;
		Gtk::TargetEntry dragdropTarget ;
		SpeakerDico_dialog* dialog_src ;
		SpeakerDico_dialog* dialog_dest ;
		bool drag_initialised ;
		virtual void on_drag_data_get (const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& selection_data, guint info, guint time, SpeakerDico_dialog* dialog) ;
		virtual bool on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time, SpeakerDico_dialog* dialog);
		void on_replace_by_data_drop(SpeakerDico_dialog* dest) ;
		void setDragAndDropTarget(SpeakerDico_dialog* dialog) ;

		Speaker* speaker ;
		std::vector<Gtk::TreeIter> paths ;

		void on_close(SpeakerDico_dialog* dico) ;
		SpeakerDico_dialog* get_dictionary(Glib::ustring path) ;

		void onRaiseDictionary(Glib::ustring scope) ;

		//> signal to raise a dictionary
		sigc::signal<void> m_signalRaiseCurrentDictionary ;
};

} //namespace


#endif /*DICTIONARYMANAGER_H_*/
