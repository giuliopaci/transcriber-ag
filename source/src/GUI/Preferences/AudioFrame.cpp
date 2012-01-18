/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* FERRY Guillaume - LECUYER Paule - MONTILLA Jmarc */
/* 	         																	*/
/********************************************************************************/
#include "AudioFrame.h"

namespace tag {

AudioFrame::AudioFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values)
: PreferencesFrame(config, parent, _("Audio panel"), _dynamic_values, _static_values)
{
	//> DISPLAY FRAME
	vbox.pack_start(display_Frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		display_Frame.add(display_vbox) ;

	display_Frame.set_label(_("Display options")) ;
	display_Frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;

			//> normalisation
			display_vbox.pack_start(normalization_Hbox, false, false, TAG_PREFERENCESFRAME_SPACE) ;
				normalization_Hbox.pack_start(normalization_checkbox, false, false, 5) ;
				normalization_Hbox.pack_start(warning_normalization, false, false, 5) ;

	normalization_checkbox.set_label(_("Use absolute norm for audio signal peaks")) ;
	normalization_checkbox.signal_clicked().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(this, &AudioFrame::on_checkboxes_changed), "normalization")) ;

			//> time scale
			display_vbox.pack_start(showTimeScale_Hbox, false, false, TAG_PREFERENCESFRAME_SPACE) ;
				showTimeScale_Hbox.pack_start(showTimeScale_checkbox, false, false, 5) ;

	showTimeScale_checkbox.set_label(_("Show time scale")) ;
	showTimeScale_checkbox.signal_clicked().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(this, &AudioFrame::on_checkboxes_changed), "showTimeScale")) ;

			//> size signal
			display_vbox.pack_start(signalSize_Hbox, false, false, TAG_PREFERENCESFRAME_SPACE) ;
				signalSize_Hbox.pack_start(signalSize_label, false, false, 5) ;
				signalSize_Hbox.pack_start(signalSize_spin, false, false, 5) ;
				signalSize_Hbox.pack_start(warning_signalSize, false, false, 5) ;

	signalSize_label.set_label(_("Audio signal zone size")) ;
	signalSize_spin.set_increments(1, 10) ;
	signalSize_spin.set_numeric(true) ;
	signalSize_spin.set_range(10, 200) ;
	//signalSize_spin.set_digits(3) ;
	signalSize_spin.set_update_policy(Gtk::UPDATE_IF_VALID) ;
	signalSize_spin.signal_value_changed().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(this, &AudioFrame::on_spins_changed), "signalSize")) ;
	signalSize_spin.set_editable(false) ;


	//> PLAY FRAME
	vbox.pack_start(play_Frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		play_Frame.add(play_vbox) ;
			//> stop on click
			play_vbox.pack_start(stopOnClick_Hbox, false, false, TAG_PREFERENCESFRAME_SPACE) ;
				stopOnClick_Hbox.pack_start(stopOnClick_checkbox, false, false, 5) ;
			stopOnClick_checkbox.set_label(_("Stop playing when signal clicked")) ;
			stopOnClick_checkbox.signal_clicked().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(this, &AudioFrame::on_checkboxes_changed), "stopOnClick")) ;

