/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* @file StringOps.cpp
* @brief various string operations
*/

#include <stdlib.h>
#include <iterator>
#include <sstream>
#include <ctype.h>
#include <string.h>

#include "Common/util/StringOps.h"

#ifdef TEST_STRINGOPS
#define _(a) a
#else
#include "Common/globals.h"
#endif

using namespace std;

char STRING_OPS_FORBIDDEN_FILE[] = {';', ',', '+', '<', '>', '|', '\"', '[', ']', ' ', '\\', '<', '>', 0} ;
char STRING_OPS_FORBIDDEN_XML[] = {'&', '\"', 0} ;


static void to_lower(char& c) { c = tolower(c); }
static void to_upper(char& c) { c = toupper(c); }



/*
* count nb words in string
*/
int StringOps::getTokenCount(const char* delims)
{
	int cnt = 0;
	unsigned long pos = 0;
	while ( pos != string::npos ) {
	 	pos = _str.find_first_not_of(delims, pos);
		if ( pos != string::npos ) {
			++cnt;
			pos = _str.find_first_of(delims, pos);
		}
	}
	return cnt;
}


/*
* extract string token
* return true if token extracted, false if no more token
*/
bool StringOps::getToken(unsigned int& start, string& dst, const char* delims, bool skipempty)
{
	dst = "";
 	while ( start < _str.length() && skipempty && strchr(delims, _str[start]) != NULL ) ++start;
	if ( start < _str.length() ) {
		for (; start < _str.length() && strchr(delims, _str[start]) == NULL;
			++start )
			dst += _str[start];
		do { ++start; } while ( start < _str.length() && skipempty && strchr(delims, _str[start]) != NULL );
		return true;
	} else return false;
}


/*
* split string in tokens
*/
int StringOps::split(vector<string>& dst, const char* delims, bool skipempty)
{
	unsigned int start = 0;
	if ( ! _str.empty() )
	{
		dst.push_back("");
		while ( getToken(start, dst.back(), delims, skipempty ) )
			dst.push_back("");
		dst.resize(dst.size()-1);
	}
	return dst.size();
}


/**
 * concat string vector tokens in result string
 * @param tokens string tokens vector
 * @param delim token separator
 * @return concatenated string
 */
const string& StringOps::concat(const std::vector<std::string>& tokens, const string& delim)
{
	_str.clear();
	vector<string>::const_iterator it;
	for (it=tokens.begin(); it != tokens.end(); ++it ) {
		if ( !_str.empty() ) _str += delim;
		_str += *it;
	}
	return _str;
}

/*
* parse XML attributes string
*/
bool StringOps::parseXMLAttr(map<string, string>& items, bool lower_names)
{
	items.clear();
	string name(""), value("");
	bool ok = true;
	vector<string> v;
	vector<string>::iterator it;

	split(v, " ");
	for ( it = v.begin(); it != v.end(); ++it ) {
		unsigned int p = 0;
		StringOps ss(*it);
		if ( ss.getToken(p, name, "= \t\"") ) {
			ss.getToken(p, value, "\"/");
			if ( lower_names ) items[StringOps(name).toLower()] = value;
			else items[name] = value;
		} else ok=false;
	}
	return ok;
}

/*
* parse time specification string and return time in seconds
*/
float StringOps::parseTimeStr() throw (const char*)
{
	float t;
	bool okformat = true;
	if ( _str.find(':') != string::npos ) {
		int hh,mm,ss,dd;
		int cnt = sscanf(_str.c_str(), "%d:%d:%d.%d", &hh, &mm, &ss, &dd);
		if ( cnt < 2 ) {
			okformat = false ;
		} else {
			int div = 10;
			if ( cnt == 2 ) {
				ss=mm; mm=hh; hh=0;
			}
			if ( cnt < 4 ) dd=0;
			else {
				while (dd > div) div *= 10;
			}
			t = (hh*3600) + (mm*60) + ss + dd/div;
		}
	} else {
		char* pt;
		t = strtod(_str.c_str(), &pt);
		if ( *pt != 0 ) {
			okformat = false ;
		}
	}

	if ( okformat == false ) {
		throw _("Invalid time format : should be hh:mm:ss.ddd or ss.ddd") ;
	}
	return t;
}

/*
* check if string contains forbidden chars
*/
bool StringOps::check_string(std::string forbidden, std::string name)
{
	unsigned int cpt = 0 ;
	bool found = false ;
	while(cpt< forbidden.size() && !found)	{
		if ( name.find(forbidden[cpt]) != std::string::npos )
			found = true ;
		cpt++ ;
	}
	return !found ;
}

/**
*  set string to lowercase
*  @return modified string
*/
string& StringOps::toLower()
{
	for_each(_str.begin(), _str.end(), to_lower );
	return _str;
}

/**
*  set string to uppercase
*  @return modified string
*/
string& StringOps::toUpper()
{
	for_each(_str.begin(), _str.end(), to_upper );
	return _str;
}

/**
* trim space chars at begining and end of string
*  @return modified string
*/
string& StringOps::trim(const char* trimchars)
{
	string::iterator it;
	unsigned int pos = 0;

	if ( _str.empty() ) return _str;

	while ( pos < _str.length() && strchr(trimchars,_str[pos]) != NULL ) ++pos;
	if ( pos > 0 ) _str.erase(0,pos);
	if ( _str.length() > 0 ) {
		for ( pos = _str.length()-1; pos >= 0 && strchr(trimchars,_str[pos]) != NULL; --pos );
		++pos;
		if ( pos < _str.length() ) _str.erase(pos);
	}
	return _str;
}


