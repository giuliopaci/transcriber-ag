/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "Editors/AnnotationEditor/dialogs/SectionPropertiesDialog.h"
#include "Editors/AnnotationEditor/dialogs/TopicsDialog.h"


#include "Common/globals.h"
#include "Common/Dialogs.h"
#include "Common/util/StringOps.h"
#include "Common/util/Utils.h"
#include "Common/icons/Icons.h"

namespace tag {

//******************************************************************************
//********************************** CONSTRUCT *********************************
//******************************************************************************

SectionPropertiesDialog::SectionPropertiesDialog(Gtk::Window& p_win, DataModel& p_m_dataModel, const string& p_m_elementId, bool p_editable)
: AnnotationPropertiesDialog(p_win, p_m_dataModel, p_m_elementId, p_editable)
{
	with_topic = false ;
	m_iLang = InputLanguageHandler::defaultInputLanguage();

	bool ok = prepare_data() ;
	if (ok)
		prepare_gui() ;
	else
		display_error() ;
}

//******************************************************************************
//********************************** GET VALUES ********************************
//******************************************************************************

bool SectionPropertiesDialog::prepare_data()
{
	if (!m_dataModel.existsElement(m_elementId))
		return false ;

	a_desc = m_dataModel.getElementProperty(m_elementId, "desc", "");
	a_desc_orig = a_desc ;
	a_topic = m_dataModel.getElementProperty(m_elementId, "topic", "");


	a_type = m_dataModel.getElementProperty(m_elementId, "type", "");
	m_type = a_type;

	// title
	title = _("Section properties");
	with_topic = true ;

	// get all subtypes
	string subtypes = m_dataModel.conventions().getConfiguration("section,subtypes") ;
	StringOps sub(subtypes) ;
	sub.split(subtypes_list, ";,") ;

	// get theirs label
	string label ;
	for (guint i = 0; i < subtypes_list.size(); ++i ) {
		label = m_dataModel.conventions().getLocalizedLabel(subtypes_list[i]) ;
		subtypes_label.push_back(label);
	}

	// get label for current type
	a_type = m_dataModel.conventions().getLocalizedLabel(a_type);
//	m_type = a_type;

	// description is free, do nothing with label
	desc_list.push_back(a_desc);

	// get all topic
	topic_list = m_dataModel.conventions().getTopics() ;

	set_title(title);


	return true ;
}

//******************************************************************************
//************************************* GUI ************************************
//******************************************************************************

void SectionPropertiesDialog::display_error()
{
	Gtk::VBox* vBox = get_vbox();
	Gtk::Alignment* align = Gtk::manage(new Gtk::Alignment()) ;
	Glib::ustring msg = _("Unable to find element") ;
	msg = "    " + msg + "    " ;
	Gtk::Label* label = Gtk::manage(new Gtk::Label(msg)) ;
	vBox->pack_start(*align, false, false, 5) ;
	align->add(*label) ;
	align->set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
	Icons::set_window_icon(this, ICO_ERROR, 11) ;
	Gtk::Button* close = add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
	close->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &SectionPropertiesDialog::onButtonClicked), Gtk::RESPONSE_CLOSE));

	vBox->show_all_children(true) ;
}

