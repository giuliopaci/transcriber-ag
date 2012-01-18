/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "GeneralFrame.h"
#include "Common/globals.h"
#include "Common/util/Utils.h"
#include "Explorer_dialog.h"
#include "Explorer_fileHelper.h"
#include "Explorer_dialog.h"
#include "UserDialog.h"

namespace tag {

GeneralFrame::GeneralFrame(Configuration* _config, Gtk::Window* _parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values)
: PreferencesFrame(_config, _parent, _("General"), _dynamic_values, _static_values)
{
	//> entry for username
	username_frame.set_label(_("User identifier")) ;
	vbox.pack_start(username_frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		username_frame.add(username_Vbox) ;
			username_Vbox.pack_start(username_Hbox, false, false, 5) ;
				username_Hbox.pack_start(username_entry, false, false, 5) ;
				username_Hbox.pack_start(username_change_button, false, false, 5) ;
				username_Hbox.pack_start(warning_acronym, false, false, 5) ;
	username_change_button.signal_clicked().connect( sigc::mem_fun(*this, &GeneralFrame::on_username_button)) ;
	username_change_button.set_label(_("Change")) ;
	username_entry.set_editable(false) ;
	username_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;

	//> frame for default audio
	audioRep_button.set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER) ;
	audioRep_button.set_title(_("Choose a directory")) ;
	audioRep_frame.set_label(_("Default directory for signal audio files")) ;
	audioRep_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
	vbox.pack_start(audioRep_frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		audioRep_frame.add(audioRep_Vbox) ;
			audioRep_Vbox.pack_start(audioRep_Hbox, false, false, 5) ;
				audioRep_Hbox.pack_start(audioRep_entry, false, false, 5) ;
				audioRep_Hbox.pack_start(audioRep_button, false, false, 5) ;
				audioRep_Hbox.pack_start(warning_audioRep, false, false, 5) ;
	audioRep_button.signal_selection_changed().connect( sigc::mem_fun(*this, &GeneralFrame::on_audioRep_changed)) ;
	audioRep_entry.set_size_request(400, -1) ;
	audioRep_entry.set_editable(false) ;

	//> frame for default browser
	//defaultBrowser_button.set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER) ;
	defaultBrowser_button.set_title(_("Choose a directory")) ;
	defaultBrowser_frame.set_label(_("Default browser")) ;
	defaultBrowser_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
	vbox.pack_start(defaultBrowser_frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		defaultBrowser_frame.add(defaultBrowser_Vbox) ;
			defaultBrowser_Vbox.pack_start(defaultBrowser_Hbox, false, false, 5) ;
				defaultBrowser_Hbox.pack_start(defaultBrowser_entry, false, false, 5) ;
				defaultBrowser_Hbox.pack_start(defaultBrowser_button, false, false, 5) ;
	defaultBrowser_button.signal_selection_changed().connect( sigc::mem_fun(*this, &GeneralFrame::on_defaultBrowser_changed)) ;
	defaultBrowser_entry.set_size_request(200, -1) ;
	defaultBrowser_entry.set_editable(false) ;

	//> check button for loading files
	vbox.pack_start(check_loadfiles, false, false, TAG_PREFERENCESFRAME_SPACE) ;
	check_loadfiles.set_label(_("Load files that were opened when TransAG shut down")) ;
	check_loadfiles.signal_clicked().connect( sigc::mem_fun(*this, &GeneralFrame::on_loadFiles_changed));

	//> check button for status bar
	vbox.pack_start(statusbarshow_check, false, false, TAG_PREFERENCESFRAME_SPACE) ;
	statusbarshow_check.set_label(_("Display status bar")) ;
	statusbarshow_check.signal_clicked().connect( sigc::mem_fun(*this, &GeneralFrame::on_statusbar_changed));

	//> check button for toolbar
	vbox.pack_start(toolbarshow_check, false, false, TAG_PREFERENCESFRAME_SPACE) ;
	toolbarshow_check.set_label(_("Display toolbar")) ;
	toolbarshow_check.signal_clicked().connect( sigc::mem_fun(*this, &GeneralFrame::on_toolbar_changed));

	//> combo for toolbar
	vbox.pack_start(toolbar_hbox, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		toolbar_hbox.pack_start(toolbar_label, false, false, 5) ;
		toolbar_hbox.pack_start(toolbar_combo, false, false, 5) ;
	toolbar_label.set_label(_("Choose toolbar style")) ;
	toolbar_combo.signal_changed().connect( sigc::mem_fun(*this, &GeneralFrame::on_toolbarCombo_changed));

	//> TIMER FRAME
	vbox.pack_start(timer_frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		timer_frame.add(timer_vbox) ;
			timer_vbox.pack_start(autosave_Hbox, false, false, 5) ;
				autosave_Hbox.pack_start(autosave_label, false, false, 5) ;
				autosave_Hbox.pack_start(autosave_spin, false, false, 5) ;
				//autosave_Hbox.pack_start(warning_autosave, false, false, 5) ;
//			timer_vbox.pack_start(activity_Hbox, false, false, 5) ;
//				activity_Hbox.pack_start(activity_label, false, false, 5) ;
//				activity_Hbox.pack_start(activity_spin, false, false, 5) ;
				//activity_Hbox.pack_start(warning_activity, false, false, 5) ;
//	activity_label.set_label(_("Period of inactivity (seconds)")) ;
	autosave_label.set_label(_("File autosave periodicity (seconds)")) ;

//	timer_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
//	timer_frame.set_label(_("Timers")) ;

//	activity_spin.set_increments(1,2) ;
//	activity_spin.set_numeric(true) ;
//	activity_spin.set_range(0,60*60*24) ;
//	activity_spin.set_update_policy(Gtk::UPDATE_IF_VALID) ;
//	activity_spin.signal_value_changed().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(this, &GeneralFrame::on_spins_changed), "activity")) ;
//	activity_spin.set_editable(false) ;
	autosave_spin.set_increments(1,2) ;
	autosave_spin.set_numeric(true) ;
	autosave_spin.set_range(0,60*60*24) ;
	autosave_spin.set_update_policy(Gtk::UPDATE_IF_VALID) ;
	autosave_spin.signal_value_changed().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(this, &GeneralFrame::on_spins_changed), "autosave")) ;
	autosave_spin.set_editable(false) ;

	warning_images.insert(warning_images.begin(), &warning_audioRep) ;
	warning_images.insert(warning_images.begin(), &warning_acronym) ;

	set_warnings_visible(false) ;

	Glib::ustring tip = _("Mouse button 1: smallest incrementation") ;
	tip.append("\n") ;
	tip.append(_("Mouse wheel click: larger incrementation")) ;
	tip.append("\n") ;
	tip.append(_("Mouse button 3: reach min/max value")) ;
	tooltip.set_tip(activity_spin, tip) ;
	tooltip.set_tip(autosave_spin, tip) ;

	reload_data() ;
	modified(false) ;

	show_all_children() ;
}

GeneralFrame::~GeneralFrame()
{
}

void GeneralFrame::reload_data()
{
	lock_data = true ;

	//for check button
	int load = config->get_GUI_loadOpenedFiles() ;
	set_check_state(check_loadfiles, load) ;

	//default browser
	Glib::ustring db = config->get_GUI_defaultBrowser() ;
	if (db.compare("")==0 || db.compare(" ")==0)
		db = _("system default") ;
	defaultBrowser_entry.set_text(db) ;

	//for audio rep
	Glib::ustring audiorep = config->get_AUDIO_defaultRep() ;
	audioRep_entry.set_text(audiorep) ;

	//for check button
	int search = config->get_GUI_statusbarShow() ;
	set_check_state(statusbarshow_check, search) ;

	//for check button
	search = config->get_GUI_toolbarShow() ;
	set_check_state(toolbarshow_check, search) ;

	//for timer spins
	int autosave = config->get_EDITOR_autosave() ;
	autosave_spin.set_value(autosave) ;
//	int activity = config->get_EDITOR_activity() ;
//	activity_spin.set_value(activity) ;

	//for mapping combo
	prepare_combo_toolbar() ;

	//for user name entry
	Glib::ustring name = config->get_USER_acronym() ;
	username_entry.set_text(name) ;

	lock_data = false ;
}



//**************************************************************************************
//***************************************************************************** CALLBACK
//**************************************************************************************

void GeneralFrame::on_combo_keyboard_change()
{
/*	modified(true);
	Glib::ustring value = keyboard_combo.get_active_text() ;
	Glib::ustring file_name = "" ;
	Glib::ustring file_path = "" ;
	if (value==AZERTY)
		file_name = param->getParameterValue("Data", "inputLanguage,inputAZERTY") ;
	else if (value==QWERTY)
		file_name = param->getParameterValue("Data", "inputLanguage,inputQWERTY") ;
	if (file_name!="") {
		Glib::ustring config = param->getParameterValue("General", "start,config") ;
		file_path = Explorer_fileHelper::build_path(config, file_name) ;
	}
	if ( file_path!="" && Glib::file_test(file_path, Glib::FILE_TEST_EXISTS) )
		param->setParameterValue("Data", "inputLanguage,inputFile", file_path, true) ;
	else
		Explorer_dialog::msg_dialog_error(_("mapping file not found"), parent, true) ;*/
}

void GeneralFrame::on_toolbarCombo_changed()
{
	if (lock_data)
		return ;

	Glib::ustring value = toolbar_combo.get_active_text() ;
	if (value.compare(TAG_PREFERENCES_GEN_TOOLICON)==0) {
		modified(true) ;
		config->set_TOOLBAR_displayLabel("false", false) ;
		set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_TOOLBAR, "false") ;
	}
	else if (value.compare(TAG_PREFERENCES_GEN_TOOLICONTEXT)==0) {
		modified(true) ;
		config->set_TOOLBAR_displayLabel("true", false) ;
		set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_TOOLBAR, "true") ;
	}
	else if (value.compare(TAG_PREFERENCES_GEN_TOOLICONTEXTHORIZONTAL)==0) {
		modified(true) ;
		config->set_TOOLBAR_displayLabel("text", false) ;
		set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_TOOLBAR, "text") ;
	}
}

