/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_AUDIOTRACKWIDGET__
#define __HAVE_AUDIOTRACKWIDGET__

#include "io/IODevice.h"
#include "filters/MuteFilter.h"
#include "filters/SoundTouchFilter.h"
#include "filters/VolumeFilter.h"

#include "Common/icons/IcoPackImageButton.h"

namespace tag {

/**
 * @class	AudioTrackWidget
 * @ingroup	AudioWidget
 *
 * Scale widget allowing playback speed control
 */
class AudioTrackWidget : public TrackWidget
{
public:
	/**
	 * Default Constructor
	 * @param device		Input IODevice pointer, can be NULL
	 * @param silentMode	True when using a SilentHandler as device
	 */
	AudioTrackWidget(IODevice* device, bool silentMode=false);

	virtual ~AudioTrackWidget();

	// -------------------------------
	// -- TrackWidget inherited API --
	// -------------------------------
	sigc::signal<void, float, float> signalSelectionChanged()	{ return a_wave->signalSelectionChanged(); }
	sigc::signal<void, float> signalCursorChanged()				{ return a_wave->signalCursorChanged(); }
	void	setTempo(float p_factor)		{ if (st_filter) st_filter->setTempo(p_factor); }
	void	setZoom(int p_zoom)				{ a_wave->setZoom(p_zoom); }
	void	setZoomMax(int p_zoomMax)		{ a_wave->setZoomMax(p_zoomMax); }
	void	setCursor(float p_cursor)		{ a_wave->setCursor(p_cursor); }
	void	setSelection(float p_beginSelection, float p_endSelection) { a_wave->setSelection(p_beginSelection, p_endSelection); }
	void	setScroll(float p_cursorScroll) { a_wave->setScroll(p_cursorScroll); }
	void	setSelectable(bool);
	void	setCursorSize(int p_pix)		{ a_wave->setCursorSize(p_pix); }
	void	setCursorColor(string p_color)	{ a_wave->setCursorColor(p_color); }
	int  	getType()						{ return 0; }

	// ---------------
	// -- INTERNALS --
	// ---------------

	// -----------------
	// -- GTK Signals --
	// -----------------

	/**
	 * GTK signal : activated
	 * @return activated signal
	 */
	sigc::signal<void, bool> signalActivated()			{ return a_signalActivated; }

	/**
	 * GTK signal : offsetUpdated
	 * @return offsetUpdated signal
	 */
	sigc::signal<void, float> signalOffsetUpdated()		{ return a_signalOffsetUpdated; }

	/**
	 * GTK signal : audioTrackSelected
	 * @return audioTrackSelected signal
	 */
	sigc::signal<void> signalAudioTrackSelected()		{ return a_wave->signalAudioTrackSelected(); }

	/**
	 * GTK signal : populatePopup
	 * @return populatePopup signal
	 */
	sigc::signal<void, int, int> signalPopulatePopup()	{ return a_wave->signalPopulatePopup(); }

	/**
	 * GTK signal : sizeChanged
	 * @return sizeChanged signal
	 */
	sigc::signal<void, int> signalSizeChanged()			{ return a_wave->signalSizeChanged(); }

	/**
	 * GTK signal : selecting
	 * @return selecting signal
	 */
	sigc::signal<void, bool, bool> signalSelecting()	{ return a_wave->signalSelecting(); }

	/**
	 * GTK signal : focusIn
	 * @return focusIn signal
	 */
	sigc::signal<void> signalFocusIn()					{ return a_signalFocusIn; }

	/**
	 * GTK signal : expanded
	 * @return expanded signal
	 */
	sigc::signal<void, bool> signalExpanded()			{ return a_signalExpanded; }


	/**
	 * Notify child waveform our current track status
	 * @param value	Current status
	 */
	void setCurrent(bool value)				{ a_wave->setCurrent(value) ; }

	/**
	 * Waveform display ready
	 * @return True if waveform is drawn, false otherwise.
	 */
	bool	isReady()						{ return a_wave->isReady(); }

	/**
	 * Returns Activate button widget size (in pixels)
	 * @return Activate button size
	 */
	int		getActivateBtnSize();

	/**
	 * Channel ID accessor
	 * @return Channel ID
	 */
	int		getChannelID()					{ return channelID; }

	/**
	 * Channel ID accessor
	 * @param id	Channel ID
	 */
	void	setChannelID(int id);

	/**
	 * Channels count accessor
	 * @param num	Channels count
	 */
	void	setNbChannels(int num);

