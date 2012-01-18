/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Icons.h"
#include <iostream>
#include "Common/util/Log.h"

namespace tag {

void Icons::set_window_icon(Gtk::Window* win, const Glib::ustring& icon, int size)
{
	try {
		Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
		Glib::RefPtr<Gdk::Pixbuf> pixbufPlay = theme->load_icon(icon, size,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
		win->set_icon(pixbufPlay);
	}
	catch (Gtk::IconThemeError e) {
		Log::err() << "Icons::set_window_icon:> " <<  e.what()  << std::endl ;
	}
	catch (Glib::FileError e) {
		Log::err() << "Icons::set_window_icon:> " <<  e.what()  << std::endl ;
	}
}

Glib::RefPtr<Gdk::Pixbuf> Icons::create_pixbuf(const Glib::ustring& icon, int size, bool& result)
{
	Glib::RefPtr<Gdk::Pixbuf> pixbufPlay ;
	try {
		Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
		pixbufPlay = theme->load_icon(icon, size,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
		result = true ;
	}
	catch (Gtk::IconThemeError e) {
		Log::err() << "Icons::set_window_icon:> " <<  e.what()  << std::endl ;
		result = false ;
	}
	catch (Glib::FileError e) {
		Log::err() << "Icons::set_window_icon:> " <<  e.what()  << std::endl ;
		result = false ;
	}
	return pixbufPlay ;
}



} // namespace
