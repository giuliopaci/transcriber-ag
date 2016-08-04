// TBparser.h: TreeBank file parser class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _TBPARSER_H_
#define _TBPARSER_H_

#include <ag/agfioError.h>
#include "TBhandler.h"

/// TreeBank parser class.
class TBparser
{
private:
  TBhandler* handler;
  
public:
  /// Parser error.
  class ParseError: public agfioError
  {
  public:
    ParseError(const string& s): agfioError("TBparser:" + s) {}
  };

  /**
   * @param h
   *    TreeBank parentheses handler.
   */
  TBparser(TBhandler& h): handler(&h) {}
  /// Set the TreeBank parentheses handler for the parser. 
  void set_handler(TBhandler& h) { handler = &h; }
  /// parser method
  void parse(const string& filename, bool input_is_a_string=false);

};

#endif
