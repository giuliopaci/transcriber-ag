// BASfile.cc: BAS file modle implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include "BASfile.h"

#define TIME  1
#define DUR   2
#define REF   3
#define LABEL 4


void
BASfile::init()
{
  tierRE[0] = new RE("([A-Z0-9]{3}):[ \t]*(.+)");
  tierRE[1] = new RE("([-0-9]+([ \t]*,[0-9]+)*)[ \t]+(.+)");
  tierRE[4] = new RE("([0-9]+)[ \t]+([0-9]+)[ \t]+([-0-9]+)[ \t]+(.+)");
  tierRE[5] = new RE("([0-9]+)[ \t]+([-0-9]+)[ \t]+(.+)");

  classMap["KAN"] = 1;
  classMap["ORT"] = 1;
  classMap["TRL"] = 1;
  classMap["TR2"] = 1;
  classMap["SUP"] = 1;
  classMap["PHO"] = 4;
  classMap["SAP"] = 4;
  classMap["MAU"] = 4;
  classMap["WOR"] = 4;
  classMap["DAS"] = 1;
  classMap["PRB"] = 5;
  classMap["PRS"] = 1;
  classMap["NOI"] = 1;

  tier_process[1] = &BASfile::tier_process1;
  tier_process[4] = &BASfile::tier_process4;
  tier_process[5] = &BASfile::tier_process5;

}


BASfile::BASfile()
: Record(5)
{
  init();
}


BASfile::BASfile(const string& filename)
: Record(5)
{
  open(filename);
  init();
}


BASfile::~BASfile()
{
  delete tierRE[0];
  delete tierRE[1];
  delete tierRE[4];
  delete tierRE[5];
}


bool
BASfile::tier_process1(const string& tier)
{
  if (!tierRE[1]->match(tier))
    return false;

  put_ith(TIME, "");
  put_ith(DUR, "");
  put_ith(REF, tierRE[1]->get_matched(1));
  put_ith(LABEL, tierRE[1]->get_matched(3));
  return true;
}


bool
BASfile::tier_process4(const string& tier)
{
  if (!tierRE[4]->match(tier))
    return false;

  put_ith(TIME, tierRE[4]->get_matched(1));
  put_ith(DUR, tierRE[4]->get_matched(2));
  put_ith(REF, tierRE[4]->get_matched(3));
  put_ith(LABEL, tierRE[4]->get_matched(4));
  return true;
}


bool
BASfile::tier_process5(const string& tier)
{
  if (!tierRE[5]->match(tier))
    return false;

  put_ith(TIME, tierRE[5]->get_matched(1));
  put_ith(DUR, "");
  put_ith(REF, tierRE[5]->get_matched(2));
  put_ith(LABEL, tierRE[5]->get_matched(3));
  return true;
}


void
BASfile::preprocess()
{
  string line;
  while (readline(line) && line.substr(0,3) != "LBD");
}

  
void
BASfile::read_entry()
{
  string line;

  while (readline(line))
  {
    // readline will return a line without leading and ending white spaces
    if (line == "")
      continue;

    if (! tierRE[0]->match(line))
    {
      cerr << "WARNING: unrecognized data format: " << get_filename() << endl
	   << "WARNING: skipping this anchor and annotation..." << endl
	   << "WARNING: wrong annotation would be created" << endl;
      continue;
    }

    put_ith(0, tierRE[0]->get_matched(1));
    int classN = classMap[tierRE[0]->get_matched(1)];
    if (!(this->*tier_process[classN])(tierRE[0]->get_matched(2)))
    {
      cerr << "WARNING: unrecognized data format: " << get_filename() << endl
	   << "WARNING: skipping this anchor and annotation..." << endl
	   << "WARNING: wrong annotation would be created" << endl;
      continue;
    }

    return;
  }

  clear_record_buf();
} 
    

bool
BASfile::read_record()
{
  read_entry();
  return (get_type() == "") ? false : true;
}


string
BASfile::get_type()
{
  return get_ith(0);
}

string
BASfile::get_time()
{
  return get_ith(TIME);
}

string
BASfile::get_dur()
{
  return get_ith(DUR);
}

string
BASfile::get_ref()
{
  return get_ith(REF);
}

string
BASfile::get_label()
{
  return get_ith(LABEL);
}
