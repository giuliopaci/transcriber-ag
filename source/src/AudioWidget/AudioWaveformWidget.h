/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_AUDIOWAVEFORMWIDGET__
#define __HAVE_AUDIOWAVEFORMWIDGET__

#include <gtkmm/drawingarea.h>
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <cairomm/context.h>

#include <gdkmm/gc.h>

#include "AudioWidget/AudioWidget.h"
#include "MediaComponent/io/IODevice.h"

namespace tag {

class	SegmentTrackWidget;
class	AudioWaveformWidget;

// --- Thread Entry Point ---
bool	AudioWaveformWidget_drawNow(AudioWaveformWidget* wave);


/**
 * @class	AudioWaveformWidget
 * @ingroup	AudioWidget
 * This class implements audio waveform drawing / controls
 */
class AudioWaveformWidget : public Gtk::DrawingArea
{
public:
	/** Default Constructor
	 * @param device	Inherited IODevice (from AudioTrackWidget)
	 * @param peaks		Peaks array (if NULL, peaks will be computed)
	 */
	AudioWaveformWidget(IODevice* device, float* peaks);

	virtual ~AudioWaveformWidget();

	// -------------------
	// --- GTK Signals ---
	// -------------------

	/**
	 * GTK signal - CursorChanged
	 * @return CursorChanged signal
	 */
	sigc::signal<void, float> signalCursorChanged()				{ return a_signalCursorChanged; }

	/**
	 * GTK signal - SelectionChanged
	 * @return SelectionChanged signal
	 */
	sigc::signal<void, float, float> signalSelectionChanged()	{ return a_signalSelectionChanged; }

	/**
	 * GTK signal - AudioTrackSelected
	 * @return AudioTrackSelected signal
	 */
	sigc::signal<void> signalAudioTrackSelected()				{ return a_signalAudioTrackSelected; }

	/**
	 * GTK signal - PopulatePopup
	 * @return PopulatePopup signal
	 */
	sigc::signal<void, int, int> signalPopulatePopup()			{ return a_signalPopulatePopup; }

	/**
	 * GTK signal - SizeChanged
	 * @return SizeChanged
	 */
	sigc::signal<void, int> signalSizeChanged()					{ return a_signalSizeChanged; }

	/**
	 * GTK signal - Selecting
	 * @return Selecting signal
	 */
	sigc::signal<void, bool, bool> signalSelecting()			{ return a_signalSelecting; }

	/**
	 * GTK signal - Emitted at peak loading end
	 * <b>parameter boolean:</b> success or failure
	 */
    sigc::signal<void, bool> signalPeaksReady() 				{ return a_signalPeaksReady; }

    /**
     * GTK signal - Emitted when peak size is invalid
     * <b>parameter boolean:</b> success or failure
     */
    sigc::signal<void, bool> signalInvalidPeakSize()            { return a_signalInvalidPeakSize; }

	// -------------------
	// --- DrawingArea ---
	// -------------------

	/**
	 * Draws single audio peak
	 * @param peakIndex	Peak index
	 */
	void	drawPoint(int peakIndex);

	/**
 	 * Draws selection tooltip
	 */
	void	selectionTip();

	/**
	 * Draws cursor
	 * @param p_cursor	New cursor position
	 */

	void	setCursor(float p_cursor);

	/**
	 * Draws selection area
	 * @param p_beginSelection	Selection Start (in seconds)
	 * @param p_endSelection	Selection End (in seconds)
	 */
	void	setSelection(float p_beginSelection, float p_endSelection);

	/**
	 * Updates scroll value and refreshes display
	 * @param p_cursorScroll	New scroll value
	 */
	void	setScroll(float p_cursorScroll);

	/**
	 * Updates focused track status
	 * @param value	True if this waveform is currently active, false otherwise.
	 */
	void	setCurrent(bool value)			{ stereo = value; }

	/**
	 * Waveform loading status
	 * @return True if waveform is loaded, false otherwise.
	 */
	bool	isReady()						{ return !a_loading; }

	/**
	 * This method initializes peaks retrieving / computing.\n
	 * If peaks file are available, they are automatically loaded.\n
	 * Otherwise, peaks will be computed.\n
	 * Once computing is done, waveform is displayed.
	 * @param	silentMode	True when using a SilentHandler
	 */
	void	computePeaks(bool silentMode);

	/**
	 *
	 * This method displays waveform once peaks are ready
	 * @param ok	True if successful, false for error
	 * @return
	 */
	bool	peaksReady(bool ok);

	/**
	 * Checks whether peaks have been computed
	 * @return	True if peaks are ready, false otherwise
	 */
	bool	checkPeaksLoading();

	
	/**
	 * Updates GUI to reflect waveform focus
	 * @param p_selected	Waveform selected flag
	 */
	void	setSelected(bool p_selected);

