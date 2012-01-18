// BASfile.h: BAS Partitur format file model.
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _BASFILE_H_
#define _BASFILE_H_

#include <string>
#include <fstream>
#include <map>
#include <ag/RE.h>
#include <ag/Record.h>

/// BAS Partitur file model.
class BASfile : public Record
{
private:
  RE* tierRE[6];
  RE* tierREsub;
  map<string,int> classMap;

  bool tier_process1(const string& tier);
  bool tier_process4(const string& tier);
  bool tier_process5(const string& tier);
  bool (BASfile::*tier_process[6])(const string&);

  void
  init();

  virtual void
  preprocess();

  virtual void
  read_entry();

public:
  BASfile();
  BASfile(const string& filename);
  ~BASfile();

  /// Read one record from the file.
  bool read_record();

  /// Get the type of the tier.
  string get_type();
  /// Get the start time of the unit.
  string get_time();
  /// Get the duration of the unit.
  string get_dur();
  /// Get simbolic link list of the unit.
  string get_ref();
  /// Get the label of the unit.
  string get_label();
};

#endif
