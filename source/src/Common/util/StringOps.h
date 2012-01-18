/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* @file StringOps.h
* @brief various thread-safe string operations
*/

#ifndef _HAVE_STRINGOPS_H
#define _HAVE_STRINGOPS_H 1

#include <string>
#include <vector>
#include <map>

/**
 * @var	STRING_OPS_FORBIDDEN_FILE
 * Forbidden chars for file name
 */
extern char STRING_OPS_FORBIDDEN_FILE[] ;

/**
 * @var	STRING_OPS_FORBIDDEN_XML
 * Forbidden chars for xml format
 */
extern char STRING_OPS_FORBIDDEN_XML[] ;

/**
* @class 		StringOps
* @ingroup		Common
*
* Various thread-safe string operations
*
*/
class StringOps
{
	public:
		/**
		* Constructor
		* @param src source string (may be modified by future operations)
		*/
		StringOps(std::string& src) : _str(src) {};

		/**
		* Constructor
		* @param src source string (may be modified by future operations)
		 */
		StringOps(const std::string& src) : _tmp(const_cast<std::string&>(src)) , _str(_tmp) {};


		/**
		* Constructor
		* @param src source string (as const char* - will not be modified by future operations)
		*/
		StringOps(const char* src) : _tmp(src), _str(_tmp) {};

		/**
		* Default constructor -> will use internal storage
		*/
		StringOps() : _str(_tmp) {};

		/**
		* Gets token count in string
		* @param delims 	Token delimiters
		* @return 			Token count
		*/
		int getTokenCount(const char* delims=" \t\n");


		/**
		* Extracts string token
		* @param start 			Start position in source string (is updated by call)
		* @param dst 			Destination string
		* @param delims 		Token delimiters
		* @param skipempty 		If true and more than one consecutive delim char found, skip them all to point to next token; if false, means empty tokens will be returned
		* @return 				True if more tokens remaining in source string, False otherwise
		*/
		bool getToken( unsigned int& start, std::string& dst, const char* delims, bool skipempty=true);


		/**
		* Splits string in tokens
		* @param dst 		Destination string vector
		* @param delims 	Token delimiters
		* @param skipempty 	If false, means empty tokens will also be returned
		* @return 			Number of token found
		*/
		int split(std::vector<std::string>& dst, const char* delims, bool skipempty=true);

		/**
		* Concats string vector tokens in result string
		* @param tokens 	String tokens vector
		* @param delim 		Item separator
		* @return 			Concatenated string
		*/
		const std::string& concat(const std::vector<std::string>& tokens, const std::string& delim=";");

		/**
		* Parses XML attributes string
		* @param[out] 	items			Vector where elements will be placed
		* @param 		lower_names 	True to lowercase attribute names
		* @return 						True if parsing succeeded, False otherwise
		*/
		bool parseXMLAttr(std::map<std::string, std::string>& items, bool lower_names=false);

		/**
		* Parses time specification string and return time in seconds
		* @return 		Time value (in seconds)
		*
		* @note Supported formats for time string are "hh:mm:ss.ddd", "mm:ss.dd" or "ss.ddd"
		*   if time string format is invalid, an error message is thrown.
		*/
		float parseTimeStr() throw (const char*);


		/**
		*  Sets string to lowercase
		*  @return 		The modified string
		*/
		std::string& toLower();

		/**
		*  Sets string to uppercase
		*  @return 		The modified string
		*/
		std::string& toUpper();


		/**
		*  Trims space chars at begining and end of string
		*  @return 		The modified string
		*/
		std::string& trim(const char* trimchars=" \t\n");


		/**
		* Checks if string contains substring
		* @param sub 			Substring to find
		* @param ignorecase 	True (default) / False; if true, assume sub is lowercased.
		* @return				True if match found
		*/
		bool contains(const char* sub, bool ignorecase=true);

		/**
		* Checks if string contains substring
		* @param sub 			Substring to find
		* @param ignorecase 	True (default) / False; if true, assume sub is lowercased.
		* @return				True if match found
		*/
		bool contains(const std::string& sub, bool ignorecase=true)
		{
			return contains(sub.c_str(), ignorecase);
		}

		/**
		* Checks if string is equal to other string, not taking casing into account
		* @param sub 		String to compare with
		* @return 			True if string matches
		*/
		bool equalsIgnoreCase(const char* sub);

		/**
		* Replaces a substring by given replacement string
		* @param sub 		Substring to find and replace
		* @param repl		Replacement string
		* @param do_all 	If true replace all occurrences in string, else replace only first occurrence (true by default)
		* @return 			Resulting string
		*/
		std::string& replace(const std::string& sub, const std::string& repl, bool do_all=true);

		/**
		* Replaces any char listed in sub by given replacement string
		* @param sub 		chars to be replaced
		* @param repl		Replacement string
		* @param do_all 	If true replace all occurrences in string, else replace only first occurrence (true by default)
		* @return 			Resulting string
		*/
		std::string& replaceAnyOf(const std::string& sub, const std::string& repl, bool do_all=true);

		/**
		* Checks if string contains forbidden chars
		* @param forbidden  Forbidden chars
		* @param s 			String to check
		* @return 			True if string doesn't contain forbidden chars, else false
		*/
		static bool check_string(std::string forbidden, const std::string s) ;

		/**
		* Gets integer value from string buffer
		* @return 		Integer value
		*/
		int toInt();

		/**
		* Sets integer value in string buffer
		* @return 		String buffer
		*/
		std::string& fromInt(int val, const char* fmt="%d");

		/**
		* Gets float value from string buffer
		* @return 		Float value
		*/
		float toFloat();

		/**
		* Sets float value in string buffer
		* @return 		String buffer
		*/
		std::string& fromFloat(float val, const char* fmt="%g");

		/**
		 * Creates a DSCV item from the given map
		 * @param items		Map with values
		 * @return			Resulting DSCV string
		 */
		std::string makeDCSVStr(const std::map<std::string, std::string>& items) ;

		/**
		 * Gets the map corresponding to the DSCV string the StringOps has been
		 * instancied from.
		 * @param[out] items	Resulting map
		 * @return				True for success, False otherwise
		 */
		bool getDCSVItems(std::map<std::string, std::string>& items) ;

	private:
		std::string _tmp;
		std::string& _str;

};

/**
 * Appends an integer to a string
 * @param s		String
 * @param i		Number
 * @return		Concatened string with the string representation of the given number
 */
std::string& operator += (std::string& s,int i);

/**
 * Appends an integer to a string
 * @param s		String
 * @param i		Number
 * @return		Concatened string with the string representation of the given number
 */
std::string& operator + (std::string& s,int i);

/**
 * Appends a double to a string
 * @param s		String
 * @param d		Number
 * @return		Concatened string with the string representation of the given number
 */
std::string& operator += (std::string& s, double d) ;

/**
 * Appends a double to a string
 * @param s		String
 * @param d		Number
 * @return		Concatened string with the string representation of the given number
 */
std::string& operator + (std::string& s, double d) ;

#endif // _HAVE_STRINGOPS_H
