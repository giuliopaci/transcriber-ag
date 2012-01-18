/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @file
 * @defgroup VideoComponent VideoComponent
 */

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <gtkmm.h>

#include "Common/widgets/GeoWindow.h"

#include "MediaComponent/base/Guesser.h"
#include "VideoBuffer.h"
#include "ToolBox.h"

#define BUFFER_MIN_W 40
#define BUFFER_MIN_H 30

namespace tag {

class AudioSignalView;

/**
 * @class	VideoWidget
 * @ingroup	VideoComponent
 *
 * This widget implements a graphical video player with standard & advanced controls:\n
 */

class VideoWidget : public Gtk::Dialog, public GeoWindow
{
public:
	/**
 	 * Default constructor
	 *
	 * @param with_toolbar 		If true, add a constrol toolbar and corresponding actiongroup is created
	 */
	VideoWidget(bool with_toolbar=false);
	virtual ~VideoWidget();

	// ---------------------
	// --- Main Controls ---
	// ---------------------

	/**
	 * Starts / stops video playback
	 * @param mode	If true, playback starts, else playback stops.
	 */
	void		setPlayback(bool mode);

	/**
	 * Pauses playback
	 */
	void		pause();

	/**
	 * Stops playback
	 */
	void		stop();

	/**
	 * Seeks N frames backward
	 * @param nframe Backward step (number of frames)
	 */
	void		seekMinus(int nframe);

	/**
	 * Seeks N frames forward
	 * @param nframe Forward step (number of frames)
	 */
	void		seekPlus(int nframe);

	/**
	 * Seeks to next video frame
	 * @return True on success, false if seek failed.
	 */
	bool		nextFrame();

	/**
	 * Seeks at timestamp (frame precision)
	 * @param timestamp	New timestamp (in seconds)
	 */
	void		seek(double timestamp);

	/**
	 * Opens a new video file
	 * @param in_path	Video file path
	 * @return	True for success, False otherwise
	 */
	bool		openFile(string in_path);

	/**
	 * Moves navigation bar to given offset
	 * @param offset	New offset (in seconds)
	 * @param stop		If set to true, playback will be stopped before seeking.
	 */
	void		setScaleOffset(double offset, bool stop);

	/**
	 * Updates selection range
	 * @param s_begin	Selection start
	 * @param s_end		Selection end
	 */
	void		setSelection(float s_begin, float s_end)
	{
		a_beginSelection	= s_begin;
		a_endSelection		= s_end;
	}
	/**
	 * Displays given decoded frame
	 * @param frame	New decoded frame
	 * @return True on success, false otherwise.
	 */
	bool		updateVideoBuffer(MediumFrame* frame);

	/**
	 * Loads and displays a frame at native resolution (used for scaling)
	 */
	void		loadNativePixbuf();

	/**
	 * Callback - Called when a keyframe (XML) has been reached
	 */
	void 		onKeyFrame();

	/**
	 * Sets parent AudioSignalView reference (used to connect signals)
	 * @param signalView	Parent AudioSignalView reference
	 */
	void		setSignalView(AudioSignalView* signalView);

	/**
	 * Returns playback status
	 * return Playback status
	 */
	bool		isPlaying()		{ return playback; }

	/**
	 * Synchronizes both audio & video streams to given timestamp
	 * @param timestamp	Reference timestamp for sync
	 */
	bool		synchronize(double timestamp);

	/**
	 * Returns current video framerate (fps)
	 * @return Video framerate
	 */
	double		getFPS()		{ return v_device->m_info()->video_fps; }

	/**
 	 * Refreshes video display
	 */
	void		refresh();

	/**
	 * Resizes buffer to actual widget size
	 */
	bool		resizeBuffer();

	// ------------------------
	// --- Events Callbacks ---
	// ------------------------

	/**
	 * Callback - Seek
	 * @param Nmode		If true, seek will be ruled by a frame number entry (N)
	 * @param rewind	If true, seek will be backward, otherwise it will be forward.
	 */
	void 			onSeek(bool Nmode, bool rewind);

