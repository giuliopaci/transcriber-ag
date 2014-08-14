/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class AudioTrackWidget
 *
 * AudioTrackWidget...
 */


#include <iostream>
#include "AudioWidget.h"
#include "Common/util/StringOps.h"

using namespace std;

namespace tag {

// --- AudioTrackWidget ---
AudioTrackWidget::AudioTrackWidget(IODevice *inDevice, bool silentMode)
	: TrackWidget()
{
	device = inDevice;
	st_filter	= NULL ;
	v_filter	= NULL ;
	m_filter	= NULL ;

	initFilters(silentMode);

	set_no_show_all(true);

	a_startSelection	= -1.0;
	a_endSelection		= -1.0;
	a_expanded			= false;
	a_activated			= true;
	a_selectable		= true;
	a_offset			= 0;

	a_mainBox		= Gtk::manage(new Gtk::HBox());
	theme			= Gtk::IconTheme::get_default();

	// -- Mute Button --
	a_pixbufAudioOn		= theme->load_icon("audio_on", 17, Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
	a_pixbufAudioOff	= theme->load_icon("audio_off", 17, Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);

	a_imageAudioOn		= new Gtk::Image(a_pixbufAudioOn);
	a_imageAudioOff		= new Gtk::Image(a_pixbufAudioOff);
	a_activateButton	= Gtk::manage(new Gtk::Button());

	a_activateButton->set_focus_on_click(false);
	a_activateButton->set_image(*a_imageAudioOn);
	a_activateButton->signal_clicked().connect(sigc::mem_fun(*this, &AudioTrackWidget::onActivateClicked));
	a_activateButton->set_size_request(30, -1);

	a_mainBox->pack_start(*a_activateButton, false, false, 1);
	a_activateButton->show();

	a_tooltips.set_tip(*a_activateButton, _("Enable / Disable Current Track"));


	// -- Waveform --
	a_wave = new AudioWaveformWidget(device, NULL);

	a_mainBox->pack_start(*a_wave, true, true, 2);
	a_wave->show();


	// -- Volume, VZoom, Pitch and Equalizer widgets --
	Glib::ustring tip = _("Adjust pitch factor from 0.4 (2.5x lower) to 2.5 (2.5x upper). This modifies pitch without altering playback speed.");
	tip.append("\n") ;
	tip.append(_("CTRL + click for reset")) ;

	a_avc = new AudioVolumeControlWidget(true, true, false, false);
	a_avc->signalValueChanged().connect(sigc::mem_fun(*this, &AudioTrackWidget::onVolumeChanged));
	a_avc->signalFocusIn().connect(sigc::mem_fun(*this, &AudioTrackWidget::onFocusIn));
	a_mainBox->pack_start(*a_avc, false, false, 1);
	a_avc->show();

	a_apc = new AudioPitchControlWidget(true, true, false, false);
	a_apc->signalValueChanged().connect(sigc::mem_fun(*this, &AudioTrackWidget::onPitchChanged));
	a_apc->signalFocusIn().connect(sigc::mem_fun(*this, &AudioTrackWidget::onFocusIn));
	a_mainBox->pack_start(*a_apc, false, false, 1);
	a_apc->hide();
	a_apc->setTooltip(tip.c_str(), _("Reset pitch factor to 1, normal pitch."));

	a_avzc = new AudioVZoomControlWidget(15.0);
	a_avzc->signalValueChanged().connect(sigc::mem_fun(*this, &AudioTrackWidget::onVZoomChanged));
	a_avzc->signalFocusIn().connect(sigc::mem_fun(*this, &AudioTrackWidget::onFocusIn));
	a_mainBox->pack_start(*a_avzc, false, false, 1);
	a_avzc->hide();


	// -- Offset frame --
	a_offsetFrame = Gtk::manage(new Gtk::Frame("Offset"));

	Gtk::Label* labelOffsetFrame = (Gtk::Label*)a_offsetFrame->get_label_widget();
	labelOffsetFrame->set_markup("<small>Offset</small>");
	a_offsetFrame->set_shadow_type(Gtk::SHADOW_IN);
	Gtk::HBox* hBoxOffsetFrame = Gtk::manage(new Gtk::HBox());

	a_entry = Gtk::manage(new Gtk::Entry());
	a_entry->signal_key_press_event().connect(sigc::mem_fun(*this, &AudioTrackWidget::onKeyPressed));
	a_entry->set_width_chars(2);
	hBoxOffsetFrame->pack_start(*a_entry, false, false, 0);
	a_entry->show();
	a_tooltips.set_tip(*a_entry, _("Define the offset (in seconds) to apply to this audio track and all its linked segments. Setting a positive offset makes start the signal later, a negative makes start the signal sooner, 0 cancels the offset."));

	Gtk::Button* update = Gtk::manage(new Gtk::Button("Update"));
	update->set_focus_on_click(false);
	Gtk::Label* labelUpdate = (Gtk::Label*)update->get_child();
	labelUpdate->set_markup("<small>Update</small>");
	update->signal_clicked().connect(sigc::mem_fun(*this, &AudioTrackWidget::onUpdateOffsetClicked));
	hBoxOffsetFrame->pack_start(*update, false, false, 0);
	update->show();
	a_tooltips.set_tip(*update, _("Apply the offset."));

	Gtk::VBox* offsetVBoxPrensentation = Gtk::manage(new Gtk::VBox()) ;
	Gtk::Alignment* offsetAlignPrensentation = Gtk::manage(new Gtk::Alignment()) ;
	offsetVBoxPrensentation->pack_start(*offsetAlignPrensentation, true, false) ;
	offsetAlignPrensentation->add(*hBoxOffsetFrame) ;
	offsetAlignPrensentation->set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
	offsetAlignPrensentation->show() ;
	offsetVBoxPrensentation->show() ;
	a_offsetFrame->add(*offsetVBoxPrensentation);
	hBoxOffsetFrame->show();

	a_mainBox->pack_start(*a_offsetFrame, false, false, 2);
	a_offsetFrame->hide();

	// -- Modification Frame --
	a_modificationFrame = Gtk::manage(new Gtk::Frame("Modification"));
	Gtk::Label* labelModificationFrame = (Gtk::Label*)a_modificationFrame->get_label_widget();
	labelModificationFrame->set_markup("<small>Modification</small>");
	a_modificationFrame->set_shadow_type(Gtk::SHADOW_IN);
	Gtk::VBox* hBoxModificationFrame = Gtk::manage(new Gtk::VBox());

	Gtk::Button* apply = Gtk::manage(new Gtk::Button("Apply..."));
	Gtk::Label* labelApply = (Gtk::Label*)apply->get_child();
	labelApply->set_markup("<small>Apply...</small>");
	hBoxModificationFrame->pack_start(*apply, false, false, 0);
	apply->show();

	Gtk::Button* undo = Gtk::manage(new Gtk::Button("Undo"));
	Gtk::Label* labelUndo = (Gtk::Label*)undo->get_child();
	labelUndo->set_markup("<small>Undo</small>");
	hBoxModificationFrame->pack_start(*undo, false, false, 0);
	undo->show();
	undo->set_sensitive(false);

	Gtk::Button* redo = Gtk::manage(new Gtk::Button("Redo"));
	Gtk::Label* labelRedo = (Gtk::Label*)redo->get_child();
	labelRedo->set_markup("<small>Redo</small>");
	hBoxModificationFrame->pack_start(*redo, false, false, 0);
	redo->show();
	redo->set_sensitive(false);

	a_modificationFrame->add(*hBoxModificationFrame);

	// -- Expand button --
	Gtk::VBox* expandBox = Gtk::manage(new Gtk::VBox());

	a_expandButton.set_image(1, ICO_EXPAND_LEFT, SMALL_TOGGLE_BUTTON_SIZE) ;
	a_expandButton.set_image(2, ICO_EXPAND_LEFT_OVER, SMALL_TOGGLE_BUTTON_SIZE) ;
	a_expandButton.set_image(3, ICO_EXPAND_LEFT_PRESS, SMALL_TOGGLE_BUTTON_SIZE) ;
	a_expandButton.signal_button_release_event().connect(sigc::mem_fun(*this, &AudioTrackWidget::onExpandClicked));
	a_expandButton.show();
	expandBox->pack_start(a_expandButton, true, false, 0);
	a_tooltips.set_tip(a_expandButton, _("Expand or collapse the pitch and the offset frame. Only volume frame is always showed."));

	a_mainBox->pack_start(*expandBox, false, false, 0);
	expandBox->show();

	pack_start(*a_mainBox, true, true, 0);
	a_mainBox->show();
}


// --- ~AudioTrackWidget ---
AudioTrackWidget::~AudioTrackWidget()
{
	delete a_imageAudioOn;
	delete a_imageAudioOff;
	delete a_wave;
	delete a_avc;
	delete a_avzc;
	delete a_apc;

	if (st_filter)
		delete st_filter;
	if (v_filter)
		delete v_filter;
	if (m_filter)
		delete m_filter;
}


// --- InitFilters ---
void AudioTrackWidget::initFilters(bool silentMode)
{
	if (device != NULL)
	{
		if (!silentMode)
			st_filter	= new SoundTouchFilter();
		v_filter	= new VolumeFilter();
		m_filter	= new MuteFilter();

		v_filter->init(device->m_info());

		if (!silentMode) {
			st_filter->setMediumPath(device->m_medium());
			st_filter->init(device->m_info());
			filters.push_back(st_filter);
		}

		filters.push_back(v_filter);
		filters.push_back(m_filter);
	}
}


// --- OnKeyPressed ---
bool AudioTrackWidget::onKeyPressed(GdkEventKey* p_event)
{
	printf("b\n");
	return true;
}


// --- ShowActivateBtn ---
void AudioTrackWidget::showActivateBtn(bool b)
{
	if (b)
		a_activateButton->show();
	else
		a_activateButton->hide();
}


// --- SetSelectable ---
void AudioTrackWidget::setSelectable(bool p_selectable)
{
	a_selectable = p_selectable;
	a_wave->setSelectable(p_selectable);
}


// --- OnActivateClicked ---
void AudioTrackWidget::onActivateClicked()
{
	a_activated = !a_activated;

	if (a_activated)
	{
		a_activateButton->set_image(*a_imageAudioOn);
		a_wave->set_sensitive(true);
		a_avc->set_sensitive(true);
		a_avzc->set_sensitive(true);
		a_apc->set_sensitive(true);
		a_offsetFrame->set_sensitive(true);
		a_modificationFrame->set_sensitive(true);
	}
	else
	{
		a_activateButton->set_image(*a_imageAudioOff);
		a_wave->set_sensitive(false);
		a_avc->set_sensitive(false);
		a_avzc->set_sensitive(false);
		a_apc->set_sensitive(false);
		a_offsetFrame->set_sensitive(false);
		a_modificationFrame->set_sensitive(false);
	}

	a_wave->setEnabled(a_activated);
	a_signalActivated.emit(a_activated);
	m_filter->mute(!a_activated);
}


// --- OnExpandClicked ---
bool AudioTrackWidget::onExpandClicked(GdkEventButton* event)
{
	if (a_expanded)
	{
		a_apc->hide();
		a_avzc->hide();
		a_modificationFrame->hide();
		a_offsetFrame->hide();
		a_expandButton.set_image(1, ICO_EXPAND_LEFT, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(2, ICO_EXPAND_LEFT_OVER, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(3, ICO_EXPAND_LEFT_PRESS, SMALL_TOGGLE_BUTTON_SIZE) ;
	}
	else
	{
		a_apc->show();
		a_avzc->show();
		a_modificationFrame->show();
		a_offsetFrame->show();
		a_expandButton.set_image(1, ICO_EXPAND_RIGHT, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(2, ICO_EXPAND_RIGHT_OVER, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(3, ICO_EXPAND_RIGHT_PRESS, SMALL_TOGGLE_BUTTON_SIZE) ;
	}

	a_expanded = !a_expanded;
	a_signalExpanded.emit(a_expanded);
	return false ;
}


// --- OnUpdateOffsetClicked ---
void AudioTrackWidget::onUpdateOffsetClicked()
{
	string str = a_entry->get_text();

	for (unsigned int i = 0; i < str.length(); i++)
		if (str[i] == ',')
			str[i] = '.';

	a_offset = my_atof(str.c_str());
	a_wave->setOffset(a_offset);
	a_signalOffsetUpdated.emit(a_offset);
}


// --- OnFocusIn ---
void AudioTrackWidget::onFocusIn()
{
	a_signalFocusIn.emit();
}


// --- SetChannelID ---
void AudioTrackWidget::setChannelID(int id)
{
	channelID = id;
	
	if (st_filter)
		st_filter->setChannelID(channelID);
	if (m_filter)
		m_filter->setChannelID(channelID);
	if (v_filter)
		v_filter->setChannelID(channelID);
}


// --- SetNbChannels ---
void AudioTrackWidget::setNbChannels(int nb)
{
	st_filter->setChannels(nb);
	m_filter->setChannels(nb);
	v_filter->setChannels(nb);
}


// --- SetOffset ---
void AudioTrackWidget::setOffset(float in_cursor, float in_offset)
{
	a_cursor = in_cursor;
	a_offset = in_offset;

	if (st_filter)
		st_filter->setOffset(a_offset);
	if (st_filter)
		st_filter->seek(a_cursor);

	// -- Widgets --
	a_wave->setOffset(a_offset);
	a_entry->set_text( StringOps().fromFloat(a_offset) );
}


// --- getActivateButtonSize ---
int AudioTrackWidget::getActivateBtnSize()
{
	int width = 30;	// Ugly hack
	return width;
}


} // namespace
