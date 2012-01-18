/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "SearchReplaceDialog.h"
#include "Explorer_dialog.h"
#include "Explorer_utils.h"
#include "Common/globals.h"
#include "Common/icons/Icons.h"
#include <iostream>
#include <gtk/gtk.h>

namespace tag {

//***************************************************************************************
//************************************************************************ CONSTRUCTOR
//***************************************************************************************

SearchReplaceDialog::SearchReplaceDialog(Configuration* configuration, int searchMode, bool cas, bool wholeWord)
{
	search_mode = searchMode;
	case_sensitive_mode = cas;
	whole_word_mode = wholeWord;

	table = NULL ;
	table_button = NULL ;
	table_gen = NULL ;
	config = configuration ;

	active = false ;
	prepare_gui() ;
}

SearchReplaceDialog::~SearchReplaceDialog()
{
	if (table)
		delete(table) ;
	if (table_button)
		delete(table_button) ;
	if (table_gen)
		delete(table_gen) ;
}


//***************************************************************************************
//************************************************************************ INTERNAL
//***************************************************************************************

void SearchReplaceDialog::set_widgets_label()
{
	tooltip.set_tip(change_mode, _("Change to toolbar mode")) ;

	close_button.set_icon("", _("_Close"), 17, "") ;
	change_mode.set_icon(ICO_SEARCH_TOOLBAR, _("_Toolbar mode"), 20, "") ;

	label_replace.set_label(_("Replace with")) ;
	label_search.set_label(_("Find")) ;

	label_inputLanguage.set_label(_("Input language: ")) ;

	frame_options.set_label(_("Options")) ;
	frame_options.set_label_align(Gtk::ALIGN_CENTER) ;

	frame_scope.set_label(_("Scope")) ;
	frame_scope.set_label_align(Gtk::ALIGN_CENTER) ;

	Glib::ustring tmp  = _("_Next") ;
	Glib::ustring tmp2 = " >>" ;
	find_fw.set_icon("", tmp+tmp2, 17, "") ;
	tmp = "<< " ;
	tmp2 = _("_Previous") ;
	find_bk.set_icon("", tmp+tmp2, 17, "") ;
	find_replace_fw.set_label(_("Replace / Find >>")) ;
	find_replace_bk.set_label(_("<< Replace / Find")) ;
	replace.set_label(_("_Replace")) ;
	replace_all.set_label(_("Replace _all")) ;

	rb_all.set_label(_("All lines")) ;
	rb_sel.set_label(_("Selected lines")) ;
	cb_whole.set_label(_("Whole word")) ;
	cb_case.set_label(_("Case sensitive")) ;
}

void SearchReplaceDialog::prepare_gui()
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

	//> general table
	table_gen = new Gtk::Table(7,1,false) ;
	table_gen->set_row_spacings(5) ;

	vbox->pack_start(align_gen, true, false) ;
	align_gen.add(*table_gen) ;

	//> header alignment
	align_header.add(hbox_header) ;

	//> Searching icon
	if (config!=NULL) {
		Glib::ustring conf = config->get_CONFIG_path() ;
		Glib::ustring iconsdir = FileInfo(conf).join("icons");
		Glib::ustring icons_gui = FileInfo(iconsdir).join("GUI");
		Glib::ustring progress = FileInfo(icons_gui).join(ICO_SEARCH_IN_PROGRESS);
		search_in_progress.set_image_path(progress, 30) ;
	}
	search_static.set_image(ICO_SEARCH_IN_PROGRESS_STATIC, 70) ;

	hbox_header.pack_start(search_in_progress_frame, false, false, 15) ;
	search_in_progress_frame.add(search_in_progress_hbox) ;
	search_in_progress_hbox.pack_start(search_in_progress, false, false) ;
	search_in_progress_hbox.pack_start(search_static, false, false) ;
	search_in_progress_frame.set_shadow_type(Gtk::SHADOW_OUT) ;

	//> Input Language
	Gtk::Alignment* align_tmp = Gtk::manage(new Gtk::Alignment()) ;
	align_tmp->add(hbox_input_language) ;
	align_tmp->set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
	align_tmp->set_padding(26, 11, 0, 0);
	hbox_input_language.pack_start(label_inputLanguage, false, false) ;
	hbox_input_language.pack_start(combo_language, false, false) ;
	hbox_header.pack_start(*align_tmp, false, false, 15) ;