	/**
	 * Forward parameters for streaming mode (to Waveform widget)
	 * @param mode	Streaming mode
	 * @param rtsp	Rtsp url
	 * @param local	Local path
	 */
	void	setStreaming(bool mode, string rtsp, string local) { a_wave->setStreaming(mode, rtsp, local); }

	/**
	 * Initializes filtering chain for this track.
	 * @param silentMode	True when using a SilentHandler as device
	 */
	void	initFilters(bool silentMode);


	// ---------------------
	// -- Stream Commands --
	// ---------------------

	/**
	 * Seeks into stream
	 * @param p_seek	Seek position (in seconds)
	 */
	void	seek(float p_seek)				{ if (st_filter) st_filter->seek(p_seek); }

	/**
	 * Stops stream
	 */
	void	stop()							{ if (st_filter) st_filter->stop(); }

	/**
	 * Starts stream
	 */
	void	play()							{ if (st_filter) st_filter->play(); }

	/**
	 * Moves cursor with an optional offset
	 * @param in_cursor	New cursor position
	 * @param in_offset	Offset position (default 0.0)
	 */
	void	setOffset(float in_cursor, float in_offset=0.0);

	/**
	 * Displays / Hides Mute button (disabled with a mono signal)
	 * @param b	True -> displays / false -> hides
	 */
	void	showActivateBtn(bool b);

	/**
	 * Vertical zoom accessor
	 * @param p_factor	Vertical zoom factor
	 */
	void	setVZoom(float p_factor)		{ a_wave->setVZoom(p_factor); a_avzc->setFactor(p_factor); }

	/**
	 * Volume accessor
	 * @param p_factor	Volume factor
	 */
	void	setVolume(float p_factor)		{ v_filter->setVolumeFactor(p_factor); a_avc->setFactor(p_factor); }

	/**
	 * Pitch accessor
	 * @param p_factor	Pitch factor
	 */
	void	setPitch(float p_factor)
	{
		if (st_filter)
			st_filter->setPitch(p_factor);
		if (a_apc)
			a_apc->setFactor(p_factor);
	}

	/**
	 * Vertical zoom accessor
	 * @return Vertical zoom factor
	 */
	float	getVZoom()
	{
		if (a_wave)
			return a_wave->getVZoom();
		else
			return 0.0 ;
	}

	/**
	 * Volume accessor
	 * @return Volume factor
	 */
	float	getVolume()
	{
		if (v_filter)
			return v_filter->getVolumeFactor();
		else
			return 0.0 ;
	}

	/**
	 * Pitch accessor
	 * @return Pitch factor
	 */
	float	getPitch()
	{
		if (st_filter)
			return st_filter->getPitch();
		else
			return 0.0 ;
	}


	// -------------------
	// -- GTK Callbacks --
	// -------------------

	/**
	 * GTK callback - Vertical zoom changed
	 * @param p_factor	Vertical zoom factor
	 */
	void	onVZoomChanged(float p_factor)	{ setVZoom(p_factor); }

	/**
	 * GTK callback - Volume changed
	 * @param p_factor	Volume factor
	 */
	void	onVolumeChanged(float p_factor)	{ setVolume(p_factor); }

	/**
	 * GTK callback - Pitch changed
	 * @param p_factor	Pitch factor
	 */
	void	onPitchChanged(float p_factor)	{ setPitch(p_factor); }

	/**
	 * GTK callback - Activate clicked
	 */
	void	onActivateClicked();

	/**
	 * GTK callback - Expand clicked
	 */
	bool	onExpandClicked(GdkEventButton* event) ;

	/**
	 * GTK callback - Key pressed
	 * @param event	Gdk key event
	 */
	bool	onKeyPressed(GdkEventKey* event);

	/**
	 * GTK callback - Update offset clicked
	 */
	void	onUpdateOffsetClicked();

	/**
	 * GTK callback - FocusIn
	 */
	void	onFocusIn();

	/**
	 * Activation status
	 * @return Activation status
	 */
	bool	isActivated()					{ return a_activated; }

	/**
	 * Track offset accessor
	 * @return Track offset
	 */
	float	getOffset()						{ return a_offset; }

	/**
	 * Selected status (forward to waveform widget)
	 * @param p_selected Selected status
	 */
	void	setSelected(bool p_selected)	{ a_wave->setSelected(p_selected); }

	/**
	 * Show last begin status (forward to waveform widget)
	 * @param p_show Show last begin status
	 */
	void	showLastBegin(bool p_show)		{ a_wave->showLastBegin(p_show); }

