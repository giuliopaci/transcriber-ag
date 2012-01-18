/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "FilePropertyDialog.h"
#include "Explorer_fileHelper.h"
#include "Explorer_utils.h"
#include "Common/util/Utils.h"
#include "Common/globals.h"
#include "Common/icons/Icons.h"

namespace tag {

FilePropertyDialog::FilePropertyDialog(Glib::ustring _path, Glib::ustring _info)
{
	general_table = NULL ;
	audio_table = NULL ;

	filter = Explorer_filter::getInstance() ;

	Glib::ustring title = _("Properties: ") + _path ;
	set_title(title) ;
	path = _path ;
	info = _info ;

	mini_parser(',', info, &info_v) ;
	Icons::set_window_icon(this, ICO_TRANSCRIBER,15) ;

	prepare_GUI() ;
}

FilePropertyDialog::~FilePropertyDialog()
{
	if (general_table)
		delete(general_table) ;
	if (audio_table)
		delete(audio_table) ;
}

void FilePropertyDialog::prepare_GUI()
{
	add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE) ;
	Gtk::VBox* box = get_vbox() ;

	box->pack_start(notebook, true, true) ;

	//> prepare GENERAL FRAME
	prepare_general() ;

	if (filter->is_audio_file(path))
		prepare_audio() ;

	box->show_all_children(true) ;
}


//**************************************************************************************
//****************************************************************************** GENERAL
//**************************************************************************************


