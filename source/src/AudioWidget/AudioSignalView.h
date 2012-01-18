/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_AUDIOSIGNALVIEW__
#define __HAVE_AUDIOSIGNALVIEW__

#include <vector>

#include "AudioWidget.h"

// -- MediaComponent --
#include "MediaComponent/base/Guesser.h"
#include "io/PortAudioStream.h"
#include "rtsp/RTSPSession.h"

#include "Common/icons/IcoPackImageButton.h"

using namespace std;

namespace tag {

/**
 * @class	AudioSignalView
 * @ingroup	AudioWidget
 * Main class in AudioWidget module
 */
class AudioSignalView : public Gtk::Frame
{
	public:
		/**
		 * Default constructor
		 * @param useVideo 	If true, opened medium is a video file (default : false)
		 */
		AudioSignalView(bool useVideo = false);
		virtual ~AudioSignalView();

		/**
		 * Gets selection boundaries
		 * @param[out] begin	Selection start (in seconds)
		 * @param[out] end		Selection end (in seconds)
		 */
		bool getSelection(float& begin, float& end)
		{
			begin	= a_beginSelection;
			end		= a_endSelection;
			return ( begin != -1.0 && end != -1.0);
		}

		/**
		 * Returns signal length
		 * @return Signal length
		 */
		float	signalLength();

		/**
		 * Sets RTSP channels count (plugin context)
		 * @param channels	RTSP channels count
		 */
		void	setRtspChannels(int channels)	{ m_channels	= channels; }

		/**
		 * Returns audio tracks count
		 * @return Audio tracks count
		 */
		int		getNbSignalTracks()			{ return a_audioTracks.size(); }

		/**
		 * Returns selection state
		 * @return True if a selection exists, false otherwise.
		 */
		bool	hasSelection()				{ return ( a_beginSelection != -1.0 && a_endSelection != -1.0 ); }


		/**
		 * Updates current status
		 * @param value Current status
		 */
		void	setCurrent(bool value)
		{
			a_audioTracks[0]->setCurrent(true);
			a_audioTracks[1]->setCurrent(true);
		}

		/**
		 * Returns stream status
		 * @return Stream status
		 */
		bool is_playing()
		{
			return a_play;
//			return a_isAlive;
		}


		/**
		 * Main playback method (THREADED)
		 */
		void play();

		/**
		 * GTK timer callback - Updates cursor at regular intervals (in playback)
		 * @return True on success, false otherwise.
		 */
		bool updateCursor();

		/**
		 * GTK timer callback - Updates playback tempo at regular intervals (in playback)
		 * @return True on success, false otherwise.
		 */
		bool updateTempo();

		/**
		 * GTK timer callback - Updates various informations at regular intervals (in playback)
		 * @return True on success, false otherwise.
		 */
		bool updateInfos();

		/**
		 * Return current audio settings (in a serialized csv form)
		 * @return Serialized settings string
		 */
		string getCurrentAudioSettings();

		/**
		 * Restores audio settings (from TAG files)
		 * @param val	Settings serialized string
		 */
		void setAudioSettings(const string& val);

		/**
		 * Terminates current playback thread (destruction)
		 * @param emitSignal 	False for preventing signal emission
		 * 						(true by default, shoudl be set to false only
		 * 						in closing or destruction actions)
		 */
		void	terminatePlayThread(bool emitSignal=true);

		/**
		 * Initializes a new playback thread\n
		 * If a thread is already running, it will be terminated.
		 */
		void	runPlayThread();

		/**
		 * Returns GTK actions group
		 * @return GTK actions group
		 */
		Glib::RefPtr<Gtk::ActionGroup> getActionGroup()	{ return a_actions; }

		/**
		 * Returns backward start delay
		 * @return Backward start delay
		 */
		float	getStartBackwardDelay()					{ return a_backward_start_delay; }

		/**
		 * Updates backward start delay
		 * @param d New delay
		 */
		void	setBackwardStartDelay(float d)			{ a_backward_start_delay = d; }

		/**
		 * Returns quickmove delay
		 * @return Quickmove delay
		 */
		float	getQuickmoveDelay()						{ return a_quickmove_delay; }

		/**
		 * Updates quickmove delay
		 * @param d New delay
		 */
		void	setQuickmoveDelay(float d)				{ a_quickmove_delay = d; }

