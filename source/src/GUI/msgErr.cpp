/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
//#include <gtkmm/plug.h>
#include <gtkmm/main.h>
#include <gtkmm/icontheme.h>
#include <gtkmm.h>
#include <string>

// --- Gtk / Gdk / X11 ---
#ifdef LINUX
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <gdk/gdkx.h>
#endif	// LINUX

#ifdef WIN32
	#include <winsock2.h>
#endif

#include "Common/globals.h"
#include "Common/icons/Icons.h"

using namespace tag ;


void set_dlg_icon(Gtk::Window* win, const Glib::ustring& icon, int size)
{
	try {
		Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
		Glib::RefPtr<Gdk::Pixbuf> pixbufPlay = theme->load_icon(icon, size,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
		win->set_icon(pixbufPlay);
	}
	catch (Gtk::IconThemeError e)
	{
		std::cerr << "Icons::set_window_icon:> " <<  e.what()  << std::endl ;
	}
	catch (Glib::FileError e)
	{
		std::cerr << "Icons::set_window_icon:> " <<  e.what()  << std::endl ;
	}
}

int main(int argc, char *argv[])
{
	std::cerr << "msgErr--> TranscriberAG is already running " << std::endl  ;

	Gtk::Main* kit = NULL ;
	try
	{
		kit = new Gtk::Main(argc, argv) ;
	}
	catch (Glib::OptionError e)
	{
		return 0 ;
	}

	Glib::ustring transag_dir			= "/usr/local/etc/TransAG";

	//> -- Prepare domain
	bindtextdomain(GETTEXT_PACKAGE, (transag_dir + "/locales").c_str());

	// -- Theme --
	Glib::ustring icon_dir  			= transag_dir + "/icons";
	Glib::ustring icon_gui_dir			= transag_dir + "/icons/GUI";
	Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
	theme->prepend_search_path( icon_dir );
	theme->prepend_search_path( icon_gui_dir );

	std::string msg =  _("An instance of TranscriberAG is already running.") ;
	Gtk::MessageDialog dialog(msg, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE, true) ;
	dialog.set_keep_above(true) ;
	set_dlg_icon(&dialog, ICO_TRANSCRIBER, 12) ;
	dialog.set_title("Attention") ;

 	g_thread_init(NULL);
	gdk_threads_init();

	gdk_threads_enter() ;
	kit->run(dialog) ;
	gdk_threads_leave() ;

	if (kit)
		delete(kit) ;

	return 0 ;
}
