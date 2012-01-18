/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "FTPFrame.h"
#include "Common/globals.h"


namespace tag {

FTPFrame::FTPFrame(Configuration* _config, Gtk::Window* _parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values)
: PreferencesFrame(_config, _parent, _("Ftp connection"), _dynamic_values, _static_values)
{
	machine_changed = false;

	Glib::ustring space = "  " ;

	//> FOR MACHINE
	vbox.pack_start(hbox_machine, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		hbox_machine.pack_start(label_machine, false, false, 6) ;
		hbox_machine.pack_start(entry_machine, false, false, 5) ;
		hbox_machine.pack_start(image_machine, false, false, 5) ;

	label_machine.set_label(_("Distant machine")) ;
	image_machine.set_image(ICO_NETWORK, 17) ;
	entry_machine.signal_changed().connect(sigc::mem_fun(*this, &FTPFrame::on_entry_machine_changed)) ;
	//> others

	reload_data() ;
	modified(false) ;
}

FTPFrame::~FTPFrame()
{
}


void FTPFrame::reload_data()
{
	lock_data = true ;

	//for machine
	Glib::ustring machine = config->get_FTP_machine() ;
	entry_machine.set_text(machine) ;

	lock_data = false ;
}

void FTPFrame::on_entry_machine_changed()
{
	Glib::ustring machine = entry_machine.get_text() ;
	if (machine != "") {
		entry_machine.set_text(machine) ;
		config->set_FTP_machine(machine, false) ;
		set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_FTPMACHINE, machine) ;
		modified(true) ;
		//machine_changed = true ;
	}
}

} //namespace
