/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  TransAGVideo				*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/** @file */

#ifndef FRAMEFILLER_H_
#define FRAMEFILLER_H_

#include "FrameBrowser.h"
#include "VideoComponent/VideoCommon/MediaCommon.h"
#include "MediaComponent/base/Guesser.h"
#include "MediaComponent/video/Thumbnailer.h"
#include <iostream>

namespace tag {
/**
 * @class 		FrameFiller
 * @ingroup		MediaComponent
 *
 * Class filling the frames browser at thread signal reception.
 */
class FrameFiller
{
	public:
		/**
		 * Constructor
		 * @param frameBrowser		Pointer on FrameBrowser
		 * @param p_videoPath		Video file path
		 * @param width				Video width
		 * @param height			Video height
		 * @param step				Extraction step
		 */
		FrameFiller(FrameBrowser* frameBrowser, const Glib::ustring& p_videoPath, int width, int height, int step);
		virtual ~FrameFiller();

		/**
		 * Launches the frame extraction
		 * @return	True for success, false otherwise
		 */
		bool launch() ;

	private:

		/*** reference on parent (don't manage !) ***/
		FrameBrowser* frameBrowser ;

		/*** loader tool ***/
		Thumbnailer* thumbNailer ;
		Glib::ustring videoPath ;

		class FrameSlot
		{
			public :
				FrameSlot(MediumFrame* frame, double p_time, bool p_critical_error)
				{
					pixbuf = MediaCommon::getPixbufFromFrame(frame) ;
					if (frame)
						time = frame->ts ;
					else
						time = p_time ;
					critical_error = p_critical_error ;
				}
				~FrameSlot() {} ;
				Glib::RefPtr<Gdk::Pixbuf> pixbuf ;
				float time ;
				bool critical_error ;
		};

		std::queue<FrameSlot*> queue ;
		Glib::Mutex mutex ;
		void treatElement() ;
		void onFrameReceived(MediumFrame* frame, double time, int code) ;
};

} // namespace

#endif /* FRAMEFILLER_H_ */
