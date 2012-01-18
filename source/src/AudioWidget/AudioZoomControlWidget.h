/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_AUDIOZOOMCONTROLWIDGET__
#define __HAVE_AUDIOZOOMCONTROLWIDGET__

namespace tag {

/**
 * @class	AudioZoomControlWidget
 * @ingroup	AudioWidget
 *
 * This class implements zoom control buttons (+/-)
 */
class AudioZoomControlWidget : public Gtk::Button
{
public:
	/**
	 * Default constructor
	 * @param p_zoomIn	True for zooming in, false for zooming out
	 */
	AudioZoomControlWidget(bool p_zoomIn);

protected:
	/**
 	 * @var a_image
	 * 
	 * Gtk image for zoom buttons
	 */
	Gtk::Image a_image;
};

} // namespace

#endif // __HAVE_AUDIOZOOMCONTROLWIDGET__