/*			//> rewind at end
			play_vbox.pack_start(rewindAtEnd_Hbox, false, false, TAG_PREFERENCESFRAME_SPACE) ;
				rewindAtEnd_Hbox.pack_start(rewindAtEnd_checkbox, false, false, 5) ;
				rewindAtEnd_Hbox.pack_start(rewindAtEnd_label, false, false, 5) ;
			rewindAtEnd_label.set_label(_("Automatic rewind at signal end")) ;
			rewindAtEnd_checkbox.signal_clicked().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(this, &AudioFrame::on_checkboxes_changed), "rewindAtEnd")) ;

			//> rewind at end selection
			play_vbox.pack_start(rewindAtEndSelection_Hbox, false, false, TAG_PREFERENCESFRAME_SPACE) ;
				rewindAtEndSelection_Hbox.pack_start(rewindAtEndSelection_checkbox, false, false, 5) ;
				rewindAtEndSelection_Hbox.pack_start(rewindAtEndSelection_label, false, false, 5) ;
			rewindAtEndSelection_label.set_label(_("Automatic rewind at selection signal end")) ;
			rewindAtEndSelection_checkbox.signal_clicked().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(this, &AudioFrame::on_checkboxes_changed), "rewindAtEndSelection")) ;
*/		play_Frame.set_label(_("Play options")) ;
		play_Frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;

	//> ADVANCED FRAME
	vbox.pack_start(advanced_frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		advanced_frame.add(advanced_vbox) ;
			advanced_vbox.pack_start(advanced_zoomResMax_Hbox, false, false, 5) ;
				advanced_zoomResMax_Hbox.pack_start(advanced_zoomResMax_label, false, false, 5) ;
				advanced_zoomResMax_Hbox.pack_start(advanced_zoomResMax_spin, false, false, 5) ;
				advanced_zoomResMax_Hbox.pack_start(warning_zoomResMax, false, false, 5) ;
			advanced_vbox.pack_start(advanced_restartDelay_Hbox, false, false, 5) ;
				advanced_restartDelay_Hbox.pack_start(advanced_restartDelay_label, false, false, 5) ;
				advanced_restartDelay_Hbox.pack_start(advanced_restartDelay_spin, false, false, 5) ;
				advanced_restartDelay_Hbox.pack_start(warning_restartDelay, false, false, 5) ;
		advanced_frame.set_label(_("Advanced options")) ;
		advanced_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;

	advanced_zoomResMax_label.set_label(_("Maximal zoom resolution (seconds/pixel)")) ;
	advanced_restartDelay_label.set_label(_("Go back before playing (seconds)")) ;

	advanced_zoomResMax_spin.set_increments(0.001, 0.005) ;
	advanced_zoomResMax_spin.set_numeric(true) ;
	advanced_zoomResMax_spin.set_range(0.001, 0.010) ;
	advanced_zoomResMax_spin.set_digits(3) ;
	advanced_zoomResMax_spin.set_update_policy(Gtk::UPDATE_IF_VALID) ;
	advanced_zoomResMax_spin.signal_value_changed().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(this, &AudioFrame::on_spins_changed), "zoomResolution")) ;
	advanced_zoomResMax_spin.set_editable(false) ;

	advanced_restartDelay_spin.set_increments(0.001, 1) ;
	advanced_restartDelay_spin.set_numeric(true) ;
	advanced_restartDelay_spin.set_range(0.000, 60.000) ;
	advanced_restartDelay_spin.set_digits(3) ;
	advanced_restartDelay_spin.set_update_policy(Gtk::UPDATE_IF_VALID) ;
	advanced_restartDelay_spin.signal_value_changed().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(this, &AudioFrame::on_spins_changed), "restart_delay")) ;
	advanced_restartDelay_spin.set_editable(false) ;

	//HERE INSERT warning images of static changes values
	warning_images.insert(warning_images.begin(), &warning_normalization) ;
	warning_images.insert(warning_images.begin(), &warning_zoomResMax) ;
	warning_images.insert(warning_images.begin(), &warning_restartDelay) ;
	warning_images.insert(warning_images.begin(), &warning_signalSize) ;

	Glib::ustring tip = _("Mouse button 1: smallest incrementation") ;
	tip.append("\n") ;
	tip.append(_("Mouse wheel click: larger incrementation")) ;
	tip.append("\n") ;
	tip.append(_("Mouse button 3: reach min/max value")) ;
	tooltip.set_tip(advanced_zoomResMax_spin, tip) ;
	tooltip.set_tip(advanced_zoomResMax_label, tip) ;
	tooltip.set_tip(advanced_restartDelay_spin, tip) ;
	tooltip.set_tip(advanced_restartDelay_label, tip) ;
	tooltip.set_tip(signalSize_spin, tip) ;

	reload_data() ;
	modified(false) ;
}


AudioFrame::~AudioFrame()
{
}


