// Timeline.h:
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef Timeline_h
#define Timeline_h

using namespace std;

/**
 * Timeline is a set of signals.
 * @version 1.3 September 21, 2001
 * @author Xiaoyi Ma
 **/
class Timeline {
 private:
  /**
   * The id of the timeline
   **/
  const Id id;

  /**
   * The metadata of the timeline.
   */
  Metadata metadata;

  /**
   *  The signals.
   */
  SignalSet signals;

  /**
   * The signal Id issuer.
   * signalIds manages id issuing, it makes sure no duplicated id to be issued
   * and new ids can be automatically generated.
   */
  Identifiers* signalIds;

 public:
  /// Constructor
  Timeline(const Id& id);

  /// Destructor
  ~Timeline();

  /// Get the id of the timeline.
  const Id getId() const { return id; }

  /// Get signal Ids.
  set<SignalId>
  getSignals() const;

  /**
   * Create a new signal and add it to the timeline.
   * @param id might be TimelineId or SignalId. If id is a TimelineId, an SignalId
   * will be assigned to new Signal. If id is a SignalId, it will try the given id
   * first, if it's taken, assign a new SignalId.
   * @return SignalId of the new signal.
   * @throw AGException IF (the id given is invalid)
   **/
  SignalId createSignal(Id id, string xlinkHref, MimeClass mimeClass,
			MimeType mimeType, Encoding encoding, Unit unit, 
			string track) throw (AGException);

  /// Remove a signal from the timeline
  void deleteSignal(Signal* signal);

  /// Set the value of a feature in the metadata
  void setFeature(FeatureName featureName, FeatureValue featureValue);

  /// Test if a feature exists in the metadata
  bool existsFeature(FeatureName featureName);

  /// Delete the specified feature in the metadata
  void deleteFeature(FeatureName featureName);

  /// Get the value of the specified feature in the metadata
  string getFeature(FeatureName featureName);

  /** Get feature names in the metadata
   * @return string set which contains all the feature names
   **/
  set<string>
  getFeatureNames();

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
  
  /**
   * Unset all features in the metadata
   * Set all the features to empty string.
   **/
  void unsetFeatures();
  
  /// Create list of SQLs used to save the timeline to the DB server
  list<string> storeSQLs(AGSetId id);

  /// Dump the timeline in AIF format
  string toString();

};

#endif
