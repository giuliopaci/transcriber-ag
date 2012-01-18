/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  @file FileDialogs.cpp
 *  @brief basic application file selection dialogs
 *
 */

#include "FileDialogs.h"

#include "Common/Dialogs.h"
#include "Common/FileInfo.h"
#include "Common/globals.h"
#include "Common/Explorer_filter.h"

#include "MediaComponent/base/Guesser.h"

namespace dlg {


Glib::ustring selectTAGFile(const Glib::ustring& filename, bool load, int& copyMediaFile, Gtk::Window* top)
{
	Glib::ustring path = "";

	Gtk::FileChooserDialog* file_chooser;
	if ( load )
		file_chooser = new Gtk::FileChooserDialog(_("Open annotation file"), Gtk::FILE_CHOOSER_ACTION_OPEN);
	else
		file_chooser = new Gtk::FileChooserDialog(_("Save annotation file"), Gtk::FILE_CHOOSER_ACTION_SAVE);

	if ( top != NULL )
		file_chooser->set_transient_for(*top);

	if ( ! filename.empty() )
	{
		Glib::ustring folder = FileInfo(filename).dirname();
		Glib::ustring bname = FileInfo(filename).Basename();
		if ( folder == "" )
			folder = g_get_home_dir();
		if ( load )
			file_chooser->select_filename(FileInfo(folder).join(bname));
		else
		{
			file_chooser->set_current_folder(folder);
			file_chooser->set_current_name(bname);
		}
	}

	Gtk::CheckButton* check = NULL ;

	if (copyMediaFile == 0 || copyMediaFile == 1)
	{
		//> Preparing extra options
		Gtk::Label* blank = Gtk::manage(new Gtk::Label("")) ;
		Gtk::HSeparator* sep = Gtk::manage(new Gtk::HSeparator()) ;
		Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox()) ;
		Gtk::Alignment* align = Gtk::manage(new Gtk::Alignment()) ;
		Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox()) ;
		Gtk::Label* label = Gtk::manage(new Gtk::Label(_("Copy associated media file(s) in the same directory"))) ;
		check = Gtk::manage(new Gtk::CheckButton()) ;
		Gtk::HSeparator* sep2 = Gtk::manage(new Gtk::HSeparator()) ;

		label->set_name("bold_label") ;

		vbox->pack_start(*sep, false, false, 3) ;
		vbox->pack_start(*hbox, false, false, 3) ;
			hbox->pack_start(*check, false, false, 3) ;
			hbox->pack_start(*label, false, false, 3) ;
			hbox->pack_start(*blank, true, true, 3) ;
		vbox->pack_start(*sep2, false, false, 3) ;

		vbox->show_all_children(true) ;
		file_chooser->set_extra_widget(*vbox) ;

		if (copyMediaFile == 1)
			check->set_active(true) ;
	}

	file_chooser->add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
	file_chooser->add_button (Gtk::Stock::OK, Gtk::RESPONSE_OK) ;
	file_chooser->set_select_multiple(false) ;

	Gtk::FileFilter filter_TAG;
	filter_TAG.set_name("TAG files");
	filter_TAG.add_pattern("*.tag");
	filter_TAG.add_pattern("*.TAG");
	file_chooser->add_filter(filter_TAG);

	Gtk::FileFilter filter_any;
	filter_any.set_name("Any files");
	filter_any.add_pattern("*");
	file_chooser->add_filter(filter_any);

	if ( file_chooser->run() ==  Gtk::RESPONSE_OK )
		path = file_chooser->get_filename();
	else
		path = "" ;

	if (check && check->get_active())
		copyMediaFile = 1 ;
	else if (check && !check->get_active())
		copyMediaFile = 0 ;
	else
		copyMediaFile = -1 ;

	file_chooser->hide();
	delete file_chooser;
	return path;
}

