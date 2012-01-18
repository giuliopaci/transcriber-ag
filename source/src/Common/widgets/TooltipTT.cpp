/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "TooltipTT.h"
#include "Common/globals.h"
#include <iostream>

namespace tag {

TooltipTT::TooltipTT() : Gtk::Window(Gtk::WINDOW_POPUP)
{
	set_name("tooltip_win");

	//>set default timing
	timing = 600 ;

	int border_frame_in ;
	//> for lookn'feel don't add second frame in windows
#ifndef WIN32
	//> set label
	add(frame_out) ;
	// no border, but enable frame with shadow
	frame_out.set_border_width(0) ;
	frame_out.set_shadow_type(Gtk::SHADOW_ETCHED_OUT) ;
	frame_out.add(frame_in) ;
	border_frame_in = 3 ;
#else
	add(frame_in) ;
	border_frame_in = 0 ;
#endif
	//let border to get text with space, but no shadow
	frame_in.set_border_width(border_frame_in) ;
	frame_in.set_shadow_type(Gtk::SHADOW_NONE) ;

	//> set window
	set_border_width(0) ;
	show_all_children() ;
	set_position(Gtk::WIN_POS_MOUSE) ;
	set_has_frame(true);
	set_keep_above(false) ;
}

TooltipTT::~TooltipTT()
{

}


bool TooltipTT::adust_in_screen(gint& x, gint& y, GdkWindow* win)
{
	bool in_screen = true ;

	//> get gdk window largeur
	gint gdkwin_width, gdkwin_height, gdkwin_depth ;
	gint gdkwinx, gdkwiny;
	gdk_window_get_geometry(win, &gdkwinx, &gdkwiny, &gdkwin_width, &gdkwin_height, &gdkwin_depth) ;
	gdk_window_get_origin(win, &gdkwinx, &gdkwiny);

	//> limit of the screen that tooltip mustn't pass over
	gint limitx = gdkwinx + gdkwin_width ;
	gint limity = gdkwiny + gdkwin_height ;

	//> compute max tooltip value
	gint tooltipWidth, tooltipHeight ;
	get_size(tooltipWidth, tooltipHeight) ;
	gint tooltipmaxX = x + tooltipWidth ;
	gint tooltipmaxY = y + tooltipHeight ;

	if (tooltipmaxX > limitx) {
		x = x - tooltipWidth - 10 ;
		in_screen = false ;
	}
	if (tooltipmaxY > limity) {
		y = y - tooltipHeight - 10 ;
		in_screen = false ;
	}

	return in_screen ;
}

void TooltipTT::compute_position(Gtk::Widget* parent, GdkEventMotion event, int& res_tooltipX, int& res_tooltipY)
{
	// get origin window
	gint winx, winy ;
	GdkWindow* window = event.window;
	gdk_window_get_origin(window, &winx, &winy) ;

	// get pointer coord
	int x, y ;
	parent->get_pointer(x,y) ;

	// compute our coord
	res_tooltipX = x+winx+10 ;
	res_tooltipY = y+winy+12 ;
}

void TooltipTT::clean()
{
	frame_in.remove() ;
}

void TooltipTT::stop()
{
	hide() ;
	reset_data() ;
	timeout.disconnect() ;
}

void TooltipTT::launch(GdkEventMotion event, Gtk::Widget* win)
{
	hide() ;
	// disconnect last timeout
	timeout.disconnect() ;
	// launch new
	timeout = Glib::signal_timeout().connect(sigc::bind<GdkEventMotion,Gtk::Widget*>(sigc::mem_fun(this, &TooltipTT::display), event, win), get_timing()) ;
}

} //namespace
