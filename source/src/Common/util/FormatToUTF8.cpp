/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "FormatToUTF8.h"
#include "Common/globals.h"
#include <fstream>

namespace tag
{

Glib::ustring FormatToUTF8::checkUTF8charBYchar(const std::string& s)
{
	std::string::const_iterator it ;
	Glib::ustring res = "" ;
	for (it=s.begin(); it!=s.end(); it++)
	{
		gchar c = *it ;
		gboolean valid = g_utf8_validate(&c, 1*sizeof(gchar), NULL) ;
    	if (valid){
    		res.append(Glib::ustring(1, c)) ;
    	}
		else {
			res.append(TAG_INVALID_UTF8_CHAR) ;
		}
	}
	return res ;
}

bool FormatToUTF8::convertFromEncodingToUTF8(const std::string& s, const std::string& input_code, Glib::ustring& res)
{
	try {
		Glib::ustring converted = Glib::convert(s, "UTF-8", input_code) ;
		res = converted ;
		return true ;
	}
	catch (const Glib::Error& e) {
		Log::err() << "conversion from " << input_code << " to UTF8 failed: " << e.what() << std::endl ;
		res = "" ;
		return false ;
	}
}

int FormatToUTF8::convertFromLocaleToUTF8(const std::string& s, Glib::ustring& res)
{
	std::string locale ;
	bool utf8 = Glib::get_charset(locale) ;

	//> locale is utf8, no need conversion
	if (utf8)
		return 2 ;

	//> otherwise try to convert
	try {
		Glib::ustring converted = Glib::locale_to_utf8(s) ;
		res = converted ;
		return 1 ;
	}
	catch (const Glib::Error& e) {
		Log::err() << "conversion from locale (" << locale << ") to UTF8 failed: " << e.what() << std::endl ;
		res = "" ;
		return -1 ;
	}
}

Glib::ustring FormatToUTF8::checkUTF8(const std::string& s, const std::string& input_code, bool fastMode, int& err)
{
	bool done = false ;
	int code = -5 ;
	Glib::ustring res = "" ;

	//> is s is utf8 return
	if (g_utf8_validate(s.c_str(), -1, NULL)) {
		err = 1 ;
		return s ;
	}

	//> convert if input code is specified
	if (!input_code.empty()) {
		err = 2 ;
		done = convertFromEncodingToUTF8(s, input_code, res) ;
	}
	//> otherwise check if locale is ok
	else {
		err = 3 ;
		code = convertFromLocaleToUTF8(s, res) ;
	}

	//> if locale conversion failed OR locale is already UTF8
	//  parse string for allowing parsed display
	if (code<0 || code==2) {
		err = 4 ;
		if (!fastMode)
			res = checkUTF8charBYchar(s) ;
		else
			res = "" ;
	}

	return res ;
}

Glib::ustring FormatToUTF8::checkUTF8(const std::string& s, const std::string& input_code, bool fastMode)
{
	int err ;
	return checkUTF8(s, input_code, fastMode, err) ;
}


Glib::ustring FormatToUTF8::guessEncoding(const std::string& s)
{
	Glib::ustring res_unused ;
	std::string locale_unused ;
	bool converted ;

	//> case 1 : UTF8
	Glib::ustring encoding = "UTF-8" ;
	bool utf8 = g_utf8_validate(s.c_str(), -1, NULL) ;
	if (utf8)
		return encoding ;

	//> case 2 : ISO-8859-1
	encoding = "ISO-8859-1" ;
	converted = convertFromEncodingToUTF8(s, encoding, res_unused) ;
	if (converted)
		return encoding ;

	//> case 3 : ISO-8859-15
	encoding = "ISO-8859-15" ;
	converted = convertFromEncodingToUTF8(s, encoding, res_unused) ;
	if (converted)
		return encoding ;

	//> case 5 : CPP-1252
	encoding = "CPP-1252" ;
	converted = convertFromEncodingToUTF8(s, encoding, res_unused) ;
	if (converted)
		return encoding ;

	// TODO add all others encoding ?

	//> other cases: not found
	TRACE << "no encoding found" << std::endl ;
	return "" ;
}


std::string FormatToUTF8::guessFileEncoding(std::string path)
{
	Log::err() << "ENCODING GUESSER: STaRT for " << path << std::endl ;

	if (!Glib::file_test(path, Glib::FILE_TEST_EXISTS)) {
		Log::err() << "no file" << std::endl ;
		return  "" ;
	}

	std::ifstream fi(path.c_str());
	std::string readbuf = "" ;
	std::string text = "" ;
	std::string encoding = "" ;

	bool found = false ;

	try
	{
		while (fi.good() && !found)
		{
			getline(fi, readbuf) ;
			if (!readbuf.empty())
			{
				encoding = tag::FormatToUTF8::guessEncoding(readbuf) ;
				if (encoding!="UTF-8")
					found = true ;
			}
		}
		fi.close();
	}
	catch (...)
	{
		Log::err() << "reading error" << std::endl ;
		fi.close();
		return "" ;
	}

	TRACE << "ENCODING GUESSER: EnD" << std::endl ;
	return encoding ;
}

//std::string FormatToUTF8::guessFileEncoding(std::string path)
//{
//	Log::err() << "ENCODING GUESSER: STaRT" << std::endl ;
//
//	if (!Glib::file_test(path, Glib::FILE_TEST_EXISTS)) {
//		Log::err() << "no file" << std::endl ;
//		return  "" ;
//	}
//
//	std::string readbuf = "" ;
//
//	std::string text = "" ;
//	std::string encoding = "" ;
//
//	bool found = false ;
//
//	int MAX = 300 ;
//	FILE* file = NULL ;
//	file = fopen(path.c_str(),"r") ;
//	char buf[MAX] ;
//	std::string tmp ;
//	if (file!=NULL)
//	{
//		while ( fgets(buf, MAX, file) !=NULL && !found)
//		{
//			tmp = std::string(buf) ;
//			encoding = tag::FormatToUTF8::guessEncoding(tmp) ;
//			if (encoding!="UTF-8")
//				found = true ;
//		}
//		fclose(file) ;
//	}
//	else
//	{
//		Log::err() << "reading error" << std::endl ;
//		fclose(file) ;
//		return "" ;
//	}
//
//	Log::err() << "ENCODING GUESSER: EnD" << std::endl ;
//	return encoding ;
//}


} // namespace


