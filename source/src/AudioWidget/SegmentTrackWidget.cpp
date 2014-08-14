/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class SegmentTrackWidget
 *
 * SegmentTrackWidget...
 */

#include "AudioWidget.h"
#include <gtk/gtkstyle.h>

#include "Common/icons/Icons.h"

namespace tag {

// --- SegmentTrackWidget ---
SegmentTrackWidget::SegmentTrackWidget(float p_duration, string p_label, int p_numTrack, int startGap, bool flatMode)
: TrackWidget(), a_iconSize(1), a_imagePrevious(Gtk::Stock::MEDIA_PREVIOUS, a_iconSize), a_imageNext(Gtk::Stock::MEDIA_NEXT, a_iconSize)
{
	a_label = p_label;
	a_numTrack = p_numTrack;
	a_expanded = false;
	a_delay = 0.0;

	set_no_show_all(true);

	if ( startGap > 0 )
	{
		pack_start(a_startGap, false, false, 1);
		a_startGap.show();
		a_startGap.set_size_request(startGap, -1);
	}

	/*
	a_activateButton = Gtk::manage(new Gtk::CheckButton);

	a_activateButton->set_focus_on_click(false);
	//a_activateButton->signal_clicked().connect(sigc::mem_fun(*this, &AudioTrackWidget::onActivateClicked));
	a_activateButton->set_size_request(30, -1);

	pack_start(*a_activateButton, false, false, 1);
	a_activateButton->show();

	a_tooltips.set_tip(*a_activateButton, _("Activate / Deactive Current Track"));
	*/

	a_segment = new SegmentWidget(p_duration, p_label, flatMode);
	pack_start(*a_segment, true, true, 2);
	a_segment->show();

	// GTK Widgets
	Gtk::Button* b = Gtk::manage(new Gtk::Button());
	b->set_image(a_imagePrevious);
//	if (!flatMode)
//	{
//		b->set_size_request(-1, 22);
		b->signal_clicked().connect(sigc::mem_fun(*this, &SegmentTrackWidget::onPreviousClicked));
		a_tooltips.set_tip(*b, string(_("Allow repositioning cursor at the previous "))+p_label+string(_(" boundary.")));
//	}
//	else
//	{
//		b->set_size_request(-1, 2);
//		b->set_sensitive(false) ;
//	}
	if (flatMode)
		b->set_size_request(-1, 2);
	else
		b->set_size_request(-1, 22);


	Gtk::Button* b2 = Gtk::manage(new Gtk::Button());
	b2->set_image(a_imageNext);
//	if (!flatMode)
//	{
//		b2->set_size_request(-1, 22);
		b2->signal_clicked().connect(sigc::mem_fun(*this, &SegmentTrackWidget::onNextClicked));
		a_tooltips.set_tip(*b2, string(_("Allow repositioning cursor at the next "))+p_label+string(_(" boundary.")));
//	}
//	else
//	{
//		b2->set_size_request(-1, 2);
//		b2->set_sensitive(false) ;
//	}

	if (flatMode)
		b2->set_size_request(-1, 2);
	else
		b2->set_size_request(-1, 22);

	b->set_focus_on_click(false);
	b->show();
	pack_start(*b, false, false, 1);

	b2->set_focus_on_click(false);
	b2->show();
	pack_start(*b2, false, false, 1);


//	if (!flatMode)
//	{
		a_segmentEndFrame = Gtk::manage(new Gtk::Frame("On segment end"));
		Gtk::Label* labelSegmentEndFrame = (Gtk::Label*)a_segmentEndFrame->get_label_widget();
		labelSegmentEndFrame->set_markup("<small>On segment end</small>");
		a_segmentEndFrame->set_shadow_type(Gtk::SHADOW_IN);
		Gtk::HBox* hBoxSegmentEndFrame = Gtk::manage(new Gtk::HBox());

		a_labelDelay = Gtk::manage(new Gtk::Label());
		a_labelDelay->set_markup(MarkupSmall((string(_("Delay"))+" :").c_str()).c_str());
		hBoxSegmentEndFrame->pack_start(*a_labelDelay, false, false, 1);
		a_labelDelay->show();

		a_entryDelay = Gtk::manage(new Gtk::Entry());
		a_entryDelay->signal_key_release_event().connect(sigc::mem_fun(*this, &SegmentTrackWidget::onKeyPressed));

		a_entryDelay->set_width_chars(2);
		hBoxSegmentEndFrame->pack_start(*a_entryDelay, false, false, 1);
		a_entryDelay->show();
		Glib::ustring tool = _("Define a delay (in seconds) when reaching a segment end, before resuming playback.") ;
		a_tooltips.set_tip(*a_entryDelay, tool);

		a_stop = Gtk::manage(new Gtk::CheckButton("Stop"));
		a_stop->set_focus_on_click(false);
		a_stop->set_active(false);

		a_stop->signal_toggled().connect(sigc::mem_fun(*this, &SegmentTrackWidget::onStopClicked));
		Gtk::Label* labelStop = (Gtk::Label*)a_stop->get_child();
		labelStop->set_markup(MarkupSmall(_("Stop")).c_str());
		hBoxSegmentEndFrame->pack_start(*a_stop, false, false, 1);
		a_stop->show();
		a_tooltips.set_tip(*a_stop, _("Stop playback when reaching a segment end."));

		a_segmentEndFrame->add(*hBoxSegmentEndFrame);
		hBoxSegmentEndFrame->show();

		pack_start(*a_segmentEndFrame, false, false, 1);
		a_segmentEndFrame->hide();
//	}
//	else
//	{
//		//TODO enable all feature in flat mode ?
//		a_segmentEndFrame = NULL ;
//		a_labelDelay = NULL ;
//		a_entryDelay = NULL ;
//		a_stop = NULL ;
//	}

	// Expand button
	Gtk::VBox* expandBox = Gtk::manage(new Gtk::VBox());

	a_expandButton.set_image(1, ICO_EXPAND_LEFT, SMALL_TOGGLE_BUTTON_SIZE) ;
	a_expandButton.set_image(2, ICO_EXPAND_LEFT_OVER, SMALL_TOGGLE_BUTTON_SIZE) ;
	a_expandButton.set_image(3, ICO_EXPAND_LEFT_PRESS, SMALL_TOGGLE_BUTTON_SIZE) ;
	a_expandButton.show();

//	if (!flatMode)
//	{
		a_expandButton.signal_button_release_event().connect(sigc::mem_fun(*this, &SegmentTrackWidget::onExpandClicked));
		a_tooltips.set_tip(a_expandButton, _("Expand or collapse the \"on segment end\" frame."));
//	}
//	else
//		a_expandButton.set_sensitive(false) ;

	expandBox->pack_start(a_expandButton, true, false, 0);
	pack_start(*expandBox, false, false, 0);
	expandBox->show();
}


// --- ~SegmentTrackWidget ---
SegmentTrackWidget::~SegmentTrackWidget()
{
	delete a_segment;
}


// --- EndSegment ---
void SegmentTrackWidget::endSegment(float p_secs, float p_size, float& p_delay, float& p_endSeg)
{
	float endSeg = -1.0;

	if (a_segment->endSegment(p_secs, p_size, endSeg))
	{
		if (a_stop && a_stop->get_active())
		{
			p_delay = -1.0;
			p_endSeg = endSeg;
			return;
		}

		p_delay = a_delay;
		p_endSeg = endSeg;

	}
	else
	{
		p_delay = 0.0;
	}
}


// --- OnStopClicked ---
void SegmentTrackWidget::onStopClicked()
{
	bool b = (a_stop && a_stop->get_active()) ;
	if (a_labelDelay)
		a_labelDelay->set_sensitive(!b);
	if (a_entryDelay)
		a_entryDelay->set_sensitive(!b);
}


// --- OnExpandClicked ---
bool SegmentTrackWidget::onExpandClicked(GdkEventButton* event)
{
	if (a_expanded)
	{
		if (a_segmentEndFrame)
			a_segmentEndFrame->hide();
		a_expandButton.set_image(1, ICO_EXPAND_LEFT, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(2, ICO_EXPAND_LEFT_OVER, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(3, ICO_EXPAND_LEFT_PRESS, SMALL_TOGGLE_BUTTON_SIZE) ;
	}
	else
	{
		if (a_segmentEndFrame)
			a_segmentEndFrame->show();
		a_expandButton.set_image(1, ICO_EXPAND_RIGHT, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(2, ICO_EXPAND_RIGHT_OVER, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(3, ICO_EXPAND_RIGHT_PRESS, SMALL_TOGGLE_BUTTON_SIZE) ;
	}
	a_expanded = !a_expanded;
	return false ;
}


// --- OnEntryChanged ---
bool SegmentTrackWidget::onKeyPressed(GdkEventKey*)
{
	Glib::ustring str ;
	if (a_entryDelay)
	{
		str = a_entryDelay->get_text();
		a_delay = my_atof( str.c_str() );
	}

	return true;
}


// --- ResetActivateButtonSize ---
void SegmentTrackWidget::resetActivateButtonSize(int i_width)
{
	if ( i_width != a_startGap.get_width() && i_width > 1)
		a_startGap.set_size_request(i_width, -1);
}

void SegmentTrackWidget::setSelectable(bool p_selectable)
{
	if (a_segment)
		a_segment->setSelectable(p_selectable);
}

} // namespace