	/**
	 * Waveform widget width accessor
	 * @return Waveform widget width
	 */
	int		getWidth()						{ return a_wave->getWidth(); }

	/**
	 * Triggers waveform peaks computation & display
	 */
	void	computePeaks(bool silentMode)					{ a_wave->computePeaks(silentMode); }

	/**
	 * Internal IODevice accessor
	 * @param inDevice	New IODevice (pointer)
	 */
	void	setDevice(IODevice *inDevice)	{ device = inDevice; }

	/**
	 * Internal IODevice accessor
	 * @return Current IODevice (pointer)
	 */
	IODevice*	getDevice()	{ return device; }

	/**
	 * Channels count accessor (waveform widget)
	 * @return Channels count
	 */
	int		getNumChannel()					{ return a_wave->getNumChannel(); }

	/**
	 * Channels count accessor (waveform widget)
	 * @param p_num	Channels count
	 */
	void	setNumChannel(int p_num)		{ a_wave->setNumChannel(p_num); }

	/**
	 * Peaks directory accessor
	 * @return Peaks directory
	 */
	const char* getPeaksDirectory() { return a_wave->getPeaksDirectory(); }

	/**
	 * Peaks directory accessor
	 * @param p_path Peaks directory
	 */
	void setPeaksDirectory(const char* p_path) { a_wave->setPeaksDirectory(p_path); }

	/**
	 * Last begin segment track
	 * @param p_track Last begin segment track (pointer)
	 */
	void setLastBeginSegmentTrack(SegmentTrackWidget* p_track) { a_wave->setLastBeginSegmentTrack(p_track); }

	/**
	 * Waveform color accessor
	 * @param mode	Color context
	 * @param color	Color value
	 */
	void set_color(Glib::ustring mode, Glib::ustring color) { if(a_wave)a_wave->set_color(mode,color);}

	/**
	 * Waveform color accessor
	 * @param mode	Color context
	 * @return Color (according to context)
	 */
	Glib::ustring get_color(Glib::ustring mode) 	{ if(a_wave)return a_wave->get_color(mode);else return "" ;}

	/**
	 * Filters accessor
	 * @return Filters (vector)
	 */
	vector<AbstractFilter*>		getFilters()		{ return filters; }

	/**
	 * Specifies video mode
	 * @param value		True or false
	 */
	void setVideoMode(bool value) { video_mode = value ; if (a_wave) a_wave->setVideoMode(value) ;}

	/**
	 * Accessor to the AudioWaveFormWidget object
	 * @return
	 */
	AudioWaveformWidget* getAudioWaveformWidget() 	{ return a_wave; }

private:
	// -- Audio Objects --
	AudioWaveformWidget*				a_wave;
	AudioVZoomControlWidget*			a_avzc;
	AudioVolumeControlWidget*			a_avc;
	AudioPitchControlWidget*			a_apc;
	IODevice*							device;

	// -- GTK Widgets --
	Gtk::Entry*							a_entry;
	Gtk::Image*							a_imageAudioOn;
	Gtk::Image*							a_imageAudioOff;
	Glib::RefPtr<Gdk::Pixbuf>			a_pixbufAudioOn;
	Glib::RefPtr<Gdk::Pixbuf>			a_pixbufAudioOff;
	Glib::RefPtr<Gtk::IconTheme>		theme;
	Gtk::HBox*							a_mainBox;
	Gtk::Button*						a_activateButton;
	Gtk::Frame*							a_modificationFrame;
	Gtk::Frame*							a_offsetFrame;
	IcoPackImageButton					a_expandButton;
	Gtk::Tooltips						a_tooltips;

	// -- Filters --
	SoundTouchFilter*					st_filter;
	VolumeFilter*						v_filter;
	MuteFilter*							m_filter;
	vector<AbstractFilter*>				filters;

	// -- Internals --
	float	a_startSelection;
	float	a_endSelection;
	bool	a_expanded;
	bool	a_activated;
	bool	a_selectable;
	float	a_offset;
	float	a_cursor;
	int		channelID;

	bool video_mode ;

	// -- Signals --
	sigc::signal<void, bool>	a_signalActivated;
	sigc::signal<void, float>	a_signalOffsetUpdated;
	sigc::signal<void>			a_signalFocusIn;
	sigc::signal<void, bool>	a_signalExpanded;

};

} // namespace

#endif // __HAVE_AUDIOTRACKWIDGET__
