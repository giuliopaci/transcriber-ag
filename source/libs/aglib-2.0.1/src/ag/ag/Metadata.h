// Metadata.h: Metadata associated with AGSet, AG, Timeline and Signal
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef Metadata_h
#define Metadata_h

#include <list>

#include <ag/AGTypes.h>
#include <ag/FeatureMap.h>

using namespace std;

/**
 * Metadata is a set of feature value pairs.
 * @version 1.4 September 21, 2001
 * @author Xiaoyi Ma
*/
class Metadata : public FeatureMap
{
 public:
  /// Set a feature and its value
  void setFeature(FeatureName f, FeatureValue v);
  
  /// Test to see if feature exists
  bool existsFeature(FeatureName f);

  /// Delete a feature
  void deleteFeature(FeatureName f);

  /**
   * Get feature names
   * @return string set which contains all the feature names
   **/
  set<string>
  getFeatureNames();

  /**
   * Set multiple features in a hash table of feature-value pairs.
   */
  void
  setFeatures(map<string,string>& f) {
    for (map<string,string>::iterator pos=f.begin(); pos!=f.end(); ++pos)
      (*this)[pos->first] = (*this)[pos->second];
  }

  /// Create list of SQLs used to save the metadata to the DB server
  list<string> storeSQLs(AGSetId agSetid, AGId agId, Id id);

  /// Dump the metadata in AIF format
  string toString();

};

#endif
