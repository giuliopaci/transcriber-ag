/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "ToolBox.h"
#include "Common/icons/Icons.h"
#include "Common/globals.h"

namespace tag {

// --- ToolBox ---
ToolBox::ToolBox()
{
	prepareToolBar();
}

void ToolBox::prepareToolBar()
{
	//-- Navigation scale
	b_scale	= Gtk::manage(new Gtk::HScale);
	b_scale->set_update_policy(Gtk::UPDATE_DISCONTINUOUS);
	b_scale->set_digits(3);
	b_scale->set_draw_value(false) ;

	// -- Navigation buttons
	b_playVideo.set_icon(Gtk::Stock::MEDIA_PLAY, "", 17, _("Play")) ;
	b_NseekMinus.set_icon(Gtk::Stock::MEDIA_REWIND, "", 17, _("Rewind by N frames")) ;
	b_seekMinus.set_icon(Gtk::Stock::MEDIA_PREVIOUS, "", 17, _("Rewind")) ;
	b_seekPlus.set_icon(Gtk::Stock::MEDIA_NEXT, "", 17, _("Forward")) ;
	b_NseekPlus.set_icon(Gtk::Stock::MEDIA_FORWARD, "", 17, _("Forward by N frames")) ;
	b_keyframe.set_icon(ICO_MEDIA_CLIPBOARD, "", 22, _("Set in frame clipboard")) ;

	// -- Navigation Bar
	Gtk::VSeparator* sep = Gtk::manage(new Gtk::VSeparator()) ;
	Gtk::VSeparator* sep2 = Gtk::manage(new Gtk::VSeparator()) ;
	Gtk::HBox* classicBox = Gtk::manage(new Gtk::HBox()) ;
	Gtk::Label* blank = Gtk::manage(new Gtk::Label(" ")) ;
	Gtk::Label* blank2 = Gtk::manage(new Gtk::Label(" ")) ;
	pack_start(frame, false, false, 2);
		frame.add(align) ;
		frame.set_shadow_type(Gtk::SHADOW_ETCHED_OUT) ;
			align.set(Gtk::ALIGN_CENTER,Gtk::ALIGN_CENTER) ;
			align.add(hbox) ;
				hbox.pack_start(b_seekMinus, false, false) ;
				hbox.pack_start(b_playVideo, false, false) ;
				hbox.pack_start(b_seekPlus, false, false) ;
				hbox.pack_start(*sep, false, false) ;
				hbox.pack_start(b_NseekMinus, false, false) ;
				hbox.pack_start(nbframe_entry, false, false) ;
				hbox.pack_start(b_NseekPlus, false, false) ;
				hbox.pack_start(*sep2, false, false) ;
				hbox.pack_start(*blank, true, true) ;
				Gtk::Alignment * alignm = Gtk::manage(new Gtk::Alignment()) ;
				hbox.pack_start(*alignm, false, false, 2);
					alignm->add(scaleLabel) ;
					alignm->set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
					scaleLabel.set_name("video_player_scale") ;
					scaleLabel.show() ;
				hbox.pack_start(*blank2, true, true) ;
				nbframe_entry.set_width_chars(5) ;
				nbframe_entry.set_tooltip_text(_("Frame number navigation")) ;
 	pack_start(*b_scale, true, false, 2);
}


// --- SetScaleRange ---
void ToolBox::setScaleRange(double v_min, double v_max)
{
	b_scale->set_range(v_min, v_max);
}

void ToolBox::setPlay(bool play)
{
	if (play)
		b_playVideo.change_icon(Gtk::Stock::MEDIA_PLAY, "", 17, _("Play"));
	else
		b_playVideo.change_icon(Gtk::Stock::MEDIA_PAUSE, "", 17, _("Pause"));
}

} // namespace
