/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "PreferencesFrame.h"
#include "Explorer_utils.h"
#include "Common/util/Utils.h"

namespace tag {

PreferencesFrame::PreferencesFrame(Configuration* _config, Gtk::Window* _parent, Glib::ustring title, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values)
{
	apply.set_sensitive(false) ;

	parent=_parent ;
	config=_config ;

	dynamic_values = _dynamic_values ;
	static_values = _static_values ;

	//> mother vbox
	add(vbox_display) ;
	vbox_display.pack_start(hbox_display, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		hbox_display.pack_start(align, false, false, 5) ;
			align.add(label) ;
	vbox_display.pack_start(scrolledw, true, true, 0) ;
		scrolledw.add(scrolled_vbox) ;
			scrolled_vbox.pack_start(scrolled_hbox, true, true, 5) ;
				scrolled_hbox.pack_start(vbox, true, true, 9) ;

	align.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	scrolledw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC) ;
	scrolledw.set_shadow_type(Gtk::SHADOW_NONE) ;

	Glib::ustring _label = " " + title ;
	label.set_label(_label) ;
	label.set_name("preferences_label") ;

	show_all_children() ;

	lock_data = false ;
	modified(false);

	set_shadow_type(Gtk::SHADOW_NONE) ;
}

PreferencesFrame::~PreferencesFrame()
{
}

void PreferencesFrame::on_apply_clicked()
{
	apply.set_sensitive(false);
	cancel.set_sensitive(false);
	config->save_user_config() ;
}

void PreferencesFrame::on_cancel_clicked()
{
}

void PreferencesFrame::modified(bool ismodified)
{
	m_signalIsModified.emit(ismodified) ;
	//apply.set_sensitive(ismodified);
	//cancel.set_sensitive(ismodified);
}

void PreferencesFrame::set_check_state(Gtk::CheckButton& check, int configuration_mode)
{
	switch (configuration_mode)
	{
		case 1:  check.set_active(true) ; break ;
		case 0:  check.set_active(false) ; break ;
		default: check.set_sensitive(false) ; break ;
	}

}

void PreferencesFrame::set_warnings_visible(bool visible)
{
	std::vector<IcoPackImage*>::iterator it ;
	for (it=warning_images.begin(); it!=warning_images.end(); it++) {
		if (visible)
			(*it)->show() ;
		else
			(*it)->hide() ;
	}
}

void PreferencesFrame::set_warnings(IcoPackImage* image, int level)
{
	if (level==1) {
		image->set_image(ICO_PREFERENCES_RELOAD1, 18) ;
		image->set_tip(_("Opened files need to be re-opened\nfor enabling modifications")) ;
		image->show() ;
		static_values->insert(static_values->begin(), image) ;
	}
	else if (level==2) {
		image->set_image(ICO_PREFERENCES_RELOAD2, 18) ;
		image->set_tip(_("Application need to be restarted\nfor enabling modifications")) ;
		image->show() ;
	}
}

void PreferencesFrame::set_formatted_boolean_dynamic_value(int param, bool value)
{
	if (value)
		(*dynamic_values)[param] = "true" ;
	else
		(*dynamic_values)[param] = "false" ;
}

void PreferencesFrame::set_formatted_integer_dynamic_value(int param, int value)
{
	Glib::ustring v = number_to_string(value) ;
	(*dynamic_values)[param] = v ;
}

void PreferencesFrame::set_formatted_string_dynamic_value(int param, Glib::ustring value)
{
	(*dynamic_values)[param] = value ;
}

bool PreferencesFrame::get_formatted_bool_dynamic_value(Glib::ustring value)
{
	if (value.compare("true")==0)
		return true ;
	else
		return false ;
}

int PreferencesFrame::get_formatted_int_dynamic_value(Glib::ustring value)
{
	int i = string_to_number<int>(value) ;
	return i ;
}


} //namespace
