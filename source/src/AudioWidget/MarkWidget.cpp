/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class MarkWidget
 *
 * MarkWidget...
 */

#include <string.h>
#include <sys/time.h>
#include "AudioWidget.h"

namespace tag {

const float MarkWidget::SHORTMIN	= 0.5;	// peaks
const float MarkWidget::SHORTMAX	= 2;	// peaks
const int	MarkWidget::MARKWIDTH	= 10;
const int	MarkWidget::MARKHEIGHT 	= 15;
const bool	MarkWidget::MARKDOWN 	= TRUE;


// --- MarkWidget ---
MarkWidget::MarkWidget(float p_duration)
: Gtk::DrawingArea(),
	a_cursorLeft(Gdk::LEFT_SIDE),
	a_cursorRight(Gdk::RIGHT_SIDE)
{
	current = false ;
	a_duration = p_duration;
	a_segmentMoving = NULL;
	a_activeTooltip = true;

	a_selectedAudioTrack = NULL;

	a_currentZoom = 1;
	a_zoomMax = 1;
	a_cursorScroll = 0.0;
	a_currentSample = 0.0;
	a_selection1 = -1.0;
	a_selection2 = -1.0;
	a_beginSelection = -1.0;
	a_endSelection = -1.0;
	a_w = 0;
	a_h = 0;
	a_beginVisible = 0;
	a_endVisible = 0;
	a_selectionnable = true;
	a_cursorSize = 2;
	a_cursorColor = "yellow";

	add_events(Gdk::POINTER_MOTION_MASK);
	add_events(Gdk::BUTTON_PRESS_MASK);
	add_events(Gdk::BUTTON_MOTION_MASK);
	add_events(Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::LEAVE_NOTIFY_MASK);

}


// --- ~MarkWidget ---
MarkWidget::~MarkWidget()
{}


// --- On_button_press_event ---
bool MarkWidget::on_button_press_event(GdkEventButton* p_event)
{
	if (p_event->button == 3)
		return false;

	int x = (int)roundf(p_event->x);
	int y = (int)roundf(p_event->y);

	float secs = a_cursorScroll + ((float)x * AudioWidget::AUDIO_ZOOM_MAX * (float)a_currentZoom);

	if (secs > a_duration)
		return false;

	int begin = (int)(a_cursorScroll / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);

	vector<LL2*>::iterator it;

	for (it = line.begin(); it != line.end(); it++)
	{
		LL2* now = *it;
		int timeCode = (int)(now->timeCode / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom) - begin;

		if (x > timeCode-(MARKWIDTH/2) && x < timeCode+(MARKWIDTH/2))
		{
			a_beginSelection = -1.0;
			a_endSelection = -1.0;
			a_signalSelectionChanged.emit(-1.0, -1.0);
			a_currentSample = now->timeCode;
			a_signalCursorChanged.emit(now->timeCode);
			break;
		}
	}

	queue_draw();

	return false;
}


// --- On_motion_notify_event ---
bool MarkWidget::on_motion_notify_event(GdkEventMotion* p_event)
{
	return FALSE;
}


// --- On_button_release_event ---
bool MarkWidget::on_button_release_event(GdkEventButton* p_event)
{
	return FALSE;
}


// --- On_leave_notify_event ---
bool MarkWidget::on_leave_notify_event(GdkEventCrossing* p_event)
{
	return FALSE;
}


// --- AddMark ---
void MarkWidget::addMark(float p_timeCode, const char* p_text, const char* p_color)
{
	LL2* newLL		= new LL2();
	newLL->timeCode = p_timeCode;
	newLL->text		= strdup(p_text);
	newLL->color	= (char*) p_color;

	if (line.size() == 0)
	{
		line.push_back(newLL);
	}
	else
	{
		vector<LL2*>::iterator it;
		bool ok = false;

		for (it = line.begin(); it != line.end(); it++)
		{
			LL2* now = *it;
			if (now->timeCode == newLL->timeCode)
			{
				break;
			}
			else
			{
				if (now->timeCode > newLL->timeCode)
				{
					line.insert(it, newLL);
					ok = true;
					break;
				}
			}
		}

		if (!ok)
			line.push_back(newLL);
	}

	queue_draw();
}


// --- RemoveMark ---
void MarkWidget::removeMark(float p_timeCode)
{
	vector<LL2*>::iterator it;

	for (it = line.begin(); it != line.end(); it++)
	{
		LL2* now = *it;
		if (now->timeCode == p_timeCode)
		{
			line.erase(it);
			break;
		}
	}

	queue_draw();
}


// --- On_expose_event ---
bool MarkWidget::on_expose_event(GdkEventExpose* p_event)
{
	Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(get_window());
	Gdk::Color blue("blue");
	get_default_colormap()->alloc_color(blue);
	Gdk::Color red("red");
	get_default_colormap()->alloc_color(red);
	Gdk::Color green("green");
	get_default_colormap()->alloc_color(green);
	Gdk::Color yellow("yellow");
	get_default_colormap()->alloc_color(yellow);
	Gdk::Color white("white");
	get_default_colormap()->alloc_color(white);
	Gdk::Color black("black");
	get_default_colormap()->alloc_color(black);
	Gdk::Color grey("grey");
	get_default_colormap()->alloc_color(grey);
	Gdk::Color grey2("grey");
	grey2.set_rgb_p(0.5, 0.5, 0.5);
	get_default_colormap()->alloc_color(grey2);
	Gdk::Color blue2("blue");
	blue2.set_rgb_p(0.0, 0.0, 0.5);
	get_default_colormap()->alloc_color(blue2);
	Gdk::Color grey3("grey");
	grey3.set_rgb_p(0.3, 0.3, 0.3);
	get_default_colormap()->alloc_color(grey3);
	Gdk::Color cyan("white");
	cyan.set_rgb_p(0.7, 1.0, 1.0);
	get_default_colormap()->alloc_color(cyan);
	Gdk::Color cyan2("white");
	cyan2.set_rgb_p(0.42, 0.6, 0.6);
	get_default_colormap()->alloc_color(cyan2);
	Gdk::Color orange("white");
	orange.set_rgb_p(1.0, 0.8, 0.2);
	get_default_colormap()->alloc_color(orange);
	Gdk::Color orange2("white");
	orange2.set_rgb_p(0.6, 0.48, 0.12);
	get_default_colormap()->alloc_color(orange2);
	Gdk::Color purple("white");
	purple.set_rgb_p(1.0, 0.6, 1.0);
	get_default_colormap()->alloc_color(purple);
	Gdk::Color purple2("white");
	purple2.set_rgb_p(0.6, 0.36, 0.6);
	get_default_colormap()->alloc_color(purple2);
	Gdk::Color gr("white");
	gr.set_rgb_p(0.4, 1.0, 0.4);
	get_default_colormap()->alloc_color(gr);
	Gdk::Color gr2("white");
	gr2.set_rgb_p(0.24, 0.6, 0.24);
	get_default_colormap()->alloc_color(gr2);
	Gdk::Color cursorColor(a_cursorColor.c_str());
	get_default_colormap()->alloc_color(cursorColor);

	get_window()->set_background(white);
	get_window()->clear();

	int w = 0;
	int h = 0;
	get_window()->get_size(w, h);

	int begin = (int)(a_cursorScroll / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);

	int w2 = (int)(a_duration / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);

	int beginSelection = (int)(a_beginSelection / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	int endSelection = (int)(a_endSelection / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);

	int cursor = (int)(a_currentSample / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom)-begin;

	int veryEnd = begin+w;

	if (w2 < veryEnd)
		veryEnd = w2;

	vector<LL2*>::iterator it;

	for (it = line.begin(); it != line.end(); it++)
	{
		LL2* now = *it;
		int timeCode = (int)(now->timeCode / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom) - begin;

		if (timeCode > 0 && timeCode < w)
		{
			Gdk::Color c1(now->color);
			get_default_colormap()->alloc_color(c1);
			gc->set_foreground(c1);

			Gdk::Point points[4];
			points[0].set_x(timeCode); points[0].set_y(0);
			points[1].set_x(timeCode+(MARKWIDTH/2)); points[1].set_y(MARKHEIGHT);
			points[2].set_x(timeCode-(MARKWIDTH/2)); points[2].set_y(MARKHEIGHT);

			get_window()->draw_polygon(gc, true, points);

			gc->set_foreground(black);

			Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(get_pango_context());
			layout->set_text(now->text);

			int width = 0;
			int height = 0;
			if (MARKDOWN)
			{
				width	= timeCode-(MARKWIDTH/2);
				height	= MARKHEIGHT+1;
			}
			else
			{
				width	= timeCode+(MARKWIDTH/2)+4;
				height	= (MARKHEIGHT/2)-9;
				if (height < 2)
					height = 2;
			}
			get_window()->draw_layout(gc, width, height, layout);
		}
	}

	if (current)
	{
		Gdk::Color bordeau("#680704");
		get_default_colormap()->alloc_color(bordeau);
		int x, y ;
		get_window()->get_size(x, y);
		gc->set_foreground(bordeau);
		gc->set_line_attributes(3, Gdk::LINE_SOLID, Gdk::CAP_ROUND, Gdk::JOIN_ROUND);
		get_window()->draw_rectangle(gc, false, 0, 0, x-1, y-1);

		get_default_colormap()->free_color(bordeau);
	}

	int height = 0;
	if (MARKDOWN)
	{
		height = MARKHEIGHT+18;
	}
	else
	{
		height = MARKHEIGHT+2;

		if (height < 21)
			height = 21;
	}

	set_size_request(-1, height);

	get_default_colormap()->free_color(blue);
	get_default_colormap()->free_color(red);
	get_default_colormap()->free_color(green);
	get_default_colormap()->free_color(yellow);
	get_default_colormap()->free_color(white);
	get_default_colormap()->free_color(black);
	get_default_colormap()->free_color(grey);
	get_default_colormap()->free_color(grey2);
	get_default_colormap()->free_color(blue2);
	get_default_colormap()->free_color(grey3);
	get_default_colormap()->free_color(cyan);
	get_default_colormap()->free_color(cyan2);
	get_default_colormap()->free_color(orange);
	get_default_colormap()->free_color(orange2);
	get_default_colormap()->free_color(purple);
	get_default_colormap()->free_color(purple2);
	get_default_colormap()->free_color(gr);
	get_default_colormap()->free_color(gr2);
	get_default_colormap()->free_color(cursorColor);

	return false;
}


// --- SetCursor ---
void MarkWidget::setCursor(float p_cursor)
{
	float lastCursor	= a_currentSample;
	a_currentSample		= p_cursor;

	int c1		= (int)(lastCursor / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	int c2		= (int)(p_cursor / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	int begin	= (int)(a_cursorScroll / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	c1 			-= begin;
	c2 			-= begin;

	int w = -1;
	int h = -1;

	if ( get_window() )
		get_window()->get_size(w, h);

	queue_draw_area(c1, 0, a_cursorSize, h);
	queue_draw_area(c2, 0, a_cursorSize, h);

}


// --- SetSelection ---
void MarkWidget::setSelection(float p_beginSelection, float p_endSelection)
{
	a_beginSelection = p_beginSelection;
	a_endSelection = p_endSelection;
	queue_draw();
}


// --- SetZoom ---
void MarkWidget::setZoom(int p_factor)
{
	a_w = (int)(a_duration / AudioWidget::AUDIO_ZOOM_MAX / (float)p_factor);
	a_currentZoom = p_factor;
	queue_draw();
}


// --- SetScroll ---
void MarkWidget::setScroll(float p_cursorScroll)
{
	a_cursorScroll = p_cursorScroll;
	queue_draw();
}


// --- Previous ---
void MarkWidget::previous()
{
	float previous = 0;
	float next = a_duration;
	vector<LL2*>::iterator it;

	for (it = line.begin(); it != line.end(); it++)
	{
		LL2* now = *it;
		float t = now->timeCode;

		if (t > previous && t < a_currentSample)
			previous = t;
		if (t < next && t > a_currentSample)
			next = t;
	}

	if (previous == 0)
		return;

	a_currentSample = previous;
	a_signalCursorChanged.emit(a_currentSample);
	a_beginSelection = -1;
	a_endSelection = -1;
	a_signalSelectionChanged.emit(a_beginSelection, a_endSelection);
}


// --- Next ---
void MarkWidget::next()
{
	float previous = 0;
	float next = a_duration;
	vector<LL2*>::iterator it;

	for (it = line.begin(); it != line.end(); it++)
	{
		LL2* now = *it;
		float t = now->timeCode;
		if (t > previous && t < a_currentSample)
			previous = t;
		if (t < next && t > a_currentSample)
			next = t;
	}

	if (next == a_duration)
		return;

	a_currentSample = next;
	a_signalCursorChanged.emit(a_currentSample);
	a_beginSelection = -1;
	a_endSelection = -1;
	a_signalSelectionChanged.emit(a_beginSelection, a_endSelection);
}

} // namespace
