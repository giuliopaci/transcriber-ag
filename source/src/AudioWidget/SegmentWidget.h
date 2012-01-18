/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#ifndef __HAVE_SEGMENTWIDGET__
#define __HAVE_SEGMENTWIDGET__

/** @file */

namespace tag {

class AudioTrackWidget;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @struct LL
 * Custom segment linked-list implementation\n
 * This structure contains all segment properties.
 */
struct LL
{
	string	id;						/**< Identifier */
	float	start;					/**< Start offset */
	float	end;					/**< End offset */
	char*	text;					/**< Text */
	char*	color;					/**< Color */
	bool	skipable;				/**< Skipable status */
	int		weight;					/**< Indicates order when segment can be overlapped */
	bool 	hidden_order;			/**< If true, overlapped elements will be inlined */
	bool 	skip_highlight;			/**< If true, highlight will be skipped if this element is the current one in signal */
	AudioTrackWidget* audioTrack;	/**< Related audio track */
	LL*		next;					/**< Next segment pointer */
} ;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 * @class	SegmentWidget
 * @ingroup	AudioWidget
 *
 * This class implements segment tracks drawing routines
 */
class SegmentWidget : public Gtk::DrawingArea
{
public:
	/**
	 * Selected audio track accessor
	 * @param p_track Selected audio track
	 */
	void setSelectedAudioTrack(AudioTrackWidget* p_track)
	{
		a_selectedAudioTrack = p_track; queue_draw();
	}

	/**
	 * Current status accessor
	 * @param value	Current status
	 */
	void set_current(bool value)
	{
		current = value ;
		on_expose_event(NULL) ;
	}

	/**
	 * Current status accessor
	 * @return Current status
	 */
	bool is_current()
	{
		return current;
	}


	const static float SHORTMIN;	/**< Minimum short value */
	const static float SHORTMAX;	/**< Maximum short value */


	/**
	 * Default constructor
	 * @param p_duration	Track duration
	 * @param label			Track label
	 * @param flatMode		True for flat mode (thick sagment)
	 */
	SegmentWidget(float p_duration, string label, bool flatMode);
	virtual ~SegmentWidget();

	/**
	 * Starts update processing
	 */
	void	startUpdate();

	/**
	 * Commits last update
	 * @param force	If true, forces commit
	 */
	void	commitUpdate(bool force=false);

	/**
	 * Adds a new segment to track
	 * @param p_id				Id
	 * @param p_start			Start offset (in seconds)
	 * @param p_end				End offset (in seconds)
	 * @param p_text			Text
	 * @param p_color			Color
	 * @param p_skipable		Skipable property
	 * @param p_weight			Weight
	 * @param hidden_order		Hidden order property
	 * @param skip_highlight	Skip highlight property
	 * @param p_audioTrack		Optional related audio track
	 */
	void	addSegment( const string& p_id,
						float p_start,
						float p_end,
						const char* p_text,
						const char* p_color,
						bool p_skipable,
						int p_weight,
						bool hidden_order,
						bool skip_highlight,
						AudioTrackWidget* p_audioTrack = NULL);

	/**
	 * Adds a new segment to track (internal)
	 * @param p_id				Id
	 * @param p_start			Start offset (in seconds)
	 * @param p_end				End offset (in seconds)
	 * @param p_text			Text
	 * @param p_color			Color
	 * @param p_skipable		Skipable property
	 * @param p_weight			Weight
	 * @param hidden_order		Hidden order property
	 * @param skip_highlight	Skip highlight property
	 * @param p_audioTrack		Optional related audio track
	 */
	void	addSegment2(const string& p_id,
						float p_start,
						float p_end,
						const char* p_text,
						const char* p_color,
						bool p_skipable,
						int p_weight,
						bool hidden_order,
						bool skip_highlight,
						AudioTrackWidget* p_audioTrack = NULL);

	/**
	 * Adds a new segment to segment cache
	 * @param p_id				Id
	 * @param p_start			Start offset (in seconds)
	 * @param p_end				End offset (in seconds)
	 * @param p_text			Text
	 * @param p_color			Color
	 * @param p_skipable		Skipable property
	 * @param p_weight			Weight
	 * @param hidden_order		Hidden order property
	 * @param skip_highlight	Skip highlight property
	 * @param p_audioTrack		Optional related audio track
	 */
	void	addSegmentToCache(const string& p_id,
							float p_start,
							float p_end,
							const char* p_text,
							const char* p_color,
							bool p_skipable,
							int p_weight,
							bool hidden_order,
							bool skip_highlight,
							AudioTrackWidget* p_audioTrack = NULL);

