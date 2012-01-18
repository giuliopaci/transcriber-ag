// Annotation.cc: An annotation associates symbolic information to a
// region of signal
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

#include <ag/Utilities.h>
#include <ag/Annotation.h>
#include <cstdio>
        
// ARC DEFINITIONS

// Constructors

Annotation::Annotation(Id id, const Annotation* a)
  : id(id),
    type(a->type),
    start(a->start),
    end(a->end),
    featureMap(a->featureMap)
{}

void Annotation::setFeature(const FeatureName& feature, const FeatureValue& value){
    featureMap[feature] = value;
}

// return the value of the specified feature, or empty string if absent
FeatureValue Annotation::getFeature(const FeatureName& feature) {
  return featureMap.getFeature(feature);
}

String Annotation::getAnnotationInfo() {
  char sStart[32];
  char sEnd[32];
  sprintf(sStart, "%f", start->getOffset());
  sprintf(sEnd, "%f", end->getOffset());
  string fstr, f, v;
  for (FeatureMap::iterator pos = featureMap.begin();
       pos != featureMap.end();
       ++pos) {
    // escape `=' and `;' in the feature name
    f = pos->first;
    f = Utilities::escapeChar(f,'=');
    f = Utilities::escapeChar(f,';');

    // escape `=' and `;' in the feature value
    v = pos->second;
    v = Utilities::escapeChar(v,'=');
    v = Utilities::escapeChar(v,';');

    // If the value is empty, just follow the feature name
    // with semicolon. Otherwise, f=v;
    if (v.empty())
      fstr += f + ";";
    else
      fstr += f + "=" + v + ";";
  }
  return Utilities::trim(type + " "
			 + sStart + " "
			 + sEnd + " "
			 + start->getId() + " "
			 + end->getId() + " "
			 + fstr);
}

// return a C string of the attribute value
const char* Annotation::getFeatureC(const FeatureName& feature)  {
  return getFeature(feature).c_str();
}

bool Annotation::existsFeature(FeatureName feature) { 
  return featureMap.find(feature) != featureMap.end();
}

void Annotation::deleteFeature(FeatureName feature) {
  featureMap.erase(feature);
}

set<string>
Annotation::getFeatureNames()
{
  set<string> ss;

  for (FeatureMap::const_iterator pos = featureMap.begin(); pos != featureMap.end(); ++pos)
    ss.insert(pos->first);

  return ss;
}

void Annotation::unsetFeatures() {
  featureMap.unsetFeatures();
}

string Annotation::toString() {
  string outString;

  outString += "<Annotation id=\"" + id + "\" type=\"" + type;
  outString += "\" start=\"" + start->getId() + "\" end=\"" 
	   + end->getId() + "\">\n";
  outString += featureMap.toString();
  outString += "</Annotation>";

  return outString;
}

// Assignment and comparisons

Annotation Annotation::operator = (const Annotation& a) {
  start = a.start;
  end = a.end;
  type = a.type;
  featureMap = a.featureMap;
  return(*this);
}

bool operator < (const Annotation& a, const Annotation& b) {
  if (a.start->getOffset() == b.start->getOffset())
    return (a.end->getOffset() < b.end->getOffset());
  else
    return (a.start->getOffset() < b.start->getOffset());
}

bool operator == (const Annotation& a, const Annotation& b) {
  return (a.start->getOffset() == b.start->getOffset()
	  && a.end->getOffset() == b.end->getOffset()
	  && a.type == b.type
	  && a.featureMap == b.featureMap);
}
