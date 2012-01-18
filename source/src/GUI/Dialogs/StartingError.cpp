/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "StartingError.h"
#include "Common/globals.h"
#include "Common/icons/Icons.h"
#include <iostream>
#include <sstream>
#include <string>
#include <gtkmm/icontheme.h>

namespace tag {

StartingError::StartingError(Glib::ustring message)
{
	Glib::RefPtr<Gdk::Screen> screen = get_screen() ;
	//set_default_size(screen->get_width(), screen->get_height());
	try {
		Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
	    Glib::RefPtr<Gdk::Pixbuf> pixbufPlay = theme->load_icon(ICO_TRANSCRIBER, 17,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
	    set_icon(pixbufPlay);
	}
	catch (Gtk::IconThemeError e) {
		Log::err() << "StartingError::StartingError:> " <<  e.what()  << std::endl ;
	}
	//> set title
	set_title("Transcriber");
	set_name("main");
	set_border_width(2) ;
	set_position(Gtk::WIN_POS_CENTER);

	//set image
	try {
		Glib::RefPtr<Gtk::IconTheme>theme = Gtk::IconTheme::get_default() ;
		Glib::RefPtr<Gdk::Pixbuf>pixbufPlay = theme->load_icon(ICO_ERROR, 42,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
		image.set(pixbufPlay);
	}
	catch (Gtk::IconThemeError e) {
		Log::err() << "StartingError::StartingError:> " <<  e.what()  << std::endl ;
	}

	//set label
	label.set_label(message) ;
	label.set_name("bold_label") ;

	blank1.set_label(" ") ;
	blank2.set_label(" ") ;
	blank3.set_label(" ") ;

	//set button
	button.set_label("Quit") ;
	button.signal_clicked().connect(sigc::mem_fun(*this, &StartingError::on_close)) ;

	add(vbox) ;
		vbox.pack_start(blank3);
	vbox.pack_start(image);
		vbox.pack_start(blank1);
	vbox.pack_start(label);
		vbox.pack_start(blank2);
		vbox.pack_start(sep);
	vbox.pack_start(button);

	show_all_children() ;
}

StartingError::~StartingError()
{
}

void StartingError::on_close()
{
	this->~StartingError() ;
}


} //namespace
