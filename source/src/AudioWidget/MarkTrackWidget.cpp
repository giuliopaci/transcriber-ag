/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class MarkTrackWidget
 *
 * MarkTrackWidget...
 */

#include "AudioWidget.h"
#include <gtk/gtkstyle.h>

namespace tag {

MarkTrackWidget::MarkTrackWidget(float p_duration, int nbtrack) : TrackWidget(), a_iconSize(1), a_imagePrevious(Gtk::Stock::MEDIA_PREVIOUS, a_iconSize), a_imageNext(Gtk::Stock::MEDIA_NEXT, a_iconSize) {

	set_no_show_all(true);

	if ( nbtrack > 1 ) {
		Gtk::HBox* vide = Gtk::manage(new Gtk::HBox());
		pack_start(*vide, false, false, 2);
		vide->show();
		vide->set_size_request(30, -1);
	}

	a_mark = new MarkWidget(p_duration);
	pack_start(*a_mark, true, true, 2);
	a_mark->show();

	Gtk::Button* b = Gtk::manage(new Gtk::Button());
	b->set_focus_on_click(false);
	b->set_image(a_imagePrevious);
	b->signal_clicked().connect(sigc::mem_fun(*this, &MarkTrackWidget::onPreviousClicked));
	pack_start(*b, false, false, 2);
	b->show();
	a_tooltips.set_tip(*b, string(_("Allow repositionning cursor at the previous mark.")));
	b->set_size_request(-1, 22);

	Gtk::Button* b2 = Gtk::manage(new Gtk::Button());
	b2->set_focus_on_click(false);
	b2->set_image(a_imageNext);
	b2->signal_clicked().connect(sigc::mem_fun(*this, &MarkTrackWidget::onNextClicked));
	pack_start(*b2, false, false, 2);
	b2->show();
	a_tooltips.set_tip(*b2, string(_("Allow repositionning cursor at the next mark.")));
	b2->set_size_request(-1, 22);

	Gtk::HBox* vide2 = Gtk::manage(new Gtk::HBox());
	pack_start(*vide2, false, false, 2);
	vide2->show();
	vide2->set_size_request(15, -1);

}

MarkTrackWidget::~MarkTrackWidget() {

	delete a_mark;

}

bool MarkTrackWidget::onKeyPressed(GdkEventKey* p_event) {
	return true;
}

} // namespace