	/**
	 * Updates GUI to reflect waveform activation status
	 * @param enabled	Waveform activation status
	 */
	void	setEnabled(bool enabled)		{ a_enabled = enabled; }

	/**
	 * Widget width accessor
	 * @return Widget width (in pixels)
	 */
	int		getWidth();

	/**
	 * Zoom accessor
	 * @param p_factor	New zoom factor
	 */
	void	setZoom(int p_factor);

	/**
	 * Vertical zoom accessor
	 * @param p_factor	New vertical zoom factor
	 */
	void	setVZoom(float p_factor)		{ a_currentVZoom = p_factor; queue_draw(); }

	/**
	 * Vertical zoom accessor
	 * @return Current vertical zoom factor
	 */
	float	getVZoom()						{ return a_currentVZoom; }

	/**
	 * Updates GUI to reflect offset change
	 * @param p_offset	New offset value
	 */
	void	setOffset(float p_offset)		{ a_offset = p_offset; queue_draw(); }

	/**
	 * Updates GUI to reflect showLastBegin flag
	 * @param p_show	ShowLastBegin flag
	 */
	void	showLastBegin(bool p_show)		{ a_showLastBegin = p_show; queue_draw(); }

	/**
	 * Updates GUI to reflect new cursor size
	 * @param p_pix	New cursor size (in pixels)
	 */
	void	setCursorSize(int p_pix)		{ a_cursorSize = p_pix; queue_draw(); }

	/**
	 *	Updates GUI to reflect new cursor color
	 * @param p_color	New cursor color
	 */
	void	setCursorColor(string p_color)
	{
		ColorsCfg::color_from_str_to_rgb(p_color, color_cursor) ;
		queue_draw();
	}

	/**
	 * Maximum zoom accessor
	 * @param p_zoomMax	New maximum zoom value
	 */
	void	setZoomMax(int p_zoomMax)		{ a_zoomMax = p_zoomMax; }

	/**
	 * Minimum selection (in pixels)\n
	 * Selections lower than this value will be discarded.
	 * @ param p_minpix	Minimum pixels size
	 */
	void	setMinPixels(int p_minpix)		{ min_pixels_sel = p_minpix; }

	/**
	 * Channel ID accessor
	 * @return Channel ID
	 */
	int		getNumChannel()					{ return a_numChannel; }

	/**
	 * Channel ID accessor
	 * @param p_num	Channel ID
	 */
	void	setNumChannel(int p_num)		{ a_numChannel = p_num; }

	/**
	 * Streaming mode parameters
	 * @param mode	Streaming mode
	 * @param rtsp	Rtsp url
	 * @param local	Local path
	 */
	void	setStreaming(bool mode, string rtsp, string local)
	{
		a_streaming	= mode;
		a_rtsp_path	= rtsp;
		a_path		= local;
	}

	/**
	 * Selectable Mode
	 * @param p_selectable	Selectable mode
	 */
	void	setSelectable(bool p_selectable) { a_selectable = p_selectable; }

	/**
	 * Affects SegmentTrack widget linked with lastBeginSegmentTrack\n
	 * Refreshes display afterwards.
	 * *@param p_track Target segment track (pointer)
	 */
	void	setLastBeginSegmentTrack(SegmentTrackWidget* p_track)	{a_lastBeginSegmentTrack = p_track; queue_draw(); }

	/**
	 * Peaks directory accessor
	 * @param p_path	Peaks directory
	 */
	void	setPeaksDirectory(const char* p_path) { a_peaksDirectory = p_path; }

	/**
	 * Peaks directory accessor
	 * @return Peaks directory
	 */
	const char* getPeaksDirectory() { return a_peaksDirectory; }


	/**
	 * SAVE_PEAKS static accessor
	 * @param b	New value
	 */
	static void setSavePeaks(bool b) { SAVE_PEAKS = b; }

	/**
	 * ABSOLUTE_PEAKS_NORM static accessor
	 * @param b	New value
	 */
	static void useAbsolutePeaksNorm(bool b) { ABSOLUTE_PEAKS_NORM = b; }

	/**
 	 * Normalize whole peaks array
	 * @param ws "Window size", used to prevent array overflows.
	 */
	void normalizePeaks(int ws) ;

	/**
	 * Loads peaks file into a float array
	 * @param in_device
	 * @param chanID
	 * @param silentMode	True when using a SilentHandler
	 * @return Peaks array (float)
	 */
	float* retrieveStreamPeaks(IODevice* in_device, int chanID, bool silentMode);

