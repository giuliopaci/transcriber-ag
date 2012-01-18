/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class AudioVolumeControlWidget
 *
 * Class allowing control of volume thanks to a graphic scale.
 */

#include "AudioWidget.h"
#include "Common/widgets/VScale_mod.h"
#include "Common/widgets/HScale_mod.h"

namespace tag {

bool AudioVolumeControlWidget::onScaleValueChanged(Gtk::ScrollType p_scrollType, double p_data)
{
	// Get factor

	float scaleValue = (float)p_data;
	float DB = scaleValueToDB(scaleValue);
	a_factor = DBtoFactor(DB);

	// Reflect modification
	updateGUI();
	a_signalValueChanged.emit(a_factor);

	return false;
}


void AudioVolumeControlWidget::onResetButtonClicked()
{
	if (a_factor == 1.0)
		return;

	a_factor = 1.0;

	// Reflect modification
	updateGUI();
	a_signalValueChanged.emit(a_factor);
}


void AudioVolumeControlWidget::updateGUI()
{

	char str[256];

	// Update scale
	float DB = factorToDB(a_factor);
	float scaleValue = DBToScaleValue(DB);
	a_scale->set_value(scaleValue);

	// Update label
	if (a_factor == 0.0)
	{
		sprintf(str, "<small>%s</small>", _("Mute"));
		a_labelValue.set_markup(str);
	}
	else
	{
		if (DB >= 0)
			sprintf(str, "<small>+%.1f</small>", DB);
		else
			sprintf(str, "<small>%.1f</small>", DB);

		a_labelValue.set_markup(str);
	}
}


float AudioVolumeControlWidget::scaleValueToDB(float p_scaleValue)
{
	if (p_scaleValue >= 201)
	{
		return -15;
	}
	else
	{
		float DB = 14.0-(p_scaleValue/100.0*14.0);

		if (DB > 14.0)
			DB = 14.0;
		else
			if (DB < -14.0)
				DB = -14.0;

		return DB;
	}
	return 0.0;
}


float AudioVolumeControlWidget::DBToScaleValue(float p_DB)
{
	if (p_DB <= -15)
		return 201;

	return (14.0-p_DB)*100.0/14.0;
}


float AudioVolumeControlWidget::DBtoFactor(float p_DB)
{
	if (p_DB <= -15)
		return 0;

	return pow(10.0, p_DB/10.0);
}


float AudioVolumeControlWidget::factorToDB(float p_factor)
{
	if (p_factor == 0)
		return -15;

	return 10.0*log10(p_factor);
}


bool AudioVolumeControlWidget::onScrollBarFocusIn(GdkEventFocus* event)
{
	a_signalFocusIn.emit();
	return FALSE;
}


AudioVolumeControlWidget::AudioVolumeControlWidget(bool p_vertical, bool p_displayValues, bool p_displayValue, bool p_displayReset)
{
	a_vertical = p_vertical;
	set_shadow_type(Gtk::SHADOW_IN);


	// Format label of the frame
	/*Gtk::Label* label = (Gtk::Label*)get_label_widget(); */
	char str[256];
	sprintf(str, "<small>%s</small>", _("Volume"));
	//label->set_markup(str);
	Glib::ustring label = _("Volume") ;
	Glib::ustring format_label = "<small>" + label + "</small>" ;


	if (a_vertical) {
		a_scale = new VScale_mod(0, 201+1, 1);
		((VScale_mod*)a_scale)->signalReset().connect(sigc::mem_fun(*this, &AudioVolumeControlWidget::reset));
	}
	else {
		a_scale = new HScale_mod(0, 201+1, 1);
		((HScale_mod*)a_scale)->signalReset().connect(sigc::mem_fun(*this, &AudioVolumeControlWidget::reset));
	}

	a_scale->signal_focus_in_event().connect(sigc::mem_fun(*this, &AudioVolumeControlWidget::onScrollBarFocusIn));
	a_scale->set_draw_value(false);
	Gdk::Color lightBlue("blue");

	if (a_vertical)
		lightBlue.set_rgb_p(0.75, 0.75, 1.0);
	else
		lightBlue.set_rgb_p(0.60, 0.60, 1.0);

	get_default_colormap()->alloc_color(lightBlue);
	Glib::ustring tip = _("Adjust volume factor from -14dB to +14dB") ;
	tip.append("\n") ;
	tip.append(_("CTRL + click for reset")) ;
	a_tooltips.set_tip(*a_scale, tip);

	if (a_vertical)
	{
		a_scale->modify_base(Gtk::STATE_SELECTED, a_scale->get_style()->get_bg(Gtk::STATE_NORMAL));
		a_scale->modify_bg(Gtk::STATE_NORMAL, lightBlue);
		a_scale->modify_bg(Gtk::STATE_PRELIGHT, lightBlue);
	}
	else
	{
		a_scale->modify_base(Gtk::STATE_SELECTED, lightBlue);
		a_scale->modify_bg(Gtk::STATE_NORMAL, a_scale->get_style()->get_bg(Gtk::STATE_NORMAL));
		a_scale->modify_bg(Gtk::STATE_PRELIGHT, a_scale->get_style()->get_bg(Gtk::STATE_NORMAL));
	}

	get_default_colormap()->free_color(lightBlue);

	Gtk::Box* vbox = NULL;

	if (a_vertical)
		vbox = Gtk::manage(new Gtk::VBox(false, 0));
	else
		vbox = Gtk::manage(new Gtk::HBox(false, 0));

	p_displayValue = true ;


	Gtk::Box* hbox = NULL;

	if (a_vertical)
		hbox = Gtk::manage(new Gtk::HBox(false, 0));
	else
		hbox = Gtk::manage(new Gtk::VBox(false, 0));

	if (p_displayValues || p_displayValue)
	{
		// Labels on the side
		Gtk::Box* labelsBox = NULL;

		if (a_vertical)
			labelsBox = Gtk::manage(new Gtk::VBox(false, 0));
		else
			labelsBox = Gtk::manage(new Gtk::HBox(false, 0));

		if (p_displayValue)
		{
			// Label value
			a_labelValue.set_size_request (35, -1);
			a_labelValueAlign.add(*labelsBox) ;
			a_labelValueAlign.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0) ;
			labelsBox->pack_start(a_labelValueData, false, false, 0);
			labelsBox->pack_start(a_labelValue, false, false, 0);
			labelsBox->pack_start(a_labelValueUnit, false, false, 0);
			a_labelValueData.set_markup(format_label) ;
			a_labelValueUnit.set_markup("<small>dB</small>") ;
			a_labelValueAlign.show_all_children(true) ;
			a_labelValue.show() ;
			a_labelValueUnit.show() ;
		}

		else {
			Gtk::Label* label1 = Gtk::manage(new Gtk::Label());
			Gtk::Label* label2 = Gtk::manage(new Gtk::Label());
			Gtk::Label* label3 = Gtk::manage(new Gtk::Label());
			Gtk::Label* label4 = Gtk::manage(new Gtk::Label());
			Gtk::Label* label5 = Gtk::manage(new Gtk::Label());
			label1->set_size_request(33, -1);
			label2->set_size_request(33, -1);
			label3->set_size_request(33, -1);
			label4->set_size_request(33, -1);
			label5->set_size_request(33, -1);

			if (a_vertical)
			{
				label1->set_markup("<small>+14dB</small>");
				label2->set_markup("<small>+7dB</small>");
				label3->set_markup("<small>0dB</small>");
				label4->set_markup("<small>-7dB</small>");
				label5->set_markup("<small>-14bB</small>");
			}
			else
			{
				label1->set_markup("<small>-14dB</small>");
				label2->set_markup("<small>-7dB</small>");
				label3->set_markup("<small>0dB</small>");
				label4->set_markup("<small>+7dB</small>");
				label5->set_markup("<small>+14bB</small>");
			}

			labelsBox->pack_start(*label1, false, false, 0);
			labelsBox->pack_start(*label2, true, true, 0);
			labelsBox->pack_start(*label3, false, false, 0);
			labelsBox->pack_start(*label4, true, true, 0);
			labelsBox->pack_start(*label5, false, false, 0);
			label1->show();
			label2->show();
			label3->show();
			label4->show();
			label5->show();
		}

		if (a_vertical)
		{
			hbox->add(*a_scale);
			hbox->add(a_labelValueAlign);
		}
		else
		{
			hbox->add(*labelsBox);
			hbox->add(*a_scale);
		}
		if (a_vertical)
			a_scale->set_size_request(-1, AudioWidget::VERTICAL_SCALE_SIZE);
		/*else
			a_scale->set_size_request(VERTICAL_SCALE_SIZE, -1);*/
		a_scale->set_name("small_scale") ;
		a_scale->show() ;
		labelsBox->show() ;
		a_labelValueAlign.show() ;
	}
	else {
		a_scale->set_size_request(5*26, -1);
		hbox->add(*a_scale);
		a_scale->show();
	}

