// BU.h: BU corpus loader class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _BU_H_
#define _BU_H_

#include <ag/agfio_plugin.h>

/// BU corpus loader class.
/**
 * Currently supported annotation files are .lba, .lbl, .wrd, .brk, .pos
 * .ton, and .msc. Note that only one of .lba and .lbl is loaded. User
 * can choose which format to load by assigning proper value ('lba' or 'lbl')
 * to 'base' feature of options argument.
 */
class DllExport BU: public agfio_plugin
{

private:

  enum { LBA=0, LBL, WRD, BRK, POS, TON, MSC };
  bool exist[MSC+1];  // indicate if each annotation file exists
  bool lbl_option;    // based on .lbl or .lba ?
  
  // Check existence of each file.
  void
  check_files(const string& prefix);

  // Process .wrd, .brk, .pos files to add annotations.
  void
  build_wbp(Paired& wrd, Paired& brk, Paired& pos, const AGId& agId);

  virtual list<AGId>
  load(const string& filename,
       const Id& id = "",
       map<string,string>* signalInfo = NULL,
       map<string,string>* options = NULL)
    throw (agfio::LoadError);


public:

  BU(): lbl_option(false) {}

};

AGFIO_PLUGIN(BU);

#endif
