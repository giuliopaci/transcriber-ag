/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef HIGHLIGHTGENERAL_H_
#define HIGHLIGHTGENERAL_H_

#include <gtkmm.h>
#include <iostream>

#include "../SearchGeneral.h"
#include "Common/icons/IcoPackImage.h"
#include "Editors/AnnotationEditor/AnnotationEditor.h"

namespace tag {
/**
 * @class 	HighlightGeneral
 * @ingroup	GUI
 *
 * Implementation of the general mechanism for research in text
 * (deals with tags text too - except for replacing).
 * Used by toolbar mode (MiniSearch class) and dialog mode (SearchReplaceDialog class)
 *
 */
class InputLanguage ;

#define SIMLE_SCOPE_SELECTION_LIMIT 30

class HighlightGeneral : public SearchGeneral
{
	public:
		/**
		 * Constructor
		 */
		HighlightGeneral();
		virtual ~HighlightGeneral();

		/* Abstract method implementation */
		void init(AnnotationEditor* editor) ;

		/**
		 * Highlight all matching terms
		 */
		void highlight() ;

	protected:
		string commandLineOffsetSegid ;
		std::vector<Glib::ustring> matches ;
		Glib::ustring _search ;
		bool whole_word_mode ;

		void highlightAllTerms(Glib::RefPtr<AnnotationBuffer> buffer) ;
		void reset_iterators(Glib::RefPtr<AnnotationBuffer> buffer) ;
		void on_search_fw(Glib::RefPtr<AnnotationBuffer> buffer) ;
		bool forward_search_insensitive_case(Glib::ustring& selection, Gtk::TextBuffer::iterator& match_start, Gtk::TextBuffer::iterator& match_end);
		void highlightAll(Glib::RefPtr<AnnotationBuffer> buffer) ;
		void select_occ(Glib::RefPtr<AnnotationBuffer> buffer) ;
		void prepareTag(Glib::RefPtr<AnnotationBuffer> buffer) ;
		bool is_whole_word() ;

	private:

		/** unused herited **/
		void on_search_bk() {} ;
		void on_search_fw() {} ;
		void select_occ() {} ;
		void prepareTag() {} ;
		void on_change_mode() {}
		void on_change_search_mode() {}
		void set_combo_search_mode() {}
		bool searchMode(const int flag_editability, const bool whole_word_allowed) { return false ;}
		void switch_file(AnnotationEditor* editor) {} ;
		virtual void close() {} ;
		bool is_active() { return false ;}
		void display_info(Glib::ustring text) {}
		int display_question(Glib::ustring text) {}
		void on_change_view() {}
		void on_buffer_changed() {}
		void set_combo_language() {}
		void on_change_language() {}
		void search_end() {}
		void selection_mode(bool mode) {}
		void on_change_cursor(const Gtk::TextBuffer::iterator& it, const Glib::RefPtr<Gtk::TextBuffer::Mark >& mark) {}


//		void checkScroll(Gtk::TextIter iter, Glib::RefPtr<AnnotationBuffer> buffer) ;
};

} //namespace


#endif /*HIGHLIGHTGENERAL_H_*/
