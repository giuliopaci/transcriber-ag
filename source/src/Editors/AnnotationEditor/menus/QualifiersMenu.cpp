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
#include "QualifiersMenu.h"
#include "Common/util/StringOps.h"
#include "Common/Dialogs.h"
#include "Common/Parameters.h"
#include "Common/FileInfo.h"


using namespace Gtk::Menu_Helpers;
using namespace std;



namespace tag {

std::string QualifiersMenu::OTHER_CHOICE = "__other_choice";


QualifiersMenu::QualifiersMenu(const string& graphtype, const string& qual_class, const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& menuconf, bool display_error)
: AnnotationMenu(_(qual_class.c_str()))
{
	std::map<std::string, std::string>::const_iterator it, itd;

	//	m_labels = configuration;

	it = configuration.find(graphtype+","+qual_class);

	Glib::ustring qualifier_error = "" ;
	bool error = false ;

	if ( it != configuration.end() ) {
		vector<string> v1, v2;
		vector<string>::iterator it1, it2;
		StringOps(it->second).split(v1, ";,");

		string sect=qual_class+",";

		for ( it1 = v1.begin(); it1 != v1.end(); ++it1 )
		{
			// get qualifier subtypes and create corresponding submenu items
			v2.clear();
			string key = (*it1)+",subtypes";

			m_typeSubmenu[*it1] = NULL;

			itd = configuration.find(key);
			if ( itd != configuration.end() )
			{
				it = menuconf.find(sect+*it1);
				if ( it != menuconf.end() )
					m_labels[*it1] = it->second;
				else
					m_labels[*it1] = *it1 ;

				//> We have subtype different from other choice, let's configure
				if ( itd->second != "" && itd->second != QualifiersMenu::OTHER_CHOICE )
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
						m_typeSubmenu[*it1]->items().push_back(MenuElem(m_labels[key],
								sigc::bind<string, string>(sigc::mem_fun(*this, &QualifiersMenu::onSelectQualifier), (*it1), (*it2))) );
					}
					key = (*it1)+",editable";
					itd = configuration.find(key);
					bool need_other_choice = ( itd != configuration.end()  && itd->second == "true" );
					if ( need_other_choice )
					{
						m_typeSubmenu[*it1]->items().push_back(MenuElem(_("_Other"),
							sigc::bind<string, string>(sigc::mem_fun(*this, &QualifiersMenu::onSelectQualifier), (*it1), QualifiersMenu::OTHER_CHOICE)) );
					}
					items().push_back(MenuElem(m_labels[*it1], *(m_typeSubmenu[*it1])));
				}
				else 
				{
					//> Check editability
					key = (*it1)+",editable";
					itd = configuration.find(key);
					bool need_other_choice = ( itd != configuration.end()  && itd->second == "true" );
					string desc = " " ;

					//> We don't have subtypes but editiability is OK, let's add editable text proposition
					if (need_other_choice)
						desc = QualifiersMenu::OTHER_CHOICE ;

					items().push_back(MenuElem(m_labels[*it1], sigc::bind<string, string>(sigc::mem_fun(*this, &QualifiersMenu::onSelectQualifier), (*it1), desc)) );
				}
			}
			else  {
				TRACE << "<!> qualifier menu: no details for key " << *it1 << endl ;
				qualifier_error.append("- " + *it1 + "\n") ;
				error = true ;
			}
		}
	}
	if (error && display_error) {
		Glib::ustring error = _("Some qualifier types or subtypes are used in conventions but are not specified: ") + Glib::ustring("\n") + qualifier_error ;
		error = error + "\n" + _("Check your conventions files.") ;
		dlg::warning(error, NULL) ;
	}
	m_nbItems = items().size();
}

/* destructor */
QualifiersMenu::~QualifiersMenu()
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
QualifiersMenu::updateMenu(bool can_create, bool can_edit, bool can_delete, bool can_be_unanchored)
{
	for ( int i=0; i < m_nbItems; ++i )
		items()[i].set_sensitive(can_create);
	AnnotationMenu::updateMenu(can_create, can_edit, can_delete, can_be_unanchored);
}

void QualifiersMenu::onSelectQualifier(string type, string desc)
{
  m_signalSetQualifier.emit(getTextIter(), type, desc);
}



} /* namespace tag */
