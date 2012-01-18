/*
 * GotToDialog.cpp
 *
 *  Created on: 16 mars 2009
 *      Author: montilla
 */

#include "GoToDialog.h"
#include "Common/icons/Icons.h"
#include "Common/util/Utils.h"
#include "Common/globals.h"

namespace tag {

GoToDialog::GoToDialog()
{
	result = -1 ;
	//title
	Icons::set_window_icon(this, ICO_TRANSCRIBER, 12) ;
	set_title(_("Go to offset")) ;

	Gtk::VBox* vbox = get_vbox() ;
	if (vbox) {
		vbox->pack_start(hbox, true, true) ;

		label.set_label(_("Position (seconds):")) ;
		hbox.pack_start(label, false, false, 3) ;
		hbox.pack_start(entry, true, true, 3) ;
		hbox.show_all_children(true) ;

		vbox->show_all_children(true) ;

		add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY) ;
		add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
	}
}

void GoToDialog::on_response(int p_id)
{
	if (p_id == Gtk::RESPONSE_APPLY)
	{
		std::string res = entry.get_text() ;
		if (res.empty())
			result = -1 ;
		else {
			result = my_atof(res.c_str()) ;
		}
	}
	else if (p_id == Gtk::RESPONSE_CANCEL || p_id==-4) {
		result = -1 ;
	}
}

GoToDialog::~GoToDialog()
{
	// TODO Auto-generated destructor stub
}

} /* namespace */
