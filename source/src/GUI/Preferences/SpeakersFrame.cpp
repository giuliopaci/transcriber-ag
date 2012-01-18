/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "SpeakersFrame.h"

namespace tag {

SpeakersFrame::SpeakersFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values)
: PreferencesFrame(config, parent, _("Speakers"), _dynamic_values, _static_values)
{
	//> frame for default audio
	GlobalSpeakersDico_button.set_action(Gtk::FILE_CHOOSER_ACTION_OPEN) ;
	GlobalSpeakersDico_button.set_title(_("Choose a directory")) ;
	GlobalSpeakersDico_frame.set_label(_("Path of global speaker dictionary file")) ;
	GlobalSpeakersDico_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
	vbox.pack_start(GlobalSpeakersDico_frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		GlobalSpeakersDico_frame.add(GlobalSpeakersDico_Vbox) ;
		GlobalSpeakersDico_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
			GlobalSpeakersDico_Vbox.pack_start(GlobalSpeakersDico_Hbox, false, false, 5) ;
				GlobalSpeakersDico_Hbox.pack_start(GlobalSpeakersDico_entry, false, false, 5) ;
				GlobalSpeakersDico_Hbox.pack_start(GlobalSpeakersDico_button, false, false, 5) ;
				GlobalSpeakersDico_Hbox.pack_start(GlobalSpeakersDico_image, false, false, 5) ;
			GlobalSpeakersDico_button.signal_selection_changed().connect( sigc::mem_fun(*this, &SpeakersFrame::on_GlobalSpeakersDico_changed)) ;
	GlobalSpeakersDico_entry.set_size_request(400, -1) ;
	GlobalSpeakersDico_entry.set_editable(false) ;

	GlobalSpeakersDico_image.set_image(ICO_SPEAKER_DICO_GLOBAL, 17) ;

	reload_data() ;
	modified(false) ;
}

SpeakersFrame::~SpeakersFrame()
{
}


void SpeakersFrame::on_GlobalSpeakersDico_changed()
{
	if (lock_data)
		return ;

	modified(true);
	Glib::ustring value = GlobalSpeakersDico_button.get_filename() ;
	if ( !Glib::file_test(value, Glib::FILE_TEST_EXISTS) )
		Explorer_dialog::msg_dialog_error(_("This directory doesn't exist"), parent, true) ;
	else {
		GlobalSpeakersDico_entry.set_text(value) ;
		config->set_global_dictionary(value, false) ;
	}
}

void SpeakersFrame::reload_data()
{
	lock_data = true ;

	//for audio rep
	Glib::ustring GlobalSpeakersDico = config->get_global_dictionary() ;
	GlobalSpeakersDico_entry.set_text(GlobalSpeakersDico) ;

	lock_data = false ;
}

} //namespace
