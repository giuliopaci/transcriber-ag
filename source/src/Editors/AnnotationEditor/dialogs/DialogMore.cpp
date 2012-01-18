/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "DialogMore.h"

namespace tag {

DialogMore::DialogMore(Gtk::Window& p_parent, VersionList* p_versionList, bool p_displayAnnotationTime, bool editable)
	: Gtk::Dialog(_("Version history"), p_parent, true, true)
{

	a_versionList = p_versionList;

	a_dateEntry = new FieldEntry*[a_versionList->size()];
	a_id = new Gtk::Entry*[a_versionList->size()];
	a_by = new Gtk::Entry*[a_versionList->size()];
	a_time = new Gtk::Entry*[a_versionList->size()];
	a_tbuffer = new Glib::RefPtr<Gtk::TextBuffer>[a_versionList->size()];
	int i = 0;

	VersionList::iterator itA = a_versionList->begin();
	while (itA != a_versionList->end()) {

		Version& v = *itA;
		Gtk::VBox* vb = Gtk::manage(new Gtk::VBox());
		Gtk::HBox* h = Gtk::manage(new Gtk::HBox());
		vb->pack_start(*h, true, true);
		h->show();

		Gtk::Label* idLabel = Gtk::manage(new Gtk::Label(string(_("No"))+" :"));
		a_id[i] = Gtk::manage(new Gtk::Entry());
		a_id[i]->set_text(v.getId());
		a_id[i]->set_sensitive(false);
		h->pack_start(*idLabel, false, false, 3);
		h->pack_start(*a_id[i], false, false, 3);
		idLabel->show();
		a_id[i]->show();
		a_id[i]->set_editable(editable);
		a_id[i]->set_sensitive(editable) ;
		Gtk::Label* dateLabel = Gtk::manage(new Gtk::Label(string(_("Date"))+" :"));
		a_dateEntry[i] = new FieldEntry(3, "/", 2) ;
		displayDate(v.getDate(), a_dateEntry[i]);
//		a_dateEntry[i]->set_sensitive(false);
		h->pack_start(*dateLabel, false, false, 3);
		h->pack_start(*a_dateEntry[i], false, false, 3);
		dateLabel->show();
		a_dateEntry[i]->show();
		a_dateEntry[i]->set_editable(editable);
		a_dateEntry[i]->set_sensitive(editable);

		Gtk::Label* byLabel = Gtk::manage(new Gtk::Label(string(_("By"))+" :"));
		a_by[i] = Gtk::manage(new Gtk::Entry());
		a_by[i]->set_text(v.getAuthor());
		h->pack_start(*byLabel, false, false, 3);
		h->pack_start(*a_by[i], false, false, 3);
		byLabel->show();
		a_by[i]->show();
		a_by[i]->set_editable(editable) ;
		a_by[i]->set_sensitive(editable) ;

		if (p_displayAnnotationTime) {
			Gtk::Label* timeLabel = Gtk::manage(new Gtk::Label(string(_("Time"))+" :"));
			a_time[i] = Gtk::manage(new Gtk::Entry());
			a_time[i]->set_width_chars(16);
			a_time[i]->set_sensitive(false);
			string wid = v.getWid();
			int annotationTime = strtol(wid.c_str(), NULL, 16);
			char str[80];
			int M = (int)(annotationTime/60);
			float S = annotationTime - M*60;
			sprintf(str, "%d min %.3f sec", M, S);
			a_time[i]->set_text(str);
			h->pack_start(*timeLabel, false, false, 3);
			h->pack_start(*a_time[i], false, false, 3);
			timeLabel->show();
			a_time[i]->show();
			a_time[i]->set_editable(false) ;
			a_time[i]->set_sensitive(false) ;
		}


		h = Gtk::manage(new Gtk::HBox());
		h->show();
		vb->pack_start(*h, Gtk::PACK_EXPAND_WIDGET, 2);

		Gtk::Label* commentLabel = Gtk::manage(new Gtk::Label(string(_("Comment"))+" :"));
		a_tbuffer[i] = Gtk::TextBuffer::create();
		a_tbuffer[i]->set_text(v.getComment());
		Gtk::TextView* commentTV = Gtk::manage(new Gtk::TextView(a_tbuffer[i]));
		Gtk::Frame* commentEntry = Gtk::manage(new Gtk::Frame());
		commentEntry->set_shadow_type(Gtk::SHADOW_IN);
		commentEntry->add(*commentTV);
		commentTV->show();
		commentTV->set_editable(editable) ;
		commentTV->set_sensitive(editable) ;
		h->pack_start(*commentLabel, false, false, 3);
		h->pack_start(*commentEntry, true, true, 3);
		commentLabel->show();
		commentEntry->show();
		commentTV->set_size_request(250, -1);

		get_vbox()->pack_start(*vb, Gtk::PACK_EXPAND_WIDGET, 5);
		vb->show();
		itA++;
		i++;
	}
//////////////////
	Gtk::Button* ok = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	Gtk::Button* cancel = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

	ok->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &DialogMore::onButtonClicked), Gtk::RESPONSE_OK));
	cancel->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &DialogMore::onButtonClicked), Gtk::RESPONSE_CANCEL));

}

DialogMore::~DialogMore() {
	delete[] a_dateEntry;
	delete[] a_id;
	delete[] a_by;
	delete[] a_time;
	delete[] a_tbuffer;
}

void DialogMore::onButtonClicked(int p_id) {

	int i = 0;
	if (p_id == Gtk::RESPONSE_OK) {
		VersionList::iterator itA = a_versionList->begin();
		while (itA != a_versionList->end()) {
			(*itA).setId(a_id[i]->get_text());
			(*itA).setDate(getDate(a_dateEntry[i]));
			(*itA).setAuthor(a_by[i]->get_text());
			(*itA).setComment(a_tbuffer[i]->get_text());
			itA++;
			i++;
		}
	}

	response(p_id);
	hide();

}

void DialogMore::displayDate(const string& s, FieldEntry* entry)
{

	int yy,mm,dd;
	bool ok = (sscanf(s.c_str(), "%d/%d/%d", &yy, &mm, &dd) == 3);
	if ( ok ) {
		char str[4];
		if ( yy > 2000 ) yy -= 2000;
		else if ( yy > 1900 ) yy -= 1900;
		sprintf(str, "%02d", yy);
		entry->set_element(0, str);
		sprintf(str, "%02d", mm);
		entry->set_element(1, str);
		sprintf(str, "%02d", dd);
		entry->set_element(2, str);
	}
}

string DialogMore::getDate(FieldEntry* entry)
{
	int yy,mm,dd;
	string s =entry->get_element(0);
	if ( s.empty() ) return "";
	yy = atoi(s.c_str());
	if ( yy < 100 )
		if ( yy >= 90 ) yy += 1900;
		else yy += 2000;
	s =entry->get_element(1);
	mm = atoi(s.c_str());
	if ( mm < 1 || mm > 12 ) mm = 1;
	s =entry->get_element(2);
	dd = atoi(s.c_str());
	if ( dd < 1 || dd > 31 ) dd = 1;

	char buf[12];
	sprintf(buf, "%04d/%02d/%02d", yy,mm,dd);
	return buf;
}

} // end namespace
