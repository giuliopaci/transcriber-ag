/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#include "TreeViewPreferences.h"

namespace tag {

TreeViewPreferences::TreeViewPreferences()
{
}

TreeViewPreferences::~TreeViewPreferences()
{
}


void TreeViewPreferences::on_cursor_changed()
{
	Gtk::TreeIter iter = get_selection()->get_selected() ;
	Gtk::TreePath path(iter) ;
	m_signalSelection.emit(path);
}

} //namespace