void SectionPropertiesDialog::prepare_gui()
{
	Gtk::VBox* vBox = get_vbox();

	//> pack combo language
	vBox->pack_start(hbox_language, false, false,3);
		hbox_language.pack_start(label_inputLanguage, false, false, 3);
		hbox_language.pack_start(combo_language, false, false, 3);
		label_inputLanguage.set_label(_("Input language")) ;
	vBox->pack_start(sep_language, false, false, 3);
	hbox_language.show() ;
	set_combo_language() ;
	combo_language.signal_changed().connect(sigc::mem_fun(*this, &SectionPropertiesDialog::on_change_language));
	combo_language.set_sensitive(m_editable);

	//> pack type combo-entry
	Gtk::HBox* hBox1 = Gtk::manage(new Gtk::HBox());
	Gtk::Label* type = Gtk::manage(new Gtk::Label(string(_("Type"))+" :"));
	a_typeEntry = Gtk::manage(new Gtk::ComboBoxText());
	vBox->pack_start(*hBox1, false, false, 15);
	hBox1->pack_start(*type, false, false, 3);
	hBox1->pack_start(*a_typeEntry, false, false, 3);
	hBox1->show();

	//> prepare type choices
	vector<string>::iterator it1 = subtypes_label.begin();
	bool label_found=false;
	while (it1 != subtypes_label.end()) {
		a_typeEntry->append_text(*it1);
		if ( a_type == *it1 ) label_found = true;
		it1++;
	}
	if ( ! label_found ) {
		// label not found in conventions -> still add it to list
		a_typeEntry->append_text(a_type);
	}
	a_typeEntry->set_active_text(a_type);
	a_typeEntry->set_sensitive(m_editable);

	//> pack topic
	if (with_topic)
	{
		vBox->pack_start(topic_hbox, false, false, 15);
		topic_hbox.pack_start(topic_label, false, false, 3) ;
		topic_hbox.pack_start(topic_entry, false, false, 3) ;
		topic_hbox.pack_start(topic_button, false, false, 5) ;
		topic_hbox.pack_start(topic_no_button, false, false) ;
		topic_label.set_label(_("Topic : ")) ;
		topic_button.set_label(_("_Change")) ;
		topic_no_button.set_label(_("N_o topic")) ;
		topic_button.set_use_underline(true) ;
		topic_no_button.set_use_underline(true) ;
		topic_button.signal_clicked().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SectionPropertiesDialog::on_topic_change), "change")) ;
		topic_no_button.signal_clicked().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SectionPropertiesDialog::on_topic_change), "clear")) ;
		topic_hbox.show_all_children() ;

		//> actualise current value
		Glib::ustring label ;
		if (a_topic.compare("")==0)
			label = TAG_TOPIC_NULL_LABEL ;
		else
			label = Topics::getTopicLabel(a_topic, topic_list) ;
		if (label.compare(TAG_TOPIC_NULL_LABEL)!=0)
			topic_no_button.set_sensitive(true) ;
		else
			topic_no_button.set_sensitive(false) ;
		topic_entry.set_text(label) ;
		topic_entry.set_has_frame(true) ;
		topic_entry.set_sensitive(false);

		// if not editable
		if ( ! m_editable )
		{
			topic_button.set_sensitive(false);
			topic_no_button.set_sensitive(false);
		}
	}

	//> pack combo description
	Gtk::HBox* hBox2 = Gtk::manage(new Gtk::HBox());
	Gtk::Label* desc = Gtk::manage(new Gtk::Label(string(_("Description"))+" :"));
	a_descEntry = new ComboEntry_mod() ;
	vBox->pack_start(*hBox2, false, false, 15);
		hBox2->pack_start(*desc, false, false, 3);
		hBox2->pack_start(hbox_entry, false, false, 3);
		hBox2->show();
			hbox_entry.pack_start(*a_descEntry, false, false, 3);
	hbox_entry.set_sensitive(m_editable);
	a_descEntry->set_input_language(m_iLang) ;
	a_descEntry->set_text(a_desc);

	// add all description available, checking if they can be edited
	vector<string>::iterator it2 = desc_label.begin();
	while (it2 != desc_label.end()) {
		string s2 = *it2;
		a_descEntry->add_in_list(s2);
		it2++;
	}

	a_descEntry->set_sensitive(m_editable) ;

	type->show();
	a_typeEntry->show();
	desc->show();
	a_descEntry->show();

	a_descEntry->set_size_request(400, -1) ;
	a_descEntry->grab_focus() ;

	Gtk::Button* ok;

	if ( m_editable)
	{
		ok = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
		Gtk::Button* cancel = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

		cancel->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &SectionPropertiesDialog::onButtonClicked), Gtk::RESPONSE_CANCEL));
	}
	else
		ok = add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_OK);

	set_default_response(Gtk::RESPONSE_OK);
	ok->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &SectionPropertiesDialog::onButtonClicked), Gtk::RESPONSE_OK));

	vBox->show_all_children() ;
