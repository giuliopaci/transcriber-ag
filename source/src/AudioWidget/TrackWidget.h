/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_TRACKWIDGET__
#define __HAVE_TRACKWIDGET__

namespace tag {

/**
 * @class	TrackWidget
 * @ingroup	AudioWidget
 *
 * Abstract base class for all track widgets (AudioTrackWidget, AudioWaveformWidget, AudioTrackWidget)
 */
class TrackWidget : public Gtk::HBox
{
public:
	/**
	 * Virtual destructor
	 */
	virtual ~TrackWidget() {}

	/**
	 * GTK signal : selectionChanged
	 */
	virtual sigc::signal<void, float, float> signalSelectionChanged() = 0;

	/**
	 * GTK signal : cursorChanged
	 */
	virtual sigc::signal<void, float> signalCursorChanged() = 0;

	/**
	 * Widget type accessor
	 * @return : Widget type
	 */
	virtual int getType() = 0;
	
	/**
	 * Tempo updater
	 * @param p_factor	New tempo factor
	 */
	virtual void setTempo(float p_factor) = 0;

	/**
	 * Graphical zoom updater
	 * @param p_zoom	New zoom level
	 */
	virtual void setZoom(int p_zoom) = 0;

	/**
	 * Maximum zoom value updater
	 * @param p_zoomMax	Maximum zoom value
	 */
	virtual void setZoomMax(int p_zoomMax) = 0;

	/**
	 * Cursor updater
	 * @param p_cursor	New cursor position
	 */
	virtual void setCursor(float p_cursor) = 0;

	/**
	 * Selection updater
	 * @param p_beginSelection	Selection start
	 * @param p_endSelection	Selection end
	 */
	virtual void setSelection(float p_beginSelection, float p_endSelection) = 0;

	/**
	 * Scroll updater
	 * @param p_cursorScroll	New scroll value
	 */
	virtual void setScroll(float p_cursorScroll) = 0;

	/**
	 * Selectable Mode
	 * @param p_selectable	Selectable mode
	 */
	virtual void setSelectable(bool p_selectable) = 0;

	/**
	 * Cursor Size
	 * @param p_pix		New cursor size
	 */
	virtual void setCursorSize(int p_pix) = 0;

	/**
	 * Cursor Color
	 * @param p_color	New cursor color
	 */
	virtual void setCursorColor(string p_color) = 0;

protected:
	/**
	 * Default Abstract Constructor
	 */
	TrackWidget() {}
};

} // namespace

#endif // __HAVE_TRACKWIDGET__

