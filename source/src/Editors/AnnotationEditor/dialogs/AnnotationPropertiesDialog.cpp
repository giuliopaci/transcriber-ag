/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 	AnnotationPropertiesDialog.h
 */

#include "Editors/AnnotationEditor/dialogs/AnnotationPropertiesDialog.h"
#include "Common/VersionInfo.h"

using namespace std;

namespace tag {

/**
 * Constructor
 * @param p_win				Reference on parent window
 * @param p_dataMode		Reference on datamodel
 * @param il				Reference on the InputLanguage used by parent editor
 * @param p_m_elementId		Element id
 * @param editable			True for edition mode, False otherwise
 */
AnnotationPropertiesDialog::AnnotationPropertiesDialog(Gtk::Window& p_win, DataModel& p_dataModel, const string& p_elementId, bool p_editable)
: Gtk::Dialog(TRANSAG_DISPLAY_NAME, p_win, true, true), m_dataModel(p_dataModel), m_editable(p_editable)
{
	m_elementId = p_elementId ;
	m_elementType = m_dataModel.getElementType(m_elementId) ;

#ifdef __APPLE__
	if (&p_win)
		set_transient_for(p_win);
#endif

}

bool AnnotationPropertiesDialog::on_key_press_event(GdkEventKey* event)
{
	string key = gdk_keyval_name (event->keyval);
	if ( key == "Return" || key == "KP_Enter"  ) {
		onButtonClicked(Gtk::RESPONSE_OK);
		return true;
	}
	return Gtk::Dialog::on_key_press_event(event);
}


}

