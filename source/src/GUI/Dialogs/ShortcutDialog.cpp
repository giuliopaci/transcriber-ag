/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "ShortcutDialog.h"
#include "Explorer_dialog.h"
#include "Explorer_utils.h"
#include "Common/util/FileHelper.h"
#include "Common/icons/Icons.h"
#include "Common/util/Utils.h"

#define DIALOG_H 600
#define DIALOG_W 500

namespace tag {

ShortcutDialog::ShortcutDialog(int _number_tree, bool _lock_name)
{
	Icons::set_window_icon(this, ICO_ROOT_PERSONNAL, 15) ;

	table = NULL ;

	number_tree = _number_tree ;

	set_has_separator(false) ;
	set_title(_("Choose target shortcut")) ;
	//set_default_size(DIALOG_W, DIALOG_H) ;

	button_chooser = new Gtk::FileChooserButton(_("Configure shortcut"), Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER) ;

	Gtk::VBox* vbox = get_vbox() ;

	table = new Gtk::Table(2, 3, false) ;
	table->set_col_spacings(15) ;
	//vbox->pack_start((*file_chooser), true,true)  ;

	vbox->pack_start(BLANK1, false,false)  ;

	vbox->pack_start(table_align, false, false)  ;

	table_align.add(*table) ;
	table_align.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0) ;
		//line 1
		table->attach(path_align, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK , 0, 0) ;
		table->attach(path_entry, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK , 0, 0) ;
		table->attach(*button_chooser, 2, 3, 0, 1, Gtk::FILL, Gtk::SHRINK , 0, 0) ;
			path_align.add(path_label) ;
			path_align.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_LEFT, 0, 0) ;
		//line2
		table->attach(display_align, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK , 0, 0) ;
		table->attach(display_entry, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK , 0, 0) ;
			display_align.add(display_label) ;
			display_align.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_LEFT, 0, 0) ;

	validate = new Gtk::Button(Gtk::Stock::OK) ;
	cancel = new Gtk::Button(Gtk::Stock::CANCEL) ;

	vbox->pack_start(BLANK2, false,false)  ;

	vbox->pack_start(separ, false,false)  ;

	vbox->pack_start(buttons_align, false, false) ;
	buttons_align.set(Gtk::ALIGN_RIGHT, Gtk::ALIGN_RIGHT, 0, 0) ;
		buttons_align.add(buttons_box) ;
				buttons_box.pack_start(*validate, false, false) ;
				buttons_box.pack_start(*cancel, false, false) ;

	BLANK1.set_label(" ");
	BLANK2.set_label(" ");
	path_label.set_label(_("Target path ")) ;
	display_label.set_label(_("Shortcut name ")) ;
	path_label.set_name("bold_label") ;
	display_label.set_name("bold_label") ;

	vbox->show_all_children(true) ;

	button_chooser->signal_selection_changed().connect( sigc::mem_fun(*this, &ShortcutDialog::on_selection_changed) ) ;
	validate->signal_clicked().connect( sigc::mem_fun(*this, &ShortcutDialog::on_validate) ) ;
	cancel->signal_clicked().connect( sigc::mem_fun(*this, &ShortcutDialog::on_cancel) ) ;

	path_entry.signal_changed().connect( sigc::mem_fun(*this, &ShortcutDialog::on_path_entry_signal_changed) ) ;

	lock_name = _lock_name ;
	if (lock_name)
		display_entry.set_sensitive(false) ;
}

ShortcutDialog::~ShortcutDialog()
{
	if (validate)
		delete(validate) ;
	if (cancel)
		delete(cancel) ;
	if (table)
		delete(table) ;
	if (button_chooser)
		delete(button_chooser) ;
}

int ShortcutDialog::check_values()
{
	Glib::ustring path = path_entry.get_text() ;
	Glib::ustring display = display_entry.get_text() ;

	//TODO check display for correct
	if (path=="")
		return -2 ;
	else if (display=="")
		return -1 ;
	else if ( !Glib::file_test(path, Glib::FILE_TEST_EXISTS) )
		return -4 ;
	else if ( manager->exist_tree(path, number_tree) )
		return -3 ;
	//don't check name for default system name (ftp cache file, work directory)
	else if (is_valid_filename(display)<0 && !lock_name)
		return -5 ;
	else if (!FileHelper::is_readable(path)||!FileHelper::is_executable(path))
		return -6 ;
	else
		return 1 ;
}

void ShortcutDialog::on_selection_changed()
{
	path_entry.set_text(button_chooser->get_filename()) ;
}

void ShortcutDialog::on_validate()
{
	int ok = check_values() ;

	if (ok==1) {
		chosen_path = path_entry.get_text() ;
		chosen_display = display_entry.get_text() ;
		response(Gtk::RESPONSE_OK) ;
	}
	else if (ok<0) {
		Glib::ustring txt ;
		if (ok==-1)
			txt = _("Please enter a shortcut name ") ;
		else if (ok==-2)
			txt = _("Please choose a target") ;
		else if (ok==-3)
			txt = _("This target is already used by a shortcut") ;
		else if (ok==-4)
			txt = _("This target does not exist") ;
		else if (ok==-5)
			txt = _("This name is too long or contains invalid symbol, please choose another") ;
		else if (ok==-6)
			txt = _("Please change directory permissions (enable read and execute) or change directory") ;
		Explorer_dialog::msg_dialog_warning(txt, this, true) ;
	}
}

void ShortcutDialog::on_cancel()
{
	response(Gtk::RESPONSE_CANCEL) ;
}

void ShortcutDialog::set_default(Glib::ustring path, Glib::ustring display)
{
	path_entry.set_text(path) ;
	display_entry.set_text(display) ;
}


void ShortcutDialog::on_path_entry_signal_changed()
{
	Glib::ustring path = path_entry.get_text() ;
	Glib::ustring display ;
	if (!lock_name)
		display = Glib::path_get_basename(path) ;
	else
		display = display_entry.get_text() ;

	if (display!="")
		display_entry.set_text(display) ;
}

} //namespace