		/**
		 * Returns current cursor position (accurate only if playback is off)
		 * @return Cursor position
		 */
		float	getCursor();

		/**
		 * Returns current cursor position, in a playback context
		 * @return Cursor position
		 */
		float	getActualCursor();

		/**
		 * Updates cursor position, and forwards it to every subtrack
		 * @param p				New cursor position
		 * @param emit_signal	True for emitting signal to other componened
		 * @param sync_video	True for sycnhronizing video elemenrs
		 */
		void	setCursor(float p, bool emit_signal=true, bool sync_video=false);

		/**
		 * Sets TAG file path related to RTSP stream
		 * @param str	Local TAG file
		 */
		void	setStreamFile(string str)	{ m_local_file	= str; }

		// -----------------------
		// --- Signal Handlers ---
		// -----------------------

		/**
		 * GTK signal - PlayedPaused
		 * @return PlayedPaused signal
		 */
		sigc::signal<void, bool> signalPlayedPaused()	{ return a_signalPlayedPaused; }

		/**
		 * GTK signal - TempoChanged
		 * @return TempoChanged signal
		 */
		sigc::signal<void, float> signalTempoChanged()	{ return a_atc->signalValueChanged(); }

		/**
		 * GTK signal - ZoomChanged
		 * @return ZoomChanged signal
		 */
		sigc::signal<void, int> signalZoomChanged()		{ return a_signalZoomChanged; }

		/**
		 * GTK signal - CursorChanged
		 * @return CursorChanged signal
		 */
		sigc::signal<void, float> signalCursorChanged() { return a_signalCursorChanged; }

		/**
		 * GTK signal - InfosUpdated
		 * @return InfosUpdated signal
		 */
		sigc::signal<void, string> signalInfosUpdated() { return a_signalInfosUpdated; }

		/**
		 * GTK signal - TrackActivated
		 * @return TrackActivated signal
		 */
		sigc::signal<void, bool> signalTrackActivated() { return a_signalTrackActivated; }

		/**
		 * GTK signal - SelectionChanged
		 * @return SelectionChanged signal
		 */
		sigc::signal<void, float, float> signalSelectionChanged() { return a_signalSelectionChanged; }

		/**
		 * GTK signal - PopulatePopup
		 * @return PopulatePopup signal
		 */
		sigc::signal<void, int, int, int, Gtk::Menu*> signalPopulatePopup() { return a_signalPopulatePopup; }

		/**
		 * GTK signal - TracksScrolled
		 * @return TracksScrolled signal
		 */
		sigc::signal<void, float> signalTracksScrolled() { return a_signalTracksScrolled; }

		/**
		 * GTK signal - SegmentModified
		 * @return SegmentModified signal
		 */
		sigc::signal<void, const string&, float, float, SegmentTrackWidget* > signalSegmentModified() { return a_signalSegmentModified; }

		/**
		 * GTK signal - AudioTrackSelected
		 * @return AudioTrackSelected signal
		 */
		sigc::signal<void, int> signalAudioTrackSelected() { return a_signalAudioTrackSelected; }

		/**
		 * GTK signal - signalPeaksReady
		 * <b>param boolean:</b> 	True if successfully proceded, False otherwise
		 */
		sigc::signal<void, bool> signalPeaksReady() { return a_signalPeaksReady ; }

		/**
		 * GTK signal - seek video
		 * <b>param double:</b> 	Video timecode
		 */
		sigc::signal<void, double>			seekVideo()		{ return a_seekVideo; }

		/**
		 * GTK signal - sync video
		 * <b>param double:</b>  Video timecode
		 * <b>param bool:</b>
		 */
		sigc::signal<void, double, bool>	syncVideo() 	{ return a_syncVideo; }

		/**
		 * GTK signal - sync video
		 * <b>param double:</b>  Video timecode
		 * <b>param bool:</b>	stop on click value
		 */
		sigc::signal<void, float, float>	syncSelection()	{ return a_videoSelection; }

		/**
		 * GTK signal - stop video signal
		 */
		sigc::signal<void>					stopVideo()		{ return a_stopVideo; }

		sigc::signal<void,int,float>		signalDelayChanged()		{ return a_signalDelayChanged ; }

		// ------------------------
		// --- Callback Methods ---
		// ------------------------

