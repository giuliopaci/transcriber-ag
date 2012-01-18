/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "TSearchGeneral.h"
#include "Explorer_utils.h"
#include "Explorer_dialog.h"

#include "Common/util/Utils.h"
#include "Common/icons/Icons.h"
#include "Common/globals.h"
#include "Common/InputLanguageHandler.h"
#include "Common/Dialogs.h"
#include "Common/widgets/GtUtil.h"
#include "Common/FileInfo.h"

#include "DataModel/speakers/Speaker.h"
#include <gtk/gtkimcontext.h>

#define SEARCH_TAG_NAME "SearchTagDialog"

namespace tag {

TSearchGeneral::TSearchGeneral()
{
	Gtk::RadioButton::Group group = rb_section.get_group() ;
	rb_turn.set_group(group) ;
	rb_event.set_group(group) ;
	rb_entity.set_group(group) ;

	button_value.set_use_underline(true) ;

	m_value_topic = "" ;
	m_value_qualifier = "" ;
	m_value_speaker = "" ;

	hasSelection = false ;
	cancel = false ;
}

TSearchGeneral::~TSearchGeneral()
{
}

void TSearchGeneral::init(AnnotationEditor* editor)
{
	active = true ;
	bak  = false ;

	external_editor = editor ;

	connection_changeActiveView = editor->signalChangeActiveView().connect(sigc::mem_fun(*this, &TSearchGeneral::on_change_view));
	external_signal_mark_set = get_external_buffer()->signal_mark_set().connect(sigc::mem_fun(*this, &TSearchGeneral::on_change_cursor));

  	//> prepare tag for selection
	prepareTag() ;

	//> connection to signal
	connection_find_fw = find_fw.signal_clicked().connect(sigc::mem_fun(*this, &TSearchGeneral::on_search_fw)) ;
	connection_find_bk = find_bk.signal_clicked().connect(sigc::mem_fun(*this, &TSearchGeneral::on_search_bk)) ;
	connection_close_button = close_button.signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &TSearchGeneral::on_response), -4)) ;
	connection_rb_section = rb_section.signal_clicked().connect(sigc::bind<string>(sigc::mem_fun(*this, &TSearchGeneral::on_type_changed), "section")) ;
	connection_rb_turn = rb_turn.signal_clicked().connect(sigc::bind<string>(sigc::mem_fun(*this, &TSearchGeneral::on_type_changed), "turn")) ;
	connection_rb_entity = rb_entity.signal_clicked().connect(sigc::bind<string>(sigc::mem_fun(*this, &TSearchGeneral::on_type_changed), "qualifier_entity")) ;
	connection_rb_event = rb_event.signal_clicked().connect(sigc::bind<string>(sigc::mem_fun(*this, &TSearchGeneral::on_type_changed), "qualifier_event")) ;
	connection_button_value = button_value.signal_clicked().connect(sigc::mem_fun(*this, &TSearchGeneral::on_button_value_clicked)) ;

	mySet_focus() ;

	topic_list = get_external_model()->conventions().getTopics() ;

	//> Default mode: topic
	rb_section.set_active(true) ;
	reset_mode("section") ;

	//> Prepare cursor
  	start_occur = get_external_buffer()->begin() ;
	end_occur = get_external_buffer()->begin() ;
	start_search = get_external_buffer()->begin() ;
	end_search = get_external_buffer()->end() ;
	current = start_search ;
}

void TSearchGeneral::reset()
{
	//remove signal
	connection_find_fw.disconnect() ;
	connection_find_bk.disconnect() ;
	connection_close_button.disconnect() ;
	connection_rb_turn.disconnect() ;
	connection_rb_section.disconnect() ;
	connection_rb_event.disconnect() ;
	connection_rb_entity.disconnect() ;
	connection_combo_search.disconnect() ;
	connection_changeActiveView.disconnect() ;
	connection_button_value.disconnect() ;

	//remove last tag
	remove_occurence(SEARCH_TAG_NAME) ;

	//diconnect
	external_signal_buffer_changed.disconnect() ;
	external_signal_mark_set.disconnect() ;
}


void TSearchGeneral::close()
{
	//desactive
	cancel = true ;

	active = false ;

	//stop connection
	reset() ;

	//hide
	myHide() ;
}

void TSearchGeneral::switch_file(AnnotationEditor* editor)
{
	close() ;
	init(editor) ;
}


