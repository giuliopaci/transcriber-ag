/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "TreeModelColumnsPreferences.h"

namespace tag {

TreeModelColumnsPreferences::~TreeModelColumnsPreferences()
{
}

void TreeModelColumnsPreferences::fill_row(Gtk::TreeModel::Row* row, const Glib::ustring& ico_path, int type, const Glib::ustring& name, int weight)
{
	TreeModelColumnsPreferences m_Columns ;
	Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();

	(*row)[m_Columns.m_config_type] = type ;
	(*row)[m_Columns.m_config_name] = name ;
	(*row)[m_Columns.m_config_ico] = theme->load_icon(ico_path, 47,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
	(*row)[m_Columns.m_config_weight] = weight ;
}

} //namespace
