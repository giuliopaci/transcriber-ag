// Feature.cc: Implementation of Feature class. 
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

#include <ag/Utilities.h>
#include <ag/FeatureMap.h>

FeatureValue FeatureMap::getFeature(FeatureName name) const throw (AGException){ 
  FeatureMap::const_iterator pos = find(name);

  if(pos == end())
    throw AGException("Feature " + name + " doesn't exist!");
  else
    return pos->second;
}

void FeatureMap::unsetFeatures() {
  for (FeatureMap::iterator pos = begin(); pos != end(); ++pos)
    pos->second = "";
}

string FeatureMap::toString() {
  string outString;

  for (FeatureMap::const_iterator pos = begin(); pos != end(); ++pos) {
    string name = Utilities::entityReferences(pos->first);
    string value = Utilities::entityReferences(pos->second);
    
    outString += "<Feature name=\"" + name + "\">";
    outString += value + "</Feature>\n";
  }
  return outString;
}
