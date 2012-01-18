// Paired.h: abstract class for (reference, label) style annotation files
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _PAIRED_H_
#define _PAIRED_H_

#include <string>
#include <fstream>
#include <ag/Record.h>

/// (time, label) pair file model.
class DllExport Paired : public Record
{
private:
  bool refread, labelread;
  // refread, labelread: indicate if ref (or label) is read

public:
  Paired();

  /// Get the first reference.
  /**
   * Some annotations explicitly annotate the first reference, but some don't.
   * This is a solution to normalize such variations.
   */
  virtual string
  get_time0();

  /// Get the start time of the annotation unit.
  string get_time();

  /// Get the label of the annotation unit.
  string get_label();

  bool open(const string& filename);

};

#endif
