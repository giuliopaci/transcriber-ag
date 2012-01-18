/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef DOC_H_
#define DOC_H_
#include "Common/globals.h"
#include <gtkmm.h>
#include <stdlib.h>

namespace tag {

/**
 * @class 		Doc
 * @ingroup		GUI
 *	User documentation of TranscriberAG
 */

class Doc
{
	public:

		/**
		 * Instantiate a documentation handler.
		 * @return
		 */
		Doc();
		virtual ~Doc();

		/**
		 * Static method for launching the user manual browser
		 * @param path path of manual file (index.html)
		 * @param parent parent window
		 * @param default_browser name of default browser to be used. Value set
		 * in TranscriberAG parameters
		 * (if it can't be found or is set to none, will try common browser)
		 */
		static void LaunchInstalledBrowser(Glib::ustring path, Gtk::Window* parent, Glib::ustring default_browser) ;

	private:
		static void LaunchInstalledBrowser_ALLOS(Glib::ustring path, Gtk::Window* parent, Glib::ustring default_browser) ;
};

} //namespace


#endif /*DOC_H_*/
