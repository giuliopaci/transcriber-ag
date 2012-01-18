/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "ListModel_Columns.h"

namespace tag {

ListModel_Columns::~ListModel_Columns()
{
}

void ListModel_Columns::fill_row(Gtk::TreeModel::Row* row, Speaker* speaker, Languages* languages)
{
	ListModel_Columns m ;

	(*row)[m.a_id] = speaker->getId() ;
	(*row)[m.a_firstName] = speaker->getFirstName() ;
	(*row)[m.a_lastName] = speaker->getLastName() ;
	(*row)[m.a_gender] = speaker->getGender() ;

	if (speaker->getLanguages().size() !=0 && languages!=NULL) {
		Speaker::Language first_language = speaker->getLanguages()[0] ;
		Glib::ustring code = first_language.getCode() ;
		(*row)[m.a_lang] = languages->get_name(code) ;
	}
	else if (speaker->getLanguages().size() ==0)
		(*row)[m.a_lang] = "" ;
	(*row)[m.a_desc] = speaker->getDescription() ;
}

} //namespace
