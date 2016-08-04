// XLabelFile.cc: XLabel class implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include "XLabelFile.h"

XLabelFile::XLabelFile()
{
  init_re();
}

XLabelFile::XLabelFile(const string& filename)
{
  open(filename);
  init_re();
}

XLabelFile::~XLabelFile()
{
  delete xre, xre2;
}


void
XLabelFile::init_re()
{
  xre = new RE("([0-9.]+)[ \t]+[0-9]+[ \t]+([^;]*[^ \t;])[ \t]*$");
  xre2 = new RE("([0-9.]+)[ \t]+[0-9]+[ \t]*$");
}

void XLabelFile::preprocess()
{
  // remove header of the open file
  string line;
  while (readline(line) && line.at(0) != '#');
}

// to cope with "explicit first entry" which doesn't have label...
string XLabelFile::get_time0()
{
  read_entry();
  if (get_ith(1) == "") {
    string tmp = get_ith(0);
    read_entry();
    return tmp;
  }
  else
    return "0";
}
    
void XLabelFile::read_entry()
{
  string line;
  while (readline(line)) {
    if (xre->match(line)) {
      put_ith(0, xre->get_matched(1));
      put_ith(1, xre->get_matched(2));
      return;
    }
    else if (xre2->match(line)) {
      put_ith(0, xre2->get_matched(1));
      put_ith(1, "");
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
