/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "SpeakerDico_dialog.h"
#include "Explorer_dialog.h"
#include "Common/util/FileHelper.h"
#include "Explorer_utils.h"
#include "Common/globals.h"
#include "Common/icons/Icons.h"
#include "Common/Dialogs.h"

#include <iostream>
#include <gtkmm/icontheme.h>

namespace tag {

//******************************************************************************
//									CONSTRUCTOR
//******************************************************************************

SpeakerDico_dialog::SpeakerDico_dialog(bool edit, bool modal, Gtk::Window* win)
{
	//> -- Presentation
	set_title(_("Speaker Dictionary")) ;
	set_modal(modal) ;
	if (modal && win)
		set_transient_for(*win) ;

	//> -- Initialize
	data_initialised = false ;
	data = NULL ;
	s_dico_backUp = NULL ;
	s_dico = NULL ;
	editor = NULL ;
	updated = false ;
	edition_enabled = edit ;
	m_light_mode = false ;
	lock_on_selection = false ;
	lock_show_details = false ;
	details_size = -1 ;

	//> -- Get languages list
	languages = Languages::getInstance() ;

	//> -- Prepare display
	prepareGUI() ;
	if(!edit)
		align_edit.hide() ;
}

SpeakerDico_dialog::~SpeakerDico_dialog()
{
	if(data)
	{
		delete(data) ;
		data=NULL ;
	}
	if(global || (!global && fromUrl) )
	{
		if (s_dico)
		{
			delete(s_dico) ;
			s_dico=NULL ;
		}
	}
	if(s_dico_backUp)
	{
		delete(s_dico_backUp) ;
		s_dico_backUp = NULL ;
	}
	std::map<int, Speaker*>::iterator it =  new_speakers.begin() ;
	std::map<int, int>::iterator it_state =  new_speakers_state.begin() ;
	while ( it!= new_speakers.end() )
	{
			delete(it->second) ;
		it++ ;
		it_state++ ;
	}
	if (close)
		delete(close) ;
	if (genApply)
		delete(genApply) ;
	if (genCancel)
		delete(genCancel) ;
}


//******************************************************************************
//									ARCHITECTURE
//******************************************************************************

void SpeakerDico_dialog::prepareGUI()
{
	genVBox = Gtk::manage(new Gtk::VBox()) ;
	add(*genVBox);

	//> -- Prepare view
	prepare_list() ;

	genApply = new Gtk::Button(Gtk::Stock::APPLY) ;
	genCancel = new Gtk::Button(Gtk::Stock::CANCEL) ;
	close = new Gtk::Button(Gtk::Stock::CLOSE) ;

	//> -- Structure
	Gtk::Label* blank1 = Gtk::manage(new Gtk::Label(" ")) ;
	Gtk::Label* blank2 = Gtk::manage(new Gtk::Label(" ")) ;
	genVBox->pack_start(paned) ;
			//> first panel
			paned.pack1(l_vbox, true, false) ;
				l_vbox.pack_start(l_hbox_top, false, true) ;
					l_hbox_top.pack_start(raise_button, false, false) ;
					l_hbox_top.pack_start(*blank1, true, true) ;
					l_hbox_top.pack_start(title_hbox, false, false) ;
						title_hbox.pack_start(title_image, false, false, 2) ;
						title_hbox.pack_start(title_label, false, false, 2) ;
					l_hbox_top.pack_start(*blank2, true, true) ;
					l_hbox_top.pack_start(details, false, false) ;
				l_vbox.add(hpaned) ;
					//top pan
					hpaned.pack1(scrollW, true, false) ;
						scrollW.add(l_listView) ;
					//bottom pan
					hpaned.pack2(l_vbox_bottom, false, false) ;
						l_vbox_bottom.pack_start(align_edit, false, false) ;
							align_edit.set(Gtk::ALIGN_CENTER, 1, 1) ;
							align_edit.add(l_hbox_bottom2) ;
								l_hbox_bottom2.pack_start(addSpeakerButton, false, true) ;
								l_hbox_bottom2.pack_start(removeSpeakerButton, false, true) ;
	genVBox->pack_start(button_align, false, false, 4) ;
		button_align.add(button_box);
		button_align.set(Gtk::ALIGN_RIGHT, 0, 0) ;
			button_box.pack_start(*genApply, false, false, 3) ;
			button_box.pack_start(*genCancel, false, false, 3) ;
			button_box.pack_start(*close, false, false, 3) ;


	//> add end dialog button and signal
	genCancel->signal_clicked().connect( sigc::bind<bool,int>(sigc::mem_fun(*this, &SpeakerDico_dialog::on_updateDictionary), false, Gtk::RESPONSE_CANCEL)) ;
	genApply->signal_clicked().connect( sigc::bind<bool,int>(sigc::mem_fun(*this, &SpeakerDico_dialog::on_updateDictionary), true, Gtk::RESPONSE_APPLY)) ;
	close->signal_clicked().connect( sigc::mem_fun(*this, &SpeakerDico_dialog::close_dialog_wrap)) ;
	signal_delete_event().connect( sigc::mem_fun(*this, &SpeakerDico_dialog::on_response) ) ;

	align_edit.set_border_width(3) ;
	scrollW.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	set_buttons() ;

	//> at starting, no row selected, so no row selection buttons available
	selection_buttons_state(true) ;

	//> show all
	show_all() ;
	genVBox->show_all_children() ;

	//> default option, no general commands
	genApply->hide() ;
	genCancel->hide() ;
	genApply->set_sensitive(false) ;
	genCancel->set_sensitive(false) ;
}


void SpeakerDico_dialog::prepare_list()
{
	// -- Create model
	refModel = Gtk::ListStore::create(columns) ;
	refSortedModel = Gtk::TreeModelSort::create(refModel) ;
	l_listView.set_model(refSortedModel) ;

	// -- Prepare tree view
	l_listView.set_rules_hint(true) ;
	l_listView.set_headers_clickable(true) ;
	l_listView.set_enable_search(true) ;
	l_listView.set_search_column(columns.a_lastName) ;
	l_listView.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE) ;