//******************************************************************************
//************************* SPECIFIC TYPES GUI MANAGEMENT **********************
//******************************************************************************

void TSearchGeneral::reset_mode(Glib::ustring type)
{
	if ( type.compare("section") == 0 )
	{
		m_type = type ;
		m_value_topic = "" ;
		m_property="topic" ;
		button_value.set_label(_("Select topic")) ;
		label_value.set_label(_("Select topic: ")) ;
		combo_value.hide() ;
		label_value.show() ;
	}
	else if ( m_type.compare("turn") == 0 )
	{
		m_type = type ;
		m_property="speaker" ;
		button_value.set_label(_("Select speaker")) ;
		label_value.set_label(_("Select speaker: ")) ;
		m_value_speaker = "" ;
		combo_value.hide() ;
		label_value.show() ;
	}
	else if ( m_type.compare("qualifier_entity") == 0 || m_type.compare("qualifier_event") == 0)
	{
		m_property="text" ;
		label_value.set_label(_("Enter text: ")) ;
		m_type = type ;
		m_value_qualifier = "" ;
		button_value.hide() ;
		combo_value.show() ;
		combo_value.set_text("") ;
		label_value.show() ;
	}
}

Glib::ustring TSearchGeneral::get_first_value(Glib::ustring type)
{
	if ( m_type.compare("section") == 0 )
	{
		return "" ;
	}
	else if ( m_type.compare("turn") == 0 )
	{
		return "" ;
	}
	else if ( m_type.compare("qualifier_entity") == 0 || m_type.compare("qualifier_event") == 0)
	{
		return "" ;
	}
}

void TSearchGeneral::on_type_changed(std::string new_type)
{
	m_type = new_type ;
	if ( m_type.compare("section") == 0 )
	{
		m_property = "topic" ;
		label_value.set_label(_("Select topic: ")) ;
		button_value.show() ;
		combo_value.hide() ;

		if (m_value_topic.empty())
			m_value_topic = get_first_value(m_type) ;

		if (!m_value_topic.empty()) {
			string label = Topics::getTopicLabel(m_value_topic, topic_list) ;
			button_value.set_label(label) ;
		}
		else
			reset_mode(m_type) ;
		remove_occurence(SEARCH_TAG_NAME) ;
	}
	else if ( m_type.compare("turn") == 0 )
	{
		m_property = "speaker" ;
		label_value.set_label(_("Select speaker: ")) ;
		button_value.show() ;
		combo_value.hide() ;
		if (!m_value_speaker.empty())
			button_value.set_label(get_external_model()->getSpeakerDictionary().getSpeaker(m_value_speaker).getFullName()) ;
		else
		{
			m_value_speaker = get_first_value(m_type) ;
			if (!m_value_speaker.empty())
				button_value.set_label(get_external_model()->getSpeakerDictionary().getSpeaker(m_value_speaker).getFullName()) ;
			else
				reset_mode(m_type) ;
		}
		remove_occurence(SEARCH_TAG_NAME) ;
	}
	else if ( m_type.compare("qualifier_entity") == 0 || m_type.compare("qualifier_event") == 0)
	{
		m_property = "text" ;
		label_value.set_label(_("Enter text: ")) ;
		button_value.hide() ;
		combo_value.show() ;
		combo_value.set_text(m_value_qualifier) ;
		remove_occurence(SEARCH_TAG_NAME) ;
	}
}


void TSearchGeneral::on_button_value_clicked()
{
	// open topic dialog
	if (m_type.compare("section") == 0)
	{
		Glib::ustring id, label ;
		TopicsDialog dialog(topic_list, NULL) ;
		dialog.set_light_mode() ;
		int res = dialog.run() ;
		if (res==Gtk::RESPONSE_APPLY)
		{
			Glib::ustring id, label ;
			bool ok = dialog.get_chosen(id, label) ;
			if (ok)
			{
				m_value_topic = id ;
				button_value.set_label(label) ;
			}
			else
				Explorer_dialog::msg_dialog_error(_("error while choosing topic"), NULL, true) ;
		}
	}
	// open dictionary
	else if (m_type.compare("turn") == 0)
	{
		dico = new SpeakerDico_dialog(false, true, NULL) ;
		dico->set_light_mode() ;
		dico->open_dictionary(external_editor) ;
		dico->signal_hide().connect( sigc::mem_fun(*this, &TSearchGeneral::on_close_speakerdialog));
		dico->show() ;
//		get_window_settings(d, SETTINGS_DICOF_NAME) ;
		if (!m_value_speaker.empty())
			dico->set_cursor_to_speaker(m_value_speaker) ;
	}
}

