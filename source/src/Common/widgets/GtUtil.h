/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file	GtUtil.h
 */

#ifndef GTUTIL_H_
#define GTUTIL_H_

#include <gtkmm.h>
#include "Common/widgets/ProgressionWatcher.h"

namespace tag {

/**
* @class 		GtUtil
* @ingroup		Common
*
* Widgets utilities
*/
class GtUtil
{
	public:
		/**
		 * Constructor
		 * @return
		 */
		GtUtil();
		virtual ~GtUtil();

		/**
		 * Checks if the given key value represents a function key
		 * @param keyval		Keyval code
		 * @return				True if the given keyval corresponds to a function key,
		 * 						False otherwise
		 */
		static bool isFunctionKey(guint32 keyval) ;

		/**
		 * Checks if the given event corresponds to an common accel key one
		 * @param event		Pointer on GdkEventKey
		 * @return			True or False (obvious)
		 */
		static bool isAccelKeyEvent(GdkEventKey* event) ;

		/**
		 * Checks if the given event corresponds to an undo/redo action
		 * @param event		Pointer on GdkEventKey
		 * @return			True or False (obvious)
		 */
		static bool isUndoRedoEventKeys(GdkEventKey* event) ;

		/**
		 * Flush the Gtk GUI by calling events_pending and iterate method
		 * of Gtk Main.
		 * @param mm					True for Gtkmm mechanism, False for Gtk mechanisme
		 * @param threadProtection		True if called from a thread, False otherwise
		 * @note						If threadProtection is set to true, the methid will use
		 * 								the gdk_threads_enter() and gdk_threads_leave() methods
		 */
		static void flushGUI(bool mm, bool threadProtection) ;

		/**
		 * Copy a file with same name
		 * @param src_path			Source file parth
		 * @param dest_directory	Destination directory
		 * @param progressWatcher	Progress bar to be used (NULL if none)
		 * @param top				Parent window
		 */
		static void copy(Glib::ustring src_path, Glib::ustring dest_directory, ProgressionWatcher* progressWatcher, Gtk::Window* top) ;

		/**
		 * Returns the background color of the first widget called by the method
		 * @return		string color presentation
		 */
		static std::string getBaseColor(Gtk::Widget* widget=NULL) ;

	private:
		static std::string baseColor  ;
};

} // namespace

#endif /* GTUTIL_H_ */
