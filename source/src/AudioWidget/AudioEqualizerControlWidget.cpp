/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

namespace tag {

/**
 * @class AudioEqualizerControlWidget
 *
 * Class allowing control of frequencies volume thanks to a graphic scale.
 */

#include "AudioWidget.h"

bool AudioEqualizerControlWidget::onScaleValueChanged(Gtk::ScrollType p_scrollType, double p_data, int p_index)
{
	// Get factor
	float scaleValue = (float)p_data;
	float DB = scaleValueToDB(scaleValue);
	a_rangesFactors[p_index] = DBtoFactor(DB);

	// Reflect modification
	updateGUI();
	a_signalValueChanged.emit(a_rangesFactors);

	return false;
}

void AudioEqualizerControlWidget::onResetButtonClicked()
{
	// Resetting factors and modification detect
	int modifications = 0;

	for (int i = 0; i < a_rangesCount+2; i++)
	{
		if (!a_displayLimits)
		{
			if (i == 0)
				i++;

			if (i == a_rangesCount+1)
				break;
		}

		if (a_rangesFactors[i] != 1.0)
		{
			a_rangesFactors[i] = 1.0;
			modifications = 1;
		}
	}

	if (!modifications)
		return;

	// Reflect modification
	updateGUI();
	updateGUI(); // Twice, if not then display bug
	a_signalValueChanged.emit(a_rangesFactors);
}


void AudioEqualizerControlWidget::updateGUI()
{
	char str[256];

	// For each range
	for (int i = 0; i < a_rangesCount+2; i++)
	{
		if (!a_displayLimits)
		{
			if (i == 0)
				i++;

			if (i == a_rangesCount+1)
				break;
		}

		// Update scale
		float DB = factorToDB(a_rangesFactors[i]);
		float scaleValue = DBToScaleValue(DB);
		a_scales[i]->set_value(scaleValue);

		// Update label
		if (a_rangesFactors[i] == 0.0)
		{
			sprintf(str, "<small>%s</small>", _("Mute"));
			a_labelsValues[i]->set_markup(str);
		}
		else
		{
			if (DB >= 0.0)
			{
				if (DB >= 9.95)
					sprintf(str, "<small>+%.0f</small>", DB);
				else
					sprintf(str, "<small>+%.1f</small>", DB);
			}
			else
			{
				if (DB <= -9.95)
					sprintf(str, "<small>%.0f</small>", DB);
				else
					sprintf(str, "<small>%.1f</small>", DB);
			}
			a_labelsValues[i]->set_markup(str);
		}
	}
}


float AudioEqualizerControlWidget::scaleValueToDB(float p_scaleValue)
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


float AudioEqualizerControlWidget::DBToScaleValue(float p_DB)
{
	if (p_DB <= -15)
		return 201;

	return (14.0-p_DB)*100.0/14.0;
}


float AudioEqualizerControlWidget::DBtoFactor(float p_DB)
{
	if (p_DB <= -15)
		return 0;

	return pow(10.0, p_DB/10.0);
}


float AudioEqualizerControlWidget::factorToDB(float p_factor)
{
	if (p_factor == 0)
		return -15;

	return 10.0*log10(p_factor);
}


AudioEqualizerControlWidget::AudioEqualizerControlWidget(int p_rangesCount, float* p_ranges, bool p_displayLimits) : Gtk::Frame(_("Equalizer"))
{
	set_shadow_type(Gtk::SHADOW_IN);
	char str[256];

	a_rangesCount = p_rangesCount;
	a_rangesFreqs = p_ranges;
	a_rangesFactors = new float[p_rangesCount+2];

	for (int i = 0; i < p_rangesCount+2; i++) a_rangesFactors[i] = 1.0;
		a_displayLimits = p_displayLimits;

	// Format label of the frame
	Gtk::Label* label = (Gtk::Label*)get_label_widget();
	sprintf(str, "<small>%s</small>", _("Equalizer"));
	label->set_markup(str);

	a_scales		= new Gtk::VScale*[a_rangesCount+2];
	a_labelsValues	= new Gtk::Label*[a_rangesCount+2+1];

	Gtk::VBox* vboxMain = Gtk::manage(new Gtk::VBox(false, 0));
	Gtk::HBox* hboxMain = Gtk::manage(new Gtk::HBox(false, 0));
	Gtk::VBox* vboxVide = Gtk::manage(new Gtk::VBox(false, 0));
	vboxVide->set_size_request(12, -1);
	hboxMain->add(*vboxVide);
	vboxVide->show();

	Gdk::Color lightBlue("blue");
	lightBlue.set_rgb_p(0.75, 0.75, 1.0);
	get_default_colormap()->alloc_color(lightBlue);

	// For each range
	for (int i = 0; i < a_rangesCount+2; i++)
	{
		if (!a_displayLimits)
		{
			if (i == 0)
				i++;

			if (i == a_rangesCount+1)
				break;
		}

		a_scales[i] = Gtk::manage(new Gtk::VScale(0, 201+1, 1));
		a_scales[i]->set_draw_value(false);
		a_scales[i]->modify_base(Gtk::STATE_SELECTED, a_scales[i]->get_style()->get_bg(Gtk::STATE_NORMAL));
		a_scales[i]->modify_bg(Gtk::STATE_NORMAL, lightBlue);
		a_scales[i]->modify_bg(Gtk::STATE_PRELIGHT, lightBlue);
		a_scales[i]->set_size_request(25, -1);

		Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox(false, 0));

		// Label value
		a_labelsValues[i] = Gtk::manage(new Gtk::Label());
		vbox->pack_start(*a_labelsValues[i], false, false, 0);
		a_labelsValues[i]->show();

		Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, 0));
		hbox->add(*a_scales[i]);
		a_scales[i]->show();

		vbox->pack_start(*hbox, true, true, 0);
		hbox->show();

		hboxMain->pack_start(*vbox, true, true, 0);
		vbox->show();

		a_scales[i]->signal_change_value().connect(sigc::bind<int>(sigc::mem_fun(*this, &AudioEqualizerControlWidget::onScaleValueChanged), i));
		a_rangesFactors[i] = 1.0;
	}

	// Labels on the side
	Gtk::VBox* labelsBox = Gtk::manage(new Gtk::VBox(false, 0));
	Gtk::Label* label0 = Gtk::manage(new Gtk::Label());
	Gtk::Label* label1 = Gtk::manage(new Gtk::Label());
	Gtk::Label* label2 = Gtk::manage(new Gtk::Label());
	Gtk::Label* label3 = Gtk::manage(new Gtk::Label());
	Gtk::Label* label4 = Gtk::manage(new Gtk::Label());
	Gtk::Label* label5 = Gtk::manage(new Gtk::Label());
	label0->set_markup("<small></small>");
	label1->set_markup("<small>+14dB</small>");
	label2->set_markup("<small>+7dB</small>");
	label3->set_markup("<small>0dB</small>");
	label4->set_markup("<small>-7dB</small>");
	label5->set_markup("<small>-14bB</small>");
	labelsBox->pack_start(*label0, false, false, 0);
	labelsBox->pack_start(*label1, false, false, 0);
	labelsBox->pack_start(*label2, true, true, 0);
	labelsBox->pack_start(*label3, false, false, 0);
	labelsBox->pack_start(*label4, true, true, 0);
	labelsBox->pack_start(*label5, false, false, 0);
	label0->show();
	label1->show();
	label2->show();
	label3->show();
	label4->show();
	label5->show();
	hboxMain->add(*labelsBox);
	labelsBox->show();

	// Frequencies
	Gtk::HBox* hboxFreqs = Gtk::manage(new Gtk::HBox(false, 0));
	if (a_displayLimits)
	{
		Gtk::Label* labelFreq = Gtk::manage(new Gtk::Label());
		labelFreq->set_markup("<small>0Hz</small>");
		labelFreq->set_size_request(25, -1);
		hboxFreqs->pack_start(*labelFreq, false, false, 0);
		labelFreq->show();
	}

	for (int i = 1; i < a_rangesCount+2; i++)
	{
		Gtk::Label* labelFreq = Gtk::manage(new Gtk::Label());
		float freq = a_rangesFreqs[i-1];
		char* hz = "";

		if ((!a_displayLimits) && ((i == 1) || (i == a_rangesCount+1)))
			hz = "Hz";

		if (freq >= 10000)
			sprintf(str, "<small>%.0fk%s</small>", freq/1000, hz);
		else
			if (freq >= 1000)
				sprintf(str, "<small>%.1fk%s</small>", freq/1000, hz);
			else
				sprintf(str, "<small>%.0f%s</small>", freq, hz);

		labelFreq->set_markup(str);

		if ((!a_displayLimits) && (i == a_rangesCount+1))
			labelFreq->set_size_request(36, -1);
		else
			labelFreq->set_size_request(25, -1);

		hboxFreqs->pack_start(*labelFreq, false, false, 0);
		labelFreq->show();
	}

	if (a_displayLimits)
	{
		Gtk::Label* labelFreq = Gtk::manage(new Gtk::Label());
		char* str = NULL;
		sprintf(str, "<small>%sHz</small>", _("inf."));
		labelFreq->set_markup(str);
		labelFreq->set_size_request(36, -1);
		hboxFreqs->pack_start(*labelFreq, false, false, 0);
		labelFreq->show();
	}

	vboxMain->pack_start(*hboxMain, true, true, 0);
	hboxMain->show();

	vboxMain->pack_start(*hboxFreqs, false, false, 0);
	hboxFreqs->show();

	// Reset button
	Gtk::Button* buttonReset = Gtk::manage(new Gtk::Button(""));
	buttonReset->signal_clicked().connect(sigc::mem_fun(*this, &AudioEqualizerControlWidget::onResetButtonClicked));
	Glib::ListHandle<Widget*> list = buttonReset->get_children();
	Glib::ListHandle<Widget*>::iterator it = list.begin();
	Gtk::Label* labelReset = (Gtk::Label*)(*it);
	sprintf(str, "<small>%s</small>", _("Reset"));
	labelReset->set_markup(str);
	vboxMain->pack_start(*buttonReset, false, false, 0);
	buttonReset->show();

	add(*vboxMain);
	vboxMain->show();

	updateGUI();
}


void AudioEqualizerControlWidget::setRangesFactors(float* p_rangesFactors)
{
	a_rangesFactors = p_rangesFactors;
	updateGUI();
}

} //namespace
