/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "SaveAudioDialog.h"
#include "Common/FileInfo.h"
#include "Common/util/Utils.h"
#include "Common/icons/Icons.h"
#include "Common/Dialogs.h"
#include "Common/Explorer_filter.h"

namespace tag {

SaveAudioDialog::SaveAudioDialog(Glib::ustring _audio_path, float _start, float _end)
{
	//TODO: default path
	set_size_request(500, -1) ;

	Icons::set_window_icon(this, ICO_TRANSCRIBER, 12) ;

	m_audio_path = _audio_path ;
	m_start = _start ;
	m_end = _end ;

	Glib::ustring offsers_s = number_to_string(m_start) + " - " + number_to_string(m_end) ;
	Glib::ustring name = g_path_get_basename(m_audio_path.c_str()) ;

	image = Gtk::manage(new Gtk::Image()) ;

	Gtk::VBox* box = get_vbox() ;
	save_button = Gtk::manage(new Gtk::Button(Gtk::Stock::SAVE)) ;
	cancel_button = Gtk::manage(new Gtk::Button(Gtk::Stock::CANCEL)) ;
	if (box) {
		box->pack_start(hbox_title, false, false, 6) ;
			try {
				Glib::RefPtr<Gtk::IconTheme>theme = Gtk::IconTheme::get_default() ;
				Glib::RefPtr<Gdk::Pixbuf>pixbufPlay = theme->load_icon("file-wave", 26, Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
				image->set(pixbufPlay);
				hbox_title.pack_start(*image, false, false, 15) ;
			}
			catch (Gtk::IconThemeError e) {
				Log::err() << "Signal menu:> while searching icon:> " <<  e.what()  << std::endl ;
			}
			hbox_title.pack_start(label_title, false, false, 15) ;
					label_title.set_label(_("Save Audio Signal")) ;
					label_title.set_name("bold_label") ;

		box->pack_start(hbox_file_infos, false, false, 6) ;
				hbox_file_infos.pack_start(label_file_infos, false, false, 15) ;
					label_file_infos.set_label(_("From file") + Glib::ustring(":")) ;
					label_file_infos.set_name("bold_label") ;
				hbox_file_infos.pack_start(scrolledW_file_infos, true, true, 3) ;
					scrolledW_file_infos.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_NEVER) ;
					scrolledW_file_infos.add(label_file_infos_value) ;
						label_file_infos_value.set_label(m_audio_path) ;
						label_file_infos_value.set_tooltip_text(m_audio_path) ;

		box->pack_start(hbox_offsets_infos, false, false, 6) ;
			hbox_offsets_infos.pack_start(label_offsets_infos, false, false, 15) ;
				label_offsets_infos.set_label(_("Selection") + Glib::ustring(":")) ;
				label_offsets_infos.set_name("bold_label") ;
			hbox_offsets_infos.pack_start(label_offsets_infos_value, false, false, 3) ;
				label_offsets_infos_value.set_label(offsers_s) ;

		box->pack_start(hbox_folder, false, false, 6) ;
			hbox_folder.pack_start(label_folder, false, false, 15) ;
				label_folder.set_name("bold_label") ;
				label_folder.set_label(_("Destination directory")) ;
			hbox_folder.pack_start(chooser, false, false, 3) ;

		box->pack_start(hbox_name, false, false, 6) ;
			hbox_name.pack_start(label_name, false, false, 15) ;
				label_name.set_name("bold_label") ;
				label_name.set_label(_("Choose a name")) ;
			hbox_name.pack_start(entry_name, false, false, 3) ;

		Gtk::HSeparator* sep = Gtk::manage(new Gtk::HSeparator()) ;
		box->pack_start(*sep, false, false, 6) ;

		box->pack_start(align_button, false, false, 6) ;
			align_button.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
			align_button.add(hbox_button) ;
				hbox_button.pack_start(*save_button, false, false, 15) ;
				hbox_button.pack_start(*cancel_button, false, false, 3) ;
	}

	chooser.set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER) ;
	save_button->signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &SaveAudioDialog::on_button_clicked), true)) ;
	cancel_button->signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &SaveAudioDialog::on_button_clicked), false)) ;
	set_title(_("Choose folder to save audio selection")) ;

	name = Explorer_filter::cut_extension(name) ;
	entry_name.set_text(name) ;

	if (box)
		box->show_all_children(true) ;
	show_all_children(true) ;
}

SaveAudioDialog::~SaveAudioDialog()
{

}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


void SaveAudioDialog::on_button_clicked(bool save)
{
	//cancel
	if (!save) {
		response(Gtk::RESPONSE_CANCEL) ;
	}
	else {
		Glib::ustring path = chooser.get_filename() ;
		FileInfo info(path) ;
		m_newName = entry_name.get_text() ;
		if (is_valid_filename(m_newName)<0)
			dlg::error(_("Invalid file name"), this) ;
		else if (!Glib::file_test(path, Glib::FILE_TEST_EXISTS) || !Glib::file_test(path, Glib::FILE_TEST_IS_DIR))
			dlg::error(_("Please choose a valid destination directory"), this) ;
		else if (!info.canWrite())
			dlg::error(_("Directory for read only"), this) ;
		else {
			FileInfo info(m_audio_path) ;
			Glib::ustring ext = Glib::ustring(info.tail()) ;
			m_newPath = Glib::build_filename(path, m_newName) ;
			m_newPath = m_newPath + ext ;
			if (Glib::file_test(m_newPath, Glib::FILE_TEST_EXISTS))
				dlg::error(_("A file with same name already exists, please change name or destination directory"), this) ;
			else
				response(Gtk::RESPONSE_APPLY) ;
		}
	}
}

} //namespace