void FilePropertyDialog::prepare_general()
{
	Glib::ustring ico = filter->switch_ico(path,-1) ;
	try {
		Glib::RefPtr<Gtk::IconTheme>theme = Gtk::IconTheme::get_default() ;
		Glib::RefPtr<Gdk::Pixbuf>pixbufPlay = theme->load_icon(ico, 50,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
		general_name_image.set(pixbufPlay);
	}
	catch (Gtk::IconThemeError e) {
		Explorer_utils::print_trace("IcoPackButton::set_icon:>", e.what(), 0) ;
	}

	//> ADD IN GUI
	general_table = new Gtk::Table(5, 2, false) ;
	general_table->set_col_spacings(15) ;
	general_table->set_row_spacings(7) ;

	notebook.append_page(general_vbox, _("General")) ;

	general_vbox.pack_start(general_blank_start, false, false) ;
	general_vbox.pack_start(general_name_al, false, false) ;
	general_name_al.add(general_name_hbox) ;
	general_name_al.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
		general_name_hbox.pack_start(general_name_image, false, false) ;
		general_name_hbox.pack_start(general_name_blank, false, false) ;
		general_name_hbox.pack_start(general_name, false, false) ;
		general_vbox.pack_start(general_blank_middle, false, false) ;
	general_vbox.pack_start(general_table_hbox, false, false) ;
	general_vbox.pack_start(general_blank_end, false, false) ;

	general_table_hbox.pack_start(general_table_blank_left, false, false) ;
	general_table_hbox.pack_start(*general_table, false, false) ;
	general_table_hbox.pack_start(general_table_blank_right, false, false) ;

	//> PREPARE DATA
	Glib::ustring type, size, modtime, actime ;
	type = info_v[EXPLORER_FI_TYPE] ;
	size = info_v[EXPLORER_FI_SIZE] ;
	modtime = info_v[EXPLORER_FI_MODTIME] ;
	actime = info_v[EXPLORER_FI_ACTIME] ;

	//prepare size
	Glib::ustring display_size = Explorer_utils::format_size(size) ;

	//prepare path
	Glib::ustring path_tmp = path ;
	Glib::ustring display_path = Glib::path_get_dirname(path_tmp) ;

	//prepare name
	Glib::ustring display_name = Glib::path_get_basename(path) ;

	//prepare type
	Glib::ustring display_type = type ;

	//prepare times
	Glib::ustring display_modtime = Explorer_utils::format_date(modtime) ;
	Glib::ustring display_actime = Explorer_utils::format_date(actime) ;

	general_name.set_label(display_name) ;
	general_name.set_name("bold_label") ;

	general_type_v.set_label(display_type) ;
	general_size_v.set_label(display_size) ;
	general_directory_v.set_label(display_path) ;
	general_modified_v.set_label(display_modtime) ;
	general_accessed_v.set_label(display_actime) ;

	set_general_labels() ;

	//> ADD IN TABLE
	general_table->attach(general_type_al , 0, 1, 0, 1, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	general_table->attach(general_size_al , 0, 1, 1, 2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	general_table->attach(general_directory_al , 0, 1, 2, 3, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	general_table->attach(general_modified_al , 0, 1, 3, 4, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	general_table->attach(general_accessed_al , 0, 1, 4, 5, Gtk::FILL, Gtk::EXPAND, 0, 0) ;

	general_table->attach(general_type_av , 1, 2, 0, 1, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	general_table->attach(general_size_av , 1, 2, 1, 2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	general_table->attach(general_directory_av , 1, 2, 2, 3, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	general_table->attach(general_modified_av , 1, 2, 3, 4, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	general_table->attach(general_accessed_av , 1, 2, 4, 5, Gtk::FILL, Gtk::EXPAND, 0, 0) ;

	general_table->show_all_children(true) ;
}

void FilePropertyDialog::set_general_labels()
{
	general_name_blank.set_label("  ") ;

	general_blank_start.set_label(" ") ;
	general_blank_end.set_label(" ") ;
	general_blank_middle.set_label(" ") ;

	general_table_blank_left.set_label("    ") ;
	general_table_blank_right.set_label("    ") ;

	general_type_l.set_label(_("Type: ")) ;
	general_size_l.set_label(_("Size: ")) ;
	general_directory_l.set_label(_("Location: ")) ;
	general_blank.set_label("") ;
	general_modified_l.set_label(_("Modified: ")) ;
	general_accessed_l.set_label(_("Last access: ")) ;

	general_type_l.set_name("bold_label") ;
	general_size_l.set_name("bold_label") ;
	general_directory_l.set_name("bold_label") ;
	general_modified_l.set_name("bold_label") ;
	general_accessed_l.set_name("bold_label") ;

	general_size_al.add(general_size_l) ;
	general_size_av.add(general_size_v) ;
	general_size_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	general_size_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;

	general_type_al.add(general_type_l) ;
	general_type_av.add(general_type_v) ;
	general_type_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	general_type_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;

	general_directory_al.add(general_directory_l) ;
	general_directory_av.add(general_directory_v) ;
	general_directory_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	general_directory_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;

	general_modified_al.add(general_modified_l) ;
	general_modified_av.add(general_modified_v) ;
	general_modified_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	general_modified_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;

	general_accessed_al.add(general_accessed_l) ;
	general_accessed_av.add(general_accessed_v) ;
	general_accessed_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	general_accessed_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
}


//**************************************************************************************
//******************************************************************************** AUDIO
//**************************************************************************************

void FilePropertyDialog::prepare_audio()
{
	try {
		Glib::RefPtr<Gtk::IconTheme>theme = Gtk::IconTheme::get_default() ;
		Glib::RefPtr<Gdk::Pixbuf>pixbufPlay = theme->load_icon(ICO_FPROPERTY_AUDIO, 50,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
		audio_name_image.set(pixbufPlay);
	}
	catch (Gtk::IconThemeError e) {
		Explorer_utils::print_trace("IcoPackButton::set_icon:>", e.what(), 0) ;
	}

	audio_table = new Gtk::Table(7, 2, false) ;
	audio_table->set_col_spacings(15) ;
	audio_table->set_row_spacings(7) ;

	notebook.append_page(audio_vbox, _("audio")) ;

	audio_vbox.pack_start(audio_blank_start, false, false) ;
	audio_vbox.pack_start(audio_name_al, false, false) ;
	audio_name_al.add(audio_name_hbox) ;
	audio_name_al.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
		audio_name_hbox.pack_start(audio_name_image, false, false) ;
		audio_name_hbox.pack_start(audio_name_blank, false, false) ;
		audio_name_hbox.pack_start(audio_name, false, false) ;
	audio_vbox.pack_start(audio_blank_middle, false, false) ;
	audio_vbox.pack_start(audio_table_hbox_align, false, false) ;
		audio_table_hbox_align.add(audio_table_hbox);
		audio_table_hbox_align.set(Gtk::ALIGN_CENTER,Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	audio_vbox.pack_start(audio_blank_end, false, false) ;

	audio_table_hbox.pack_start(audio_table_blank_left, false, false) ;
	audio_table_hbox.pack_start(*audio_table, false, false) ;
	audio_table_hbox.pack_start(audio_table_blank_right, false, false) ;

	//> PREPARE DATA
	Glib::ustring type, channel, resolution, samplingRate, totalSample, encoding, duration  ;
	type = info_v[EXPLORER_FI_TYPE] ;
	channel = info_v[EXPLORER_FI_CHANNEL] ;
	resolution = info_v[EXPLORER_FI_SAMPLING_RES] ;
	samplingRate = info_v[EXPLORER_FI_SAMPLING_RATE] ;
	totalSample = info_v[EXPLORER_FI_TOTAL_SAMPLE] ;
	encoding = info_v[EXPLORER_FI_ENCODING] ;
	duration = info_v[EXPLORER_FI_DURATION] ;

	//> duration
	Glib::ustring display_duration ;
	if (!duration.empty()) {
		double duration_seconds = string_to_number<double>(duration) ;
		std::vector<int> time ;
		Explorer_utils::get_time(duration_seconds, &time) ;

		display_duration = number_to_string(time[0]) + "h "
				+ number_to_string(time[1]) + "min "
				+ number_to_string(time[2]) + "sec" ;
	}

	Glib::ustring space = " " ;

	//prepare name
	Glib::ustring display_name = Glib::path_get_basename(path) ;

	//prepare channel
	Glib::ustring display_channel ;
	if (channel == number_to_string(1))
		display_channel = _("Mono") ;
	else if (channel == number_to_string(2))
		display_channel = _("Stereo") ;

	//prepare type
	Glib::ustring display_type = "---" ;
	if (!type.empty())
		display_type = type ;

	//prepare sampling rate
	Glib::ustring display_srate = "---" ;
	if (!samplingRate.empty())
		display_srate = samplingRate + space + "Hz" ;

	//prepare total sample
	Glib::ustring display_tsample = "---" ;
	if (!totalSample.empty())  ;
		display_tsample = totalSample ;

	//prepare sampling resolution
	Glib::ustring display_sres = "---" ;
	if (!resolution.empty())
		display_sres = resolution + space +  _("bits") ;

	//encoding
	if (encoding.empty())
		encoding = "---" ;

	audio_name.set_label(display_name) ;
	audio_name.set_name("bold_label") ;

	audio_type_v.set_label(display_type) ;
	audio_duration_v.set_label(display_duration) ;
	audio_channel_v.set_label(display_channel) ;
	audio_samplingRate_v.set_label(display_srate) ;
	audio_totalSample_v.set_label(display_tsample) ;
	audio_samplingResolution_v.set_label(display_sres) ;
	audio_encoding_v.set_label(encoding) ;

	set_audio_labels() ;

	//> ADD IN TABLE
	audio_table->attach(audio_type_al , 0, 1, 0, 1, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_duration_al , 0, 1, 1, 2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_channel_al , 0, 1, 2, 3, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_samplingRate_al, 0, 1, 3, 4, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_totalSample_al , 0, 1, 4, 5, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_samplingResolution_al , 0, 1, 5, 6, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_encoding_al , 0, 1, 6, 7, Gtk::FILL, Gtk::EXPAND, 0, 0) ;

	audio_table->attach(audio_type_av , 1, 2, 0, 1, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_duration_av , 1, 2, 1, 2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_channel_av , 1, 2, 2, 3, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_samplingRate_av , 1, 2, 3, 4, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_totalSample_av , 1, 2, 4, 5, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_samplingResolution_av , 1, 2, 5, 6, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	audio_table->attach(audio_encoding_av , 1, 2, 6, 7, Gtk::FILL, Gtk::EXPAND, 0, 0) ;

	audio_table->show_all_children(true) ;
}

void FilePropertyDialog::set_audio_labels()
{
	audio_name_blank.set_label("  ") ;

	audio_blank_start.set_label(" ") ;
	audio_blank_end.set_label(" ") ;
	audio_blank_middle.set_label(" ") ;

	audio_table_blank_left.set_label("    ") ;
	audio_table_blank_right.set_label("    ") ;

	audio_type_l.set_label(_("Type: ")) ;
	audio_type_l.set_name("bold_label") ;
	audio_type_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_type_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_type_av.add(audio_type_v) ;
	audio_type_al.add(audio_type_l) ;

	audio_duration_l.set_label(_("Duration: ")) ;
	audio_duration_l.set_name("bold_label") ;
	audio_duration_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_duration_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_duration_av.add(audio_duration_v) ;
	audio_duration_al.add(audio_duration_l) ;

	audio_channel_l.set_label(_("Channel: ")) ;
	audio_channel_l.set_name("bold_label") ;
	audio_channel_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_channel_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_channel_av.add(audio_channel_v) ;
	audio_channel_al.add(audio_channel_l) ;

	audio_samplingRate_l.set_label(_("Sampling rate: ")) ;
	audio_samplingRate_l.set_name("bold_label") ;
	audio_samplingRate_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_samplingRate_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_samplingRate_av.add(audio_samplingRate_v) ;
	audio_samplingRate_al.add(audio_samplingRate_l) ;

	audio_totalSample_l.set_label(_("Sample count: ")) ;
	audio_totalSample_l.set_name("bold_label") ;
	audio_totalSample_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_totalSample_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_totalSample_av.add(audio_totalSample_v) ;
	audio_totalSample_al.add(audio_totalSample_l) ;

	audio_samplingResolution_l.set_label(_("Sampling resolution: ")) ;
	audio_samplingResolution_l.set_name("bold_label") ;
	audio_samplingResolution_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_samplingResolution_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_samplingResolution_av.add(audio_samplingResolution_v) ;
	audio_samplingResolution_al.add(audio_samplingResolution_l) ;

	audio_encoding_l.set_label(_("Encoding: ")) ;
	audio_encoding_l.set_name("bold_label") ;
	audio_encoding_av.set(Gtk::ALIGN_LEFT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_encoding_al.set(Gtk::ALIGN_RIGHT,Gtk::ALIGN_LEFT, 0.0, 0.0) ;
	audio_encoding_av.add(audio_encoding_v) ;
	audio_encoding_al.add(audio_encoding_l) ;
}

} //namespace
