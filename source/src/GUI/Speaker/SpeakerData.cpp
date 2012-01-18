/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Explorer_dialog.h"
#include "Explorer_utils.h"
#include "SpeakerData.h"
#include "Common/globals.h"
#include "Common/icons/Icons.h"
#include "Common/util/StringOps.h"
#include <iostream>
#include <gtk/gtk.h>

#define TAG_SPEAKERDATA_MINIMAL_W 


namespace tag {

SpeakerData::SpeakerData(Gtk::Window* _parent, Speaker* speak, Languages* lang, bool is_new_speaker, bool edition, bool from_global)  
{
	parent = _parent ;
	
	from_global_dico = from_global ;
	
	table = NULL ;
	
	//set_use_underline(true) ;
	set_label(_("Speakers data"));
	set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
	
	adding_mode = false ;
	hasChanged = false ;
	lock_signals = false; 
	
	//pack_start(hbox_id) ;
	table = new Gtk::Table(5, 2, true) ;
	table->set_col_spacings(15) ;
	table->set_row_spacings(10) ;

	//first columns
	align_lab_firstName.add(label_firstName) ;
	align_lab_firstName.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0, 15) ;
	align_lab_lastName.add(label_lastName) ;
	align_lab_lastName.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0, 15) ;
	align_lab_gender.add(label_gender) ;
	align_lab_gender.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0, 15) ;

	align_lab_language.add(label_language) ;
	align_lab_language.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0, 15) ;

	align_lab_description.add(label_description) ;
	align_lab_description.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0, 15) ;

	table->attach(align_lab_firstName, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK , 0, 15) ;
	table->attach(align_lab_lastName, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK , 0, 15) ;
	table->attach(align_lab_gender, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK , 0, 15) ;
	table->attach(align_lab_language, 0, 1, 3, 4, Gtk::FILL, Gtk::SHRINK , 0, 15) ;
	table->attach(align_lab_description, 0, 1, 4, 5, Gtk::FILL, Gtk::SHRINK , 0, 15) ;

	//second column
	align_firstName.add(entry_firstName) ;
	align_firstName.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0, 15) ;
	align_lastName.add(entry_lastName) ;
	align_lastName.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0, 15) ;
	align_gender.add(combo_gender) ;
	align_gender.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0, 15) ;

	table->attach(align_firstName, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK , 0, 15) ;
	table->attach(align_lastName, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK , 0, 15) ;
	table->attach(align_gender, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK , 0, 15) ;
	table->attach(align_language, 1, 2, 3, 4, Gtk::FILL, Gtk::FILL , 0, 15) ;
	table->attach(entry_description, 1, 2, 4, 5, Gtk::FILL, Gtk::FILL , 0, 15) ;

	table->set_homogeneous(false) ;
	
	//align_language.add(scrollW);
	align_language.add(language_box) ;
	language_box.pack_start(language_list, true, true);
	language_box.pack_start(language_button_align, false, false) ;
	language_button_align.add(language_button_box) ;
	language_button_box.pack_start(language_button_add, false, false) ;
	language_button_box.pack_start(language_button_remove, false, false) ;
	language_button_align.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 15) ;	
	
	add(align_general) ;
	align_general.add(general_hbox) ;
	general_hbox.pack_start(blank_left, false, false) ;
	general_hbox.pack_start(vbox, true, true) ;
	general_hbox.pack_start(blank_right, false, false) ;
		blank_left.set_label("   ") ;
		blank_right.set_label("   ") ;
	vbox.pack_start(*table, true, true); 
	vbox.pack_start(sep, false, false) ;
	vbox.pack_start(align, false, false) ;
		align.add(hbox_button) ;
			//hbox_button.pack_start(button_edit, false, true) ;
			//hbox_button.pack_start(button_cancel, false, true) ;
			hbox_button.pack_start(button_validate, false, true) ;
	vbox.pack_start(blank, false, false) ;
		blank.set_label(" ") ;

	align.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	align_general.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;

	language_button_add.set_icon(ICO_SPEAKER_ADDLANGUAGE, _("Add language"), 12, _("add a new language")) ;
	language_button_add.set_relief(Gtk::RELIEF_NONE) ;
	language_button_remove.set_relief(Gtk::RELIEF_NONE) ;
	language_button_remove.set_icon(ICO_SPEAKER_REMOVELANGUAGE, _("Remove language"), 12, _("remove the selected language")) ;
	language_button_add.signal_clicked().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SpeakerData::addRemove_language), "add")) ;
	language_button_remove.signal_clicked().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SpeakerData::addRemove_language), "remove")) ;
	
	button_validate.set_use_underline(true) ;
	button_validate.set_label(_("_Validate")) ;
	button_validate.signal_clicked().connect( sigc::mem_fun(*this, &SpeakerData::on_validate) ) ; 

	set_labels() ;
	set_combo() ;
	show_all_children() ;

	entry_description.signal_changed().connect( sigc::mem_fun(*this, &SpeakerData::on_entries_changed) ) ;
	entry_firstName.signal_changed().connect( sigc::mem_fun(*this, &SpeakerData::on_entries_changed) ) ;
	entry_lastName.signal_changed().connect( sigc::mem_fun(*this, &SpeakerData::on_entries_changed) ) ;
	combo_gender.signal_changed().connect( sigc::mem_fun(*this, &SpeakerData::on_entries_changed) ) ;

	//> get datas
	speaker = speak ;
	languages = Languages::getInstance() ;
	prepare_list() ;
	actualise_datas(is_new_speaker) ;

	is_editable = edition ;
	edition_mode(edition) ;
	button_validate.set_sensitive(false);
	if (languages==NULL) {
		language_button_add.set_sensitive(false) ;
		language_button_remove.set_sensitive(false) ;
		list_editable(false) ;
	}
	
	hasChanged = false ;
}

