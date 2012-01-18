/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

/**
 * @defgroup	AudioWidget
 */

#ifndef __HAVE_AUDIOWIDGET__
#define __HAVE_AUDIOWIDGET__

#include <gtkmm.h>
#include <gtkmm/icontheme.h>
#include <libintl.h>
#include <iostream>
#include "Common/portabilite.h"
#include "Common/Dialogs.h"
#include "Common/PangoMarkup.h"

#include "Common/util/Log.h"
#include "Common/util/Utils.h"
#include "Common/ColorsCfg.h"

#define MAX_CHANNELS 2	/**< Max authorized channels */
#define SMALL_TOGGLE_BUTTON_SIZE 14

namespace tag {

/**
 * @class	AudioWidget
 * @ingroup	AudioWidget
 *
 * Global class gathering tool functions for audio widgets
 */

class AudioWidget
{
public:
	
	/**
	 * @var AUDIO_ZOOM_MAX
	 * 
	 * Maximum zoom allowed in graphical tracks
	 */
	static float AUDIO_ZOOM_MAX;

	/**
	 * @var VERTICAL_SCALE_SIZE
	 * 
	 * Default vertical size for scales
	 */
	static guint VERTICAL_SCALE_SIZE;

	/**
	 * @var HORIZONTAL_SCALE_SIZE
	 * 
	 * Default horizontal size for scales
	 */
	static guint HORIZONTAL_SCALE_SIZE;
};

} // namespace

#include "AudioWaveformWidget.h"
#include "AudioVZoomControlWidget.h"
#include "AudioVolumeControlWidget.h"
#include "AudioSoundTouchControlWidget.h"
#include "AudioPitchControlWidget.h"
#include "AudioSpeedControlWidget.h"
#include "AudioTempoControlWidget.h"
#include "AudioPlayControlWidget.h"
#include "AudioZoomControlWidget.h"

#include "TrackWidget.h"
#include "SegmentTooltip.h"
#include "SegmentWidget.h"
#include "SegmentTrackWidget.h"
#include "MarkWidget.h"
#include "MarkTrackWidget.h"
#include "ScaleWidget.h"
#include "ScaleTrackWidget.h"
#include "AudioTrackWidget.h"

#endif // __HAVE_AUDIOWIDGET__

