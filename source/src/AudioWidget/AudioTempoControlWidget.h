/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_AUDIOTEMPOCONTROLWIDGET__
#define __HAVE_AUDIOTEMPOCONTROLWIDGET__

namespace tag {

/**
 * @class	AudioTempoControlWidget
 * @ingroup	AudioWidget
 *
 * Scale widget allowing playback tempo control
 */
class AudioTempoControlWidget : public AudioSoundTouchControlWidget
{
public:
	/**
	 * Default constructor
	 * @param p_vertical		Vertical orientation
	 * @param p_displayValues	Values display
	 * @param p_displayValue	Actual value display
	 * @param p_displayReset	Reset button display
	 */
	AudioTempoControlWidget(bool p_vertical, bool p_displayValues, bool p_displayValue, bool p_displayReset)
		: AudioSoundTouchControlWidget(_("Tempo"), 4, p_vertical, p_displayValues, p_displayValue, p_displayReset) {}
};

} // namespace

#endif // __HAVE_AUDIOTEMPOCONTROLWIDGET__

