/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class ScaleWidget
 *
 * ScaleWidget...
 */

#include <string.h>
#include <sys/time.h>
#include "AudioWidget.h"
#include "Common/widgets/GtUtil.h"

namespace tag {

// --- ScaleWidget ---
ScaleWidget::ScaleWidget(float p_duration) : Gtk::DrawingArea()
{
	a_duration		= p_duration;
	a_currentZoom	= 1;
	a_zoomMax		= 1;
	a_cursorScroll	= 0.0;
	a_w				= 0;
	a_h				= 0;

	initColormap();
}


// --- ~ScaleWidget ---
ScaleWidget::~ScaleWidget()
{
	get_default_colormap()->free_color(white);
	get_default_colormap()->free_color(black);
	get_default_colormap()->free_color(baseGrey);
}


// --- InitColormap ---
void ScaleWidget::initColormap()
{
	string color = GtUtil::getBaseColor(NULL) ;
	if (!color.empty())
		baseGrey.set(color) ;
	else
		baseGrey.set("grey") ;

	white.set("white");
	black.set("black");

//	lightGrey.set_rgb_p(0.9, 0.9, 0.9);

	get_default_colormap()->alloc_color(white);
	get_default_colormap()->alloc_color(black);
	get_default_colormap()->alloc_color(baseGrey);
}


// --- On_expose_event ---
bool ScaleWidget::on_expose_event(GdkEventExpose* p_event)
{
	gc = Gdk::GC::create(get_window());

	get_window()->set_background(baseGrey);
	get_window()->clear();

	int w = 0;
	int h = 0;
	get_window()->get_size(w, h);

	int begin = (int)(a_cursorScroll / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);

	float visible = ((float)w * (float)a_currentZoom * AudioWidget::AUDIO_ZOOM_MAX);
	visible = roundf(visible*100.0)/100.0;

	gc->set_foreground(white);

	float visible2 = visible;

	if (visible2 + a_cursorScroll > a_duration)
		visible2 = a_duration - a_cursorScroll;

	// -- block eventual bug (fixed but better to check)
	if (visible2 < 0)
		visible2 = visible ;

	if (visible2<0)
	{
		TRACE << "visible2=" << visible2 << " - visible=" << visible << " - a_cursorScroll=" << a_cursorScroll << " - a_duration=" << a_duration << std::endl ;
		gdk_beep() ;
		return true ;
 	}

	int v2 = (int)(visible2 / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	get_window()->draw_rectangle(gc, true, 0, 0, v2, h);

	char str[80];
	gc->set_foreground(black);
	get_window()->draw_line(gc, 0, 0, v2, 0);

	float scale = 1.0;
	if (visible >= 1.5	&& visible < 20)	scale = 10.0;
	if (visible >= 20	&& visible < 180)	scale = 100.0;
	if (visible >= 180	&& visible < 1800)	scale = 600.0;
	if (visible >= 1800)					scale = 6000.0;

	float dec = scale / (AudioWidget::AUDIO_ZOOM_MAX * (float)a_currentZoom * 100.0);

	for (int i = (int)(a_cursorScroll*10.0/scale); i <= ((int)((a_cursorScroll+visible2)*10.0/scale)) ; i++)
	{
		int x = (int)((float)i*scale / 10.0 / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom)-begin;

		for (int j = 1; j <= 9; j++)
		{
			if ((i*10.0+j)*scale/100.0 > a_duration)
				break;
			get_window()->draw_line(gc, x+(int)(dec*j), 0, x+(int)(dec*j), 3);
		}

		Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(get_pango_context());

		int h = (i*(int)scale)/36000;
		int m = ((i*(int)scale)/600)%60;
		int s = ((i*(int)scale)/10)%60;
		int dx = (i*(int)scale)%10;

		if (scale == 1.0)
			if (h == 0)
				if (m == 0)
					sprintf(str, "%d,%d", s, dx);
				else
					sprintf(str, "%d:%.2d,%d", m, s, dx);
			else
				sprintf(str, "%d:%.2d:%.2d,%d", h, m, s, dx);
		else
			if (h == 0)
				if (m == 0)
					sprintf(str, "%d", s);
				else
					sprintf(str, "%d:%.2d", m, s);
			else
				sprintf(str, "%d:%.2d:%.2d", h, m, s);

		layout->set_markup(MarkupSmall(str).c_str());
		get_window()->draw_line(gc, x, 0, x, 7);
		get_window()->draw_layout(gc, x-(strlen(str)*3), 7, layout);
	}

	set_size_request(-1, 19);
	
	return false;
}


// --- SetZoom ---
void ScaleWidget::setZoom(int p_factor)
{
	a_w = (int)(a_duration / AudioWidget::AUDIO_ZOOM_MAX / (float)p_factor);
	a_currentZoom = p_factor;
	queue_draw();
}


// --- SetScroll ---
void ScaleWidget::setScroll(float p_cursorScroll)
{
	a_cursorScroll = p_cursorScroll;
	queue_draw();
}

} // namespace
