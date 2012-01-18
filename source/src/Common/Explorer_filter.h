/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef EXPLORER_FILTER_H_
#define EXPLORER_FILTER_H_

#include <gtkmm.h>
#include "globals.h"
#include "Common/util/Log.h"

//> ComboBox display
#define DISPLAY_AUDIONOTE 		_("Audio / Annotation files")
#define DISPLAY_TRANSCRIBER		_("TranscriberAG Files")
#define DISPLAY_AUDIO 			_("Audio files")
#define DISPLAY_ALL 			_("All files")
#define DISPLAY_VIDEO 			_("Video files")

/**
 * @var EXT_NONE
 * Defined value returned when no extension has been found in a file path
 */
#define EXT_NONE ""

namespace tag {

/**
 * @class 		Explorer_filter
 * @ingroup		Common
 *
 *	This singleton controls the file types handled by TranscriberAG.\n
 *
 *	It provides a ComboBoxText filter with pre-defined display mode for file types
 *	visibility (used for instance in the TreeManager for explorer filtering)
 *
 * @note		This is a (very) basic guesser using file extensions.
 * @note		For <em>aglib</em> format plugin, the Explorer_filter uses the Formats class.
 */

class Explorer_filter
{
	public:

		/**
		 * Get the Exporer_filter instance.\n
		 * When called for the first time, instantiates it.
		 *
		 * @param with_gui_composant	True for instantiating the filter ComboBox, False otherwise
		 * @return						Pointer on the Explorer_filter instance
		 */
		static Explorer_filter* getInstance(bool with_gui_composant=true) ;

		/**
		 * Deletes the Explorer_filter instance
		 */
		void kill() ;

		/**
		 * Accessor to the ComboBoxText filter
		 * @return		Pointer on the Gtk::ComboBoxText
		 */
		Gtk::ComboBoxText* get_combo() ;

		/**
		 * Accessor to the text displayed in the filter combo for <em>all files visibility</em>
		 * @return		<em>All files visibility</em> label for the combo
		 */
		Glib::ustring get_DISPLAY_ALL() {return DISPLAY_ALL ;}

		/**
		 * Accessor to the text displayed in the filter combo for <em>video files only visibility</em>
		 * @return		<em>Video files only visibility</em> label for the combo
		 */
		Glib::ustring get_DISPLAY_VIDEO() {return DISPLAY_VIDEO ;}

		/**
		 * Accessor to the text displayed in the filter combo for <em>audio files only visibility</em>
		 * @return		<em>Audio files only visibility</em> label for the combo
		 */
		Glib::ustring get_DISPLAY_AUDIO() {return DISPLAY_AUDIO ;}

		/**
		 * Accessor to the text displayed in the filter combo for <em>TranscriberAG files only visibility</em>
		 * @return		<em>TranscriberAG files visibility</em> label for the combo
		 */
		Glib::ustring get_DISPLAY_TAG(){return DISPLAY_TRANSCRIBER ;}

		/**
		 * Accessor to the text displayed in the filter combo for <em>audio and transcription files visibility</em>
		 * @return		<em>audio and transcription files visibility</em> label for the combo
		 */
		Glib::ustring get_DISPLAY_AUDIONOTE(){return DISPLAY_AUDIONOTE ;}

		/**
		 * Arbitrary defined value for none extension value
		 * @return		Value for "no extension" extention
		 */
		Glib::ustring get_EXT_NONE() { if (none) return none->extension ; else return "" ;}

		/**
		 *  Set the active text of the filter combobox
		 * @param s		Visibility label of the combobox
		 * @see			get_DISPLAY_ALL()
		 * @see			get_DISPLAY_AUDIO()
		 * @see			get_DISPLAY_VIDEO()
		 * @see			get_DISPLAY_AUDIONOTE()
		 * @see			get_DISPLAY_TAG()
		 */
		void set_filter_chosen(Glib::ustring s) ;

		/**
		 * Check if a file is recognized as an audio file by TranscriberAG
		 * @param path		Path of the candidate file
		 * @return			True if the file is recognized as such, False otherwise
		 */
		bool is_audio_file(Glib::ustring path) ;

		/**
		 * Check if a file is recognized as an annotation file by TranscriberAG
		 * @param path		Path of the candidate file
		 * @return			True if the file is recognized as such, False otherwise
		 * @note 			The annotation files are the files in TranscriberAG format or
		 * 					the files that can be converted into TranscriberAG formart.
		 * @see 			Formats class for supported annotation formats
		 *
		 */
		bool is_annotation_file(Glib::ustring path) ;

		/**
		 * Check if the file can be converted into TranscriberAG format
		 * @param path		Path of the candidate file
		 * @return			True if the file can be converted, False otherwise
		 * @note 			The convertible formats are defined in a configuration file.\n
		 * 					The conversion is proceeded by <em>aglib</em> plugins.
		 * @see 			Formats class for supported annotation formats
		 */
		bool is_import_annotation_file(Glib::ustring path) ;

		/**
		 * Check if a file is recognized as a video file by TranscriberAG
		 * @param path		Path of the candidate file
		 * @return			True if the file is recognized as such, False otherwise
		 */
		bool is_video_file(Glib::ustring path) ;

		/**
		 * Check if a file is recognized as a picture file by TranscriberAG
		 * @param path		Path of the candidate file
		 * @return			True if the file is recognized as such, False otherwise
		 */
		bool is_image_file(Glib::ustring path) ;

		/**
		 * Check if a file is a TranscriberAG file (TAG format)
		 * @param path		Path of the candidate file
		 * @return			True if the file is recognized as such, False otherwise
		 */
		bool is_tag_file(Glib::ustring path) ;

