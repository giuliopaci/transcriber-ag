/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
*  @file 		StatsWidget.h
*/

#ifndef __HAVE_DIALOGSTATSWIDGET__
#define __HAVE_DIALOGSTATSWIDGET__

#include <gtkmm.h>
#include <sstream>
#include <map>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DataModel/signals/SignalSegment.h"
#include "DataModel/speakers/Speaker.h"
#include "DataModel/speakers/SpeakerDictionary.h"
#include "DataModel/DataModel_StatHelper.h"

using namespace std;

namespace tag {

/**
* @class 		StatsWidget
* @ingroup		AnnotationEditor
*
* Widgets displaying annotation statistics for a file.\n
*
*/
class StatsWidget : public Gtk::HBox
{
	public:
		/**
		 * Constructor
		 * @param p_model						Reference on parent editor model
		 * @param p_displayAnnotationTime		True for passed active annotation time,
		 * 										False otherwise.
		 * @return
		 */
		StatsWidget(DataModel& p_model, bool p_displayAnnotationTime);

		/**
		 * Desctructor
		 */
		virtual ~StatsWidget();

	private:
		list<string>* m_annotationsToDisplay;
		bool m_displayAnnotationTime;
		void printStatus(ostream& out, DataModel& data);
		void printStatistics(ostream& out, DataModel& data, int notrack, bool details, bool with_bg, string with_annots);

};

}

#endif // __HAVE_DIALOGSTATSWIDGET__
