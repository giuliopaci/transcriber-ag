/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "SearchReplaceGeneral.h"
#include "Explorer_utils.h"
#include "Common/util/Utils.h"
#include "Explorer_dialog.h"
#include "Common/icons/Icons.h"
#include "Common/globals.h"
#include "Common/InputLanguageHandler.h"

#include <gtk/gtkimcontext.h>

#define SEARCH_REPLACE_SELECT_TAG_NAME 	"SearchSelectDialog"
#define SEARCH_REPLACE_TAG_NAME 		"SearchReplaceDialog"

namespace tag {

SearchReplaceGeneral::SearchReplaceGeneral()
{
	Gtk::RadioButton::Group group = rb_all.get_group() ;
	rb_sel.set_group(group) ;
}

SearchReplaceGeneral::~SearchReplaceGeneral()
{
}

void SearchReplaceGeneral::init(AnnotationEditor* editor)
{
	lock_cursor = true ;

	active = true ;
	count = 0 ;
	replace_all_mode = false ;
	bak  = false ;
	found = false ;
	_search = "" ;
	_replace = "" ;
	hasSelection = false ;

	flag_uneditable = 0 ;

	std::vector<AnnotationView*>::iterator it ;

	external_editor = editor ;

	connection_changeActiveView = editor->signalChangeActiveView().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_change_view));
	external_signal_mark_set = get_external_buffer()->signal_mark_set().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_change_cursor));

  	//> prepare tag for selection
	prepareTag() ;

	// initialise iterator
  	start_occur = get_external_buffer()->begin() ;
	end_occur = get_external_buffer()->begin() ;
	start_search = get_external_buffer()->begin() ;
	end_search = get_external_buffer()->end() ;
	current = start_search ;

	//> get editor selection
	check_initial_selection() ;

	lock_cursor = false ;
	//> connection to signal
	connection_replace_all = replace_all.signal_clicked().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_replace_all)) ;
    connection_replace = replace.signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &SearchReplaceGeneral::on_replace_find), false )) ;
	connection_find_fw = find_fw.signal_clicked().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_search_fw)) ;
	connection_find_bk = find_bk.signal_clicked().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_search_bk)) ;
	connection_close_button = close_button.signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &SearchReplaceGeneral::on_response), -4)) ;
	connection_rb_all = rb_all.signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &SearchReplaceGeneral::on_scope_change), false)) ;
	connection_rb_sel = rb_sel.signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &SearchReplaceGeneral::on_scope_change), true)) ;
	connection_cb_case = cb_case.signal_clicked().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_case_sensitive)) ;
	connection_cb_whole = cb_whole.signal_clicked().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_whole_word)) ;
	connection_combo_replace = combo_replace.signal_changed().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::get_entries));
	connection_combo_search = combo_search.signal_changed().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_signal_changed));
	connection_change_mode = change_mode.signal_clicked().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_change_mode)) ;

	set_combo_search_mode() ;

	find_fw.set_use_underline(true) ;
	find_bk.set_use_underline(true) ;
	replace.set_use_underline(true) ;
	replace_all.set_use_underline(true) ;

	if (combo_search.get_text()=="") {
		find_fw.set_sensitive(false) ;
		find_bk.set_sensitive(false) ;
		find_replace_bk.set_sensitive(false) ;
		find_replace_fw.set_sensitive(false) ;
		replace.set_sensitive(false) ;
		replace_all.set_sensitive(false) ;
		cb_case.set_sensitive(true) ;
	}
	else {
		find_fw.set_sensitive(true) ;
		find_bk.set_sensitive(true) ;
	}
	mySet_focus() ;

	//TODO
	//prepare combo
	iLang = editor->get_input_language() ;
	combo_search.set_input_language(iLang) ;
	combo_replace.set_input_language(iLang) ;
	set_combo_language() ;
	combo_language.signal_changed().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_change_language));
	combo_searchMode.signal_changed().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_change_search_mode));

	cb_whole.set_active(whole_word_mode);
	cb_case.set_active(case_sensitive_mode);
}

void SearchReplaceGeneral::active_search(bool active)
{
	find_bk.set_sensitive(active) ;
	find_fw.set_sensitive(active) ;
}

