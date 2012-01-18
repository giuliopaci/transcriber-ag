/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  @file Dialogs.cpp
 *  @brief basic application dialogs
 *
 */

#include "Dialogs.h"
#include "globals.h"
#include "icons/Icons.h"
#include "util/Utils.h"
#include "Common/VersionInfo.h"
namespace dlg {


/**
*  Confirmsg3 :
*    builds and pops up a 3-choices dialog : yes / no / cancel
*/
Confirmsg3::Confirmsg3(const Glib::ustring& title, Gtk::Window& parent, const Glib::ustring& msg, bool modal)
		: Gtk::Dialog(title, parent, modal)
		, m_label(msg), m_image(Gtk::Stock::DIALOG_QUESTION, Gtk::ICON_SIZE_DIALOG)
{ buildDialog();  set_transient_for(parent); }

Confirmsg3::Confirmsg3(const Glib::ustring& title, const Glib::ustring& msg, Gtk::Window* parent, bool modal)
		: Gtk::Dialog(title, modal), m_label(msg)
{ buildDialog(); if ( parent != NULL ) set_transient_for(*parent); }

Confirmsg3::Confirmsg3(const Glib::ustring& msg, Gtk::Window* parent)
		: Gtk::Dialog(_("Confirmation"), true), m_label(msg)
{ buildDialog(); if ( parent != NULL ) set_transient_for(*parent); }



void Confirmsg3::buildDialog()
{
	get_vbox()->pack_start(m_hbox);
	m_hbox.pack_start(m_image, Gtk::PACK_SHRINK, 10);
	m_hbox.pack_start(m_label, Gtk::PACK_EXPAND_WIDGET, 10);
	get_vbox()->show_all();

	m_yesButton = add_button(Gtk::Stock::YES, Gtk::RESPONSE_YES);
	m_noButton = add_button(Gtk::Stock::NO, Gtk::RESPONSE_NO);
	m_cancelButton = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

	tag::Icons::set_window_icon(this, ICO_TRANSCRIBER, 11) ;
//	raise();
}


void Confirmsg3::setActionLabel(int id, const Glib::ustring& str)
{
	switch (id) {
		case Gtk::RESPONSE_YES : m_yesButton->set_label(str); break;
		case Gtk::RESPONSE_NO : m_noButton->set_label(str); break;
		case Gtk::RESPONSE_CANCEL : m_cancelButton->set_label(str); break;

		}
	}

void Confirmsg3::setMessage(const Glib::ustring& msg)
{
	m_label.set_label(msg);
}



/*
*  Shortcuts for standard dialogs
*/
static int _commonmsg( Gtk::Window* parent,const Glib::ustring& a, Gtk::MessageType type=Gtk::MESSAGE_INFO, Gtk::ButtonsType btns=Gtk::BUTTONS_OK)
{
	Gtk::MessageDialog *dlg;
	int response;
	
	if ( parent != NULL )
	{
		dlg = new Gtk::MessageDialog(*parent, a, false, type, btns, true);
		dlg->set_transient_for(*parent);
	}
	else
		dlg = new Gtk::MessageDialog(a, false, type, btns, true);

	tag::Icons::set_window_icon(dlg, ICO_TRANSCRIBER, 11) ;
	dlg->set_title(TRANSAG_DISPLAY_NAME) ;

	response = dlg->run();
	dlg->hide();
	delete dlg;
	return response;
}

static int _detailedMsg(Gtk::Window* parent, const Glib::ustring& a, const Glib::ustring& detailed, Gtk::MessageType type, bool expanded)
{
	Gtk::MessageDialog *dlg;
	int response;
	Gtk::ButtonsType btns = Gtk::BUTTONS_CLOSE ;
	if ( parent != NULL )
	{
		dlg = new Gtk::MessageDialog(*parent, a, false, type, btns, true);
		dlg->set_transient_for(*parent);
	}
	else
		dlg = new Gtk::MessageDialog(a, false, type, btns, true);

	// - default decoration
	tag::Icons::set_window_icon(dlg, ICO_TRANSCRIBER, 11) ;
	dlg->set_title(TRANSAG_DISPLAY_NAME) ;
	dlg->set_resizable(true) ;

	// - expander for show / hode details
	Gtk::Expander expand ;
	expand.set_label(_("Details")) ;
	expand.set_expanded(expanded) ;

	// - scrolled window for a better display
	Gtk::ScrolledWindow scrollw ;
	scrollw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC) ;

	// - view for displaying our detailed message
	Gtk::TextView view ;
	view.get_buffer()->set_text(detailed) ;
	view.set_editable(false) ;
	view.set_cursor_visible(false) ;

	// - add all
	scrollw.add(view) ;
	expand.add(scrollw) ;
	((Gtk::Dialog*)dlg)->get_vbox()->pack_start(expand, true, true) ;

	// - show all
	view.show() ;
	scrollw.show() ;
	expand.show() ;
	((Gtk::Dialog*)dlg)->get_vbox()->show() ;

	// - run business
	response = dlg->run();
	dlg->hide();
	delete dlg;
	return response;
}


void msg(const Glib::ustring& a, Gtk::Window* parent)
{
	_commonmsg(parent, a, Gtk::MESSAGE_INFO);
}

void msg(const Glib::ustring& a, const Glib::ustring& detailed, Gtk::Window* parent, bool expanded)
{
	_detailedMsg(parent, a, detailed, Gtk::MESSAGE_INFO, expanded);
}

void warning(const Glib::ustring& a, Gtk::Window* parent)
{
	_commonmsg(parent, a, Gtk::MESSAGE_WARNING);
}

