/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/* $Id */

/** @file */

#ifndef TREEVIEWPREFERENCES_H_
#define TREEVIEWPREFERENCES_H_

#include <gtkmm.h>

namespace tag {
/**
 * @class 	TreeViewPreferences
 * @ingroup	GUI
 *
 * Gtk::TreeView implementation for the tree packed into the PreferencesDialog
 */
class TreeViewPreferences : public Gtk::TreeView
{
	public:
		/**
		 * Constructor
		 */
		TreeViewPreferences();
		virtual ~TreeViewPreferences();

		/**
		 * Signal emitted when the tree selection changes\n
		 * <b>Gtk::TreeIter* parameter:</b> 		Pointer on the new Gtk::TreeIter
		 * @return									The corresponding signal
		 */
		sigc::signal<void, Gtk::TreePath>& signalSelection() { return m_signalSelection; }

	private :

		Gtk::TreePath selection_active_iter ;
		/*
		 * used for tooltip to compare current and old iterator
		 */

		virtual void on_cursor_changed() ;

		/* signal emitted each time a row is selected (simple click) */
		sigc::signal<void, Gtk::TreePath> m_signalSelection ;
};

} //namespace

#endif /*TREEVIEWPREFERENCES_H_*/
