/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#ifndef VIDEOBUFFER_H
#define VIDEOBUFFER_H

#include <gtkmm.h>
#include <glibmm.h>
#include <string.h>

/** @file */

using namespace std;

namespace tag {

/**
 * @class	VideoBuffer
 * @ingroup	VideoComponent
 *
 * Video player - Display component (GTK Image)
 *
 */

class VideoBuffer : public Gtk::Image
{
public:
	/**
	 * Default constructor
	 */
	VideoBuffer();

	/**
	 * Initializes internal signals
	 */
	void initSignals();
	
private:
};

} // namespace

#endif