void TSearchGeneral::on_close_speakerdialog()
{
	Speaker* s = dico->get_current_speaker() ;
	if (s) {
		m_value_speaker = s->getId() ;
		string label = s->getFullName() ;
		button_value.set_label(label) ;
	}
	else {
		m_value_speaker = "" ;
		button_value.set_label(_("Select speaker")) ;
	}
	if (dico) {
		dico->close_dialog() ;
		delete(dico) ;
	}
}

//******************************************************************************
//***************************** RESEARCH BUSINESS ******************************
//******************************************************************************


void TSearchGeneral::on_search_fw()
{
	if ( search_error() )
		return ;

	lock_cursor = true ;
	load_state() ;

	if (bak) {
		bak = false ;
		if (found)
			current = end_occur ;
	}

	bool end  = false ;
	found = false ;

	searchingStatus(true) ;
	while (!found && !end && !cancel)
	{
		found = my_forward_search(current) ;
		if (found) {
			 load_state() ;
			 current = end_occur ;
			 save_state(0) ;
		}
		end = (current==get_external_buffer()->end()) ;

		// flush
		GtUtil::flushGUI(false, false) ;
	}

	searchingStatus(false) ;

	if (!found && end) {
		search_end() ;
	}

	if (!found)
		remove_occurence(SEARCH_TAG_NAME) ;

	save_state(0) ;

	lock_cursor = false ;
}

void TSearchGeneral::on_search_bk()
{
	if ( search_error() )
		return ;

	lock_cursor = true ;
	load_state() ;

	if (!bak) {
		bak = true ;
		if (found)
			current = start_occur ;
	}

	bool end  = false ;
	found = false ;

	searchingStatus(true) ;
	while (!found && !end)
	{
		found = my_backward_search(current) ;
		if (found) {
			 load_state() ;
			 current = start_occur ;
			 save_state(0) ;
		}
		end = (current==get_external_buffer()->begin()) ;

		// flush
		GtUtil::flushGUI(false, false) ;
	}
	searchingStatus(false) ;

	if (!found && end) {
		search_end() ;
	}

	if (!found)
		remove_occurence(SEARCH_TAG_NAME) ;

	save_state(0) ;

	lock_cursor = false ;
}


std::string TSearchGeneral::getSearchedParentElement(const string& id)
{
	string type = get_external_model()->getParentElement(id, m_type) ;
	if (id.empty())
		return "" ;
	else if ( type.compare(m_type)==0 )
		return id ;
	else {
		string parent = get_external_model()->getParentElement(id, m_type) ;
		return parent ;
	}
}

bool TSearchGeneral::my_forward_search(Gtk::TextIter iter)
{
	bool prefix ;
	string tagname = m_type ;
	if ( m_type.compare("section") == 0 )
		prefix = false ;
	else
		prefix = true ;

	//> For forward case, as turn label are placed above segments, if cursor is
	//  placed in a segment the turn it belongs to won't be candidate to the
	//  search !
	//  	Let's fix it (case 1)
	//  If the cursor is place in a candidate label, the use of toggle methods
	//  won't see it as canditate
	//		Let's fix it (case 2)

	Glib::RefPtr<Gtk::TextTag> tag = get_external_buffer()->iterHasTag(current, tagname, prefix) ;
	Glib::RefPtr<Gtk::TextTag> nulltag = (Glib::RefPtr<Gtk::TextTag>)0 ;

	string baseType = get_external_model()->mainstreamBaseType("transcription_graph") ;

	Glib::ustring id = get_external_buffer()->anchors().getAnchorIdNearPos(current, baseType) ;
	Glib::ustring parent_id = getSearchedParentElement(id) ;

	bool classic_behaviour = true ;
	if (!tag)
	{
		// CASE 1: overlapping case
		// if the parent is candidate to match, let's start from it
		if (!parent_id.empty() && selected_id.compare(parent_id)!=0 && last_overlap_checked.compare(parent_id)!=0)
		{
			string id = check_conditions(parent_id) ;
			if (!id.empty()) {
				actualize_occurence(id, SEARCH_TAG_NAME) ;
				return true ;
			}
			classic_behaviour = false ;
			last_overlap_checked = parent_id ;
		}
	}
	// CASE 2 : we're already in a label with good tag
	else
	{
		string id = check_conditions(current) ;
		// current label match and is not the previously selected:
		// let's start from it
		if ( !id.empty() && id.compare(selected_id)!=0 )
			classic_behaviour = false ;
	}

	// CASE 3: classic behaviour, NeXT
	if (classic_behaviour) {
		current.forward_to_tag_toggle(nulltag) ;
	}

	//> Special case has been handled, let's proceed the search

	tag = get_external_buffer()->iterHasTag(current, tagname, prefix) ;
	if (!tag)
		return false ;
	else
	{
		Glib::ustring id = check_conditions(current) ;
		if (!id.empty())
		{
			actualize_occurence(id, SEARCH_TAG_NAME) ;
			return true ;
		}
		return false ;
	}
}

