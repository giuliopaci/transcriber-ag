/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "MiniSearch.h"
#include "Common/globals.h"
#include "Common/icons/Icons.h"
#include "Explorer_dialog.h"

namespace tag {

MiniSearch::MiniSearch(int searchMode, bool cas, bool wholeWord)
{
	search_mode = searchMode;
	case_sensitive_mode = cas;
	whole_word_mode = wholeWord;

	parent = NULL ;
	active = false ;
	prepare_gui() ;
}

MiniSearch::~MiniSearch()
{

}

void MiniSearch::set_widgets_label()
{
	tooltip.set_tip(change_mode, _("Change to search / replace mode")) ;
	tooltip.set_tip(close_button, _("Close search toolbar")) ;

	close_button.set_icon(ICO_SEARCH_CLOSE, "", 17, "") ;
	change_mode.set_icon(ICO_SEARCH_PANEL, "", 20, "") ;
	find_fw.set_icon(ICO_SEARCH_NEXT, _("_Next"), 17, "") ;
	find_bk.set_icon(ICO_SEARCH_PREVIOUS, _("_Previous"), 17, "") ;

	close_button.set_relief(Gtk::RELIEF_NONE);
	change_mode.set_relief(Gtk::RELIEF_NONE);
	find_fw.set_relief(Gtk::RELIEF_NONE);
	find_bk.set_relief(Gtk::RELIEF_NONE);

	result.set_name("label_bold");

	cb_whole.set_label(_("Whole word")) ;
	cb_case.set_label(_("Case")) ;

	clear_selection.set_label(_("Clear selection")) ;
}

void MiniSearch::prepare_gui()
{
	align_gen.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_close.set(Gtk::ALIGN_RIGHT, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_box.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_mode.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0.0, 0.0) ;

	clear_selection.signal_clicked().connect(sigc::mem_fun(*this, &MiniSearch::on_clear_selection)) ;
	clear_selection.set_relief(Gtk::RELIEF_NONE);

	add(align_gen) ;
	add_events(Gdk::FOCUS_CHANGE_MASK) ;
	align_gen.add(hbox_gen) ;

	hbox_gen.pack_start(close_button, false, false, 7) ;
	hbox_gen.pack_start(hbox_search, false, false, 7) ;
		hbox_search.pack_start(combo_search, false, false) ;
		hbox_search.pack_start( *(combo_search.get_arrow()) , false, false) ;

	hbox_gen.pack_start(find_bk, false, false, 7) ;

	hbox_gen.pack_start(find_fw, false, false, 7) ;
	hbox_gen.pack_start(cb_whole, false, false, 7) ;
	hbox_gen.pack_start(cb_case, false, false, 7) ;
	hbox_gen.pack_start(combo_language, false, false, 7) ;
	hbox_gen.pack_start(combo_searchMode, false, false, 7) ;
	hbox_gen.pack_start(change_mode, false, false, 7) ;

	set_widgets_label() ;
}

void MiniSearch::display_info(Glib::ustring text)
{
	result.set_label(text) ;
	combo_search.set_name("combo_search_end") ;
	gdk_beep() ;
}

int MiniSearch::display_question(Glib::ustring text)
{
	if (parent)
	    return Explorer_dialog::msg_dialog_question(text, parent, true, "") ;
	else {
		display_info(text) ;
		return Gtk::RESPONSE_NO ;
	}
}

void MiniSearch::myHide()
{
	hide() ;
}

void MiniSearch::on_change_mode()
{
	close() ;
	m_signalSearchModeChange.emit(1) ;
}

void MiniSearch::mySet_focus()
{
	combo_search.grab_focus() ;
}

bool MiniSearch::myHasFocus()
{
	return combo_search.has_focus() ;
}

void MiniSearch::on_clear_selection()
{
	if (hasSelection) {
		selection_mode(false) ;
	}
}

} //namespace
