/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "HScale_mod.h"
#include <iostream>

namespace tag {

HScale_mod::HScale_mod()
{
}

HScale_mod::HScale_mod(double min, double max, double step) : Gtk::HScale(min, max, step)
{
}

HScale_mod::~HScale_mod()
{
}

bool HScale_mod::on_button_press_event(GdkEventButton* event)
{
	if ( event->button==1 && event->type==GDK_BUTTON_PRESS 
			&& (event->state & GDK_CONTROL_MASK) 
		) 
	{
		a_signalReset.emit() ;
		return true ;
	}
	else
		return Gtk::HScale::on_button_press_event(event) ;
}

} //namespace
