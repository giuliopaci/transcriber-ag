// TB_UPenn_WSJ_EscapeHandler.cc:
// Haejoong Lee
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include "TB_UPenn_WSJ_EscapeHandler.h"
#include <iostream>

const string TB_UPenn_WSJ_EscapeHandler::
escaped[] = {
  "-LRB-",
  "-LCB-",
  "-LSB-",
  "-RRB-",
  "-RCB-",
  "-RSB-"
};

const string TB_UPenn_WSJ_EscapeHandler::
surface[] = {
  "(",
  "{",
  "[",
  ")",
  "}",
  "]"
};

const int TB_UPenn_WSJ_EscapeHandler::
NITEM = 6;

string TB_UPenn_WSJ_EscapeHandler::
decode(const string& tag, const string& s)
{
  int m, n;
  string ss = s;

  for (int i=0; i < NITEM; i++) {
    m = ss.find(escaped[i]);
    while (m != string::npos) {
      n = m + escaped[i].length();
      ss.replace(m, n, surface[i]);
      n = m + surface[i].length();
      m = ss.find(escaped[i], n);
    }
  }
  
  return ss;
}

string TB_UPenn_WSJ_EscapeHandler::
encode(const string& tag, const string& s)
{
  int m, n;
  string ss = s;

  for (int i=0; i < NITEM; i++) {
    m = ss.find(surface[i]);
    while (m != string::npos) {
      n = m + surface[i].length();
      ss.replace(m, n, escaped[i]);
      n = m + escaped[i].length();
      m = ss.find(surface[i], n);
    }
  }
  
  return ss;
}
