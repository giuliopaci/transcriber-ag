/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef ABSTRACTFILTER_H
#define ABSTRACTFILTER_H

#include "base/Types.h"
#include "io/IODevice.h"

/**
 * @class	AbstractFilter
 * @ingroup	MediaComponent
 *
 * Abstract Base Class for audio/video filters
 */
class AbstractFilter
{
public:
	/**
	 * Virtual Destructor(must be implemented in subclasses)
	 */
	virtual ~AbstractFilter() {};

	/**
	 * Filters input frame
	 * @param[in,out] frame	MediumFrame pointer to be filtered
	 */
	virtual	void	filter(MediumFrame*& frame) = 0;

	/**
	 * Defines medium path
	 * @param path Medium path
	 */
	void			setMediumPath(char *path)	{ medium_path	= path;	}

	/**
	 * Enables / disables filter
	 * @param in_enabled	Control flag
	 */
	void			setEnabled(bool in_enabled)	{ enabled		= in_enabled; }

	/**
	 * Defines channels number
	 * @param channels	Channels number
	 */
	void			setChannels(int channels)	{ fChannels		= channels; }

	/**
	 * Defines channelID
	 * @param id Channel ID
	 */
	void			setChannelID(int id)		{ channelID		= id; }


protected:
	/**
	 * @var medium_path 
	 * Medium path
	 */
	char*	medium_path;

	/**
	 * @var enabled
	 * Activation status
	 */
	bool	enabled;

	/**
	 * @var fChannels
	 * Global channels
	 */
	int		fChannels;

	/**
	 * @var channelID
	 * Dedicated Channel Identifier
	 */
	int		channelID;
};

#endif

