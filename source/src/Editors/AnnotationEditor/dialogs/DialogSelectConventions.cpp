/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class DialogSelectConventions
 *
 * DialogSelectConventions...
 */

#include "DialogSelectConventions.h"
#include "Common/globals.h"

namespace tag {

DialogSelectConventions::DialogSelectConventions(Gtk::Window& p_parent, string& p_title, string& p_lang, string& p_conventions,
													vector<string>& p_lang_list, vector<string>& p_conv_list, string& forced_convention)
: Gtk::Dialog(p_title.c_str(), p_parent, true, true), a_lang(p_lang), a_conventions(p_conventions), a_forced_convention(forced_convention)
{

	Gtk::HBox* hBox = Gtk::manage(new Gtk::HBox());
	get_vbox()->pack_start(*hBox, false, false, 15);
	hBox->show();

	Gtk::VBox* vBox = Gtk::manage(new Gtk::VBox());
	hBox->pack_start(*vBox, false, false, 15);
	vBox->show();

	Gtk::HBox* hBox1 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* hBox2 = Gtk::manage(new Gtk::HBox());
	vBox->pack_start(*hBox1, false, false, 3);
	vBox->pack_start(*hBox2, false, false, 3);
	hBox1->show();
	hBox2->show();

	Gtk::Label* type = Gtk::manage(new Gtk::Label(string(_("Transcription language"))+" :"));
	type->set_text(type->get_text()+"                             ");
	a_langEntry = Gtk::manage(new Gtk::ComboBoxText());
	Gtk::Label* desc = Gtk::manage(new Gtk::Label(string(_("Transcription conventions"))+" :"));
	desc->set_text(desc->get_text()+"                             ");
	a_convEntry = Gtk::manage(new Gtk::ComboBoxText());

	vector<string>::iterator it1 = p_lang_list.begin();
	while (it1 != p_lang_list.end()) {
		a_langEntry->append_text(*it1);
		it1++;
	}
	a_langEntry->set_active_text(p_lang);

	vector<string>::iterator it2 = p_conv_list.begin();
	while (it2 != p_conv_list.end()) {
		a_convEntry->append_text(*it2);
		it2++;
	}
	a_convEntry->set_active_text(p_conventions);

	hBox1->pack_start(*type, false, false, 3);
	hBox1->pack_start(*a_langEntry, false, false, 3);
	hBox2->pack_start(*desc, false, false, 3);
	hBox2->pack_start(*a_convEntry, false, false, 3);

	type->show();
	a_langEntry->show();
	desc->show();
	a_convEntry->show();

	type->set_size_request(180, -1);
	desc->set_size_request(180, -1);
	a_langEntry->set_size_request(250, -1);
	a_convEntry->set_size_request(250, -1);

	if (!forced_convention.empty()) {
		a_convEntry->append_text(forced_convention) ;
		a_convEntry->set_active_text(forced_convention) ;
		a_convEntry->set_sensitive(false) ;
	}

	Gtk::Button* ok = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	Gtk::Button* cancel = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

	ok->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &DialogSelectConventions::onButtonClicked), Gtk::RESPONSE_OK));
	cancel->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &DialogSelectConventions::onButtonClicked), Gtk::RESPONSE_CANCEL));
	set_default_response(Gtk::RESPONSE_OK);
	set_focus(*ok);
}

DialogSelectConventions::~DialogSelectConventions() {

}

void DialogSelectConventions::onButtonClicked(int p_id) {

	response(p_id);
	hide();

	if (p_id == Gtk::RESPONSE_OK) {
		a_lang = a_langEntry->get_active_text();
		if (a_forced_convention.empty())
			a_conventions = a_convEntry->get_active_text();
		else
			a_conventions = a_forced_convention ;
	}
}

}