	/**
	 * Removes segment
	 * @param p_line	Target line
	 * @param p_index	Target index
	 */
	void	removeSegment(int p_line, int p_index);

	/**
	 * Removes segment
	 * @param p_id	Segment Id
	 */
	void	removeSegment(const string& p_id);

	/**
	 * Removes all segments related to an audio track
	 * @param p_audioTrack	Target audio track
	 */
	void	removeSegments(AudioTrackWidget* p_audioTrack);

	/**
	 * Re-order segment lists, after segment updates
	 */
	void	remanageSegments();

	/**
	 * Re-order segment lists, after segment updates (internal)
	 */
	void	remanageSegments2(LL* start);

	/**
	 * Re-order cached segments list, after segment updates
	 */
	void	manageCachedSegments();

	/**
	 * Empties segment lines
	 */
	void	emptyLines();

	/**
	 * Inserts current segment pointer in lines
	 * @param now	Current segment pointer
	 */
	void	insertInLines(LL* now);

	/**
	 * Returns next segment that cannot be skipped (ex: speech segment)
	 * @param p_secs	Current cursor position
	 */
	float	nextNotSkipable(float p_secs);

	/**
	 * Returns next segment end and delay (if set)
	 * @param p_secs		Current cursor position
	 * @param p_size		Current cursor size (unused)
	 * @param[out] p_endSeg	Next segmend end (end of track -> -1)
	 */
	bool	endSegment(float p_secs, float p_size, float& p_endSeg);

	/**
	 * Finds segment on a specified track
	 * @param track		Track number
	 * @param p_cursor	Cursor position
	 * @return Segment pointer on success, NULL otherwise.
	 */
	LL*		findSegment(int track, float p_cursor);

	/**
	 * Returns a vector containing current segment ids
	 * @return Current ids (vector)
	 */
	vector<string> getCurrentSegmentId();

	/**
	 * Refreshes the whole widget
	 */
	void	refresh();

	/**
	 * Updates display to reflect new zoom factor
	 * @param p_factor	New zoom factor
	 */
	void	setZoom(int p_factor);

	/**
	 * Updates display to reflect new cursor position
	 * @param p_cursor	New cursor position
	 */
	void	setCursor(float p_cursor);

	/**
	 * Zoom factor accessor
	 * @param p_beginSelection	selection start
	 * @param p_endSelection	selection end
	 */
	void	setSelection(float p_beginSelection, float p_endSelection);

	/**
	 * Updates display to reflect new scroll value
	 * @param p_cursorScroll	New cursor scroll
	 */
	void	setScroll(float p_cursorScroll);

	/**
	 * Updates selectable status
	 * @param p_selectable	Selectable status
	 */
	void	setSelectable(bool p_selectable);

	/**
 	 * Switches to previous segment
	 */
	void	previous();

	/**
 	 * Switches to next segment
	 */
	void	next();

	/**
	 * Updates active tooltip status
	 * @param p_activeTooltip	Active tooltip status
	 */
	void	setActiveTooltip(bool p_activeTooltip) { a_activeTooltip = p_activeTooltip; }

	/**
	 * Returns closest segment start, based on last begin position
	 * @param p_cursor Cursor position
	 * @return Closest segment start
	 */
	float	getLastBegin(float p_cursor);

	/**
	 * Updates segment text
	 * @param p_id		Target segment Id
	 * @param p_text	New segment text
	 */
	void	updateSegmentText(const string& p_id, const string& p_text);

	/**
	 * Updates display to reflect new cursor size
	 * @param p_pix	New cursor size
	 */
	void	setCursorSize(int p_pix) { a_cursorSize = p_pix; queue_draw(); }

	/**
	 * Updates display to reflect new cursor color
	 * @param p_color	New cursor color
	 */
	void	setCursorColor(string p_color)
	{
		a_cursorColor = p_color; // Reset cursor color
		cursorColor.set( a_cursorColor.c_str() );
		get_default_colormap()->alloc_color(cursorColor);
		queue_draw();
	}

	/**
	 * Sets widget color for specified mode
	 * @param mode	Widget mode (role)
	 * @param color	New color
	 */
	void	set_color(Glib::ustring mode, Glib::ustring color) ;

	/**
	 * Set the maximal zoom
	 * @param p_zoomMax	Maximal zoom
	 */
	void	setZoomMax(int p_zoomMax) { a_zoomMax = p_zoomMax; }

	// -----------------------
	// --- Drawing Methods ---
	// -----------------------

