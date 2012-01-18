/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "IcoPackImageButton.h"
#include <iostream>

namespace tag {

IcoPackImageButton::IcoPackImageButton()
{
	add_events(Gdk::BUTTON_RELEASE_MASK) ;
	add_events(Gdk::BUTTON_PRESS_MASK) ;
	add_events(Gdk::ENTER_NOTIFY_MASK) ;
	add_events(Gdk::LEAVE_NOTIFY_MASK) ;
	add(hbox) ;
	hbox.pack_start(image1, false, false) ;
	hbox.pack_start(image2, false, false) ;
	hbox.pack_start(image3, false, false) ;
	set_name("notebook_button") ;
	set_mode(1) ;
	hbox.show() ;
	show() ;
}

IcoPackImageButton::~IcoPackImageButton()
{

}

bool IcoPackImageButton::on_enter_notify_event(GdkEventCrossing* event)
{
	bool res = false ;
	set_mode(2) ;
	return res ;
}

bool IcoPackImageButton::on_leave_notify_event(GdkEventCrossing* event)
{
	bool res = false ;
	set_mode(1) ;
	return res ;
}

bool IcoPackImageButton::on_button_press_event(GdkEventButton* event)
{
	bool res = false ;
	set_mode(3) ;
	return res ;
}

bool IcoPackImageButton::on_button_release_event(GdkEventButton* event)
{
	bool res = false ;

	//> if release but cursor not over button, do nothing
	if ( !(image3.is_visible() && !image2.is_visible() && !image1.is_visible()) )
		return true ;

	set_mode(1) ;
	return res ;
}

void IcoPackImageButton::set_image(int mode, const Glib::ustring& ico, int size)
{
	if (mode==1)
		image1.set_image(ico, size) ;
	else if (mode==2)
		image2.set_image(ico, size) ;
	else if (mode==3)
		image3.set_image(ico, size) ;
}

void IcoPackImageButton::set_image_path(int mode, const Glib::ustring& path, int size)
{
	if (mode==1)
		image1.set_image_path(path, size) ;
	else if (mode==2)
		image2.set_image_path(path, size) ;
	else if (mode==3)
		image3.set_image_path(path, size) ;
}


void IcoPackImageButton::set_tooltip(const Glib::ustring& tip)
{
	tooltip.set_tip(*this, tip) ;
}

void IcoPackImageButton::set_mode(int mode)
{
	switch (mode)
	{
		case 1:  	image1.show() ;
					image2.hide() ;
					image3.hide() ;
					break ;
		case 2:		image2.show() ;
					image3.hide() ;
					image1.hide() ;
					break ;
		case 3:		image3.show() ;
					image2.hide() ;
					image1.hide() ;
					break ;
		default: 	image1.show() ;
					image2.hide() ;
					image3.hide() ;
	}
}

} //namespace
