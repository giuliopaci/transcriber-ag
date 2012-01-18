/********************************************************************************/
/******************** (c) Bertin Technologies 2006 - 2009  **********************/
/*			  TranscriberAG version 1.x				*/
/* Authors: 									*/
/* See COPYING for license information										  	*/
/* 	         								*/
/********************************************************************************/

#include "GraphicsCanvas.h"
#include <string>
#include <sstream>

using namespace std;

/** GraphicsCanvas */
GraphicsCanvas::GraphicsCanvas(int inOpacity)
	: opacity(inOpacity)
{
	/** Variables */
	state		= Idle;
	selected	= NULL;

	/** Initialization methods */
	initCanvas();
	initSignals();
}


/** initCanvas */
void GraphicsCanvas::initCanvas()
{
	Gnome::Canvas::init();
	set_center_scroll_region(false);
	
	group = root();
}


/** initSignals */
void GraphicsCanvas::initSignals()
{}


/** getOpacity */
int GraphicsCanvas::getOpacity()
{
	return opacity;
}


/** setOpacity */
void GraphicsCanvas::setOpacity(int inOpacity)
{
	opacity = inOpacity;
}


/*************/
/*   ITEMS   */
/*************/

/** addRectangle */
Rect*
GraphicsCanvas::addRectangle(int x1, int y1, int x2, int y2)
{
	return new Gnome::Canvas::Rect(*root(), x1, y1, x2, y2);
}


/** addEllipse */
Ellipse*
GraphicsCanvas::addEllipse(int x1, int y1, int x2, int y2)
{
	return new Gnome::Canvas::Ellipse(*root(), x1, y1, x2, y2);
}


/** addText */
Text*
GraphicsCanvas::addText(Glib::ustring text, int x, int y)
{
	return new Gnome::Canvas::Text(*root(), x, y, text);
}

/** addPixbuf (default) */
Pixbuf*
GraphicsCanvas::addPixbuf(int x, int y)
{
	Pixbuf* item = new Gnome::Canvas::Pixbuf(*root());

	if (x != 0 || y != 0)
		item->move(x, y);

	return item;
}


/** addPixbuf (provided pixbuf) */
Pixbuf*
GraphicsCanvas::addPixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, int x, int y)
{
	return new Gnome::Canvas::Pixbuf(*root(), x, y, pixbuf);
}


/**************/
/*   EVENTS   */
/**************/

/** on_button_press_event */
bool GraphicsCanvas::on_button_press_event(GdkEventButton *e)
{
	double world_x, world_y;

	cur_x = e->x;
	cur_y = e->y;

	window_to_world(cur_x, cur_y, world_x, world_y);

	selected = get_item_at(world_x, world_y);

	if (e->button == 1)
		if (selected)
			state = ItemMotion;
		else
			state = ItemCreation;

	return true;
}


/** on_button_release_event */
bool GraphicsCanvas::on_button_release_event(GdkEventButton*)
{
	state = Idle;

	return true;
}


/** on_motion_notify_event */
bool GraphicsCanvas::on_motion_notify_event(GdkEventMotion *e)
{
	switch (state)
	{
		case ItemMotion:
			moveItem(selected, e->x - cur_x, e->y - cur_y);
			cur_x = e->x;
			cur_y = e->y;
			break;

		case ItemCreation:
			break;

		default:
			return true;
	}
}


/** moveItem */
void GraphicsCanvas::moveItem(Item *item, double dx, double dy)
{
	// -- Bounds --
	double x1, x2, y1, y2;
	int cx = get_width();
	int cy = get_height();

	// -- Check --
	item->get_bounds(x1, y1, x2, y2);
	
	x1+=dx;
	x2+=dx;
	y1+=dy;
	y2+=dy;

	if ( (x1 >= 0 && x1 <= cx) && (x2 >= 0 && x2 <= cx) &&
		 (y1 >= 0 && y1 <= cy) && (y2 >= 0 && y2 <= cy) )
	{
		item->move(dx, dy);
	}
}

