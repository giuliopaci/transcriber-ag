/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class AudioSoundTouchControlWidget
 *
 * Class allowing control of pitch, speed or tempo thanks to a graphic scale.
 */

#include "AudioWidget.h"
#include "Common/widgets/HScale_mod.h"
#include "Common/widgets/VScale_mod.h"

namespace tag {

// --- OnScaleValueChanged ---
bool AudioSoundTouchControlWidget::onScaleValueChanged(Gtk::ScrollType p_scrollType, double p_data)
{
	a_factor = scaleValueToFactor( (float)p_data );

	// -- Notify --
	updateGUI();
	a_signalValueChanged.emit(a_factor);

	return false;
}


// --- OnResetButtonClicked ---
void AudioSoundTouchControlWidget::onResetButtonClicked()
{
	if (a_factor == 1.0)
		return;

	a_factor = 1.0;

	// -- Notify --
	updateGUI();
	a_signalValueChanged.emit(a_factor);
}


// --- UpdateGUI ---
void AudioSoundTouchControlWidget::updateGUI()
{
	// -- Scale --
	a_scale->set_value( factorToScaleValue(a_factor) );

	// -- Label --
	char str[256];
	sprintf(str, "<small>x%.2f</small>", a_factor);
	a_labelValue.set_markup(str);
}


// --- ScaleValueToFactor ---
float AudioSoundTouchControlWidget::scaleValueToFactor(float p_scaleValue)
{
	float factor = 0.0;
	float scaleValue = p_scaleValue;

	if (scaleValue > 200)
		scaleValue = 200;
	else
		if (scaleValue < 0)
			scaleValue = 0;

	if (scaleValue <= 100.0)
		factor = (100.0-scaleValue)/100.0*(a_maxFactor-1)+1.0;
	else
		factor = ((200.0-scaleValue)/100.0*(a_maxFactor-1)+1.0)/a_maxFactor;

	if (!a_vertical)
		factor = 1.0/factor;

	return factor;
}


// --- FactorToScaleValue ---
float AudioSoundTouchControlWidget::factorToScaleValue(float p_factor)
{
	float factor = p_factor;

	if (!a_vertical)
		factor = 1.0/factor;

	if (factor <= 1.0)
		return 200.0-(factor*a_maxFactor-1.0)/(a_maxFactor-1)*100.0;
	else
		return 100.0-(factor-1.0)/(a_maxFactor-1)*100.0;

	return 0.0;
}


// --- FormatStringFloat ---
char* AudioSoundTouchControlWidget::formatStringFloat(float p_f)
{
	static char s_str[256];

	if (roundf((p_f-truncf(p_f))*10.0) == 0.0)
		sprintf(s_str, "<small>%.0f</small>", p_f);
	else
		if (roundf((p_f*10.0-truncf(p_f*10.0))*100.0) == 0.0)
			sprintf(s_str, "<small>%.1f</small>", p_f);
		else
			sprintf(s_str, "<small>%.2f</small>", p_f);

	return s_str;
}


// --- OnScrollBarFocusIn ---
bool AudioSoundTouchControlWidget::onScrollBarFocusIn(GdkEventFocus* event)
{
	a_signalFocusIn.emit();
	return false;
}


// --- AudioSoundTouchControlWidget ---
AudioSoundTouchControlWidget::AudioSoundTouchControlWidget( char*	p_title,
															float	p_maxFactor,
															bool	p_vertical,
															bool	p_displayValues,
															bool	p_displayValue,
															bool	p_displayReset )
	: a_labelValue(""), a_buttonReset("")
{
	// -- Variables --
	char			str[256];
	Glib::ustring	format_label;
	Glib::ustring	label	= p_title;
	Gdk::Color		lightBlue("blue");
	Gtk::Box*		vbox	= NULL;
	Gtk::Box*		hbox	= NULL;

	p_displayValue	= true;
	p_displayValues	= false;	
	a_maxFactor		= p_maxFactor;
	a_vertical		= p_vertical;
	format_label	= "<small>" + label + "</small>";

	// -- Settings --
	set_shadow_type(Gtk::SHADOW_IN);


	// -- ScaleBars --
	if (!a_vertical)
	{
		set_label(format_label) ;
		Gtk::Label* label = (Gtk::Label*)get_label_widget();
		label->set_markup(format_label) ;
	}

	if (a_vertical)
	{
		a_scale = new VScale_mod(0, 201, 1);
		((VScale_mod*)a_scale)->signalReset().connect(sigc::mem_fun(*this, &AudioSoundTouchControlWidget::reset));
	}
	else
	{
		a_scale = new HScale_mod(0, 201, 1);
		((HScale_mod*)a_scale)->signalReset().connect(sigc::mem_fun(*this, &AudioSoundTouchControlWidget::reset));
	}

	a_scale->signal_focus_in_event().connect(sigc::mem_fun(*this, &AudioSoundTouchControlWidget::onScrollBarFocusIn));
	a_scale->set_draw_value(false);

	if (a_vertical)
		lightBlue.set_rgb_p(0.75, 0.75, 1.0);
	else
		lightBlue.set_rgb_p(0.60, 0.60, 1.0);

	get_default_colormap()->alloc_color(lightBlue);

	a_scale->modify_base(Gtk::STATE_SELECTED, a_scale->get_style()->get_bg(Gtk::STATE_NORMAL));
	a_scale->modify_bg(Gtk::STATE_NORMAL, lightBlue);
	a_scale->modify_bg(Gtk::STATE_PRELIGHT, lightBlue);

	get_default_colormap()->free_color(lightBlue);

	if (a_vertical)
		vbox = Gtk::manage(new Gtk::VBox(false, 0));
	else
		vbox = Gtk::manage(new Gtk::HBox(false, 0));

	if (a_vertical)
		hbox = Gtk::manage(new Gtk::HBox(false, 0));
	else
		hbox = Gtk::manage(new Gtk::VBox(false, 0));


	
	if (p_displayValues || p_displayValue)
	{
		// -- Side labels --
		Gtk::Box* labelsBox = NULL;
		if (a_vertical)
			labelsBox = Gtk::manage(new Gtk::VBox(false, 0));
		else
			labelsBox = Gtk::manage(new Gtk::HBox(false, 0));

		if (p_displayValue && !a_vertical)
		{
			// -- Label value --
			a_labelValue.set_size_request (35, -1);
			vbox->pack_start(a_labelValue, false, false, 0);
			a_labelValue.show();
		}
		else
		if (p_displayValue && a_vertical)
		{
			a_labelValue.set_size_request (35, -1);
			a_labelValueAlign.add(*labelsBox) ;
			a_labelValueAlign.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0) ;
			labelsBox->pack_start(a_labelValueData, false, false, 0);
			labelsBox->pack_start(a_labelValue, false, false, 0);
			a_labelValueData.set_markup(format_label) ;
			a_labelValueAlign.show_all_children(true) ;
			a_labelValue.show() ;
			a_labelValueData.show() ;
			a_labelValueUnit.show() ;
		}
		else
		{
			Gtk::Label* label1 = Gtk::manage(new Gtk::Label());
			Gtk::Label* label2 = Gtk::manage(new Gtk::Label());
			Gtk::Label* label3 = Gtk::manage(new Gtk::Label());
			Gtk::Label* label4 = Gtk::manage(new Gtk::Label());
			Gtk::Label* label5 = Gtk::manage(new Gtk::Label());
			label1->set_size_request(26, -1);
			label2->set_size_request(26, -1);
			label3->set_size_request(26, -1);
			label4->set_size_request(26, -1);
			label5->set_size_request(26, -1);

			if (a_vertical)
			{
				label1->set_markup(formatStringFloat(a_maxFactor));
				label2->set_markup(formatStringFloat((a_maxFactor+1)/2));
				label3->set_markup("<small>1</small>");
				label4->set_markup(formatStringFloat(((1/a_maxFactor)+1)/2));
				label5->set_markup(formatStringFloat(1/a_maxFactor));
			}
			else
			{
				label1->set_markup(formatStringFloat(1/a_maxFactor));
				label2->set_markup(formatStringFloat(((1/a_maxFactor)+1)/2));
				label3->set_markup("<small>1</small>");
				label4->set_markup(formatStringFloat((a_maxFactor+1)/2));
				label5->set_markup(formatStringFloat(a_maxFactor));
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
			a_scale->set_size_request(-1, AudioWidget::VERTICAL_SCALE_SIZE);
			a_scale->show() ;
			labelsBox->show() ;
			a_labelValueAlign.show() ;
		}
		else
		{
			hbox->add(*labelsBox);
			hbox->add(*a_scale);
			a_scale->set_size_request(AudioWidget::HORIZONTAL_SCALE_SIZE, -1);
			a_scale->show();
			labelsBox->show();
		}
		a_scale->set_name("small_scale") ;
	}
	else
	{
		a_scale->set_size_request(-1, -1);
		hbox->add(*a_scale);
		a_scale->show();
	}

	vbox->pack_start(*hbox, true, true, 0);
	hbox->show();

	if (p_displayReset)
	{
		// -- Reset button --
		a_buttonReset.set_focus_on_click(false);
		a_buttonReset.signal_clicked().connect(sigc::mem_fun(*this, &AudioSoundTouchControlWidget::onResetButtonClicked));
		Glib::ListHandle<Widget*> list = a_buttonReset.get_children();
		Glib::ListHandle<Widget*>::iterator it = list.begin();
		Gtk::Label* labelReset = (Gtk::Label*)(*it);
		sprintf(str, "<small>%s</small>", _("Reset"));
		labelReset->set_markup(str);
		vbox->pack_start(a_buttonReset, false, false, 0);
		a_buttonReset.show();
	}

	add(*vbox);
	vbox->show();

	a_scale->signal_change_value().connect(sigc::mem_fun(*this, &AudioSoundTouchControlWidget::onScaleValueChanged));

	a_factor = 1.0;
	updateGUI();
}


// --- ~AudioSoundTouchControlWidget ---
AudioSoundTouchControlWidget::~AudioSoundTouchControlWidget()
{
	if (a_scale)
		delete(a_scale);
}


// --- SetFactor ---
void AudioSoundTouchControlWidget::setFactor(float p_factor)
{
	if ( a_factor != p_factor )
	{
		a_factor = p_factor;
		updateGUI();
	}
}


// --- Reset ---
void AudioSoundTouchControlWidget::reset()
{
	onResetButtonClicked();
}

} // namespace
