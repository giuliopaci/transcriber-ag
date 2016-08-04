// BUpos.cc: BUpos class implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include "BUpos.h"


BUpos::BUpos()
: Paired()
{
  init_re();
}


BUpos::BUpos(const string& filename)
: Paired()
{
  open(filename);
  init_re();
}


BUpos::~BUpos()
{
  delete posre;
}


void
BUpos::init_re()
{
  posre = new RE("([^ \t]+)[ \t]+([^ \t]+)[ \t]*$");
}


void BUpos::read_entry()
{
  string line;
  while (readline(line)) {
    if (posre->match(line)) {
      put_ith(0, posre->get_matched(1));
      put_ith(1, posre->get_matched(2));
      return;
    }
    else if (line != "") {
      cerr << "WARNING: unrecognized format in " << get_filename()
	   << ": (" + line + ")" << endl;
      cerr << "WARNING: wrong annotation may be created" << endl;
    }
  }

  clear_record_buf();
}
