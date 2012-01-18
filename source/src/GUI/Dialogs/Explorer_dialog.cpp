/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Explorer_dialog.h"
#include "Common/globals.h"
#include "Explorer_utils.h"
#include "Common/util/FileHelper.h"
#include "Common/Explorer_filter.h"
#include "Common/icons/Icons.h"

namespace tag {

Explorer_dialog::Explorer_dialog()
{
}

Explorer_dialog::~Explorer_dialog()
{
}

void Explorer_dialog::msg_dialog_info(Glib::ustring text, Gtk::Window* win, bool modal)
{
	Gtk::MessageDialog dialog(*win, text, modal) ;
	Icons::set_window_icon(&dialog, ICO_TRANSCRIBER, 17) ;
	//> launch dialog
	if (win)
		dialog.set_transient_for(*win) ;
	dialog.run() ;
}

void Explorer_dialog::msg_dialog_warning(Glib::ustring text, Gtk::Window* win, bool modal)
{
	Gtk::MessageDialog dialog(text, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, modal) ;
	Icons::set_window_icon(&dialog, ICO_TRANSCRIBER, 17) ;
	//> launch dialog
	if (win)
		dialog.set_transient_for(*win) ;
	dialog.run() ;
}

void Explorer_dialog::msg_dialog_error(Glib::ustring text, Gtk::Window* win, bool modal)
{
	Gtk::MessageDialog dialog(text, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, modal) ;
	Icons::set_window_icon(&dialog, ICO_TRANSCRIBER, 17) ;
	//> launch dialog
	if (win)
		dialog.set_transient_for(*win) ;
	dialog.run() ;
}


int Explorer_dialog::msg_dialog_question(Glib::ustring text, Gtk::Window* win, bool modal, Glib::ustring second_text)
{
	Gtk::Dialog dialog(_("Confirm"), *win, modal, false) ;
	Icons::set_window_icon(&dialog, ICO_TRANSCRIBER, 17) ;

	Gtk::HBox box ;
	Gtk::Label ltext, ltext_second ;
	Gtk::Image image ;
	Gtk::Label blank_top, blank_bottom, blank1 ;
	blank_top.set_label(" ");
	blank_bottom.set_label(" ");

	Gtk::IconSize *size = new Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) ;
	image.set(Gtk::Stock::DIALOG_QUESTION, *size) ;

	box.pack_start(image, false, false) ;
	box.pack_start(ltext, false, false) ;
	ltext.set_label("   " + text) ;

	dialog.get_vbox()->pack_start(blank_top) ;
	dialog.get_vbox()->pack_start(box) ;

	if (second_text!="") {
		blank1.set_label(" ") ;
		ltext_second.set_label(second_text) ;
		dialog.get_vbox()->pack_start(blank1, false, false) ;
		dialog.get_vbox()->pack_start(ltext_second, false, false) ;
	}

	dialog.get_vbox()->pack_start(blank_bottom) ;
	dialog.get_vbox()->show_all_children() ;
	ltext.set_name("dialog_label") ;

  	dialog.add_button(Gtk::Stock::YES, Gtk::RESPONSE_YES);
 	Gtk::Button* no = dialog.add_button(Gtk::Stock::NO, Gtk::RESPONSE_NO);
	dialog.set_focus(*no) ;
	if (win)
		dialog.set_transient_for(*win) ;

	int res = dialog.run() ;
	if (size)
		delete(size) ;

	return res  ;
}


