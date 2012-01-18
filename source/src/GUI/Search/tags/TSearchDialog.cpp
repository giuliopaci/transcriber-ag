/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "TSearchDialog.h"
#include "Explorer_dialog.h"
#include "Explorer_utils.h"
#include "Common/globals.h"
#include "Common/FileInfo.h"
#include "Common/icons/Icons.h"
#include <iostream>
#include <gtk/gtk.h>

namespace tag {

//***************************************************************************************
//************************************************************************ CONSTRUCTOR
//***************************************************************************************

TSearchDialog::TSearchDialog(Configuration* configuration)
{
	active = false ;
	config = configuration ;
	prepare_gui() ;
}

TSearchDialog::~TSearchDialog()
{
}


//***************************************************************************************
//************************************************************************ INTERNAL
//***************************************************************************************

void TSearchDialog::set_ico_searching()
{
	bool ok = true ;

	if (config!=NULL) {
		Glib::ustring conf = config->get_CONFIG_path() ;
		Glib::ustring iconsdir = FileInfo(conf).join("icons");
		Glib::ustring icons_gui = FileInfo(iconsdir).join("GUI");
		Glib::ustring progress = FileInfo(icons_gui).join(ICO_SEARCH_IN_PROGRESS);
		search_in_progress.set_image_path(progress, 30) ;
	}

	search_static.set_image(ICO_SEARCH_IN_PROGRESS_STATIC, 70) ;
	search_static.show() ;
}

void TSearchDialog::set_widgets_label()
{
	close_button.set_icon("", _("_Close"), 17, "") ;

	Glib::ustring tmp  = _("Next") ;
	Glib::ustring tmp2 = " >>" ;
	find_fw.set_icon("", tmp+tmp2, 17, "") ;
	tmp = "<< " ;
	tmp2 = _("Previous") ;
	find_bk.set_icon("", tmp+tmp2, 17, "") ;

	rb_section.set_label(_("Sections with given topic")) ;
	rb_turn.set_label(_("Turn of given speaker")) ;
	rb_event.set_label(_("Event with given text")) ;
	rb_entity.set_label(_("Entity with given text")) ;
}

void TSearchDialog::prepare_gui()
{
	set_modal(false) ;
	set_has_separator(false) ;
	set_border_width(5) ;

	try {
		Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
	    Glib::RefPtr<Gdk::Pixbuf> pixbufPlay = theme->load_icon(ICO_SEARCH_DIALOG, 17,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
	    set_icon(pixbufPlay);
	}
	catch (Gtk::IconThemeError e) {
		Log::err() << "MyWidget::MyWidget:> " <<  e.what()  << std::endl ;
	}

	set_title(TRANSAG_DISPLAY_NAME);

	Gtk::VBox *vbox = get_vbox() ;

	Gtk::HSeparator* sep = Gtk::manage(new Gtk::HSeparator());
	Gtk::HSeparator* sep2 = Gtk::manage(new Gtk::HSeparator());
	vbox->pack_start(hbox_gen, true, true) ;
	vbox->pack_start(align_header, false, false, 2) ;
	vbox->pack_start(*sep2, true, true, 2) ;
	vbox->pack_start(frame_type, true, true, 2) ;
	vbox->pack_start(frame_value, true, true, 2) ;
	vbox->pack_start(*sep, true, true, 2) ;
	vbox->pack_start(align_button, true, true, 2) ;
	vbox->pack_start(align_end, true, true, 2) ;

	//> header alignment
	align_header.add(hbox_header) ;

	//> Searching icon
	set_ico_searching() ;
	search_in_progress_frame.add(search_in_progress_hbox) ;
	search_in_progress_hbox.pack_start(search_in_progress, false, false) ;
	search_in_progress_hbox.pack_start(search_static, false, false) ;
	search_in_progress_frame.set_shadow_type(Gtk::SHADOW_OUT) ;
	hbox_header.pack_start(search_in_progress_frame, false, false) ;

	//> TYPE RADIO BUTTONS
	frame_type.add(align_type) ;
	frame_type.set_label(_("Type of research")) ;
		align_type.add(vbox_type) ;
			vbox_type.pack_start(rb_section, false, false, 1) ;
			vbox_type.pack_start(rb_turn, false, false, 1) ;

	//> VALUE COMBO
	frame_value.add(align_value) ;
	frame_value.set_label(_("Object of research")) ;
		align_value.add(hbox_value) ;
			hbox_value.pack_start(label_value, false, false, 3) ;
			hbox_value.pack_start(combo_value, false, false, 3) ;
			hbox_value.pack_start(button_value, false, false, 3) ;

	//> BUTTONS
	align_button.add(bbox_find) ;
		bbox_find.pack_start(find_bk, false, false, 3) ;
		bbox_find.pack_start(find_fw, false, false, 3) ;
		bbox_find.pack_start(close_button, false, false, 3) ;

	//> set_align
	align_header.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_button.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_value.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_type.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_gen.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;

	set_widgets_label() ;
	show_all_children(true) ;
	search_in_progress.hide() ;
}


//**************************************************************************************
//***************************************************************************** INTERNAL
//**************************************************************************************

void TSearchDialog::myHide()
{
	saveGeoAndHide() ;
}

void TSearchDialog::display_info(Glib::ustring text)
{
	Explorer_dialog::msg_dialog_info(text,this, true) ;
}

int TSearchDialog::display_question(Glib::ustring text)
{
	return Explorer_dialog::msg_dialog_question(text,this, true, "") ;
}

void TSearchDialog::mySet_focus()
{
	combo_value.grab_focus() ;
}

bool TSearchDialog::on_key_press_event(GdkEventKey* event)
{
	return Gtk::Dialog::on_key_press_event(event); ;
}

//**************************************************************************************
//										GEOMETRY INTERFACE
//**************************************************************************************

void TSearchDialog::getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	get_position(pos_x, pos_y) ;
	panel = -1 ;
	get_size(size_xx, size_yy) ;
}

void TSearchDialog::setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel)
{
	if (pos_x>0 && pos_y>0)
		move(pos_x, pos_y) ;
	if (size_xx>0 && size_yy>0)
		resize(size_xx, size_yy) ;
}

Glib::ustring TSearchDialog::getWindowTagType()
{
	return SETTINGS_TSEARCH_NAME ;
}

void TSearchDialog::saveGeoAndHide()
{
	if (is_visible())
		savePos() ;
	hide() ;
}

int TSearchDialog::loadGeoAndDisplay(bool rundlg)
{
	loadPos() ;
	show() ;
	return 1 ;
}

void TSearchDialog::getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) {}

} //namespace