	int col_num ;
	Gtk::TreeViewColumn* column ;
	std::vector<int> col ;

	//> -- Append column
	col_num = l_listView.append_column(_("Last Name"), columns.a_lastName) ;
		col.insert(col.end(), col_num) ;
	col_num = l_listView.append_column(_("First Name"), columns.a_firstName) ;
		col.insert(col.end(), col_num) ;
	col_num = l_listView.append_column(_("Language"), columns.a_lang) ;
		col.insert(col.end(), col_num) ;
	col_num = l_listView.append_column(_("Description"), columns.a_desc) ;
		col.insert(col.end(), col_num) ;

	//> -- For each column
	for (guint i=0; i<col.size(); i++)
	{
		col_num = col[i] ;
		//> Here it seems that the get_column function uses column numbers starting
		//  from 0, whereas the append_column gives number columns starting from 1
		column = l_listView.get_column(col_num-1) ;
		if (column)
		{
			//> enable header to be clicked
			column->set_clickable(true) ;
			//> connect signal emitted when the column header is clicked
			column->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &SpeakerDico_dialog::on_header_column_clicked),col_num)) ;
		}
	}

	//> -- Connection to signals
	l_listView.signalSelection().connect(sigc::mem_fun(*this, &SpeakerDico_dialog::on_selection)) ;
	l_listView.signal_row_activated().connect(sigc::mem_fun(*this, &SpeakerDico_dialog::on_row_activated)) ;

	//> -- Preparation for sorting
	// (!) WHEN ADDING NEW SORT COLUMN, need to modify the mapping in callback "on_header_column_clicked"
	refSortedModel->set_sort_func(SPEAKER_COL_ID, sigc::mem_fun(*this, &SpeakerDico_dialog::on_list_sort)) ;
	refSortedModel->set_sort_func(SPEAKER_COL_FNAME, sigc::mem_fun(*this, &SpeakerDico_dialog::on_list_sort)) ;
	refSortedModel->set_sort_func(SPEAKER_COL_LNAME, sigc::mem_fun(*this, &SpeakerDico_dialog::on_list_sort)) ;
	refSortedModel->set_sort_func(SPEAKER_COL_LANGUAGE, sigc::mem_fun(*this, &SpeakerDico_dialog::on_list_sort)) ;
	refSortedModel->set_sort_func(SPEAKER_COL_DESC, sigc::mem_fun(*this, &SpeakerDico_dialog::on_list_sort)) ;

	//> -- Default sorting: on LastName
	refSortedModel->set_sort_column(SPEAKER_COL_LNAME, Gtk::SORT_ASCENDING) ;
}

void SpeakerDico_dialog::pack_speaker_frame(Gtk::Frame* sd)
{
	paned.pack2(*data, true, false) ;
}

Gtk::Frame* SpeakerDico_dialog::get_speaker_frame()
{
	paned.get_child2() ;
}


//******************************************************************************
//									OPENING
//******************************************************************************

