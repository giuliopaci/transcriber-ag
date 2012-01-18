/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef VOLUMEFILTER_H
#define VOLUMEFILTER_H

#include "AbstractFilter.h"
#include <vector>

using namespace std;

/**
 * @class	VolumeFilter
 * @ingroup	MediaComponent
 *
 * Role : affects volume level for a single channel
 */
class VolumeFilter : public AbstractFilter
{
public:
	/**
	 * Constructor
	 */
	VolumeFilter();

	/**
	 * Initializes filter with a MediumInfo structure
	 * @param info	MediumInfo structure (pointer)
	 */
	void			init(MediumInfo* info);
	void			filter(MediumFrame*& frame);

	/**
	 * Sets volume factor
	 * @param factor Volume factor(float)
	 */
	void			setVolumeFactor(float factor);

	/**
	 * Gets volume factor
	 * @return Volume factor
	 */
	float			getVolumeFactor();


private:
	float			v_factor;
	MediumFrame*	frame;
};

#endif

