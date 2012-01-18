/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef MUTEFILTER_H
#define MUTEFILTER_H

#include "AbstractFilter.h"

/**
 * @class	MuteFilter
 * @ingroup	MediaComponent
 *
 * Role : mutes/activates a single audio channel
 */
class MuteFilter : public AbstractFilter
{
public:
	/**
	 * Constructor
	 */
	MuteFilter();

	void	filter(MediumFrame*& frame);

	/**
	 * Mutes associated channel
	 @param mt Mute flag (boolean)
	 */
	void	mute(bool mt)			{ isMuted = mt; }

private:
	bool	isMuted;
};

#endif

