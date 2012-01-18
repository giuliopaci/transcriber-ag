// agfioError.c: agfio error class implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/agfioError.h>

agfioError::agfioError(const string& s)
: msg("agf:" + s)
{
  // print the error message at the moment of throwing an exception
  cerr << msg << endl;
}

const char*
agfioError::what() const
  throw()
{
  return msg.c_str();
}
