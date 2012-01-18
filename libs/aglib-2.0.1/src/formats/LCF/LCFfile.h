// LCFfile.h: LDC Callhome Format file model
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _LCFFILE_H_
#define _LCFFILE_H_

#include <ag/RE.h>
#include <ag/Record.h>

/// LDC Callhome Format file model.
class LCFfile : public Record
{
private:
  RE* re1, *re2;
  string comment;
  string line;

  void
  init();

  virtual void
  preprocess();

  virtual void
  read_entry();

public:
  LCFfile();
  LCFfile(const string& filename);
  ~LCFfile();

  /**
   * Read one record from the input file.
   */
  bool
  read_record();

  /**
   * Get comment (file header) from the input file.
   */
  string
  get_comment();
};

#endif
