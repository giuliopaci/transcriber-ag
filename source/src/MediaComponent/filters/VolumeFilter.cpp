/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "VolumeFilter.h"

// --- VolumeFilter ---
VolumeFilter::VolumeFilter()
{}


// --- Init ---
void VolumeFilter::init(MediumInfo* s_info)
{
	v_factor = 1.0;
}


// --- Filter ---
void VolumeFilter::filter(MediumFrame *&f)
{
	if (f == NULL)
		return;

	for(int i=0; i<f->len / fChannels; i++)
		f->samples[fChannels*i + channelID] = (int16_t)(f->samples[fChannels*i + channelID] * v_factor);
}


// --- SetVolumeFactor ---
void VolumeFilter::setVolumeFactor(float v)
{
	v_factor = v;
}


// --- GetVolumeFactor ---
float VolumeFilter::getVolumeFactor()
{
	return v_factor;
}

