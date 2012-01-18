/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef LISTVIEW_MOD_H_
#define LISTVIEW_MOD_H_

#include <gtkmm.h>

namespace tag {
/**
 * @class 		ListView_mod
 * @ingroup		GUI
 *
 * ListView implementation for the speaker list packed in the SpeakerDico_dialog
 * class and the languages list packed in the SpeakerData class
 */
class ListView_mod : public Gtk::TreeView
{
	public :
		/**
		 * Constructor
		 */
		ListView_mod() ;
		~ListView_mod() {} ;

		/**
		 * Define the model tree the view will be connected to
		 * @param modelTree		The model of the view
		 */
		void set_modelTree(Glib::RefPtr<Gtk::TreeStore> modelTree) { model = modelTree ; }

		/**
		 * Signal emitted when the tree selection changes\n
		 * <b>Gtk::TreeIter* parameter:</b> 	Pointer on the new Gtk::TreeIter
		 * @return		Corresponding signal
		 */
		sigc::signal<void, std::vector<Gtk::TreeIter> >& signalSelection() { return m_signalSelection; }

	private :
		Gtk::TreeIter selection_active_iter ;

		Glib::RefPtr<Gtk::TreeStore> model ;

		virtual void on_cursor_changed() ;

		//signal emitted each time a row is selected (simple click)
		//pass iter
		sigc::signal<void, std::vector<Gtk::TreeIter> > m_signalSelection ;
} ;

} //namespace

#endif /*LIST_VIEW_H_*/

