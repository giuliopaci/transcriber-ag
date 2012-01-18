/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_SEGMENTTRACKWIDGET__
#define __HAVE_SEGMENTTRACKWIDGET__

#include "Common/icons/IcoPackImageButton.h"

namespace tag {
/**
 * @class	SegmentTrackWidget
 * @ingroup	AudioWidget
 * This class implements segment tracks controls
 */
class SegmentTrackWidget : public TrackWidget
{
public:
	/**
	 * Default constructor
	 *
	 */
	SegmentTrackWidget(float p_duration, string p_label, int p_numTrack, int startGap, bool flatMode);
	virtual ~SegmentTrackWidget();

	// ---------------------------------
	// --- TrackWidget Inherited API ---
	// ---------------------------------
	int getType()							{ return 1 ;}
	void setTempo(float p_factor) 			{}
	void setZoom(int p_zoom) 				{ a_segment->setZoom(p_zoom); }
	void setZoomMax(int p_zoomMax) 			{ a_segment->setZoomMax(p_zoomMax); }
	void setCursor(float p_cursor) 			{ a_segment->setCursor(p_cursor); }
	void setScroll(float p_cursorScroll)	{ a_segment->setScroll(p_cursorScroll); }
	void setCursorSize(int p_pix)			{ a_segment->setCursorSize(p_pix); }
	void setCursorColor(string p_color)		{ a_segment->setCursorColor(p_color); }
	void setSelection(float p_beginSelection, float p_endSelection) { a_segment->setSelection(p_beginSelection, p_endSelection); }

	sigc::signal<void, float, float> signalSelectionChanged()	{ return a_segment->signalSelectionChanged(); }
	sigc::signal<void, float> signalCursorChanged()				{ return a_segment->signalCursorChanged(); }


	// -----------------
	// --- Internals ---
	// -----------------

	/**
	 * Current status accessor
	 * @param value	Current status
	 */
	void set_current(bool value)
	{
		if (a_segment)
			a_segment->set_current(value);
	}

	/**
	 * Current status accessor
	 * @return Current status
	 */
	bool get_current()
	{
		if (a_segment)
			return a_segment->is_current();
		else
			return false;
	}

	/**
	 * Segment track label accessor
	 * @return Segment track label
	 */
	string getLabel() { return a_label; }

	/**
	 * Track number accessor
	 * @return Track number
	 */
	int getNumTrack() { return a_numTrack; }

	// ----------------------
	// -- Signals Handlers --
	// ----------------------

	/**
	 * GTK signal - SegmentModified
	 * @return SegmentModified signal
	 */
	sigc::signal<void, string, float, float> signalSegmentModified()	{ return a_segment->signalSegmentModified(); }

	/**
	 * GTK signal - SegmentClicked
	 * @return SegmentClicked signal
	 */
	sigc::signal<void, AudioTrackWidget*> signalSegmentClicked()		{ return a_segment->signalSegmentClicked(); }

	/**
	 * GTK signal - GetText
	 * @return GetText signal
	 */
	sigc::signal<void, const string&, char*&>& signalGetText()			{ return a_segment->signalGetText(); }


	// -------------------------
	// -- Segments Management --
	// -------------------------

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
	void addSegment(const string& p_id,
					float p_start,
					float p_end,
					const char* p_text,
					const char* p_color,
					bool p_skipable,
					int p_weight,
					bool hidden_order,
					bool skip_highlight,
					AudioTrackWidget* p_audioTrack = NULL)
	{ a_segment->addSegment(p_id, p_start, p_end, p_text, p_color, p_skipable, p_weight, hidden_order, skip_highlight, p_audioTrack); }

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
	void addSegmentToCache( const string& p_id,
							float p_start,
							float p_end,
							const char* p_text,
							const char* p_color,
							bool p_skipable,
							int p_weight,
							bool hidden_order,
							bool skip_highlight,
							AudioTrackWidget* p_audioTrack = NULL)
	{ a_segment->addSegmentToCache(p_id, p_start, p_end, p_text, p_color, p_skipable, p_weight, hidden_order, skip_highlight, p_audioTrack); }

	/**
	 * Removes segment
	 * @param p_line	Target line
	 * @param p_index	Target index
	 */
	void removeSegment(int p_line, int p_index)			{ a_segment->removeSegment(p_line, p_index); }

