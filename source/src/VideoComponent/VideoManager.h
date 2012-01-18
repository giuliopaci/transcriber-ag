/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/* $Id */

/** @file */

#ifndef VIDEOMANAGER_H_
#define VIDEOMANAGER_H_

#include "VideoComponent/VideoPlayer/VideoWidget.h"
#include "VideoComponent/FrameBrowser/FrameBrowser.h"

namespace tag {
/**
 * @def 	VIDEO_MANAGER_PLAYERMODE
 * @brief	Mode for showing/hiding the video player via the VideoManager.
 */
#define VIDEO_MANAGER_PLAYERMODE		0


/**
 * @def 	VIDEO_MANAGER_BROWSERMODE
 * @brief	Mode for showing/hiding the video browser via the VideoManager.
 */
#define VIDEO_MANAGER_BROWSERMODE		1

/**
 * @def 	VIDEO_MANAGER_ALLMODE
  * @brief	Mode for showing/hiding both video player and
  * 		video browser via the VideoManager.
 */
#define VIDEO_MANAGER_ALLMODE			2



/**
 * @class 		VideoManager
 * @ingroup		VideoComponent
 *
 * Basic manager for controlling several video objects\n
 * In the current version uses player and browser.
 */
class VideoManager
{
	public:
		/**
		 * Constructor
		 * @param player	Pointer on a video player
		 * @param browser	pointer on a video browser
		 */
		VideoManager(VideoWidget* player, FrameBrowser* browser) ;

		/**
		 * Destructor
		 */
		virtual ~VideoManager() {} ;

		/**
		 * Hides the video component(s), depending on the mode.\n
		 * Mainly dedicated for focus hide or minimized hide.\n
		 * For hiding option, see hideVideoComponent(int) method
		 */
		void hideVideo() ;

		/**
		 * Hides the video component(s), depending on the mode.
		 */
		void showVideo() ;

		/**
		 * Show or hide a video component (or all)
		 * @param mode		Defines which component to hide.\n
		 * 					- VIDEO_MANAGER_PLAYERMODE: 	player\n
		 * 					- VIDEO_MANAGER_BROWSERMODE: 	browser\n
		 * 					- VIDEO_MANAGER_ALLMODE: 		all components\n
		 */
		void toggleDisplayVideoComponent(int mode) ;

		/**
		 * Gets the video action group
		 * @return		Reference on the video action group
		 */
		Glib::RefPtr<Gtk::ActionGroup> getActionGroup() ;

		/**
		 * Gets the UI corresponding to the video action group
		 * @return		UI string of the video action group
		 */
		Glib::ustring getUIInfo() ;

	private:
		/** Controlled object pointer **/
		VideoWidget* player ;
		FrameBrowser* browser ;

		std::map<int,int> status ; /*** status of each compoenent ***/
		Glib::RefPtr<Gtk::ActionGroup> action_group ;
};

}

#endif /* VIDEOMANAGER_H_ */
