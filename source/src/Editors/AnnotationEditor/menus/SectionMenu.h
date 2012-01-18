/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		SectionMenu.h
 */

#ifndef _HAVE_SECTION_MENU
#define _HAVE_SECTION_MENU


#include <map>
#include <string>

#include "AnnotationMenu.h"
#include "Common/Parameters.h"

namespace tag {
/**
 * @class 		SectionMenu
 * @ingroup		AnnotationEditor
 *
 * Contextual menu for editor Section labels.\n
 * Displays available action on annotation of type section.
 *
 */
class SectionMenu : public AnnotationMenu
{
	public:
		/**
		 * Constructor
		 * @param subtypes			List of available section types
		 * @param configuration		Configuration map
		 */
		SectionMenu(const string& subtypes, const std::map<std::string, std::string>& configuration) ;
		~SectionMenu();

		/**
		 * Accessor to all labels
		 * @return		Map with all labels
		 */
		const std::map<std::string, std::string>& getLabels() const { return m_labels; }

		/**
		 * Signal emitted when section is modified
		 * <b>const Gtk::TextIter& parameter:</b>		Text position
		 * <b>std::string parameter:</b>				Section type
		 * <b>std::string parameter:</b>				Section description
		 */
		sigc::signal<void, const Gtk::TextIter&, std::string, std::string>& signalSetSection() { return  m_signalSetSection; }

	private:
		void onSelectSection(std::string type, std::string desc);
		sigc::signal<void, const Gtk::TextIter&, std::string, std::string>  m_signalSetSection;
		std::map<std::string, std::string> m_labels;

} ;

} /* namespace tag */


#endif  // _HAVE_SECTION_MENU
