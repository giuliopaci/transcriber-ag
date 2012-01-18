/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "Editors/AnnotationEditor/dialogs/QualifierPropertiesDialog.h"

#include "Common/globals.h"
#include "Common/Dialogs.h"
#include "Common/util/StringOps.h"
#include "Common/util/Utils.h"
#include "Common/icons/Icons.h"

#include "Common/InputLanguage.h"
#include "Common/InputLanguageHandler.h"

namespace tag {

//******************************************************************************
//********************************** CONSTRUCT *********************************
//******************************************************************************

QualifierPropertiesDialog::QualifierPropertiesDialog(Gtk::Window& p_win, DataModel& p_dataModel, const string& p_elementId, bool p_editable)
: AnnotationPropertiesDialog(p_win, p_dataModel, p_elementId, p_editable)
{
	a_normalize = false ;
	a_iLang = InputLanguageHandler::defaultInputLanguage();

	bool ok = prepare_data() ;
	if (ok)
		prepare_gui() ;
	else
		display_error() ;
}

void QualifierPropertiesDialog::configureMenuLabels(const std::map<string, string>& labels)
{
	a_menuLabels = labels;
	std::map<string, string>::iterator it ;
	string key = m_type + ",";
	for ( it = a_menuLabels.begin(); it != a_menuLabels.end(); ++it )
	{
		// type_label
		if (it->first.find(',') == string::npos)
		{
			string type_label;
			type_label = it->second;
			del_underscore(type_label);
			a_subtypeLabels.push_back(type_label);

			if ( a_type == it->first )
				a_type = type_label;
		}

		// a_descLabel
		else if ( it->first.compare(0, key.length(), key) == 0 )
		{
			string buf = it->second;
			del_underscore(buf);

			if ( ! buf.empty() )
			{
				a_descLabel.push_back(buf);
				buf = it->first.substr(key.length());
				a_descList.push_back(buf);
			}


			if (a_desc == buf)
				a_desc = a_descLabel.back();
		}
	}

	//> prepare type choices
	vector<string>::iterator it1 = a_subtypeLabels.begin();
	while (it1 != a_subtypeLabels.end()) {
		string s1 = *it1;
		a_typeEntry->append_text(s1);
		it1++;
	}

	a_lastDesc[a_type] = a_desc;
	a_typeEntry->set_active_text(a_type);
	a_typeEntry->set_sensitive(m_editable);
}

//******************************************************************************
//********************************** GET VALUES ********************************
//******************************************************************************

bool QualifierPropertiesDialog::prepare_data()
{
	if (!m_dataModel.existsElement(m_elementId))
		return false ;

	a_desc = m_dataModel.getElementProperty(m_elementId, "desc", "");
	a_desc_orig = a_desc ;
	a_norm = m_dataModel.getElementProperty(m_elementId, "norm", "");

		string buf;

		a_type = m_dataModel.getElementType(m_elementId);
		m_type = a_type;
		// prepare qualifier
		string title;
		if (m_dataModel.conventions().isQualifierClassType("entity", m_elementType) || m_elementType.compare("qualifier_entity")==0)
		{
			a_qualifierType = "qualifier_entity" ;
			title = _("Named entity properties");
			a_normalize = m_dataModel.conventions().typeCanBeNormalized(a_type) ;
		}
		// prepare event
		else
		{
			a_qualifierType = "qualifier_event" ;
			title = _("Event properties");
			a_normalize = false ;
		}

		// get all subtypes
		string subtypes = m_dataModel.conventions().getConfiguration("transcription_graph,"+a_qualifierType) ;
		StringOps sub(subtypes) ;
		sub.split(a_subtypeList, ";,") ;

		// get available type & desc labels
//		AnnotationRenderer renderer =

		// stock current label type
		a_type = m_dataModel.conventions().getLocalizedLabel(a_type);

	set_title(title);



	return true ;
}

//******************************************************************************
//************************************* GUI ************************************
//******************************************************************************

void QualifierPropertiesDialog::display_error()
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
	close->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &QualifierPropertiesDialog::onButtonClicked), Gtk::RESPONSE_CLOSE));

	vBox->show_all_children(true) ;
}

