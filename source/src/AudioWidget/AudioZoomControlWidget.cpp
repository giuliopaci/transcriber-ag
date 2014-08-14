/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class AudioZoomControlWidget
 *
 * AudioZoomControlWidget...
 */

#include "AudioWidget.h"

namespace tag {

// --- AudioZoomControlWidget ---
AudioZoomControlWidget::AudioZoomControlWidget(bool p_zoomIn)
 :	Gtk::Button(),
	a_image( (p_zoomIn ? Gtk::Stock::ZOOM_IN : Gtk::Stock::ZOOM_OUT),
			 Gtk::ICON_SIZE_MENU)
{
	set_image(a_image);
}

} // namespace