	vbox->pack_start(*hbox, true, true, 0);
	hbox->show();

	if (p_displayReset)
	{
		// Reset button
		Gtk::Button* buttonReset = Gtk::manage(new Gtk::Button(""));
		buttonReset->signal_clicked().connect(sigc::mem_fun(*this, &AudioVolumeControlWidget::onResetButtonClicked));
		Glib::ListHandle<Widget*> list = buttonReset->get_children();
		Glib::ListHandle<Widget*>::iterator it = list.begin();
		Gtk::Label* labelReset = (Gtk::Label*)(*it);
		sprintf(str, "<small>%s</small>", _("Reset"));
		labelReset->set_markup(str);
		vbox->pack_start(*buttonReset, false, false, 0);
		buttonReset->show();
		a_tooltips.set_tip(*buttonReset, _("Reset volume factor to 1, normal volume."));
	}

	add(*vbox);
	vbox->show();

	a_factor = 1.0;
	a_scale->signal_change_value().connect(sigc::mem_fun(*this, &AudioVolumeControlWidget::onScaleValueChanged));

	updateGUI();
}


void AudioVolumeControlWidget::setFactor(float p_factor)
{
	if ( a_factor != p_factor ) {
		a_factor = p_factor;
		updateGUI();
	}
}

void AudioVolumeControlWidget::reset()
{
	onResetButtonClicked() ;
}


} // namespace

