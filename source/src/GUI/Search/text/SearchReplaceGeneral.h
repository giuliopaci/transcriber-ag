/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef SEARCHREPLACEGENERAL_H_
#define SEARCHREPLACEGENERAL_H_

#include <gtkmm.h>
#include <iostream>
#include "../SearchGeneral.h"
#include "Common/icons/IcoPackImage.h"

namespace tag {
/**
 * @class 	SearchReplaceGeneral
 * @ingroup	GUI
 *
 * Implementation of the general mechanism for research in text
 * (deals with tags text too - except for replacing).
 * Used by toolbar mode (MiniSearch class) and dialog mode (SearchReplaceDialog class)
 *
 */
class InputLanguage ;

#define SIMLE_SCOPE_SELECTION_LIMIT 30

class SearchReplaceGeneral : public SearchGeneral
{
	public:
		/**
		 * Constructor
		 */
		SearchReplaceGeneral();
		virtual ~SearchReplaceGeneral();

		/* Abstract method implementation */
		void init(AnnotationEditor* editor) ;
		void close() ;
		void switch_file(AnnotationEditor* editor) ;
		bool is_active() {return active ; }
		void on_search_fw() ;
		void on_search_bk() ;

		/**
		 * Set the status activation
		 * @param value
		 */
		void set_active(bool value) { active=false ;}

		/**
		 * Initialize the scope search
		 */
		void check_initial_selection() ;

		/**
		 * Signal emitted when the search mode changes
		 * @return		The corresponding signal
		 */
		sigc::signal<void,int>& signalSearchModeChange() { return m_signalSearchModeChange; }

		/**
		 * Get the list of searched words
		 * @return		Vector containing all words searched
		 */
		std::vector<Glib::ustring> get_findList() {return findList ;}

		/**
		 * Set a list of woards to be displayed in the ComboBoxText
		 * @param l		Vector containing all words searched
		 */
		void set_findList(std::vector<Glib::ustring> l) ;

		/**
		 * Accessor to the external editor connected to the text search components
		 * @return		The external AnnotationEditor using the text search components
		 */
		AnnotationEditor* get_editor() { return external_editor ; }

		/**
		 * Accessor to the current searched word
		 * @return		The term currently searched
		 */
		Glib::ustring get_current_search() { return combo_search.get_text() ;}

		/**
		 * Set the search term
		 * @param value		The term to be search (displayed in the ComboBoxTex)
		 */
		void set_current_search(Glib::ustring value) { combo_search.set_text(value) ;}

		/**
		 * Search a word with insensitive case mode
		 * @param selection		the word to search in the text widget
		 * @param match_start		the current cursor's position mark
		 * @param match_end		end position of the searched word
		 * @return			true if the word is found else false
		 */
		bool backward_search_insensitive_case(Glib::ustring& selection, Gtk::TextBuffer::iterator& match_start, Gtk::TextBuffer::iterator& match_end);

		/**
		 * Search a word with insensitive case mode
		 * @param selection		the word to search in the text widget
		 * @param match_start		the current cursor's position mark
		 * @param match_end		end position of the searched word
		 * @return			true if the word is found else false
		 */
		bool forward_search_insensitive_case(Glib::ustring& selection, Gtk::TextBuffer::iterator& match_start, Gtk::TextBuffer::iterator& match_end);

		/**
		 * Defines the search option values
		 * @param searchMode	Text, Tags, or both
		 * @param cas			True for case sensitive, false otherwise
		 * @param wholeWord		True for whole word matching, false otherwise
		 * @return
		 */
		void setSearchOptions(int searchMode, bool cas, bool wholeWord) ;

		/**
		 * Gets the search option values
		 * @param[out] searchMode	Text, Tags, or both
		 * @param[out] cas			True for case sensitive, false otherwise
		 * @param[out] wholeWord	True for whole word matching, false otherwise
		 */
		void getSearchOptions(int& searchMode, bool& cas, bool& wholeWord) ;

		/**
		 * Set the focus of the text search components
		 */
		virtual void mySet_focus()=0 ;

		/**
		 * Hide the text search components
		 */
		virtual void myHide()=0 ;

	protected:
		typedef enum { TT, TEXT, TAGS } SearchPolicy;

		//> WIDGETS
		Gtk::Tooltips tooltip ;

		Gtk::RadioButton scope_all ;
		Gtk::RadioButton scope_selec ;
		Gtk::Label label_search ;
		ComboEntry_mod combo_search ;
		Gtk::Label label_replace ;
		ComboEntry_mod combo_replace ;
		IcoPackButton find_bk ;
		IcoPackButton find_fw ;
		Gtk::Button find_replace_bk ;
		Gtk::Button find_replace_fw ;
		Gtk::Button replace ;
		Gtk::Button replace_all ;

		IcoPackButton close_button ;
		IcoPackButton change_mode ;

