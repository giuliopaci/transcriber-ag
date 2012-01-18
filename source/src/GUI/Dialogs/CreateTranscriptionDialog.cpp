/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "CreateTranscriptionDialog.h"
#include "Explorer_dialog.h"
#include "Explorer_utils.h"
#include "Common/icons/Icons.h"

namespace tag {


//******************************************************************************
//********************************** AUDIO LINE*********************************
//******************************************************************************

CreateTranscriptionDialog::AudioLine::AudioLine(Glib::ustring path, int number,
			const std::vector<Glib::ustring>& extensions, Glib::ustring& _last_selected)
:last_selected(_last_selected)
{
	chooser = Gtk::manage(new Gtk::FileChooserButton(Gtk::FILE_CHOOSER_ACTION_OPEN)) ;

	pack_start(number_label, false, false, 5) ;
	pack_start(entry, false, false, 5) ;
	pack_start(*chooser, false, false, 5) ;

	number_label.set_label(number_to_string(number)) ;
	entry.set_text(path) ;
	entry.set_editable(false) ;
	entry.set_size_request(400, -1) ;
	chooser->signal_selection_changed().connect(sigc::mem_fun(this, &AudioLine::on_file_chosen)) ;

	if ( !last_selected.empty()) {
		Glib::ustring tmp = last_selected ;
		Glib::ustring dir = Glib::path_get_dirname(tmp) ;
		chooser->set_current_folder(dir) ;
	}

	Gtk::FileFilter* filter_AUDIO = Gtk::manage(new Gtk::FileFilter()) ;
	filter_AUDIO->set_name("Supported Audio files");
	if (extensions.size()==0) {
		filter_AUDIO->add_pattern("*.wav");
		filter_AUDIO->add_pattern("*.WAV");
	}
	else {
		std::vector<Glib::ustring>::const_iterator it ;
		for (it=extensions.begin(); it!= extensions.end(); it++) {
			filter_AUDIO->add_pattern( "*" + (*it).uppercase() ) ;
			filter_AUDIO->add_pattern( "*" + (*it).lowercase() ) ;
		}
	}
	chooser->set_filter(*filter_AUDIO) ;
	show_all_children(true) ;
}


void CreateTranscriptionDialog::AudioLine::on_file_chosen()
{
	Glib::ustring path = chooser->get_filename() ;
//	if ( Glib::file_test(path, Glib::FILE_TEST_EXISTS)  ) {
		entry.set_text(path) ;
		last_selected = path ;
//	}
//	else
//		Explorer_dialog::msg_dialog_error(_("Can't find this file"), NULL, true) ;
}

//******************************************************************************
//************************************ DIALOG **********************************
//******************************************************************************

CreateTranscriptionDialog::CreateTranscriptionDialog(std::vector<Glib::ustring>& audio_paths, const std::vector<Glib::ustring>& audio_ext)
:audio_extensions(audio_ext), audio_files(audio_paths)
{
	current_number = 0 ;
	last_selected = "" ;

	Gtk::VBox* box = get_vbox() ;
	if (box) {
		box->pack_start(title_box, false, false, 6) ;
			title_box.pack_start(title_label, false, false, 6) ;
			title_label.set_label(_("New Transcription")) ;
			title_label.set_name("bold_label") ;
		Gtk::HSeparator* sep = Gtk::manage(new Gtk::HSeparator()) ;
		box->pack_start(*sep, false, false, 6) ;
	}

	set_title(_("Choose audio file(s) for new transcription")) ;
	set_has_separator(true) ;
	Icons::set_window_icon(this, ICO_TRANSCRIBER, 12) ;

	std::vector<Glib::ustring>::iterator it ;

	for (it=audio_paths.begin(); it!=audio_paths.end(); it++) {
		add_audio(*it) ;
		last_selected = *it ;
	}

	//add a new blank entry
	add_audio_file() ;

	button_add.set_icon(ICO_FPROPERTY_AUDIO, _("Add audio file"), 17, _("Add an audio file for create new transcription")) ;
	button_add.signal_clicked().connect(sigc::mem_fun(this, &CreateTranscriptionDialog::add_audio_file)) ;
	get_action_area()->pack_start(button_add, false, false, 15) ;
	button_apply = add_button(_("Create transcription"), Gtk::RESPONSE_APPLY) ;
	button_cancel = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;

	if (box)
		box->show_all_children(true) ;
	show_all_children(true) ;
}

CreateTranscriptionDialog::~CreateTranscriptionDialog()
{
	std::vector<AudioLine*>::iterator it ;
	for (it=lines.begin(); it!=lines.end(); it++) {
		if (*it)
			delete(*it) ;
	}
}

void CreateTranscriptionDialog::add_audio(Glib::ustring path)
{
	current_number ++ ;
	AudioLine* line = new AudioLine(path, current_number, audio_extensions, last_selected) ;
	lines.push_back(line) ;
	Gtk::VBox* box = NULL ;
	box = get_vbox() ;
	if (box) {
		box->pack_start(*line, false, false, 5) ;
		box->show_all_children(true) ;
	}
	if (current_number==2)
		button_add.set_sensitive(false) ;
}

void CreateTranscriptionDialog::add_audio_file()
{
	add_audio("") ;
}

void CreateTranscriptionDialog::on_response(int id)
{
	audio_files.clear() ;
	if (id == Gtk::RESPONSE_APPLY) {
		int cpt = 0 ;
		std::vector<AudioLine*>::iterator it ;
		for (it=lines.begin(); it!=lines.end(); it++) {
			if (*it) {
				if ((*it)->entry.get_text()!="") {
					audio_files.push_back((*it)->entry.get_text()) ;
					cpt++;
				}
			}
		}
		Explorer_utils::print_trace("> multi channels mode: ", cpt, 1) ;
	}
}



} //namespace
