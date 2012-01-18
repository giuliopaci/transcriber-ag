// TIMITfile.h: TIMIT file class
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _TIMITFILE_H_
#define _TIMITFILE_H_

#include <ag/Paired.h>
#include <ag/RE.h>

/// TIMIT file model.
class TIMITfile : public Paired
{
private:
  RE *Tre;  // TIMIT annotation pattern
  virtual void
  read_entry();

  void init_re();

public:
  TIMITfile();
  TIMITfile(const string& filename);
  ~TIMITfile();
};

#endif