void SearchReplaceGeneral::active_replace(bool active)
{
	find_replace_bk.set_sensitive(active) ;
	find_replace_fw.set_sensitive(active) ;
	replace.set_sensitive(active) ;
	replace_all.set_sensitive(active) ;
}

void SearchReplaceGeneral::get_entries()
{
	_search = combo_search.get_text() ;
	_replace = combo_replace.get_text() ;
	Glib::ustring selection ;

	if (_search!="") {
		active_search(true) ;
	}
	else {
		active_search(false) ;
	}
	if (_replace!="" && count>0 && flag_uneditable==0) {
		active_replace(true) ;
	}
	else {
		active_replace(false) ;
	}
}


void SearchReplaceGeneral::on_signal_changed()
{
	combo_replace.set_name("combo_search_default") ;
	combo_search.set_name("combo_search_default") ;
	get_entries() ;
}


bool SearchReplaceGeneral::add_in_list(std::vector<Glib::ustring>* vect, Glib::ustring name)
{
	bool is = false;

	std::vector<Glib::ustring>::iterator it ;
	if (vect->size()!=0 && name!="") {
		it=vect->begin() ;
		while (it!=vect->end() && !is) {
			if ((*it)==name)
				is = true ;
			it++ ;
		}
	}
	if (!is || vect->size()==0) {
		vect->insert(vect->begin(), name) ;
		return true ;
	}
	else
		return false ;
}


//**************************************************************************************
//***************************************************************************** BUSINESS
//**************************************************************************************


bool SearchReplaceGeneral::backward_search_insensitive_case(Glib::ustring& selection, Gtk::TextBuffer::iterator& match_start, Gtk::TextBuffer::iterator& match_end)
{
	bool foundWord = false;
	bool exit;
	Gtk::TextBuffer::iterator aux_match_start, ref_match_start;
	Glib::ustring::iterator it;

	match_start = current;
	match_start.backward_char(); // search forward

	if ( match_start.is_start() )
		match_start.forward_to_end();

	// while not found parse the text widget backward
	while ( (! foundWord ) && (! match_start.is_start()) )
	{
		aux_match_start = match_start;
		it = selection.begin();
		exit = false;

		// parse the searched word from match_start iterator
		while (( ! exit ) && ( it != selection.end() ))
		{
			if ( tolower(aux_match_start.get_char()) != tolower(*it) )
				exit = true;
			else
			{
				++it;
				aux_match_start.forward_char();
			}
		}

		if ( ! exit )
		{
			foundWord = true;
			match_end = aux_match_start;
		}
		else
		{
			match_start.backward_char();
		}
	}

	current = match_start;

	return foundWord;
}



bool SearchReplaceGeneral::forward_search_insensitive_case(Glib::ustring& searched, Gtk::TextBuffer::iterator& match_start, Gtk::TextBuffer::iterator& match_end)
{
	bool foundWord = false;
	bool exit;
	Gtk::TextBuffer::iterator tmp_itext ;
	Glib::ustring::iterator it;

	match_start = current;
//	match_start.forward_char() ; // search forward

	// End text case
	if ( match_start.is_end() )
		match_start.set_line(0) ;

	// While not at text end and not match found, parse forward from match_start iterator
	while ( (! foundWord ) && (! match_start.is_end()) )
	{
		tmp_itext = match_start ;
		it = searched.begin() ;
		exit = false;

		// parse the searched word from match_start iterator
		while (( ! exit ) && ( it != searched.end() ))
		{
			// Not same character, don't need to keep on
			if ( tolower(tmp_itext.get_char()) != tolower(*it) )
				exit = true;
			// Ok, let's continue
			else {
				++it;
				tmp_itext.forward_char();
			}
		}

		if ( ! exit )
		{
			foundWord = true;
			match_end = tmp_itext;
		}
		else
			match_start.forward_char();
	}

	current = match_start ;

	return foundWord;
}


bool SearchReplaceGeneral::searchMode(const int flag_editability, const bool whole_word_allowed)
{
	if ( whole_word_allowed )
	{
		switch (search_mode)
		{
			case TT: // Text and Tags
				return true;
				break;
			case TEXT:
				return ( flag_uneditable == 0 );
				break;
			case TAGS:
				return ( flag_uneditable == 1 );
				break;
			default:
				return false;
		}
	}
	else
		return false;
}


