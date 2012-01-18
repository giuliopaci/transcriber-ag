/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @file QualifierMenu.cpp
 *   @brief Qualfiers popup menu implementation
 */

#include <iostream>
#include <map>
#include <iterator>

#include "Common/globals.h"
#include "Common/util/Utils.h"
#include "BackgroundMenu.h"
#include "Common/util/StringOps.h"
#include "Common/Dialogs.h"
#include "Common/Parameters.h"
#include "Common/FileInfo.h"


using namespace Gtk::Menu_Helpers;
using namespace std;



namespace tag {

BackgroundMenu::BackgroundMenu(const string& qual_class)
 : AnnotationMenu(_(qual_class.c_str())), m_nbItems(0)
{
/*	std::map<std::string, std::string>::const_iterator it, itd;

//	m_labels = configuration;
	it = configuration.find("transcription_graph,"+qual_class);
	Glib::ustring qualifier_error = "" ;
	bool error = false ;

	if ( it != configuration.end() ) {
 		vector<string> v1, v2;
		vector<string>::iterator it1, it2;
		StringOps(it->second).split(v1, ";,");

		string sect=qual_class+",";

		for ( it1 = v1.begin(); it1 != v1.end(); ++it1 )
		{
			v2.clear();
			string key = (*it1)+",subtypes";

			m_typeSubmenu[*it1] = NULL;

			itd = configuration.find(key);
			if ( itd != configuration.end() && itd->second != "" && itd->second != "other" )
			{
				StringOps(itd->second).split(v2, ";,");
				m_typeSubmenu[*it1] = Gtk::manage (new Gtk::Menu());
				for ( it2 = v2.begin(); it2 != v2.end(); ++it2 )
				{
					key = *it1+string(",")+*it2;
					it = menuconf.find(sect+key);
					if ( it != menuconf.end() )
						m_labels[key] = it->second;
					else
						m_labels[key] = *it2 ;
//					if ( *it2 == "other" )  ICI CAS OTHER -> popup input win
					m_typeSubmenu[*it1]->items().push_back(MenuElem(m_labels[key],
							sigc::bind<string, string>(sigc::mem_fun(*this, &BackgroundMenu::onSelectQualifier), (*it1), (*it2))) );
				}
				it = menuconf.find(sect+*it1);
				if ( it != menuconf.end() )
					m_labels[*it1] = it->second;
				else
					m_labels[*it1] = *it1 ;
				items().push_back(MenuElem(m_labels[*it1], *(m_typeSubmenu[*it1])));
			}
			else if ( itd != configuration.end() ) {
				it = menuconf.find(sect+*it1);
				if ( it != menuconf.end() )
					m_labels[*it1] = it->second;
				else
					m_labels[*it1] = *it1 ;
				items().push_back(MenuElem(m_labels[*it1], sigc::bind<string, string>(sigc::mem_fun(*this, &BackgroundMenu::onSelectQualifier), (*it1), itd->second)) );
			}
			else  {
				TRACE << "<!> qualifier menu: no details for key " << *it1 << endl ;
				qualifier_error.append("- " + *it1 + "\n") ;
				error = true ;
			}
			}
		}
	if (error && display_error) {
		Glib::ustring error = _("Convention details not found for qualifier keys: ") + Glib::ustring("\n") + qualifier_error ;
		dlg::error(error, NULL) ;
	}
	m_nbItems = items().size();*/
}

/* destructor */
BackgroundMenu::~BackgroundMenu()
{
	/*
	std::map<std::string, Gtk::Menu*>::iterator it;
	for ( it = m_typeSubmenu.begin(); it != m_typeSubmenu.end(); ++it ) {
		TRACE_D << " Delete qual submenu for " << it->first << " ptr=" << hex << (guint32)it->second << dec << endl;
		if ( it->second != NULL )
			delete it->second;
	}
	*/
}

void
BackgroundMenu::updateMenu(bool can_create, bool can_edit, bool can_delete, bool can_be_unanchored)
{
	for ( int i=0; i < m_nbItems; ++i )
		items()[i].set_sensitive(can_create);
	AnnotationMenu::updateMenu(can_create, can_edit, can_delete, can_be_unanchored);
}


} /* namespace tag */
