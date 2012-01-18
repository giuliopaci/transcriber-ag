// TIMITfile.cc: TIMITfile class implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include "TIMITfile.h"


void
TIMITfile::init_re()
{
  Tre = new RE("[0-9]+[ \t]+([0-9]+)[ \t]+(.*[^ \t])[ \t]*$");
}

TIMITfile::TIMITfile()
{
  init_re();
}

TIMITfile::TIMITfile(const string& filename)
{
  open(filename);
  init_re();
}

TIMITfile::~TIMITfile()
{
  delete Tre;
}


void
TIMITfile::read_entry()
{
  string line;

  while (readline(line)) {
    if (Tre->match(line)) {
      put_ith(0, Tre->get_matched(1));
      put_ith(1, Tre->get_matched(2));
      return;
    }
    else if (line != "") {
      cerr << "WARNING: unrecognized format" << endl
	   << "WARNING: " << get_filename() << endl
	   << "WARNING: (" << line << ")" << endl
	   << "WARNING: wrong annotation may be created" << endl;
    }
  }
  
  clear_record_buf();
}