void SearchReplaceGeneral::on_search_fw()
{
	remove_occ() ;

	combo_search.set_name("combo_search_default") ;
	lock_cursor = true ;

	load_state() ;

	get_entries() ;

	enable_replace(true) ;

	if (add_in_list(&findList, _search) )
		combo_search.add_in_list(_search) ;

	if (bak) {
		bak = false ;
		if (found)
			current = end_occur ;
	}

	if ( cb_case.get_active() )
		found = current.forward_search(_search, Gtk::TEXT_SEARCH_TEXT_ONLY , start_occur, end_occur) ;
	else
		found = forward_search_insensitive_case(_search, start_occur, end_occur) ;

	get_entries() ;
	//> if found an occurence
	if (found && start_occur.compare(end_search) < 0)
	{
      	if (start_occur.editable())
      		enable_replace(true) ;
      	else
      		enable_replace(false) ;

		 // set current position
		 current = end_occur ;
		 save_state(0) ;

		 bool whole_word_allowed = true ;
		 //> if we search only whole word, check it
		 if (whole_word_mode)
		 	whole_word_allowed = is_whole_word() ;
		 //> if all tags are editable, we can select
		 if ( searchMode(flag_uneditable, whole_word_allowed) ) {
			    select_occ() ;
	   			count++ ;
		 }
		 //> else go on for next search
		 else {
			save_state(0) ;
			on_search_fw() ;
		 }
 	}
 	else {
 		save_state(0) ;
 		found = false ;
 		search_end() ;
 	}
	get_entries() ;

	save_state(0) ;

	lock_cursor = false ;
}



void SearchReplaceGeneral::on_search_bk()
{
	remove_occ() ;

	combo_search.set_name("combo_search_default") ;
	lock_cursor = true ;

	load_state() ;

	get_entries() ;
	enable_replace(true) ;

	if (add_in_list(&findList, _search) )
		combo_search.add_in_list(_search) ;

	if (!bak) {
		bak = true ;
		if (found)
			current = start_occur ;
	}

	if ( cb_case.get_active() )
		found = current.backward_search(_search, Gtk::TEXT_SEARCH_TEXT_ONLY , start_occur, end_occur) ;
	else
		found = backward_search_insensitive_case(_search, start_occur, end_occur) ;

	get_entries() ;

	if (found && start_occur.compare(start_search) > 0)
	{
     	if (start_occur.editable())
     		enable_replace(true) ;
     	else
     		enable_replace(false) ;

     	 // set_current position
		 current = start_occur ;
		 save_state(0) ;

		 bool whole_word_allowed = true ;
		 //> if we search only whole word, check it
		 if (whole_word_mode)
		 	whole_word_allowed = is_whole_word() ;
		 //> if all tags are editable, we can select
		 if ( searchMode(flag_uneditable, whole_word_allowed) ) {
			    select_occ() ;
	   			count++ ;
		 }
		 //> else go on for next search
		 else {
			save_state(0) ;
			on_search_bk() ;
		}
	}
 	else {
 		found = false ;
 		save_state(0) ;
 		search_end() ;
 	}
	get_entries() ;

	save_state(0) ;

	lock_cursor = false ;
}

bool SearchReplaceGeneral::on_replace(bool user_action)
{
	if (flag_uneditable==1)
		return false ;

	lock_cursor = true ;
	load_state() ;

	if (add_in_list(&replaceList, _replace) )
		combo_replace.add_in_list(_replace) ;

 	Glib::ustring selection = get_external_buffer()->get_slice(start_occur, end_occur, false);

 	bool replace = (selection==_search && start_occur.editable()) ;

 	if ( replace )
 	{
 		if ( (whole_word_mode&&is_whole_word()) || !whole_word_mode ) {
 	 		bool isEndScope = (end_search==end_occur) ;
	 		save_state(0) ;
			Gtk::TextBuffer::iterator tmp ;
			//> replace text
			tmp = get_external_view()->getBuffer()->replaceText(start_occur,end_occur, _replace, user_action) ;
 			load_state() ;
			//> get new end occurence, usefull to well hilight, and to next search
 			end_occur=tmp ;
			//> if selection scope ended
 			if (isEndScope&&hasSelection) {
				end_search = end_occur ;
				get_external_buffer()->apply_tag(dialogTag_sel, start_search,end_search) ;
			}
			save_state(0) ;
			select_occ() ;
 		}
	}
 	else
 		save_state(0) ;

 	lock_cursor = false ;

	return replace ;
}