SpeakerData::~SpeakerData()
{
	if (table)
		delete(table) ;
}

void SpeakerData::set_labels()
{
	Glib::ustring name ;
	Glib::ustring space = " " ;
	Glib::ustring display ;

	name = _("Language") ;
	display = name + space ;
	label_language.set_label(display) ;
	label_language.set_name("speakerDataLabel") ;

	name = _("First Name") ;
	display = name + space ;
	label_firstName.set_label(display) ;
	label_firstName.set_name("speakerDataLabel") ;

	name = _("Last Name") ;
	display = name + space ;
	label_lastName.set_label(display) ;
	label_lastName.set_name("speakerDataLabel") ;

	name = _("Gender") ;
	display = name + space ;
	label_gender.set_label(display) ;
	label_gender.set_name("speakerDataLabel") ;

	name = _("Description") ;
	display = name + space ;
	label_description.set_label(display) ;
	label_description.set_name("speakerDataLabel") ;	
}


//**************************************************************************************
//********************************************************************* BUTTONS CALLBACK
//**************************************************************************************

bool SpeakerData::save() 
{
	if (lock_signals)
		return false ;
	
	int res = get_updated_datas() ;
	if (res==1) {
		modified(false);
		if (adding_mode) {
			m_signalSpeakerAdded.emit() ;
			adding_mode = false ;
			set_label(_("Speaker Data")) ;
		}
		else
			m_signalSpeakerUpdated.emit() ;
		m_signalModified.emit() ;
		return true ;
	}
	else
		return false ;
}

void SpeakerData::on_validate()
{
	save() ;
}

void SpeakerData::on_cancel()
{

}


//**************************************************************************************
//********************************************************************* BUSINESS METHODS
//**************************************************************************************


