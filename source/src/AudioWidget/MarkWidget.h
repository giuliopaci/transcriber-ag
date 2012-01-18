/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_MARKWIDGET__
#define __HAVE_MARKWIDGET__

namespace tag {

//#include "AudioWidget.h"
class AudioTrackWidget;

/**< Internal calss */
struct LL2 {
	float timeCode;
	char* text;
	char* color;
	LL2* next;
};

/**
 * @class 	MarkWidget
 * @ingroup	AudioWidget
 * MarkWidget...
 */
class MarkWidget : public Gtk::DrawingArea {

	protected:

		//jm
		bool current ;

		float a_duration;
		float a_secsPerPix;
		int a_w;
		int a_h;
		int a_beginVisible;
		int a_endVisible;
		vector<LL2*> line;
		int a_xPrec;
		bool on_expose_event(GdkEventExpose* p_event);
		bool on_button_press_event(GdkEventButton* p_event);
		bool on_motion_notify_event(GdkEventMotion* p_event);
		bool on_button_release_event(GdkEventButton* p_event);
		bool on_leave_notify_event(GdkEventCrossing* p_event);

		float a_currentSample;
		float a_selection1;
		float a_selection2;
		float a_beginSelection;
		float a_endSelection;

		int a_currentZoom;
		int a_zoomMax;
		float a_cursorScroll;
		bool a_selectionnable;

		int a_cursorSize;
		string a_cursorColor;

		sigc::signal<void, float> a_signalCursorChanged;	/**< GTK cursor changed signal */
		sigc::signal<void, float, float> a_signalSelectionChanged;
		sigc::signal<void, int, float, float> a_signalSegmentModified;
		sigc::signal<void, AudioTrackWidget*> a_signalSegmentClicked;

		LL2* a_segmentMoving;
		bool a_leftMoving;
		float a_lastCur;
		Gdk::Cursor a_cursorLeft;
		Gdk::Cursor a_cursorRight;

//		SegmentTooltip a_tooltip;
		bool a_activeTooltip;

		AudioTrackWidget* a_selectedAudioTrack;

	public:

		/**
		 * Select audio track
		 * @param p_track	Audio track pointer
		 */
		void setSelectedAudioTrack(AudioTrackWidget* p_track) { a_selectedAudioTrack = p_track; queue_draw(); }

		/**
		 *
		 * @param value
		 */
		void set_current(bool value) {
			current = value ;
			on_expose_event(NULL) ;
		}

		/**
		 *
		 * @return
		 */
		bool is_current() { return current ;}

		/**< */
		const static float SHORTMIN; // peaks
		/**< */
		const static float SHORTMAX; // peaks
		/**< */
		const static int MARKWIDTH;
		/**< */
		const static int MARKHEIGHT;
		/**< */
		const static bool MARKDOWN;
		/**< */
		static bool POUIT;

		/**
		 * Constructor
		 */
		MarkWidget(float p_duration);
		virtual ~MarkWidget();

		/**
		 *
		 * @param p_timeCode
		 * @param p_text
		 * @param p_color
		 */
		void addMark(float p_timeCode, const char* p_text, const char* p_color);

		/**
		 *
		 * @param p_timeCode
		 */
		void removeMark(float p_timeCode);

		/***
		 * Sets zoom
		 * @param p_factor	Zoom factor
		 */
		void setZoom(int p_factor);

		/**
		 * Sets cursor
		 * @param p_cursor	Cursor
		 */
		void setCursor(float p_cursor);

		/**
		 * Sets selection
		 * @param p_beginSelection	Selection start time
		 * @param p_endSelection	Selection end time
		 */
		void setSelection(float p_beginSelection, float p_endSelection);

		/**
		 * Sets scroll
		 * @param p_cursorScroll	Scroll position
		 */
		void setScroll(float p_cursorScroll);

		/**
		 * Sets selectionable
		 * @param p_selectionnable	True for selectionable, False otherwise
		 */
		void setSelectionnable(bool p_selectionnable) { a_selectionnable = p_selectionnable; }

		/**
		 * GTK cursor changed signal accessor
		 * @return The GTK cursor changed signal
		 */
		sigc::signal<void, float> signalCursorChanged() { return a_signalCursorChanged; }

		/**
		 * GTK cursor changed signal selection
		 * @return The GTK cursor changed signal
		 */
		sigc::signal<void, float, float> signalSelectionChanged() { return a_signalSelectionChanged; }

		/**
		 * Previous mark
		 */
		void previous();

		/**
		 * 	Next mark
		 */
		void next();

		/**
		 * Enables / disables tooltip
		 * @param p_activeTooltip	True or false
		 */
		void setActiveTooltip(bool p_activeTooltip) { a_activeTooltip = p_activeTooltip; }

		/**
		 * Sets cursor size
		 * @param p_pix		Cursor size in pixels
		 */
		void setCursorSize(int p_pix) { a_cursorSize = p_pix; queue_draw(); }

		/**
		 * Sets cursor color
		 * @param p_color	Cursor color in string representation (hexadecimal)
		 */
		void setCursorColor(string p_color) { a_cursorColor = p_color; queue_draw(); }

		/**
		 * Sets zoom max
		 * @param p_zoomMax	 zoom
		 */
		void setZoomMax(int p_zoomMax) { a_zoomMax = p_zoomMax; }

};

} // namespace

#endif // __HAVE_MARKWIDGET__
