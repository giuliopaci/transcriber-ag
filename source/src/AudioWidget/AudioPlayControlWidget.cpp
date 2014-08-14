/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class AudioPlayControlWidget
 *
 * AudioPlayControlWidget...
 */

#include <iostream>
#include "AudioWidget.h"

namespace tag {

void AudioPlayControlWidget::updateGUI()
{
	if (a_play)
		set_image(a_imgPause);
	else
		set_image(a_imgPlay);

	show();
}

AudioPlayControlWidget::~AudioPlayControlWidget() 
{
}

AudioPlayControlWidget::AudioPlayControlWidget() : Gtk::Button(),
	a_imgPlay(Gtk::Stock::MEDIA_PLAY, Gtk::ICON_SIZE_MENU),
	a_imgPause(Gtk::Stock::MEDIA_PAUSE, Gtk::ICON_SIZE_MENU)
{
	a_play = false;

	updateGUI();
}

} // namespace
