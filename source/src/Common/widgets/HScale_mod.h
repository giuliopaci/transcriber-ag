/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file	HScale_mod.h
 */

#ifndef HSCALE_MOD_H_
#define HSCALE_MOD_H_

#include <gtkmm.h>

namespace tag {

/**
* @class 		HScale_mod
* @ingroup		Common
*
* Basic enhancement of Gtk::HScale.\n
* Provides the possibility to reset the scale on a mouse click
*/
class HScale_mod : public Gtk::HScale
{
	public:
		/**
		 * Constructor
		 */
		HScale_mod();
		/**
		 * Constructor
		 * @param min		Minimum value of the scale range
		 * @param max		Maximum value of the scale range
		 * @param step		Sclaing step
		 */
		HScale_mod(double min, double max, double step) ;
		virtual ~HScale_mod();

		/**
		 *  Signal emitted when CTRL + mouse button 1 is pressed
		 */
		sigc::signal<void> signalReset() { return a_signalReset ; }

	private:
		bool on_button_press_event(GdkEventButton* button) ;
		sigc::signal<void> a_signalReset ;
} ;

} //namespace

#endif /*HScale_MOD_H_*/
