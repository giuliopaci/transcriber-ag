/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  TransAGVideo				*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "FBListView.h"

namespace tag {

FBListView::FBListView()
{
}

FBListView::~FBListView()
{
}

void FBListView::on_cursor_changed()
{
	Gtk::TreeIter iter = get_selection()->get_selected() ;
	m_signalSelection.emit(iter);
}


}
