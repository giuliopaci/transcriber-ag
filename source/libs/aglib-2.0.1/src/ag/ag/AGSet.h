// AGSet.h: An AGSet is a set of AGs
// Author: Xiaoyi Ma
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef AGSet_h
#define AGSet_h

#include <ag/AG.h>
#include <ag/Identifiers.h>
#include <ag/Timeline.h>
#include <ag/Metadata.h>

using namespace std;

typedef vector<Timeline*> TimelineSet;
typedef vector<AG*> AGS;

/** 
 * An AGSet contains a set AGs (Annotation Graphs)
 * @author Xiaoyi Ma
 * @see AG
**/
class AGSet {
 private:
  /// The identifier of the AGSet
  const Id id;

  /// The timelines of the AGSet
  TimelineSet timelineSet;
  
  /// The id issuer for timelines
  Identifiers* timelineIds;

  /// The AGs of the AGSet
  AGS ags;

  /// The Metadata of the AGSet
  Metadata metadata;

  /// All feature names in the AGSet
  FeatureNameSet features;

  /// The id issuer for AG
  Identifiers* agIds;
  
 public:

  /// A constructor.
  AGSet(AGSetId id);

  /// A destructor.
  ~AGSet();

  /// Get the id of the AGSet
  Id getId() const {return id; }

  /**
   * Create a new timeline and add it to the AGSet.
   * @param id might be AGSetId or TimelineId. If id is an AGSetId, a TimelineId
   * will be assigned to the new timeline. If id is a TimelineId, it will try the given id
   * first, if it's taken, assign a new TimelineId.
   * @return TimelineId of the new timeline.
   * @throw AGException IF (the id given is invalid)
   **/
  TimelineId createTimeline(Id id) throw (AGException);

  /// Delete the specified timeline
  void deleteTimeline(TimelineId timelineId) throw (AGException);

  /**
   * Create a new AG and add it to the AGSet.
   * @param id might be AGSetId or AGId. If id is a AGSetId, the id issuer
   * will generate a new AGId. If id is a AGId, it will try the given id 
   * first, if it's taken, generate a new AGId.
   * @return AGId of the new AG.
   * @throw AGException IF (the id given is invalid)
   * @note default value for timeline is NULL.
   **/
  AGId createAG(Id id, Timeline* timeline=NULL) throw (AGException);

  /// Delete the specified AG
  void deleteAG(AGId agId);
 
  /// Get all AG ids
  set<AGId>
  getAGIds();

  /**
   * Add a feature name to the feature name set.
   * When a database server is used, the current schema stores the annotation
   * features of each AGSet in a separate table which stores all the features
   * of an annotation in a single record and use the FeatureName as the column
   * heading. In order to create such a table, an AGSet has to keep track of 
   * how many feature names are there and what they are.
   *
   * The strategy is to keep a set of feature names in the AGSet, and add a 
   * feature's name to the set everytime a SetFeature is called and its id is 
   * an AnnotationId.
   * addFeatureName adds a feature name to the set.
   **/
  void addFeatureName(FeatureName feature) { features.insert(feature); }

  /// Set the value of a feature in the metadata
  void setFeature(FeatureName featureName, FeatureValue featureValue);

  /// Test if a feature exists in the metadata 
  bool existsFeature(FeatureName featureName);
 
  /// Delete the specified feature in the metadata
  void deleteFeature(FeatureName featureName);
 
  /// Get the feature value of the specified feature in the metadata
  string getFeature(FeatureName featureName);

  /// Get all feature names in the metadata
  set<string>
  getFeatureNames();

  /**
   * Get all feature names of all annotations in this AGSet.
   *
   * @param S
   *   The container for this method to put the results.
   *   Note that this method doesn't clean S before it puts
   *   the results.
   * @param type
   *   If given, only the annotations of the given type is considered
   *   in the computation -- things will get slow.
   */
  void
  getAnnotationFeatureNames(set<string>& S, const AnnotationType& type);

  /**
   * Get types of all annotations in this AGSet.
   *
   * @param S
   *   The container for this method to put the results.
   *   Note that this method doesn't clean S before it puts
   *   the results.
   */
  void getAnnotationTypes(set<string>& S);

  /**
   * Set multiple features in a hash table of feature-value pairs.
   */
  void
  setFeatures(map<string,string>& features)
  { metadata.setFeatures(features); }
 
  /** 
   * Get all the features in a hash table of feature-value pairs.
   */
  map<string,string>
  getFeatures()
  { return (map<string,string>) metadata; }
 
  /// Unset all features in the metadata
  void unsetFeatures();
 
  /** dump the specified AG in ATLAS Level 0 form
   * @see ag.dtd
   **/
  string toXML(AGId agId);

  /// Create list of SQLs used to save the AGSet to the DB server
  list<string> storeSQLs();

  /** dump the AGSet in ATLAS Level 0 form
   * @see ag.dtd
   **/
  string toString();
};

#endif