	/**
	 * Draws background area (layer 3)
	 */
	void	drawArea();

	/**
	 * Draws every segment track (layer 2)
	 */
	void	drawTracks();

	/**
 	 * Draws specified segment (layer 2)
	 */
	bool	drawSegment(LL* current, int);

	/**
	 * This method draws cursor (layer 1)
	 */
	void	drawCursor();


	// ------------------------
	// --- Signal Accessors ---
	// ------------------------

	/**
	 * GTK signal - CursorChanged
	 * @return CursorChanged signal
	 */
	sigc::signal<void, float>					signalCursorChanged()	{ return a_signalCursorChanged; }

	/**
	 * GTK signal - SelectionChanged
	 * @return SelectionChanged signal
	 */
	sigc::signal<void, float, float>			signalSelectionChanged(){ return a_signalSelectionChanged; }

	/**
	 * GTK signal - SegmentModified
	 * @return SegmentModified signal
	 */
	sigc::signal<void, string, float, float>	signalSegmentModified() { return a_signalSegmentModified; }

	/**
	 * GTK signal - SegmentClicked
	 * @return SegmentClicked signal
	 */
	sigc::signal<void, AudioTrackWidget*>		signalSegmentClicked()	{ return a_signalSegmentClicked; }

	/**
	 * GTK signal - GetText
	 * @return GetText signal
	 */
	sigc::signal<void, const string&, char*&>&	signalGetText()			{ return a_signalGetText; }


private:
	bool	current ;

	bool 	flatMode ;

	float	a_duration ;
	float	a_secsPerPix ;
	int		a_w ;
	int		a_h ;
	int		a_beginVisible ;
	int		a_endVisible ;
	int		a_linesCount ;
	int		a_xPrec ;
	int 	a_lineWidthCoeff ;
	LL*		a_lines[10] ;
	LL*		a_lines_last[10] ;
	LL*		a_prev_start[10] ;

	// Init Functions
	void	initColormap();

	// GUI Events
	bool	on_expose_event(GdkEventExpose* p_event);
	bool	on_button_press_event(GdkEventButton* p_event);
	bool	on_motion_notify_event(GdkEventMotion* p_event);
	bool	on_button_release_event(GdkEventButton* p_event);
	bool	on_leave_notify_event(GdkEventCrossing* p_event);
	bool	on_configure_event(GdkEventConfigure* p_event);

	// Signals
	sigc::signal<void, float>					a_signalCursorChanged;
	sigc::signal<void, float, float>			a_signalSelectionChanged;
	sigc::signal<void, string, float, float>	a_signalSegmentModified;
	sigc::signal<void, AudioTrackWidget*>		a_signalSegmentClicked;
	sigc::signal<void, const string&, char*&>	a_signalGetText;

	float	a_currentSample;
	float	a_selection1;
	float	a_selection2;
	float	a_beginSelection;
	float	a_endSelection;
	float	a_cursorScroll;

	int		a_currentZoom;
	int		a_zoomMax;
	int		a_cursorSize;
	bool	a_selectable;
	bool	a_canDraw;
	int		a_inTransaction;
	bool	a_updated;
	bool	move_start, move_end;

	string	a_cursorColor;

	LL*		a_segmentMoving;
	LL*		a_cache_start;
	LL*		a_cache_tail;
	bool	a_leftMoving;
	float	a_lastCur;

	Gdk::Cursor	a_cursorLeft;
	Gdk::Cursor	a_cursorRight;
	string		a_label;


	SegmentTooltip	a_tooltip;
	bool			a_activeTooltip;
	int				width, height;

	AudioTrackWidget* a_selectedAudioTrack;

	// GDK Colors
	Gdk::Color	white, black;
	Gdk::Color	cursorColor, clearTextColor ;
	Gdk::Color	baseGrey ;

	// Control variables
	bool		firstSegment;
	bool		remanaged;

	// Current Segments (extrema)
	float		seg1_start, seg1_end;
	float		seg2_start, seg2_end;
	int			p_start, p_end;

	// Expose variables
	int			beginSelection, endSelection, begin, w, h;
	int			w2, veryEnd, cursor;
	int			clip_x, clip_x2, clip_width, clip_height;
	int			tw, th;
	float		s1, s2;
	bool		localUpdate;
	int 		prev_begin_track;
	Glib::RefPtr<Gdk::GC> gc;

	// GTK Window Instance
	Glib::RefPtr<Gdk::Window>	win;
};

} // namespace

#endif // __HAVE_SEGMENTWIDGET__

