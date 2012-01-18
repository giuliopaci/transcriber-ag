/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "DynamicTable.h"

namespace tag {

DynamicTable::DynamicTable(guint nbRows, guint nbColumns, bool homogenous)
: Gtk::Table(nbRows, nbColumns, homogenous)
{
	current_nb_rows = 0 ;
	columns = nbColumns ;
	rows = nbRows ;

	//default values
	xpadding = 0 ;
	ypadding = 0 ;
	xattach = Gtk::FILL ;
	yattach = Gtk::SHRINK ;
}


void DynamicTable::set_padding_options(guint xpad, guint ypad)
{
	xpadding = xpad ;
	ypadding = ypad ;
}

void DynamicTable::set_attach_options(Gtk::AttachOptions xopt, Gtk::AttachOptions yopt)
{
	xattach = xopt ;
	yattach = yopt ;
}

bool DynamicTable::add_row(const std::vector<Gtk::Widget*>& widgets)
{
	//resize table if needed
	if ( current_nb_rows==rows ) {
		rows++ ;
		resize(rows+1, columns) ;
	}

	//param are not like table schema
	if (widgets.size()!=columns)
		return false ;

	std::vector<Gtk::Widget*>::const_iterator it ;
	int col = 0 ;
	for (it=widgets.begin(); it!=widgets.end(); it++) {
		if (!*it)
			return false ;
		else {
			attach(**it, col, col+1, rows, rows+1, xattach, yattach, xpadding, ypadding) ;
			col ++ ;
		}
	}
	current_nb_rows++ ;
	return true ;
}

DynamicTable::~DynamicTable()
{
	// TODO Auto-generated destructor stub
}

} //namespace
