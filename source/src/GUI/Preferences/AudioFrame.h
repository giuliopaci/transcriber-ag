/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* FERRY Guillaume - LECUYER Paule - MONTILLA Jmarc  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef AUDIOFRAME_H_
#define AUDIOFRAME_H_

#include "PreferencesFrame.h"

namespace tag {
/**
 * @class 	AudioFrame
 * @ingroup	GUI
 *
 * Frame for consulting and editing Audio part preferences
 *
 */
class AudioFrame : public PreferencesFrame
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
		AudioFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values) ;

		virtual ~AudioFrame() ;

		void reload_data() ;

	private:
		Gtk::Frame display_Frame ;
			Gtk::VBox display_vbox ;
				//> NORMALISATION PICS
				Gtk::HBox normalization_Hbox ;
					Gtk::CheckButton normalization_checkbox ;
					IcoPackImage warning_normalization ;
				//> Show Time Scale
				Gtk::HBox showTimeScale_Hbox ;
					Gtk::CheckButton showTimeScale_checkbox ;
				//> Signal Bar Size
					Gtk::HBox signalSize_Hbox ;
						Gtk::Label signalSize_label ;
						Gtk::SpinButton signalSize_spin ;
						IcoPackImage warning_signalSize ;

		Gtk::Frame play_Frame ;
			Gtk::VBox play_vbox ;
				//> STOP on CLIK
				Gtk::HBox stopOnClick_Hbox ;
					Gtk::CheckButton stopOnClick_checkbox ;
				//> Rewind At end
				Gtk::HBox rewindAtEnd_Hbox ;
					Gtk::CheckButton rewindAtEnd_checkbox ;
				//> Rewind at end selection
				Gtk::HBox rewindAtEndSelection_Hbox ;
					Gtk::CheckButton rewindAtEndSelection_checkbox ;

		Gtk::Frame advanced_frame ;
			Gtk::VBox advanced_vbox ;
				//> Zoom maximal
				Gtk::HBox advanced_zoomResMax_Hbox ;
					Gtk::Label advanced_zoomResMax_label ;
					Gtk::SpinButton advanced_zoomResMax_spin ;
					IcoPackImage warning_zoomResMax ;
				//> Restart Delay (Go Back before playing)
				Gtk::HBox advanced_restartDelay_Hbox ;
					Gtk::Label advanced_restartDelay_label ;
					Gtk::SpinButton advanced_restartDelay_spin ;
					IcoPackImage warning_restartDelay ;
		void on_checkboxes_changed(Glib::ustring mode) ;
		void on_spins_changed(Glib::ustring mode) ;
};

} //namespace

#endif /*AUDIOFRAME_H_*/
