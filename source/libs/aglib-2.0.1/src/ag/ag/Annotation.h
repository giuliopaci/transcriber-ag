// Annotation.h: An annotation associates symbolic information to a
// region of signal
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef Annotation_h
#define Annotation_h

#include <string>

#include <ag/AGTypes.h>
#include <ag/Anchor.h>
#include <ag/FeatureMap.h>
#include <ag/Identifiers.h>

using namespace std;

/**
 * An annotation associates symbolic information to a region of signal.
 * An annotation has a start anchor and an end anchor which defines what
 * region of which signals are being annotated. An annotation also contains
 * feature value pairs as its content.
 * @version 1.5 September 21, 2001
 * @author Xiaoyi Ma
 **/
class Annotation {
  friend class AG;
  friend class AnnotationIndex;

 private:
  /// the identifier for the annotation
  const Id id;

  /// the type of the annotation  
  String type;

  /// the start anchor of the region being annotated
  Anchor* start;

  /// the end anchor of the region being annotated
  Anchor* end;

  /// the features of the annotation
  FeatureMap featureMap;

 protected:
  /// set the start anchor of the annotation
  void setStartAnchor(Anchor* a) { start = a; }

  /// set the end anchor of the annotation
  void setEndAnchor(Anchor* a) { end = a; }

  /// Set a feature and its value
  void setFeature(const FeatureName& feature, const FeatureValue& value);

 public:
  /// Copy an annotation, using the given identifier
  Annotation(Id id, const Annotation*);

  /**
   * A constructor.
   * Create an annotation with given identifier, for the specified region and type.
   * @param id the id of the annotation
   * @param start the start anchor of the annotation
   * @param end the end anchor of the annotation
   * @param type the type of the annotation
   **/
  Annotation(Id id, Anchor* start, Anchor* end, String type)
    : id(id), type(type), start(start), end(end)
  {}


  /// Get the id of the annotation
  Id getId() const { return id; }

  /// Get the start anchor of the annotation
  Anchor* getStartAnchor() const { return start; }

  /// Get the end anchor of the annotation
  Anchor* getEndAnchor() const { return end; }

  /**
   * Get all the information of the annotation
   * Get all the information of the specified annotation
   * @return a string containing annotation type, start anchor offset, 
   * end anchor offset, start anchor id, end anchor id and all features
   * in DCSV format.
   * @note see http://dublincore.org/documents/2000/07/28/dcmi-dcsv/
   * for details about DCSV format.
   **/
  String getAnnotationInfo();


  /// Get the type of the annotation
  String getType() const { return type; }

  /// Get the featuremap of the annotation
  const FeatureMap getFeatureMap() const { return featureMap; }

  /// Get the specified feature of the annotation
  FeatureValue getFeature(const FeatureName&) ;

  /// Get the value of specified feature
  const char* getFeatureC(const FeatureName&) ;

  /// Test if the specified feature exists
  bool existsFeature(FeatureName featureName);

  /// Delete the specified feature
  void deleteFeature(FeatureName featureName);
  
  /**
   * Get feature names
   * @return string set which contains all the feature names
   **/
  set<string>
  getFeatureNames();

  /**
   * Get all the features.
   * Returns all features in a hash table of feature-value pairs.
   */
  map<string,string>
  getFeatures()
  { return (map<string,string>) featureMap; }

  
  /**
   * Unset all the features.
   * Set all the features to empty string.
   **/
  void unsetFeatures();

  /// Get a reference of the annotation graph containing this annotation.
  AG* getAGRef()
  { return Identifiers::getAGRef(Identifiers::getNamespace(id)); }

  ///Dump the annotation in AIF format
  string toString();

  Annotation operator = (const Annotation&);
  friend bool operator < (const Annotation&, const Annotation&);
  friend bool operator == (const Annotation&, const Annotation&);
};


/**
 * Annotation compare function.
 * A function to compare annotations by first comparing their start anchor offsets,
 * if they are the same then comparing their end anchor offsets,
 * if they are the same then compare their ids.
 **/
class AnnotationCompFunc {
 public:
  bool operator() (const Annotation* a1, const Annotation* a2) const {
    Offset a1StartOffset = a1->getStartAnchor()->getOffset();
    Offset a1EndOffset = a1->getEndAnchor()->getOffset();
    Offset a2StartOffset = a2->getStartAnchor()->getOffset();
    Offset a2EndOffset = a2->getEndAnchor()->getOffset();
    
    if (a1StartOffset != a2StartOffset)
      return a1StartOffset < a2StartOffset;
    else if (a1EndOffset != a2EndOffset)
      return a1EndOffset < a2EndOffset;
    else
      return a1->getId() < a2->getId();
  }
};

/**
 * Annotation Sequence.
 * a set of annotations sorted by AnnotationCompFunc, i.e. start anchor and end
 * anchor offsets.
 **/
typedef set<Annotation*,AnnotationCompFunc> AnnotationSeq;
typedef set<Annotation*> AnnotationSet;
#endif
