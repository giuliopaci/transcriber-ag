/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef TSEARCHMANAGER_H_
#define TSEARCHMANAGER_H_

#include "TSearchDialog.h"
#include "Configuration.h"
#include "AnnotationEditor/AnnotationEditor.h"

namespace tag {
/**
 * @class 	TSearchManager
 * @ingroup	GUI
 *
 * Tag search component controller
 * @note 	the tag search toolbar mode isn't implemented yet, only dialog is available
 *
 */
class TSearchManager
{
	public:
		/**
		 * Constructor
		 * @param configuration		Application configuration object
		 */
		TSearchManager(Configuration* configuration) ;
		virtual ~TSearchManager();

		/**
		 * Initialize tag search components for given AnnotationEditor
		 * @param editor	The AnnotationEditor that will be connected to the tag search components
		 */
		void init(AnnotationEditor* editor) ;

		/**
		 * Change the AnnotationEditor the tag search components have access to
		 * @param editor	The AnnotationEditor that will be connected to the tag search components
		 */
		void switch_file(AnnotationEditor* editor) ;

		/**
		 * Return the activity status of the tag search components
		 * @return		True if the search toolbar or the search dialog is active, false otherise
		 */
		bool is_active() {return dialog->is_active() ;}

		/**
		 * Indicates if the toolbar mode is activated
		 * @return		True if the current mode is the toolbar mode
		 * @note		The toolbar mode is not yet implemented
		 */
		bool is_mini() {return false ; }

		/**
		 * Accessor to the dialog component
		 * @return		Pointer on the dialog component
		 */
		TSearchDialog* get_dialog() { return dialog ;}

		/**
		 * Specific show method for the tag search components
		 */
		void myShow() ;

		/**
		 * Specific hide method for the tag search components
		 */
		void myHide() ;

		/**
		 *	Search forward
		 */
		void search_forward()
		{
			if (dialog->is_active())
				dialog->on_search_fw() ;
		}

		/**
		 * Search backward
		 */
		void search_backward()
		{
			if (dialog->is_active())
				dialog->on_search_bk() ;
		}

		/**
		 * Indicates if one of the search component has focus
		 * @return	True if handled, false otherwise
		 */
		bool myHasFocus() ;

		/**
		 *  Wrapper for check selection methods of the tag search components
		 */
		void check_selection() ;


	private :
		TSearchDialog* dialog ;
		bool mini_mode ;

		Configuration* config ;

		/*
		 * mode=0: from dialog to mini mode
		 * mode=1: from mini mode to dialog
		 */
		void change_search_mode(int mode) ;
		void search_exit(int rep) ;
} ;

} //namespace

#endif /*SEARCHMANAGER_H_*/
