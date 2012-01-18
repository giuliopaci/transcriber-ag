/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "ListModelColumns_language.h"

namespace tag {

void ListModelColumns_language::fill_row(Gtk::TreeRow* row, Speaker::Language* language, bool editable, Languages* language_list)
{
	ListModelColumns_language m ;

	Glib::ustring code = language->getCode() ;
	(*row)[m.a_code] = code ;
	(*row)[m.a_name] = language_list->get_name(code) ;

	if ( language->getDialect() =="")
		(*row)[m.a_dialect] = "none" ;
	else
		(*row)[m.a_dialect] = language->getDialect() ;

	if ( language->getAccent() == "" )
		(*row)[m.a_accent] = "none" ;
	else
		(*row)[m.a_accent] = language->getAccent() ;
	(*row)[m.a_isNative] = language->isNative() ;
	(*row)[m.a_isUsual] = language->isUsual() ;

	//by default edition is not enabled
	if (!editable) {
		(*row)[m.a_activatable_combo] = Gtk::CELL_RENDERER_MODE_INERT ;
		(*row)[m.a_activatable_toggle] = false ;
	}
	else {
		(*row)[m.a_activatable_combo] = Gtk::CELL_RENDERER_MODE_EDITABLE ;
		(*row)[m.a_activatable_toggle] = true ;
	}
}

} //namespace