	/**
	 * Removes segment
	 * @param p_id	Segment Id
	 */
	void removeSegment(const string& p_id)				{ a_segment->removeSegment(p_id); }

	/**
	 * Removes all segments related to an audio track
	 * @param p_audioTrack	Target audio track
	 */
	void removeSegments(AudioTrackWidget* p_audioTrack) { a_segment->removeSegments(p_audioTrack); }

	/**
	 * Starts update processing
	 */
	void startUpdate()									{ a_segment->startUpdate(); }

	/**
	 * Commits last update
	 * @param force	If true, forces commit
	 */
	void commitUpdate(bool force=false)					{ a_segment->commitUpdate(force); }

	/**
	 * Re-order segment lists, after segment updates
	 */
	void remanageSegments()								{ a_segment->remanageSegments(); }

	/**
	 * Re-order cached segments list, after segment updates
	 */
	void manageCachedSegments()							{ a_segment->manageCachedSegments(); }

	/**
	 * Returns the next segment that can't be skipped (ex: speech segment)
	 * @param p_secs	Current cursor position
	 */
	float nextNotSkipable(float p_secs)					{ return a_segment->nextNotSkipable(p_secs); }

	/**
	 * Returns next segment end and delay (if set)
	 * @param p_secs		Current cursor position
	 * @param p_size		Current cursor size (unused)
	 * @param[out] p_delay	Delay that would be applied at segment end (no delay -> -1)
	 * @param[out] p_endSeg	Next segmend end (end of track -> -1)
	 */
	void endSegment(float p_secs, float p_size, float& p_delay, float& p_endSeg);

	/**
	 * Selectable status accessor
	 */
	void setSelectable(bool p_selectable);

	/**
	 * Callback - Previous button clicked
	 */
	void onPreviousClicked()	{ a_segment->previous(); }

	/**
	 * Callback - Next button clicked
	 */
	void onNextClicked()		{ a_segment->next(); }

	/**
	 * Callback - Stop button clicked
	 */
	void onStopClicked();

	/**
	 * Callback - Expand button clicked
	 */
	bool onExpandClicked(GdkEventButton* event) ;

	/**
	 * Callback - Tooltip hidden
	 */
	void onTooltipHidden();

	/**
	 * GTK callback - Key pressed
	 * @param p_event Gdk key event
	 */
	bool onKeyPressed(GdkEventKey* p_event);

	/**
	 * Refreshes display
	 */
	void refresh();

	/**
 	 * Restores activate button size
	 * @param b_size
	 */
	void resetActivateButtonSize(int b_size);

	/**
	 * Active tooltip status accessor
	 * @param p_activeTooltip tooltip status
	 */
	void setActiveTooltip(bool p_activeTooltip) { a_segment->setActiveTooltip(p_activeTooltip); }

	/**
	 * Returns closest segment start, based on last begin position
	 * @param p_cursor Cursor position
	 * @return Closest segment start
	 */
	float getLastBegin(float p_cursor)							{ return a_segment->getLastBegin(p_cursor); }

	/**
	 * Updates segment text
	 * @param p_id		Target segment Id
	 * @param p_text	New segment text
	 */
	void updateSegmentText(const string& p_id, string p_text)	{ a_segment->updateSegmentText(p_id, p_text); }

	/**
	 * Sets selected audio track
	 * @param p_track	Selected audio track
	 */
	void setSelectedAudioTrack(AudioTrackWidget* p_track)		{ a_segment->setSelectedAudioTrack(p_track); }

private:
	SegmentWidget*		a_segment;

	sigc::signal<void>	a_signalTooltipHidden;
	sigc::signal<void>	a_signalPreviousClicked;
	sigc::signal<void>	a_signalNextClicked;

	Gtk::Frame*			a_segmentEndFrame;
	Gtk::Label*			a_labelDelay;
	Gtk::Entry*			a_entryDelay;
	Gtk::CheckButton*	a_stop;
	Gtk::CheckButton*	a_activateButton;
	IcoPackImageButton	a_expandButton;
	Gtk::HBox			a_startGap;
	Gtk::IconSize		a_iconSize;
	Gtk::Image			a_imagePrevious;
	Gtk::Image			a_imageNext;
	Gtk::Image			a_imageAdd;
	Gtk::Tooltips		a_tooltips;

	string				a_label;
	int 				a_numTrack;
	float				a_delay;
	bool				a_expanded;
};

} // namespace


#endif // __HAVE_SEGMENTTRACKWIDGET__