		/**
		 * Saves selected signal portion to external file\n
		 * If there's no selection, the whole medium is saved\n
		 * Export format : WAV
		 * @param start		Start offset
		 * @param end		End offset
		 * @param path		Output path
		 * @param warnings	If true, graphical popups are enabled.
		 */
		bool saveSelection(float start, float end, char* path, bool warnings);

		/**
		 * GTK timer callback - Updates various informations at regular intervals (in playback)
		 * @return True on success, false otherwise.
		 */
		bool updateInfos2();

		/**
		 * Callback - PlayPause button clicked
		 */
		void onPlayPauseClicked();

		/**
		 * Callback - Stop button clicked
		 */
		void onStopClicked();

		/**
		 * Callback - Tempo changed
		 * @param p_factor	New tempo value
		 */
		void onTempoChanged(float p_factor);

		/**
		 * Callback - ZoomIn button clicked
		 * @param ev	Gdk event button
		 */
		bool onZoomInClicked(GdkEventButton* ev);

		/**
		 * Callback - ZoomOut button clicked
		 * @param ev	Gdk event button
		 */
		bool onZoomOutClicked(GdkEventButton* ev);

		/**
		 * ZoomIn clicked (dummy)
		 */
		void onZoomInClickedVoid() { onZoomInClicked(NULL); };

		/**
		 * ZoomOut clicked (dummy)
		 */
		void onZoomOutClickedVoid() { onZoomOutClicked(NULL); };

		/**
		 * Callback - Cursor changed
		 * @param p_cursor New cursor value
		 */
		void onCursorChanged(float p_cursor);

		/**
		 * Callback - Selection changed
		 * @param p_begin	New selection start
		 * @param p_end		New selection end
		 */
		void onSelectionChanged(float p_begin, float p_end);

		/**
		 * Callback - Select All clicked
		 */
		void onSelectAllClicked();

		/**
		 * Callback - Clear selection
		 */
		void onClearSelectionClicked();

		/**
		 * @deprecated
		 * Callback - Show video clicked
		 */
		void onShowVideoClicked();

		/**
		 * @deprecated
		 * Callback - Track scrolled
		 * @param p_type	Scroll type
		 * @param p_value	Scroll value
		 * @return false
		 */
		bool onTrackScrolled(Gtk::ScrollType p_type, double p_value);

		/**
		 * Callback - Track activated
		 * @param p_activated	Activate status
		 */
		void onAudioTrackActivated(bool p_activated);

		/**
		 * Callback - Track offset updated
		 * @param p_offset	New offset value
		 * @param p_track	Related audio track
		 */
		void onAudioTrackOffsetUpdated(float p_offset, AudioTrackWidget* p_track);

		/**
		 * Callback - Track selected
		 * @param p_track	Selected track
		 */
		void onAudioTrackSelected(AudioTrackWidget* p_track);

		/**
		 * Callback - Segment clicked
		 * @param p_track	Segment track
		 */
		void onSegmentClicked(AudioTrackWidget* p_track);

		/**
		 * Callback - Segment modified
		 * @param p_id		Segment Id
		 * @param p_start	New start offset
		 * @param p_end		New end offset
		 * @param p_track	Segment track
		 */
		void onSegmentModified(string p_id, float p_start, float p_end, SegmentTrackWidget* p_track);

		/**
		 * Callback - Populate popup
		 * @param p_x		Popup X coordinate
		 * @param p_y		Popup Y coordinate
		 * @param p_track	Audio track
		 */
		void onPopulatePopup(int p_x, int p_y, AudioTrackWidget* p_track);

		/**
		 * Callback - Loop clicked
		 */
		void onLoopClicked();

		/**
		 * Callback - Key pressed
		 * @param p_event	Gdk key event
		 * @return True
		 */
		bool onKeyPressed(GdkEventKey* p_event);

		/**
		 * Callback - Expanded
		 * @param b	Expanded mode
		 */
		void onExpanded(bool b);

		/**
		 * Callback - Tooltip hidden
		 */
		void onTooltipHidden();

		// ----------------------
		// --- Audio Controls ---
		// ----------------------

		/**
		 * Plays / pauses playback
		 * @param p_play 		Playback status
		 * @param emitSignal 	True for emitting signal, false otherwise
		 */
		void setPlay(bool p_play, bool emitSignal=true);

