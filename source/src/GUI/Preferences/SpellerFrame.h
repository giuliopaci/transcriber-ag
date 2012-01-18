/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef SPELLERFRAME_H_
#define SPELLERFRAME_H_

#include "PreferencesFrame.h"

namespace tag {
/**
 * @class 	SpellerFrame
 * @ingroup	GUI
 *
 * Frame for consulting and editing speller preferences
 *
 */
class SpellerFrame : public PreferencesFrame
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
		SpellerFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values) ;
		virtual ~SpellerFrame();

		void reload_data() ;

	private:

		//> DICO ENABLE SPELLER
		Gtk::HBox dico_enable_HBox ;
			Gtk::CheckButton dico_enable_checkbox ;

		//> DEFAULT DICO DIRECTORY
		Gtk::Frame dicoPath_frame ;
			Gtk::VBox dicoPath_Vbox ;
				Gtk::Label dicoPath_label ;
				Gtk::HBox dicoPath_Hbox ;
					Gtk::Entry dicoPath_entry ;
					Gtk::FileChooserButton dicoPath_button ;
		void on_dicoPath_changed() ;

		//> DICO OPTIONS
		Gtk::HBox dico_allowUser_HBox ;
			Gtk::CheckButton dico_allowUser_checkbox ;
		Gtk::HBox dico_allowIgnoredWord_HBox ;
			Gtk::CheckButton dico_allowIgnoredWord_checkbox ;
		void on_dicoCheckBoxes_changed(Glib::ustring mode, Glib::ustring submode) ;

		void enableDicoGUI(bool enable) ;

};

} //namespace

#endif /*SPELLERFRAME_H_*/
