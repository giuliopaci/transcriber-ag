// Signal.h:
// Author: Xiaoyi Ma, Steven Bird
/// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
/// Web: http:///www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
/// For license information, see the file `LICENSE' included
/// with the distribution.


#ifndef Signal_h
#define Signal_h

#include <ag/AGTypes.h>
#include <ag/Metadata.h>

using namespace std;

/**
 * Signal is the thing to be annotated,
 * it can be audio, video, or text.
 * @version 1.3 September 21, 2001
 * @author Xiaoyi Ma
 **/
class Signal {
 private:
  /// The identifier of this signal, required
  const Id id;

  /// The mimeClass of the signal, required
  const MimeClass mimeClass;

  /// The mimeType of the signal, required
  const MimeType mimeType;

  /// The encoding of the signal, required
  const Encoding encoding;

  /// The type of the signal, fixed
  const string xlinkType;

  /// The href of the signal, required
  const string xlinkHref;

  /// The unit of the signal, required
  const Unit unit;

  /// The track of the signal, optional
  const Track track;

  /// Metadata of the signal, optional
  Metadata  metadata;

 public:
  /// Create a signal
  Signal(const Id id, string xlinkHref, MimeClass mimeClass, MimeType mimeType, Encoding encoding, Unit unit, Track track="");

  /// Get the id of the signal
  const Id getId() const ;

  /// Get the mimeclass of the signal
  const MimeClass getMimeClass() const;

  /// Get the mimetype of the signal
  const MimeType getMimeType() const;

  /// Get the encoding of the signal
  const Encoding getEncoding() const;

  /// Get the xlink:href of the signal
  const string getXlinkHref() const;

  /// Get the xlink:type of the signal
  const string getXlinkType() const;

  /// Get the unit of the signal
  const Unit getUnit() const;

  /// Get the track of the signal
  const Track getTrack() const;

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

  /// Create list of SQLs used to save the signal to the DB server
  list<string> storeSQLs(AGSetId agSetId, TimelineId timelineId);

  /// Dump the signal in AIF format
  string toString();

};

typedef set<Signal*> SignalSet;

#endif

