/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "AboutDialog.h"
#include "Common/icons/Icons.h"
#include "Explorer_utils.h"
#include "Common/VersionInfo.h"
#include "Common/globals.h"

namespace tag {

AboutDialog::AboutDialog(const Glib::ustring& transcriber_image, const Glib::ustring& title, const Glib::ustring& transcriber_icon,
							const Glib::ustring& text, const Glib::ustring& under)
{
	if (transcriber_image.compare("")!=0)
		image.set_image(transcriber_image, 400) ;
	if (title.compare("")!=0)
		set_title(title) ;
	if (transcriber_icon.compare("")!=0)
		Icons::set_window_icon(this, transcriber_icon, 17) ;

	Gtk::VBox* box = get_vbox() ;

	tview = new Gtk::TextView(Gtk::TextBuffer::create()) ;
	tview->set_editable(false) ;
	tview->set_cursor_visible(false) ;
	buffer = tview->get_buffer() ;

	set_text(text) ;

	BLANK1.set_label(" ") ;
	box->pack_start(hbox_main, true, false) ;
		hbox_main.pack_start(image,false,false) ;
		hbox_main.pack_start(vbox_right,true,true) ;
	vbox_right.pack_start(*tview, true, true) ;
	if ( !under.empty() )
	{
		under_label.set_markup("<small>"+under+"</small>") ;
		vbox_right.pack_start(under_label, true, true) ;
	}
	box->pack_start(BLANK2, false, false) ;
	box->pack_start(hbox_buttons, false, false) ;

	box->show_all_children() ;

	add_button(_("Cl_ose"), Gtk::RESPONSE_CLOSE) ;
}

AboutDialog::~AboutDialog()
{
	delete(tview) ;
}

void AboutDialog::set_text(Glib::ustring txt)
{
	buffer->set_text(txt) ;
}

} //namespace
