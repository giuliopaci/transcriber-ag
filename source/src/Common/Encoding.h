/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef _HAVE_ENCODING_H
#define _HAVE_ENCODING_H

#include <string>
#include <string.h>
#include <ctype.h>
#include <string>

namespace utils {
/**
* @class 		Encoding
* @ingroup		Common
*
* Encoding utilities
*/
class Encoding
{

	public:
		virtual ~Encoding() {}
		/**
		 * @return 		Encoding name
		 */
		virtual const char* name() const = 0;
		/**
		 * Checks whether the string is UTF8 encoded
		 * @param pt	String to be checked
		 * @return		True if ok, False otherwise
		 */
		virtual bool valid_string(const char* pt) const = 0;

		virtual int strlen(const char* pt, bool with_check=false) const =0;
		virtual int strlen(const std::string& s, int start=0, bool with_check=false) const =0;
		virtual int strnlen(const char* pt, const char* pend, bool with_check=false) const =0;
		int strnlen(const char* pt, int nboct, bool with_check=false) const
		{ return strnlen(pt, pt+nboct, with_check); }
		virtual int strnlen_bytes(const char* pt, int nbchar, bool with_check=false) const =0;
		int strnlen_bytes(const std::string& s, int nbchar, bool with_check=false) const
		{ return strnlen_bytes(s.c_str(), nbchar, with_check); }
		virtual int nboct(const char* pt, bool with_check=false) const = 0;
		virtual int nboct(const std::string& s, int pos, bool with_check=false) const = 0;

		/**
		 * Wrapper for <em>Glib::ispunct</em> method
		 * @param c		Character to check
		 * @return		<em>ispunct</em> returned value
		 */
		virtual bool is_punct(char c) const = 0;

		/**
		 * Return a pointer on the Encoding instance corresponding to the given
		 * encoding name
		 * @param encoding		%Encoding name
		 * @return				The corresponding Encoding instance
		 */
		static const Encoding* getEncoding(std::string encoding) throw (const char*);

};

/**
* @class 		Encoding_latin1
* @ingroup		Common
*
* Encoding utilities for ISO-8859-1
*/
class Encoding_latin1 : public Encoding
{
	public:
		const char* name() const { return "iso8859-1"; }
		bool valid_string(const char* pt) const { return true; }
		int strlen(const char* pt, bool with_check=false) const
		{ return ::strlen((const char*)pt); }
		int strlen(const std::string& s, int start=0, bool with_check=false) const
		{ return (s.length() - start); }
		int strnlen(const char* pt, const char* pend, bool with_check=false) const
		{ int l = ::strlen(pt); return ( pend > pt && (pend-pt) < l ? (pend-pt) : l); }
		int strnlen_bytes(const char* pt, int nbchar, bool with_check=false) const
		{ return nbchar; }
		int nboct(const char* pt, bool with_check=false) const
		{ return (*(const char*)pt == 0 ? 0 : 1); }
		int nboct(const std::string& s, int pos, bool with_check=false) const
		{ return (pos < (int)s.length() ?  1 : 0); }
		bool is_punct(char c) const
		{ return (ispunct(c) != 0); }
};

/**
* @class 		Encoding_utf8
* @ingroup		Common
*
* Encoding utilities for UTF-8
*/
class Encoding_utf8 : public Encoding
{
	public:
		const char* name() const { return "utf-8"; }
		bool valid_string(const char* pt) const ;
		int strlen(const char* pt, bool with_check=false) const ;
		int strlen(const std::string& s, int start=0, bool with_check=false) const;
		int strnlen(const char* pt, const char* pend, bool with_check=false) const ;
		int strnlen_bytes(const char* pt, int nbchar, bool with_check=false) const;
		int nboct(const char* pt, bool with_check=false) const ;
		int nboct(const std::string& s, int pos, bool with_check=false) const ;
		bool is_punct(char c) const
		{ return (ispunct(c) != 0); }

		/**
		 * Converts some strange UTF-8 punct signs (quotes) to
		 * their basic monobyte equivalent.
		 * @param token
		 */
		static void convertPunct(std::string& token) ;
};

} /* namespace utils */

#endif // _HAVE_ENCODING_H
