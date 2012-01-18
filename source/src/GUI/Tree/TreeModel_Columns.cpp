/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "TreeModel_Columns.h"
#include <iostream>
#include <gtkmm/icontheme.h>

namespace tag {


/* Fill a created row with chosen values */
void TreeModel_Columns::fill_row(Gtk::TreeModel::Row* row, const Glib::ustring& name, const Glib::ustring& display
								, const Glib::ustring& desc, const Glib::ustring& ico_path, const Glib::ustring& ico_path2
								, int sysType, int root, int rootType)
{
	TreeModel_Columns m_Columns ;
	Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();

	(*row)[m_Columns.m_file_display] = display ;
	(*row)[m_Columns.m_file_display_wNumber] = display ;
	(*row)[m_Columns.m_file_name] = name ;
	(*row)[m_Columns.m_file_desc] = desc ;
	(*row)[m_Columns.m_file_ico] = theme->load_icon(ico_path, 19,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
	(*row)[m_Columns.m_file_sysType] = sysType ;
	(*row)[m_Columns.m_file_root] = root ;
	(*row)[m_Columns.m_file_rootType] = rootType ;
	(*row)[m_Columns.m_file_nbFilteredFiles] = "" ;

	if (sysType==2 || sysType==3 || sysType==4 || sysType==5 || sysType==6  )
		(*row)[m_Columns.m_file_displayWeight] = 550 ;
	else
		(*row)[m_Columns.m_file_displayWeight] = 400 ;

	if (ico_path2.compare("")!=0) {
		(*row)[m_Columns.m_file_ico2] = theme->load_icon(ico_path2, 19,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
		(*row)[m_Columns.m_file_isExpander] = true ;
	}
	else
		(*row)[m_Columns.m_file_isExpander] = false ;
}

} //namespace

