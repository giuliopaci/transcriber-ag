/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <gtkmm.h>
#include "Common/icons/IcoPackToolButton.h"

namespace tag {

/**
 * @class	ToolBox
 * @ingroup VideoComponent
 *
 * This class implements navigation scalebar & toolbar
 */
 
class ToolBox : public Gtk::VBox
{
public:
	/**
	 * Default constructor
	 */
	ToolBox();

	/**
 	 * Initializes new navigation toolbar (and scalebar)
	 */
		void	prepareToolBar();

	/**
	 * Updates navigation bar value range
	 * @param min_value	Minimum value in range
	 * @param max_value	Maximum value in range
	 */
	void	setScaleRange(double min_value, double max_value);

	/**
	 * Actualize Play / Pause button
	 * @param play		True for playing status, False for pause status
	 */
	void setPlay(bool play) ;

	// -----------------
	// --- Variables ---
	// -----------------
	IcoPackToolButton 	b_playVideo;	/**< Play/Stop toolbutton */
	IcoPackToolButton	b_seekMinus;	/**< Seek backward button */
	IcoPackToolButton	b_seekPlus;		/**< Seek forward button */
	IcoPackToolButton	b_keyframe;		/**< Set keyframe button */
	IcoPackToolButton	b_NseekMinus;	/**< Seek N backward button */
	IcoPackToolButton	b_NseekPlus;	/**< Seek N forward button */

	Gtk::HScale* 		b_scale;		/**< Navigation horizontal scale bar*/
	Gtk::Entry 			nbframe_entry;	/**< Frame step entry */

private:

	Gtk::HBox		hbox;
	Gtk::Frame			frame;
	Gtk::Alignment		align;
	Gtk::Label 			scaleLabel ;
};

} // namespace

#endif

