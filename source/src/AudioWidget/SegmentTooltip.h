/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef SEGMENTTOOLTIP_H_
#define SEGMENTTOOLTIP_H_

#include "Common/widgets/TooltipTT.h"

namespace tag {

/**
 * @class	SegmentTooltip
 * @ingroup	AudioWidget
 *
 * This class implements segment tooltips drawing routines
 */

class SegmentTooltip : public TooltipTT
{
public:
	/**
	 * Default constructor
	 */
	SegmentTooltip();
	virtual ~SegmentTooltip();

	static int TEXT_MAX_SIZE;	/**< Maximum characters in tooltip */

	/**
	 * TEXT_MAX_SIZE accessor
	 * @param i	New maximum text size
	 */
	static void setTextMaxSize(int i) { TEXT_MAX_SIZE = i; }

	/**
	 * Sets new tooltip text with parameters
	 * @param p_text	Text content	
	 * @param start		Start offset
	 * @param end		End offset
	 */
	void set_data(Glib::ustring p_text, float start, float end) ;


protected:
	/**
	 * Prepares widgets, and triggers the tooltip timer
	 */
	void			prepare_tooltip();

	/**
	 * Resets tooltip internal data
	 */
	void			reset_data();

	/**
	 * Displays preprocessed tooltip to target window
	 * @param event	Gdk motion event
	 * @param win	Target window
	 * @return True on success, false otherwise
	 */
	bool			display(GdkEventMotion event, Gtk::Widget* win);

	// -- Variables --
	Gtk::Label		a_label;	/**< Tooltip label */
	Glib::ustring	data;		/**< Displayed text */
	float			start;		/**< Start offset */
	float			end;		/**< End offset */
};

} // namespace

#endif /*SEGMENTTOOLTIP_H_*/

