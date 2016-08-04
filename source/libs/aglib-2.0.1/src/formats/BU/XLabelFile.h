// XLabelFile.h: class definition for XLabel files
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _XLABELFILE_H_
#define _XLABELFILE_H_

#include <ag/Paired.h>
#include <ag/RE.h>

/// XLabel annotation file model.
class XLabelFile : public Paired
{
private:
  RE *xre, *xre2;
  // xre2: RE pattern for annotations which don't have labels

  virtual void preprocess();
  virtual void read_entry();

  void init_re();

public:
  XLabelFile();
  XLabelFile(const string&filename);
  ~XLabelFile();

  /// get the first time of the annotation file.
  virtual string
  get_time0();
};

#endif