void AudioFrame::on_spins_changed(Glib::ustring mode)
{
	if (lock_data)
		return ;

	if (mode.compare("zoomResolution")==0)
	{
		double value = advanced_zoomResMax_spin.get_value() ;
		if (value>=0) {
			modified(true) ;
			config->set_AUDIO_resolutionZoom(value, false) ;
			set_warnings(&warning_zoomResMax,1) ;
			//iset_dynamic_value(TAG_PREFERENCES_PARAM_AUTOSAVE, value) ;
		}
	}
	else if (mode.compare("restart_delay")==0)
	{
		double value = advanced_restartDelay_spin.get_value() ;
		if (value>=0) {
			modified(true) ;
			config->set_AUDIO_restartDelay(value, false) ;
			set_warnings(&warning_restartDelay,1) ;
			//iset_dynamic_value(TAG_PREFERENCES_PARAM_AUTOSAVE, value) ;
		}
	}
	else if (mode.compare("signalSize")==0)
	{
		double value = signalSize_spin.get_value() ;
		if (value>=0) {
			modified(true) ;
			config->set_AUDIO_signalVScaleSize(value, false) ;
			set_warnings(&warning_signalSize,1) ;
			//iset_dynamic_value(TAG_PREFERENCES_PARAM_AUTOSAVE, value) ;
		}
	}


}

void AudioFrame::on_checkboxes_changed(Glib::ustring mode)
{
	if (lock_data)
		return ;

	if ( mode.compare("normalization")==0 ) {
		modified(true) ;
		config->set_AUDIO_absoluteNormalisation(normalization_checkbox.get_active(), false) ;
		set_warnings(&warning_normalization,1) ;
	}
	else if ( mode.compare("stopOnClick")==0 ) {
		modified(true) ;
		config->set_AUDIO_stopOnClick(stopOnClick_checkbox.get_active(), false) ;
		set_formatted_boolean_dynamic_value(TAG_PREFERENCES_PARAM_STOPONCLICK, stopOnClick_checkbox.get_active()) ;
	}
/*	else if ( mode.compare("rewindAtEnd")==0 ) {
		modified(true) ;
		config->set_AUDIO_rewindAtEnd(rewindAtEnd_checkbox.get_active(), false) ;
	}
	else if ( mode.compare("rewindAtEndSelection")==0 ) {
		modified(true) ;
		config->set_AUDIO_rewindAtSelectionEnd(rewindAtEnd_checkbox.get_active(), false) ;
	}
*/	else if ( mode.compare("showTimeScale")==0 ) {
		modified(true) ;
		config->set_AUDIO_showTimeScale(showTimeScale_checkbox.get_active(), false) ;
		set_formatted_boolean_dynamic_value(TAG_PREFERENCES_PARAM_TIMESCALE, showTimeScale_checkbox.get_active()) ;
	}
}


void AudioFrame::reload_data()
{
	lock_data = true ;

	//absolute norm
	int absolute = config->get_AUDIO_absoluteNormalisation() ;
 	set_check_state(normalization_checkbox, absolute) ;

 	//stop on click
	int stopOnClick = config->get_AUDIO_stopOnClick() ;
	set_check_state(stopOnClick_checkbox, stopOnClick) ;

/*	bool rewindAtEnd = config->get_AUDIO_rewindAtEnd() ;
	rewindAtEnd_checkbox.set_active(rewindAtEnd) ;

	bool rewindAtSelectionEnd = config->get_AUDIO_rewindAtSelectionEnd() ;
	rewindAtEndSelection_checkbox.set_active(rewindAtSelectionEnd) ;
*/

	//time scale
	int showTimeScale = config->get_AUDIO_showTimeScale() ;
 	set_check_state(showTimeScale_checkbox, showTimeScale) ;

 	//zoom resolution
	double zoom = config->get_AUDIO_resolutionZoom() ;
	advanced_zoomResMax_spin.set_value(zoom) ;

	//restart delay
	double resdelay = config->get_AUDIO_restartDelay() ;
	advanced_restartDelay_spin.set_value(resdelay) ;

	//signal zone size
	double size = config->get_AUDIO_signalVScaleSize() ;
	signalSize_spin.set_value(size) ;

	lock_data = false ;
}

} //namespace
