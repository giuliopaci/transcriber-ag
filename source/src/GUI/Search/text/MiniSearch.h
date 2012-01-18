/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef MINISEARCH_H_
#define MINISEARCH_H_

#include "SearchReplaceDialog.h"

namespace tag {

/**
 * @class 	MiniSearch
 * @ingroup	GUI
 *
 * Search component for search 'n replace toolbar mode
 * Use SearchReplaceGeneral mechanisms
 *
 */
class MiniSearch : public SearchReplaceGeneral, public Gtk::EventBox
{
	public:
		/**
		 * Constructor
		 */
		MiniSearch(int searchMode = 0, bool cas = false, bool wholeWord = false);
		virtual ~MiniSearch();

		/**
		 * Sets focus state
		 */
		void mySet_focus() ;

		/**
		 * Accessor for focus state
		 * @return	True if widget has focus, false otherwise
		 */
		bool myHasFocus() ;

		/**
		 * Set parent window
		 * @param p_parent		Parent window
		 */
		void setParent(Gtk::Window* p_parent) { parent = p_parent ;}

	private:
		//widgets
		Gtk::Window* parent ;

		Gtk::Table *table ;

		Gtk::Alignment align_gen ;
		Gtk::Alignment align_close ;
		Gtk::Alignment align_box ;
		Gtk::Alignment align_mode ;

		Gtk::Button clear_selection ;

		Gtk::HBox hbox_gen ;
		Gtk::Label result ;
		Gtk::HBox hbox_search ;

		void prepare_gui() ;
		void set_widgets_label();

		void display_info(Glib::ustring text) ;
		int display_question(Glib::ustring text);
		void myHide();
		void on_change_mode() ;
		void on_clear_selection() ;
};

} //namespace

#endif /*MINISEARCH_H_*/
