// TFfile.h: Table Format file model
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _TFFILE_H_
#define _TFFILE_H_

#include <ag/Record.h>

/// Table Format(TF) file model.
class TFfile : public Record
{
private:
  string separator;

protected:
  virtual void
  read_entry();
  
public:
  /**
   * @param n number of fields
   * @param separator field separator
   */
  TFfile(int n, const string& separator);

  /**  
   * @param n number of fields 
   * @param separator field separator 
   * @param filename input file
   */
  TFfile(int n, const string& separator, const string& filename);

  /// read a record from the current file.
  bool read_record();

};

#endif
