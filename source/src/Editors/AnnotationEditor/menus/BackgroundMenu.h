/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/**
*  @file 	BackgroundMenu.h
*/

#ifndef _HAVE_BACKGROUND_MENU
#define _HAVE_BACKGROUND_MENU


#include <map>
#include <string>

#include "AnnotationMenu.h"
#include "Common/Parameters.h"

namespace tag {


/**
* @class 		BackgroundMenu
* @ingroup		AnnotationEditor
*
* Speaker selection menu widget used by the AudioSignalView.
*/
class BackgroundMenu : public AnnotationMenu
{
	public:
		/**
		 * Constructor
		 * @param qual_class	Qualifier class
		 */
		BackgroundMenu(const string& qual_class) ;
		~BackgroundMenu();

		/**
		 * Accessor to a label menu
		 * @param key	Option key
		 * @return		The corresponding label
		 */
		const std::string& getMenuLabel(const std::string& key) { return m_labels[getType()+","+key]; }

		/**
		 * Accessor to all labels
		 * @return		Map with all labels
		 */
		const std::map<std::string, std::string>& getLabels() const { return m_labels; }

		/* Inherited methods */
		virtual void updateMenu(bool can_create=true, bool can_edit=true, bool can_delete=true, bool can_be_unanchored=false);

	private:
		std::map<std::string, Gtk::Menu*>  m_typeSubmenu;
		std::map<std::string, std::string> m_labels;
		int m_nbItems;
};

} /* namespace tag */


#endif  // _HAVE_BACKGROUND_MENU
