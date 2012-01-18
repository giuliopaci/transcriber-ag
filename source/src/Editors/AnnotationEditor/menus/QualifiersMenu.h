/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		QualifiersMenu.h
 */
#ifndef _HAVE_QUALIFIERS_MENU
#define _HAVE_QUALIFIERS_MENU


#include <map>
#include <string>

#include "AnnotationMenu.h"
#include "Common/Parameters.h"

namespace tag {
/**
 * @class 		QualifiersMenu
 * @ingroup		AnnotationEditor
 *
 * Contextual menu for editor qualifiers labels.\n
 * Displays available action on annotation of type qualifier (named entities or events).
 */
class QualifiersMenu : public AnnotationMenu
{
	public:
		/**
		 * Constructor
		 * @param graph_type		Type of graph the qualifier type correspond to
		 * @param qual_class		Qualifier class
		 * @param configuration		Parameters map
		 * @param menuconf			Layout configuration map
		 * @param display_error		True
		 * @return					True for displaying dialog error when error occurs, False for hiding it
		 */
		QualifiersMenu(const string& graph_type, const string& qual_class, const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& menuconf, bool display_error) ;
		~QualifiersMenu();

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

		/**
		 * Signal emitted when a qualifier has been chosen in the menu.\n
		 * <b>const Gtk::TextIter& parameter:</b>		text position\n
		 * <b>std::string parameter:</b>				qualifier type\n
		 * <b>std::string parameter:</b>				qualifier description\n
		 */
		sigc::signal<void, const Gtk::TextIter&, std::string, std::string>& signalSetQualifier() { return  m_signalSetQualifier; }

		/* Inherited method */
		virtual void updateMenu(bool can_create=true, bool can_edit=true, bool can_delete=true, bool can_be_unanchored=false);

	private:
		void onSelectQualifier(std::string type, std::string desc);

		/* member attributes */
		sigc::signal<void, const Gtk::TextIter&, std::string, std::string>  m_signalSetQualifier;
		std::map<std::string, Gtk::Menu*>  m_typeSubmenu;
		std::map<std::string, std::string> m_labels;
		int m_nbItems;

	public:
		static std::string OTHER_CHOICE ;

};

} /* namespace tag */


#endif  // _HAVE_QUALIFIERS_MENU