int SpeakerData::get_updated_datas() 
{
	Glib::ustring msg = _("Please fill the ") ;
	Glib::ustring msg2 = _("Invalid character in ") ;
	Glib::ustring tmp = entry_firstName.get_text() ;
	Glib::ustring txt ;
	int res = 1 ;

	//if (tmp!="")
	int check = check_locutor_name(tmp) ;
	if ( check>0 )
		speaker->setFirstName(tmp) ;
	else {
		txt = msg2 + label_firstName.get_label()  ;
		res = -1 ;	
	}
		
	tmp = entry_lastName.get_text() ;
	check = check_locutor_name(tmp) ;
	if (tmp!="" && res!=-1 && check>0)
		speaker->setLastName(tmp) ;
	else if (tmp=="" && res!=-1 ) { 
		txt = msg + label_lastName.get_label()  ;
		res = -1 ;
	}
	else if (check<0) {
		txt = msg2 + label_lastName.get_label()  ;
		res = -1 ;		
	}

	tmp = combo_gender.get_active_text() ;
	if (tmp!="" && res!=-1) {
		speaker->setGender(Speaker::UNDEF_GENDER) ;
		if (tmp=="Male" || tmp=="Homme") {
			speaker->setGender(Speaker::MALE_GENDER) ;
		}else if (tmp=="Female" || tmp=="Femme") {
			speaker->setGender(Speaker::FEMALE_GENDER) ;
		}
	}
	else if (tmp=="" && res!=-1 ) { 
		txt = tmp + label_gender.get_label()  ;
		res = -1 ;
	}

	if (res!=-1) {
		tmp = entry_description.get_text() ;
		check = check_locutor_name(tmp) ;
		if (check>0)
			speaker->setDescription(tmp) ;
		else {
			res=-1 ;
			txt = msg2 + label_description.get_label()  ;
		}
	}
	int indice ;
	bool ok = check_new_languages(indice) ;
	if (ok && res!=-1) {
		speaker->setLanguages(languages_tmp) ;
		fill_model( speaker->getLanguages() );		
		new_languages.clear() ;
	}
	else if (!ok && res!=-1) {
		Gtk::TreeNodeChildren children = refModel->children() ;
		//Gtk::TreeIter iter = new_languages[indice] ;
		Gtk::TreeIter iter = children[indice] ;
		if(refModel->iter_is_valid(iter)) {
			Gtk::TreePath path = refModel->get_path(iter) ; 
			language_list.set_cursor(path) ;
		}
		txt = _("Invalid language, check the information")  ;
		res=-1 ;	
	}	

	if (res==-1)
		Explorer_dialog::msg_dialog_warning(txt,parent,true) ;
	return res ;
}

void SpeakerData::actualise_datas(bool empty)
{
	if (!empty) {
		entry_firstName.set_text(speaker->getFirstName()) ;
		entry_lastName.set_text(speaker->getLastName()) ;		
		if (speaker->getGender()==Speaker::MALE_GENDER){
			combo_gender.set_active_text(_("Male")) ;
		}
		else if (speaker->getGender()==Speaker::FEMALE_GENDER) {
			combo_gender.set_active_text(_("Female")) ;
		}
		else {
			combo_gender.set_active_text(_("Undefined")) ;
		}
		entry_description.set_text(speaker->getDescription()) ;
		languages_tmp.clear() ;
		languages_tmp = std::vector<Speaker::Language>(speaker->getLanguages()) ;
		fill_model( languages_tmp ) ;
	}
	else {
		entry_firstName.set_text("") ;				
		entry_lastName.set_text("") ;	
		combo_gender.set_active_text(_("Undefined")) ;
		entry_description.set_text("") ;	
	}
}


void SpeakerData::set_combo()
{
	combo_gender.append_text(_("Male"));
	combo_gender.append_text(_("Female"));
	combo_gender.append_text(_("Undefined"));
	combo_gender.set_active_text("Undefined") ;	

	combo_gender.get_entry()->set_editable(false);
}


void SpeakerData::set_editable(bool editable)
{
	if (!editable) {
		button_edit.hide() ;
	}
	else {
		button_edit.show() ;
	}	
}

/*
 * Hide/Show edition panel
 */
void SpeakerData::edition_mode(bool edit) 
{
	bool editable ;
	Glib::ustring tmp ;
	
	//edition is enable
	if (edit) {
		editable = true ;
		language_button_box.show() ;
		button_validate.show() ;
		modified(false) ;
	}
	//edition is forbiden
	else {
		editable = false; 
		language_button_box.hide() ;
		button_validate.hide() ;
		sep.hide();
	}

	list_editable(editable) ;

	//> frame
	entry_firstName.set_has_frame(editable) ;
	entry_lastName.set_has_frame(editable) ;
	entry_description.set_has_frame(editable) ;	
	
	//> edition entry
	entry_firstName.set_editable(editable) ;
	entry_lastName.set_editable(editable) ;
	entry_description.set_editable(editable) ;	
	//TODO
	//it's no very clear when unsensitive, maybe changing the colors ?
	//entry_firstName.set_sensitive(editable) ;
	//entry_lastName.set_sensitive(editable) ;
	//entry_description.set_sensitive(editable) ;	
	
	//> edition combo
	combo_gender.set_sensitive(editable) ;
}