void warning(const Glib::ustring& msg, const Glib::ustring& detailed, Gtk::Window* parent, bool expanded)
{
	_detailedMsg(parent, msg, detailed, Gtk::MESSAGE_WARNING, expanded) ;
}

void error(const Glib::ustring& a, Gtk::Window* parent)
{
	_commonmsg(parent, a, Gtk::MESSAGE_ERROR);
}

void error(const Glib::ustring& msg, const Glib::ustring& detailed, Gtk::Window* parent, bool expanded)
{
	_detailedMsg(parent, msg, detailed, Gtk::MESSAGE_ERROR, expanded) ;
}

bool confirm(const Glib::ustring& a, Gtk::Window* parent)
{
	Gtk::Dialog* dialog = NULL ;
	if (parent)
		dialog = new Gtk::Dialog("", *parent, true, false) ;
	else
		dialog = new Gtk::Dialog("", true, false) ;

	tag::Icons::set_window_icon(dialog, ICO_TRANSCRIBER, 11) ;

	Gtk::HBox box ;
	Gtk::Label ltext,  blank_top, blank_bottom ;
	Gtk::Image image ;
	blank_top.set_label(" ");
	blank_bottom.set_label(" ");

	Gtk::IconSize *size = new Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) ;
	image.set(Gtk::Stock::DIALOG_QUESTION, *size) ;

	box.pack_start(image, false, false) ;
	box.pack_start(ltext, false, false) ;
	ltext.set_label("   " + a) ;

	dialog->get_vbox()->pack_start(blank_top) ;
	dialog->get_vbox()->pack_start(box) ;
	dialog->get_vbox()->pack_start(blank_bottom) ;
	dialog->get_vbox()->show_all_children() ;
	ltext.set_name("dialog_label") ;

  	dialog->add_button(Gtk::Stock::YES, Gtk::RESPONSE_YES);
 	Gtk::Button* no = dialog->add_button(Gtk::Stock::NO, Gtk::RESPONSE_NO);
	dialog->set_focus(*no) ;

	int res = dialog->run() ;
	if (size)
		delete(size) ;
	if (dialog)
		delete(dialog) ;

	return ( res == Gtk::RESPONSE_YES) ;
}

int confirmOrCancel(const Glib::ustring& a, const Glib::ustring& choice1, const Glib::ustring& choice2, Gtk::Window* parent)
{
	Gtk::Dialog* dialog = NULL ;
	if (parent)
		dialog = new Gtk::Dialog("", *parent, true, false) ;
	else
		dialog = new Gtk::Dialog("", true, false) ;

	tag::Icons::set_window_icon(dialog, ICO_TRANSCRIBER, 11) ;

	Gtk::HBox box ;
	Gtk::Label ltext,  blank_top, blank_bottom ;
	Gtk::Image image ;
	blank_top.set_label(" ");
	blank_bottom.set_label(" ");

	Gtk::IconSize *size = new Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) ;
	image.set(Gtk::Stock::DIALOG_QUESTION, *size) ;

	box.pack_start(image, false, false) ;
	box.pack_start(ltext, false, false) ;
	ltext.set_label("   " + a) ;

	dialog->get_vbox()->pack_start(blank_top) ;
	dialog->get_vbox()->pack_start(box) ;
	dialog->get_vbox()->pack_start(blank_bottom) ;
	dialog->get_vbox()->show_all_children() ;
	ltext.set_name("dialog_label") ;

  	if ( choice1.empty() )
  		dialog->add_button(Gtk::Stock::YES, Gtk::RESPONSE_YES);
  	else
  		dialog->add_button(choice1, Gtk::RESPONSE_YES);

  	if ( choice2.empty() )
  		dialog->add_button(Gtk::Stock::NO, Gtk::RESPONSE_NO);
  	else
  		dialog->add_button(choice2, Gtk::RESPONSE_NO);

 	Gtk::Button* cancel = dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog->set_focus(*cancel) ;

	int res = dialog->run() ;
	if (size)
		delete(size) ;
	if (dialog)
		delete(dialog) ;

	return res  ;
}

bool confirmWithButton(const Glib::ustring& button_yes_txt, const Glib::ustring& button_no_txt, const Glib::ustring& a, Gtk::Window* parent)
{
	Gtk::Dialog* dialog = NULL ;
	if (parent)
		dialog = new Gtk::Dialog("", *parent, true, false) ;
	else
		dialog = new Gtk::Dialog("", true, false) ;

	tag::Icons::set_window_icon(dialog, ICO_TRANSCRIBER, 11) ;

	Gtk::HBox box ;
	Gtk::Label ltext,  blank_top, blank_bottom ;
	Gtk::Image image ;
	blank_top.set_label(" ");
	blank_bottom.set_label(" ");

	Gtk::IconSize *size = new Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) ;
	image.set(Gtk::Stock::DIALOG_QUESTION, *size) ;

	box.pack_start(image, false, false) ;
	box.pack_start(ltext, false, false) ;
	ltext.set_label("   " + a) ;

	dialog->get_vbox()->pack_start(blank_top) ;
	dialog->get_vbox()->pack_start(box) ;
	dialog->get_vbox()->pack_start(blank_bottom) ;
	dialog->get_vbox()->show_all_children() ;
	ltext.set_name("dialog_label") ;

  	dialog->add_button(button_yes_txt, Gtk::RESPONSE_YES);
 	Gtk::Button* no = dialog->add_button(button_no_txt, Gtk::RESPONSE_NO);
	dialog->set_focus(*no) ;

	int res = dialog->run() ;
	if (size)
		delete(size) ;
	if (dialog)
		delete(dialog) ;

	return ( res == Gtk::RESPONSE_YES) ;
}

}