void SearchReplaceGeneral::on_replace_find(bool backDir)
{
	if (!backDir) {
		on_replace(true) ;
		on_search_fw() ;
	}
	else {
		on_replace(true) ;
		on_search_bk() ;
	}
}

void SearchReplaceGeneral::on_replace_all()
{
	lock_cursor = true ;
	load_state() ;
	get_external_buffer()->begin_user_action() ;

	bak = false ;

	//if no selection, start from beginning
	if ( rb_sel.get_active()==false ) {
		start_search =  get_external_buffer()->begin() ;
		end_search = get_external_buffer()->end() ;
		current = start_search ;
		save_state(0) ;
		on_search_fw() ;
	}

	load_state() ;
	count_replace = 0 ;
	replace_all_mode = true ;
	//> found will be updated at false when search_fw
	// will reach end cursor
	while(found) {
		bool replace = on_replace(false) ;
		//> actualise current at end of last replacement
		load_state() ;
		current=end_occur ;
		save_state(3) ;
		if (replace)
			count_replace++ ;
		//> go ahead
		on_search_fw() ;
	}
	replace_all_mode = false ;

	lock_cursor = false ;
	get_external_buffer()->end_user_action() ;
}


void SearchReplaceGeneral::search_end()
{
	Glib::ustring txt ;
	if (count > 0 && replace_all_mode) {
		txt = number_to_string(count_replace) + " " +  _("occurrences replaced") ;
		display_info(txt) ;
	}
	else if (/*count > 0 &&*/ !bak) {
		txt = _("No more occurrences, research from the start ?") ;
		int response = display_question(txt) ;
		if (response==Gtk::RESPONSE_YES) {
			count=0 ;
			current = start_search ;
			select_occ() ;
			save_state(0) ;
			on_search_fw() ;
		}
	}
	else {
		txt = _("no occurences found") ;
		display_info(txt) ;
	}
}

void SearchReplaceGeneral::close()
{
	//desactive
	active = false ;

	//stop connection
	reset() ;

	//clear combo
	combo_language.clear() ;

	//hide
	myHide() ;
}

void SearchReplaceGeneral::reset()
{
	//remove signal
	connection_replace_all.disconnect() ;
	connection_replace.disconnect() ;
	connection_find_fw.disconnect() ;
	connection_find_bk.disconnect() ;
	connection_close_button.disconnect() ;
	connection_rb_all.disconnect() ;
	connection_rb_sel.disconnect() ;
	connection_cb_case.disconnect() ;
	connection_cb_whole.disconnect() ;
	connection_combo_replace.disconnect() ;
	connection_combo_search.disconnect() ;
	connection_change_mode.disconnect()	;

	connection_changeActiveView.disconnect() ;
	//remove last tag
	removeSelectionTag() ;
	remove_occ() ;

	//diconnect
	external_signal_buffer_changed.disconnect() ;
	external_signal_mark_set.disconnect() ;
}

void SearchReplaceGeneral::switch_file(AnnotationEditor* editor)
{
	close();
	init(editor);
}


void SearchReplaceGeneral::on_scope_change(bool selection)
{
	load_state() ;
	if (selection) {
		Gtk::TextBuffer::iterator tmp1 ;
		Gtk::TextBuffer::iterator tmp2 ;
		hasSelection = get_external_buffer()->get_selection_bounds(tmp1,tmp2) ;
		if (!hasSelection) {
			rb_all.set_active(true) ;
		}
		else {
			hasSelection = get_external_buffer()->get_selection_bounds(start_search,end_search) ;
			save_state(0) ;
			selection_mode(true) ;
		}
	}
	else if (!selection) {
		save_state(0) ;
		selection_mode(false) ;
	}
	save_state(0) ;
}

void SearchReplaceGeneral::selection_mode(bool withSelection)
{
	lock_cursor = true ;
	load_state() ;
	if (!withSelection) {
		get_external_buffer()->remove_tag(dialogTag_sel, get_external_buffer()->begin(), get_external_buffer()->end()) ;
		start_search =  get_external_buffer()->begin() ;
		end_search = get_external_buffer()->end() ;
		//current = start_search ;
		Glib::RefPtr<Gtk::TextBuffer::Mark> mark = get_external_buffer()->get_insert() ;
		current = mark->get_iter() ;
	}
	else {
		//> place current to the start of selection
		current = start_search ;
		//> erase selection
		get_external_buffer()->select_range(end_search, end_search) ;
		//> apply general tag
		get_external_buffer()->apply_tag(dialogTag_sel, start_search,end_search) ;
	}
	save_state(0) ;
	lock_cursor = false ;
}