		/**
		 * Check if the given extension matches the filter chosen with the filter combo
		 * @param 	ext		Candidate extension
		 * @return			True if the extension is allowed by the current filter,
		 * 					False otherwise
		 */
		bool is_file_in_filter(Glib::ustring ext) ;

		/**
		 * Get a label corresponding to given extension.
		 * annotation supported format.
		 * @param extension		File extension (format .ext)
		 * @return				A label to be displayed in GUI.
		 */
		Glib::ustring get_display_from_ext(Glib::ustring extension) ;

		/**
		 * Get the specific icon name corresponding to given extension
		 * @param extension		File extension (format .ext)
		 * @return				Specific icon name
		 * @see					Icons class for pre-defined icon names.
		 */
		Glib::ustring get_icon_from_ext(Glib::ustring extension) ;

		/**
		 * Accessor to all supported audio file extensions.
		 * @return		A vector containing all extensions of supported audio files.
		 */
		std::vector<Glib::ustring> get_audio_extensions() ;

		/**
		 * Accessor to all supported transcription file extensions.
		 * @return		A vector containing all extensions of supported transcription files.
		 */
		std::vector<Glib::ustring> get_annotation_extensions() ;

		/**
		 * Accessor to all supported video files extensions.
		 * @return		A vector containing all extensions of supported video files.
		 */
		std::vector<Glib::ustring> get_video_extensions() ;

		/**
		 * Get all all supported transcription file AG formats and extensions
		 * available for import.
		 * @return  	A file information map [AG format - extension] for supported import conversion
		 */
		std::map<Glib::ustring, Glib::ustring> get_import_annotations() ;

		/**
		 * Get all all supported transcription file AG formats and extensions
		 * available for export.
		 * @return  	A file information map [AG format - extension] for supported export conversion
		 */
		std::map<Glib::ustring, Glib::ustring> get_export_annotations() ;

		/**
		 * Quick accessor for the file icon corresponding to Explorer_tree file type code
		 * @param path			File path
		 * @param root_type		Explorer_tree file type code
		 * @return				A defined icon name
		 * @see					Icons class for defined icon names
		 * @see					Explorer_tree class for defined file type code
		 */
		Glib::ustring switch_ico(Glib::ustring path, int root_type) ;

		/**
		 * Quick accessor for the file icon corresponding to the given extension.
		 * @param ext		File extension
		 * @return			A defined icon name
		 * @see				Icons class for defined icon names
		 */
		Glib::ustring icons_by_extension(Glib::ustring ext) ;

		/**
		 * Basic method for getting file extension
		 * @param name		File path or file name
		 * @return			File extension if '.' character found, defined NONE_EXT string else
		 */
		static Glib::ustring get_extension(Glib::ustring name) ;

		/**
		 * Basic method for getting a name without its extension
		 * @param name		File path or file name
		 * @return			File name or file path without extension
		 */
		static Glib::ustring cut_extension(Glib::ustring name) ;

		/**
		 * Checks if a specific media file exists in the given directory
		 * @param dir		Directory where we will search
		 * @param name		Name of the searched media file
		 * @param myType	Mode for research: "audio" for audio file or "video" for video file
		 * @return			The file path found, or empty value if none matched.
		 * @note 			The research is proceeded with video or audio file recognized by the application.
		 */
		static std::string lookForMediaFile(const std::string& dir, const std::string& name, const std::string& myType ) ;

		/**
		 * Checks if a specific file exists in the given directory
		 * @param dir			Directory where we will search
		 * @param name			Name of the searched file
		 * @param extension		Extension of the file we want to find
		 * @return				The file path found, or empty value if none matched.
		 */
		static std::string lookForFile(const std::string& dir, const std::string& name, const std::string& extension) ;

	private:
		static Explorer_filter* m_filter ;

		Explorer_filter(bool with_gui_composant) ;
		virtual ~Explorer_filter() ;

		class FileType
		{
			public:
				FileType(Glib::ustring format, Glib::ustring type,
							Glib::ustring ext,  Glib::ustring display,
							Glib::ustring ico ="",
							bool is_special=false, bool is_importable=false) ;
				virtual ~FileType() {} ;
				Glib::ustring extension ;
				Glib::ustring display ;
				Glib::ustring icon ;
				Glib::ustring format ;
				Glib::ustring type ;
				bool special ;
				bool importable ;

				void print()
				{
					Log::setTraceLevel(Log::OFF) ;
					Log::trace() << "Format=" << format << std::endl ;
					Log::trace() << "Type=" << type << std::endl ;
					Log::trace() << "Display=" << display << std::endl ;
					Log::trace() << "Extension=" << extension << std::endl ;
					Log::trace() << "icon=" << icon << std::endl ;
				}
		} ;


	private:
		Gtk::ComboBoxText* combo ;
		Glib::ustring filter_chosen ;

		FileType* folder ;
		FileType* none ;
		FileType* all ;

		std::map<Glib::ustring, FileType*> annotations ;
		std::map<Glib::ustring, FileType*> audios ;
		std::map<Glib::ustring, FileType*> images ;
		std::map<Glib::ustring, FileType*> videos ;
		std::map<Glib::ustring, FileType*> others ;

		void fill() ;
		void set_combo(Glib::ustring default_option);
		bool is_extension_in_map(Glib::ustring ext, const std::map<Glib::ustring, FileType*> formatsMap) ;
		std::vector<Glib::ustring> get_extensions_from_map(const std::map<Glib::ustring, FileType*> map) ;
} ;

} //namespace

#endif
