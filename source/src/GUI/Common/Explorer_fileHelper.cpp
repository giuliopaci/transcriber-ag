/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

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

#include "MediaComponent/base/Guesser.h"
#include "Explorer_fileHelper.h"
#include "Explorer_utils.h"
#include "Common/util/Utils.h"
#include "Common/icons/Icons.h"
#include "Common/Explorer_filter.h"
#include "Common/util/StringOps.h"


namespace tag {


//**************************************************************************************
//**************************************************************************** CONSTRUCT
//**************************************************************************************


Explorer_fileHelper::Explorer_fileHelper()
{
}

Explorer_fileHelper::~Explorer_fileHelper()
{
}

//**************************************************************************************
//****************************************************************************** METHODS
//**************************************************************************************

int Explorer_fileHelper::get_file_size(FileInfo* file_to_compute)
{
	int size_result = 0;

//	// if Directory then parse all files inside
//	if ( file_to_compute->isDirectory() )
//	{
//		GError *error = NULL;
//		GDir* dir = g_dir_open(path.c_str(), 0, &error);
//		const char* filename;
//		string aux_path ="";
//
//		FileInfo* dir_to_compute;
//		// parse all files in
//		while ( (filename = g_dir_read_name(dir)) )
//		{
//
//			aux_path = path + "/" + filename;
//			dir_to_compute = new FileInfo(aux_path);
//
//			size_result += dir_to_compute->size();
//
//			// if the inside file is a directory then reapply the get_size_rec func
//			if ( dir_to_compute->isDirectory() )
//				size_result += get_file_size(aux_path);
//
//
//			delete dir_to_compute;
//		}
//		g_dir_close(dir);
//	}
//	else
//		size_result = file_to_compute->size();
//
//	delete file_to_compute;

	if (file_to_compute)
		size_result = file_to_compute->size() ;

	return size_result;
}

/**
 * 	Construct a string
 */
Glib::ustring Explorer_fileHelper::TAG_file_info(Glib::ustring path)
{
	struct stat buf ;
	int stat_ok = g_stat(path.c_str(), &buf) ;

	//> from ustring to char*
	Glib::ustring res ;
	FileInfo* f = new FileInfo(path.c_str()) ;

	//> NAME
	Glib::ustring name_file = Glib::path_get_basename(path) ;

	//> TYPE
	Glib::ustring ext ;
	if ( Glib::file_test(path, Glib::FILE_TEST_IS_DIR) )
		ext="dir" ;
	else
		ext = Explorer_filter::get_extension(name_file);

	//true if audio file, false other types
	bool other = false ;

	Glib::ustring size, channels_s, samplingRate_s, totalSamplesCount_s ;
	Glib::ustring samplingResolution_s, local_s, encoding, mdate_s, acdate_s ;
	Glib::ustring duration ;

	//> SIZE DATA
	int size_int = Explorer_fileHelper::get_file_size(f) ;
	if (size_int == 0)
		size = "0" ;
	else
		size =  number_to_string(size_int) ;

//	size = number_to_string(Explorer_fileHelper::get_file_size(path));

	//> AUDIO DATA
	Explorer_filter* filter = Explorer_filter::getInstance() ;
	if (filter->is_audio_file(path))
	{
		try {
			IODevice* device = Guesser::open(path.c_str()) ;
			if (device) {
				int channels = device->m_info()->audio_channels ;
				int samplingRate =  device->m_info()->audio_sample_rate ;
				long totalSamplesCount = device->m_info()->audio_samples ;
				double durat = device->m_info()->audio_duration ;
				int samplingResolution = device->m_info()->audio_sample_resolution ;

				encoding = device->m_info()->audio_encoding ;
				ext = device->m_info()->audio_codec ;
				duration = number_to_string(durat) ;
				channels_s = number_to_string(channels)  ;
				samplingRate_s = number_to_string(samplingRate)  ;
				totalSamplesCount_s = number_to_string(totalSamplesCount) ;
				if (samplingResolution>0)
					samplingResolution_s = number_to_string(samplingResolution) ;
				else
					samplingResolution_s = "" ;

				local_s = "local" ;
				device->m_close() ;
				delete(device) ;
			}
			else
				other = true ;
		}
		catch (const char* e) {
			other = true ;
		}
	}
	//> other file, default values
	else
		other = true ;

	if (other) {
		ext = filter->get_display_from_ext(ext) ;
		other = true ;
		channels_s = ""  ;
		samplingRate_s = ""  ;
		totalSamplesCount_s = "" ;
		samplingResolution_s = "" ;
		local_s = "local" ;
		encoding = "" ;
		duration = "" ;
	}

	//> DATES FILE
	if (stat_ok>=0) {
		char date[100] ;
		strcpy(date, ctime(&(buf.st_mtime)) ) ;
		g_strstrip(date) ;
		mdate_s = Glib::ustring(date) ;
		char date2[100] ;
		strcpy(date2, ctime(&(buf.st_ctime)) ) ;
		g_strstrip(date2) ;
		acdate_s = Glib::ustring(date2) ;
	}
	else {
		mdate_s = "" ;
		acdate_s = "" ;
	}

	//> Add information in vector
	//  DIRTY CAUTION: the order is linked with numbers defined in Explorer_fileHelper.h !
	//when adding one add a define too
	res.append(size) ; res.append(",");
	res.append(ext) ;  res.append(",");
	res.append(channels_s) ; res.append(",");
	res.append(samplingRate_s) ; res.append(",");
	res.append(totalSamplesCount_s) ; res.append(",");
	res.append(samplingResolution_s) ; res.append(",");
	res.append(local_s); res.append(",");
	res.append(encoding); res.append(",");
	res.append(mdate_s); res.append(",");
	res.append(acdate_s); res.append(",");
	res.append(name_file) ; res.append(",");
	res.append(duration) ;

	if (f)
		delete(f) ;

	return res ;
}

int Explorer_fileHelper::get_size_from_file_info(Glib::ustring info)
{
	int size = -1 ;
	std::vector<Glib::ustring> vect ;
	int result = mini_parser(',', info, &vect) ;
	if (result!=-1) {
		size = string_to_number<int>(vect[EXPLORER_FI_SIZE]) ;
	}
	return size ;
}

int Explorer_fileHelper::check_multi_audio(std::vector<Glib::ustring> audios)
{
	std::vector<Glib::ustring>::iterator it ;

	string last_encoding = "" ;
	double last_frame = 0 ;

	bool onlymono = true ;
	bool sametype = true ;
	bool samerate = true ;
	bool unique = true ;

	std::set<Glib::ustring> unique_l ;

	for (it=audios.begin(); it!=audios.end() && onlymono && sametype ; it++)
	{
		IODevice* device = Guesser::open((*it).c_str()) ;
		if (device)
		{
			int channel = device->m_info()->audio_channels ;
			string type = device->m_info()->audio_codec ;
			float rate = device->m_info()->audio_sample_rate ;

			if (channel != 1)
				onlymono = false ;
			if (!last_encoding.empty() && type != last_encoding)
				sametype = false ;
			if (samerate && last_frame!=0 && rate != last_frame)
				samerate = false ;

			last_encoding = type ;
			last_frame = rate  ;

			device->m_close() ;
			delete(device) ;
		}
		unique_l.insert(*it) ;
	}

	if (unique_l.size() != audios.size())
		unique = false ;

	if (!onlymono)
		return -2 ;
	if (!sametype)
		return -1 ;
	if (!unique)
		return -3 ;
	if (!samerate)
		return 1 ;

	return 2 ;
}

} //namespace