bool TSearchGeneral::my_backward_search(Gtk::TextIter iter)
{
	bool prefix ;
	string tagname = m_type ;
	if ( m_type.compare("section") == 0 )
		prefix = false ;
	else
		prefix = true ;

	Glib::RefPtr<Gtk::TextTag> tag = get_external_buffer()->iterHasTag(current, tagname, prefix) ;
	Glib::RefPtr<Gtk::TextTag> nulltag = (Glib::RefPtr<Gtk::TextTag>)0 ;

	if (tag)
		current.backward_to_tag_toggle(nulltag) ;

	current.backward_to_tag_toggle(nulltag) ;

	//> Special case has been handled, let's proceed the search

	tag = get_external_buffer()->iterHasTag(current, tagname, prefix) ;
	if (!tag) {
		list< Glib::RefPtr<Gtk::TextTag > > tags = const_cast<Gtk::TextBuffer::iterator&>(iter).get_tags();
		list< Glib::RefPtr<Gtk::TextTag > >::iterator it;
		return false ;
	}
	else {
		Glib::ustring id = check_conditions(current) ;
		if (!id.empty())
		{
			actualize_occurence(id, SEARCH_TAG_NAME) ;
			return true ;
		}
		return (!id.empty()) ;
	}
}

Glib::ustring TSearchGeneral::check_conditions(Gtk::TextIter iter)
{
	list< Glib::RefPtr<Gtk::TextTag > > tags = const_cast<Gtk::TextBuffer::iterator&>(iter).get_tags();
	list< Glib::RefPtr<Gtk::TextTag > >::iterator it;
	Glib::ustring id = get_external_buffer()->getPreviousAnchorId(iter, m_type, true);
	return check_conditions(id) ;
}

Glib::ustring TSearchGeneral::check_conditions(const string& id)
{
	if (id.empty()) {
		return "" ;
	}

	if (id.compare(selected_id)==0) {
		return "" ;
	}

	string prop = get_external_model()->getElementProperty(id, m_property, "") ;
	string value ;
	if (m_type.compare("section")==0)
		value = m_value_topic ;
	else if (m_type.compare("turn")==0)
		value = m_value_speaker ;
	else
		value = m_value_qualifier ;


	if ( ( prop.compare(value) == 0 ) || ( prop.empty() && value == TAG_TOPIC_NULL_LABEL) ) {
		return id ;
	}
	else {
		return "" ;
	}
}


/*
 *
 */
void TSearchGeneral::actualize_occurence(const std::string& id, const string& tagname)
{
	string type = get_external_model()->getElementType(id) ;
	get_external_view()->setTag(tagname, type, id, true, get_external_view()->getViewTrack(), start_occur, end_occur) ;
	get_external_buffer()->setCursor(start_occur, true) ;
	save_state(2) ;
	selected_id = id ;
}

void TSearchGeneral::remove_occurence(const string& tagname)
{
	get_external_buffer()->clearTag(tagname) ;
	selected_id.clear() ;
	last_overlap_checked.clear() ;
}

void TSearchGeneral::check_initial_selection()
{

}

//******************************************************************************
//******************************* COMBO LANGUAGE *******************************
//******************************************************************************

void TSearchGeneral::set_combo_language()
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
	if(iLang == NULL)
		iLang = InputLanguageHandler::get_input_language_by_name(DEFAULT_LANGUAGE) ;
	else
		combo_language.set_active_text(iLang->getLanguageDisplay());
	 combo_language.set_focus_on_click(false) ;
}

