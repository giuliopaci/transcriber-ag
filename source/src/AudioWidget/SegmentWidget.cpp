/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class SegmentWidget
 *
 * SegmentWidget...
 */

#include <string.h>
#include <sys/time.h>
#include "AudioWidget.h"
#include "Common/widgets/GtUtil.h"

namespace tag {

const float SegmentWidget::SHORTMIN = 0.5;	// peaks
const float SegmentWidget::SHORTMAX = 2;	// peaks

// --- SegmentWidget ---
SegmentWidget::SegmentWidget(float p_duration, string label, bool flatMode)
: Gtk::DrawingArea(),
	a_cursorLeft(Gdk::LEFT_SIDE),
	a_cursorRight(Gdk::RIGHT_SIDE),
	a_label(label)
{
	current					= false;
	a_canDraw				= true;
	a_inTransaction			= 0;
	a_canDraw				= true;
	a_cache_start			= NULL;
	a_cache_tail			= NULL;
	a_segmentMoving			= NULL;
	a_activeTooltip			= true;
	a_selectedAudioTrack	= NULL;
	a_duration				= p_duration;

	a_linesCount			= 0;
	a_currentZoom			= 1;
	a_zoomMax				= 1;
	a_cursorScroll			= 0.0;
	a_currentSample			= 0.0;
	a_selection1			= -1.0;
	a_selection2			= -1.0;
	a_beginSelection		= -1.0;
	a_endSelection			= -1.0;
	a_w						= 0;
	a_h						= 0;
	tw						= 0;
	th						= 0;
	a_beginVisible			= 0;
	a_endVisible			= 0;
	a_selectable		= true;
	localUpdate				= false;
	firstSegment			= true;
	a_cursorSize			= 2;
	a_cursorColor			= "yellow";

	for (int i = 0; i < 10; i++)
	{
		a_lines[i]		= NULL;
		a_lines_last[i]	= NULL;
		a_prev_start[i]	= NULL;
	}

	initColormap();

	add_events(Gdk::POINTER_MOTION_MASK);
	add_events(Gdk::BUTTON_PRESS_MASK);
	add_events(Gdk::BUTTON_MOTION_MASK);
	add_events(Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::LEAVE_NOTIFY_MASK);

	// -- Control variables --
	remanaged = false;

	this->flatMode = flatMode ;
	if (flatMode)
		a_lineWidthCoeff = 10 ;
	else
		a_lineWidthCoeff = 20 ;
	prev_begin_track = -1;
}


// --- ~SegmentWidget ---
SegmentWidget::~SegmentWidget()
{
	for (int i = 0; i < a_linesCount; i++)
	{
		LL* now = a_lines[i];

		while (now != NULL)
		{
			LL* old = now;
			now = now->next;
			if ( old->text != NULL )
				free(old->text);
			if ( old->color != NULL )
				free(old->color);
			delete old;
		}
	}

	get_default_colormap()->free_color(clearTextColor);
	get_default_colormap()->free_color(white);
	get_default_colormap()->free_color(black);
	get_default_colormap()->free_color(cursorColor);
	get_default_colormap()->free_color(baseGrey);
}


// --- StartUpdate ---
void SegmentWidget::startUpdate()
{
	a_canDraw = false;
	if ( a_inTransaction == 0 )
		a_updated=false;
	a_inTransaction++;
}


// apply all segment updates : remanage segments and allow drawing
void SegmentWidget::commitUpdate(bool force)
{
	if ( force || a_inTransaction <= 1 )
	{
		if ( a_updated )
			remanageSegments();

		a_inTransaction	= 0;
		a_canDraw		= true;
	}
	else
		a_inTransaction--;
}


// --- On_button_press_event ---
bool SegmentWidget::on_button_press_event(GdkEventButton* p_event)
{
	a_tooltip.stop() ;

/* In non-selectionnable, let's allow to select
 * an element
 */
//	if (!a_selectable)
//		return false;

	int x = (int)roundf(p_event->x);
	int y = (int)roundf(p_event->y);

	y = y - 2;
	if (y < 0)
		y = 0;

	float secs = a_cursorScroll + ((float)x * AudioWidget::AUDIO_ZOOM_MAX * (float)a_currentZoom);

	if (secs > a_duration)
		return false;

	int j = y/20;

	LL* cur = a_lines[j];
	bool ok = false;

	while (cur != NULL && !ok)
	{
		float curStart	= cur->start;
		float curEnd	= cur->end;

		if (cur->audioTrack != NULL)
		{
			curStart	+= cur->audioTrack->getOffset();
			curEnd		+= cur->audioTrack->getOffset();
		}

		int start	= (int)roundf((float)(curStart - a_cursorScroll) / ((float)AudioWidget::AUDIO_ZOOM_MAX * (float)a_currentZoom));
		int end		= (int)roundf((float)(curEnd - a_cursorScroll) / ((float)AudioWidget::AUDIO_ZOOM_MAX * (float)a_currentZoom));

		float startf	= ((curStart - a_cursorScroll) / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
		float endf		= ((curEnd - a_cursorScroll) / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
		float sizef		= endf - startf;

		if ((x >= start && x <= end) || (sizef >= SHORTMIN && sizef <= SHORTMAX && x >= start-5 && x <= end+5))
		{
			int size	= end-start;
			int border	= 2;


			if ((x <= start + border) || (size < 1))
			{
				a_segmentMoving = cur;
				a_leftMoving = true;
				a_lastCur = secs;
			}
			else if (x >= end - border)
			{
				a_segmentMoving = cur;
				a_leftMoving = false;
				a_lastCur = secs;
			}
			else
			{
				if (p_event->button == 1)
				{
					// -- Segment Selection (Extent) --
					if (p_event->state & GDK_SHIFT_MASK)
					{
						if (curStart < a_currentSample)
						{
							a_endSelection		= a_currentSample;
							a_beginSelection	= curStart;
							a_currentSample		= curStart;
						}
						else
						{
							a_beginSelection	= a_currentSample;
							a_endSelection		= curEnd;
						}

						a_signalCursorChanged.emit(a_currentSample);
					}
					else
					{
						a_currentSample		= curStart;
						a_beginSelection	= curStart;
						a_endSelection		= curEnd;

						a_signalCursorChanged.emit(a_currentSample);
					}

					a_signalSelectionChanged.emit(a_beginSelection, a_endSelection);
					a_signalSegmentClicked.emit(cur->audioTrack);
				}
			}
			ok = true;
		}
		cur = cur->next;
	}
	queue_draw();

	return false;
}


// --- On_motion_notify_event ---
bool SegmentWidget::on_motion_notify_event(GdkEventMotion* p_event)
{
	a_tooltip.stop();

	if ( a_canDraw == false ) return false;


	int x = (int)roundf(p_event->x);
	int y = (int)roundf(p_event->y);

	float secs = a_cursorScroll + ((float)x * AudioWidget::AUDIO_ZOOM_MAX * (float)a_currentZoom);

	y = y - 2;

	if (y < 0)
		y = 0;

	if (a_segmentMoving == NULL)
	{
		if (secs > a_duration)
			return false;

		int j = y/20;

		LL* cur = a_lines[j];
		bool ok = false;

		while (cur != NULL && !ok)
		{
			float curStart	= cur->start;
			float curEnd	= cur->end;

			if (cur->audioTrack != NULL)
			{
				curStart += cur->audioTrack->getOffset();
				curEnd += cur->audioTrack->getOffset();
			}

			int start	= (int)roundf((float)(curStart - a_cursorScroll) / ((float)AudioWidget::AUDIO_ZOOM_MAX * (float)a_currentZoom));
			int end		= (int)roundf((float)(curEnd - a_cursorScroll) / ((float)AudioWidget::AUDIO_ZOOM_MAX * (float)a_currentZoom));

			float startf	= ((curStart - a_cursorScroll) / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
			float endf		= ((curEnd - a_cursorScroll) / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
			float sizef		= endf - startf;

			if ((x >= start && x <= end) || (sizef >= SHORTMIN && sizef <= SHORTMAX && x >= start-5 && x <= end+5))
			{
				ok			= true;
				int size	= end-start;
				int border	= 2;

				// -- Segment Tooltip --
				if (a_activeTooltip)
				{
					if (cur->text)
						a_tooltip.set_data(cur->text, cur->start, cur->end);
					else
						a_tooltip.set_data("", -1, -1);
					a_tooltip.launch(*p_event, this);
				}

				if (a_selectable)
				{
					if ((x <= start + border) || (size < 1))
					{
						get_window()->set_cursor(a_cursorLeft);

						move_start	= true;
						move_end	= false;

					}
					else if (x >= end - border)
					{
						get_window()->set_cursor(a_cursorRight);

						move_start	= false;
						move_end	= true;
					}
					else
					{
						get_window()->set_cursor();
					}
				}
			}
			cur = cur->next;
		}

		if (a_selectable && ok == false)
			get_window()->set_cursor();
	}
	else if (a_selectable)
	{
		float curStart	= a_segmentMoving->start;
		float curEnd	= a_segmentMoving->end;

		if (a_segmentMoving->audioTrack != NULL)
		{
			curStart += a_segmentMoving->audioTrack->getOffset();
			curEnd += a_segmentMoving->audioTrack->getOffset();
		}

		if (secs < 0)
			secs = 0;
		else
			if (secs > a_duration)
				secs = a_duration;

		float move = secs - a_lastCur;

		if (a_leftMoving)
		{
			if (curStart + move < 0)
				move = - curStart;

			if (curStart + move > curEnd)
			{
				curStart = curEnd;
				curEnd = curStart + move;
				a_leftMoving = false;
				get_window()->set_cursor(a_cursorRight);
			}
			else
			{
				curStart += move;
			}
		}
		else
		{
			if (curEnd + move > a_duration)
				move = a_duration - curEnd;

			if (curEnd + move < curStart)
			{
				curEnd = curStart;
				curStart = curEnd + move;
				a_leftMoving = true;
				get_window()->set_cursor(a_cursorLeft);
			}
			else
			{
				curEnd += move;
			}
		}

		a_segmentMoving->start	= curStart;
		a_segmentMoving->end	= curEnd;

		if (a_segmentMoving->audioTrack != NULL)
		{
			a_segmentMoving->start -= a_segmentMoving->audioTrack->getOffset();
			a_segmentMoving->end -= a_segmentMoving->audioTrack->getOffset();
		}

		queue_draw();
	}

	if (a_selectable)
		a_lastCur = secs;


	return false;
}


// --- On_button_release_event ---
bool SegmentWidget::on_button_release_event(GdkEventButton* p_event)
{
	guint modifier = p_event->state;

	if (!a_selectable)
		return false;

	if (a_segmentMoving != NULL)
	{
		remanageSegments();

		if (modifier & GDK_CONTROL_MASK)
		{
			TRACE << "Special Segment Resize : CTRL + Left-Click" << std::endl ;

			if (move_start)
				a_signalSegmentModified.emit(a_segmentMoving->id,
											 a_currentSample,
											 a_segmentMoving->end);

			if (move_end)
				a_signalSegmentModified.emit(a_segmentMoving->id,
											 a_segmentMoving->start,
											 a_currentSample);
		}
		else
		{
			a_signalSegmentModified.emit(a_segmentMoving->id, a_segmentMoving->start, a_segmentMoving->end);
		}
		a_segmentMoving = NULL;
	}

	move_start = move_end = false;

	return false;
}


// --- On_leave_notify_event ---
bool SegmentWidget::on_leave_notify_event(GdkEventCrossing* p_event)
{
	a_tooltip.stop();
	get_window()->set_cursor();

	return false;
}


// --- On_configure_event ---
bool SegmentWidget::on_configure_event(GdkEventConfigure* p_event)
{
	if (!win)
		win = get_window();
	else
		win->get_size(width, height);

	queue_draw();
}


// --- AddSegment ---
void SegmentWidget::addSegment(const string& p_id, float p_start, float p_end, const char* p_text, const char* p_color, bool p_skipable, int p_weight, bool hidden_order, bool skip_highlight, AudioTrackWidget* p_audioTrack)
{
	a_canDraw=false;
	addSegment2(p_id, p_start, p_end, p_text, p_color, p_skipable, p_weight, hidden_order, skip_highlight, p_audioTrack);
	if ( ! a_inTransaction ) remanageSegments();
}


// --- AddSegment2 ---
void SegmentWidget::addSegment2(const string& p_id, float p_start, float p_end, const char* p_text, const char* p_color, bool p_skipable, int p_weight, bool hidden_order, bool skip_highlight, AudioTrackWidget* p_audioTrack)
{
    LL* now         = new LL();
    now->id         = p_id;
    now->start      = p_start;
    now->end        = p_end;
    now->text       = (p_text ? strdup(p_text) : NULL );
    now->color      = strdup(p_color) ;  // ATTENTION ! ICI on presuppose que p_color ne sera jamais desalloue par l'appelant !
    now->skipable   = p_skipable;
    now->weight     = p_weight;
    now->audioTrack = p_audioTrack;
    now->hidden_order = hidden_order ;
    now->skip_highlight = skip_highlight ;
    insertInLines(now);
}


// --- AddSegmentToCache ---
void SegmentWidget::addSegmentToCache(const string& p_id, float p_start, float p_end, const char* p_text, const char* p_color, bool p_skipable, int p_weight, bool hidden_order, bool skip_highlight, AudioTrackWidget* p_audioTrack)
{
	LL* now			= new LL();
	now->id			= p_id;
	now->start		= p_start;
	now->end		= p_end;
	now->text		= (p_text ? strdup(p_text) : NULL );
	now->color		= strdup(p_color) ;  // ATTENTION ! ICI on presuppose que p_color ne sera jamais desalloue par l'appelant !
	now->skipable	= p_skipable;
	now->weight		= p_weight;
	now->audioTrack = p_audioTrack;
	now->hidden_order = hidden_order ;
	now->skip_highlight = skip_highlight ;
	now->next = NULL;

	if ( a_cache_start == NULL )
	{
		a_cache_start	= now;
		a_cache_tail	= now;
	}
	else
	{
		a_cache_tail->next	= now;
		a_cache_tail		= now;
	}
}

// --- manageCachedSegments ---
void SegmentWidget::manageCachedSegments()
{
	a_canDraw = false;
	emptyLines();
	remanageSegments2(a_cache_start);
	a_cache_start = a_cache_tail = NULL;
}


void SegmentWidget::insertInLines(LL* now)
{
	if (now && now->hidden_order && now->weight > 0)
		return ;

	int i = 0;
	bool ok = false;

	while (i < 10 && !ok)
	{
		if (i > a_linesCount)
			a_linesCount = i;

		if (a_lines[i] == NULL)
		{
			a_lines[i] = now;
			now->next = NULL;
			ok = true;
			a_linesCount++;
			break;
		}

		LL* cur = a_lines[i];
		LL* prec = NULL;

		float nowStart	= now->start;
		float nowEnd	= now->end;


		while (cur != NULL && !ok)
		{
			float curStart	= cur->start;
			float curEnd	= cur->end;

			if ((nowEnd - curStart) < AudioWidget::AUDIO_ZOOM_MAX )
			{
				if (prec)
					prec->next = now;
				else
					a_lines[i] = now;

				now->next = cur;
				ok = true;
			}
			else
			if (!((nowStart - curEnd) > -1.*AudioWidget::AUDIO_ZOOM_MAX)) break;

			prec = cur;
			cur = cur->next;
		}

		if (cur == NULL && !ok)
		{
			prec->next = now;
			now->next = NULL;
			ok = true;
			a_prev_start[i] = NULL;
		}
		i++;
	}
	if ( ok ) a_updated=true;
}


// --- RemoveSegment ---
void SegmentWidget::removeSegment(int p_line, int p_index)
{
	LL* toDel = NULL;
	a_canDraw = false;

	if (p_index == 0)
	{
		toDel = a_lines[p_line];
		a_lines[p_line] = a_lines[p_line]->next;
	}
	else
	{
		LL* now = a_lines[p_line];

		for (int i = 1; i < p_index; i++)
			now = now->next;

		toDel		= now->next;
		now->next	= now->next->next;
	}

	if ( toDel != NULL )
	{
		if ( a_segmentMoving == toDel )	a_segmentMoving = NULL;
		if ( toDel->text != NULL )		free(toDel->text);
		delete toDel;
		a_updated=true;
		a_prev_start[p_line] = NULL;
	}

	if ( ! a_inTransaction )
		remanageSegments();
}


// --- RemoveSegment ---
void SegmentWidget::removeSegment(const string& p_id)
{
	a_canDraw = false;

	for (int i = 0; i < a_linesCount; i++)
	{
		LL* now = a_lines[i];
		LL* prev = now;

		while (now != NULL)
		{
			if (now->id == p_id)
			{
				if ( a_segmentMoving == now )
					a_segmentMoving = NULL;

				if ( a_lines[i] == now )
					a_lines[i] = now->next;
				else
					prev->next = now->next;

				if ( a_lines_last[i] == now )
					a_lines_last[i] = (prev == now ? now->next : prev);

				if ( now->text != NULL )
					free(now->text);

				delete now;
				a_prev_start[i] = NULL;
				a_updated = true;
				break;
			}
			prev=now;
			now = now->next;
		}
	}

	if ( ! a_inTransaction )
		remanageSegments();
}



// --- RemoveSegments ---
void SegmentWidget::removeSegments(AudioTrackWidget* p_audioTrack)
{
	a_canDraw=false;
	for (int i = 0; i < a_linesCount; i++)
	{
		LL* now = a_lines[i];
		LL* prec = NULL;
		while (now != NULL)
		{
			LL* next = now->next;

			if (now->audioTrack == p_audioTrack)
			{
				if (prec == NULL)
					a_lines[i] = next;
				else
					prec->next = next;

				if ( now->text != NULL ) free(now->text);
				delete now;
				now = next;
			}
			else
			{
				prec = now;
				now = next;
			}
		}
		a_prev_start[i] = NULL;
	}

	if ( ! a_inTransaction ) remanageSegments();
}


// --- RemoveSegments ---
void SegmentWidget::emptyLines()
{
	for (int i = 0; i < a_linesCount; i++)
	{
		LL* now = a_lines[i];
		LL* next ;
		while (now != NULL)
		{
			next = now->next;
			if ( now->text != NULL ) free(now->text);
			delete now;
			now = next;
		}
		a_lines[i] = NULL;
		a_prev_start[i] = NULL;
	}
}




// --- NextNotSkipable ---
float SegmentWidget::nextNotSkipable(float p_secs)
{
	float min = -1;

	for (int i = 0; i < a_linesCount; i++)
	{
		LL* now = a_lines[i];

		while (now != NULL)
		{
			if (!now->skipable)
			{
				float curStart	= now->start;
				float curEnd	= now->end;

				if (now->audioTrack != NULL)
				{
					curStart += now->audioTrack->getOffset();
					curEnd += now->audioTrack->getOffset();
				}

				if ((p_secs >= curStart) && (p_secs < curEnd))
					return p_secs;
				else
					if (p_secs < curStart)
					{
						if ((min == -1) || (curStart < min))
							min = curStart;
					}
			}

			now = now->next;
		}
	}
	return min;
}


// --- EndSegment ---
bool SegmentWidget::endSegment(float p_secs, float p_size, float& p_endSeg)
{
	for (int i = 0; i < a_linesCount; i++)
	{
		LL* now = a_lines_last[i];

		if (now != NULL)
		{
			if (now->end > p_secs)
			{
				p_endSeg = now->end;
				return true;
			}
		}
	}

	return false;
}


// --- InitColormap ---
void SegmentWidget::initColormap()
{
	string color = GtUtil::getBaseColor(NULL) ;
	if (!color.empty())
		baseGrey.set(color) ;
	else
		baseGrey.set("grey") ;

	white.set("white");
	black.set("black");

	cursorColor.set( a_cursorColor.c_str() );
	clearTextColor.set_rgb_p(0.9, 0.9, 0.9) ;

	get_default_colormap()->alloc_color(clearTextColor);
	get_default_colormap()->alloc_color(white);
	get_default_colormap()->alloc_color(black);
	get_default_colormap()->alloc_color(cursorColor);
	get_default_colormap()->alloc_color(baseGrey);
}


// --- On_expose_event ---
bool SegmentWidget::on_expose_event(GdkEventExpose* p_event)
{
	// -- Exceptions --
	if (!a_canDraw || p_event == NULL)
		return false;

	gc = Gdk::GC::create( get_window() );

	w = 0;
	h = 0;

	get_window()->set_background(white);
	get_window()->clear();
	get_window()->get_size(w, h);

	begin			= (int)(a_cursorScroll / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	w2				= (int)(a_duration / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	cursor			= (int)(a_currentSample / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom) - begin;
	beginSelection	= (int)(a_beginSelection / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	endSelection	= (int)(a_endSelection / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	veryEnd 		= begin + w;

	if (w2 < veryEnd)
		veryEnd = w2;

	// Clip coords
	GdkRectangle rect = p_event->area;

	clip_width	= rect.width;
	clip_height	= rect.height;
	clip_x		= rect.x;

	// --- Update Mode ---
	localUpdate = (clip_width != w);

	// --- Drawing ---
	drawArea();
	drawTracks();
	drawCursor();

	set_size_request(-1, a_lineWidthCoeff * a_linesCount + 1);

	return false;
}


// --- DrawArea ---
void SegmentWidget::drawArea()
{
	// -- Bg Rectangle --
	gc->set_foreground(baseGrey);
	get_window()->draw_rectangle(gc, true, clip_x, 0, clip_width, clip_height);
}


// --- DrawCursor ---
void SegmentWidget::drawCursor()
{
	if (cursor < veryEnd && cursor >= 0)
	{
		gc->set_foreground(cursorColor);

		for (int i = 0; i < a_cursorSize; i++)
		{
			get_window()->draw_line(gc, cursor+i, 0, cursor+i, clip_height);
		}
	}
}


// --- DrawTracks ---
void SegmentWidget::drawTracks()
{
	for (int j = 0; j < a_linesCount; j++)
	{
		// -- Background grey zone --
		gc->set_foreground(baseGrey);
		get_window()->draw_rectangle(gc, true, clip_x, j*a_lineWidthCoeff+1, clip_width, j*a_lineWidthCoeff+1+18);

		/* PLR optimisation pour ne pas repartir du début à chaque fois, surtout en lecture. */
		LL* cur = NULL;
		if ( begin == prev_begin_track )
			cur = a_prev_start[j];
		else a_prev_start[j] = NULL;
		if ( cur == NULL )
			cur = a_lines[j];

		// PLR avoid browsing full list each time !
		bool done1 = false;
		while (cur != NULL)
		{
			if ( drawSegment(cur, j) ) {
				if ( a_prev_start[j] == NULL  ) a_prev_start[j] = cur;
				done1=true;
			} else { if (done1) break; }
			cur = cur->next;
		}
	}
	prev_begin_track = begin;
}


// --- DrawSegment ---
bool SegmentWidget::drawSegment(LL *cur, int j)
{
	if (cur && cur->hidden_order && cur->weight > 0)
		return true ;

	float curStart	= cur->start;
	float curEnd	= cur->end;

	if (cur->audioTrack != NULL)
	{
		curStart+= cur->audioTrack->getOffset();
		curEnd	+= cur->audioTrack->getOffset();
	}

	int start	= (int)(curStart/ AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom) - begin;
	int end		= (int)(curEnd	/ AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom) - begin;
	int start2	= start;
	int end2	= end;

	// -- Position Check --
	if (localUpdate)
	{
		if ( (start > (clip_x + clip_width)) || (end < clip_x) )
			return false;
	}
	else
	{
		if (start > w || end < 0)
			return false;

		if (start < 0)
			start = -1;

		if (end > w)
			end = w+1;
	}

	// -- Drawing --
	bool inSelection = (a_currentSample > curStart && a_currentSample < curEnd) ||
						(!(a_endSelection <= curStart) && !(a_beginSelection >= curEnd));

	Gdk::Color c1(cur->color);
	float moyenne	= (c1.get_red_p() + c1.get_green_p() + c1.get_blue_p()) / 3;

	if ((cur->audioTrack != a_selectedAudioTrack) || !cur->audioTrack->isActivated())
		c1.set_rgb_p(moyenne, moyenne, moyenne);

	if (inSelection && !cur->skip_highlight)
		c1.set_rgb_p(c1.get_red_p(), c1.get_green_p() , c1.get_blue_p());
	else
		c1.set_rgb_p(c1.get_red_p()*0.86, c1.get_green_p()*0.86, c1.get_blue_p()*0.86);

	get_default_colormap()->alloc_color(c1);

	start	= (start < clip_x ? clip_x : start);
	end		= (end > (clip_x + clip_width) ? (clip_x + clip_width) : end);

	// -- Segment Fill --
	gc->set_foreground(c1);
	get_window()->draw_rectangle(gc, true, start, j*a_lineWidthCoeff, end-start, a_lineWidthCoeff);

	// -- Segment Border --
	gc->set_foreground(black);

	get_window()->draw_line(gc, start, j*a_lineWidthCoeff, end, j*a_lineWidthCoeff);
	get_window()->draw_line(gc, start, (j+1)*a_lineWidthCoeff, end, (j+1)*a_lineWidthCoeff);

	if (end2 <= end)
		get_window()->draw_line(gc, end2, j*a_lineWidthCoeff, end2, (j+1)*a_lineWidthCoeff);

	if (start2 >= start)
		get_window()->draw_line(gc, start2, j*a_lineWidthCoeff, start2, (j+1)*a_lineWidthCoeff);


	// -- Selection Colors ? --
//	if (inSelection)
		gc->set_foreground(black);
//	else
//		gc->set_foreground(clearTextColor);

	// -- Text --
	if ( (end - start > 6))
	{
		Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(get_pango_context());
		if ( cur->text == NULL )
			a_signalGetText(cur->id, cur->text);

		if (!flatMode)
		{
			Glib::ustring space = " " ;
			layout->set_text(space + cur->text + space);
			//TODO compute length of visible part and add a space frontier at end
			get_window()->draw_layout(gc, start2 + 3, j*a_lineWidthCoeff+2, layout);
		}
	}

	// -- Astuce de sioux - Pango::Layout --
	gc->set_foreground(baseGrey);
	get_window()->draw_rectangle(gc, true, end+1, j*a_lineWidthCoeff+1, w - (end+1), j*a_lineWidthCoeff+1+18);

	return true;
}


// --- SetCursor ---
void SegmentWidget::setCursor(float p_cursor)
{
	float lastCursor	= a_currentSample;
	a_currentSample		= p_cursor;

	int c1	= (int) (lastCursor / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	int c2	= (int) (p_cursor / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);

	c1 -= begin;
	c2 -= begin;

	int w = width;
	int h = height;

	// -- Tracé des curseurs de segments --
	if (c1 == c2)
		return;

	queue_draw_area(c1, 0, a_cursorSize, h);
	queue_draw_area(c2, 0, a_cursorSize, h);


	int w2		= (int)(a_duration / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	int veryEnd	= begin+w;

	if (w2 < veryEnd)
		veryEnd = w2;

	for (int j = 0; j < a_linesCount; j++)
	{
		LL* cur = NULL;

		// Storing cursors
		if (a_lines_last[j] == NULL)
		{
			cur = a_lines[j];
			a_lines_last[j] = cur;
		}
		else
		{
			cur = a_lines_last[j];
		}

		if (remanaged)
		{
			remanaged = false;
			cur = findSegment(j, p_cursor);
			a_lines_last[j] = cur;
		}


		if (cur == NULL)
			continue;


		// --- Segment 1 ---
		seg1_start	= cur->start;
		seg1_end	= cur->end;

		// --- Offset check ---
		if (cur->audioTrack != NULL)
		{
			seg1_start	+= cur->audioTrack->getOffset();
			seg1_end	+= cur->audioTrack->getOffset();
		}

		int start	= (int)(seg1_start / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom) - begin;
		int end		= (int)(seg1_end / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom) - begin;

		bool inSegment1 =	( (p_cursor >= seg1_start) && (p_cursor < seg1_end) ) ||
							( (a_endSelection > seg1_start) && (a_beginSelection < seg1_end) );


		// --- Test Segment 1 ---
		if (!inSegment1)
		{
			// --- Segment 2 ---
			queue_draw_area(start, j*a_lineWidthCoeff, end - start, a_lineWidthCoeff);
			cur = cur->next;

			if (cur == NULL)
			{
				// -- No segment 2 --> Search --
				cur = findSegment(j, p_cursor);

				if (cur == NULL)
					continue;
			}

			// -- Check Next Segment --
			seg2_start	= cur->start;
			seg2_end	= cur->end;

			if (cur->audioTrack != NULL)
			{
				seg2_start	+= cur->audioTrack->getOffset();
				seg2_end	+= cur->audioTrack->getOffset();
			}

			int start2	= (int)(seg2_start / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom) - begin;
			int end2	= (int)(seg2_end / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom) - begin;

			bool inSegment2 =	( (p_cursor >= seg2_start) && (p_cursor <= seg2_end) ) ||
								( (a_endSelection > seg2_start) && (a_beginSelection < seg2_end) );

			// -- Discontinuity handler --
			if (!inSegment2)
				cur = findSegment(j, p_cursor);

			if (cur == NULL)
				continue;

			// -- Segment 3 --
			seg2_start	= cur->start;
			seg2_end	= cur->end;

			if (cur->audioTrack != NULL)
			{
				seg2_start	+= cur->audioTrack->getOffset();
				seg2_end	+= cur->audioTrack->getOffset();
			}

			start2	= (int)(seg2_start / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom) - begin;
			end2	= (int)(seg2_end / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom) - begin;


			if (start2 < veryEnd && end2 > 0)
			{
				// Boundaries Check
				if (end2 > veryEnd + 10)
					end2 = veryEnd + 10;

				if (end2 > 32000)
					end2 = 32000;

				if (start2 < -32000)
					start2 = -32000;

				a_lines_last[j] = cur;

				queue_draw_area(start2, j*a_lineWidthCoeff, end2 - start2, a_lineWidthCoeff);
			}
		}

		queue_draw_area(start, j*a_lineWidthCoeff, end - start, a_lineWidthCoeff);
	}
}


// --- FindSegment ---
LL* SegmentWidget::findSegment(int track, float p_cursor)
{
	LL* segment = a_lines[track];

	while (segment != NULL)
	{
		float off = 0.0;
		if (segment->audioTrack != NULL)
			off = segment->audioTrack->getOffset();
		if ( (p_cursor >= (segment->start+off)) && (p_cursor <= (segment->end+off)) )
			return segment;

		segment = segment->next;
	}

	return NULL;
}


// --- SetSelection ---
void SegmentWidget::setSelection(float p_beginSelection, float p_endSelection)
{
	a_beginSelection = p_beginSelection;
	a_endSelection = p_endSelection;

	queue_draw();
}


// --- SetZoom ---
void SegmentWidget::setZoom(int p_factor)
{
	a_w = (int)(a_duration / AudioWidget::AUDIO_ZOOM_MAX / (float)p_factor);
	a_currentZoom = p_factor;
	queue_draw();
}


// --- SetScroll ---
void SegmentWidget::setScroll(float p_cursorScroll)
{
	if (p_cursorScroll != a_cursorScroll)
	{
		a_cursorScroll = p_cursorScroll;
		queue_draw();
	}
}


vector<string> SegmentWidget::getCurrentSegmentId()
{
	vector<string> v;

	for (int i=0; i < a_linesCount; i++)
	{
		if (a_lines_last[i] != NULL)
			v.push_back( a_lines_last[i]->id );
	}

	return v;
}


// --- RemanageSegments ---
void SegmentWidget::remanageSegments()
{
	a_canDraw	= false;
	remanaged	= true;
	LL* start	= a_lines[0];
	int cnt		= 0;

	for (int i = 0; i < a_linesCount; i++)
	{
		if ( a_lines[i] != NULL )
		{
			++cnt;
			LL* now = a_lines[i];
			while (now->next != NULL)
			{
				now = now->next;
				++cnt;
			}

			if (i < a_linesCount-1)
				now->next = a_lines[i+1];
		}
		a_prev_start [i] = NULL;
	}

	remanageSegments2(start);
}


// --- RemanageSegments ---
void SegmentWidget::remanageSegments2(LL* start)
{
	a_canDraw=false;
	remanaged = true;
	bool restore_last = false;
	LL* weight_sorted_start[10];
	LL* weight_sorted_tail[10];
	int cnt = 0;

	for (int i = 0; i < 10; i++)
	{
		if ( a_lines_last[i] != NULL )
			restore_last = true;

		a_lines[i]				= NULL;
		a_lines_last[i]			= NULL;
		a_prev_start[i]			= NULL;
		weight_sorted_start[i]	= NULL;
		weight_sorted_tail[i]	= NULL;
	}

	a_linesCount = 0;

	// Weight-sorted
	LL* now = start;
	LL* suiv = NULL;
	int i;

	while (now != NULL)
	{
		cnt++;
		i = now->weight;
		if ( i < 0 )	i = 0;
		if ( i >= 10 )	i = 9;

		if ( weight_sorted_start[i] == NULL )
			weight_sorted_start[i] = now;
		else
			weight_sorted_tail[i]->next = now;
		weight_sorted_tail[i] = now;
		now = now->next;
	}

	start = NULL;

	for (int i = 0; i < 10; i++)
	{
		if ( weight_sorted_start[i] != NULL )
		{
			if ( start == NULL )
			{
				start	= weight_sorted_start[i];
				suiv	= weight_sorted_tail[i];
			}
			else
			{
				suiv->next	= weight_sorted_start[i];
				suiv		= weight_sorted_tail[i];
			}
		}
	}

	if ( suiv != NULL )
		suiv->next = NULL;

	now = start;
	while (now != NULL)
	{
		suiv = now->next;
		insertInLines(now);
		now = suiv;
	}

	if ( restore_last )
		for (int i = 0; i < a_linesCount; i++)
			a_lines_last[i] = findSegment(i, a_currentSample);

	a_canDraw = true;
	a_updated = false;
	queue_draw();
}

// --- Previous ---
void SegmentWidget::previous()
{
	float previous	= 0;
	float epsilon	= 0.001;

	for (int i = 0; i < a_linesCount; i++)
	{
		LL* cur = a_lines[i];
		float offset = 0.0;
		if (cur->audioTrack != NULL)
			offset = cur->audioTrack->getOffset();

		while (cur != NULL)
		{
			float curStart	= cur->start + offset;
			float curEnd	= cur->end + offset;

			if (curStart <= a_currentSample && curEnd > a_currentSample)
			{
				if ( curStart < (a_currentSample - (2*epsilon)) )
					previous = curStart;
				break;
			}
			previous = curStart;

			cur = cur->next;
		}
	}

	a_currentSample = previous;
	a_signalCursorChanged.emit(a_currentSample);
	a_beginSelection = -1;
	a_endSelection = -1;
	a_signalSelectionChanged.emit(a_beginSelection, a_endSelection);
}


// --- Next ---
void SegmentWidget::next()
{
	float next		= a_duration;
	float epsilon	= 0.001;

	for (int i = 0; i < a_linesCount; i++)
	{
		LL* cur = a_lines[i];
		float offset = 0.0;
		if (cur->audioTrack != NULL)
			offset = cur->audioTrack->getOffset();

		while (cur != NULL)
		{
			float curStart	= cur->start + offset;
			float curEnd	= cur->end + offset;

			if (curStart <= a_currentSample && curEnd > a_currentSample)
			{
				if ( cur->next != NULL )
				{
					cur=cur->next;
					next = cur->start;
				}
					else next = a_duration ;
				break;
			}
			cur = cur->next;
		}
	}

	a_currentSample = next;
	a_signalCursorChanged.emit(a_currentSample);
	a_beginSelection = -1;
	a_endSelection = -1;
	a_signalSelectionChanged.emit(a_beginSelection, a_endSelection);
}


// --- GetLastBegin ---
float SegmentWidget::getLastBegin(float p_cursor)
{
	float lastBegin = -1;

	for (int i = 0; i < a_linesCount; i++)
	{
		LL* cur = a_lines[i];

		while (cur != NULL)
		{
			float curStart	= cur->start;
			float curEnd	= cur->end;

			if (cur->audioTrack != NULL) {
				curStart += cur->audioTrack->getOffset();
				curEnd += cur->audioTrack->getOffset();
			}

			if (curStart > lastBegin && curStart < p_cursor && curEnd > p_cursor)
				lastBegin = curStart;

			cur = cur->next;
		}
	}
	return lastBegin;
}


// --- UpdateSegmentText ---
void SegmentWidget::updateSegmentText(const string& p_id, const string& p_text)
{
	for (int i = 0; i < a_linesCount; i++)
	{
		int j = 0;
		LL* now = a_lines[i];

		while (now != NULL)
		{
			if (now->id == p_id)
			{
				if ( now->text != NULL )
					free(now->text);

				now->text = strdup(p_text.c_str());
				queue_draw();
				return;
			}
			now = now->next;
			j++;
		}
	}
}


void SegmentWidget::set_color(Glib::ustring mode, Glib::ustring color)
{
	if (mode.compare(TAG_COLORS_AUDIO_WAVE_CURSOR)==0)
		setCursorColor(color);
}

void SegmentWidget::setSelectable(bool p_selectable)
{
	a_selectable = p_selectable;
}

} // namespace

