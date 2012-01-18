/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class AudioVZoomControlWidget
 *
 * Class allowing control of volume thanks to a graphic scale.
 */

#include "AudioWidget.h"
#include "Common/widgets/VScale_mod.h"
#include "Common/widgets/HScale_mod.h"
namespace tag {

// --- AudioVZoomControlWidget ---
AudioVZoomControlWidget::AudioVZoomControlWidget(float p_maxFactor)
{
	set_shadow_type(Gtk::SHADOW_IN);
	a_maxFactor = p_maxFactor;

	// -- Variables --
	char		str[256];
	Gtk::Box*	vbox		= NULL;
	Gtk::Box*	hbox		= NULL;
	Gtk::Box*	labelsBox	= NULL;
	Gdk::Color	lightBlue("blue");
	
	// -- Label Format --
	sprintf(str, "<small>%s</small>", _("VZoom"));

	// -- Widgets Init --
	a_scale		= new VScale_mod(0, 201+1, 1) ;
	vbox		= Gtk::manage(new Gtk::VBox(false, 0));
	hbox		= Gtk::manage(new Gtk::HBox(false, 0));
	labelsBox	= Gtk::manage(new Gtk::VBox(false, 0));

	a_scale->set_draw_value(false);
	
	lightBlue.set_rgb_p(0.75, 0.75, 1.0);
	get_default_colormap()->alloc_color(lightBlue);
	
	// -- Tooltip --
	Glib::ustring tip = _("Adjust vertical zoom factor") ;
	tip.append("\n") ;
	tip.append(_("CTRL + click for reset") ) ;
	a_tooltips.set_tip(*a_scale, tip);

	// -- Scale --
	a_scale->modify_base(Gtk::STATE_SELECTED,	a_scale->get_style()->get_bg(Gtk::STATE_NORMAL));
	a_scale->modify_bg(Gtk::STATE_NORMAL,		lightBlue);
	a_scale->modify_bg(Gtk::STATE_PRELIGHT,		lightBlue);
	a_scale->set_name("small_scale");

	get_default_colormap()->free_color(lightBlue);


	// -- Side Labels --
	a_labelValueAlign.add(*labelsBox) ;
	a_labelValueAlign.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0) ;

	labelsBox->pack_start(a_labelValueData, false, false, 0);
	labelsBox->pack_start(a_labelValue, false, false, 0);

	Glib::ustring lab			= _("VZoom") ;
	Glib::ustring format_label	= "<small>" + lab + "</small>" ;

	a_labelValueData.set_markup(format_label) ;
	a_scale->set_size_request(-1, AudioWidget::VERTICAL_SCALE_SIZE);

	hbox->add(*a_scale);
	hbox->add(a_labelValueAlign);
	a_scale->show();
	a_labelValue.show() ;
	a_labelValueData.show() ;
	labelsBox->show();

	// -- Main Layout --
	vbox->pack_start(*hbox, true, true, 0);
	hbox->show();
	a_labelValueAlign.show();

	add(*vbox);
	vbox->show();

	// -- Signals --
	a_scale->signal_focus_in_event().connect(sigc::mem_fun(*this, &AudioVZoomControlWidget::onScrollBarFocusIn));
	a_scale->signal_change_value().connect(sigc::mem_fun(*this, &AudioVZoomControlWidget::onScaleValueChanged));
	((VScale_mod*)a_scale)->signalReset().connect(sigc::mem_fun(*this, &AudioVZoomControlWidget::reset));

	// -- Defaults --
	a_factor = 1.0;

	updateGUI();
}


// --- ~AudioVZoomControlWidget ---
AudioVZoomControlWidget::~AudioVZoomControlWidget()
{
	if (a_scale)
		delete(a_scale);
}


// --- ScaleValueToFactor ---
float AudioVZoomControlWidget::scaleValueToFactor(float p_scaleValue)
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

	return factor;
}


// --- FactorToScaleValue ---
float AudioVZoomControlWidget::factorToScaleValue(float p_factor)
{
	float factor = p_factor;

	if (factor <= 1.0)
		return 200.0-(factor*a_maxFactor-1.0)/(a_maxFactor-1)*100.0;
	else
		return 100.0-(factor-1.0)/(a_maxFactor-1)*100.0;

	return 0.0;
}


// --- OnScaleValueChanged ---
bool AudioVZoomControlWidget::onScaleValueChanged(Gtk::ScrollType p_scrollType, double p_data)
{
	a_factor = scaleValueToFactor( (float)p_data );

	// -- Notify --
	updateGUI();
	a_signalValueChanged.emit(a_factor);

	return false;
}


// --- UpdateGUI ---
void AudioVZoomControlWidget::updateGUI()
{
	char str[256];

	// -- Scale --
	float scaleValue = factorToScaleValue(a_factor);
	a_scale->set_value(scaleValue);

	// -- Label --
	sprintf(str, "<small>x%.2f</small>", a_factor);
	a_labelValue.set_markup(str);
}


// --- OnScrollBarFocusIn ---
bool AudioVZoomControlWidget::onScrollBarFocusIn(GdkEventFocus* event)
{
	a_signalFocusIn.emit();
	return FALSE;
}


// --- FormatStringFloat ---
char* AudioVZoomControlWidget::formatStringFloat(float p_f)
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


// --- SetFactor ---
void AudioVZoomControlWidget::setFactor(float p_factor)
{
	if ( a_factor != p_factor )
	{
		a_factor = p_factor;
		updateGUI();
	}
}


void AudioVZoomControlWidget::reset()
{
	if (a_factor == 1.0)
		return;

	a_factor = 1.0;

	// -- Notify --
	updateGUI();
	a_signalValueChanged.emit(a_factor);
}

} // namespace