void SpeakerDico_dialog::open_dictionary(Glib::ustring _url)
{
	// -- Initialization
	global = true ;
	fromUrl = true ;
	global_dico_path = _url ;
	current_file = "" ;
	new_speakers_indice = 0 ;

	// -- Clear old model if exists
	refModel->clear() ;

	// -- Display
	bool iconified ;
	Glib::RefPtr<Gdk::Pixbuf>pixbufPlay = Icons::create_pixbuf(ICO_SPEAKER_DICO_GLOBAL, 60, iconified) ;
	if (iconified)
		set_icon(pixbufPlay) ;
	else
		Log::err() << "Speaker dictionary : display error : unable to load window icon" << std::endl ;

	string title = _("GLOBAL dictionary") ;
	set_title(title) ;
	title_image.set_icon(ICO_SPEAKER_DICO_GLOBAL, "", 18, title) ;
	title_label.set_label(title) ;
	title_label.set_name("bold_label") ;

	// -- Opening
	s_dico_backUp = new SpeakerDictionary() ;
	s_dico_backUp->loadDictionary(global_dico_path) ;
	s_dico = new SpeakerDictionary(*s_dico_backUp) ;

	// -- Buttons
	genApply->show() ;
	genCancel->show() ;
	raise_button.set_icon(ICO_SPEAKER_DICO_LOCAL, "", 18, _("Raise current file dictionary")) ;
	raise_button.signal_released().connect(sigc::mem_fun(this, &SpeakerDico_dialog::onRaiseButtonReleased)) ;
	details.set_icon(ICO_SPEAKER_DETAILS, "", 17, _("Show details")) ;

	// -- Model
	int nb  = fill_model() ;
	if ( nb>0)
	{
		Gtk::TreeIter iter = refModel->children().begin() ;
		Gtk::TreePath path = refModel->get_path(iter) ;
		l_listView.set_cursor(path);
		prepare_datas_panel(iter) ;
		lock_show_details = true ;
		details.set_active(true) ;
		lock_show_details = false ;
		on_show_details() ;
		set_focus(l_listView) ;
	}
	else
	{
		details.set_sensitive(false) ;
		removeSpeakerButton.set_sensitive(false) ;
		if (m_light_mode)
			choose_speaker.set_sensitive(false) ;
	}
}

void SpeakerDico_dialog::open_dictionary(AnnotationEditor* _editor)
{
	global = false ;
	fromUrl = false ;
	editor = _editor ;
	refModel->clear() ;
	current_file = editor->getFileName() ;
	global_dico_path = "" ;
	new_speakers_indice = 0 ;

	// -- Display
	bool iconified ;
	Glib::RefPtr<Gdk::Pixbuf>pixbufPlay = Icons::create_pixbuf(ICO_SPEAKER_DICO_LOCAL, 60, iconified) ;
	if (iconified)
		set_icon(pixbufPlay) ;
	else
		Log::err() << "Speaker dictionary : display error : unable to load window icon" << std::endl ;
	Glib::ustring title = _("Local dictionary") ;
	Glib::ustring title_path = title ;
	title_path.append(" ") ;
	title_path.append(current_file) ;
	set_title(title_path) ;
	title_image.set_icon(ICO_SPEAKER_DICO_LOCAL, "", 18, title_path) ;
	title_label.set_label(title) ;
	title_label.set_name("bold_label") ;
	title_label.set_tooltip_text(title_path) ;

	// -- Opening
	s_dico = &(editor->getSpeakerDictionary()) ;

	// -- Button
	raise_button.set_icon(ICO_SPEAKER_DICO_GLOBAL, "", 18, _("Raise global dictionary")) ;
	raise_button.signal_released().connect(sigc::mem_fun(this, &SpeakerDico_dialog::onRaiseButtonReleased)) ;
	details.set_icon(ICO_SPEAKER_DETAILS, "", 17, _("Show details")) ;

	// -- Model
	int nb  = fill_model() ;
	if ( nb>0)
	{
		Gtk::TreeIter iter = refModel->children().begin() ;
		Gtk::TreePath path = refModel->get_path(iter) ;
		l_listView.set_cursor(path);
		prepare_datas_panel(iter) ;
		lock_show_details = true ;
		details.set_active(true) ;
		lock_show_details = false ;
		on_show_details() ;
		set_focus(l_listView) ;
	}
	else
	{
		details.set_sensitive(false) ;
		removeSpeakerButton.set_sensitive(false) ;
		if (m_light_mode)
			choose_speaker.set_sensitive(false) ;
	}
}



//******************************************************************************
//									BUTTONS
//******************************************************************************

