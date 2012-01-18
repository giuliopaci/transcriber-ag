/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class ScaleTrackWidget
 *
 * ScaleTrackWidget...
 */

#include "AudioWidget.h"
#include <gtk/gtkstyle.h>

namespace tag {

// --- ScaleTrackWidget ---
ScaleTrackWidget::ScaleTrackWidget(float p_duration, int nbtrack) : TrackWidget()
{
	set_no_show_all(true);

	if ( nbtrack > 1 )
	{
		pack_start(a_startGap, false, false, 1);
		a_startGap.show();
		a_startGap.set_size_request(30, -1);
	}

	a_scale	= new ScaleWidget(p_duration);
	pack_start(*a_scale,true,	true,	2);

	a_void	= Gtk::manage( new Gtk::HBox );
	a_void->set_size_request(82, -1);	// Fixed size, iiirk!
	pack_start(*a_void,	false,	false,	0);
	
	a_scale->show();
	a_void->show();
}


// --- ~ScaleTrackWidget ---
ScaleTrackWidget::~ScaleTrackWidget()
{
	delete a_scale;
}

} // namespace
