// FeatureMap.h
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef FeatureMap_h
#define FeatureMap_h

#include <set>
#include <map>
#include <ag/AGTypes.h>
#include <ag/AGException.h>

using namespace std;

/** A feature map is a set of name-value pairs, which 
 * is used in Metadata and Annotation features
 * @author Xiaoyi Ma
*/
class FeatureMap : public map<string,string>
{
public:
  /// Get the value of the feature
  FeatureValue
  getFeature(FeatureName name) const
    throw (AGException);

  /// Unset all the features
  void
  unsetFeatures();

  /// Dump the feature map in AIF format
  string
  toString();

};

#endif