void SpeakerDico_dialog::set_buttons()
{
	//> set label
	details.set_use_underline(true) ;

	Glib::ustring tmp = _("_Add a speaker") ;
	addSpeakerButton.set_icon(ICO_SPEAKER_ADDSPEAKER, tmp, 15, _("add a new speaker")) ;
	addSpeakerButton.set_use_underline(true) ;
	tmp = _("_Remove speaker") ;
	removeSpeakerButton.set_icon(ICO_SPEAKER_REMOVESPEAKER, tmp, 15, _("remove the selected speaker")) ;
	removeSpeakerButton.set_use_underline(true) ;

	//> connect to corresponding actions
	details.signal_toggled().connect( sigc::mem_fun(*this, &SpeakerDico_dialog::on_show_details) ) ;
	addSpeakerButton.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SpeakerDico_dialog::on_edit), "adding")) ;
	removeSpeakerButton.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SpeakerDico_dialog::on_edit), "removing")) ;
}

void SpeakerDico_dialog::selection_buttons_state(bool activated)
{
	details.set_sensitive(activated) ;
	removeSpeakerButton.set_sensitive(activated) ;
}

void SpeakerDico_dialog::on_show_details()
{
	if (lock_show_details)
		return ;

	//> -- Treat details button
	if (details.get_active())
	{
		if (get_speaker_frame())
			get_speaker_frame()->show() ;
		if (details_size != -1)
			resize(get_width()+details_size+1, get_height()) ;
		details.set_icon(ICO_SPEAKER_DETAILS, "", 17, _("Hide details")) ;
	}
	else
	{
		//> compute size for simulate details removing
		int pos = paned.get_position() ;
		int w = get_width() ;
		details_size = w - pos ;
		resize(pos, get_height()) ;
		if (get_speaker_frame())
			get_speaker_frame()->hide() ;
		details.set_icon(ICO_SPEAKER_DETAILS, "", 17, _("Show details")) ;
	}
}

void SpeakerDico_dialog::on_signal_modified()
{
	genApply->set_sensitive(true) ;
	genCancel->set_sensitive(true) ;
	updated = true ;
}

//******************************************************************************
//									DICO BUSINESS
//******************************************************************************

int SpeakerDico_dialog::deleteSpeaker(Glib::ustring id)
{
	//> delete from dictionary
	bool res = s_dico->deleteSpeaker(id) ;

	if (res) {
		//> delete from view
		get_current_path_as_activeIter(active_path) ;
		Gtk::TreeIter iterator = refSortedModel->convert_iter_to_child_iter(active_iter) ;
		refModel->erase(iterator) ;
		//> actualize file
		return 1 ;
	}
	else
		return -1 ;
}

int SpeakerDico_dialog::replaceSpeaker(Glib::ustring id, Glib::ustring global_src_id)
{
	// Update in dictionary
	current_speaker->setId(id) ;
	// Add old global-id if available
	if (!global_src_id.empty())
		current_speaker->setScope(global_src_id) ;
	int res = updateSpeaker(id) ;
	if (res<0)
		Explorer_dialog::msg_dialog_error(_("Error while replacing speaker"), this, true) ;
	else {
		l_listView.set_cursor(active_path) ;
	}
	return 1 ;
}

void SpeakerDico_dialog::addSpeaker(bool changeId, Glib::ustring global_src_id)
{
	addSpeakerButton.set_sensitive(true) ;
	if (data)
	{
		data->switch_to_add(false) ;
		data->set_has_changed(false) ;
	}

	std::vector<Gtk::TreeIter> v ;
	Glib::ustring id ;

	//> -- Prepare a new id
	if (changeId)
	{
		Glib::ustring id = s_dico->getUniqueId() ;
		current_speaker->setId(id) ;
	}
	//> -- Keep former one
	else
		id = current_speaker->getId() ;

	int rep = Gtk::RESPONSE_YES ;

	//> -- Check for similar speaker (smae first & last name)
	check_speaker(*current_speaker, &v) ;
	guint size =v.size() ;
	if (size!=0)
	{
		std::vector<Gtk::TreeIter>::iterator it = v.begin() ;
		while (it!=v.end())
		{
			l_listView.get_selection()->select(*it) ;
			it++ ;
		}
		Glib::ustring txt ;
		if (size==1)
			txt = _("An existing speaker seems to correspond to the one you want to add\nDo you want to continue ?") ;
		else if (size>1)
			txt = _("Existing speakers seem to correspond to the one you want to add\nDo you want to continue ?") ;

		rep = Explorer_dialog::msg_dialog_question(txt,this,true, "") ;
	}

	//> -- Replace
	if (rep==Gtk::RESPONSE_YES)
	{
		// add old global id in specific scope field
		if (!global_src_id.empty())
			current_speaker->setScope(global_src_id) ;

		// add in model
		bool res = s_dico->updateSpeaker(*current_speaker, true) ;

		// add in view
		Gtk::TreeIter iter ;
		Gtk::TreeRow row ;
		if (res)
		{
			//> add in GUI
			iter = refModel->append() ;
			row =*iter ;
			columns.fill_row(&row,current_speaker, languages) ;
			//> actualise file
			Glib::ustring id = Glib::ustring( current_speaker->getId() ) ;
			new_speakers_state[new_speakers_indice-1] = 1 ;
			//> actualise GUI
			genApply->set_sensitive(true) ;
			genCancel->set_sensitive(true) ;
			updated = true ;
			//> set selection
			Gtk::TreeIter filteredIter = refSortedModel->convert_child_iter_to_iter(iter) ;
			Gtk::TreePath path = refSortedModel->get_path(filteredIter) ;
			l_listView.set_cursor(path) ;
		}
		else
		{
			Explorer_dialog::msg_dialog_error(_("Error while adding speaker"), this, true);
			updated = false ;
		}
	}

	if (!details.get_active())
		details.set_active(true) ;
}