//	a_typeEntry->grab_focus();
}

SectionPropertiesDialog::~SectionPropertiesDialog()
{
	if (a_descEntry)
		delete(a_descEntry) ;
}


//******************************************************************************
//********************************** CALLBACK **********************************
//******************************************************************************

void SectionPropertiesDialog::onButtonClicked(int p_id)
{
	if (p_id == Gtk::RESPONSE_OK && m_editable ) {
		prepareResultValues() ;
		// set result in model
		m_dataModel.setElementProperty(m_elementId, "type", get_type(), false);
		m_dataModel.setElementProperty(m_elementId, "desc", get_desc(), true);
//		qnorm = dlg->get_norm() ; ??
		if (get_topic().compare("")==0)
			m_dataModel.deleteElementProperty(m_elementId, "topic");
		else
			m_dataModel.setElementProperty(m_elementId, "topic", get_topic(), true);
		if (get_desc().compare("")==0)
			m_dataModel.deleteElementProperty(m_elementId, "desc");
		else
			m_dataModel.setElementProperty(m_elementId, "desc", get_desc(), true);
	}

	hide();
	response(m_editable ? p_id : Gtk::RESPONSE_CANCEL);
}


void SectionPropertiesDialog::prepareResultValues()
{
	//> get current values (caution: correspond to label)
	a_type = a_typeEntry->get_active_text();
	a_desc = a_descEntry->get_text();

	int i ;

	for (i = 0; i < subtypes_label.size() && ( subtypes_label[i] != a_type ) ; ++i );
	if ( i < subtypes_label.size() )
		m_type = subtypes_list[i];

	//> topic : do nothing, always actualized when combo changes
	// TODO should be the same with other features
}


void SectionPropertiesDialog::set_combo_language()
{
	combo_language.clear() ;
	std::map<std::string, tag::InputLanguage*>::iterator ite;
	 for(ite = InputLanguageHandler::get_first_language_iter();
	 	ite != InputLanguageHandler::get_last_language_iter();
	 	ite++)
	 {
		 if ((*ite).second->isActivated())
			 combo_language.append_text((*ite).second->getLanguageDisplay());
	 }
	if(m_iLang == NULL)
		combo_language.set_active(0);
	else
		combo_language.set_active_text(m_iLang->getLanguageDisplay());
	 combo_language.set_focus_on_click(false) ;
}

void SectionPropertiesDialog::on_change_language()
{
	Glib::ustring display = combo_language.get_active_text() ;
	if (display != "")
	{
		//> actualize input language for entry
		m_iLang = InputLanguageHandler::get_input_language_by_name(display);
		a_descEntry->set_input_language(m_iLang) ;

		//> check if an external IME is in used
		// and set corresponding mode (swith will be processed at focus event)
		if (m_iLang && m_iLang->getLanguageType().compare(IME_LANGUAGE)==0)
			a_descEntry->setIMEstatus(true) ;
		else
			a_descEntry->setIMEstatus(false) ;
	}
	else
		TRACE_D << "SectionPropertiesDialog::on_change_language NULL " << std::endl ;
}

void SectionPropertiesDialog::on_topic_change(Glib::ustring mode)
{
	if (mode.compare("change")==0)
	{
		Glib::ustring id, label ;
		TopicsDialog dialog(topic_list, this) ;
		int res = dialog.run() ;
		if (res==Gtk::RESPONSE_APPLY) {
			Glib::ustring id, label ;
			bool ok = dialog.get_chosen(id, label) ;
			if (ok) {
				a_topic = id ;
				topic_entry.set_text(label) ;
				if (label.compare(TAG_TOPIC_NULL_LABEL)!=0)
					topic_no_button.set_sensitive(true) ;
			}
			else
				dlg::error(_("error while choosing topic"), this) ;
		}
	}
	else if (mode.compare("clear")==0)
	{
		topic_no_button.set_sensitive(false) ;
		a_topic = "" ;
		topic_entry.set_text(TAG_TOPIC_NULL_LABEL) ;
	}
}


}
