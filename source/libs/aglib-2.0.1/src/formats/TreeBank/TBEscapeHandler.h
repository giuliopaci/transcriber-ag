// TBEscapeHandler.h:
// Haejoong Lee
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _TBESCAPEHANDLER_H_
#define _TBESCAPEHANDLER_H_

#include <string>
#include <ag/agfioError.h>  // for DllExport

using namespace std;

/// 
class DllExport TBEscapeHandler {
private:
public:
  virtual string
  decode(const string& cat, const string& s)
  { return s; }

  virtual string
  encode(const string& cat, const string& s)
  { return s; }
};

#endif
