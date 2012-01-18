/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/** @file */

#ifndef GENERALFRAME_H_
#define GENERALFRAME_H_

#include <gtkmm.h>
#include "PreferencesFrame.h"
#define AZERTY "AZERTY"
#define QWERTY "QWERTY"

#define TAG_PREFERENCES_GEN_TOOLICON _("Only icons")
#define TAG_PREFERENCES_GEN_TOOLICONTEXT _("Icons and text")
#define TAG_PREFERENCES_GEN_TOOLICONTEXTHORIZONTAL _("Only text")

namespace tag {
/**
 * @class 	GeneralFrame
 * @ingroup	GUI
 *
 * Frame for consulting and editing application global preferences
 *
 */
class GeneralFrame : public PreferencesFrame
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
		GeneralFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values);
		virtual ~GeneralFrame();

		void reload_data() ;

	private:
		//> LOAD OPENED FILES
		Gtk::CheckButton check_loadfiles ;
		void on_loadFiles_changed() ;

		//> DEFAULT AUDIO DIRECTORY
		Gtk::Frame audioRep_frame ;
			Gtk::VBox audioRep_Vbox ;
				Gtk::Label audioRep_label ;
				Gtk::HBox audioRep_Hbox ;
					Gtk::Entry audioRep_entry ;
					Gtk::FileChooserButton audioRep_button ;
					IcoPackImage warning_audioRep ;
		void on_audioRep_changed() ;

		//> DEFAULT WEB BROWSER
		Gtk::Frame defaultBrowser_frame ;
			Gtk::VBox defaultBrowser_Vbox ;
				Gtk::Label defaultBrowser_label ;
				Gtk::HBox defaultBrowser_Hbox ;
					Gtk::Entry defaultBrowser_entry ;
					Gtk::FileChooserButton defaultBrowser_button ;
		void on_defaultBrowser_changed() ;

		//> DEFAULT TOOLBAR DISPLAY
		Gtk::HBox toolbarshow_hbox ;
			Gtk::CheckButton toolbarshow_check ;
		void on_toolbar_changed() ;

		//> DEFAULT statusBAR DISPLAY
		Gtk::HBox statusbarshow_hbox ;
			Gtk::CheckButton statusbarshow_check ;
		void on_statusbar_changed() ;

		//> TOOLBAR STYLE
		Gtk::HBox toolbar_hbox ;
			Gtk::ComboBoxText toolbar_combo ;
			Gtk::Label toolbar_label ;
		void on_toolbarCombo_changed() ;

		//> Transcription name
		Gtk::Frame username_frame ;
			Gtk::VBox username_Vbox ;
				Gtk::HBox username_Hbox ;
					Gtk::Label username_label ;
					Gtk::Entry username_entry ;
					Gtk::Button username_change_button ;
					IcoPackImage warning_acronym ;

		//> inactivity delai
		Gtk::Frame timer_frame ;
			Gtk::VBox timer_vbox ;
				Gtk::HBox autosave_Hbox ;
					Gtk::Label autosave_label ;
					Gtk::SpinButton autosave_spin ;
					//IcoPackImage warning_autosave ;
				Gtk::HBox activity_Hbox ;
					Gtk::Label activity_label ;
					Gtk::SpinButton activity_spin ;
					//IcoPackImage warning_activity ;

		//> KEYBOARD MAPPING (deprecated)
		Gtk::Frame keyboard_frame ;
			Gtk::VBox keyboard_Vbox ;
				Gtk::HBox keyboard_Hbox ;
					Gtk::Label keyboard_label ;
					Gtk::ComboBoxText keyboard_combo ;
		sigc::connection keyboard_connection ;

		void prepare_combo_toolbar() ;
		void on_combo_keyboard_change() ;
		void on_entries_changed(Glib::ustring mode) ;
		void on_spins_changed(Glib::ustring mode) ;

		void on_username_button() ;
		void on_username_changed(bool modified) ;

};

} //namespace

#endif /*GENERALFRAME_H_*/
