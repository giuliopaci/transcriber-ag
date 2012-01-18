/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		SectionRenderer.h
 */

#ifndef _HAVE_SECTION_MENU
#define _HAVE_SECTION_MENU


#include <map>
#include <string>

#include "AnnotationMenu.h"
#include "Common/Parameters.h"

namespace tag {

class AnnotationEditor;
/**
* @class 		SignalPopupMenu
* @ingroup 		AnnotationEditor
*
*  Contextual signal menu.\n
*  Called when right-click is used on the signal view part
*/
class SignalPopupMenu : public AnnotationMenu
{
	public:
		/**
		* Constructor
		* @param edit			Pointer on the parent editor
		* @param trackmenu		Pointer on the track menu (should be defined by audio compoenent)
		*/
		SignalPopupMenu(AnnotationEditor* edit, Gtk::Menu* trackmenu) ;
		~SignalPopupMenu();

		/**
		 * Presents menu to user
		 * @param textIter		Text position
		 * @param x				x position
		 * @param y				y position
		 * @param event_time	Event time
		 */
		void popup(const Gtk::TextIter& textIter, int x, int y, guint32 event_time);
};

} /* namespace tag */


#endif  // _HAVE_SECTION_MENU
