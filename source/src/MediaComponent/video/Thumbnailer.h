/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef THUMBNAILER_H
#define THUMBNAILER_H

#include <glibmm.h>
#include <string>
#include <valarray>

#include "MediaComponent/base/Guesser.h"
#include "MediaComponent/video/VideoResult.h"


/**
*  @class 		Thumbnailer
*  @ingroup		MediaComponent
*
*  Class used for extracting thumbnails from video.\n
*/
class Thumbnailer
{
public:
	/**
	 * Constructor
	 * @param inPath	File path
	 * @param inWidth	Width
	 * @param inHeight	Height
	 * @param inStep	Extraction step
	 * @return
	 */
	Thumbnailer(string inPath, int inWidth, int inHeight, int inStep);
	~Thumbnailer();

	/**
	 * Launches extraction and gets some information
	 * @return	VideoResult object
	 */
	VideoResult*	process();

	/**
	 *	Inits thumbnailer with existing thumb file
	 */
	void			initThumbs();

	/**
	 *	Joins the extraction thread
	 */
	void			joinThread();

	/**
	 * Signal emitted when a frame has been extracted
	 * <b>param MediumFrame*:</b> MediumFrame object
	 * <b>param double:</b> frame time
	 * <b>param int</b> return code
	 * 		(frame=NULL,  code=1 :  frame error)
	 * 		(frame=NULL,  code=2 :  critical error, process stopped)
	 */
	sigc::signal<void, MediumFrame*, double, int> signalFrameReady()	{ return s_FrameReady; }

protected:
	/**< get **/
	void		getThumbs();
	/**< set **/
	void		saveThumbs();

private:
	string		path;
	string		thumbsFile;
	int			width;
	int			height;
	int			step;
	int			rowStride;

	IODevice*	device;
	GThread*	processThread;
	VideoResult	vResult;

	sigc::signal<void, MediumFrame*, double, int> s_FrameReady;
};

#endif