	/**
	 * Computes peaks into a float array
	 * @param in_device
	 * @param chanID
	 * @param silentMode	True when using a SilentHandler
	 * @return Peaks array (float)
	 */
	float* computeStreamPeaks(IODevice* in_device, int chanID, bool silentMode);

	/**
 	 * Waveform color accessor
	 * @param mode	Color context
	 * @param color	Color value (hexadecimal format)
	 */
	void set_color(Glib::ustring mode, Glib::ustring color) ;

	/**
 	 * Waveform color accessor
	 * @param mode	Color context
	 * @return 		Color value (hexadecimal format)
	 */
	Glib::ustring get_color(Glib::ustring mode) ;

	/**
	 * Sets the video mode
	 * @param value			True for video mode, false otherwise
	 */
	void setVideoMode(bool value) { video_mode = value ;}

	static bool SAVE_PEAKS;				/**< If true, computed peaks will be saved in a peaks file. */
	static bool ABSOLUTE_PEAKS_NORM;	/**< If true, peaks will be normalized. */

private:
	const static int BUFFER_SIZE = AUDIO_BUFFER_SIZE;
	const static int MAX_PEAK = 32767;

	int		a_w;
	int		a_h;
	int		a_beginVisible;
	int		a_endVisible;
	int		a_xPrec;
	int		a_lastPressedButton;
	int		a_samplesCount;
	int		a_samplingRate;
	int		a_currentZoom;
	int		a_zoomMax;
	int		a_cursorSize;
	int		min_pixels_sel;
	int		p_offset;
	int		a_peaksLength;

	float*	a_currentPeaks;
	float*	a_maxPeaks;

	float	a_duration;
	float	a_cursorScroll;
	float	a_currentSample;
	float	a_selection1;
	float	a_selection2;
	float	a_beginSelection;
	float	a_endSelection;
	float	a_secsPerPeak;
	float	a_offset;
	float	lastSegment;
	float	beginSelection, endSelection;
	float	a_currentVZoom;

	bool	a_selectable;
	bool	a_enabled;
	bool	a_showLastBegin;
	bool	a_selected;
	bool	a_loading;
	bool	a_abortLoading;
	bool	a_cursorIn;
	bool	a_trackingMode;
	bool	stereo;
	bool	drawnOnce;
	bool	a_streaming;
	bool	a_backextent;
    bool    video_mode ;
	bool	a_emit_on_release;

	string	a_rtsp_path;
	string	a_path;
	string	a_cursorColor;

	SegmentTrackWidget* a_lastBeginSegmentTrack;

	bool	on_expose_event(GdkEventExpose* p_event);
	bool	on_button_press_event(GdkEventButton* p_event);
	bool	on_motion_notify_event(GdkEventMotion* p_event);
	bool	on_button_release_event(GdkEventButton* p_event);
	bool	on_enter_notify_event(GdkEventCrossing* p_event);
	bool	on_leave_notify_event(GdkEventCrossing* p_event);
	void	on_size_allocate(Gtk::Allocation& p_allocation);

	sigc::signal<void, float>			a_signalCursorChanged;
	sigc::signal<void, float, float>	a_signalSelectionChanged;
	sigc::signal<void>					a_signalAudioTrackSelected;
	sigc::signal<void, int, int>		a_signalPopulatePopup;
	sigc::signal<void, int>				a_signalSizeChanged;
	sigc::signal<void, bool, bool>		a_signalSelecting;

	sigc::signal<void, bool>			a_signalPeaksReady;
    sigc::signal<void, bool>            a_signalInvalidPeakSize;

	// -- Cairo Zone --
	Cairo::RefPtr<Cairo::Context>	 	cr;
	Glib::RefPtr<Gdk::Window>			window;
	Cairo::FontOptions					font_options;
	Cairo::TextExtents					extents;

	std::vector<double> color_wave_bg_active ;
	std::vector<double> color_wave_bg ;
	std::vector<double> color_wave_fg ;
	std::vector<double> color_selection ;
	std::vector<double> color_disabled ;
	std::vector<double> color_tip_bg ;
	std::vector<double> color_tip_fg ;
	std::vector<double> color_cursor ;
	std::vector<double> color_segmentEnd ;

	// Global variables
	int w, h, w2, h2;
	int	clip_x, clip_width;
	int begin, end, cursor;

	Glib::Mutex a_mut1;
	Glib::RefPtr<Gdk::GC> gc;
	Glib::Timer	tim;
	const char* a_peaksDirectory;
	int			a_numChannel;
    int         a_arraySize;
	IODevice*	device;
};

} // namespace

#endif // __HAVE_AUDIOWAVEFORMWIDGET__

