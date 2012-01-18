/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* FERRY Guillaume - LECUYER Paule - MONTILLA Jmarc	*/
/* 	         																	*/
/********************************************************************************/
#include "VideoFrame.h"

namespace tag {

VideoFrame::VideoFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values)
: PreferencesFrame(config, parent, _("Video panel"), _dynamic_values, _static_values)
{
	vbox.pack_start(fbrowser_Frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		fbrowser_Frame.add(fbrowser_vbox) ;

		//> size signal
		fbrowser_vbox.pack_start(resolution_Hbox, false, false, TAG_PREFERENCESFRAME_SPACE) ;
			resolution_Hbox.pack_start(resolution_label, false, false, 5) ;
			resolution_Hbox.pack_start(resolution_spin, false, false, 5) ;
			resolution_Hbox.pack_start(warning_resolution, false, false, 5) ;

	fbrowser_Frame.set_label(_("Browser options")) ;
	fbrowser_Frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;

	resolution_label.set_label(_("Extract a frame each N seconds")) ;
	resolution_spin.set_increments(1, 5) ;
	resolution_spin.set_numeric(true) ;
	resolution_spin.set_range(1, 3600) ;
	resolution_spin.set_update_policy(Gtk::UPDATE_IF_VALID) ;
	resolution_spin.signal_value_changed().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(this, &VideoFrame::on_spins_changed), "fb_resolution")) ;
	resolution_spin.set_editable(false) ;

	reload_data() ;
	modified(false) ;
}

VideoFrame::~VideoFrame()
{
}

void VideoFrame::on_spins_changed(Glib::ustring mode)
{
	if (lock_data)
		return ;

	if (mode.compare("fb_resolution")==0)
	{
		int value = resolution_spin.get_value() ;
		if (value>=0) {
			modified(true) ;
			config->set_VIDEO_frameBrowserResolution(value, false) ;
			set_formatted_integer_dynamic_value(TAG_PREFERENCES_PARAM_FBROWSER_RESOLUTION, value) ;
		}
	}
}

void VideoFrame::on_checkboxes_changed(Glib::ustring mode)
{
	if (lock_data)
		return ;
}

void VideoFrame::reload_data()
{
	lock_data = true ;

	// resolution
	int size = config->get_VIDEO_frameBrowserResolution() ;
	resolution_spin.set_value(size) ;

	lock_data = false ;
}

} // namespace
