/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		UnitMenu.h
 */
#ifndef _HAVE_UNIT_MENU
#define _HAVE_UNIT_MENU


#include <map>
#include <string>

#include "AnnotationMenu.h"
#include "Common/Parameters.h"

namespace tag {
/**
 * @class 		UnitMenu
 * @ingroup		AnnotationEditor
 *
 * 		Contextual menu for mainstream base unit whose data input is pre-defined.\n
 *  Displays available action on annotation of units (named entities or events).
 */
class UnitMenu : public AnnotationMenu
{
	public:
		/**
		 * Constructor for units based on event
		 * @param graphtype		Type of graph the qualifier type correspond to
		 * @param type				Annotation type
		 * @param configuration		Parameters map
		 * @param menuconf			Layout configuration map
		 * @param display_error		True for displaying dialog error when error occurs, False for hiding it
		 */
		UnitMenu(const string& graphtype, const string& type,
								const std::map<std::string, std::string>& configuration,
								const std::map<std::string, std::string>& menuconf,
								bool display_error) ;

		/**
		 * Constructor for units based on textual data
		 * @param graphtype		Type of graph the qualifier type correspond to
		 * @param type				Annotation type
		 */
		UnitMenu(const string& graphtype, const string& type) ;

		~UnitMenu();

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
		sigc::signal<void, const Gtk::TextIter&, std::string, std::string, float, float>& signalSetUnit() { return  m_signalSetUnit; }

		/* Inherited method */
		virtual void updateMenu(bool can_create=true, bool can_edit=true, bool can_delete=true, bool can_be_unanchored=false);

	private:
		void onSelectUnit(std::string type, std::string desc);

		/* member attributes */
		bool eventData ;
		sigc::signal<void, const Gtk::TextIter&, std::string, std::string, float, float>  m_signalSetUnit ;
		std::map<std::string, Gtk::Menu*>  m_typeSubmenu;
		std::map<std::string, std::string> m_labels;
		int m_nbItems;

		void configureEventMenu(const string& graphtype, const string& type,
								const std::map<std::string, std::string>& configuration,
								const std::map<std::string, std::string>& menuconf,
								bool display_error) ;

		void configureTextualMenu(const string& graphtype, const string& type,
								const std::map<std::string, std::string>& configuration,
								const std::map<std::string, std::string>& menuconf,
								bool display_error) ;

	public:
		static std::string OTHER_CHOICE ;

};

} /* namespace tag */


#endif  // _HAVE_UNIT_MENU
