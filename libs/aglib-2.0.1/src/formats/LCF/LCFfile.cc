// LCFfile.cc: LDC Callhome Format file model implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include "LCFfile.h"

void
LCFfile::init()
{
  string pat1, pat2;

  // with speaker
  pat1 =  "^[ \t]*";        // initial white spaces
  pat1 += "([0-9.]+)";      // (1) start time
  pat1 += "[ \t]+";         // spaces
  pat1 += "([0-9.]+)";      // (2) end time
  pat1 += "[ \t]+";         // spaces
  pat1 += "([^ \t:]+):";    // (3) speaker
  pat1 += "[ \t]+";         // spaces
  pat1 += "(.*)$";          // (4) utterance

  // with no speaker
  pat2 =  "^[ \t]*";        // initial white spaces
  pat2 += "([0-9.]+)";      // (1) start time
  pat2 += "[ \t]+";         // spaces
  pat2 += "([0-9.]+)";      // (2) end time
  pat2 += "[ \t]+";         // spaces
  pat2 += "(.*)$";          // (3) utterance

  re1 = new RE(pat1);
  re2 = new RE(pat2);
}

LCFfile::LCFfile()
  : Record(4)
{}

LCFfile::LCFfile(const string& filename)
  : Record(4)
{
  open(filename);
  init();
}

LCFfile::~LCFfile()
{
  delete re1, re2;
}

void
LCFfile::preprocess()
{
  while (readline(line))
    if (line.empty())
      continue;
    else if (line.at(0) == '#')
      // CAREFUL: line.at() causes an error if the line is empty
      comment += line + "\n";
    else
      break;
}

void
LCFfile::read_entry()
{
  if (line == "") {
    clear_record_buf();
    return;
  }

  if (re1->match(line)) {
    put_ith(0, re1->get_matched(1));
    put_ith(1, re1->get_matched(2));
    put_ith(2, re1->get_matched(3));
    put_ith(3, re1->get_matched(4));
  }
  else if (re2->match(line)) {
    put_ith(0, re2->get_matched(1));
    put_ith(1, re2->get_matched(2));
    put_ith(3, re2->get_matched(3));
  }
  else if (line.at(0) != '#') {
    // if it is a comment line, just skip it.
    // otherwise, there's something wrong in the format file.
    // but don't stop processing.
    // say something, skip the line, and keep going.
    cerr << "LCFfile: unrecognized entry format" << endl
	 << "LCFfile: skip the line..." << endl;
  }

  while (readline(line) && line == "");
}
  
bool
LCFfile::read_record()
{
  read_entry();
  return (get_ith(0) == "") ? false : true;
}

string
LCFfile::get_comment()
{
  return comment;
}