int SpeakerDico_dialog::updateSpeaker(Glib::ustring id)
{
	int ret ;
	bool res = s_dico->updateSpeaker(*current_speaker, false) ;
	if (res) {
		get_current_path_as_activeIter(active_path) ;
		Gtk::TreeIter iterator = refSortedModel->convert_iter_to_child_iter(active_iter) ;
		Gtk::TreeRow row = **iterator ;
		columns.fill_row(&row, current_speaker, languages) ;
		ret = 1 ;
	}
	else {
		ret = -1 ;
	}
	return ret ;
}

void SpeakerDico_dialog::check_speaker(Speaker speaker, std::vector<Gtk::TreeIter>* list)
{
	ListModel_Columns c ;
	Glib::ustring last_name = speaker.getLastName() ;
	Glib::ustring first_name = speaker.getFirstName() ;

	Gtk::TreeModel::Children children = refSortedModel->children() ;
	Gtk::TreeIter i = children.begin();
	while ( i!=children.end())
	{
		Glib::ustring last = (*i)[c.a_lastName] ;
		Glib::ustring first = (*i)[c.a_firstName] ;
		if(last == last_name && first==first_name)
		{
			list->insert(list->end(), i) ;
		}
		i++ ;
	}
}

/*
 * Execute ations launched by user such as removing, updating or adding speaker
 */
void SpeakerDico_dialog::on_edit(Glib::ustring options)
{
	if (data && data->get_hasChanged() && (options.compare("updating")!=0) )  {
		Glib::ustring text = _("Current speaker has been modified, continue ?") ;
		int res = Explorer_dialog::msg_dialog_question(text, this, true, _("Modifications not validated will be lost")) ;
		if (res==Gtk::RESPONSE_NO)
			return ;
	}

	//DO ACTION
	Glib::ustring txt = "" ;

	if (options=="removing") {
		Glib::ustring text ;
		bool is_used = false ;
		if (!global)
			is_used = editor->getDataModel().speakerInUse(active_id) ;
		//in use
		if (is_used) {
			text = _("This speaker is used in the annotation file, can't remove it") ;
			Explorer_dialog::msg_dialog_warning(text, this, true);
		}
		//can remove
		else {
			text = _("Are you sure to delete the selected speaker ?") ;
			int ok = Explorer_dialog::msg_dialog_question(text, this, true, "");
			if (ok==Gtk::RESPONSE_YES) {
				int res = deleteSpeaker(active_id) ;
				if (res==-1)
					txt = _("error while deleting speaker") ;
				else {
					//if removing last speaker
					if (refSortedModel->children().size()==0) {
						lock_show_details = true ;
						details.set_active(false) ;
						lock_show_details = false ;
						on_show_details() ;
						details.set_sensitive(false) ;
					}
					removeSpeakerButton.set_sensitive(false) ;
					genApply->set_sensitive(true) ;
					genCancel->set_sensitive(true) ;
					updated = true ;
				}
			}
		}
	}
	else if (options=="adding")
	{
		addSpeakerButton.set_sensitive(false) ;

		current_speaker = new Speaker() ;
		Glib::ustring id = s_dico->getUniqueId() ;
		current_speaker->setId(id) ;

		//temporary stock
		new_speakers[new_speakers_indice] = current_speaker ;
		new_speakers_state[new_speakers_indice] = 0 ;
		new_speakers_indice ++ ;

		if (data_initialised)
			data->changeSpeaker(current_speaker) ;
		else
		{
			data = new SpeakerData(this, current_speaker, languages, true, edition_enabled, global) ;
			initializeDataPanel(data) ;
		}
		//> active second panel
		lock_show_details = true ;
		details.set_active(true) ;
		lock_show_details = false ;
		details.set_sensitive(true) ;
		on_show_details() ;
		//> prepare adding mode
		data->switch_to_add() ;
	}
	else if (options=="updating") {
		int res = updateSpeaker(active_id) ;
		if (res==-1)
			txt = _("error while updating speaker") ;
	}

	//DISPLAY MESSAGE
	if (txt!="")
		Explorer_dialog::msg_dialog_error(txt, this, true) ;

}

