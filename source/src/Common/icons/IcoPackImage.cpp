/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "IcoPackImage.h"
#include <iostream>
#include "Common/util/Log.h"

namespace tag {

IcoPackImage::IcoPackImage()
{
}

IcoPackImage::~IcoPackImage()
{
}

int IcoPackImage::set_image(const Glib::ustring& ico, int size)
{
	try {
		Glib::RefPtr<Gtk::IconTheme>theme = Gtk::IconTheme::get_default() ;
		Glib::RefPtr<Gdk::Pixbuf>pixbufPlay = theme->load_icon(ico, size,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
		set(pixbufPlay);
		return 1 ;
	}
	catch (Gtk::IconThemeError e) {
		Log::err() << "IcoPackImage::set_image " <<  e.what()  << std::endl ;
		return -1 ;
	}
}

int IcoPackImage::set_image(const Gtk::StockID& stock, int size)
{
	set(stock, Gtk::IconSize(size)) ;
	return 1 ;
}


int IcoPackImage::set_image_path(const Glib::ustring& path, int size)
{
	int res = 1 ;
	try {
		Glib::RefPtr<Gdk::PixbufAnimation>pixbufPlay = Gdk::PixbufAnimation::create_from_file(path) ;
		set(pixbufPlay);
		set_pixel_size(size) ;
	}
	catch (Gtk::IconThemeError e) {
		Log::err() << "IcoPackImage::set_image_path " <<  e.what()  << std::endl ;
		res=-1 ;
	}
	catch (Glib::Error e) {
		res = -1;
	}
	return res ;
}

int IcoPackImage::set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip)
{
	int res = set_image(icon, size) ;
	set_tip(tooltip) ;
	return res ;
}

int IcoPackImage::set_icon(const Gtk::StockID& stock, const Glib::ustring& label, int size, const Glib::ustring& tooltip)
{
	int res = set_image(stock, size) ;
	set_tip(tooltip) ;
	return res ;
}

void IcoPackImage::set_tip(const Glib::ustring& tip)
{
	tooltip.set_tip(*this, tip) ;
}

} //namespace