		IcoPackImage search_in_progress ;	/**< search_in_progress gif */
		IcoPackImage search_static ;	/**< search_in_progress gif */


		Gtk::CheckButton cb_whole ; 	/**< Whole word option button */
		Gtk::CheckButton cb_case ;		/**< Case sensitive option button */
		Gtk::RadioButton rb_all ;		/**< All scope radio button */
		Gtk::RadioButton rb_sel ;		/**< Selection scope radio button */

		//> INTERNAL
		Glib::ustring _search ;			/**< Object of the research */
		Glib::ustring _replace ;		/**< Replacement term */

		int count_replace ;				/**< Number of replacement done in last research */
		int flag_uneditable ;			/**< Flag indicating the editability of the current position */

		/* mode of research */
		bool replace_all_mode ;			/**< Replace all words flag mode */
		bool  whole_word_mode ;			/**< Search only whole word flag mode */
		bool  case_sensitive_mode ;		/**< Search with case sensitive word flag mode */
		int	search_mode;				/**< Search in text, tags, text & tags */

		/* Stock all internal signals */
		sigc::connection connection_replace_all ;		/**< Connection to replace all button */
		sigc::connection connection_replace ;			/**< Connection to replace button */
		sigc::connection connection_find_fw ;			/**< Connection to find forward button */
		sigc::connection connection_find_bk ;			/**< Connection to find backward button */
		sigc::connection connection_close_button ;		/**< Connection to close button */
		sigc::connection connection_rb_all ;			/**< Connection to all scope radio button */
		sigc::connection connection_rb_sel ;			/**< Connection to selection scope radio button */
		sigc::connection connection_cb_case ;			/**< Connection to case sensitive check button */
		sigc::connection connection_cb_whole ;			/**< Connection to whole word checkbutton */
		sigc::connection connection_combo_replace ;		/**< Connection to replace combo entry */
		sigc::connection connection_combo_search ;		/**< Connection to search  combo entry */
		sigc::connection connection_change_mode	;		/**< Connection to change search mode button */

		/* list of searched words */
		std::vector<Glib::ustring> findList ;		/**< List of terms in search list */
		std::vector<Glib::ustring> replaceList ;	/**< List of terms in replace list */

		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> METHODS

		/**
		 * Replace last occurrence by the replacement term
		 * @param user_action	If true, replacement will be done inside a single
		 * 						user action (used for undo/redo mechanism by UndoableAnnotationView)
		 * @return				True if replacement ended successfully, false otherwise
		 */
		bool on_replace(bool user_action) ;

		/**
		 * Replace last occurrence by the replace term and search for the next one
		 * @param back			True for backward mode, False otherwise
		 */
		void on_replace_find(bool back) ;

		/**
		 * Loop on all external buffer and replace all matching occurrence with
		 * the replacement term
		 */
		void on_replace_all() ;

		/**
		 * Activate or deactivate the search buttons
		 * @param active	True for activating, false otherwise
		 */
		void active_search(bool active) ;

		/**
		 * Activate or deactivate the replace buttons
		 * @param active	True for activating, false otherwise
		 */
		void active_replace(bool active) ;

		/* Abstract method implementation */
		virtual void selection_mode(bool withSelection) ;

		/**
		 * @var m_signalSearchModeChange
		 * Signal emitted when the search mode change\n
		 * <b>int parameter:</b>\n
		 * 			 1 = from toolbar mode to dialog mode\n
		 * 			 0 = from dialog mode to toolbar mode\n
		 */
		sigc::signal<void,int> m_signalSearchModeChange ;

	private:

		/* internal*/
		bool add_in_list(std::vector<Glib::ustring>* vect, Glib::ustring name) ;
		void reset() ;
		void get_entries() ;

		/* options */
		void on_case_sensitive() ;
		void on_whole_word() ;
		void word_to_whole_word(bool reverse) ;
		bool is_whole_word() ;
		void on_scope_change(bool selection) ;
		void on_signal_changed() ;
		void on_change_cursor(const Gtk::TextBuffer::iterator& it, const Glib::RefPtr<Gtk::TextBuffer::Mark >& mark) ;
		void on_buffer_changed() ;
		void on_change_view() ;

		/* display */
		void select_occ() ;
		void remove_occ() ;

		void search_end() ;
		void enable_replace(bool mode) ;

		virtual void display_info(Glib::ustring text)=0 ;
		virtual int display_question(Glib::ustring text)=0 ;
		virtual void on_change_mode()=0 ;
		void on_change_language() ;
		void set_combo_language() ;
		void on_change_search_mode();
		void set_combo_search_mode() ;
		void prepareTag() ;
		bool searchMode(const int flag_editability, const bool whole_word_allowed) ;
};

} //namespace


#endif /*SEARCHREPLACEGENERAL_H_*/
