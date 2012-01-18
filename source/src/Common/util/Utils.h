/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
* @defgroup 	Common
*/


/**
* @file 	Utils.h
* @brief 	Various useful methods
*/

#ifndef UTILS_H_
#define UTILS_H_

#include <sys/time.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <math.h>

#ifndef TRACE_ALLOC
#define TRACE_ALLOC 0
#endif

#if TRACE_ALLOC == 0
#define FREE(a) free(a);
#define CALLOC(a,b) (b*) calloc(a, sizeof(b));
#define MALLOC(a,b) (b*) malloc(a*sizeof(b));
#else
#define FREE(a) my_free((void**)&a, __FILE__, __LINE__, TRACE_ALLOC);
#define CALLOC(a,b) (b*) my_calloc(a, sizeof(b), __FILE__, __LINE__, TRACE_ALLOC);
#define MALLOC(a,b) (b*) my_malloc(a*sizeof(b), __FILE__, __LINE__, TRACE_ALLOC);
#endif

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <set>
#include <sstream>
#include <glibmm.h>

#include "Log.h"

using namespace std ;

/**
 * Gets current time in milliseconds
 * @return		Time in milliseconds
 */
long get_time_millisec();

/**
 * Re-implementation of calloc method (enables OS portability)
 * @param nb			Element number
 * @param sz			Elements size
 * @param f				File
 * @param l				Line
 * @param trace_alloc	If set to 1 displays trace in err output
 */
void* my_calloc( int nb, int sz, const char* f, int l, int trace_alloc);

/**
 * Re-implementation of free method (enables OS portability)
 * @param a				Pointe on the pointerto free
 * @param f				File
 * @param l				Line
 * @param trace_alloc	If set to 1 displays trace in err output
 */
void my_free(void** a, const char* f, int l, int trace_alloc);

/**
 * Re-implementation of atof method (enables OS portability)
 * @param s			String to be formatted
 * @return			The float representation of the given string
 */
float my_atof(const char* s);

/**
 * Checks a filename validity
 * @param file_name		File name to be checked
 * @return				>=0 : success\n
 * 						-1	: empty name\n
 * 						-2  : invalid length\n
 * 						-3  : forbidden chars
 * @see					StringOps::STRING_OPS_FORBIDDEN_FILE for characters
 * 						declared as forbidden.
 */
int is_valid_filename(Glib::ustring file_name) ;

/**
 * Template for converting a string to a number representation
 * @param s		String to format
 * @return		The number representation of the given string
 */
template <class JNUMBER>
JNUMBER string_to_number(const Glib::ustring& s)
{
	JNUMBER res = 0 ;
	std::istringstream istr(s) ;
	istr >> res ;
	return res ;
}

/**
 * Template for converting a string to a number representation
 * @param s		String to format
 * @return		The number representation of the given string
 */
template <class JNUMBER>
JNUMBER string_to_number(const std::string& s)
{
	JNUMBER res = 0 ;
	std::istringstream istr(s) ;
	istr >> res ;
	return res ;
}

/**
 * Template for converting a number to its string representation
 * @param number	Number to format
 * @return			The string representation of the given number$
 * @warning			If you want to manipulate floats and keep correct precision, it is
 * 					highly recommended to use the method float_to_string(float,int).
 */
template <class NUMBER>
string number_to_string(NUMBER number)
{
	std::ostringstream ostr ;
	ostr << number ;
	return ostr.str() ;
}

/**
 * Gets the string representation of a float with "precision" numbers
 * after coma.
 * @param myFloat		A float
 * @param precision		Number of numbers after coma (6 by default, -1 if you don't mind)
 * @return				String representation of float
 */
std::string float_to_string(float myFloat, int precision=6) ;

/**
 * Checks whether the given string belongs to <em>list</em>
 * @param list		List of strings
 * @param key		String we wonder if it belongs to <em>list</em>
 * @return			True if it were found, false otherwise
 */
bool is_in_list(const std::list<string>& list, Glib::ustring key) ;

/**
 * Checks whether the given string belongs to <em>list</em>
 * @param vect		List of strings
 * @param value		String we wonder if it belongs to <em>list</em>
 * @return			True if it were found, false otherwise
 */
bool is_in_svect(const std::vector<Glib::ustring>& vect, Glib::ustring value) ;

/**
 * Checks whether the given string belongs to <em>list</em>
 * @param vect		List of strings
 * @param value		String we wonder if it belongs to <em>list</em>
 * @return			True if it were found, false otherwise
 */
bool is_in_svect(const std::vector<std::string>& vect, Glib::ustring value) ;

/**
 * For a given string, replaces a substring by another string
 * @param data				String
 * @param toBeReplaced		Substring of <em>data</em> we want to replace
 * @param replace			The string that will replace <em>toBeReplaced</em>
 * @return					The resulting string
 */
Glib::ustring replace_in_string(Glib::ustring data, Glib::ustring toBeReplaced, Glib::ustring replace) ;

/**
 * Print a string map
 * @param map		Map to display
 * @param display	String that will be printed just before the map
 * @remark			Used for debug
 */
void print_map_s(const std::map<string,string>& map, string display) ;

/**
 * Print a string/int map
 * @param map		Map to display
 * @param display	String that will be printed just before the map
 * @remark			Used for debug
 */