void SpeakerData::changeSpeaker(Speaker* s) 
{
	lock_signals = true ;
	speaker = s ;
	actualise_datas(false) ;
	set_label(_("Speaker Data")) ;
	button_validate.set_sensitive(false) ;
	//> After filling model, Gtk seems changing editability of
	// combo and check box in language list
	if (is_editable==false)
		edition_mode(is_editable) ;
	hasChanged = false ;
	lock_signals = false ;
}

void SpeakerData::switch_to_add(bool value)
{
	if (value) {
		adding_mode = true ;
		set_focus_child(entry_firstName) ;
		set_label(_("NEW SPEAKER")) ;
	}
	else {
		adding_mode = false ;
	}
} 

void SpeakerData::prepare_list()
{
	language_list.set_hover_selection(false) ;
	//create model
	refModel = Gtk::ListStore::create(columns) ;
	language_list.set_model(refModel) ;

	//prepare treeview
	language_list.set_rules_hint(true) ;	
	language_list.set_headers_clickable(true) ; 
	language_list.get_selection()->set_mode(Gtk::SELECTION_BROWSE) ;	

	//PREPARE COMBO LIST
	prepare_combo() ;
	
	//ADD LIST COLUMN: EDITABLE OR NOT (DYNAMIC)
	Gtk::TreeViewColumn* pColumn ;
	int col_num ;
	std::vector<int> col ;
	Gtk::CellRendererCombo* cell_c ;
	Gtk::CellRendererToggle* cell_t ;

	//> append column 
	cell_c = new Gtk::CellRendererCombo() ;
	cell_c->set_property("editable", true) ;
	cell_c->set_property("model", modelComboLanguage) ;
	cell_c->set_property("has-entry",false) ;
	cell_c->set_property("text-column",0) ;
	cell_c->signal_edited().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SpeakerData::on_combo_edited), "language")) ;
	col_num = language_list.append_column(_("Language"), *cell_c ); 
	pColumn = language_list.get_column(col_num-1) ;
	if (pColumn) {
		//#ifdef GLIBMM_PROPERTIES_ENABLED
			pColumn->add_attribute(cell_c->property_mode(), columns.a_activatable_combo) ;		
			pColumn->add_attribute(cell_c->property_text(), columns.a_name) ;		
	}

	cell_c = new Gtk::CellRendererCombo() ;
	cell_c->set_property("editable", true) ;
	cell_c->set_property("model", modelComboDialect) ;
	cell_c->set_property("has-entry",false) ;
	cell_c->set_property("text-column",0) ;
	cell_c->signal_edited().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SpeakerData::on_combo_edited), "dialect")) ;
	col_num = language_list.append_column(_("Dialect"), *cell_c ); 
	pColumn = language_list.get_column(col_num-1) ;
	if (pColumn) {
		//#ifdef GLIBMM_PROPERTIES_ENABLED
			pColumn->add_attribute(cell_c->property_mode(), columns.a_activatable_combo) ;		
			pColumn->add_attribute(cell_c->property_text(), columns.a_dialect) ;
	}

	cell_c = new Gtk::CellRendererCombo() ;
	cell_c->set_property("editable", true) ;
	cell_c->set_property("model", modelComboAccent) ;
	cell_c->set_property("has-entry",false) ;
	cell_c->set_property("text-column",0) ;
	cell_c->signal_edited().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SpeakerData::on_combo_edited), "accent")) ;
	col_num = language_list.append_column(_("Accent"), *cell_c ); 
	pColumn = language_list.get_column(col_num-1) ;
	if (pColumn) {
		//#ifdef GLIBMM_PROPERTIES_ENABLED
			pColumn->add_attribute(cell_c->property_mode(), columns.a_activatable_combo) ;
			pColumn->add_attribute(cell_c->property_text(), columns.a_accent) ;
	}

	col_num = language_list.append_column_editable(_("is usual"), columns.a_isUsual ); 
	pColumn = language_list.get_column(col_num-1) ;
	if (pColumn) {
		//#ifdef GLIBMM_PROPERTIES_ENABLED
			cell_t = static_cast<Gtk::CellRendererToggle*>(pColumn->get_first_cell_renderer()) ;
			cell_t->signal_toggled().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SpeakerData::on_toggle_edited), "isUsual")) ;
			pColumn->add_attribute(cell_t->property_activatable(), columns.a_activatable_toggle) ;	
	}

	col_num = language_list.append_column_editable(_("is native"), columns.a_isNative ); 
	pColumn = language_list.get_column(col_num-1) ;
	if (pColumn) {
		//#ifdef GLIBMM_PROPERTIES_ENABLED
			cell_t = static_cast<Gtk::CellRendererToggle*>(pColumn->get_first_cell_renderer()) ;
			cell_t->signal_toggled().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SpeakerData::on_toggle_edited), "isNative")) ;
			pColumn->add_attribute(cell_t->property_activatable(), columns.a_activatable_toggle) ;		
	}

	//> connection to signals
	language_list.signalSelection().connect(sigc::mem_fun(*this, &SpeakerData::on_languageList_selection)) ;
}

