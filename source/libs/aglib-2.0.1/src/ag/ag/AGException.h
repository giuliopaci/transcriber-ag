// AGException: Class definition of AGException
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef AGException_h
#define AGException_h

#include <string>

using namespace std;

/**
 * AGException is a class for possible exceptions in AG Library.
 * @author Xiaoyi Ma
 * @note needs to be refined. Derived classes may need to be defined
 * so that more detailed information can be provided.
 **/
class AGException{
 private:
  string message;

 public:
  /// A constructor.
  AGException(const string& message) { this->message = message; }
  /// Get the error message.
  string error() { return message; }
};

#endif
