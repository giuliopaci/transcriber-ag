/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "SpellerFrame.h"
#include "Explorer_dialog.h"

namespace tag {

SpellerFrame::SpellerFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values)
: PreferencesFrame(config, parent, _("Speller Tool"), _dynamic_values, _static_values)
{
	vbox.pack_start(dico_enable_HBox, false, false, 7) ;
		dico_enable_HBox.pack_start(dico_enable_checkbox, false, false, 5) ;
	dico_enable_checkbox.set_label(_("Use speller")) ;

	//> dictionary path
	vbox.pack_start(dicoPath_frame, false, false, 6) ;
	dicoPath_frame.add(dicoPath_Vbox) ;
	dicoPath_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
		dicoPath_Vbox.pack_start(dicoPath_Hbox, false, false, 7) ;
			dicoPath_Hbox.pack_start(dicoPath_entry, false, false, 5) ;
			dicoPath_Hbox.pack_start(dicoPath_button, false, false, 5) ;

	dicoPath_button.set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER) ;
	dicoPath_button.set_title(_("Choose a directory")) ;
	dicoPath_frame.set_label(_("Directory of speller dictionaries")) ;

	dicoPath_button.signal_selection_changed().connect( sigc::mem_fun(*this, &SpellerFrame::on_dicoPath_changed)) ;
	dicoPath_entry.set_size_request(300, -1) ;
	dicoPath_entry.set_editable(false) ;

	//> dictionnary option
	vbox.pack_start(dico_allowUser_HBox, false, false, 7) ;
		dico_allowUser_HBox.pack_start(dico_allowUser_checkbox, false, false, 5) ;
	vbox.pack_start(dico_allowIgnoredWord_HBox, false, false, 7) ;
		dico_allowIgnoredWord_HBox.pack_start(dico_allowIgnoredWord_checkbox, false, false, 5) ;
	dico_allowUser_checkbox.set_label(_("Allow use of personal dictionary")) ;
	dico_allowIgnoredWord_checkbox.set_label(_("Allow to ignore word")) ;

	dico_allowUser_checkbox.signal_clicked().connect( sigc::bind<Glib::ustring,Glib::ustring>(sigc::mem_fun(*this, &SpellerFrame::on_dicoCheckBoxes_changed), "dico", "user-dico"));
	dico_allowIgnoredWord_checkbox.signal_clicked().connect( sigc::bind<Glib::ustring,Glib::ustring>(sigc::mem_fun(*this, &SpellerFrame::on_dicoCheckBoxes_changed), "dico", "ignore-word"));
	dico_enable_checkbox.signal_clicked().connect( sigc::bind<Glib::ustring,Glib::ustring>(sigc::mem_fun(*this, &SpellerFrame::on_dicoCheckBoxes_changed), "dico", "enable"));

	reload_data() ;
	modified(false) ;

	show_all_children() ;
}

SpellerFrame::~SpellerFrame()
{
}


void SpellerFrame::reload_data()
{
	lock_data = true ;

	//for speller state
	int activated = config->get_SPELLER_state() ;
	enableDicoGUI(activated) ;

	//for speller directory
	Glib::ustring rep = config->get_SPELLER_dicoPath() ;
	dicoPath_entry.set_text(rep) ;

	//for ignore option
	int allowIW = config->get_SPELLER_allowIgnoreWord() ;
	set_check_state(dico_allowIgnoredWord_checkbox, allowIW) ;

	//for user option
	int allowUD = config->get_SPELLER_allowUserDico() ;
	set_check_state(dico_allowUser_checkbox, allowUD) ;

	lock_data = false ;
}

/* SPELL */
//void SpellerFrame::on_dicoCheckBoxes_changed(Glib::ustring mode, Glib::ustring submode)
//{
//	if (lock_data)
//		return ;
//
//	modified(true) ;
//	//DICO
//	if (mode.compare("dico")==0)
//	{
//		if (submode.compare("enable")==0) {
//			bool active = dico_enable_checkbox.get_active() ;
//			enableDicoGUI(active) ;
//			config->set_SPELLER_state(active, false) ;
//			set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_SPELLER_PATH, "unused-string") ;
//		}
//		else if (submode.compare("user-dico")==0) {
//			config->set_SPELLER_allowUserDico(dico_allowUser_checkbox.get_active(), false) ;
//			set_formatted_boolean_dynamic_value(TAG_PREFERENCES_PARAM_SPELLER_ALLOW, dico_allowUser_checkbox.get_active()) ;
//		}
//		else if (submode.compare("ignore-word")==0) {
//			config->set_SPELLER_allowIgnoreWord(dico_allowIgnoredWord_checkbox.get_active(), false) ;
//			set_formatted_boolean_dynamic_value(TAG_PREFERENCES_PARAM_SPELLER_IGNORE, dico_allowIgnoredWord_checkbox.get_active()) ;
//		}
//	}
//}

void SpellerFrame::on_dicoPath_changed()
{
	if (lock_data)
		return ;

	modified(true);
	Glib::ustring value = dicoPath_button.get_filename() ;
	if ( !Glib::file_test(value, Glib::FILE_TEST_EXISTS) )
		Explorer_dialog::msg_dialog_error(_("This directory doesn't exist"), parent, true) ;
	else {
		dicoPath_entry.set_text(value) ;
		config->set_SPELLER_dicoPath(value, false) ;
		set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_SPELLER_PATH, value) ;
	}
}

void SpellerFrame::enableDicoGUI(bool enable)
{
	set_check_state(dico_enable_checkbox, enable) ;
	dicoPath_entry.set_sensitive(enable) ;
	dicoPath_button.set_sensitive(enable) ;
	dico_allowIgnoredWord_checkbox.set_sensitive(enable) ;
	dico_allowUser_checkbox.set_sensitive(enable) ;
}


} //namespace
