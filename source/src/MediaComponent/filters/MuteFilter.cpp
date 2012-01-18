/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "MuteFilter.h"

// --- MuteFilter ---
MuteFilter::MuteFilter()
{
	isMuted = false;
}


// --- Filter ---
void MuteFilter::filter(MediumFrame *&f)
{
	if (isMuted && f != NULL)
		for(int i=0; i<f->len / fChannels; i++)
			f->samples[i*fChannels + channelID] = 0;
}

