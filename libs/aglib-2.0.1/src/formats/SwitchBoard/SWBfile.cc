// SWBfile.cc: SwitchBoard file class implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <cstdio>
#include "SWBfile.h"
/* (( BT Patch -- */
#include "ag/AGAPI.h"
#define atof myatof
/* -- BT Patch )) */

SWBfile::SWBfile()
: Record(4), turnover(false)
{
  init_re();
}

SWBfile::SWBfile(const string& filename)
: Record(4), turnover(false)
{
  open(filename);
  init_re();
}

SWBfile::~SWBfile()
{
  delete Sre;
}


void
SWBfile::init_re()
{
  string s;
  // Switch Board file entry pattern
  // (1) Speaker      ---> get_field(2)
  // (2) Utterence #  ---> get_field(3)
  // (3) Word start   ---> get_field(0)
  // (4) Word duration
  // (5) Word         ---> get_field(1)
  s = ("[ \t]*([AB])\\.([0-9]+)[ \t]+([^ \t]+)[ \t]+");
  s += ("([^ \t]+)[ \t]+([^ \t]+)[ \t]*$");
  Sre = new RE(s);
}


static string
ftoa(double x)
{
//  stringstream ss;
//  ss << x;
//  return ss.str();
  char s[36];
  sprintf(s, "%.15e", x);
  return string(s);
}

string
SWBfile::get_time0()
{
  read_entry();
  put_ith(0, start);
  turnover = false;
  return start;
}
  
//
// surface entry format(interface):
//    END[0] LABEL[1] SPEAKER[2] UTTR#[3]
//
// physical entry format:
//    SPEAKER(0) UTTR#(1) START(2) DUR(3) LABEL(4)
//
void
SWBfile::read_entry()
{
  string line;

  while (readline(line))
  {
    if (Sre->match(line))
    {
      put_ith(1, label);
      put_ith(2, speaker);
      put_ith(3, uttn);
      label = Sre->get_matched(5);
      speaker = Sre->get_matched(1);

      if (get_ith(2) == speaker)
      {
	// in the middle of a turn,
	// the end point is the start of the next word
	start = Sre->get_matched(3);
	if (start.at(0) == '-')
	  start = "-9999.0";
	put_ith(0, start);
	turnover = false;
      }
      else
      {
	// at the end of a turn, the end point is caculated as the sum
	// of start of the word and the duration of the word
	put_ith(0, ftoa(atof(start.c_str()) + atof(dur.c_str())));
	start = Sre->get_matched(3);
	if (start.at(0) == '-')
	  start = "-9999.0";
	uttn = Sre->get_matched(2);
	turnover = true;
      }

      dur = Sre->get_matched(4);  // duration
      if (dur == "*")
	dur = "-99999.0";

      return;
    }
    else if (line != "")
    {
      cerr << "WARNING: unrecognized format" << endl
	   << "WARNING: " << get_filename() << endl
	   << "WARNING: (" << line << ")" << endl
	   << "WARNING: wrong annotation may be created" << endl;
    }
  }

  if (label != "")
  {
    put_ith(0, ftoa(atof(get_ith(0).c_str()) + atof(dur.c_str())));
    put_ith(1, label);
    put_ith(2, speaker);
    put_ith(3, uttn);
    label = "";
    start = "";
    turnover = true;
  }
  else
  {
    clear_record_buf();
  }
}


bool
SWBfile::read_record()
{
  read_entry();
  return (get_label() == "") ? false : true;
}

string
SWBfile::get_time()
{
  return get_ith(0);
}

string
SWBfile::get_label()
{
  return get_ith(1);
}

string
SWBfile::get_spkr()
{
  return get_ith(2);
}

string
SWBfile::get_uttn()
{
  return get_ith(3);
}

string
SWBfile::get_next_time()
{
  return start;
}

bool
SWBfile::turn_over()
{
  return turnover;
}
