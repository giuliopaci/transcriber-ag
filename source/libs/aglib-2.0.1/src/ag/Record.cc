// Record.cc: Record style file model implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/Record.h>

bool
Record::open(const string& filename)
{
  this->filename = filename;

  if (input.is_open()) {
    input.close();
    input.clear();
  }

  input.open(filename.c_str());

  if (input.good())
    preprocess();

  return input.good();
}


bool
Record::readline(string& line)
{
  if (getline(input, line))
    return true;
  else {
    line = "";
    return false;
  }
}


void
Record::put_ith(int i, const string& v)
{
  if (i < 0 || i >= size)
    throw RangeError("out of field access range");

  record_buf[i] = v;
}


string
Record::get_ith(int i)
{
  if (i < 0 || i >= size)
    throw RangeError("out of field access range");

  return record_buf[i]; 
}