Glib::ustring selectExportFile(const Glib::ustring& filename, Gtk::Window* top, Glib::ustring& format)
{
	Glib::ustring path = "";

	tag::Explorer_filter* filter = tag::Explorer_filter::getInstance() ;

	if (!filter)
		return "" ;

	Gtk::FileChooserDialog* file_chooser;
	file_chooser = new Gtk::FileChooserDialog(_("Export annotation file"), Gtk::FILE_CHOOSER_ACTION_SAVE);

	if ( top != NULL )
		file_chooser->set_transient_for(*top);

	if ( ! filename.empty() )
	{
		Glib::ustring folder = FileInfo(filename).dirname();
		Glib::ustring bname = FileInfo(filename).Basename();
		if ( folder == "" )
		{
			folder = g_get_home_dir();
			file_chooser->set_current_folder(folder);
			file_chooser->set_current_name(bname);
		}
	}

	//> Preparing extra options
	Gtk::Label* blank = Gtk::manage(new Gtk::Label("")) ;
	Gtk::HSeparator* sep = Gtk::manage(new Gtk::HSeparator()) ;
	Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox()) ;
	Gtk::Alignment* align = Gtk::manage(new Gtk::Alignment()) ;
	Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox()) ;
	Gtk::Label* label = Gtk::manage(new Gtk::Label(_("Select output format: "))) ;
	Gtk::ComboBoxText* combo = Gtk::manage(new Gtk::ComboBoxText()) ;
	Gtk::HSeparator* sep2 = Gtk::manage(new Gtk::HSeparator()) ;

	label->set_name("bold_label") ;

	vbox->pack_start(*sep, false, false, 3) ;
	vbox->pack_start(*hbox, false, false, 3) ;
		hbox->pack_start(*blank, true, true, 3) ;
		hbox->pack_start(*label, false, false, 3) ;
		hbox->pack_start(*combo, false, false, 3) ;
	vbox->pack_start(*sep2, false, false, 3) ;

	//> ANNOTATION TYPE FOR EXPORT
	std::map<Glib::ustring, Glib::ustring> exports = filter->get_export_annotations() ;
	std::map<Glib::ustring, Glib::ustring>::iterator it ;
	for(it=exports.begin(); it!=exports.end(); it++)
	{
		if (it->first != "TransAG")
		{
			combo->append_text(it->first) ;
			combo->set_active_text(it->first) ;
		}
	}

	vbox->show_all_children(true) ;
	file_chooser->set_extra_widget(*vbox) ;


	//> Preparing filter
	Gtk::FileFilter filter_ANNOT;
	std::map<Glib::ustring, Glib::ustring> imports = filter->get_import_annotations() ;
	for(it=exports.begin(); it!=exports.end(); it++) {
		filter_ANNOT.add_pattern("*" + (it->second).uppercase()) ;
		filter_ANNOT.add_pattern("*" + (it->second).lowercase());
	}

	filter_ANNOT.set_name(_("Transcription files")) ;
    file_chooser->add_filter(filter_ANNOT);

	file_chooser->add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
	file_chooser->add_button (Gtk::Stock::OK, Gtk::RESPONSE_OK) ;
	file_chooser->set_select_multiple (false) ;

	int rep = file_chooser->run() ;

	if ( rep ==  Gtk::RESPONSE_OK )
	{
		path = file_chooser->get_filename() ;

		//> DEALS WITH FORMAT
		format = combo->get_active_text() ;
		// force adequat extension
		string path_ext = filter->get_extension(path) ;
		string format_ext = exports[format] ;
		if (format_ext!=path_ext)
			path = path + format_ext ;
	}
	else
		path = "" ;

	file_chooser->hide();

	delete file_chooser;

	return path;
}

