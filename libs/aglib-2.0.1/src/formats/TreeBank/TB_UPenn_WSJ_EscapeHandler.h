// TB_UPenn_WSJ_EscapeHandler.h:
// Haejoong Lee
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _TB_UPENN_WSJ_ESCAPEHANDLER_H_
#define _TB_UPENN_WSJ_ESCAPEHANDLER_H_

#include "TBEscapeHandler.h"

class DllExport TB_UPenn_WSJ_EscapeHandler
:public TBEscapeHandler {
  
private:
  static const string escaped[];
  static const string surface[];
  static const int NITEM;

public:
  virtual string
  decode(const string& tag, const string& s);

  virtual string
  encode(const string& tag, const string& e);
};

#endif
