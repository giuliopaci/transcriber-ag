// BUpos.h: class definition for BU .pos file
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _BUPOS_H_
#define _BUPOS_H_

#include <ag/Paired.h>
#include <ag/RE.h>

/// BU corpus .pos file model.
class BUpos : public Paired
{
private:
  RE *posre;

  virtual void
  read_entry();

  void
  init_re();

public:
  BUpos();
  BUpos(const string &filename);
  ~BUpos();
};

#endif
