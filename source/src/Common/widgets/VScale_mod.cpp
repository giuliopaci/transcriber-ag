/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "VScale_mod.h"
#include <iostream>

namespace tag {

VScale_mod::VScale_mod()
{
}

VScale_mod::VScale_mod(double min, double max, double step) : Gtk::VScale(min, max, step)
{
}

VScale_mod::~VScale_mod()
{
}

bool VScale_mod::on_button_press_event(GdkEventButton* event)
{
	if ( event->button==1 && event->type==GDK_BUTTON_PRESS 
			&& (event->state & GDK_CONTROL_MASK) 
		) 
	{	
		a_signalReset.emit() ;
		return true ;
	}
	else
		return Gtk::VScale::on_button_press_event(event) ;
}

} //namespace

