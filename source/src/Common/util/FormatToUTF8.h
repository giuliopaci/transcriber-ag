/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
* @file 	FormatToUTF8.h
* Basic UTF8 helper
*/

#ifndef FORMATTOUTF8_
#define FORMATTOUTF8_

#include <glibmm.h>
#include <iostream>

/**
 * @var TAG_INVALID_UTF8_CHAR
 * Character replacement
 */
#define TAG_INVALID_UTF8_CHAR "�"

namespace tag
{

/**
 * @class 		FormatToUTF8
 * @ingroup		Common
 *
 * Basic UTF8 helper
 *
 */
class FormatToUTF8
{
	public :
	    // -----------------------------------------------------------------------
	    //  Static methods
	    // -----------------------------------------------------------------------

		/**
		 * Checks the valid UTF8 form of a given string.\n
		 * Uses convertFromEncodingToUTF8(const std::string&,const std::string&,Glib::ustring&),
		 * convertFromLocaleToUTF8(const std::string&,Glib::ustring&) and checkUTF8charBYchar(const std::string&)
		 * in successives conversion.
		 * @param 		s				String to check
		 * @param 		input_code		Current encoding of the string (if empty, the method will use the locale)
		 * @param 		fastMode		If False and conversion from encoding or from locale to UTF8 failed,
		 * 								the string will be parsed characters by characters (efficient but slow).\n
		 * 								If True and previous conversion failed, the slow characters by characters
		 * 								parsing will be skipped and an empty string is returned.
		 * @param[out] 	code			Indicates which processing has been done
										1: s is already in UTF8, nothing done\n
		 * 								2: conversion from encoding
		 * 								3: conversion from locale
		 * 								4: if <em>fastMode</em> set to True, conversion char by char
		 * @return						The UTF8 formated string or empty if failure
		 */
		static Glib::ustring checkUTF8(const std::string& s, const std::string& input_code, bool fastMode, int& code) ;

		/**
		 * Wrapper to checkUTF8(const std::string&,const std::string&,bool,int&) for ignoring code
		 * @param 		s				String to check
		 * @param 		input_code		Current encoding of the string (if empty, the method will use the locale)
		 * @param 		fastMode		If False and conversion from encoding or from locale to UTF8 failed,
		 * 								the string will be parsed characters by characters (efficient but slow).\n
		 * 								If True and previous conversion failed, the slow characters by characters
		 * 								parsing will be skipped and an empty string is returned.
		 * @return						The UTF8 formated string or empty if failure
		 */
		static Glib::ustring checkUTF8(const std::string& s, const std::string& input_code, bool fastMode) ;

		/**
		 * Checks a string characters by characters. Each time an invalid UTF8 character
		 * is found, it is replaced by TAG_INVALID_UTF8_CHAR
		 * @param s		String to check
		 * @return		Valid UTF8 string
		 */
		static Glib::ustring checkUTF8charBYchar(const std::string& s) ;

		/**
		 * Converts a string from given encoding to UTF8
		 * @param 		s				String to convert
		 * @param 		input_code		Encoding in which the given string should be
		 * @param[out] 	res				A valid UTF8 string, or empty string if conversion failed
		 * @return						True for successful conversion, False otherwise
		 */
		static bool convertFromEncodingToUTF8(const std::string& s, const std::string& input_code, Glib::ustring& res) ;

		/**
		 * Converts a string from locale to UTF8
		 * @param 		s				String to convert
		 * @param[out] 	res				A valid UTF8 string, or empty string if conversion failed
		 * @return						True for successful conversion, False otherwise
		 */
		static int convertFromLocaleToUTF8(const std::string& s, Glib::ustring& res) ;

		/**
		 * Checks a string for guessing its encoding. (successive conversions)\n
		 * (checks UTF8, ISO-8859-1, ISO-8859-15, ISO-8859-2, ISO-8859-5,
	     *		   ISO-8859-6, CPP-1252, check CPP-1251, check CPP-1256)
		 * @param s		String to check
		 * @return		The encoding found, or empty string if encoding couldn't be determined.
		 */
	    static Glib::ustring guessEncoding(const std::string& s) ;

	    /**
	     * Checks a file encoding using guessEncoding(const std::string&) method.\n
	     * Declares the file UTF8-valid if <b>all</b> characters found are UTF8 compatible.
	     * @param path		File path
		 * @return			The encoding found, or empty string if encoding couldn't be determined.
	     */
	    static std::string guessFileEncoding(std::string path) ;
};

} // namespace

#endif /*FORMATTOUTF8_*/

