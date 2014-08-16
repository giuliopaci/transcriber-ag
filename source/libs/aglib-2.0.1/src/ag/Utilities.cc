// Utilities.cc: some commonly used functions, mostly operations 
// on strings.
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/Utilities.h>

string Utilities::trim(const string& s) {
  string::size_type idx = s.find_last_not_of(" ");
  if (idx == std::string::npos) // if all chars are space
    return "";                  // return empty string
  else                          // else
    return s.substr(0,idx+1);
}

string Utilities::entityReferences(const string& s) {
    int length;
    char c;
    string news;
    
    if (s.find_first_of("&<>\"") != std::string::npos) {
      length = s.length();
      for (int i = 0; i < length; i++) {
	c = s[i];
	if (c == '&')
	  news += "&amp;";
	else if (c == '<')
	  news += "&lt;";
	else if (c == '>')
	  news += "&gt;";
	else if (c == '"')
	  news += "&quot;";
	else
	  news += c;
      }
      return news;
    } else
      return s;
}

vector<string> Utilities::splitString(const string& s, char c) {
  string piece;
  bool escaped = false;
  vector<string> pieces;

  // split by unescaped semi-colons and put strings in pieces
  for (int i = 0; i <= s.length(); i++) {

    // if parsing is done on the whole string,
    // put piece into pieces if it's not empty
    if (i == s.length()) {
      if (!piece.empty())
	pieces.push_back(piece);
      break;
    }

    char cc = s[i];

    if (cc == '\\') {    // if it's an escape
      if (escaped) {
	piece += "\\\\";
	escaped = false;
      } else
	escaped = true;
    } else if (cc == c) {      // if it's the split char
      if (escaped) {
	piece += cc;
	escaped = false;
      } else {
	if (!piece.empty())
	  pieces.push_back(piece);
	piece.erase();
	escaped = false;
      }
    } else { // otherwise
      if (escaped){
	piece += '\\';
	piece += cc;
      }
      else
	piece += cc;
      escaped = false;
    }
  }

  return pieces;
}

string Utilities::escapeChar(const string& s, char c) {
  int length;
  string news;

  if (s.find_first_of(c) != std::string::npos) {
    length = s.length();
    for (int i = 0; i < length; i++) {
      if (s[i] == c) {
	news += '\\';
	news += c;
     } else
	news += s[i];
    }
    return news;
  } else
    return s;
}

string
Utilities::next_tok(string& str)
{
  int i, j;
  string s;

  i = str.find_first_not_of(" ");
  if (i == string::npos)
    return "";

  j = str.find_first_of(" ", i);
  if (j == string::npos) {
    s = str.substr(i);
    str = "";
  }
  else {
    s = str.substr(i, j-i);
    str = str.substr(j);
  }
  return s;
}

void
Utilities::string2set(string str, set<string>& s)
{
  string tok;
  while ((tok=next_tok(str)) != "")
    s.insert(tok);
}