/*
 * Callback to Apply or Cancel general buttons
 */
void SpeakerDico_dialog::on_updateDictionary(bool update, int response_id)
{
	//USER SAVE MODIFICATIONS
	if (response_id==Gtk::RESPONSE_APPLY) {
		if (!FileHelper::is_writable(global_dico_path)) {
			Explorer_dialog::msg_dialog_warning(_("Please check dictionary permissions"), this, true) ;
		}
		else {
			bool res = s_dico->saveDictionary(global_dico_path) ;
			if (!res)
				Explorer_dialog::msg_dialog_error(_("Error while saving dictionary"), this, true) ;
			else {
				updated = false ;
				genCancel->set_sensitive(false) ;
				genApply->set_sensitive(false) ;
				if (data)
					data->set_has_changed(false) ;
			}
		}
	}
	else if (response_id==Gtk::RESPONSE_CANCEL) {
		open_dictionary(global_dico_path) ;
		updated = false ;
		genCancel->set_sensitive(false) ;
		genApply->set_sensitive(false) ;
		if (data)
			data->set_has_changed(false) ;
	}
}

void SpeakerDico_dialog::set_light_mode()
{
	if (m_light_mode)
		return ;

	m_light_mode = true ;

	lock_on_selection = m_light_mode ;
	lock_show_details = m_light_mode ;

	Gtk::Widget* w = paned.get_child2() ;
	if (w)
		w->hide() ;
	genApply->hide() ;
	genCancel->hide() ;
	addSpeakerButton.hide() ;
	removeSpeakerButton.hide() ;
	details.hide() ;

	choose_speaker.set_icon(ICO_PREFERENCES_APPLY, _("Select speaker"), 12, "");
	choose_speaker.show() ;
	button_box.pack_start(choose_speaker, false, false, 3) ;
	choose_speaker.signal_clicked().connect( sigc::mem_fun(*this, &SpeakerDico_dialog::on_select_speaker) ) ;
}

void SpeakerDico_dialog::allowRaiseOption(bool value)
{
	if (value)
		raise_button.show() ;
	else
		raise_button.hide() ;
}

//******************************************************************************
//										SPEAKER PANEL
//******************************************************************************

void SpeakerDico_dialog::prepare_datas_panel(Gtk::TreeIter iter)
{
	if (!refSortedModel->iter_is_valid(iter))
		return ;

	//> sotck current iterator
	stock_current_iter_as_activePath(iter) ;

	//> now a row is selected, so selection options are available
	selection_buttons_state(true) ;

	//> retrieve speaker
	active_id = (*iter)[columns.a_id] ;

	current_speaker = new Speaker( s_dico->getSpeaker(active_id) ) ;

	//> load speaker data and display it
	if (!data_initialised)
	{
		data = new SpeakerData(this, current_speaker, languages, false, edition_enabled, global) ;
		initializeDataPanel(data) ;
	}
	else
		data->changeSpeaker(current_speaker) ;
	data->switch_to_add(false) ;
}


void SpeakerDico_dialog::initializeDataPanel(SpeakerData* data)
{
	if (!data)
		return ;

	data_initialised = true ;

	// -- Pack
	pack_speaker_frame(data) ;

	// -- Drag n drop configuration
	data->drag_dest_set(target_list,Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
	data->signalOnDrop().connect(sigc::mem_fun(*this, &SpeakerDico_dialog::on_data_drop))  ;

	// -- Speaker management signal
	data->signalSpeakerUpdated().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &SpeakerDico_dialog::on_edit), "updating")) ;
	data->signalSpeakerAdded().connect(sigc::bind<bool,Glib::ustring>(sigc::mem_fun(*this, &SpeakerDico_dialog::addSpeaker), false, ""));

	// -- Status signal
	data->signalModified().connect(sigc::mem_fun(*this, &SpeakerDico_dialog::on_signal_modified));
}

void SpeakerDico_dialog::onRaiseButtonReleased()
{
	string toCall ;
	if (global)
		toCall = "local" ;
	else
		toCall = "global" ;

	signalRaiseDictionary().emit(toCall) ;
}


//******************************************************************************
//										TREE BUSINESS
//******************************************************************************

/*
 * On selection actualise current data in second panel
 */
