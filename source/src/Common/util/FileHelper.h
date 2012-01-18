/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
* @file 	FileHelper.h
* Regular expression management
*/

#ifndef FILEHELPER_H_
#define FILEHELPER_H_

#include <glib.h>
#include <gtkmm.h>

namespace tag {

/**
 * @class 		FileHelper
 * @ingroup		Common
 *
 * Utilities for file treatments.
 *
 */
class FileHelper
{
	public:
		/**
		 * Constructor
		 * @return
		 */
		FileHelper();
		virtual ~FileHelper();

		/**
		 * Concats <em>part1</em> and <em>part2</em> with filesytem separator (depending on the OS)
		 * @param part1		First part of the path
		 * @param part2		Last part of the path
		 * @return			Resulting path
		 */
		static Glib::ustring build_path(const Glib::ustring& part1, const Glib::ustring& part2) ;

		/**
		 * Path delimiter, depending on the OS.
		 */
		static std::string _path_delim ;

		/**
		 * Checks the write permission
		 * @param path		File path
		 * @return			True if the file can be written, False otherwise
		 */
		static bool is_writable(const std::string& path) ;

		/**
		 * Checks the read permission
		 * @param path		File path
		 * @return			True if the file can be read, False otherwise
		 */
		static bool is_readable(const std::string& path) ;

		/**
		 * Checks the execution permission
		 * @param path		File path
		 * @return			True if the file can be executed, False otherwise
		 */
		static bool is_executable(const std::string& path) ;

		/**
		 * Checks if a file of given name exists in given directory
		 * @param dir_path		Directory path
		 * @param file_name		File name
		 * @return				True a file with corresponding name was found, False otherwise
		 */
		static bool exist_file_in_dir(const std::string& dir_path, const std::string& file_name) ;

		/**
		 * Checks filename
		 * @param path		File path
		 * @return			 -5  : already existing\n
		 *   				 <0  : invalid\n
		 * 	 	 			 >=0 : success\n
		 * @see				is_valid_filename(const std::string&)
		 */
		static int check_rename_filename(const std::string& path) ;

		/**
		 * Removes a file from filesystem (using g_remove)
		 * @param path		Path of the file to remove
		 * @param bar		ProgressBar if needed (NULL otherwise)
		 * @return			-41  : file doesn't exist\n
		 * 					-20  : permissions denied (read / write)\n
		 * 					<0   : error of g_remove\n
		 * 					>=0  : success
		 */
		static int remove_from_filesystem(const std::string& path, Gtk::ProgressBar* bar=NULL) ;

		/**
		 * Creates a directory at given path (using g_mkdir)
		 * @param path		File path of the new directory
		 * @return			-20  : permissions denied\n
		 * 					<0   : error of g_mkdir\n
		 * 					>=0  : success
		 */
		static int create_directory(const std::string& path) ;

		/**
		 * Creates a directory at given path, creating all parents directories
		 * if they don't exist yet. (using g_mkdir_with_parents)
		 * @param path		File path of the new directory
		 * @return			-20  : permissions denied\n
		 * 					<0   : error of g_mkdir_with_parents\n
		 * 					>=0  : success
		 */
		static int create_directory_with_parents(const std::string& path) ;

		/**
		 * Renames a file in file system (using g_mkdir)
		 * @param path			Path of the file to rename
		 * @param new_path		New path of file
		 * @return				-41: file doesn't exist\n
		 * 						-20: permissions denied (read / write)\n
		 * 						<0 : error of g_rename\n
		 * 						>=0: success
		 */
		static int rename_in_filesystem(const std::string& path, const std::string& new_path) ;

		/**
		 * Copy a file or a directory
		 * @param src_file				Path of the file to copy
		 * @param dest_directory		Destination directory path
		 * @param bar					ProgressBar if needed (NULL otherwise)
		 * @return						-41: <em>src_file</em> does not exist\n
		 *								-42: <em>dest_directory</em> does not exist\n
		 *								-30: <em>dest_directory</em> is not a directory\n
		 *								-20: permissions denied (read / write)\n
		 *								-10: destination and source are same file\n
		 *								<0 : error of g_rename\n
		 *								>=0: success
		 */
		static int copy_in_filesystem(const std::string& src_file, const std::string& dest_directory, Gtk::ProgressBar* bar=NULL) ;

		/**
		 * Copy a file with a new name
		 * @param src_path				Path of the file to copy
		 * @param dest_path				New path
		 * @param threadProtection		True for thread protection
		 * @param bar					Progression bar to be used (NULL for none)
		 * @param skipFlushGui			True for disable the flush gui action. Set to false by
		 * 								default, should be set to true only for avoiding gui thread
		 * 								freeze.
		 * @return						-41: <em>src_file</em> does not exist\n
		 *								-42: <em>dest_directory</em> does not exist\n
		 *								-20: permissions denied (read / write)\n
		 *								-10: destination and source are same file\n
		 *								<0 : error\n
		 *								>=0: success
		 */
		static int copy_and_rename(const std::string& src_path, const std::string& dest_path, bool threadProtection, Gtk::ProgressBar* bar, bool skipFlushGui=false) ;

		/**
		 * Move a file or a directory
		 * @param src		Path of the file to be moved
		 * @param dest		Destination folder path
		 * @param bar		ProgressBar if needed (NULL otherwise)
		 * @return		 		-20: permissions denied (read / write)\n
		 *						<0 : error\n
		 *						>=0: success
		 */
		static int move_in_filesystem(const std::string& src, const std::string& dest, Gtk::ProgressBar* bar=NULL) ;

		/**
		 * Checks if there is a transcription file associated to the given audio file
		 * @param audio_path	Audio file path
		 * @return				Path of the matching transcription file, or empty value if none could be found
		 * @note				v1: basic research if transcription file with same name in same directory
		 */
		static std::string exist_transcription_file(const std::string& audio_path);

		/**
		 * Decomposes a file path following the file system separator and places
		 * all elements in a vector
		 * @param path		File path
		 * @param v			Pointer on the vector that will contain the result
		 * @return			Number of elements or -1 if an error occurred
		 */
		static int cut_path(const Glib::ustring& path, std::vector<Glib::ustring>* v) ;

		/**
		 * Searches an INFO file name for the given file name
		 * @param name		File name
		 * @return			A corresponding INFO file name (.info)
		 */
		static std::string get_name_audio_parameter_file(const std::string& name) ;

		/**
		 * Checks if all files contained in the given vector exist in file system
		 * @param files		Vector of file path
		 * @return			True if they exist, False otherwise
		 */
		static bool existFiles(const std::vector<Glib::ustring>& files) ;

		/**
		 * Checks if all files contained in the given vector exist in file system
		 * @param files		Vector of file path
		 * @return			True if they exist, False otherwise
		 */
		static bool existFiles(const std::vector<std::string>& files) ;

		/**
		 * Checks if the given file exists in file system
		 * @param file		File path
		 * @return			True if the file exists, false otherwise
		 */
		static bool existFile(const Glib::ustring& file) ;

		/**
		 * Checks if the given file exists in file system
		 * @param file		File path
		 * @return			True if the file exists, false otherwise
		 */
		static bool existFile(const std::string& file) ;

	private:
		static int basic_copy(const char* dst, const char* src, double size, bool threadProtection, Gtk::ProgressBar* bar=NULL, bool skipFlushGui= false) ;
};

} //namespace

#endif /*FileHelper_H_*/