	/**
	 * Event callback - Delete
	 * @param event	Gdk event
	 * @return True on success, false otherwise.
	 */
	virtual bool	on_delete_event(GdkEventAny* event);

	/**
	 * Exposent event callback
	 * @para	Event pointer
	 * @return	True if handled, false otherwise
	 */
	bool			onExposeEvent(GdkEventExpose*);

	/**
	 * Event callback - FocusIn
	 * @param event	Gdk focus event
	 * @return True on success, false otherwise.
	 */
	bool			on_focus_in_event(GdkEventFocus* event);

	/**
	 * Event callback - FocusOut
	 * @param event	Gdk focus event
	 * @return True on success, false otherwise.
	 */
	bool			on_focus_out_event(GdkEventFocus* event);

	/**
	 * Event callback - VideoResized
	 * @param event	Gdk expose event
	 * @return True on success, false otherwise.
	 */
	bool			onVideoResized(GdkEventExpose* event);

	/**
	 * Event callback - SizeRequest
	 * @param req object
	 */
	void			onSizeRequest(Gtk::Requisition* req);

	/**
	 * Event callback - SeekValueChanged
	 * @param scroll	Gtk scroll type
	 * @param val		New seek value
	 * @return True on success, false otherwise.
	 */
	bool			onSeekValueChanged(Gtk::ScrollType scroll, double val);

	/**
	 * Event callback - Navigation scale updated
	 */
	void			onScaleUpdated();


	// -------------------------
	// --- Signals Accessors ---
	// -------------------------

	/**
	 * Signal Accessor - seekInVideo
	 * @return seekInVideo signal
	 */
	sigc::signal<void, double>			videoSeeked()		{ return seekInVideo; }

	/**
	 * Signal Accessor - videoPlayPause
	 * @return videoPlayPause signal
	 */
	sigc::signal<void, bool>			videoPlayPaused()	{ return videoPlayPause; }

	/**
	 * Signal Accessor - onTimeChange
	 * @return onTimeChange signal
	 */
	sigc::signal<void, double>			onTimeChanged()		{ return onTimeChange; }

	/**
	 * Signal Accessor - m_signalKeyframe
	 * @return m_signalKeyframe signal
	 */
	sigc::signal<void, double>			signalKeyframe()	{ return m_signalKeyframe; }

	/**
	 * Signal Accessor - m_signalError
	 * @return m_signalError signal
	 */
	sigc::signal<void, Glib::ustring>	signalError()		{ return m_signalError; }

	/**
	 * Signal Accessor - m_signalCloseButton
	 * @return m_signalCloseButton signal
	 */
	sigc::signal<void>					signalCloseButton()		{ return m_signalCloseButton; }

	// ----------------------
	// --- Static Methods ---
	// ----------------------

	/**
	 * Extracts converted RGB frame according to given parameters
	 * @param time		Requested timestamp (frame precision)
	 * @param device	Current stream's IODevice
	 * @param width		Output pixbuf width
	 * @param height	Output pixbuf height
	 * @return New Gdk pixbuf
	 */
	static Glib::RefPtr<Gdk::Pixbuf> getPixbufFromFrame(float time, IODevice* device, int width, int height) ;

	/**
	 * Normalizes dimensions, according to stream aspect ratio
	 * @param[in,out] v_width	Width to be normalized
	 * @param[in,out] v_height	Height to be normalized
	 * @param device			Current stream's IODevice
	 */
	static void normalizeDimensions(int& v_width, int& v_height, IODevice* device) ;


	// --------------------
	// --- Action group ---
	// --------------------

	/**
	 * ActionGroup Accessor
	 * @return UI ActionGroup
	 */
	Glib::RefPtr<Gtk::ActionGroup>	getActionGroup()	{ return ui_actions ; }