void SpeakerData::on_list_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column)
{
}

/*
 * On selection actualise current data in second panel
 */
void SpeakerData::on_languageList_selection(std::vector<Gtk::TreeIter> _paths)
{
	if (lock_signals)
		return ;

	Gtk::TreeIter iter ;
	guint size = _paths.size() ;
	if (size == 1)
		iter = _paths[0] ;
	else
		iter = _paths[size-1] ;

	Glib::ustring code_language = (**iter)[columns.a_code] ;
	fill_dialect_accent(code_language, false);
}

void SpeakerData::actualise_language_combos(const Gtk::TreePath& path, const Gtk::TreeIter& iter) 
{
	if (iter) {
		Glib::ustring code_language = (**iter)[columns.a_code] ;
		fill_dialect_accent(code_language, false);
	}
}

//**************************************************************************************
//************************************************************* LANGUAGES LIST BUSINESSS 
//**************************************************************************************

void SpeakerData::fill_model(std::vector<Speaker::Language> language)
{
	refModel->clear() ;

	std::vector<Speaker::Language>::iterator it = language.begin() ;  
	if (languages) {
		Gtk::TreeRow row ;
		while ( it!=language.end() ) {
			row = *(refModel->append()) ;
			columns.fill_row( &row, &(*it),true, languages) ;
			it++ ;
		} 	
	}
}

void SpeakerData::list_editable(bool value)
{
/*	Gtk::TreeModel::Children children = refModel->children() ;
	Gtk::TreeModel::iterator it = children.begin() ;
	while(it!=children.end()) {
		if (value)
			(*it)[columns.a_activatable_combo] = Gtk::CELL_RENDERER_MODE_EDITABLE ;
		else
			(*it)[columns.a_activatable_combo] = Gtk::CELL_RENDERER_MODE_INERT ;
		(*it)[columns.a_activatable_toggle] = value ;
		it++ ;
	}
*/
	language_list.set_sensitive(value) ;
}	 

void SpeakerData::prepare_combo() 
{
//	modelComboLanguage.clear() ;
	
	if (languages!=NULL) {
		modelComboLanguage = Gtk::ListStore::create(c_columns) ; 
	
		std::vector<Glib::ustring> lang = languages->get_names() ;
		
		Gtk::TreeRow row ;
		
		//LANGUAGE
		for (guint i=0; i<lang.size(); i++) {
			row = *(modelComboLanguage->append()) ;			
			SpeakerData::ComboModelColumn::fill_row(&row, lang[i]) ;	
		}
		fill_dialect_accent("", true);
	}
}