void QualifierPropertiesDialog::prepare_gui()
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
	combo_language.signal_changed().connect(sigc::mem_fun(*this, &QualifierPropertiesDialog::on_change_language));
	combo_language.set_sensitive(m_editable);

	//> pack type combo-entry
	Gtk::HBox* hBox1 = Gtk::manage(new Gtk::HBox());
	Gtk::Label* type = Gtk::manage(new Gtk::Label(string(_("Type"))+" :"));
	a_typeEntry = Gtk::manage(new Gtk::ComboBoxText());
	vBox->pack_start(*hBox1, false, false, 15);
	hBox1->pack_start(*type, false, false, 3);
	hBox1->pack_start(*a_typeEntry, false, false, 3);
	hBox1->show();

	a_typeEntry->signal_changed().connect(sigc::mem_fun(*this, &QualifierPropertiesDialog::on_type_change));

	//> pack combo description
	Gtk::HBox* hBox2 = Gtk::manage(new Gtk::HBox());
	Gtk::Label* desc = Gtk::manage(new Gtk::Label(string(_("Description"))+" :"));
	a_descEntry = new ComboEntry_mod() ;
	vBox->pack_start(*hBox2, false, false, 15);
		hBox2->pack_start(*desc, false, false, 3);
		hBox2->pack_start(hbox_entry, false, false, 3);
		hBox2->show();
			hbox_entry.pack_start(*a_descEntry, false, false, 3);
			hbox_entry.pack_start( *( a_descEntry->get_arrow() ), false, false, 3);
	hbox_entry.set_sensitive(m_editable);
	a_descEntry->set_input_language(a_iLang) ;
	a_descEntry->set_text(a_desc);

	// add all description available, checking if they can be edited
	vector<string>::iterator it2 = a_descLabel.begin();
	while (it2 != a_descLabel.end()) {
		string s2 = *it2;
		a_descEntry->add_in_list(s2);
		it2++;
	}

	//> check if desc can be edited as user wants
	/* -> qualifier: depends on "editable" value in convention fill
	 * -> mainstream : never editable
	 */
	bool freeDesc = m_dataModel.conventions().typeCanBeEdited(m_type) ;
	a_descEntry->set_sensitive(freeDesc) ;

	//> pack entry normalization
	if (a_normalize) {
		vBox->pack_start(normalization_hbox, false, false, 15);
			normalization_hbox.pack_start(normalization_label, false, false, 3) ;
			normalization_hbox.pack_start(normalization_entry, false, false, 3) ;
		normalization_label.set_label(_("Normalisation : ")) ;
		normalization_entry.set_text(a_norm) ;
		normalization_hbox.show_all_children() ;
	}

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

		cancel->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &QualifierPropertiesDialog::onButtonClicked), Gtk::RESPONSE_CANCEL));
	}
	else
		ok = add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_OK);

	set_default_response(Gtk::RESPONSE_OK);
	ok->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &QualifierPropertiesDialog::onButtonClicked), Gtk::RESPONSE_OK));

	vBox->show_all_children() ;
//	a_typeEntry->grab_focus();
}

QualifierPropertiesDialog::~QualifierPropertiesDialog()
{
	if (a_descEntry)
		delete(a_descEntry) ;
}


//******************************************************************************
//********************************** CALLBACK **********************************
//******************************************************************************

void QualifierPropertiesDialog::onButtonClicked(int p_id)
{
	if (p_id == Gtk::RESPONSE_OK && m_editable ) {
		prepareResultValues() ;
		if (!get_norm().empty())
			m_dataModel.setQualifier(m_elementId, get_type(), get_desc(), get_norm(), true);
		else
			m_dataModel.setQualifier(m_elementId, get_type(), get_desc(), true);
	}

	hide();
	response(m_editable ? p_id : Gtk::RESPONSE_CANCEL);
}


void QualifierPropertiesDialog::prepareResultValues()
{
	//> get current values (caution: correspond to label)
	a_type = a_typeEntry->get_active_text();
	a_desc = a_descEntry->get_text();

	int i ;

		// tmp bug: don't allow empty desc for qualifiers
		if (a_desc.empty())
			a_desc = " " ;
		for (i = 0; i < a_subtypeLabels.size() && ( a_subtypeList[i].substr(0,m_type.length()) != m_type ) ; ++i );


	if ( i < a_subtypeLabels.size() )
		a_type = a_subtypeList[i];
	else
		a_type = m_elementType;


		for ( i = 0; i < a_descList.size() && (a_descLabel[i] != a_desc) ; ++i);
		if ( i < a_descLabel.size() )
			a_desc = a_descList[i];

	//> normalization

	if (a_normalize)
		a_norm =  normalization_entry.get_text() ;

}