void SpeakerDico_dialog::on_selection(std::vector<Gtk::TreeIter> _paths)
{
	//> lock : go out
	if (lock_on_selection && !m_light_mode)
		return ;

	paths = _paths ;
	Gtk::TreeIter iter, old_iter ;
	guint size = _paths.size() ;

	//> get selection, or first row if multiple selection
	if (size == 1) {
		iter = _paths[0] ;
	}
	else {
		iter = _paths[size-1] ;
	}

	//> for light mode stock iter
	if (m_light_mode) {
		stock_current_iter_as_activePath(iter) ;
		return ;
	}

	if ( data && data->is_adding_mode() ) {
		Glib::ustring text = _("New speaker has not been validated, continue edition ?") ;
		Glib::ustring text2 = _("if you choose NO, new speaker won't be added") ;
		int res = Explorer_dialog::msg_dialog_question(text, this, true, text2) ;
		if (res==Gtk::RESPONSE_NO) {
			prepare_datas_panel(iter) ;
			addSpeakerButton.set_sensitive(true) ;
		}
	}
	//> check if current modification
	else if (data && data->get_hasChanged())
	{
		Glib::ustring question = _("Warning") ;
		Glib::ustring txt = _("Current speaker has been modified, do you want to validate modifications ?") ;
		//int res = Explorer_dialog::msg_dialog_question(question, this, true, txt) ;
		dlg::Confirmsg3 dialog(question, *this, txt, true);
		bool change = false ;
		Gtk::TreePath path ;
		switch ( dialog.run() ) {
			//> save and change for new speaker selected if saved ok
			case Gtk::RESPONSE_YES:
				path = refSortedModel->get_path(iter) ;
				change = data->save() ;
				iter = refSortedModel->get_iter(path) ;
				break;
			//> don't save and change for new speaker selected
			case Gtk::RESPONSE_NO:
				change = true ;
				break ;
			//> don't save and rollback to current speaker
			case Gtk::RESPONSE_CANCEL:
				change = false ;
				break ;
		}
		//> change to selected speaker
		if (change)
			prepare_datas_panel(iter) ;
		//> rollback to previous speaker
		else {
			Glib::ustring id = data->get_speaker() ;
			if (id.compare("")!=0) {
				lock_on_selection = true ;
				set_cursor_to_speaker(id) ;
				lock_on_selection = false ;
			}
		}
	}
	//> first initialisation
	else {
		prepare_datas_panel(iter) ;
	}
}

int SpeakerDico_dialog::fill_model()
{
	int cpt = 0 ;
	SpeakerDictionary::iterator it = s_dico->begin() ;
	Gtk::TreeRow tmp ;
	while ( it!=s_dico->end() )
	{
		tmp = *(refModel->append()) ;
		columns.fill_row(&tmp, &(it->second), languages) ;
		it++ ;
		cpt++;
	}
	return cpt ;
}


/**
 *  Callback called each time the sort function is called
 */
int SpeakerDico_dialog::on_list_sort(const Gtk::TreeModel::iterator& it1, const Gtk::TreeModel::iterator& it2)
{
	int res_return, column, res_compare ;
	Glib::ustring tmp1, tmp2 ;
	Gtk::SortType type ;

	//get sort column
	refSortedModel->get_sort_column_id(column, type) ;

	//get first value
	(*(it1)).get_value(column, tmp1) ;

	//get second
	(*(it2)).get_value(column, tmp2) ;

	//compare
	res_compare = tmp1.compare(tmp2) ;

	if (res_compare < 0)
		res_return = -1 ;
	else if (res_compare==0)
		res_return =  0 ;
	else
		res_return = 1 ;
	return res_return ;
}

void SpeakerDico_dialog::on_header_column_clicked(int col)
{
	//> altern is used to determined the sort direction
	int static altern = 1 ;

	//>receive the number column relative to the view, we need to match
	// it with the number columns of model ListModel_Columns
	int model_column = -1 ;
	switch(col) {
		case 1 :  model_column = SPEAKER_COL_LNAME ; break ;
		case 2 :  model_column = SPEAKER_COL_FNAME ; break ;
		case 3 :  model_column = SPEAKER_COL_LANGUAGE ; break ;
		case 4 :  model_column = SPEAKER_COL_DESC ; break ;
	}

	//> alternatively order ascending or descending
	if (altern%2!=0)
		refSortedModel->set_sort_column(model_column, Gtk::SORT_ASCENDING) ;
	else
		refSortedModel->set_sort_column(model_column, Gtk::SORT_DESCENDING) ;

	altern ++ ;
}

void SpeakerDico_dialog::on_select_speaker()
{
	get_current_path_as_activeIter(active_path) ;
	active_id = (*active_iter)[columns.a_id] ;
	current_speaker = new Speaker( s_dico->getSpeaker(active_id) ) ;
	m_signalSelectedSpeaker.emit(active_id) ;
	close_dialog(true) ;
}

