// Paired.cc: implementation of Paired class
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/Paired.h>


Paired::Paired()
: Record(2)
{}


string
Paired::get_time0()
{
  return "0";
};


string
Paired::get_time()
{
  if (refread) {
    read_entry();
    labelread = false;
  }
  else
    refread = true;

  return get_ith(0);
}

string
Paired::get_label()
{
  if (labelread) {
    read_entry();
    refread = false;
  }
  else
    labelread = true;

  return get_ith(1);
}

bool
Paired::open(const string& filename)
{
  refread = labelread = true;
  return Record::open(filename);
}
