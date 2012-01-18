/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_MARKTRACKWIDGET__
#define __HAVE_MARKTRACKWIDGET__

namespace tag {

/**
 * @class	MarkTrackWidget
 * @ingroup	AudioWidget
 * @deprecated
 */

class MarkTrackWidget : public TrackWidget
{
public:
	/**
	 * Default constructor
	 * @param p_duration	Track duration
	 * @param nbtrack		Tracks count (default : 1)
	 */
	MarkTrackWidget(float p_duration, int nbtrack=1);
	virtual ~MarkTrackWidget();

	// ---------------------------------
	// --- TrackWidget Inherited API ---
	// ---------------------------------
	void setTempo(float p_factor) {}
	void setZoom(int p_zoom) { a_mark->setZoom(p_zoom); }
	void setZoomMax(int p_zoomMax) { a_mark->setZoomMax(p_zoomMax); }
	void setCursor(float p_cursor) { a_mark->setCursor(p_cursor); }
	void setSelection(float p_beginSelection, float p_endSelection) { a_mark->setSelection(p_beginSelection, p_endSelection); }
	void setScroll(float p_cursorScroll) { a_mark->setScroll(p_cursorScroll); }
	void setCursorSize(int p_pix) { a_mark->setCursorSize(p_pix); }
	void setCursorColor(string p_color) { a_mark->setCursorColor(p_color); }
	int	getType() { return -1 ;}

	sigc::signal<void, float, float> signalSelectionChanged() { return a_mark->signalSelectionChanged(); }
	sigc::signal<void, float> signalCursorChanged() { return a_mark->signalCursorChanged(); }


	// -----------------
	// --- Internals ---
	// -----------------
	void set_current(bool value)
	{
		if (a_mark)
			a_mark->set_current(value) ;
	}

	bool get_current() {if (a_mark) return a_mark->is_current() ; else return false ;}


	void addMark(float p_timeCode, const char* p_text, const char* p_color) { a_mark->addMark(p_timeCode, p_text, p_color); }
	void removeMark(float p_timeCode) { a_mark->removeMark(p_timeCode); }

	void setSelectionnable(bool p_selectionnable) { a_mark->setSelectionnable(p_selectionnable); }
	void onPreviousClicked() { a_mark->previous(); }
	void onNextClicked() { a_mark->next(); }
	bool onKeyPressed(GdkEventKey* p_event);
	void setActiveTooltip(bool p_activeTooltip) { a_mark->setActiveTooltip(p_activeTooltip); }

	protected:

		MarkWidget* a_mark;
		sigc::signal<void> a_signalPreviousClicked;
		sigc::signal<void> a_signalNextClicked;
		Gtk::IconSize a_iconSize;
		Gtk::Image a_imagePrevious;
		Gtk::Image a_imageNext;
		Gtk::Tooltips a_tooltips;

};

} // namespace

#endif // __HAVE_MARKTRACKWIDGET__