void SpeakerDico_dialog::on_row_activated(const Gtk::TreeModel::Path& p, Gtk::TreeViewColumn* c)
{
	if (m_light_mode) {
		on_select_speaker() ;
	}
	if (data && data_initialised) {
		data->my_grab_focus() ;
	}
}

void SpeakerDico_dialog::stock_current_iter_as_activePath(Gtk::TreeIter iter)
{
	Gtk::TreeIter parent = refSortedModel->convert_iter_to_child_iter(iter) ;
	active_path = refModel->get_path(parent) ;
}

void SpeakerDico_dialog::get_current_path_as_activeIter(Gtk::TreePath path)
{
	Gtk::TreeIter parent = refModel->get_iter(path) ;
	active_iter = refSortedModel->convert_child_iter_to_iter(parent) ;
}

void SpeakerDico_dialog::set_cursor_to_speaker(Glib::ustring id)
{
	if (id != "no_speech")
	{
		Gtk::TreeModel::Children children = refSortedModel->children() ;
		Gtk::TreeIter it = children.begin();
		bool found = false ;
		while ( it!=children.end() && !found) {
			Gtk::TreeIter tmp_iter = it ;
			if ( (*tmp_iter)[columns.a_id]==id ) {
				found = true ;
				Gtk::TreePath path = refSortedModel->get_path(tmp_iter) ;
				l_listView.set_cursor(path);
			}
			it++ ;
		}

		if ( !found )
			Explorer_dialog::msg_dialog_warning(_("User not found"), this, true) ;
	}
}


//******************************************************************************
//										DRAG'N'DROP
//******************************************************************************

void SpeakerDico_dialog::setDragAndDropTarget(std::vector<Gtk::TargetEntry> targetList)
{
	target_list = targetList ;
	l_listView.drag_dest_set(target_list,Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
	l_listView.drag_source_set(target_list,Gdk::MODIFIER_MASK,Gdk::ACTION_MOVE) ;
	l_listView.drag_source_set_icon(ICO_SPEAKER_SPEAKER) ;
}

void SpeakerDico_dialog::on_data_drop()
{
	if (!global && edition_enabled){
		int res = Explorer_dialog::msg_dialog_question(_("Do you want to replace this speaker ?"), this, true, "") ;
		if (res==Gtk::RESPONSE_YES)
			m_signalDropOnData.emit() ;
	}
	else if (global) {
		gdk_beep() ;
		Explorer_dialog::msg_dialog_info(_("Functionality not available in global dictionary"), this, true) ;
	}
	else
		gdk_beep() ;
}

//******************************************************************************
//										CLOSING
//******************************************************************************

void SpeakerDico_dialog::close_dialog_wrap()
{
	close_dialog() ;
}

bool SpeakerDico_dialog::close_dialog(bool keep_speaker_selection)
{
	bool close = true ;

	if (is_global() && is_updated()) {
		Glib::ustring title = _("Confirm") ;
		Glib::ustring question = _("Global speaker dictionary has been modified, do you want to save it before closing ?") ;
		dlg::Confirmsg3 dialog(title, *this, question, true);
		switch ( dialog.run() ) {
			case Gtk::RESPONSE_YES:
				save();
				break;
			case Gtk::RESPONSE_NO:
				break ;
			case Gtk::RESPONSE_CANCEL:
				close = false ;
				break;
		}
	}

	if (close) {
		//> get position before hiding !
		if (!keep_speaker_selection)
			current_speaker = NULL ;
		saveGeoAndHide() ;
		signalClose().emit() ;
	}

	return close ;
}

bool SpeakerDico_dialog::on_response(GdkEventAny* event)
{
	close_dialog() ;
}

//******************************************************************************
//									GEOMETRY INTERFACE
//******************************************************************************

void SpeakerDico_dialog::getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	get_position(pos_x, pos_y) ;
	panel = paned.get_position() ;
	get_size(size_xx, size_yy) ;
}

void SpeakerDico_dialog::setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel)
{
	if (pos_x>0 && pos_y>0)
		move(pos_x, pos_y) ;
	paned.set_position(panel) ;
	if (size_xx>0 && size_yy>0)
		resize(size_xx, size_yy) ;
}

Glib::ustring SpeakerDico_dialog::getWindowTagType()
{
	if (global)
		return SETTINGS_DICOG_NAME ;
	else
		return SETTINGS_DICOF_NAME ;
}

int SpeakerDico_dialog::loadGeoAndDisplay(bool rundlg)
{
	loadPos() ;
	show() ;
	return 1 ;
}


void SpeakerDico_dialog::saveGeoAndHide()
{
	if (is_visible())
		savePos() ;
	hide() ;
}

void SpeakerDico_dialog::getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) {}

} //namespace
