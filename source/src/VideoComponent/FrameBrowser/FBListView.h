/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */


#ifndef FBLISTVIEW_H_
#define FBLISTVIEW_H_

#include <gtkmm.h>

namespace tag {
/**
 * @class 		FBListView
 * @ingroup		MediaComponent
 * List view implementation for the frames browser.
 */
class FBListView : public Gtk::TreeView
{
	public:
		FBListView();
		virtual ~FBListView();

		/**
		 * Signal emitted when the tree selection changes\n
		 * <b>const Gtk::TreeIter& parameter:</b> 	Pointer on the new Gtk::TreeIter
		 * @return		Corresponding signal
		 */
		sigc::signal<void, const Gtk::TreeIter& >& signalSelection() { return m_signalSelection; }

	private :
		virtual void on_cursor_changed() ;
		sigc::signal<void, const Gtk::TreeIter& > m_signalSelection ;
};

} //namespace

#endif /* FBLISTVIEW_H_ */
