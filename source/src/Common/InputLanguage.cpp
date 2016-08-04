/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "InputLanguage.h"
#include <iostream>
#include "globals.h"
//#include "AnnotationView.h"

namespace tag
{

InputLanguage::InputLanguage(std::string display, std::string lan, std::string shortcut, Glib::ustring MappingMode, bool modeLeft2Right, bool spaceSeparated, bool isActivated, bool modifyMapping)
{
	m_display = display ;
	m_languageType = lan ;
	m_modeLeft2Right = modeLeft2Right ;
	m_languageShortcut = shortcut ;
	m_isSpaceSeparated = spaceSeparated ;
	m_isActivated = isActivated ;
	m_modifyMapping = modifyMapping ;
	m_mappingMode = MappingMode ;
}

InputLanguage::~InputLanguage()
{
	std::vector<GunicharRange*>::iterator ite;
	for(ite = m_gunichar_range.begin(); ite != m_gunichar_range.end() ; ite++)
		delete (*ite);
}

bool
InputLanguage::addKeyMap(gunichar gdk_key_value, gunichar hardware_code, gunichar mapped_value, const Glib::ustring& replace_value, std::string modifier)
{
	ILKeyMap *map = new ILKeyMap(gdk_key_value, hardware_code, mapped_value, replace_value, modifier);
	m_keyMap.push_back(map);
}

bool
InputLanguage::hasKeyMap(GdkEventKey* event, Glib::ustring& res)
{
	gunichar gdk_key_value = (gunichar)event->keyval ;
	gunichar hardware_code = (gunichar)event->hardware_keycode ;
	const Glib::ustring& modifier = get_transcriber_modifier(event) ;

	//printf("KEY: %x\n", gdk_key_value) ;
	//printf("HARDWARE: %x\n\n", hardware_code) ;
	//std::TRACE << "[Mode = " << m_mappingMode << "]" << std::endl ;

	std::vector<ILKeyMap*>::iterator ite;
	bool is_in_map = false;

	for(ite = m_keyMap.begin() ; ite != m_keyMap.end(); ite++)
	{
		if ( m_mappingMode.compare(LANGUAGE_HARDWARE_MAPPING) == 0 )
			is_in_map = ( (*ite)->hardware_code == hardware_code && (*ite)->modifier.compare(modifier)==0 ) ;
		else if ( m_mappingMode.compare(LANGUAGE_LOGIC_MAPPING) == 0 )
			is_in_map = ((*ite)->gdk_key_val == gdk_key_value && (*ite)->modifier.compare(modifier)==0 ) ;
		else
			is_in_map = false ;

		//if same key value and same modifier, OK !
		if ( is_in_map  )
		{
			res = (*ite)->replace_value ;
			break ;
		}
	}
	return is_in_map;
}

Glib::ustring InputLanguage::get_transcriber_modifier(GdkEventKey* event)
{
	Glib::ustring modifier = "" ;

	if (event!=NULL) {
		if ( (event->state & GDK_SHIFT_MASK) && (event->state & GDK_CONTROL_MASK) )
			modifier = TRANS_MODIFIER_CTRLSHIFT ;
		else if ( (event->state & GDK_SHIFT_MASK) || (event->state & GDK_LOCK_MASK) )
			modifier = TRANS_MODIFIER_MAJ ;
#ifdef WIN32
		else if ( (event->state & GDK_MOD1_MASK) || (event->state & GDK_MOD2_MASK) )
			modifier = TRANS_MODIFIER_ALT ;
#else
	#ifdef __APPLE__
		else if ( (event->state & GDK_Meta_L) || (event->state & GDK_Meta_R) )
			modifier = TRANS_MODIFIER_ALT ;
	#else
		else if ( (event->state & GDK_MOD1_MASK) || (event->state & GDK_MOD5_MASK) )
			modifier = TRANS_MODIFIER_ALT ;
	#endif
#endif
	}
	else {
		TRACE << "event  NULL" << std::endl ;
	}

	return modifier ;
}

bool InputLanguage::check_insertion_rules(Gtk::TextView *view,const Gtk::TextIter& iter, const Glib::ustring& s)
{
	return true ;
}

bool
InputLanguage::check_insertion_rules_str(const Glib::ustring& text, int pos, const Glib::ustring& s)
{
	return true ;
}

void
InputLanguage::postProcessing(Gtk::TextView *view, Gtk::TextIter iter)
{
}

void
InputLanguage::postLoadingKeyMap(xercesc::DOMNode *node, gunichar c)
{
}

void
InputLanguage::addGunicharRange(GunicharRange *rg)
{
	if(rg != NULL)
		m_gunichar_range.push_back(rg);
}

bool
InputLanguage::is_gunichar_in_bound(gunichar c)
{
	std::vector<GunicharRange*>::iterator ite;
	for(ite = m_gunichar_range.begin() ; ite != m_gunichar_range.end(); ite++)
		if((c >= ((*ite)->start)) && (c <= ((*ite)->end)))
			return true;
	return false;
}

bool
InputLanguage::proceedDeletion(GdkEventKey *event, Gtk::TextView *view, Gtk::TextIter it)
{
	return false;
}

Glib::ustring InputLanguage::special_string_format(const Glib::ustring& s)
{
	return s ;
}


Glib::ustring InputLanguage::processingString(Glib::ustring s)
{
	return s ;
}

Glib::ustring InputLanguage::unprocessingString(Glib::ustring s)
{
	return s ;
}


}


