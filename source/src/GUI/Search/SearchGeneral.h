/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef SEARCHGENERAL_H_
#define SEARCHGENERAL_H_

#include "AnnotationEditor/AnnotationEditor.h"
#include "AnnotationEditor/AnnotationView.h"
#include "Common/icons/IcoPackButton.h"
#include "Common/widgets/ComboEntry_mod.h"
#include "Common/VersionInfo.h"

namespace tag {

/**
 * @class 	SearchGeneral
 * @ingroup	GUI
 *
 * General mechanism of search widgets.
 * Used by text search feature (dialog and toolbar) and annotation search feature
 *
 */
class SearchGeneral
{
	public:
		/**
		 *	Constructor
		 */
		SearchGeneral();
		virtual ~SearchGeneral();

		/* controlling methods */
		/**
		 * Connect the search widget to the given AnnotationEditor
		 * @param editor	Pointer on the AnnotationEditor that will use the
		 * 					search function
		 */
		virtual void init(AnnotationEditor* editor) = 0;

		/**
		 * Disconnect the previous AnnotationEditor and connect the new given one
		 * @param editor	Pointer on the AnnotationEditor that will use the
		 * 					search function
		 */
		virtual void switch_file(AnnotationEditor* editor) = 0 ;

		/**
		 * Launch a forward search
		 */
		virtual void on_search_fw() = 0 ;

		/**
		 * Launch a backward search
		 */
		virtual void on_search_bk() = 0 ;

		/**
		 * Close the search widget
		 * @note signal disconnection, search Gtk::TextTag cleaning, ...
		 */
		virtual void close() = 0 ;

		/**
		 * Return the status of the search component
		 * @return	True if active, false otherwise
		 */
		bool is_active() {return active ; }

	protected:

		Gtk::Tooltips tooltip ;

		//> ------------------------ INTERNAL

		/* external text and data */
		/**
		 * @var external_editor
		 * Pointer on the AnnotationEditor that's using the search component
		 */
		AnnotationEditor* external_editor ;

		/**
		 * @var iLang
		 * Pointer on the InputLanguage of the external_editor
		 */
		tag::InputLanguage* iLang ;

		/**
		 * @var bak
		 * Indicates search direction (backward for true, forward otherwise)
		 */
		bool bak  ;

		/**
		 * @var active
		 * Indicates if the component is active or not
		 */
		bool active ;

		/**
		 * @var hasSelection
		 * External buffer selection state
		 */
		bool hasSelection ;

		/**
		 * @var lock_cursor
		 * Flag for locking the cursor position  treatment while beeing used in some process
		 */
		bool lock_cursor ;

		/**
		 * @var found
		 * Status of the research (true if at least one occurrence has been found)
		 */
		bool found ;

		/**
		 * @var count
		 * Counter for the number of occurrences
		 */
		int count ;

		/**
		 * @var dialogTag_occ
		 * Gtk::TextTag used for flagging the matching occurrences
		 */
		Glib::RefPtr<Gtk::TextTag> dialogTag_occ ;

		/**
		 * @var dialogTag_sel
		 * Gtk::TextTag used for flagging the line scope if the scope search has
		 * been set to selected lines
		 */
		Glib::RefPtr<Gtk::TextTag> dialogTag_sel ;

		Glib::RefPtr<Gtk::TextBuffer::Mark> mark_current ; 		/**< Current search position mark */
		Glib::RefPtr<Gtk::TextBuffer::Mark>	mark_start_occur ;	/**< Current occurrence start mark */
		Glib::RefPtr<Gtk::TextBuffer::Mark> mark_end_occur ;	/**< Current occurrence end mark */
		Glib::RefPtr<Gtk::TextBuffer::Mark>	mark_start_search ;	/**< Current search scope start mark */
		Glib::RefPtr<Gtk::TextBuffer::Mark> mark_end_search ;	/**< Current search scope end mark */
		Gtk::TextBuffer::iterator current ;						/**< Current search position iterator */
		Gtk::TextBuffer::iterator start_search ;				/**< Current occurrence start iterator */
		Gtk::TextBuffer::iterator end_search ;					/**< Current occurrence end iterator */
		Gtk::TextBuffer::iterator start_occur ;					/**< Current search scope start iterator */
		Gtk::TextBuffer::iterator end_occur ;					/**< Current search scope end iterator */

		sigc::connection external_signal_mark_set ;				/**< Connection to signal_mark_set of the external editor */
		sigc::connection external_signal_buffer_changed ;		/**< Connection to signal_changed of the external buffer */
		sigc::connection connection_changeActiveView	;		/**< Connection to signalChangeActiveView signal of the external editor */

		Gtk::Label label_inputLanguage ; 	/**< Label for the InputLanguage ComboBoxText */
		Gtk::ComboBoxText combo_language ; 	/**< ComboBoxText for changing of InputLangue */

		Gtk::ComboBoxText combo_searchMode ; 	/**< ComboBoxText for changing of search mode */

		/**
		 * Save iterator position into mark buffer
		 * (because iterator are invalidated when the buffer changed)
		 * @param mode		Indicates which iterator to save\n
		 * 					0: all
		 * 					1: start/end search
		 * 					2: star/end occur
		 * 					3: current
		 */
		void save_state(int mode=0) ;

		/**
		 * Load iterator position from mark buffer
		 */
		void load_state() ;

		/**
		 * Remove the last search tag applied
		 */
		void removeSelectionTag() ;

		/**
		 * Implementation of the Gtk callback
		 * @param response		Gtk dialog return code
		 */
		void on_response(int response) ;

		/**
		 * Method for displaying information
		 * @param text		Text to be displayed
		 */
		virtual void display_info(Glib::ustring text)=0 ;

		/**
		 * Method for displaying information
		 * @param text		Question text to be displayed
		 * @return
		 */
		virtual int display_question(Glib::ustring text)=0 ;

		/**
		 * Callback to use at an AnnotationView change
		 */
		virtual void on_change_view()=0 ;

		/**
		 * Callback to use at an AnnotationView change
		 */
		virtual void on_buffer_changed() = 0 ;

		/**
		 *  Method to prepare the combo language
		 */
		virtual void set_combo_language() = 0 ;

		/**
		 * Callback to use at InputLanguage change
		 */
		virtual void on_change_language() = 0 ;

		/**
		 * Method to use when the search ends
		 */
		virtual void search_end()=0 ;

		/**
		 * Actualize search component when selection mode change
		 * @param mode		True if the scope use a selection, false otherwise
		 */
		virtual void selection_mode(bool mode)=0 ;

		/**
		 * Callback to use when the cursor position changes in the external editor
		 * @param it	Pointer on new cursor iterator in the external buffer
		 * @param mark  Pointer on new cursor mark in the external buffer
		 */
		virtual void on_change_cursor(const Gtk::TextBuffer::iterator& it, const Glib::RefPtr<Gtk::TextBuffer::Mark >& mark) = 0 ;

		/**
		 * Accessor to the external buffer
		 * @return		Pointer on the AnnotationBuffer
		 */
		 Glib::RefPtr<AnnotationBuffer> get_external_buffer() {
			return external_editor->getActiveView()->getBuffer() ;
		}

		/**
		 * Accessor to the external view
		 * @return		Pointer on the external AnnotationView
		 */
		AnnotationView* get_external_view() {
			return external_editor->getActiveView() ;
		}

		/**
		 * Accessor to the external model
		 * @return		Pointer on the external DataModel
		 */
		DataModel* get_external_model() {
			return external_editor->getDataModelPtr() ;
		}
} ;

}

#endif /* SEARCHGENERAL_H_ */
