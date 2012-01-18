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

#ifndef __HAVE_ANNOTATIONPROPERTIESDIALOG__
#define __HAVE_ANNOTATIONPROPERTIESDIALOG__

#include <gtkmm.h>
#include <sstream>
#include <map>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DataModel/DataModel.h"


using namespace std;

namespace tag {


/**
 * @class AnnotationPropertiesDialog
 *
 * Dialogs for displaying and editing qualifier properties
 */
class AnnotationPropertiesDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param p_win				Reference on parent window
		 * @param p_dataModel		Reference on datamodel
		 * @param p_elementId		Element id
		 * @param editable			True for edition mode, False otherwise
		 */
		AnnotationPropertiesDialog(Gtk::Window& p_win, DataModel& p_dataModel, const string& p_elementId, bool editable);
		virtual ~AnnotationPropertiesDialog() {}

		/**
		 * Modifies editability
		 * @param b			True for edition mode, False otherwise
		 */
		void setEditable(bool b=false) { m_editable = b; }

	protected:
		/**< DATA RECEIVED **/
		DataModel& m_dataModel ;
		/**< Annotation id */
		string m_elementId ;
		/**< Annotation type */
		string m_elementType ;
		/**< Editability */
		bool m_editable ;

		/**
		 * Button clicked handler
		 * @param p_id		Button id
		 */
		virtual void onButtonClicked(int p_id) = 0;

		/**
		 * Key press event handler
		 * @param event		Key event
		 * @return			True if handled, false otherwise
		 */
		virtual bool on_key_press_event(GdkEventKey* event);
} ;

}

#endif // __HAVE_ANNOTATIONPROPERTIESDIALOG__
