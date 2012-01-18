// TFfile.cc: Table Format file model implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include "TFfile.h"

TFfile::TFfile(int n, const string& separator)
  : Record(n), separator(separator)
{}

TFfile::TFfile(int n, const string& separator, const string& filename)
  : Record(n), separator(separator)
{
  open(filename);
}

void
TFfile::read_entry()
{
  string line;

  // read a line from input file
  line = "";
  while (line == "") {
    // hit the EOF
    if (!readline(line)) {
      clear_record_buf();
      return;
    }
  }

  int m = 0;                    // field index
  int i = 0;                    // start index of the field in the string
  int j = line.find(separator); // end index of the field in the string
  while (j != string::npos && m < get_size()-1) {
    put_ith(m++, line.substr(i, j-i));
    i = j + separator.length();
    j = line.find(separator, i);
  }
  put_ith(m, line.substr(i));
}

bool
TFfile::read_record()
{
  read_entry();
  return (get_ith(0) == "") ? false : true;
}