/**
* check if string contains substring
* @param sub substring to find
* @param ignorecase true (default) / false; if true, assume sub is lowercased.
* @return true if match found
*/
bool StringOps::contains(const char* sub, bool ignorecase)
{
	if ( ignorecase) toLower();
	return (_str.find(sub) != std::string::npos);
}

/**
* check if string is equal to other string, not taking casing into account
* @param sub string to compare with
* @return true if string matches
*/
bool StringOps::equalsIgnoreCase(const char* sub)
{
	toLower();
	string cmp = sub;
	return (_str.compare(StringOps(cmp).toLower()) ==0 );
}

/**
 * replace a substring by given replacement string
 * @param sub substring to find and replace
 * @param repl	replacement string
 * @param do_all if true replace all occurrences in string, else replace only first occurrence
 * @return result string
 */

std::string& StringOps::replace(const std::string& sub, const std::string& repl, bool do_all)
{
	unsigned long pos;
	int len = sub.length();
	bool done = false;
	while ( !done && (pos = _str.find(sub)) != string::npos ) {
		_str.replace(pos, len, repl);
		if ( !do_all ) done = true;
	}
	return _str;
}

/**
 * replace a substring by given replacement string
 * @param sub substring to find and replace
 * @param repl	replacement string
 * @param do_all if true replace all occurrences in string, else replace only first occurrence
 * @return result string
 */

std::string& StringOps::replaceAnyOf(const std::string& sub, const std::string& repl, bool do_all)
{
	unsigned long pos;
	int len = 1;
	bool done = false;
	while ( !done && (pos = _str.find_first_of(sub)) != string::npos ) {
		_str.replace(pos, len, repl);
		if ( !do_all ) done = true;
	}
	return _str;
}

/**
*  get int value from string buffer
* @return int value
*/
int StringOps::toInt()
{
		return strtol(_str.c_str(), NULL, 10);
}

/**
*  set int value in string buffer
* @return string buffer
*/
string& StringOps::fromInt(int val, const char* fmt)
{
	char bufmt[20];
	sprintf(bufmt, fmt, val);
	_str = bufmt;
	return _str;
}


/**
*  get float value from string buffer
* @return float value
*/
float StringOps::toFloat()
{
	return strtod(_str.c_str(), NULL);
}

/**
*  set float value in string buffer
* @return string buffer
*/
string& StringOps::fromFloat(float val, const char* fmt)
{
	char bufmt[20];
	sprintf(bufmt, fmt, val);
	_str = bufmt;
	return _str;
}

/*
string& StringOps::addFloat(float val, const char* fmt)
{
	char bufmt[20];
	sprintf(bufmt, fmt, val);
	_str += bufmt;
	return _str;
}
*/

/*
* useful operators
*/

string& operator += (string& s,int i) {
        char buf[20];
        sprintf(buf, "%d", i);
        s += buf;
        return s;
}

string& operator + (string& s,int i) {
        char buf[20];
        sprintf(buf, "%d", i);
        s += buf;
        return s;
}

string& operator += (string& s, double d) {
        char buf[20];
        sprintf(buf, "%g", d);
        s += buf;
        return s;
}

string& operator + (string& s, double d) {
        char buf[20];
        sprintf(buf, "%g", d);
        s += buf;
        return s;
}

/*========================================================================
*
*  deal with DCSV-formatted values
*
*========================================================================*/

string StringOps::makeDCSVStr(const map<string, string>& items)
{
	map<string, string>::const_iterator it;
	ostringstream os;
	for ( it = items.begin(); it!=items.end(); ++it )
			os << it->first << "=" << it->second << "; ";
	_str = os.str();
	return _str;
}

bool StringOps::getDCSVItems(map<string, string>& items)
{
	items.clear();
	string name(""), value("");
	bool ok = true;
	vector<string> v;
	vector<string>::iterator it;

	split(v, ";");
	for ( it = v.begin(); it != v.end(); ++it ) {
		unsigned int p = 0;
		StringOps ss(*it);
		if ( ss.getToken(p, name, "= \t") ) {
			ss.getToken(p, value, ";");
			items[name] = value;
		} else ok=false;
	}
	return ok;
}

/*-------------------- TEST BLOCK  ----------------------------------------*/

#ifdef TEST_STRINGOPS

#include <iostream>
#include <iterator>
#include "Common/util/StringOps.h"
using namespace std;

void  test_const_str( const string& s)
{
	StringOps g(s);
	cout << "recu = " << g.toFloat() << endl;
	g.fromFloat(35.1);
	cout << "modifié = " << g.toFloat() << endl;
}

int main()
{
	string mytest="1";
	cout << "init mytest=" << mytest << endl;
	StringOps ops(mytest);
	ops.fromFloat(12.5);
	cout << "before test_const_str mytest=" << mytest << endl;
	test_const_str(mytest);
	cout << "after test_const_str mytest=" << mytest << endl;
	return 0;
}

#endif /* TEST_STRINGOPS */