	//UNIQUE COLUMN
	table_gen->attach(align_header, 0, 1, 0, 1, Gtk::EXPAND, Gtk::EXPAND , 0, 0) ;
	table_gen->attach(sep2, 					0, 1, 1, 2, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;
	table_gen->attach(align_combo, 			0, 1, 2, 3, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;
	table_gen->attach(align_frame, 			0, 1, 3, 4, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;
	table_gen->attach(align_button, 		0, 1, 4, 5, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;
	table_gen->attach(sep, 					0, 1, 5, 6, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;
	table_gen->attach(align_end, 			0, 1, 6, 7, Gtk::EXPAND, Gtk::EXPAND , 0, 0) ;

	//> table with combo
	table = new Gtk::Table(2,2,false) ;
	table->set_col_spacings(10) ;
	table->set_row_spacings(5) ;
	align_combo.add(*table) ;
	// first column
	table->attach(label_search, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;
	table->attach(label_replace, 0, 1, 1, 2, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;
	// second column
	table->attach(box_search, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;
	table->attach(box_replace, 1, 2, 1, 2, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;

	box_search.pack_start(combo_search, false, false) ;
	box_search.pack_start( *(combo_search.get_arrow() ), false, false) ;

	box_replace.pack_start(combo_replace, false, false) ;
	box_replace.pack_start( *(combo_replace.get_arrow() ), false, false) ;

	//> options in frame
	align_frame.add(hbox_options) ;
	hbox_options.pack_start(frame_options, false, false) ;
	hbox_options.pack_start(frame_scope, false, false) ;

	frame_scope.add(bbox_scope) ;
	bbox_scope.pack_start(rb_all, false, false) ;
	bbox_scope.pack_start(rb_sel, false, false) ;

	frame_options.add(bbox_option) ;
	bbox_option.pack_start(cb_whole, false, false) ;
	bbox_option.pack_start(cb_case, false, false) ;
	bbox_option.pack_start(combo_searchMode, false, false) ;

	//> buttons
	table_button = new Gtk::Table(2,2,true) ;
	table_button->set_col_spacings(10) ;
	table_button->set_row_spacings(5) ;

	align_button.add(*table_button) ;
	table_button->attach(find_bk, 0, 1, 0, 1, Gtk::FILL, Gtk::EXPAND , 0, 0) ;
	table_button->attach(replace, 0, 1, 1, 2, Gtk::FILL, Gtk::EXPAND , 0, 0) ;
	table_button->attach(find_fw, 1, 2, 0, 1, Gtk::FILL, Gtk::EXPAND , 0, 0) ;
	table_button->attach(replace_all, 1, 2, 1, 2, Gtk::FILL, Gtk::EXPAND , 0, 0) ;

	align_end.add(hbox_ends) ;
	hbox_ends.pack_start(change_mode, false, false) ;
	hbox_ends.pack_start(close_button, false, false) ;


	//> set_align
	align_header.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_button.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_combo.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_frame.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_gen.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_end.set(Gtk::ALIGN_RIGHT, Gtk::ALIGN_RIGHT, 0.0, 0.0) ;

	set_widgets_label() ;
	show_all_children() ;
	search_in_progress.hide() ;
}


//**************************************************************************************
//***************************************************************************** INTERNAL
//**************************************************************************************

void SearchReplaceDialog::myHide()
{
	saveGeoAndHide() ;
}

void SearchReplaceDialog::display_info(Glib::ustring text)
{
	Explorer_dialog::msg_dialog_info(text,this, true) ;
}

int SearchReplaceDialog::display_question(Glib::ustring text)
{
	return Explorer_dialog::msg_dialog_question(text,this, true, "") ;
}

void SearchReplaceDialog::on_change_mode()
{
	close() ;
	m_signalSearchModeChange.emit(0) ;
}

void SearchReplaceDialog::mySet_focus()
{
	combo_search.grab_focus() ;
}

bool SearchReplaceDialog::on_key_press_event(GdkEventKey* event)
{
	if (event->keyval==GDK_Return && combo_search.get_text()!="") {
		on_search_fw() ;
		return true ;
	}
	else if ( (event->is_modifier & GDK_CONTROL_MASK) && (event->keyval==GDK_f || event->keyval==GDK_F) ) {
		check_initial_selection() ;
		return true ;
	}
	else
		return Gtk::Dialog::on_key_press_event(event); ;
}


//**************************************************************************************
//										GEOMETRY INTERFACE
//**************************************************************************************

void SearchReplaceDialog::getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	get_position(pos_x, pos_y) ;
	panel = -1 ;
	get_size(size_xx, size_yy) ;
}

void SearchReplaceDialog::setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel)
{
	if (pos_x>0 && pos_y>0)
		move(pos_x, pos_y) ;
	if (size_xx>0 && size_yy>0)
		resize(size_xx, size_yy) ;
}

Glib::ustring SearchReplaceDialog::getWindowTagType()
{
	return SETTINGS_SEARCH_NAME ;
}

int SearchReplaceDialog::loadGeoAndDisplay(bool rundlg)
{
	loadPos() ;
	show() ;
	return 1 ;
}

void SearchReplaceDialog::saveGeoAndHide()
{
	if (is_visible())
		savePos() ;
	hide() ;
}

void SearchReplaceDialog::getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) {}

} //namespace