void GeneralFrame::on_audioRep_changed()
{
	if (lock_data)
		return ;

	Glib::ustring value = audioRep_button.get_filename() ;
	if ( !Glib::file_test(value, Glib::FILE_TEST_EXISTS) )
		Explorer_dialog::msg_dialog_error(_("This directory doesn't exist"), parent, true) ;
	else {
		modified(true);
		audioRep_entry.set_text(value) ;
		config->set_AUDIO_defaultRep(value, false) ;
		set_warnings(&warning_audioRep, 1) ;
	}
}

void GeneralFrame::on_defaultBrowser_changed()
{
	if (lock_data)
		return ;

	Glib::ustring value = defaultBrowser_button.get_filename() ;
//TODO test if correct application for LINUX / WIN32
	/*if ( !Glib::file_test(value, Glib::FILE_TEST_EXISTS) )
		Explorer_dialog::msg_dialog_error(_("This directory doesn't exist"), parent, true) ;
	else {*/
		modified(true);
		defaultBrowser_entry.set_text(value) ;
		config->set_GUI_defaultBrowser(value, false) ;
	//}
}

void GeneralFrame::on_loadFiles_changed()
{
	if (lock_data)
		return ;

	modified(true);
	config->set_GUI_loadOpenedFiles(check_loadfiles.get_active(), false) ;
}

