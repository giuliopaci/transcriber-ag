// SWBfile.h: class for SwitchBoard annotation file
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _SWBFILE_H_
#define _SWBFILE_H_

#include <string>
#include <fstream>
#include <ag/Record.h>
#include <ag/RE.h>

/// SwitchBoard file model.
class SWBfile : public Record
{
private:
  bool turnover;
  string speaker;
  string uttn;
  string start;
  string dur;
  string label;
  RE *Sre;

  virtual void
  read_entry();

  void
  init_re();

public:
  SWBfile();
  SWBfile(const string &filename);
  ~SWBfile();

  /// Read one record from the annotation file.
  bool read_record();
  /// Get the very first time of the annotation.
  string get_time0();
  /// Get the time of current annotation unit.
  string get_time();
  /// Get the label.
  string get_label();
  /// Get the speaker.
  string get_spkr();
  /// Get the utterence number.
  string get_uttn();
  /// Get the time of the next annotation unit.
  string get_next_time();
  /**
   * @return true
   *    If there was a turnover.
   * @return false
   *    If there was no turnover.
   */
  bool turn_over();
};

#endif
