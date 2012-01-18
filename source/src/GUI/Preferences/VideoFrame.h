/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* FERRY Guillaume - LECUYER Paule - MONTILLA Jmarc 	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef VIDEOFRAME_H_
#define VIDEOFRAME_H_

#include <gtkmm.h>
#include "PreferencesFrame.h"

namespace tag {
/**
 * @class 	VideoFrame
 * @ingroup	GUI
 *
 * Frame for consulting and editing Video preferences
 *
 */
class VideoFrame : public PreferencesFrame
{
	public:

		/**
		 * Constructor
		 * @param config			Pointer on the Configuration object (instanciated in top level)
		 * @param parent			Parent window
		 * @param _dynamic_values	Pointer on a map where all dynamic values that have been modified are kept(out)\n
		 * 							Dynamic values are formed by a code and a value\n
		 * 							int: code of the option (see <em>define macros</em> in Configuration file documentation
		 * 							Glib::ustring:  new value for the option
		 * @param _static_values	Pointer on a vector the modified static value images are inserted to (out)\n
		 * 							The modified static value images are displayed in notebook header when a static preference
		 * 							is changed.
		 * @note					A static value is an option that needs the notebook page to be closed
		 * 							for the modification to be visible. in constrast the dynamic values are
		 * 							immediatly applied.
		 * @remarks					Some static values could be changed into dynamic values {evolution}
		 */
		VideoFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values) ;
		virtual ~VideoFrame();

		void reload_data() ;


	private:

		/*** structure ***/
		Gtk::Frame fbrowser_Frame ;
			Gtk::VBox fbrowser_vbox ;
				//> Frame browser resolution
				Gtk::HBox resolution_Hbox ;
					Gtk::Label resolution_label ;
					Gtk::SpinButton resolution_spin ;
					IcoPackImage warning_resolution ;

		void on_checkboxes_changed(Glib::ustring mode) ;
		void on_spins_changed(Glib::ustring mode) ;
};

}

#endif /* VIDEOFRAME_H_ */
