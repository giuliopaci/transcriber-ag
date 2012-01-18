/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "UserDialog.h"
#include "Common/globals.h"
#include "Common/icons/Icons.h"
#include "Common/VersionInfo.h"
#include "Explorer_fileHelper.h"
#include "Explorer_utils.h"
#include "Common/util/Utils.h"
#include "Explorer_dialog.h"
#include <gtkmm/icontheme.h>
#include <gtkmm.h>

namespace tag {

UserDialog::UserDialog(Configuration* conf, bool _save)
{
	save = _save ;

	table = NULL ;
	set_modal(true) ;
	set_resizable(false) ;
	set_title(TRANSAG_DISPLAY_NAME) ;
	Icons::set_window_icon(this, ICO_TRANSCRIBER,15) ;

	config = conf ;

	Gtk::VBox* box = get_vbox() ;
	table = new Gtk::Table(1, 2, false) ;
	box->add(blank1) ;
	box->pack_start(general_align, true, false) ;
			general_align.add(general_hbox) ;
			general_hbox.pack_start(general_label, false, false) ;
			general_hbox.pack_start(general_image, false, false);
	box->add(blank2) ;
	box->add(sep) ;
	box->add(blank3) ;
	box->pack_start(table_align, true, false) ;
			table_align.add(*table) ;
				//line 1
				table->attach(login_align, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK , 0, 0) ;
				table->attach(login_entry, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK , 0, 0) ;
						login_align.add(login_label) ;
	set_widget() ;

	box->show_all_children() ;
	Gtk::Button* apply = add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY) ;

	try {
		Glib::RefPtr<Gtk::IconTheme>theme = Gtk::IconTheme::get_default() ;
		Glib::RefPtr<Gdk::Pixbuf>pixbufPlay = theme->load_icon(ICO_ID, 60,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
		general_image.set(pixbufPlay);
	}
	catch (Gtk::IconThemeError e) {
		Log::err() << "IcoPackButton::set_icon:> " <<  e.what()  << std::endl ;
	}

	login_entry.grab_focus() ;
}

UserDialog::~UserDialog()
{
	if (table)
		delete (table) ;
}

void UserDialog::set_widget()
{
	general_align.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0) ;
	general_label.set_label(_("Please choose an identifier")) ;
	general_label.set_name("bold_label") ;

	general_align.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0) ;
	general_align.set_padding(0,0,0,15) ;

	login_label.set_label(_("Identifier:")) ;
	login_label.set_name("bold_label") ;

	table->set_col_spacings(15) ;
	table->set_row_spacings(5) ;

	login_align.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0, 0) ;

	table->show_all_children(true) ;
}

Glib::ustring UserDialog::get_identifier()
{
	return login_entry.get_text() ;
}


void UserDialog::on_response(int response)
{
	if (response==Gtk::RESPONSE_APPLY) {
		Glib::ustring ide = get_identifier() ;
		int res = is_valid_filename(ide) ;
		if (res<0) {
			Glib::ustring text ;
			if (res==-1)
				text = _("Please enter an identifier") ;
			else if (res==-2)
				text = _("Please choose a shorter identifier") ;
			else if (res==-3)
				text = _("Identifier contains invalid symbol") ;
			Explorer_dialog::msg_dialog_warning(text,this,true) ;
		}
		else {
			config->set_USER_acronym(ide, save) ;
			hide() ;
			m_signalIdentified.emit(true) ;
		}
	}
	else if (response==-4) 	{
			m_signalIdentified.emit(false) ;
	}
}

bool UserDialog::on_key_press_event(GdkEventKey* event)
{
	bool res = Gtk::Window::on_key_press_event(event);

	if (event->keyval==GDK_Return)
		on_response(Gtk::RESPONSE_APPLY) ;

	return res ;
}

} //namespace
