/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  TransAGVideo				*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include <iostream>
#include "FrameFiller.h"
#include "Common/util/Utils.h"
#include "MediaComponent/video/VideoResult.h"

#define WAITIME			0.0003

namespace tag {

FrameFiller::FrameFiller(FrameBrowser* frameBroser, const Glib::ustring& p_videoPath, int width, int height, int step)
{
	frameBrowser = frameBroser ;
	videoPath = p_videoPath ;

	thumbNailer = new Thumbnailer(p_videoPath, width, height, step) ;
	thumbNailer->signalFrameReady().connect(sigc::mem_fun(this, &FrameFiller::onFrameReceived)) ;
}

FrameFiller::~FrameFiller()
{
	if (thumbNailer)
		delete (thumbNailer) ;
}

bool FrameFiller::launch()
{
	if (!frameBrowser || !thumbNailer)
		return false;

	VideoResult* result = thumbNailer->process() ;

	//> Error at processing ? stop.
	if (result && result->processState==1)
		return false ;
	//> Let our thread running :)
	else {
		Glib::Thread::create( sigc::mem_fun(*this,&FrameFiller::treatElement), false ) ;
		return true ;
	}
}

/**
 *	frame=NULL,  code=1 :  error for frame
 *	frame=NULL,  code=2 :  critical error, process stopped
 *	frame!=NULL  :		   ok
 */
void FrameFiller::onFrameReceived(MediumFrame* frame, double time, int code)
{
	while (!mutex.trylock()) ;

	FrameSlot* slot = NULL ;

	// good frame
	if (frame ) {
		slot = new FrameSlot(frame, time, false) ;
		delete(frame) ;
	}
	// bad frame
	else if ((!frame && code==1))
		slot = new FrameSlot(frame, time, false) ;
	// bad process, stop
	else if ((!frame && code==2))
		slot = new FrameSlot(frame, time, true) ;

	queue.push(slot) ;

	mutex.unlock() ;
}

//------------------------------------------------------------------------------
//								THREAD BUSINESS
//------------------------------------------------------------------------------

void FrameFiller::treatElement()
{
	bool stop = false ;
	while(!stop)
	{
		if (mutex.trylock())
		{
			// have smtg to eat ?
			if (queue.size()>0)
			{
				// catch data
				FrameSlot* slot = queue.front() ;
			 	queue.pop() ;
				// let cooker work
				mutex.unlock() ;
				// treat data if available
				if (slot && !slot->critical_error) {
					frameBrowser->addFrame(slot->pixbuf, slot->time) ;
					delete(slot) ;
				}
				// critical error: stop all
				else if (slot && slot->critical_error) {
					stop = true ;
					delete(slot) ;
					frameBrowser->ready(true) ;
					delete(thumbNailer) ;
					thumbNailer = NULL ;
				}
				// treat end case
				else {
					stop = true ;
					frameBrowser->ready(false) ;
					delete(thumbNailer) ;
					thumbNailer = NULL ;
				}
			}
			// nothing in fridge, let cooker work
			else {
				mutex.unlock() ;
				mySleep(WAITIME) ;
			}
		}
	}
}

} //namespace