void SearchReplaceGeneral::on_case_sensitive()
{
	if (cb_case.get_active() && case_sensitive_mode==false ) {
		case_sensitive_mode = true ;
	}
	else if (!cb_case.get_active() && case_sensitive_mode==true ){
		case_sensitive_mode = false ;
	}
}

void SearchReplaceGeneral::on_whole_word()
{
	if (cb_whole.get_active() && whole_word_mode==false ) {
		whole_word_mode = true ;
	}
	else if (!cb_whole.get_active() && whole_word_mode==true ){
		whole_word_mode = false ;
	}
}

bool SearchReplaceGeneral::is_whole_word()
{
	bool res, found_first, found_last ;
	found_first = start_occur.starts_word() ;
	found_last = end_occur.ends_word() ;
	if (found_first && found_last)
		res = true ;
	else
		res=false;
	return res ;
}

void SearchReplaceGeneral::set_findList(std::vector<Glib::ustring> l)
{
	std::vector<Glib::ustring>::iterator it = l.begin() ;
	while(it!=l.end()) {
		if (add_in_list(&findList,*it))
			combo_search.add_in_list(*it) ;
		it++ ;
	}
}

//******************************************************************************
//******************************* COMBO LANGUAGE *******************************
//******************************************************************************

void SearchReplaceGeneral::set_combo_search_mode()
{
	// to save search_mode from changeMode signal
	int searchMode = search_mode;

	combo_searchMode.clear() ;

	combo_searchMode.append_text(_("Text & Tags"));
	combo_searchMode.append_text(_("Text"));
	combo_searchMode.append_text(_("Tags"));
	combo_searchMode.set_focus_on_click(false) ;

	search_mode = searchMode;
	switch (search_mode)
	{
		case TT: // Text and Tags
			combo_searchMode.set_active_text(_("Text & Tags"));
			break;
		case TEXT:
			combo_searchMode.set_active_text(_("Text"));
			break;
		case TAGS:
			combo_searchMode.set_active_text(_("Tags"));
	}
}


void SearchReplaceGeneral::on_change_search_mode()
{
	Glib::ustring m_search_mode = combo_searchMode.get_active_text() ;

	if ( m_search_mode == _("Text & Tags") )
		search_mode = TT;
	else if ( m_search_mode == _("Text") )
		search_mode = TEXT;
	else
		search_mode = TAGS;
}


void SearchReplaceGeneral::set_combo_language()
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

void SearchReplaceGeneral::on_change_language()
{
	Glib::ustring display = combo_language.get_active_text() ;
	if (display != "")
	{
		//> actualize input language for each combo
		iLang = InputLanguageHandler::get_input_language_by_name(display);
		combo_search.set_input_language(iLang) ;
		combo_replace.set_input_language(iLang) ;

		//> check if an external IME is in used, and tell each combo
		if (iLang && iLang->getLanguageType().compare(IME_LANGUAGE)==0) {
			combo_search.setIMEstatus(true) ;
			combo_replace.setIMEstatus(true) ;
		}
		else {
			combo_search.setIMEstatus(false) ;
			combo_replace.setIMEstatus(false) ;
		}
	}
}


void SearchReplaceGeneral::check_initial_selection()
{
	//> get editor selection
	hasSelection = get_external_buffer()->get_selection_bounds(start_search,end_search) ;

	//Editor has selection
	if (hasSelection) {
		Glib::ustring selection = get_external_buffer()->get_slice(start_search, end_search) ;
		int chars_in_line = start_search.get_chars_in_line() ;
		if (selection.size()>SIMLE_SCOPE_SELECTION_LIMIT) {
			rb_sel.set_active(true) ;
			save_state(0) ;
			selection_mode(true) ;
			combo_search.force_cursor() ;
		}
		else {
			lock_cursor = true ;
			hasSelection = false ;
			start_search = get_external_buffer()->begin() ;
			end_search = get_external_buffer()->end() ;
			current = start_search ;
			_search = selection ;
			combo_search.set_text(selection) ;
			rb_all.set_active(true) ;
			save_state(0) ;
			selection_mode(false);
			lock_cursor = false ;
		}
	}
	else {
		rb_all.set_active(true) ;
		save_state(0) ;
		selection_mode(false);
		combo_search.force_cursor() ;
	}
}


