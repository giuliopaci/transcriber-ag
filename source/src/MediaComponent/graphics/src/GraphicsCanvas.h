/********************************************************************************/
/******************** (c) Bertin Technologies 2006 - 2009  **********************/
/*			  TranscriberAG version 1.x				*/
/* Authors: 									*/
/* See COPYING for license information										  	*/
/* 	         								*/
/********************************************************************************/

/** @file */

#ifndef GRAPHICSCANVAS_H
#define GRAPHICSCANVAS_H

#include <libgnomecanvasmm.h>

using namespace Gnome::Canvas;


/**
 * Custom enum type, describing canvas states, such as :
 * - Idle :		canvas is available for any operations
 * - Creation :	an item is being created
 * - Motion :	an item is being moved
 * - Resize :	an item is being resized 
 */
typedef enum
{
	Idle = 0,
	ItemCreation,
	ItemMotion,
	ItemResize,
} LayerState;


/**
 * Custom enum type, describing item available types :
 * - Rectangle
 * - Ellipse (includes Circles)
 * - Pixbuf (External pictures)
 * - Text
 */
typedef enum
{
	ItemRectangle = 0,
	ItemEllipse,
	ItemPixbuf,
	ItemText
} ItemShape;


/**
 * @class	GraphicsCanvas
 * @ingroup	MediaComponent
 *
 * Custom canvas (based on GnomeCanvas), providing support for :
 * - Text items
 * - Geometric items (ellipses / rectangles)
 * - Custom transparency (on compatible systems)
 * - Items graphical interactions (currently, moving items is enabled)
 */
class GraphicsCanvas : public CanvasAA
{
public:
	/**
	 * Default GraphicsCanvas constructor
	 * @param opacity	Custom opacity level (requires hardware with transparency support)
	 */
	GraphicsCanvas(int opacity=0);

	/**
	 * Accessor for opacity level
	 * @return Opacity value
	 */
	int		getOpacity();

	/**
	 * Setter for opacity level
	 * @param opacity	New opacity value
	 */
	void	setOpacity(int opacity);

	/**
	 * Instantiates a new rectangle item
	 * @param x1		First point : Leftmost X position (in canvas coordinates)
	 * @param y1		First point : Topmost Y position (in canvas coordinates)
	 * @param x2		Second point: Rightmost X position (in canvas coordinates)
	 * @param y2		Second point: Bottommost Y position (in canvas coordinates)
	 * @return			Rectangle item pointer
	 */
	Rect*		addRectangle(int x1 = 0,
							 int y1 = 0,
							 int x2 = 100,
							 int y2 = 50);

	/**
	 * Instantiates a new rectangle item
	 * @param x1		First point : Leftmost X position (in canvas coordinates)
	 * @param y1		First point : Topmost Y position (in canvas coordinates)
	 * @param x2		Second point: Rightmost X position (in canvas coordinates)
	 * @param y2		Second point: Bottommost Y position (in canvas coordinates)
	 * @return			Ellipse item pointer
	 */
	Ellipse*	addEllipse(int x1 = 0,
						   int y1 = 0,
						   int x2 = 50,
						   int y2 =  50);

	/**
	 * Instantiates a new text item, filled with provided text
	 * @param text	Input text
	 * @param x		X position (in canvas coordinates)
	 * @param y		Y position (in canvas coordinates)
	 * @return		Text item pointer
	 */
	Text*	addText(Glib::ustring text,
					int x = 0,
					int y = 0);

	/**
	 * Instantiates a new pixbuf item (using a default stock image)
	 * @param x		X position (in canvas coordinates)
	 * @param y		Y position (in canvas coordinates)
	 * @return		Pixbuf item pointer
	 */
	Pixbuf* addPixbuf(int x = 0,
					  int y = 0);

	/**
	 * Instantiates a new pixbuf item (using a provided pixbuf reference)
	 * @param pix	Instantiated pixbuf reference (Gdk::Pixbuf)
	 * @param x		X position (in canvas coordinates)
	 * @param y		Y position (in canvas coordinates)
	 * @return		Pixbuf item pointer
	 */
	Pixbuf*	addPixbuf(Glib::RefPtr<Gdk::Pixbuf> pix,
					  int x = 0,
					  int y = 0);

	/**
	 * Translates an item by X/Y offsets
	 * @param item	Target item
	 * @param dx	X translation offset
	 * @param dy	Y translation offset
	 */
	void	moveItem(Item* item, double dx, double dy);

protected:
	/**
	 * Initializes canvas coordinates system / items group
	 */
	void	initCanvas();

	/**
	 * Initializes GTK standard signals, for graphical controls
	 */
	void	initSignals();


	/**
	 * GDK event handlers Events Handlers
	 */

	/**
	 * Handler for mouse buttons events (press)
	 * @param e EventButton pointer
	 * @return True to stop event propagation, false otherwise.
	 */
	bool	on_button_press_event(GdkEventButton* e);
	
	/**
	 * Handler for mouse buttons events (release)
	 * @param e EventButton pointer
	 * @return True to stop event propagation, false otherwise.
	 */
	bool	on_button_release_event(GdkEventButton* e);

	/**
	 * Handler for mouse motion events
	 * @param e EventMotion pointer
	 * @return True to stop event propagation, false otherwise.
	 */
	bool	on_motion_notify_event(GdkEventMotion* e);


private:
	/**
	 * Opacity level, ranging from 0 to 255
	 * - (0 = transparent / 255 : opaque)
	 */
	int			opacity;

	Group*		group;
	Item*		selected;
	LayerState	state;
	double		cur_x, cur_y;
};

#endif

