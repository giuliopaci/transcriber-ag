/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_SCALETRACKWIDGET__
#define __HAVE_SCALETRACKWIDGET__

namespace tag {

/**
 * @class	ScaleTrackWidget
 * @ingroup	AudioWidget

 * This class implements ScaleTrack controls\n
 */

class ScaleTrackWidget : public TrackWidget
{
public:
	/**
	 * Default constructor
	 * @param p_duration	Track duration
	 * @param nbtrack		Tracks count
	 */
	ScaleTrackWidget(float p_duration, int nbtrack=1);
	virtual ~ScaleTrackWidget();

	// ---------------------------------
	// --- TrackWidget Inherited API ---
	// ---------------------------------
	int  getType()							{return 2;}
	void setTempo(float)					{}
	void setCursor(float)					{}
	void setCursorSize(int)					{}
	void setCursorColor(string)				{}
	void setSelection(float, float)			{}
	void setZoom(int p_zoom)				{ a_scale->setZoom(p_zoom); }
	void setZoomMax(int p_zoomMax)			{ a_scale->setZoomMax(p_zoomMax); }
	void setScroll(float p_cursorScroll)	{ a_scale->setScroll(p_cursorScroll); }
	void setSelectable(bool mode)			{}

	sigc::signal<void, float>			signalCursorChanged()		{ return a_signalCursorChanged; }
	sigc::signal<void, float, float>	signalSelectionChanged()	{ return a_signalSelectionChanged; }

private:
		ScaleWidget*	a_scale;
		Gtk::HBox*		a_void;
		Gtk::HBox		a_startGap;

		sigc::signal<void, float>			a_signalCursorChanged;
		sigc::signal<void, float, float>	a_signalSelectionChanged;
};

} // namespace

#endif
