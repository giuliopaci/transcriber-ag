/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_AUDIOPLAYCONTROLWIDGET__
#define __HAVE_AUDIOPLAYCONTROLWIDGET__

namespace tag {

/**
 * @class	AudioPlayControlWidget
 * @ingroup	AudioWidget
 * 
 * This class implements play/pause button
 */
class AudioPlayControlWidget : public Gtk::Button
{
public:
	/**
	 * Default constructor
	 */
	AudioPlayControlWidget();

	/**
	 * Default destructor
	 */
	~AudioPlayControlWidget();

	/** Playback status
	 * @return True if playing, false otherwise.
	 */
	bool getPlay()				{ return a_play; }

	/**
	 * Sets playback status (and updates GUI)
	 * @param p_play	Playback state
	 */
	void setPlay(bool p_play)	{ a_play = p_play; updateGUI(); }


protected:
	bool a_play;			/**< Playback state */
	Gtk::Image a_imgPlay;	/**< Gtk image - Play */
	Gtk::Image a_imgPause;	/**< Gtk image - Pause */

	/**
	 * Updates the whole GUI
	 */
	void updateGUI();
};

} // namespace

#endif // __HAVE_AUDIOPLAYCONTROLWIDGET__

