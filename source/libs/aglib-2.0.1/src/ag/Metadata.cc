// Metadata.cc: Metadata associated with AGSet, AG, Timeline and Signal
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/Metadata.h>

void Metadata::setFeature(FeatureName f, FeatureValue v) {
  (*this)[f] = v;
}

bool Metadata::existsFeature(FeatureName f) {
  return find(f) != end();
}

void Metadata::deleteFeature(FeatureName f) {
  erase(f);
}

set<string>
Metadata::getFeatureNames()
{
  set<string> fnames;
  Metadata::iterator pos;

  for (pos = begin(); pos != end(); ++pos)
    fnames.insert(pos->first);

  return fnames;
}

list<string> Metadata::storeSQLs(AGSetId agSetId, AGId agId, Id id) {
  list<string> sqls;

#ifdef _MSC_VER
  {
#endif
  for (Metadata::const_iterator pos = begin(); pos != end(); ++pos)
    sqls.push_back("insert into METADATA (AGSETID, AGID, ID, NAME, VALUE) values ('"
		   + agSetId + "','" + agId + "','" + id + "','" + pos->first + "','"
		   + pos->second + "')");
#ifdef _MSC_VER
  }
#endif
  
  return sqls;
}

string Metadata::toString() {
  string outString;

  outString += "<Metadata>\n";
  
#ifdef _MSC_VER
  {
#endif
  for (Metadata::const_iterator pos = begin(); pos != end(); ++pos) {
    outString += "<MetadataElement name=\"" + pos->first + "\">";
    outString += pos->second;
    outString += "</MetadataElement>\n";
  }
#ifdef _MSC_VER
  }
#endif

  outString += "</Metadata>";

  return outString;
}
