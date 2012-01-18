/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "AudioWidget.h"

namespace tag {

int SegmentTooltip::TEXT_MAX_SIZE = 100;

SegmentTooltip::SegmentTooltip() : TooltipTT()
{
	set_name("tooltip_segment");
	frame_in.add(a_label) ;
	data = "" ;
	start = -1 ;
	end = -1 ;
}

SegmentTooltip::~SegmentTooltip()
{
}


void SegmentTooltip::set_data(Glib::ustring p_text, float p_start, float p_end)
{
	data = p_text ;
	start = p_start ;
	end = p_end ;
}

void SegmentTooltip::prepare_tooltip()
{
	//> compute max size text
	gint m = (TEXT_MAX_SIZE-3)/2;

	//> see if cutting text is needed
	if (data.size() > TEXT_MAX_SIZE)
		data = data.substr(0, m-1) + "..." + data.substr(data.size()-1-(m-1), data.size()-1);
	else if (data.empty())
		data = _("(no text)") ;

/*
	//TODO display time
	Glib::ustring time ="\n" ;
	if (start>=0 && end>=0) {
		time.append(float_to_string(start)) ;
		time.append(" - ") ;
		time.append(float_to_string(end)) ;
	}
	data.append(time) ;
*/
	//> prepare display
	a_label.set_label(data);
	a_label.set_line_wrap(TRUE);
	frame_in.show_all_children() ;
	show_all_children(true) ;
}


bool SegmentTooltip::display(GdkEventMotion event, Gtk::Widget* win)
{
	if (/*!event ||*/ !win)
		return true ;

	if (start==-1 || end==-1)
		return true ;

	// compute position
	int toolx, tooly ;
	compute_position(win, event, toolx, tooly) ;

	// prepare tooltip
	prepare_tooltip() ;

	// adjust size
	resize(5,5) ;
	set_default_size(5,5) ;

	//adjust position
	adust_in_screen(toolx, tooly, gdk_get_default_root_window ()) ;
	move(toolx, tooly) ;

	//> don't use show method cause dispay redrawed
	//> resizing for forcing complete resize, otherwise keep all size
	reshow_with_initial_size() ;

	start = -1 ;
	end = -1 ;
	timeout.disconnect() ;

	return true ;
}

void SegmentTooltip::reset_data()
{
	data = "" ;
	start = -1 ;
	end = -1 ;
}

} // namespace
