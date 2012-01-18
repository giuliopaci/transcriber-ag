/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @file AnnotationMenu.cpp
 *   @brief default AnnotationEditor context-sensitive popup menu implementation
 */

#include "AnnotationMenu.h"

using namespace Gtk::Menu_Helpers;
using namespace std;

namespace tag {

AnnotationMenu::AnnotationMenu(string type, bool add_std, bool editable)
: m_type(type), m_addStdOptions(add_std), m_editable(editable),
	m_editImage(Gtk::Stock::PROPERTIES, Gtk::ICON_SIZE_MENU),
	m_deleteImage(Gtk::Stock::CUT, Gtk::ICON_SIZE_MENU)
{
	m_unanchoredItem = NULL ;
	m_editItem = NULL;
	m_deleteItem = NULL;
	m_selectionStart = -1.0 ;
	m_selectionEnd = -1.0 ;
	m_popupMark = (Glib::RefPtr<Gtk::TextMark>)0;
}

/* Popup context-sensitive menu at the position calculated from last text event */
void AnnotationMenu::popup(const Gtk::TextIter& iter, int x, int y, guint32 event_time,
							bool can_create, bool can_edit, bool can_delete, bool canBeUnanchored,
							float start, float end)
{
	m_x=x;
	m_y=y;
	setIter(iter);

	m_selectionStart = start ;
	m_selectionEnd = end ;

	updateMenu(can_create, can_edit, can_delete, canBeUnanchored);
	select_first();

	//Gtk::Menu::popup(sigc::mem_fun(*this, &AnnotationMenu::onPopupMenuPosition), 1, event_time);
	((Gtk::Menu*)(this))->popup(sigc::mem_fun(*this, &AnnotationMenu::onPopupMenuPosition), 1, event_time);
}



void AnnotationMenu::setIter(const Gtk::TextIter& iter)
{
	m_textIter = iter;
	if ( m_popupMark == 0 )
		m_popupMark = iter.get_buffer()->create_mark(iter);
	else iter.get_buffer()->move_mark(m_popupMark, iter);
}


void AnnotationMenu::updateMenu(bool can_create, bool can_edit, bool can_delete, bool canBeUnanchored)
{
	if ( m_addStdOptions )
	{
		if ( m_editItem == NULL )
		{
			char label[80];

			//TODO a faire pour tous
			Glib::ustring displayed_type = m_type ;
			if ( displayed_type.compare("qualifier_event")==0 )
				displayed_type = _("event") ;
			else if ( displayed_type.compare("qualifier_entity")==0 )
				displayed_type = _("named entity") ;
			else if ( displayed_type.find("unit_")!=std::string::npos )
				displayed_type = _("unit") ;

			if ( m_editable )
				sprintf(label, _("_Edit %s properties"), displayed_type.c_str());
			else
				sprintf(label, _("_Display %s properties"), displayed_type.c_str());

			m_editItem = new ImageMenuElem(label, m_editImage, sigc::mem_fun(*this, &AnnotationMenu::onEditAnnotation));
			
			if ( m_editable )
			{
				sprintf(label, _("_Delete %s"), displayed_type.c_str());
				m_deleteItem = new ImageMenuElem(label, m_deleteImage, sigc::mem_fun(*this, &AnnotationMenu::onDeleteAnnotation));

				sprintf(label, _("_Remove time mark %s"), displayed_type.c_str());
				m_unanchoredItem = new MenuElem(label, sigc::mem_fun(*this, &AnnotationMenu::onUnanchorAnnotation));
			}
		}

		if ( m_editable )
		{
			if ( items().size() > 0 )
				items().push_back(SeparatorElem());
			items().push_back(*m_unanchoredItem);

			if ( items().size() > 0 )
				items().push_back(SeparatorElem());
			items().push_back(*m_deleteItem);
		
			items().push_back(SeparatorElem());
		}

		// add edit annotation menu option
		items().push_back(*m_editItem);

		m_addStdOptions = false;
	}

	// check if std menu items && set their sensitive state
	if ( ( m_editItem != NULL ) && m_editable )
		m_editItem->get_child()->set_sensitive(can_edit);
	if ( m_deleteItem != NULL )
		m_deleteItem->get_child()->set_sensitive(can_delete);
	if ( m_unanchoredItem != NULL )
		m_unanchoredItem->get_child()->set_sensitive(canBeUnanchored);
}


/*  compute menu position from event data */
void
AnnotationMenu::onPopupMenuPosition(int& x, int& y, bool& push_in)
{
	x=m_x; y=m_y;
	push_in = TRUE;
}

/*  edit Annotation */
void
AnnotationMenu::onEditAnnotation()
{
	m_signalEditAnnotation.emit(getTextIter());
}

/*  delete Annotation */
void
AnnotationMenu::onDeleteAnnotation()
{
	m_signalDeleteAnnotation.emit(getTextIter());
}

/*  delete Annotation */
void
AnnotationMenu::onUnanchorAnnotation()
{
	m_signalUnanchorAnnotation.emit(getTextIter());
}


const Gtk::TextIter& AnnotationMenu::getTextIter()
{
	m_textIter = m_popupMark->get_iter();
	return m_textIter;
}


}
