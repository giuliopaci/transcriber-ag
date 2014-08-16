// Anchor.cc: An anchor is a named offset into signal data
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

#include <cstdio>

#include <ag/Anchor.h>


Anchor::Anchor(Id id, Offset offset, Unit unit, set<string> const &signals)
  : id(id), offset(offset), unit(unit), signals(signals)
{
  anchored = true;
}

Anchor::Anchor(Id id, Unit unit, set<string> const &signals)
  : id(id), offset(0.0), unit(unit), signals(signals)
{
  anchored = false;
}


// "copy" constructor, but with the given id.
Anchor::Anchor(Id id, const Anchor* a)
  : id(id),
    offset(a->offset),
    unit(a->unit),
    signals(a->signals),
    anchored(a->anchored)
{}

bool operator < (const Anchor& a, const Anchor& b) {
  if (a.anchored && b.anchored && a.offset != b.offset)
    return a.offset < b.offset;
  else if (a.anchored && b.anchored && a.offset == b.offset)
    return a.id < b.id;
  else if (a.anchored && !b.anchored)
    return true;
  else if (!a.anchored && b.anchored)
    return false;
  else
    return a.id < b.id;
}

string Anchor::toString() {
  char dstring[20];
  string outString, sids;

  outString += "<Anchor id=\"" + id + "\"";

  if (anchored) {
    sprintf(dstring,"%f",offset);
    outString += " offset=\"" + string(dstring) + "\"";
  }

  if (unit != "")
    outString += " unit=\"" + unit + "\"";
  
  if (!signals.empty()) {
    outString += " signals=\"";
    set<string>::iterator pos = signals.begin();
    outString += *pos;
    for (++pos; pos != signals.end(); ++pos)
      outString += " " + *pos;
    outString += "\"";
  }

  outString += "></Anchor>";

  return(outString);
}

