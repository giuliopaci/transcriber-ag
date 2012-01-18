/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef GUESSER_H
#define GUESSER_H

#include "MediaComponent/io/FFMpegHandler.h"
#include "MediaComponent/io/SndFileHandler.h"
#include "MediaComponent/io/RTSPHandler.h"
#include "MediaComponent/io/SilentHandler.h"
#include "MediaComponent/base/Types.h"

/**
 * @class	Guesser
 * @ingroup	MediaComponent
 * 
 * Class providing an unique static method 'open', compatible with all supported formats.
 */

//#define GUESSER_TRACE 1

class Guesser
{
public:
	/**
	 * Static method : opens a medium
	 * @param mediumPath	Medium URI
	 * @param mediumCtx		Medium Context (Default : AudioCtx)
	 * @return An instantiated IODevice (on success), NULL otherwise
	 */
	static	IODevice*	open(const char* mediumPath, MediumCtx mediumCtx = AudioCtx);

	/**
	 * Static method : opens an silent medium
	 * @param nbTracks		Number of channels needed
	 * @param length		Length of signal needed
	 * @return				An instantiated IODevice
	 */
	static	IODevice*	open(int nbTracks, double length);
};

#endif

