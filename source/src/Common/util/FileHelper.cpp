/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "FileHelper.h"

#include "Utils.h"
#include "Common/icons/Icons.h"
#include "Common/Explorer_filter.h"
#include "Common/FileInfo.h"
#include "Common/util/StringOps.h"
#include "Common/widgets/GtUtil.h"

#include <glib/gstdio.h>
#include <unistd.h>
#include <string>
#include <set>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>
#include <time.h>


namespace tag {

#ifdef WIN32
  std::string FileHelper::_path_delim = "\\";
#else
  std::string FileHelper::_path_delim = "/";
#endif

//**************************************************************************************
//**************************************************************************** CONSTRUCT
//**************************************************************************************


FileHelper::FileHelper()
{
}

FileHelper::~FileHelper()
{
}


//**************************************************************************************
//****************************************************************************** METHODS
//**************************************************************************************

/* Search existing note file for a given audio file
 * v.1: only search note file with same name,
 * in same directory
 */
std::string FileHelper::exist_transcription_file(const std::string& audio_path)
{
	Explorer_filter* filter = Explorer_filter::getInstance() ;

	//directory of file for which we search a note file
	std::string father_path = Glib::path_get_dirname(audio_path) ;
	//name of audio file with extension
	std::string name_ext =  Glib::path_get_basename(audio_path) ;
	//idem without extension
	std::string name = Explorer_filter::cut_extension( Glib::path_get_basename (audio_path) );
	std::string tmp ;
	std::string res ;
	bool found = false ;

	try
	{
		Glib::Dir dir(father_path) ;
		//> go over all files in directory
		while ( ((tmp=dir.read_name())!="") && (found==false) ) {
			if ( filter->is_annotation_file(tmp) ) {
				res = Explorer_filter::cut_extension(tmp) ;
				//> if same name of file and not same file
				if (res==name && (tmp!=name_ext)) {
					found=true ;
					res = tmp ;//comment if  you don't want to keep extension in file name
				}
			}
		}
		dir.close() ;
		if (found)
			res = build_path(father_path,res) ;
		else
			res = "" ;
	}
	catch (Glib::FileError e) {
		print_trace("Explorer_fileHelper:> exist_note_file:> ", e.what(), 0) ;
		res = "" ;
	}
	return res  ;
}

bool FileHelper::existFiles(const std::vector<std::string>& signalFiles)
{
	if (signalFiles.empty())
		return false ;

	bool allExist = true ;
	std::vector<std::string>::const_iterator it_files ;
	for (it_files=signalFiles.begin(); it_files!=signalFiles.end() && allExist ; it_files++)
		allExist = allExist && existFile(*it_files) ;
	return allExist ;
}

bool FileHelper::existFile(const std::string& signalFile)
{
	if (signalFile.empty())
		return false ;

	return FileInfo(signalFile).exists() ;
}


bool FileHelper::existFiles(const std::vector<Glib::ustring>& signalFiles)
{
	if (signalFiles.empty())
		return false ;

	bool allExist = true ;
	std::vector<Glib::ustring>::const_iterator it_files ;
	for (it_files=signalFiles.begin(); it_files!=signalFiles.end() && allExist ; it_files++)
		allExist = allExist && existFile(*it_files) ;
	return allExist ;
}

bool FileHelper::existFile(const Glib::ustring& signalFile)
{
	if (signalFile.empty())
		return false ;

	return FileInfo(signalFile).exists() ;
}

Glib::ustring FileHelper::build_path(const Glib::ustring& left, const Glib::ustring& right)
{
	return Glib::build_filename(left,right) ;
}

bool FileHelper::exist_file_in_dir(const std::string& dir_path, const std::string& file_name)
{
	bool found = false ;
	std::string tmp ;

	try {
		Glib::Dir dir(dir_path) ;
		//> go over all files in directory
		while ( ((tmp=dir.read_name())!="") && (found==false) ) {
				if (tmp==file_name)
					found=true ;
		}
		dir.close();
	}
	catch (Glib::FileError e) {
		print_trace("Explorer_fileHelper:> exist_file_in_dir:>", e.what(), 0) ;
	}
	return found  ;
}


/*
 *  Check if a name is ok for file that beeing renamed
 *  Return:
 * 		-5: already existing
 *   	<0: invalid
 * 	 	 1: ok
 *
 */
int FileHelper::check_rename_filename(const std::string& path)
{
	std::string path_tmp = path ;
	std::string dir = Glib::path_get_dirname(path_tmp) ;
	std::string name = Glib::path_get_basename(path) ;
	std::string name_wt_ext = Explorer_filter::cut_extension(name) ;

	if ( exist_file_in_dir(dir, name) )
		return -5 ;
	else
		return is_valid_filename(name_wt_ext) ;
}


//**************************************************************************************
//*************************************************************************** FILESYSTEM
//**************************************************************************************


/*
 *  Remove a file or a directory, recursively
 *  Only remove from fileSystem
 */
int FileHelper::remove_from_filesystem(const std::string& path, Gtk::ProgressBar* bar)
{
	if (!Glib::file_test(path, Glib::FILE_TEST_EXISTS)) {
		return -41 ;
	}

	bool isDir = Glib::file_test(path, Glib::FILE_TEST_IS_DIR) ;
	std::string tmp ;
	std::string new_path ;
	int cpt = 0 ;

	std::string path_tmp = path ;
	std::string father = Glib::path_get_dirname(path_tmp) ;
	// father not writable / path is a not writable directory
	if (!is_writable(father) || (!is_writable(path)&&isDir)  )
		return -20 ;
	if (!is_executable(father))
		return -20 ;

	if (!isDir)
	{
		cpt = g_remove(path.c_str()) ;
		if (bar)
		{
			bar->pulse() ;
			bar->set_text(Glib::path_get_basename(path)) ;
		}
	}
	else
	{
		try
		{
			Glib::Dir dir(path) ;
			//> go over all files in directory
			while ( (tmp=dir.read_name())!="")
			{
				new_path = FileHelper::build_path(path, tmp) ;
				cpt =remove_from_filesystem(new_path, bar) ;
				// flush
				GtUtil::flushGUI(false,false) ;
			}
			dir.close() ;
			cpt = g_remove(path.c_str()) ;
		}
		catch (Glib::FileError e)
		{
			print_trace("FileHelper::remove_from_filesystem :> Glib::Dir::EXCEPTION", e.what(), 0) ;
		}
	}
	return cpt ;
}


int FileHelper::create_directory(const std::string& path)
{
	std::string path_tmp = path ;
	std::string father = Glib::path_get_dirname(path_tmp) ;
	// if father not writable

	if (!is_writable(father) || !is_executable(father))
		return -20 ;

#ifdef _WIN32
	return g_mkdir(path.c_str(), 0) ;
#else
	return g_mkdir(path.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IXUSR) ;
#endif
}

int FileHelper::create_directory_with_parents(const std::string& path)
{
	int res ;
	//if father not writable
#ifdef _WIN32
	res = g_mkdir_with_parents(path.c_str(), S_IRUSR|S_IWUSR|S_IXUSR) ;
#else
	res = g_mkdir_with_parents(path.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IXUSR) ;
#endif
	if (res>=0) {
		if (!is_writable(path) )
			res = -20 ;
	}

	return res ;
}


int FileHelper::rename_in_filesystem(const std::string& path, const std::string& new_path)
{
	std::string path_tmp = path ;
	std::string father = Glib::path_get_dirname(path_tmp) ;
	//if father not writable
	if (!Glib::file_test(path, Glib::FILE_TEST_EXISTS))
		return -41 ;
	if (!is_writable(father) || !is_executable(father))
		return -20 ;
	return g_rename( path.c_str(),new_path.c_str() ) ;
}

int FileHelper::basic_copy(const char* dst, const char* src, double size, bool threadProtection, Gtk::ProgressBar* bar, bool skipFlushGui)
{
	int MAX_SIZE = 512 ;
	char* buf[MAX_SIZE] ;

	FILE* fp_dst = fopen(dst, "wb");
	FILE* fp_src = fopen(src, "rb");

	if (fp_dst==NULL)
		return -1 ;

	if (fp_src == NULL)
		return -2 ;

	//char line[MAXLINE_LEN] ;
	int nombre = -2 ;
	int written = -2 ;
    int cpt = 0 ;
    int total_written = 0 ;
    int total_read = 0 ;
    float progression = 0.0 ;

    if (bar)
    {
    	string name = Glib::path_get_basename(string(src)) ;
    	bar->set_text(name) ;
    }

	while ( (nombre = fread(buf, 1, MAX_SIZE, fp_src)) > 0 )
	{
		//> Write
		written = fwrite(buf, 1, nombre, fp_dst) ;

		//> Count
		total_read = total_read + nombre ;
        total_written = total_written + written ;
        cpt++ ;

        //> Progress
        if (bar && total_written!=0)
			bar->set_fraction(total_written/size) ;

        //> Flush
        if (cpt%10==0 && !skipFlushGui)
			GtUtil::flushGUI(false,threadProtection) ;
	}

	int res = 1 ;
	res = fclose(fp_dst) ;
	fclose(fp_src) ;

    if (res==EOF)
    {
    	Log::err() << "basicopy:> file descriptor error" << std::endl ;
    	return -1 ;
    }
    if (total_read!=total_written)
    {
    	Log::err() << "basicopy:> error on written elements number" << std::endl ;
    	return -1 ;
    }
    if ( (size-total_read) > 0 )
    {
    	Log::err() << "basicopy:> error on read elements number" << std::endl ;
    	return -1 ;
    }
	Log::out() << "basicopy:> done." << std::endl ;
    return 1 ;
}

int FileHelper::copy_in_filesystem(const std::string& src, const std::string& folder_dest, Gtk::ProgressBar* bar)
{
	std::string path_tmp = src ;
	std::string father = Glib::path_get_dirname(path_tmp) ;
	if (!Glib::file_test(src, Glib::FILE_TEST_EXISTS))
		return -41 ;
	if (!Glib::file_test(folder_dest, Glib::FILE_TEST_EXISTS))
		return -42 ;
	if (!Glib::file_test(folder_dest, Glib::FILE_TEST_IS_DIR))
		return -30 ;
	//> if dest is not writable
	if (!is_writable(folder_dest) || !is_executable(folder_dest))
		return -20 ;
	//father
	if (!is_executable(father))
		return -20 ;

	//name of file to copy
	std::string name = Glib::path_get_basename(src) ;
	//future path of file after copy
	std::string new_path = build_path(folder_dest, name) ;

	FileInfo info(src) ;
	int size = info.size() ;

	//> Copy in same file
	if (src==new_path)
		return -10 ;

	//> Prepare
	//> Src is a file
	if (!Glib::file_test(src, Glib::FILE_TEST_IS_DIR))
	{
		int res = basic_copy(new_path.c_str(),src.c_str(), size, false, bar) ;
		if (res==-1)
			remove_from_filesystem(new_path) ;
		return res ;
	}
	//> Src is a directory
	else
	{
		std::string current_name ;
		std::string current_path ;

		//create target directory
		create_directory(new_path) ;
		try
		{
			Glib::Dir dir(src) ;
			//> for each files contained in it do reccurence
			while ( (current_name=dir.read_name()) != "" )
			{
				current_path = build_path(src, current_name) ;
				copy_in_filesystem(current_path, new_path, bar) ;
	        	// let event each file
				// flush
				GtUtil::flushGUI(false,false) ;
			}
			dir.close() ;
			return 1 ;
		}
		catch (Glib::FileError e)
		{
			print_trace("FileHelper::copy_in_filesystem:> DEST", new_path, 0) ;
			return -1 ;
		}
	}
}

int FileHelper::copy_and_rename(const std::string& src_path, const std::string& dest_path, bool threadProtection, Gtk::ProgressBar* bar, bool skipFlushGui)
{
	std::string path_tmp = src_path ;
	std::string dest_path_tmp = dest_path ;
	std::string father = Glib::path_get_dirname(path_tmp) ;
	std::string folder_dest = Glib::path_get_dirname(dest_path_tmp) ;
	if (!Glib::file_test(src_path, Glib::FILE_TEST_EXISTS))
		return -41 ;
	if (!Glib::file_test(folder_dest, Glib::FILE_TEST_EXISTS))
		return -42 ;
	//> if folder dest is not writable
	if (!is_writable(folder_dest) || !is_executable(folder_dest))
		return -20 ;
	//father
	if (!is_executable(father))
		return -20 ;

	FileInfo info(src_path) ;
	int size = info.size() ;

	//> copy in same file
	if (src_path==dest_path)
		return -10 ;

	//> src is a file
	if (!Glib::file_test(src_path, Glib::FILE_TEST_IS_DIR)) {
		int res = basic_copy(dest_path.c_str(),src_path.c_str(), size, threadProtection, bar, skipFlushGui) ;
		if (res==-1)
			remove_from_filesystem(dest_path) ;
		return res ;
	}
}

int FileHelper::move_in_filesystem(const std::string& src, const std::string& dest, Gtk::ProgressBar* bar)
{
	bool isDir = Glib::file_test(src, Glib::FILE_TEST_IS_DIR) ;
	std::string path_tmp = src ;
	std::string father = Glib::path_get_dirname(path_tmp) ;
	//> dest not writable / father dir not writable / src is a not writable directory
	if (!is_writable(father) || (!is_writable(src)&&isDir) || !is_executable(father))
		return -20 ;
	if ( !is_writable(dest) || !is_executable(dest) )
		return -20 ;

	//> copy to destnation
	int res = copy_in_filesystem(src, dest, bar) ;

	//> erase old
	if (res>=0) {
		res = remove_from_filesystem(src) ;
	}
	return res ;
}

int FileHelper::cut_path(const Glib::ustring& path, std::vector<Glib::ustring>* v)
{
	//WARNING!! ONLY FOR UNIX FOR THE MOMENT
	//TODO do it for WINDOW
	return mini_parser('/',path,v) ;
}

std::string FileHelper::get_name_audio_parameter_file(const std::string& path_or_name)
{
	std::string wt_extension = Explorer_filter::cut_extension(path_or_name) ;
	std::string new_path_or_name = wt_extension + ".info" ;
	return new_path_or_name	;
}

bool FileHelper::is_writable(const std::string& path)
{
	FileInfo info(path) ;
	return info.canWrite() ;
}

bool FileHelper::is_readable(const std::string& path)
{
	bool res = true ;
	int i = g_access(path.c_str(), R_OK) ;
	if (i!=0)
		res = false ;
	return res ;
}

bool FileHelper::is_executable(const std::string& path)
{
	bool res = true ;
	#if !defined(_WIN32)
	int i = g_access(path.c_str(), X_OK) ;
	if (i!=0)
		res = false ;
	#endif
	return res ;
}

} //namespace