void SpeakerData::fill_dialect_accent(const Glib::ustring& lang, bool first_time)
{
	Gtk::TreeRow row ;
	if (languages!=NULL) {
		if ( first_time ) {
			modelComboDialect = Gtk::ListStore::create(c_columns) ;
			modelComboAccent = Gtk::ListStore::create(c_columns) ;	
		} else {
			modelComboDialect->clear() ;	
			modelComboAccent->clear() ;
		}
		std::vector<Glib::ustring> dialect = languages->get_dialects(lang) ;
		std::vector<Glib::ustring> accent = languages->get_accents(lang) ;

		//DIALECT
		row = *(modelComboDialect->append()) ;			
		SpeakerData::ComboModelColumn::fill_row(&row, "none") ;			
		for (guint i=0; i<dialect.size(); i++) {
			row = *(modelComboDialect->append()) ;			
			SpeakerData::ComboModelColumn::fill_row(&row, dialect[i]) ;			
		}
	
		//ACCENT
		row = *(modelComboAccent->append()) ;			
		SpeakerData::ComboModelColumn::fill_row(&row, "none") ;			
		for (guint i=0; i<accent.size(); i++) {
			row = *(modelComboAccent->append()) ;			
			SpeakerData::ComboModelColumn::fill_row(&row, accent[i]) ;			
		}
	}
}

void SpeakerData::on_combo_edited(const Glib::ustring& path, const Glib::ustring& s2, Glib::ustring combo)
{
	if (lock_signals)
		return ;
	
	//print_languages(languages_tmp) ; 
	Gtk::TreePath tpath = Gtk::TreePath(path) ;
	Gtk::TreeIter iter = refModel->get_iter(tpath) ;
	int indice = get_language_from_iter(iter) ;
	int ko = 1 ;
	//> LANGUAGE
	if (combo=="language") {
		Glib::ustring dialect = (*iter)[columns.a_dialect] ;
		Glib::ustring code = languages->get_code(s2) ;
		//unique key is couple (language,dialect)
		if ( !exists_language(languages_tmp, code, dialect, iter) ) {
			//change in list
			languages_tmp[indice].setCode(code) ;
			//change in widget list
			(*iter)[columns.a_code]=code ;
			(*iter)[columns.a_name] = s2 ;
			fill_dialect_accent(code, false);
		}
		else {
			ko=-1 ;
		} 	
	}
	//> DIALECT
	else if (combo=="dialect"){
		Glib::ustring code = (*iter)[columns.a_code] ;		
		//unique key is couple (language,dialect)
		if ( !exists_language(languages_tmp, code, s2, iter) ) {
			//change in widget list
			languages_tmp[indice].setDialect(s2) ;	
			//change in widget list
			(*iter)[columns.a_dialect] = s2 ;
		}
		else {
			ko=-1 ;
		}
	}
	//> ACCENT
	else if (combo=="accent"){
			//change in widget list			
			languages_tmp[indice].setAccent(s2) ;
			//change in widget list			
			(*iter)[columns.a_accent]=s2 ;		
	}
	if (ko==-1) {
		Explorer_dialog::msg_dialog_warning(_("This language already exists for this speaker"),parent, true) ;
	}
/*	else if (ko==-2)  {
		Explorer_dialog::msg_dialog_warning(_("Invalid language, check the information"),parent, true) ;
	}*/
	else {
		modified(true) ;
	}
}

void SpeakerData::on_toggle_edited(const Glib::ustring& path, Glib::ustring toggle)
{
	if (lock_signals)
		return ;
	
	bool native, usual ;	
	Gtk::TreePath tpath = Gtk::TreePath(path) ;
	Gtk::TreeIter iter = refModel->get_iter(tpath) ;
	int indice = get_language_from_iter(iter) ;
	Speaker::Language* language = &(languages_tmp[indice]) ; 
	
	if ( toggle=="isUsual" ) {
		usual = (*iter)[columns.a_isUsual] ;
		language->setUsual(usual) ;		
	}
	else if ( toggle=="isNative" ) {
		native = (*iter)[columns.a_isNative] ;
		language->setNative(native) ;
	}
	//actualise
	modified(true) ;
}

bool SpeakerData::exists_language(std::vector<Speaker::Language> languages, Glib::ustring code, Glib::ustring dialect, Gtk::TreeIter iterator_except)
{
	bool found = false ;
	Gtk::TreeModel::Children children = refModel->children() ;
	Gtk::TreeIter i = children.begin();
	while ( i!=children.end() && !found ) {
		if ( (*i)[columns.a_code]==code && (*i)[columns.a_dialect]==dialect && !i.equal(iterator_except)  ) 
			found = true ;
		i++ ;
	}		
	return found ;
}