void print_map_s(const std::map<string,int>& map, string display) ;

/**
 * Print a ustring map
 * @param map		Map to display
 * @param display	String that will be printed just before the map
 * @remark			Used for debug
 */
void print_map_s(const std::map<Glib::ustring,Glib::ustring>& map, string display) ;

/**
 * Print a string vector
 * @param vect		Map to display
 * @param display	String that will be printed just before the map
 * @remark			Used for debug
 */
void print_vector_s(const std::vector<string>& vect, string display) ;

/**
 * Print a string vector
 * @param vect		Map to display
 * @param display	String that will be printed just before the map
 * @remark			Used for debug
 */
void print_vector_s(const std::vector<Glib::ustring>& vect, string display) ;

/**
 * Merges two map
 * @param map1		First map
 * @param map2		Second map
 * @return			The merged map
 * @attention		Basic method, doesn't check element existence for two elements with same
 * 					key (to be improved if needed)
 */
std::map<std::string, std::string> merge_map_s(const std::map<std::string, std::string>& map1,
													const std::map<std::string, std::string>& map2 ) ;

/**
 * Merges strings vector v2 into strings vector v1
 * @param[in,out]	v1		Vector
 * @param 			v2		Vector
 * @param unique			True for checking element unicity into vector resulting
 */
void merge_vector_s(std::vector<std::string>& v1, const std::vector<std::string>& v2, bool unique) ;

/**
 * Wrapper for converting a string map into DCSV format
 * @param map		Map with elements
 * @return			The given DCSV string
 * @note			Uses StringOps::makeDCSVStr(const std::map<std::string,std::string>&)
 */
string fromDCSVmapTOstring(const std::map<string,string>& map) ;

/**
 * Wrapper for converting a DCSV string to a map
 * @param str		DCSV string
 * @return			The given DCSV elements
 */
std::map<string,string> fromDCSVstringTOmap(std::string str) ;

/**
 * Removes the given prefix from the given string
 * @param s		String
 * @param sub	Prefix
 * @return		The resulting string
 */
std::string remove_prefix_s(std::string s, std::string sub) ;

/**
 * Computes the round of the given float
 * @param myfloat	Float number
 * @return			The resulting integer
 */
int my_roundf(double myfloat) ;

/**
 * Parses a string following the given separator, and places all elements into a
 * ustring vector.
 * @param separator		Seperator uses to determinates element borders
 * @param code			String to be parsed
 * @param vector		Pointer on vector used to receive all elements
 * @return				Number of elements
 */
int mini_parser(char separator, Glib::ustring code, std::vector<Glib::ustring>* vector );

/**
 * Parses a string following the given separator, and places all elements into a
 * string vector.
 * @param separator		Seperator uses to determinates element borders
 * @param code			String to be parsed
 * @param vector		Pointer on vector used to receive all elements
 * @return				Number of elements
 */
int mini_parser(char separator, Glib::ustring code, std::vector<string>* vector );

/**
 * Cut the given string s by finding the first or last word (word separators are " \n\t").
 * @param 		s				The string to cut
 * @param[out]  remaining		The remaining string after cut
 * @param 		first			True for getting the first word, false to get the last
 * @return						The first or the last word inside the string
 * @note						The string won't be trimmed, all spaces characters are kept
 * 								inside first/last string and remaining string.
 */
string getFirstLastWord(const string& s, string& remaining, bool first) ;

/***
 * Concat all elements of the given vector with the ";" separator.
 * @param vector		Elements to be concatened
 * @return				The concatened string
 */
std::string set2string(std::set<std::string> set) ;

/**
 * Reverse method of set2string method
 * @param s
 * @param set
 * @return
 */
int string2set(std::string s, std::set<std::string>* set) ;

/**
 * Computes the real regular path of the given file (resolves relative part and symlink)
 * @param path		Path to check
 * @return			The resulting path, or empty value if it couldn't be determined
 * @note			Uses FileInfo::::realpath()
 */
std::string real_regular_path(std::string path) ;

/**
 * @param s		string to be printed
 * @param arg	argument to be printed
 * @param mode	mode of trace
 */
template <class ARGUMENT>
static void print_trace(Glib::ustring s, ARGUMENT arg, int mode)
{
	if (mode==1) {
		Log::err() << s << " -> " << arg << std::endl ;
	}
	else if (mode==0) {
		Log::err() << s << " -> " << arg << std::endl ;
	}
}

/**
 * Same as sleep() methods but enable to set milliseconds values
 * @param time		Value in seconds
 */
void mySleep(float time) ;

/**
 * Delete the memory zone pointed by the pointed pointer and set
 * the pointer to NULL
 * @param pointer		Pointer on the pointer to be destroyed
 */
void deleteAndNull(void** pointer) ;

/**
 * Decodes a base64 encoded string
 * @param url	Base64 encoded string
 * @return		The corresponding decoded string
 */
Glib::ustring base64decode(Glib::ustring url) ;

/**
 * Encodes a string into base64 format
 * @param url	String to encode
 * @return		The corresponding encoded string
 */
Glib::ustring base64encode(Glib::ustring url) ;

/**
 *
 * @param str
 * @param toffset
 * @return
 */
int checkStringOffset(Glib::ustring str, int toffset) ;

#endif /* UTILS_H_ */
