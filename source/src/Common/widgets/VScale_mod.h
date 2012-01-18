/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file	VScale_mod.h
 */

#ifndef VSCALE_MOD_H_
#define VSCALE_MOD_H_

#include <gtkmm.h>

namespace tag {

/**
* @class 		VScale_mod
* @ingroup		Common
*
* Basic enhancement of Gtk::VScale.\n
* Provides the possibility to reset the scale on a mouse click
*/
class VScale_mod : public Gtk::VScale
{
	public:
		/**
		 * Constructor
		 */
		VScale_mod();
		/**
		 * Constructor
		 * @param min		Minimum value of the scale range
		 * @param max		Maximum value of the scale range
		 * @param step		Sclaing step
		 */
		VScale_mod(double min, double max, double step) ;
		virtual ~VScale_mod();

		/**
		 *  Signal emitted when CTRL + mouse button 1 is pressed
		 */
		sigc::signal<void> signalReset() { return a_signalReset ; }

	private:
		bool on_button_press_event(GdkEventButton* button) ;
		sigc::signal<void> a_signalReset ;
} ;

} //namespace

#endif /*VSCALE_MOD_H_*/
