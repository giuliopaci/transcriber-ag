/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_SCALEWIDGET__
#define __HAVE_SCALEWIDGET__

namespace tag {

/**
 * @class	ScaleWidget
 * @ingroup	AudioWidget
 *
 * This class implements ScaleTrackWidget drawing routines
 */

class ScaleWidget : public Gtk::DrawingArea
{
public:
	/**
 	 * Default constructor
	 * @param p_duration	Track duration
	 */
	ScaleWidget(float p_duration);
	virtual ~ScaleWidget();

	/**
 	 * Updates zoom factor and refreshes graphical area
	 * @param p_factor New zoom factor
	 */
	void	setZoom(int p_factor);

	/**
 	 * Updates scroll value and refreshes graphical area
	 * @param p_cursorScroll New cursor scroll
	 */
	void	setScroll(float p_cursorScroll);

	/**
 	 * Updates maximum zoom factor
	 * @param p_zoomMax	New maximum zoom factor
	 */
	void	setZoomMax(int p_zoomMax) { a_zoomMax = p_zoomMax; }

private:
	void	initColormap();

	float	a_duration;
	float	a_secsPerPix;
	float	a_cursorScroll;
	int		a_w;
	int		a_h;
	int		a_currentZoom;
	int		a_zoomMax;
	bool	on_expose_event(GdkEventExpose* p_event);

	// -- Colors --
	Glib::RefPtr<Gdk::GC>	gc;
	Gdk::Color baseGrey ;
	Gdk::Color white;
	Gdk::Color black;
};

} // namespace

#endif