void GeneralFrame::on_statusbar_changed()
{
	if (lock_data)
		return ;

	modified(true);
	config->set_GUI_statusbarShow(statusbarshow_check.get_active(), false) ;
	set_formatted_boolean_dynamic_value(TAG_PREFERENCES_PARAM_GENERAL_STATUSBARSHOW, statusbarshow_check.get_active()) ;
}

void GeneralFrame::on_toolbar_changed()
{
	if (lock_data)
		return ;

	modified(true);
	config->set_GUI_toolbarShow(toolbarshow_check.get_active(), false) ;
	set_formatted_boolean_dynamic_value(TAG_PREFERENCES_PARAM_GENERAL_TOOLBARSHOW, toolbarshow_check.get_active()) ;
}

void GeneralFrame::on_entries_changed(Glib::ustring mode)
{
	if (lock_data)
		return ;

	//> USER NAME ENTRY
	if (mode.compare("username")==0)
	{
		Glib::ustring ide = username_entry.get_text() ;
		int res = is_valid_filename(ide) ;
		if (res<0) {
			Glib::ustring text ;
			if (res==-1)
				text = _("Please enter an identifier") ;
			else if (res==-2)
				text = _("Please choose a shorter identifier") ;
			else if (res==-3)
				text = _("Identifier contains invalid symbol") ;
			Explorer_dialog::msg_dialog_warning(text,parent,true) ;
		}
		else {
			modified(true) ;
			config->set_USER_acronym(ide, false) ;
		}
	}
}

