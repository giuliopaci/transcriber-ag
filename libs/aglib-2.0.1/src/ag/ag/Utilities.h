// Utilities.h: some commonly used functions.
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef Utilities_h
#define Utilities_h

#include <string>
#include <vector>
#include <set>

using namespace std;

#ifdef _MSC_VER
#define DllExport __declspec( dllexport )
#else
#define DllExport
#endif

/** 
 * Utilities class defines some commonly used functions.
 * Utilities class defines some commonly used functions, such
 * as trim, escapeChar, etc. All the functions are static.
 * @author Xiaoyi Ma
*/
class DllExport Utilities {
 public:
  /**
   * entityReferences converts the special characters in a string
   * to their entity references so that the string can be a legal
   * HTML or XML string.
   * Certain characters, such as the left bracket (<), ampersand (&), 
   * etc. are reserved by HTML, SGML and XML to represent special
   * attributes such as the start of HTML elements, graphic characters,
   * and so on.
   *
   * HTML allows special referencing to represent these special 
   * characters. These are indicated by either character references or
   * entity references.
   *
   * Entity references use symbolic names to represent the characters. 
   * Entity references have three parts:
   * <pre>
   *   1. a leading ampersand character, (&), 
   *   2. the name of the entity (in ascii characters) 
   *   3. a terminating semicolon (;). 
   * </pre>
   * Thus the entity reference for less than symbol (<) is &lt;.
   * @see http://www.w3c.org for details.
   **/
   static string entityReferences(const string& s);
  /// remove the trailing spaces from string s if there are some.
  static string trim(const string& s);
  /// split a string by unescaped char.
  static vector<string> splitString(const string& s, char c);
  /// escape all char c in string s.
  static string escapeChar(const string& s, char c);

  static string next_tok(string& str);
  static void string2set(string str, set<string>& s);

};

#endif
