/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "VideoManager.h"
#include "Common/globals.h"

namespace tag {

VideoManager::VideoManager(VideoWidget* p_player, FrameBrowser* p_browser)
{
	player = p_player ;
	browser = p_browser ;

	if (player)
		player->signalCloseButton().connect(sigc::bind<int>(sigc::mem_fun(*this, &VideoManager::toggleDisplayVideoComponent), VIDEO_MANAGER_PLAYERMODE)) ;
	if (browser)
		browser->signalCloseButton().connect(sigc::bind<int>(sigc::mem_fun(*this, &VideoManager::toggleDisplayVideoComponent), VIDEO_MANAGER_BROWSERMODE)) ;

	status[VIDEO_MANAGER_PLAYERMODE] = 1 ;
	status[VIDEO_MANAGER_BROWSERMODE] = 1 ;
}

void VideoManager::hideVideo()
{
	if (player)
		player->saveGeoAndHide() ;
	if (browser)
		browser->saveGeoAndHide() ;
}

void VideoManager::showVideo()
{
	if (player && status[VIDEO_MANAGER_PLAYERMODE] == 1 )
		player->loadGeoAndDisplay() ;
	if (browser && status[VIDEO_MANAGER_BROWSERMODE] == 1 )
		browser->loadGeoAndDisplay() ;
}

void VideoManager::toggleDisplayVideoComponent(int mode)
{
	int state = 1 ;

	switch(mode)
	{
		case VIDEO_MANAGER_ALLMODE :
		{
			if (!player->is_visible() || !browser->is_visible())
			{
				browser->loadGeoAndDisplay() ;
				player->loadGeoAndDisplay() ;
				int state = 1 ;
			}
			else
			{
				player->saveGeoAndHide() ;
				browser->saveGeoAndHide() ;
				state = 0 ;
			}
			status[VIDEO_MANAGER_PLAYERMODE] = state ;
			status[VIDEO_MANAGER_BROWSERMODE] = state ;
			break ;
		}
		case VIDEO_MANAGER_BROWSERMODE :
		{
			if (!browser->is_visible()) {
				browser->loadGeoAndDisplay() ;
				state = 1 ;
			}
			else {
				browser->saveGeoAndHide() ;
				state = 0 ;
			}
			status[VIDEO_MANAGER_BROWSERMODE] = state ;
			break ;
		}
		case VIDEO_MANAGER_PLAYERMODE :
		{
			if (!player->is_visible()) {
				player->loadGeoAndDisplay() ;
				state = 1 ;
			}
			else {
				player->saveGeoAndHide() ;
				state = 0 ;
			}
			status[VIDEO_MANAGER_PLAYERMODE] = state ;
			break ;
		}
	} // end switch
}



Glib::ustring VideoManager::getUIInfo()
{
	Glib::ustring ui_info =
	"<ui>"
	"  <menubar name='MenuBar'>"
	"    <menu action='videoMenu'>"
	"      <menuitem action='hide_video_all'/>"
	"      <menuitem action='Hide_video_player'/>"
	"      <menuitem action='hide_video_browser'/>"
	"      <separator/>"
	"    </menu>"
	"  </menubar>"
	"</ui>" ;

	return ui_info ;
}

Glib::RefPtr<Gtk::ActionGroup> VideoManager::getActionGroup()
{
	action_group = Gtk::ActionGroup::create("video_signal");

	if (player)
		player->completeActionGroup(action_group) ;

	action_group->add( Gtk::Action::create("hide_video_all", _("Show/Hide video panels"), ""),
					Gtk::AccelKey("<control>F1"),
					sigc::bind<int>(sigc::mem_fun(*this, &VideoManager::toggleDisplayVideoComponent), VIDEO_MANAGER_ALLMODE) ) ;

	action_group->add( Gtk::Action::create("hide_video_browser", _("Show/Hide video browser"), ""),
					Gtk::AccelKey("<control>F3"),
					sigc::bind<int>(sigc::mem_fun(*this, &VideoManager::toggleDisplayVideoComponent), VIDEO_MANAGER_BROWSERMODE) ) ;

	action_group->add( Gtk::Action::create("Hide_video_player", _("Show/Hide video player"), ""),
					Gtk::AccelKey("<control>F2"),
					sigc::bind<int>(sigc::mem_fun(*this, &VideoManager::toggleDisplayVideoComponent), VIDEO_MANAGER_PLAYERMODE) ) ;

	return action_group ;
}

} // namespace
