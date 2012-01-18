/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @file SectionMenu.cpp
 *   @brief Qualfiers popup menu implementation
 */

#include <iostream>
#include <map>
#include <iterator>

#include "Common/globals.h"
#include "Common/util/Utils.h"
#include "SectionMenu.h"
#include "Common/util/StringOps.h"
#include "Common/Dialogs.h"
#include "Common/Parameters.h"
#include "Common/FileInfo.h"


using namespace Gtk::Menu_Helpers;
using namespace std;



namespace tag {




SectionMenu::SectionMenu(const string& subtypes, const std::map<std::string, std::string>& menuconf)
: AnnotationMenu(_("section"))
{
	std::map<std::string, std::string>::const_iterator it, itd;

	if ( ! subtypes.empty() ) {
		// "noise;pronounce;disfluence;lexical;entity";
		vector<string> v1;
		vector<string>::iterator it1;
		std::map<string, string>::const_iterator it2;
		StringOps(subtypes).split(v1, ";");

		for ( it1 = v1.begin(); it1 != v1.end(); ++it1 ) {
			if ( (it2 = menuconf.find("section,"+*it1)) != menuconf.end() ) {
				m_labels[*it1] = it2->second;
				if ( m_labels[*it1].empty() ) m_labels[*it1] = *it1;
			} else  m_labels[*it1] = *it1;
			items().push_back(MenuElem(m_labels[*it1],
					sigc::bind<string, string>(sigc::mem_fun(*this, &SectionMenu::onSelectSection), (*it1), "")) );
		}
	}
}


/* destructor */
SectionMenu::~SectionMenu()
{
}


void SectionMenu::onSelectSection(string type, string desc)
{
  m_signalSetSection.emit(getTextIter(), type, desc);
}



} /* namespace tag */