int Explorer_dialog::msg_dialog_cancel(Glib::ustring text, Gtk::Window* win, bool modal, Glib::ustring second_text, Gtk::Entry* entry)
{
	Gtk::Dialog dialog("", *win, modal, false) ;
	Icons::set_window_icon(&dialog, ICO_TRANSCRIBER, 17) ;

	Gtk::HBox box ;
	Gtk::Label ltext, ltext_second ;
	Gtk::Image image ;
	Gtk::Label blank_top, blank_bottom, blank1 ;
	blank_top.set_label(" ");
	blank_bottom.set_label(" ");

	Gtk::IconSize *size = new Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) ;
	image.set(Gtk::Stock::DIALOG_QUESTION, *size) ;

	box.pack_start(image, false, false) ;
	box.pack_start(ltext, false, false) ;
	ltext.set_label("   " + text) ;

	dialog.get_vbox()->pack_start(blank_top) ;
	dialog.get_vbox()->pack_start(box) ;

	if (second_text!="") {
		blank1.set_label(" ") ;
		ltext_second.set_label(second_text) ;
		dialog.get_vbox()->pack_start(blank1, false, false) ;
		dialog.get_vbox()->pack_start(ltext_second, false, false) ;
	}

	if (entry) {
		blank1.set_label(" ") ;
		dialog.get_vbox()->pack_start(*entry, false, false) ;
	}

	dialog.get_vbox()->pack_start(blank_bottom) ;
	dialog.get_vbox()->show_all_children() ;
	ltext.set_name("dialog_label") ;

  	dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
 	Gtk::Button* no = dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.set_focus(*no) ;

	if (win)
		dialog.set_transient_for(*win) ;
	int res = dialog.run() ;
	if (size)
		delete(size) ;

	return res  ;
}



/*
 *  Used when renaming a file or a directory
 * Check the validity of the name, and if this name is already used
 * Return "" if cancel, name if validated
 */
Glib::ustring Explorer_dialog::msg_rename(Gtk::Window* win, Glib::ustring old_path)
{
	Glib::ustring old_name = Glib::path_get_basename(old_path) ;
	Glib::ustring ext = Explorer_filter::get_extension( Glib::path_get_basename(old_path) ) ;
	Glib::ustring dir = Glib::path_get_dirname(old_path) ;

	Glib::ustring new_path ;
	Glib::ustring res = "" ;
	Glib::ustring normal_text = _("Please enter a name") ;
	Glib::ustring existing_text = _("Name already in use\nPlease choose another name") ;
	Glib::ustring invalid_text = _("invalid name") ;

	Glib::ustring text = normal_text ;
	int response = Gtk::RESPONSE_YES ;
	bool ok = false ;
	int check = 0 ;

	//> add entry
	Gtk::Entry* entry = new Gtk::Entry() ;
	entry->set_text(old_name) ;

	while( response!=Gtk::RESPONSE_CANCEL && !ok)
	{
		//> launch dialog
		response = msg_dialog_cancel(text, win, true, "", entry) ;
		res = entry->get_text() ;

		if (res=="")
			text = _("Please enter an identifier") ;
		else {
			new_path = FileHelper::build_path(dir, res) ;
			//Glib::ustring new_name = Explorer_fileHelper::cut_extension(res) ;
			check = FileHelper::check_rename_filename(new_path) ;
			if (check==-5)
				text = existing_text ;
			else if (check==-1)
				text = _("Please enter an identifier") ;
			else if (check==-2)
				text = _("Please choose a shorter identifier") ;
			else if (check==-3)
				text = _("Identifier contains invalid symbol") ;
			else
				ok=true ;
		}
	}
	if (response==Gtk::RESPONSE_CANCEL)
		res="" ;

	if (entry)
		delete(entry) ;

	return res ;
}

/** Displayed when opening an audio file
 *  Ask user if he wants to create a new annotation file
 *
 *
 * 	Return:
 * 		Gtk::RESPONSE_YES
 * 		Gtk::RESPONSE_NO
 */
int Explorer_dialog::msg_dialog_open_audio(bool RTS_exists, Glib::ustring path, Gtk::Window* win)
{
	Glib::ustring text ;
	Glib::ustring under_text ;
	Glib::ustring name = Glib::path_get_basename(path) ;

	//> Anotation file exists, user can choose to use it or creating new one
	if(RTS_exists) 	{
		text=_("Create a new annotation file ?\n(choose NO to use existing one)") ;
		under_text=_("Existing file: ") + name  ;

		//> make YEs/NO dialog
		return msg_dialog_question(text,win,true,under_text) ;
	}
	//> Anotation file doesn't exist, we'll create a new one
	else {
		text = _("No anotation file found, a new file will be created") ;
		Gtk::MessageDialog dialog(*win, text) ;
		//> launch dialog
		int result = dialog.run() ;
		//> return res
		return result ;
	}
}

} //namespace