		/**
		 * Updates tempo factor
		 * @param p_factor		New tempo factor
		 * @param p_upd_widget	If true, widget will be updated.
		 */
		void setTempo(float p_factor, bool p_upd_widget=false);

		/**
		 * @param p_factor		New tempo factor
		 * @param p_upd_widget	If true, widget will be updated.
		 */
		void setTempo2(float p_factor, bool p_upd_widget=false);

		/**
		 * Moves cursor (connected to F1 / F4 keys)
		 * @param p_dist	Move offset (F1 -> -1 / F4 -> 1)
		 */
		void moveCursor(float p_dist);

		/**
		 * Updates current zoom
		 * @param p_zoom 	New zoom factor
		 * @param init		True for initialization call
		 * @return True if zoom is valid, false otherwise.
		 */
		bool setZoom(int p_zoom, bool init=false);

		/**
		 * Updates cursor position (in audio stream only)
		 * @param p_cursor	New cursor position
		 */
		void setCursorAudio(float p_cursor);

		/**
		 * Updates cursor position (graphical)
		 */
		void setCursorTracks(float p_cursor);

		/**
		 * Update cursor position (internal)
		 */
		void setCursorTracks2(float p_cursor);

		/**
		 * Sets new selection range
		 * @param p_begin	New selection start
		 * @param p_end		New selection end
		 */
		void setSelection(float p_begin, float p_end);

		/**
		 * Sets new scroll offset
		 * @param p_cursorScroll	New scroll offset
		 */
		void setScroll(float p_cursorScroll);

		/**
		 * Refreshes all visible tracks
		 */
		void refreshTracks();

		/**
		 * Adds a new audio track to tracks list
		 * @param p_track	Audio track instance (pointer)
		 */
		void addAudioTrack(AudioTrackWidget* p_track);

		/**
		 * Adds a new scale track
		 */
		void addScaleTrack();

		/**
		 * Adds a new audio stream\n
		 * This key method also generate audio tracks, according to channels count
		 * @param inFile		File path
		 * @param passPhrase	Pass phrase (optional, can be NULL)
		 * @return				True for success, False for failure
		 */
		bool addAudioStream(char* inFile, char* passPhrase);

		/**
		 * Add new multiple streams (multi-audio file)\n
		 * Iterates on streams vector, and calls addAudioStream
		 * @param stream_paths	Files vector
		 * @return				True for success, False for failure
		 */
		bool addAudioStreams(const vector<string>& stream_paths);

		/**
		 * Opens an empty stream
		 * @param nbChannels		Number of channels needed
		 * @param length			Length of signal needed
		 * @return					True or false
		 */
		bool addEmptyAudioStream(int nbChannels, double length) ;

		/**
		 * Adds new segment track
		 * @param p_label		Track label
		 * @param p_numTrack	Track number
		 * @param flatMode		True for flatmode (small segments), false otherwise
		 * @return				Pointer on the newly segment track created
		 */
		SegmentTrackWidget* addSegmentTrack(string p_label, int p_numTrack, bool flatMode);

		/**
		 * @deprecated
		 * Adds a new mark track
		 */
		MarkTrackWidget* addMarkTrack();

		/**
		 * Adjusts zoom in order to have the whole signal visible
		 * @return True on success, false otherwise
		 */
		bool setZoomEntire();

		/**
		 * Computes maximum zoom value
		 * @param width	Optional width value (default: 0)
		 */
		void calcZoomMax(int width=0);

		/**
		 * Selects an audio track
		 * @param p_selectedAudioTrack Selected audio track
		 */
		void setSelectedAudioTrack(int p_selectedAudioTrack);

		/**
		 * Returns audio track duration
		 * @param p_audioTrackIndex	Audio track index
		 */
		float getAudioTrackDuration(int p_audioTrackIndex);

		/**
		 * Returns audio track format (WAV / MP3 / ...)
		 * @param p_audioTrackIndex	Audio track index
		 */
		const char* getAudioTrackFormat(int p_audioTrackIndex);

		/**
		 * Returns audio track encoding
		 * @param p_audioTrackIndex	Audio track index
		 */
		const char* getAudioTrackEncoding(int p_audioTrackIndex);

		/**
		 * Returns next segment that cannot be skipped (ex: speech segment)
		 * @param p_secs	Current cursor position
		 */
		float nextNotSkipable(float p_secs);

