/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

#ifndef GEOWINDOW_H_
#define GEOWINDOW_H_

#include <gtkmm.h>
#include "Common/widgets/Settings.h"

namespace tag {
/**
 * @class 	GeoWindow
 * @ingroup	Common
 *
 * Interface for enabling geometry management.
 * All windows or dialogs of the application should implement this interface in
 * order to use the Settings class for keeping/using old position and size.
 * @see		Settings class
 */
class GeoWindow
{
	public :
		/**
		 * Constructor
		 */
		GeoWindow();

		/**
		 * Destructor
		 * @return
		 */
		virtual ~GeoWindow();

		/**
		 * Saves the geometric window position and hide it
		 */
		virtual void saveGeoAndHide() = 0 ;

		/**
		 * Shows the window at the previous settings position
		 * @param rundlg	True for dialogs (will use Gtk::Run), false for window (will use Gtk::show)
		 * @return			Return of Gtk::run() for dialog, 1 for others.
		 */
		virtual int loadGeoAndDisplay(bool rundlg=false) = 0 ;

	protected :
		/**
		 *  Gets the position value saved in settings and move the window for fitting it.
		 */
		void loadPos() ;

		/**
		 *  Saves the position values of the window in the settings
		 */
		void savePos() ;

		/**
		 * Gets the geometry values
		 * @param[out] size_xx		Width
		 * @param[out] size_yy		Height
		 * @param[out] pos_x		x position
		 * @param[out] pos_y		y position
		 * @param[out] panel		panel position (-1 if no panel)
		 */
		virtual void getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) = 0 ;

		/**
		 * Gets the default geometry values, used when no settings could be found
		 * @param[out] size_xx		Width
		 * @param[out] size_yy		Height
		 * @param[out] pos_x		x position
		 * @param[out] pos_y		y position
		 * @param[out] panel		panel position (-1 if no panel)
		 */
		virtual void getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) = 0 ;

		/**
		 * Sets the geometry values
		 * @param size_xx	Width
		 * @param size_yy	Height
		 * @param pos_x		x position
		 * @param pos_y		y position
		 * @param panel		panel position (-1 if no panel)
		 */
		virtual void setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel) = 0 ;

		/**
		 * Accessor to the window TAG-type code
		 * @return	the window TAG-type code
		 */
		virtual Glib::ustring getWindowTagType() = 0 ;
};

}

#endif /* GEOWINDOW_H */
