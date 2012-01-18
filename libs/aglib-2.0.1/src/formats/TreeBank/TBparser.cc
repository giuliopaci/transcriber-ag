// TBparser.cc: TreeBank parser implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <fstream>
#if defined(__GNUC__) && __GNUC__ < 3
  #include "ag_sstream.h"
#else
  #include <sstream>
#endif
#include <ag/RE.h>
#include "TBparser.h"

void TBparser::parse(const string &filename, bool input_is_a_string)
{
  streambuf* buf;
  
  // dirty hack to load a string
  if (input_is_a_string) {
    static stringbuf sb;
    sb.str(filename);
    buf = &sb;
  }
  else {
    static filebuf fb;
    if (fb.is_open()) fb.close();
    if (!fb.open(filename.c_str(), ios::in))
      throw ParseError("can't open file: " + filename);
    buf = &fb;
  }

  istream input(buf);

  RE start("[ \t\n]*([^ \t\n)]*)[ \t\n]*([^ \t\n)]*)");
  RE end("[ \t\n]*\\)");
  RE blank("[ \t\n]*");
  string line;
  int i, count = 0;     // count = counter for open(not closed) parentheses.

  while (getline(input, line, '(')) {
    if (count++) {
      start.match(line);
      handler->start_paren(start.get_matched(1), start.get_matched(2));
      i = start.get_span(0)[1];
      while (end.match(line.substr(i))) {
	count--;
	handler->end_paren();
	i += end.get_span(0)[1];
      }
      if (!blank.match(line.substr(i)))
	throw ParseError("unrecognized data format");
    }
  }
}