		/**
		 * Handles end of streams / end of selections.\
		 * Restores cursor positions according to settings (loop, speech only, ...)
		 */
		void backToStart();

		/**
		 * Returns next segment end and delay (if set)
		 * @param p_secs		Current cursor position
		 * @param p_size		Current cursor size (unused)
		 * @param[out] p_delay	Delay that would be applied at segment end (no delay -> -1)
		 * @param[out] p_endSeg	Next segmend end (end of track -> -1)
		 */
		void endSegment(float p_secs, float p_size, float& p_delay, float& p_endSeg);

		/**
		 * Updates active tooltip status
		 * @param p_activeTooltip	Active tooltip status
		 */
		void setActiveTooltip(bool p_activeTooltip);

		/**
		 * Updates minimum track sizes (vector)
		 * @param p_minSizes	Minimum sizes (vector)
		 */
		void setMinSizes(vector<float> p_minSizes);

		/**
		 * Displays specified track
		 * @param p_num Track index
		 */
		void showTrack(int p_num);

		/**
		 * GTK callback - ClickedForPopup
		 * @param p_event	Gdk button event
		 * @return True
		 */
		bool onClickedForPopup(GdkEventButton* p_event);

		/**
		 * Updates cursor size
		 * @param p_pix	New cursor size
		 */
		void setCursorSize(int p_pix);

		/**
		 * Updates cursor color
		 * @param p_color New cursor color
		 */
		void setCursorColor(string p_color);

		/**
		 * GTK callback - SizeChanged
		 * @param width	New widget width
		 */
		void onSizeChanged(int width);

		/**
		 * Callback - Selecting
		 * @param p_select	True if we're actually selecting a signal portion
		 * @param p_left	True if selection motion is right to left
		 */
		void onSelecting(bool p_select, bool p_left);

		/**
		 * Callback - FocusIn
		 */
		void onFocusIn();

		/**
		 * Callback - ExpandClicked
		 */
	bool onExpandClicked(GdkEventButton* event);

		/**
		 * Updates peaks directory
		 * @param p_path	New peaks directory
		 */
		void setPeaksDirectory(const char* p_path);

		// -- High-level interface --
		/**
		 * Stores toplevel window pointer
		 * @param win	Toplevel window pointer
		 */
		void			setToplevel(Gtk::Window *win)	{ toplevel = win; }

		/**
		 * Returns toplevel window pointer
		 * @return Toplevel window pointer (NULL if not defined)
		 */
		Gtk::Window*	getToplevel()          			{ return toplevel; }

		/**
		 * Releases all audio resources held by current signal view
		 */
		void freeAudioResources();

		// ----------------------
		// --- Timing Methods ---
		// ----------------------

		/**
		 * Custom interruptable sleep\n
		 * iSleep has a similar behaviour as usleep, except that it can be interrupted by a control variable\n
		 * Maximum latency for interruption is 1ms
		 * @param sleep			Sleep time (in seconds)
		 * @param[out] watch	Control variable (if set to true, sleep is interrupted)
		 */
		void iSleep(float sleep, bool& watch); // Sleep that can be interrupted through local variable


		/**
		 * Adjusts tempo with specified offset
		 * @param p_factor New tempo offset
		 */
		void adjustTempo(float p_factor)
		{
			setTempo((a_tempo + p_factor), true);
		}

		/**
		 * Returns current zoom level
		 * @return Current zoom level
		 */
		int	getZoom()
		{
			return a_zoom;
		}

		/**
		 * Returns current audio track index
		 * @return Current track index
		 */
		int getSelectedAudioTrack()
		{
			return a_selectedAudioTrack;
		}

		/**
		 * Returns audio track
		 * @param p_index Input index
		 * @return Audio track pointer
		 */
		AudioTrackWidget* getAudioTrack(int p_index)
		{
			return a_audioTracks[p_index];
		}

		/**
		 * Returns serialized informations
		 * @return Informations (string)
		 */
		string getInfos()
		{
			return a_infos;
		}

		/**
		 * Gives focus to a specified widget
		 * @param widget Target widget
		 */
		void setToFocus(Gtk::Widget* widget)
		{
			a_toFocus = widget;
		}

		/**
		 * Updates StopOnClick mode
		 * @param p_b	StopOnClick mode
		 */
		void setStopOnClick(bool p_b)
		{
			a_stopOnClick = p_b;
		}

