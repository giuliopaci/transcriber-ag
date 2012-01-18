/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef FTPFRAME_H_
#define FTPFRAME_H_

#include "PreferencesFrame.h"

namespace tag {
/**
 * @class 	FTPFrame
 * @ingroup	GUI
 *
 * Frame for FTP configuration
 * @deprecated 		Not Used Anymore
 */
class FTPFrame : public PreferencesFrame
{
	public:
		/*! DEPRECATED */
		FTPFrame(Configuration* _config, Gtk::Window* _parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values) ;
		virtual ~FTPFrame();
		void reload_data() ;

		/*! DEPRECATED */
		void set_machine_changed(bool value) {machine_changed = value;}
		/*! DEPRECATED */
		bool get_machine_changed() { return machine_changed ;}

	private :
		bool machine_changed ;
		void on_audioRep_changed() ;
		void on_loadFiles_changed() ;

		Gtk::HBox hbox_machine ;
		Gtk::Label label_machine ;
		Gtk::Entry entry_machine ;
		IcoPackImage image_machine ;

		void on_entry_machine_changed() ;
};

} //namespace

#endif /*FTPFRAME_H_*/
