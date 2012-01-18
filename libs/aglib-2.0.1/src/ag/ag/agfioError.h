// agfioError.h: agfio error class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _AGFIOERROR_H_
#define _AGFIOERROR_H_

#include <iostream>
#include <string>
#include <exception>

#ifdef _MSC_VER
#define DllExport __declspec( dllexport )
#else
#define DllExport
#endif

using namespace std;

/// agfio error class.
/**
 * Almost every exception class of the library inherits from this class.
 * There is no special advantage to inherit this class except that consitency
 * can be kept in a convenient way.
 */
class DllExport agfioError : public exception
{
private:
  string msg;

public:
  /**
   * @param s
   *    Error message.
   */
  agfioError(const string& s);

  ~agfioError() throw() {}

  /**
   * @return
   *    C string containing error message
   */
  virtual const char*
  what() const throw();
};

#endif