		/**
		 * Activate/Deactivate speech-only mode to all tracks
		 * @param p_b	Speech-only mode
		 */
		void setOnlySpeechAllTracks(bool p_b)
		{
			a_onlySpeechAllTracks = p_b;
		}

		/**
		 * Gets OnlySpeechAllTracks status
		 * @return True if all tracks are in speech-only mode, false otherwise.
		 */
		bool getOnlySpeechAllTracks()
		{
			return a_onlySpeechAllTracks;
		}

		/**
		 * Returns current peaks directory
		 * @return Peaks directory
		 */
		const char* getPeaksDirectory()
		{
			return a_peaksDirectory;
		}

		/**
		 * Returns current tempo
		 * @return Tempo value
		 */
		float getTempo()
		{
			return a_tempo;
		}

		/**
		 * Returns RewindAtEnd status
		 * @return RewindAtEnd status
		 */
		bool getRewindAtEnd()
		{
			return a_rewindAtEnd;
		}

		/**
		 * Returns RewindAtSelectionEnd status
		 * @return RewindAtSelectionEnd status
		 */
		bool getRewindAtSelectionEnd()
		{
			return a_rewindAtSelectionEnd;
		}

		/**
		 * Sets RewindAtEnd status
		 * @param p_rewindAtEnd	New status
		 */
		void setRewindAtEnd(bool p_rewindAtEnd)
		{
			a_rewindAtEnd = p_rewindAtEnd;
		}

		/**
		 * Sets RewindAtSelectionEnd status
		 * @param p_rewindAtSelectionEnd	New status
		 */
		void setRewindAtSelectionEnd(bool p_rewindAtSelectionEnd)
		{
			a_rewindAtSelectionEnd = p_rewindAtSelectionEnd;
		}

		/**
		 * Returns streaming status
		 * @return True if current medium is remote, false otherwise.
		 */
		bool isStreaming() { return streamingMode ; }

		/**
		 * Updates editable mode
		 * @param editable	Editable mode
		 */
		void setEditable(bool editable);

		/**
		 * Updates show_scale mode
		 * @param show	Show_scale mode
		 */
		void show_scale(bool show);

		/**
		 * Updates signal color
		 * @param mode	Target context
		 * @param color	New color
		 */
		void set_signal_color(Glib::ustring mode, Glib::ustring color);

		/**
		 * Returns signal color (for specified context)
		 * @param mode Target context
		 * @return Context color
		 */
		Glib::ustring get_signal_color(Glib::ustring mode);
		
		// ----------------------
		// --- Static Methods ---
		// ----------------------

		/**
		 * Updates maximum zoom value (global)
		 * @param max	New zoom value
		 */
		static void setZoomMax(float max) { AudioWidget::AUDIO_ZOOM_MAX=max; }

		/**
		 * Updates savePeaks value (Waveform)
		 * @param t	If true, computed peaks will be saved in peaks files.
		 */
		static void setSavePeaks(bool t=true) { AudioWaveformWidget::SAVE_PEAKS=t; }

		/**
		 * Updates absolutePeaksNorm value (Waveform)
		 * @param t If true, computed peaks will be also normalized.
		 */
		static void setUseAbsolutePeaksNorm(bool t) { AudioWaveformWidget::useAbsolutePeaksNorm(t); }

		/**
		 * Updates show_scale value
		 * @param show If true, scale widget will be visible, hidden otherwise.
		 */
		static void setShowScale(bool show=true) { a_scaleToShow=show; }

		/**
		 * Updates vertical scale size
		 * @param size	New vertical scale size
		 */
		static void setVerticalScaleSize(guint size) { AudioWidget::VERTICAL_SCALE_SIZE=size;  }

		/**
		 * Updates horizontal scale size
		 * @param size	New horizontal scale size
		 */
		static void setHorizontalScaleSize(guint size) { AudioWidget::HORIZONTAL_SCALE_SIZE=size;  }

		/**
		 * If set to true, indicates that a multi channel signal will be treated as a mono signal
		 * @param p_singleSignal		True or False
		 */
		void setSingleSignal(bool p_singleSignal) { singleSignal = true ; }

		/**
		 * Returns whether the component has been configured to use silent playing mode
		 * @return			True of False
		 */
		bool isSilentMode()  { return silentMode ;}

