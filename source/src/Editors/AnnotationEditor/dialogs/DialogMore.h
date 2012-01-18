/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 	DialogMore.h
 */

#ifndef __HAVE_DIALOGMORE__
#define __HAVE_DIALOGMORE__

#include <gtkmm.h>
#include <sstream>
#include <map>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Editors/AnnotationEditor/handlers/SAXAnnotationsHandler.h"

#include "DataModel/signals/SignalSegment.h"
#include "DataModel/speakers/Speaker.h"
#include "DataModel/speakers/SpeakerDictionary.h"
#include "DataModel/versions/VersionList.h"

#include "Common/widgets/FieldEntry.h"

using namespace std;

namespace tag {
/**
 * @class DialogMore
 *
 * Dialog used by the DialogFileProperties class for displaying
 * additional information about modified versions
 */
class DialogMore : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param p_parent						Reference on parent window
		 * @param p_versionList					Reference on versions list object
		 * @param p_displayAnnotationTime		True for displaying time passed on annotating
		 * @param editable						True for edition mode, False otherwise
		 */
		DialogMore(Gtk::Window& p_parent, VersionList* p_versionList, bool p_displayAnnotationTime, bool editable);
		virtual ~DialogMore();

	private:
		VersionList* a_versionList;
		FieldEntry** a_dateEntry;
		Gtk::Entry** a_id;
		Gtk::Entry** a_by;
		Gtk::Entry** a_time;
		Glib::RefPtr<Gtk::TextBuffer>* a_tbuffer;
		string getDate(FieldEntry* entry);
		void displayDate(const string& s, FieldEntry* entry);
		void onButtonClicked(int p_id);
};

}

#endif // __HAVE_DIALOGFILEPROPERTIES__