	/**
	 * UI Info Accessor
	 * @return UI string description
	 */
	Glib::ustring					getUIInfo()			{ return ui_info ; }

	/**
	 * Get menu items for Ui
	 * @return		UI strings
	 */
	std::vector<Glib::ustring> getUIMenuItems() ;

	/**
	 * Completes the given action group with all video player action
	 * @param action_group		Reference on an existing action group
	 */
	void completeActionGroup(Glib::RefPtr<Gtk::ActionGroup> action_group) ;

	/*** Geometry interface ***/
	virtual void saveGeoAndHide() ;
	virtual int loadGeoAndDisplay(bool rundlg=false) ;

protected:
	/**
	 * Initializes video player base widgets
	 */
	void		initWidgets();

	/**
	 * Initializes video player menus
	 */
	void		initMenus();

	/**
	 * Initializes video player toolbars
	 */
	void		initToolbars();

	/**
	 * Initializes video player signals
	 */
	void		initSignals();

	/**
	 * Initializes widgets layout
	 */
	void		buildUI();


	// -----------------
	// --- Callbacks ---
	// -----------------

	/**
	 * Callback - Play button clicked
	 */
	void		playClicked();

	/**
	 * Converts absolute time to AGTime
	 * @param timestamp	Timestamp
	 */
	Glib::ustring	convertToTime(double timestamp);

	/**
	 * Format time for display purpose
	 * @param ts	Time in second
	 * @return		Time string format
	 */
	Glib::ustring formatDisplayTime(double ts) ;

	/**
	 * Converts to AGTime to absolute time
	 * @param AGTime			Timestamp in AGTime format
	 * @param force_previous	If true, return systematically previous frame timestamp
	 * @param device			Current IODevice reference
	 */
	string			fromAGTIMEtoTXML(float AGTime, bool force_previous, IODevice* device);

private:
	// -- Signals --
	sigc::signal<void, bool>			videoPlayPause;
	sigc::signal<void, double>			seekInVideo;
	sigc::signal<void, double>			onTimeChange;
	sigc::signal<void, double>			m_signalKeyframe;
	sigc::signal<void, Glib::ustring>	m_signalError;
	sigc::signal<void>					m_signalCloseButton;

	// Child Widgets
	ToolBox		*toolbox;
	Gtk::HScale	*b_scale;
	Gtk::Entry	*b_entry;

//	Gtk::VBox		vbox, canvas_box;
	Gtk::VBox		canvas_box;
	Gtk::Button		button;
	Gtk::Window*	toplevel;
	Glib::RefPtr<Gtk::UIManager>	ui_manager ;
	Glib::RefPtr<Gtk::ActionGroup>	ui_actions ;
	Glib::RefPtr<Gtk::RadioAction>	ui_radio ;
	Glib::ustring 					ui_info ;

	Gtk::EventBox eventBufferBox ;

	// -- Video Controller --
	IODevice*					v_device;
	Glib::RefPtr<Gdk::Pixbuf>	v_pixbuf;
	VideoBuffer*				v_buffer;
	sigc::connection			sigc_conn;
	sigc::connection			sigc_rgb;
	uint8_t*					inData;

	// -- Related objects --
	AudioSignalView*			m_signalView;
	MediumFrame*				currentFrame;

	// -- States --
	bool 			playback;
	bool			firstResizeDone;
	bool			resizeIn;
	bool			toolbarMode;
	bool			first_sync;
	double			last_audio_offset;
	double			video_ts;
	double			frame_duration;
	double			last_seek_time, seek_time;
	double			video_offset;
	float			a_beginSelection, a_endSelection;
	int				buffer_width;
	int				buffer_height;
	int				new_video_width;
	int				new_video_height;

	void			createActionGroup();
	void			createUIInfo();

	/*** Geometry interface ***/
	virtual void	getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
	virtual void	setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel) ;
	virtual void	getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
	virtual Glib::ustring getWindowTagType()  ;
};

} // namespace

#endif

