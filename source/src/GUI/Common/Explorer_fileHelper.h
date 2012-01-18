/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef EXPLORER_FILEHELPER_H_
#define EXPLORER_FILEHELPER_H_

#include <glib.h>
#include <gtkmm.h>

#include "Common/FileInfo.h"

#define EXPLORER_FI_SIZE 0
#define EXPLORER_FI_TYPE 1
#define EXPLORER_FI_CHANNEL 2
#define EXPLORER_FI_SAMPLING_RATE 3
#define EXPLORER_FI_TOTAL_SAMPLE 4
#define EXPLORER_FI_SAMPLING_RES 5
#define EXPLORER_FI_IMPORT_STATE 6
#define EXPLORER_FI_ENCODING 7
#define EXPLORER_FI_MODTIME 8
#define EXPLORER_FI_ACTIME 9
#define EXPLORER_FI_NAME 10
#define EXPLORER_FI_DURATION 11

namespace tag {

/**
 * @class Explorer_fileHelper
 * @ingroup GUI
 *
 * Static methods for some file operation specific to TranscriberAG GUI
 *
 */

class Explorer_fileHelper
{
	public:
		Explorer_fileHelper();
		virtual ~Explorer_fileHelper();

		/**
		 * Static method that computes file information in specific format
		 * from the file pointed by path
		 * @param path	Path of file that we wants to format data
		 * @return		Specific formatted string representing file data
		 */
		static Glib::ustring TAG_file_info(Glib::ustring path) ;

		/**
		 * Computes file size
		 * @param file_to_compute	File Info structure
		 * @return					Size of file, or 0 if File structure was null
		 */
		static int get_file_size(FileInfo* file_to_compute) ;

		/**
		 * Get file size from formatted string info computed by <em>TAG_file_info</em> method
		 * @param info	Formatted string info computed by <em>TAG_file_info</em> method
		 * @see			TAG_file_info()
		 * @note		Compare with <em>get_size_from_du</em> this method gives back
		 * 				wrong directory size (linux like - just block size)
		 * @return		Size of file in bytes
		 */
		static int get_size_from_file_info(Glib::ustring info) ;

		/**
		 * Take all the audio paths candidates for the creation of a new
		 * transcription and check if they validate multi-audio constraints
		 * @param audios	vector of all audio file paths
		 * @return			-3: no unique files\n
		 * 					-2: one of file is a stereo (not allowed yet)\n
		 * 					-1: one of file has not the same encoding\n
		 * 					 1: one of file has not the same rate\n
		 * 					 2: ok for rate, encoding and channels\n
		 */
		static int check_multi_audio(std::vector<Glib::ustring> audios) ;
};

} //namespace

#endif /*EXPLORER_FILEHELPER_H_*/
