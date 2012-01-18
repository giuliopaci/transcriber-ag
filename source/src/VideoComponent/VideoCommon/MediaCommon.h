/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  TransAGVideo				*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/** @file */

#ifndef MEDIACOMMON_H_
#define MEDIACOMMON_H_

#include "MediaComponent/base/Guesser.h"
#include <gtkmm.h>

namespace tag {
/**
 * @class 		MediaCommon
 * @ingroup		MediaComponent
 *
 * Useful methods for media treatment.
 */
class MediaCommon
{
	public:
		MediaCommon(){} ;
		virtual ~MediaCommon(){} ;

		/**
		 * Extracts converted RGB frame according to given parameters
		 * @param time		Requested timestamp (frame precision)
		 * @param device	Current stream's IODevice
		 * @param width		Output pixbuf width
		 * @param height	Output pixbuf height
		 * @return New Gdk pixbuf
		 */
		static Glib::RefPtr<Gdk::Pixbuf> getPixbufFromFrame(float time, IODevice* device, int width, int height) ;

		/**
		 * Extracts converted RGB frame according to given medium frame
		 * @param medium	Medium frame
		 * @return New Gdk pixbuf
		 */
		static Glib::RefPtr<Gdk::Pixbuf> getPixbufFromFrame(MediumFrame* medium) ;

		/**
		 * Normalizes dimensions, according to stream aspect ratio
		 * @param[in,out] v_width	Width to be normalized
		 * @param[in,out] v_height	Height to be normalized
		 * @param device			Current stream's IODevice
		 */
		static void normalizeDimensions(int& v_width, int& v_height, IODevice* device) ;
};

}

#endif /* MEDIACOMMON_H_ */