void GeneralFrame::on_username_button()
{
	UserDialog user(config, false) ;
	user.signalIdentified().connect(sigc::mem_fun(this, &GeneralFrame::on_username_changed)) ;
	user.run() ;
}

void GeneralFrame::on_username_changed(bool changed)
{
	if (lock_data)
		return ;

	if (changed) {
		modified(true) ;
		Glib::ustring name = config->get_USER_acronym() ;
		username_entry.set_text(name) ;
		set_warnings(&warning_acronym, 1) ;
	}
}

void GeneralFrame::on_spins_changed(Glib::ustring mode)
{
	if (lock_data)
		return ;

	if (mode.compare("autosave")==0)
	{
		int value = autosave_spin.get_value_as_int() ;
		if (value>=0) {
			modified(true) ;
			config->set_EDITOR_autosave(autosave_spin.get_value(), false) ;
			set_formatted_integer_dynamic_value(TAG_PREFERENCES_PARAM_AUTOSAVE, value) ;
		}
	}
//	else if (mode.compare("activity")==0)
//	{
//		int value = activity_spin.get_value_as_int() ;
//		if (value>=0) {
//			modified(true) ;
//			config->set_EDITOR_activity(activity_spin.get_value(), false) ;
//			set_formatted_integer_dynamic_value(TAG_PREFERENCES_PARAM_INACTIVITY, value) ;
//		}
//	}

}


//**************************************************************************************
//*********************************************************************** INITIALISATION
//**************************************************************************************

void GeneralFrame::prepare_combo_toolbar()
{
	toolbar_combo.clear() ;

	toolbar_combo.append_text(TAG_PREFERENCES_GEN_TOOLICON) ;
	toolbar_combo.append_text(TAG_PREFERENCES_GEN_TOOLICONTEXT) ;
	toolbar_combo.append_text(TAG_PREFERENCES_GEN_TOOLICONTEXTHORIZONTAL) ;

	Glib::ustring current = config->get_TOOLBAR_displayLabel() ;
	if (current.compare("false")==0)
		toolbar_combo.set_active_text(TAG_PREFERENCES_GEN_TOOLICON) ;
	else if (current.compare("true")==0)
		toolbar_combo.set_active_text(TAG_PREFERENCES_GEN_TOOLICONTEXT) ;
	else if (current.compare("text")==0)
		toolbar_combo.set_active_text(TAG_PREFERENCES_GEN_TOOLICONTEXTHORIZONTAL) ;
}

} //namespace
