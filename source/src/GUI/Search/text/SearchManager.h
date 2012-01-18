/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */
#ifndef SEARCHMANAGER_H_
#define SEARCHMANAGER_H_

#include "SearchReplaceDialog.h"
#include "MiniSearch.h"
#include "Configuration.h"
#include "Common/Parameters.h"
#include "AnnotationEditor/AnnotationEditor.h"

namespace tag {

/**
 * @class 	SearchManager
 * @ingroup	GUI
 *
 * SearchReplace components controller
 * Switches from toolbar mode and dialog mode
 */
class SearchManager
{
	public:
		/**
		 * Constructor
		 * @param configuration		Application configuration object
		 * @param parent			Parent window
		 */
		SearchManager(Configuration* configuration, Gtk::Window* parent);
		virtual ~SearchManager();

		/**
		 * Initialize text search components for given AnnotationEditor
		 * @param editor	The AnnotationEditor that will be connected to the text search components
		 */
		void init(AnnotationEditor* editor) ;

		/**
		 * Change the AnnotationEditor the text search components have access to
		 * @param editor	The AnnotationEditor that will be connected to the text search components
		 */
		void switch_file(AnnotationEditor* editor) ;

		/**
		 * Return the activity status of the text search components
		 * @return		True if the search toolbar or the search dialog is active, false otherise
		 */
		bool is_active() {return (mini->is_active()||dialog->is_active()) ;}

		/**
		 * Indicates if the toolbar mode is activated
		 * @return		True if the current mode is the toolbar mode
		 */
		bool is_mini() {return mini->is_active() ; }

		/**
		 * Accessor to the toolbar component
		 * @return		Pointer on the toolbar component
		 */
		MiniSearch* get_mini() { return mini ;}

		/**
		 * Accessor to the dialog component
		 * @return		Pointer on the dialog component
		 */
		SearchReplaceDialog* get_dialog() { return dialog ;}

		/**
		 * Specific show method for the text search components
		 */
		void myShow() ;

		/**
		 * Specific hide method for the text search components
		 */
		void myHide() ;

		/**
		 *	Search forward
		 */
		void search_forward()
		{
			if (mini->is_active())
				mini->on_search_fw() ;
			else if (dialog->is_active())
				dialog->on_search_fw() ;
		}

		/**
		 * Search backward
		 */
		void search_backward() {
			if (mini->is_active())
				mini->on_search_bk() ;
			else if (dialog->is_active())
				dialog->on_search_bk() ;
		}

		/**
		 * Indicates if one of the search component has focus
		 * @return	True if handled, false otherwise
		 */
		bool myHasFocus() ;

		/**
		 *  Wrapper for check selection methods of the text search components
		 */
		void check_selection() ;

	private :

		Configuration* config;
		MiniSearch* mini ;
		SearchReplaceDialog* dialog ;
		Gtk::Window* parent ;
		bool mini_mode ;

		/*
		 * mode=0: from dialog to mini mode
		 * mode=1: from mini mode to dialog
		 */
		void restoreData(int mode) ;
		/*
		 * mode=0: from dialog to mini mode
		 * mode=1: from mini mode to dialog
		 */
		void change_search_mode(int mode) ;
		void search_exit(int rep) ;
} ;

} //namespace

#endif /*SEARCHMANAGER_H_*/
