/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include <unistd.h>

#include "Doc.h"
#include "Explorer_dialog.h"
#include "Common/Dialogs.h"
#include "Common/VersionInfo.h"
#include "Common/util/Utils.h"

namespace tag {

Doc::Doc()
{
}

Doc::~Doc()
{
}

void Doc::LaunchInstalledBrowser(Glib::ustring path, Gtk::Window* parent, Glib::ustring default_browser)
{
	LaunchInstalledBrowser_ALLOS(path, parent, default_browser) ;
}

void Doc::LaunchInstalledBrowser_ALLOS(Glib::ustring path, Gtk::Window* parent, Glib::ustring default_browser)
{
	std::vector<Glib::ustring> browsers ;

	//> default browser will be tried first
	if (!default_browser.empty())
		browsers.insert(browsers.end(), default_browser) ;

	#ifdef __APPLE__
	browsers.insert(browsers.end(), "/Applications/Safari.app/Contents/MacOS/Safari");
	#endif
	#ifdef WIN32
	browsers.insert(browsers.end(), "C:\\Program Files\\Internet Explorer\\iexplore.exe");
	browsers.insert(browsers.end(), "C:\\Program Files\\Mozilla Firefox\\firefox.exe");
	#endif
	browsers.insert(browsers.end(), "firefox") ;
	browsers.insert(browsers.end(), "seamonkey") ;
	browsers.insert(browsers.end(), "opera") ;
	browsers.insert(browsers.end(), "mozilla") ;
	browsers.insert(browsers.end(), "konqueror") ;
	browsers.insert(browsers.end(), "netscape") ;
	browsers.insert(browsers.end(), "epiphany") ;

	//> 1 - Find available browser in path
	Glib::ustring browser = "" ;
	for (guint i = 0; i<browsers.size() && browser.compare("")==0 ; i++)
	{
		char* brow = g_find_program_in_path( browsers[i].c_str() ) ;
		if (brow)
			browser = browsers[i] ;
	}

	//> 2 - Launch
	Glib::ustring error = "" ;
	Glib::ustring command = "" ;
	if (browser.compare("")!=0)
	{
#ifndef WIN32
		command = browser + " " + path ;
#else
		command = "\"" +  browser + "\"" ;
		command = command + " \"" + replace_in_string(path, "/", "\\") + "\"" ;
#endif

		try
		{
			Glib::spawn_command_line_async( command );
		}
		catch (Glib::SpawnError e)
		{
			Log::err() << TRANSAG_DISPLAY_NAME << " --> Glibmm SpawnError : " << e.what() << std::endl ;
			Glib::ustring s1 = _("An error occured while launching documentation browser") ;
			Glib::ustring s2 = _("Please check your default browser in preferences") ;
			s1 = s1 + "\n" + s2 ;
			if (parent)
				Explorer_dialog::msg_dialog_error(s1, parent, true) ;
		}
	}
	else
		error = _("No browser found") ;

	if (error.compare("")!=0 && parent)
		Explorer_dialog::msg_dialog_error(error, parent, true) ;
}

} //namespace