		/**
		 * Launch peaks computing in a thread
		 * @param audioTrack	Pointer on corresponding AudioTrackWidget
		 * @param silentMode	True for silent mode, false otherwise
		 */
		void computePeaksInThread(AudioTrackWidget* audioTrack, bool silentMode) ;

	private:
		int BUFFER_SIZE;

		// GTK Widgets
		Gtk::Box* 			a_mainBox;
		Gtk::HBox* 			a_toolBar;
		Gtk::Frame* 		a_playbackFrame;
		Gtk::HBox* 			a_hBoxPlaybackFrame;
		Gtk::Frame* 		a_selectionAutoFrame;
		Gtk::HBox* 			a_selectionAutoHBox;
		Gtk::Frame*			a_zoomFrame;
		Gtk::HBox* 			a_hBoxZoomFrame;
		Gtk::Frame* 		a_selectionFrame;
		Gtk::HBox* 			a_hBoxSelectionFrame;
		Gtk::Button* 		a_all;
		Gtk::Button* 		a_clear;
		Gtk::Button*		a_showVideo;
		Gtk::Adjustment* 	a_adjust;
		Gtk::Frame* 		a_infosFrame;
		Gtk::HBox* 			a_hBoxInfosFrame;
		Gtk::HBox* 			a_hBoxInfosFrame1;
		Gtk::HBox* 			a_vBoxInfosFrame2;
		Gtk::HBox* 			a_hBoxInfosFrame3;
		Gtk::Label* 		a_labelDelay;
		Gtk::Entry* 		a_entryDelay;
		Gtk::Tooltips 		a_tooltips;
		Gtk::CheckButton* 	a_loop;
		Gtk::CheckButton*	a_speechOnly;
		Gtk::CheckButton* 	a_autoSelectionPlay;
		IcoPackImageButton	a_expandButton;
		Gtk::Entry* 		a_infoCursor;
		Gtk::Entry* 		a_infoTotal;
		Gtk::Entry* 		a_infoSelection;
		Gtk::Entry* 		a_infoVisible;
		Gtk::Entry* 		a_infoMinSize1;
		Gtk::Entry* 		a_infoMinSize2;
		Gtk::HScrollbar* 	a_scrollBar;
		Gtk::Widget*		a_toFocus;
		Glib::Timer			a_timer;
		Glib::Timer			pa_timer;
		Glib::Timer			a_delaytimer;
		Gtk::Frame			a_tempoFrame ;
		Gtk::Table* 		infoTable ;
		Gtk::Alignment		align_infoTable ;
		Gtk::Expander 		a_moreExpander ;
		Gtk::HBox			a_moreHbox ;

		// Transcriber Widgets
		AudioTempoControlWidget*	a_atc;
		AudioPlayControlWidget*		a_playPause;
		AudioZoomControlWidget*		a_zin;
		AudioZoomControlWidget*		a_zout;
		Gtk::Window*				toplevel;

		// Global Tracks
		vector<TrackWidget*>		a_tracks;
		vector<AudioTrackWidget*>	a_audioTracks;
		vector<SegmentTrackWidget*>	a_segmentTracks;
		vector<MarkTrackWidget*>	a_markTracks;


		// Control Variables
		bool a_play;
		bool a_stopOnClick;
		bool a_threadTerminated;
		bool a_onlySpeechAllTracks;
		bool a_expanded;
		bool a_cursorChanged;
		bool a_wait;
		bool a_isAlive;
		bool a_updateCursor;
		bool a_updateInfos;
		bool a_upd_widget;
		bool a_rewindAtEnd;
		bool a_loopActive;
		bool a_updateLoop_timeout;
		bool a_scaleAdded;
		bool a_rewindAtSelectionEnd;
		bool a_delay;
		bool a_windowConnected;
		bool a_selecting;
		bool a_updateLoop_play_param1;
		bool a_updateLoop_play;
		bool a_leftMoving;
		bool tc;
		bool lastBlock;
		bool isRewinding;
		bool streamingMode;
		bool a_useAbsolutePeaksNorm ;
		bool a_interrupt;
		bool a_sleep;
		bool a_firstFrame;
		bool pa_initialized;

		bool a_delay_on;
		bool a_loopDelay;
		bool a_closure;