void SearchReplaceGeneral::on_change_cursor(const Gtk::TextBuffer::iterator& it, const Glib::RefPtr<Gtk::TextBuffer::Mark >& mark)
{
	if (active)
	{
		//> only proceed if insert mark
		if (mark->get_name()=="insert" && !lock_cursor) {
			current = it ;
			// disable selection mode when click on text
			remove_occ() ;
			if (hasSelection)
				selection_mode(false) ;
			save_state(3) ;
		}
	}
}

void SearchReplaceGeneral::on_buffer_changed()
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

void SearchReplaceGeneral::select_occ()
{
	get_external_buffer()->apply_tag(dialogTag_occ, start_occur, end_occur) ;
	get_external_buffer()->setCursor(start_occur, true) ;
	get_external_view()->scroll_to(start_occur, 0.2) ;
}

void SearchReplaceGeneral::remove_occ()
{
	get_external_buffer()->clearTag(SEARCH_REPLACE_TAG_NAME) ;
//	get_external_buffer()->remove_tag(dialogTag_occ, start_occur, end_occur) ;
}


void SearchReplaceGeneral::enable_replace(bool can_replace)
{
	if (can_replace) {
		flag_uneditable = 0 ;
		active_replace(true) ;
	}
	else {
		flag_uneditable = 1 ;
		active_replace(false) ;
	}
}

void SearchReplaceGeneral::on_change_view()
{
//	Explorer_utils::print_trace("SearchReplaceGeneral::on_change_view:> RECEIVED", 1) ;
	external_editor->clearViewTag(SEARCH_REPLACE_SELECT_TAG_NAME) ;
	external_editor->clearViewTag(SEARCH_REPLACE_TAG_NAME) ;
	external_signal_buffer_changed.disconnect() ;
	external_signal_mark_set.disconnect() ;
	external_signal_mark_set = get_external_buffer()->signal_mark_set().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_change_cursor));
	external_signal_buffer_changed = get_external_buffer()->signal_changed().connect(sigc::mem_fun(*this, &SearchReplaceGeneral::on_buffer_changed));
	current = get_external_buffer()->get_mark("insert")->get_iter() ;
 	start_occur = get_external_buffer()->begin() ;
	end_occur = get_external_buffer()->begin() ;
	start_search = get_external_buffer()->begin() ;
	end_search = get_external_buffer()->end() ;
	save_state(0) ;
}

void SearchReplaceGeneral::prepareTag()
{
  	dialogTag_sel = get_external_buffer()->get_tag_table()->lookup(SEARCH_REPLACE_SELECT_TAG_NAME) ;
	if (!dialogTag_sel) {
		dialogTag_sel = get_external_buffer()->create_tag(SEARCH_REPLACE_SELECT_TAG_NAME);
	  	dialogTag_sel->property_foreground().set_value("#000000");
	  	dialogTag_sel->property_background().set_value("#C7C7FA");
  	}

  	dialogTag_occ = get_external_buffer()->get_tag_table()->lookup(SEARCH_REPLACE_TAG_NAME) ;
	if (!dialogTag_occ) {
		dialogTag_occ = get_external_buffer()->create_tag(SEARCH_REPLACE_TAG_NAME);
		dialogTag_occ->property_foreground().set_value("#000000");
		dialogTag_occ->property_background().set_value("#80D480");
  	}
}

void SearchReplaceGeneral::setSearchOptions(int searchMode, bool cas, bool wholeWord)
{
	// data
	search_mode = searchMode ;
	case_sensitive_mode = cas ;
	whole_word_mode = wholeWord ;

	// state
	set_combo_search_mode() ;
	cb_case.set_active(case_sensitive_mode) ;
	cb_whole.set_active(whole_word_mode) ;
}

void SearchReplaceGeneral::getSearchOptions(int& searchMode, bool& cas, bool& wholeWord)
{
	searchMode = search_mode ;
	cas = case_sensitive_mode ;
	wholeWord = whole_word_mode ;
}

} //namespace