void TSearchGeneral::on_change_language()
{
	Glib::ustring display = combo_language.get_active_text() ;
	if (display != "")
	{
		//> actualize input language for each combo
		iLang = InputLanguageHandler::get_input_language_by_name(display);
		combo_value.set_input_language(iLang) ;

		//> check if an external IME is in used, and tell each combo
		if (iLang && iLang->getLanguageType().compare(IME_LANGUAGE)==0) {
			combo_value.setIMEstatus(true) ;
		}
		else {
			combo_value.setIMEstatus(false) ;
		}
	}
}

void TSearchGeneral::search_end()
{
	Glib::ustring txt ;
	if (bak) {
		txt = _("no more occurrences") ;
		display_info(txt) ;
	}
	else if (!bak) {
		txt = _("No more occurrences, research from the start ?") ;
		int response = display_question(txt) ;
		if (response==Gtk::RESPONSE_YES) {
			count=0 ;
			current = start_search ;
//			select_occ() ;
			save_state(0) ;
			on_search_fw() ;
		}
	}
}

bool TSearchGeneral::search_error()
{
	Glib::ustring txt = "" ;

	if (m_type.compare("section")==0 && m_value_topic.empty()) {
		txt = _("Please choose a topic") ;
	}
	else if (m_type.compare("turn")==0 && m_value_speaker.empty()) {
		txt = _("Please choose a speaker") ;
	}
	else if (m_value_qualifier.empty()==0) {
		txt = _("Please enter a text") ;
	}

	if (!txt.empty()) {
		display_info(txt) ;
		return true ;
	}
	return false ;
}


void TSearchGeneral::on_change_cursor(const Gtk::TextBuffer::iterator& it, const Glib::RefPtr<Gtk::TextBuffer::Mark >& mark)
{
	if (active)
	{
		//> only proceed if insert mark
		if (mark->get_name()=="insert" && !lock_cursor)
		{
			current = it ;
			remove_occurence(SEARCH_TAG_NAME) ;

			// disable selection mode when click on text
			if (hasSelection)
				selection_mode(false) ;
			save_state(3) ;
		}
	}
}

void TSearchGeneral::on_buffer_changed()
{
	if (active && !lock_cursor) {
		if (!hasSelection) {
			start_search = get_external_buffer()->begin() ;
			end_search = get_external_buffer()->end() ;
			save_state(1) ;
		}
		else {
			load_state() ;
			current = start_search ;
			save_state(4) ;
		}
	}
}

void TSearchGeneral::on_change_view()
{
	prepareTag() ;
	external_editor->clearViewTag(SEARCH_TAG_NAME) ;
	external_signal_buffer_changed.disconnect() ;
	external_signal_mark_set.disconnect() ;
	external_signal_mark_set = get_external_buffer()->signal_mark_set().connect(sigc::mem_fun(*this, &TSearchGeneral::on_change_cursor));
	external_signal_buffer_changed = get_external_buffer()->signal_changed().connect(sigc::mem_fun(*this, &TSearchGeneral::on_buffer_changed));
	current = get_external_buffer()->get_mark("insert")->get_iter() ;
 	start_occur = get_external_buffer()->begin() ;
	end_occur = get_external_buffer()->begin() ;
	start_search = get_external_buffer()->begin() ;
	end_search = get_external_buffer()->end() ;
	save_state(0) ;
}


void TSearchGeneral::prepareTag()
{
  	dialogTag_occ = get_external_buffer()->get_tag_table()->lookup(SEARCH_TAG_NAME) ;
	if (!dialogTag_occ)
	{
		dialogTag_occ = get_external_buffer()->create_tag(SEARCH_TAG_NAME);
	  	dialogTag_occ->property_foreground().set_value("#000000");
	  	dialogTag_occ->property_background().set_value("#80D480");
  	}
}


void TSearchGeneral::searchingStatus(bool searching)
{
	cancel = false ;
	if (searching)
	{
		if ( !connection_find_fw.blocked() )
			connection_find_fw.block(true) ;
		if ( !connection_find_bk.blocked() )
			connection_find_bk.block(true) ;
		search_in_progress.show() ;
		search_static.hide() ;
	}
	else
	{
		if ( connection_find_fw.blocked() )
			connection_find_fw.block(false) ;
		if ( connection_find_bk.blocked() )
			connection_find_bk.block(false) ;
		search_in_progress.hide() ;
		search_static.show() ;
	}
//	find_bk.set_sensitive(!searching) ;
//	find_fw.set_sensitive(!searching) ;
}

} //namespace



