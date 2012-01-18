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

#include "UnitMenu.h"

#include "Common/globals.h"
#include "Common/util/Utils.h"
#include "Common/util/StringOps.h"
#include "Common/Dialogs.h"
#include "Common/Parameters.h"
#include "Common/FileInfo.h"


using namespace Gtk::Menu_Helpers;
using namespace std;



namespace tag {

std::string UnitMenu::OTHER_CHOICE = "__other_choice";

UnitMenu::UnitMenu(const string& graphtype, const string& type)
{
	eventData = false ;
	m_nbItems = items().size();
}

UnitMenu::UnitMenu(const string& graphtype, const string& type,
						const std::map<std::string, std::string>& configuration,
						const std::map<std::string, std::string>& menuconf,
						bool display_error)
{
	eventData = true ;

	bool menu_ok = true ;
	Glib::ustring unit_error = "" ;

	std::map<std::string, std::string>::const_iterator it_submain ;
	it_submain = configuration.find(type+",subtypes") ;

	if (it_submain != configuration.end() )
	{
		vector<string> submains ;
		vector<string>::iterator it_submains ;
		StringOps(it_submain->second).split(submains, ";,");

		for ( it_submains = submains.begin(); it_submains != submains.end(); ++it_submains )
		{
			std::map<std::string, std::string>::const_iterator it_type, itd;
			it_type = configuration.find(*it_submains+",input") ;

			Glib::ustring unit_error = "" ;
			bool error = false ;

			if ( it_type != configuration.end() )
			{
				vector<string> types ;
				vector<string>::iterator it_types;
				StringOps(it_type->second).split(types, ";,");

				//REMARK We currently use the same layout than annotated event
				//TODO: check if we need to have a specific unit layout
				string sect="qualifier_event,";

				// -- For each "event" type
				for ( it_types = types.begin(); it_types != types.end(); ++it_types )
				{
					vector<string> subtypes ;
					vector<string>::iterator it_subtypes ;

					// get qualifier subtypes and create corresponding submenu items
					string key = (*it_types)+",subtypes";

					m_typeSubmenu[*it_types] = NULL;

					itd = configuration.find(key);
					// -- Search for subtype
					if ( itd != configuration.end() )
					{
						it_type = menuconf.find(sect+*it_types);
						if ( it_type != menuconf.end() )
							m_labels[*it_types] = it_type->second;
						else
							m_labels[*it_types] = *it_types ;

						//> We have subtype different from other choice, let's configure
						if ( itd->second != "" && itd->second != UnitMenu::OTHER_CHOICE )
						{
							StringOps(itd->second).split(subtypes, ";,");
							m_typeSubmenu[*it_types] = Gtk::manage (new Gtk::Menu());
							for ( it_subtypes = subtypes.begin(); it_subtypes != subtypes.end(); ++it_subtypes )
							{
								key = *it_types+string(",")+*it_subtypes;
								it_type = menuconf.find(sect+key);
								if ( it_type != menuconf.end() )
									m_labels[key] = it_type->second;
								else
									m_labels[key] = *it_subtypes ;

								m_typeSubmenu[*it_types]->items().push_back(MenuElem(m_labels[key],
										sigc::bind<string, string>(sigc::mem_fun(*this, &UnitMenu::onSelectUnit), (*it_types), (*it_subtypes))) );
							}
							key = (*it_types)+",editable";
							itd = configuration.find(key);
							bool need_other_choice = ( itd != configuration.end()  && itd->second == "true" );
							if ( need_other_choice )
							{
								m_typeSubmenu[*it_types]->items().push_back(MenuElem(_("_Other"),
									sigc::bind<string, string>(sigc::mem_fun(*this, &UnitMenu::onSelectUnit), (*it_types), UnitMenu::OTHER_CHOICE)) );
							}
							items().push_back(MenuElem(m_labels[*it_types], *(m_typeSubmenu[*it_types])));
						}
						else
						{
							//> Check editability
							key = (*it_types)+",editable";
							itd = configuration.find(key);
							bool need_other_choice = ( itd != configuration.end()  && itd->second == "true" );
							string desc = " " ;

							//> We don't have subtypes but editiability is OK, let's add editable text proposition
							if (need_other_choice)
								desc = UnitMenu::OTHER_CHOICE ;

							items().push_back(MenuElem(m_labels[*it_types], sigc::bind<string, string>(sigc::mem_fun(*this, &UnitMenu::onSelectUnit), (*it_types), desc)) );
						}
					}
					else  {
						TRACE << "<!> qualifier menu: no details for key " << *it_types << endl ;
						unit_error.append("- " + *it_types + "\n") ;
						menu_ok = true ;
					}
				}
			} // end "no input defined for submain"
			else
				menu_ok = false ;
		}// end for each submain
	} // end "no submain defined"
	else
		menu_ok = false ;

	if ( !menu_ok && display_error)
	{
		Glib::ustring error = _("Error when loading annotation menu for foreground events: ") + Glib::ustring("\n") + unit_error ;
		dlg::error(error, NULL) ;
	}

	m_nbItems = items().size();
}

/* destructor */
UnitMenu::~UnitMenu()
{
}

void UnitMenu::updateMenu(bool can_create, bool can_edit, bool can_delete, bool can_be_unanchored)
{
	for ( int i=0; i < m_nbItems; ++i )
		items()[i].set_sensitive(can_create);
	AnnotationMenu::updateMenu(can_create, can_edit, can_delete, can_be_unanchored);
}

void UnitMenu::onSelectUnit(string type, string desc)
{
	m_signalSetUnit.emit(getTextIter(), type, desc, m_selectionStart, m_selectionEnd) ;
}

} /* namespace tag */
