/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file TooltipTT.h
 */

#ifndef TOOLTIPTT_H_
#define TOOLTIPTT_H_

#include <gtkmm.h>

namespace tag {

/**
 * @class 		TooltipTT
 * @ingroup		Common
 *
 * Basic implementation of a tooltip for displaying widgets.
 *
 */

class TooltipTT : public Gtk::Window
{
	public:
		/**
		 * Constructor
		 */
		TooltipTT();
		virtual ~TooltipTT();

		/**
		 * Set the delay appearance of the tooltip. Once the delay is over
		 * the tooltip is dispayed.
		 * @param i		Time value for the tooltip appearance
		 */
		void set_timing(double i) { timing = i ; }

		/**
		 * Accessor to the tooltip appearance delay.
		 * @return		The timing value
		 */
		double get_timing() { return timing ; }

		/**
		 * Connect the timeout signal to the abstract TooltipTT::display() method.
		 * @param event		GdkEventMotion event got from the on_motion_event
		 * @param win		Pointer on the parent window on which the tooltip is displayed
		 * @attention 		Must be called in the on_motion_event method
		 */
		void launch(GdkEventMotion event, Gtk::Widget* win) ;

		/**
		 * Hide tootlip and reset the timer.
		 * @note  	Should be called inside on_leave_notify_event, on_focus_out_event, etc...,
		 * 			i.e all methods that require the tooltip to be hidden
		 */
		void stop() ;

		/**
		 * Set the Gtk RcStyle of tooltip
		 * @param name : name of rc style
		 */
		void setRCname(Glib::ustring name) { set_name(name); }

		/**
		 * Accessor to the internal tooltip frame inside which the data will be diplayed
		 * @return		Pointer on the tooltip data frame
		 */
		Gtk::Frame* getGeneralFrame() { return &frame_in ; }

		/**
		 * Clean the widget structure data (i.e remove widgets inside the generalFrame)
		 * @see 	getGeneralFrame()
		 */
		void clean() ;

	protected:
		/* ADD IN THAT FRAME ALL ELEMENTS YOU WANT */

		/**
		 * @var frame_in
		 * Frame that embeds the widget to be popped
		 */
		Gtk::Frame frame_in;

		Gtk::Frame frame_out; 		/**< Presentation frame */
		Gtk::VBox vbox;				/**< Presentation box */

		double timing ; 			/**< timing before poppin' */

		sigc::connection timeout ;  /**< timeout connection */

		/**
		 * Compute the position where the tooltip will be displayed
		 * @param parent				Window parent
		 * @param event					Motion event
		 * @param[out] res_tooltipX		Computed x tooltip position
		 * @param[out] res_tooltipY		Computed y tooltip position
		 */
		void compute_position(Gtk::Widget* parent, GdkEventMotion event, int& res_tooltipX, int& res_tooltipY) ;

		/**
		 * Adjust correct position for the tooltip to be fully visible
		 * @param[out] x		Adjusted tooltip x position
		 * @param[out] y		Adjusted tooltip y position
		 * @param win			Window parent
		 * @return				True if successful, False otherwise
		 */
		bool adust_in_screen(gint& x, gint& y, GdkWindow* win) ;

		/**
		 * Display the tooltip
		 * @param event		GdkEventMotion event
		 * @param win		Parent window
		 * @return			True (for fitting timeout connection slot)
		 * @remarks			This method should use the compute_position()
		 * 					and adust_in_screen(gint&,gint&,GdkWindow*) methods
		 * 					for providing a correct display
		 */
		virtual bool display(GdkEventMotion event, Gtk::Widget* win) = 0 ;

		/**
		 * The tooltip can requires additional information to be set before
		 * calling the launch() method.\n
		 * The reset_data() method is called each time the stop method is used
		 * for resetting these additional data.
		 */
		virtual void reset_data() = 0 ;
};

} //namespace

#endif /*TOOLTIPTT_H_*/
