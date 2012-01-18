/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef TAG_ISO639_H_
#define TAG_ISO639_H_

/**
 * @namespace ISO639
 * Static methods for language codes accessor
 */
namespace ISO639
{
	/**
	* Gets the Iso639-2 code corresponding to the given 2 letters code
	* @param two_letter_code	A 2 letters code
	* @param bib 		if true, targets Iso639-2b, else Iso639-2t codes (default true)
	* @return					The corresponding Iso639-2 code or empty value if not found
	*/
	const char* get3LetterCode(const char* two_letter_code, bool bib = true);

	/**
	* Gets the 2 letters code corresponding to the given Iso639-2 code
	* @param three_letter_code		A Iso639-2b or Iso639-2t code
	* @return						The corresponding 2 letters code or empty value if not found
	*/
	const char* get2LetterCode(const char* three_letter_code);

	/**
	* Gets the language name corresponding to the given the Iso639-2 code,
	* using the given locale
	* @param three_letter_code		A Iso639-2b or Iso639-2t code
	* @param locale				Given locale
	* @return						The corresponding language name (english name if locale one couldn't be found)
	*/
	const char* getLanguageName(const char* three_letter_code, const char* locale="fre");

	/**
	* Gets the Iso639-2 code corresponding to the given language name,
	* @param name		Language name
	* @param bib 		if true, targets Iso639-2b, else Iso639-2t codes (default true)
	* @return			The corresponding Iso639-2 code
	*/
	const char* get3LetterCodeFromName(const char* name, bool bib = true );

	/**
	* @param bib 		if true, targets Iso639-2b, else Iso639-2t codes (default true)
	* @return 	The Iso639-2 code corresponding to the locale
	*/
	const char* getLocale(bool bib = true);

} /* namespace ISO639 */

#endif /*TAG_ISO639_H_*/
