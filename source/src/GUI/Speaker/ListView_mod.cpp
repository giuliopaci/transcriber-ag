/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "ListView_mod.h"

#include <gtk/gtk.h>

#include <iostream>

namespace tag {

ListView_mod::ListView_mod()
{

}

void ListView_mod::on_cursor_changed()
{
	int cpt=0 ;
	std::vector<Gtk::TreePath> paths = get_selection()->get_selected_rows() ;
	std::vector<Gtk::TreeIter> iters ;
	for(guint i = 0; i<paths.size(); i++)
	{
		Gtk::TreeIter iter = get_model()->get_iter(paths[i]) ;
		iters.insert(iters.end(), iter) ;
		cpt++ ;
	}
	m_signalSelection.emit(iters);
}

} //namespace