Glib::ustring selectAudioFile(bool& close, const Glib::ustring& filename, const Glib::ustring& title, Gtk::Window* top, bool force_mono, const std::string& def_rep)
{
	Glib::ustring path = "";

	Glib::ustring atitle = title;
	if ( atitle.empty() )
		atitle=_("Audio file to open");

	Gtk::FileChooserDialog* file_chooser = new Gtk::FileChooserDialog(atitle, Gtk::FILE_CHOOSER_ACTION_OPEN);

	if ( !def_rep.empty() && Glib::file_test(def_rep, Glib::FILE_TEST_EXISTS) )
		file_chooser->set_current_folder(def_rep);

	if ( ! filename.empty() )
	{
		Glib::ustring folder = FileInfo(filename).dirname();
		Glib::ustring bname = FileInfo(filename).Basename();
		if ( folder == "" )
			folder = g_get_home_dir();
		file_chooser->select_filename(FileInfo(folder).join(bname));
	}

	if ( top != NULL )
		file_chooser->set_transient_for(*top);

	//> AUDIO FILTER
	Gtk::FileFilter filter_audio;

	tag::Explorer_filter* filter = tag::Explorer_filter::getInstance() ;
	if (filter)
	{
		std::vector<Glib::ustring> tmp = filter->get_audio_extensions() ;
		std::vector<Glib::ustring>::iterator it ;

		for(it=tmp.begin(); it!=tmp.end(); it++){
			filter_audio.add_pattern("*" + (*it).uppercase()) ;
			filter_audio.add_pattern("*" + (*it).lowercase());
		}
	}
	else
		filter_audio.add_mime_type("audio/x-wav");

	filter_audio.set_name("Audio files");
	file_chooser->add_filter(filter_audio);

//	//> ALL FILTER
//	Gtk::FileFilter filter_any;
//	filter_any.set_name("Any files");
//	filter_any.add_pattern("*");
//	file_chooser->add_filter(filter_any);

	file_chooser->add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
	file_chooser->add_button (_("Without signal"), Gtk::RESPONSE_NO) ;
	file_chooser->add_button (Gtk::Stock::OK, Gtk::RESPONSE_OK) ;
	file_chooser->set_select_multiple (false) ;

	bool ok = false ;

	while (!ok)
	{
		int rep = file_chooser->run() ;
		if (rep ==  Gtk::RESPONSE_CANCEL || rep ==  Gtk::RESPONSE_CLOSE)
		{
			ok = true ;
			close = true ;
		}
		else if (rep == Gtk::RESPONSE_OK )
		{
			path = file_chooser->get_filename();
			IODevice* device = Guesser::open(path.c_str()) ;
			if (device) {
				if (device->m_info()->audio_channels>1 && force_mono)
					warning(_("This audio file has too much channels for current transcription"), NULL) ;
				else
					ok = true ;
			}
			else
				ok = true ;
		}
		else
			ok = true ;
	}

	file_chooser->hide();
	delete file_chooser;
	return path;
}

Glib::ustring selectVideoFile(bool& close, const Glib::ustring& filename, const Glib::ustring& title, Gtk::Window* top)
{
	Glib::ustring path = "";

	Glib::ustring atitle = title;
	if ( atitle.empty() ) atitle=_("Audio file to open");

	Gtk::FileChooserDialog* file_chooser = new Gtk::FileChooserDialog(atitle, Gtk::FILE_CHOOSER_ACTION_OPEN);

	if ( ! filename.empty() ) {
		Glib::ustring folder = FileInfo(filename).dirname();
		Glib::ustring bname = FileInfo(filename).Basename();
		if ( folder == "" ) folder = g_get_home_dir();
		file_chooser->select_filename(FileInfo(folder).join(bname));
	}

	if ( top != NULL ) file_chooser->set_transient_for(*top);

	//> AUDIO FILTER
	Gtk::FileFilter filter_video;

	tag::Explorer_filter* filter = tag::Explorer_filter::getInstance() ;
	if (filter) {
		std::vector<Glib::ustring> tmp = filter->get_video_extensions() ;
		std::vector<Glib::ustring>::iterator it ;

		for(it=tmp.begin(); it!=tmp.end(); it++){
			filter_video.add_pattern("*" + (*it).uppercase()) ;
			filter_video.add_pattern("*" + (*it).lowercase());
		}
	}
	else
		filter_video.add_mime_type("audio/x-wav");

	filter_video.set_name("Video files");
	file_chooser->add_filter(filter_video);

	//> ALL FILTER
	Gtk::FileFilter filter_any;
	filter_any.set_name("Any files");
	filter_any.add_pattern("*");
	file_chooser->add_filter(filter_any);


	file_chooser->add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
//	file_chooser->add_button (_("Without signal"), Gtk::RESPONSE_NO) ;
	file_chooser->add_button (Gtk::Stock::OK, Gtk::RESPONSE_OK) ;
	file_chooser->set_select_multiple (false) ;

	int rep = file_chooser->run() ;

	if ( rep ==  Gtk::RESPONSE_OK )
		path = file_chooser->get_filename();
	else if ( rep ==  Gtk::RESPONSE_CANCEL || rep ==  Gtk::RESPONSE_CLOSE )
		close = true ;

	file_chooser->hide();
	delete file_chooser;
	return path;
}

}




