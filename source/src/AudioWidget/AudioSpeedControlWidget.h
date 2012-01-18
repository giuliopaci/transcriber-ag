/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_AUDIOSPEEDCONTROLWIDGET__
#define __HAVE_AUDIOSPEEDCONTROLWIDGET__

#include "Common/globals.h"

namespace tag {

/**
 * @class	AudioSpeedControlWidget
 * @ingroup	AudioWidget
 *
 * Scale widget allowing playback speed control
 */
class AudioSpeedControlWidget : public AudioSoundTouchControlWidget
{
	public:
	/**
	 * Default constructor
	 * @param p_vertical		Vertical orientation
	 * @param p_displayValues	Values display
	 * @param p_displayValue	Actual value display
	 * @param p_displayReset	Reset button display
	 */
	AudioSpeedControlWidget(bool p_vertical, bool p_displayValues, bool p_displayValue, bool p_displayReset)
		: AudioSoundTouchControlWidget(_("Speed"), 2.5, p_vertical, p_displayValues, p_displayValue, p_displayReset) {}
};

} // namespace

#endif // __HAVE_AUDIOSPEEDCONTROLWIDGET__