void QualifierPropertiesDialog::set_combo_language()
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
	if(a_iLang == NULL)
		combo_language.set_active(0);
	else
		combo_language.set_active_text(a_iLang->getLanguageDisplay());
	 combo_language.set_focus_on_click(false) ;
}

void QualifierPropertiesDialog::on_change_language()
{
	Glib::ustring display = combo_language.get_active_text() ;
	if (display != "")
	{
		//> actualize input language for entry
		a_iLang = InputLanguageHandler::get_input_language_by_name(display);
		a_descEntry->set_input_language(a_iLang) ;

		//> check if an external IME is in used
		// and set corresponding mode (swith will be processed at focus event)
		if (a_iLang && a_iLang->getLanguageType().compare(IME_LANGUAGE)==0)
			a_descEntry->setIMEstatus(true) ;
		else
			a_descEntry->setIMEstatus(false) ;
	}
	else
		TRACE_D << "QualifierPropertiesDialog::on_change_language NULL " << std::endl ;
}


void QualifierPropertiesDialog::on_type_change()
{
	//> Only for qualifier we need to actualize available desc when
	//  type changes.
	//  Otherwise, do nothing
	if ( m_elementType.compare(0, 9, "qualifier") != 0 &&
			!m_dataModel.isQualifierType(m_elementType) )
		return ;


	//> Get current values (caution: correspond to label)
	a_type = a_typeEntry->get_active_text();

	// get available type & desc labels

	std::map<string, string>::const_iterator it ;

	for ( it = a_menuLabels.begin(); it != a_menuLabels.end(); ++it )
	{
		// type_label
		if (it->first.find(',') == string::npos)
		{
			string type_label;
			type_label = it->second;
			del_underscore(type_label);
			a_subtypeLabels.push_back(type_label);

			string type_sec;
			type_sec = it->second;
			del_underscore(type_sec);
		// CREATE DEL UNDERSCORE		 !!!!!!!!

			if ( a_type == type_sec )
				m_type = it->first;
		}
	}


	//> Get corresponding desc labels
	string buf;
	a_descLabel.clear() ;
	a_descList.clear() ;
	string key = m_type + ",";
	for ( it = a_menuLabels.begin(); it != a_menuLabels.end(); ++it )
	{
		if ( it->first.compare(0, key.length(), key) == 0 )
		{
			buf = it->second;
			del_underscore(buf);

			if ( (buf != a_type) && (! buf.empty()) )
			{
				a_descLabel.push_back(buf);
				buf = it->first.substr(key.length());
				a_descList.push_back(buf);
			}
		}
	}

	//> Reset desc combo
	a_descEntry->clearMenu() ;
	vector<string>::iterator it2 = a_descLabel.begin();
	while (it2 != a_descLabel.end()) {
		string s2 = *it2;
		a_descEntry->add_in_list(s2);
		it2++;
	}

	//TODO keep for all type the lastly selected desc, and select it when a type is changed
	a_desc = a_lastDesc[a_type];
	if ( a_desc.empty() && (a_descLabel.size() > 0) && ( ! a_descLabel.front().empty()) )
		a_desc = a_descLabel.front() ;

	a_descEntry->set_text(a_desc) ;
	a_lastDesc[a_type] = a_desc;

	for ( it = a_menuLabels.begin(); it != a_menuLabels.end(); ++it )
	{
		if ( it->first.compare(0, key.length(), key) == 0 )
			del_underscore(buf);
	}

//	a_descEntry->set_sensitive(m_dataModel.conventions().typeCanBeEdited(m_type)) ;
	bool freeDesc = m_dataModel.conventions().typeCanBeEdited(m_type) ;
	a_descEntry->set_sensitive(freeDesc) ;
}


void QualifierPropertiesDialog::del_underscore (string &st)
{
	unsigned long pos = st.find('_');
	if (pos != string::npos)
		st.erase(pos, 1);
}

}