int SpeakerData::get_language_from_iter(Gtk::TreeIter iter)
{
	Gtk::TreeModel::Children children = refModel->children() ;
	Gtk::TreeModel::iterator it = children.begin() ;
	int cpt = -1 ;
	bool found = false ;
	while(it!=children.end() && !found) {		
		Gtk::TreeIter tmp =  (*it) ;
		if (tmp.equal(iter))
			found = true ;
		it++;
		cpt++;
	}
	return cpt ; 
}

void SpeakerData::print_languages(std::vector<Speaker::Language> languages)
{
	std::vector<Speaker::Language>::iterator it = languages.begin() ;
	while ( it!=languages.end() ) {
		//std::cout << "<<Entry| " 
				//<< (*it).getCode() << " - " 
				//<< (*it).getDialect() << " - " 
				//<< (*it).getAccent() << " - " 				 
				//<< (*it).isNative() << " - "
				//<< (*it).isUsual() 
		//<< " |Entry" << std::endl ;
		it++ ;	
	}
}

void SpeakerData::addRemove_language(Glib::ustring mode) 
{ 
	//REMOVE A LANGUAGE
	if (mode=="remove") {
		Gtk::TreeIter iter = language_list.get_selection()->get_selected() ;
		if (refModel->iter_is_valid(iter) && iter) {
			int indice = get_language_from_iter(iter) ;
			guint cpt = 0 ;
			bool found = false ;
			std::vector<Speaker::Language>::iterator it = languages_tmp.begin() ;
			while(it!=languages_tmp.end()&&!found) {
				if (cpt==indice)
						found=true ;
				else
					it++ ;
				cpt++ ;
			}
			if (found) {
				languages_tmp.erase(it) ;	
				std::vector<Gtk::TreeIter>::iterator it2 = new_languages.begin() ;
				bool found = false ;
				while (it2!=new_languages.end() && !found) {
					if ((*it2)==iter)
						found = true ;
					else 
						it2++ ;
				} 
				if (found)
					new_languages.erase(it2) ;
				refModel->erase(iter) ;
				modified(true) ;
			}
		}
		else {
			Explorer_dialog::msg_dialog_warning(_("Select a language"), parent, true);	
		}
	}
	//ADD A LANGUAGE
	else {
		Speaker::Language* l = new Speaker::Language("--", false,false,"","" ) ;
		languages_tmp.insert(languages_tmp.end(), *l) ;
		Gtk::TreeIter _new =  refModel->append() ;
		Gtk::TreeRow row= *(_new) ;
		columns.fill_row(&row, l, true, languages) ;
		new_languages.insert(new_languages.end(), _new) ;
		modified(true) ;
	}
}

// return the indice in languages_tmp map where information is unavalaible
bool SpeakerData::check_new_languages(int& indice)
{
	bool res = true ;
	//std::vector<Gtk::TreeIter>::iterator iter = new_languages.begin() ;
	
	Gtk::TreeNodeChildren children = refModel->children() ;
	Gtk::TreeIter iter = children.begin() ;
	int cpt = -1 ;
	while ( iter!= children.end() && res )	{
		if ( (*(*iter))[columns.a_code] == "--"  || (*(*iter))[columns.a_code] == "" || (*(*iter))[columns.a_name] == "" )	
			res=false ;
		iter++ ;
		cpt++ ;
	}
	if (!res) {
		indice = cpt ;
		return res ;
	}
	else
		return 1 ;
}

void SpeakerData::on_entries_changed()
{
	if (lock_signals)
		return ;
	
	modified(true) ;	
}

void SpeakerData::modified(bool mod) 
{
	hasChanged=mod ;
	button_validate.set_sensitive(mod) ;
}

bool SpeakerData::on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time)
{
	//ListModel_Columns c ;
	if (!adding_mode)
		m_signalOnDrop.emit() ;
	else
		gdk_beep() ;
	return true ;
}

int SpeakerData::check_locutor_name(Glib::ustring name)
{
	bool correct = StringOps::check_string(STRING_OPS_FORBIDDEN_XML, name) ;
	if (correct)
		return 1 ;
	else
		return -1 ;
}

Glib::ustring SpeakerData::get_speaker() 
{
	if (speaker)
		return speaker->getId() ;
	else
		return "" ;
}

void SpeakerData::my_grab_focus()
{
	grab_focus() ;
	entry_firstName.grab_focus() ;
}

}//NAMESPACE
