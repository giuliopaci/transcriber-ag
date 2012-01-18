/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef FILEPROPERTYDIALOG_H_
#define FILEPROPERTYDIALOG_H_

#include <gtkmm.h>
#include "Common/Explorer_filter.h"

namespace tag {
/**
 * @class 		FilePropertyDialog
 * @ingroup		GUI
 * Display file information such as mimetype, size, etc...
 */
class FilePropertyDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param path		file path
		 * @param info		formatted string by <em>TAG_file_info</em> method
		 * 					of <em>Explorer_fileHelper</em> class
		 * @return
		 */
		FilePropertyDialog(Glib::ustring path, Glib::ustring info);
		virtual ~FilePropertyDialog();

	private:
		Explorer_filter* filter ;

		std::vector<Glib::ustring> info_v ;
		Glib::ustring path ;
		Glib::ustring info ;

		Gtk::Notebook notebook ;

		//************************************ GENERAL FRAME
		Gtk::Frame general_frame ;
			Gtk::VBox general_vbox ;
				Gtk::Label general_blank_start ;
				Gtk::Alignment general_name_al ;
					Gtk::HBox general_name_hbox ;
						Gtk::Image general_name_image ;
						Gtk::Label general_name_blank ;
						Gtk::Alignment general_name_label_al ;
							Gtk::Label general_name ;
				Gtk::Label general_blank_middle ;
				Gtk::HBox general_table_hbox ;
					Gtk::Label general_table_blank_left ;
						Gtk::Table* general_table ;
					Gtk::Label general_table_blank_right ;
				Gtk::Label general_blank_end ;

		Gtk::Label general_type_l ;
		Gtk::Label general_size_l ;
		Gtk::Label general_directory_l ;
		Gtk::Label general_modified_l ;
		Gtk::Label general_accessed_l ;
		Gtk::Label general_type_v ;
		Gtk::Label general_size_v ;
		Gtk::Label general_directory_v ;
		Gtk::Label general_modified_v ;
		Gtk::Label general_accessed_v ;
		Gtk::Label general_blank ;
		Gtk::Alignment general_type_av ;
		Gtk::Alignment general_size_av ;
		Gtk::Alignment general_directory_av ;
		Gtk::Alignment general_modified_av ;
		Gtk::Alignment general_accessed_av ;
		Gtk::Alignment general_type_al ;
		Gtk::Alignment general_size_al ;
		Gtk::Alignment general_directory_al ;
		Gtk::Alignment general_modified_al ;
		Gtk::Alignment general_accessed_al ;

		void prepare_GUI() ;
		void prepare_general() ;
		void set_general_labels() ;


		//************************************ Audio frame

		Gtk::Frame audio_frame ;
			Gtk::VBox audio_vbox ;
				Gtk::Label audio_blank_start ;
				Gtk::Alignment audio_name_al ;
					Gtk::HBox audio_name_hbox ;
						Gtk::Image audio_name_image ;
						Gtk::Label audio_name_blank ;
							Gtk::Label audio_name ;
				Gtk::Label audio_blank_middle ;
				Gtk::Alignment audio_table_hbox_align ;
					Gtk::HBox audio_table_hbox ;
						Gtk::Label audio_table_blank_left ;
							Gtk::Table* audio_table ;
						Gtk::Label audio_table_blank_right ;
				Gtk::Label audio_blank_end ;

		Gtk::Label audio_type_l ;
		Gtk::Label audio_type_v ;
		Gtk::Alignment audio_type_av ;
		Gtk::Alignment audio_type_al ;

		Gtk::Label audio_duration_l ;
		Gtk::Label audio_duration_v ;
		Gtk::Alignment audio_duration_av ;
		Gtk::Alignment audio_duration_al ;

		Gtk::Label audio_channel_l ;
		Gtk::Label audio_channel_v ;
		Gtk::Alignment audio_channel_av ;
		Gtk::Alignment audio_channel_al ;

		Gtk::Label audio_samplingRate_l ;
		Gtk::Label audio_samplingRate_v ;
		Gtk::Alignment audio_samplingRate_av ;
		Gtk::Alignment audio_samplingRate_al ;

		Gtk::Label audio_totalSample_l ;
		Gtk::Label audio_totalSample_v ;
		Gtk::Alignment audio_totalSample_av ;
		Gtk::Alignment audio_totalSample_al ;

		Gtk::Label audio_samplingResolution_l ;
		Gtk::Label audio_samplingResolution_v ;
		Gtk::Alignment audio_samplingResolution_av ;
		Gtk::Alignment audio_samplingResolution_al ;

		Gtk::Label audio_encoding_l ;
		Gtk::Label audio_encoding_v ;
		Gtk::Alignment audio_encoding_av ;
		Gtk::Alignment audio_encoding_al ;

		void prepare_audio() ;
		void set_audio_labels() ;

};

} //namespace

#endif /*FILEPROPERTYDIALOG_H_*/