		/*** Silent mode ***/
		bool silentMode ;
		int silentModeNbChannels ;
		int silentModeLength ;

		// Variables
		float	a_tempo;
		float	buf_len;
		double	a_cursor;
		float	c_cursor;
		float	pre_seek;
		float	a_beginSelection;
		float	a_endSelection;
		float	a_cursorScroll;
		float	a_lastCursor;
		float	a_cursorDisplay;
		float	a_backward_start_delay;
		float	a_quickmove_delay;
		float	a_durationMax;
		float	a_duration;
		float	a_updateTempo;
		float	a_endOfSegment;
		float	a_lastEnd;
		float	a_lastDelay;
		float	a_endDelay;
		float	a_maxOffset;
		float	cpt_ms;
		float	t1, t2, t3, t4, t5;
		float	c1, c2, c3, c4, c5;

		double	a_blocks;
		double	d1, d2, d3, d4, d5;
		double	a_lastElapsed;

		int		a_zoom;
		int		a_zoomMax;
		int		a_zoomRequest;
		int		a_selectedAudioTrack;
		int		a_cursorSize;
		int		upcur;
		int		nb_blocks;
		int		scale_indice;


		bool	track_clicked;
		int		m_channels;

		bool 	singleSignal ;
	 	bool 	useVideo ;

		string	a_infos;
		string	a_cursorColor;
		string	a_audio_url;
		string	m_local_file;

		timeval tv;

		// Signals
		sigc::signal<void, bool>			a_signalPlayedPaused;
		sigc::signal<void, int>				a_signalZoomChanged;
		sigc::signal<void, float>			a_signalCursorChanged;
		sigc::signal<void, float, float>	a_signalSelectionChanged;
		sigc::signal<void, float>			a_signalTracksScrolled;
		sigc::signal<void, int>				a_signalAudioTrackSelected;
		sigc::signal<void, string>			a_signalInfosUpdated;
		sigc::signal<void, bool>			a_signalTrackActivated;

		sigc::signal<void, int, float>		a_signalDelayChanged;


		sigc::signal<void, double, bool>	a_syncVideo;
		sigc::signal<void, double>			a_seekVideo;
		sigc::signal<void>					a_stopVideo;
		sigc::signal<void, float, float>	a_videoSelection;

		sigc::signal<void, int, int, int, Gtk::Menu*> a_signalPopulatePopup;
		sigc::signal<void, const string&, float, float, SegmentTrackWidget* > a_signalSegmentModified;

		sigc::signal<void, bool>					a_signalPeaksReady ;

		// --- MediaComponent ---
		PortAudioStream			*pa;
		IODevice				*device;
		MediumInfo				*s_info;
		RTSPSession*			rtsp_session;

		// Playing Thread
		GThread* 	a_playThread;

		// Actions
		Glib::RefPtr<Gtk::ActionGroup>	a_actions;
		Glib::RefPtr<Gtk::Action>		a_menuPlay;
		Glib::RefPtr<Gtk::Action>		a_menuPause;
		Glib::RefPtr<Gtk::ToggleAction>	a_menuLoop;

		vector<float>			a_minSizes;
		vector<float>			a_offsets;
		vector<Glib::ustring>	a_signalFiles;
		Glib::Mutex a_mut1;
		GMutex* seek_mutex;

		const char* a_peaksDirectory;

		bool updateLoop();
		void on_size_allocate(Gtk::Allocation& p_allocation);

		void terminateAddAudioStreamThread(bool ok, IODevice* device, AudioTrackWidget* track_w) ;
		void terminateAddEmptyAudioStreamThread(bool ok, IODevice* device, double length) ;

		float a_updateLoop_cursorTracks_param1;
		bool a_updateLoop_cursorTracks;
		Glib::ustring a_updateLoop_dlgError_param1;
		bool a_updateLoop_dlgError;
		string a_entryDelayText;
		Glib::Timer a_uptime;

		// show scale or not
		static bool	a_scaleToShow;

		// -- Seek variables --
		bool	seekRequest;
		float	seekPosition;

		bool a_traceReadCursor;  // set print cursor trace

		/** Loading variables **/
		int m_nbThread ;
		int m_totalNbThread ;
		Glib::Mutex a_mut_pk;
};

} // namespace

#endif // __HAVE_AUDIOSIGNALVIEW__
