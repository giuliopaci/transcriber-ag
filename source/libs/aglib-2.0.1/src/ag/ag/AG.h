// AG.h: A AG is a set of annotations along with the index structure.
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef AG_h
#define AG_h

#include <list>

#include <ag/AGTypes.h>
#include <ag/Identifiers.h>
#include <ag/Anchor.h>
#include <ag/Annotation.h>
#include <ag/AnnotationIndex.h>
#include <ag/Timeline.h>

typedef set<FeatureName> FeatureNameSet;

using namespace std;

/**
 * An AG (Annotation Graph) is a set of annotations along with the index structure.
 * @author Xiaoyi Ma
 * @note that operations on anchors are private - these are
 * used only by the class itself when building an annotation
*/
class AG : public AnnotationSeq {
 private:
  /// The identifier of the AG
  const Id id;

  /// The type of the AG
  string type;

  /// The timeline of the AG
  Timeline* timeline;

  /// The Metadata of the AG
  Metadata metadata;

  /// The index structure
  AnnotationIndex I;

  /// The anchor id issuer
  Identifiers* anchorIds;
  
  /// The annotation id issuer
  Identifiers* annotationIds;

 public:

  /// A constructor.
  AG(Id id, Timeline* timeline=NULL, string type="");

  /// A destructor.
  ~AG();

  /// Get the id of the AG
  Id getId() const { return id; }

  /// Set Type
  void setType(string type) { this->type = type; }

  /// Get Type
  string getType() const { return type; }

  /// Get Timeline
  Timeline* getTimeline() const { return timeline; }

  /// Add an anchor
  void addAnchor(Anchor* a);
  
  /// Delete an anchor
  void deleteAnchor(Anchor* a) throw (AGException);

  /// Set an anchor's offset to the specified value
  void setAnchorOffset(Anchor*, Offset);

  /**
   * Split an anchor in two.
   * Split an anchor a in two, creating a new anchor a'
   * having the same offset as the original one.
   * Anchor a has all the incoming annotations, while
   * anchor a' has all the outgoing annotations.
   * The new anchor a' is returned.
   **/
  Anchor* splitAnchor(Anchor* original);

  /// Add a new annotation to the AG
  void add(Annotation* anchor);

  /**
   * Create a new annotation.
   * @param id might be AGId or AnnotationId. If id is an AGId, an AnnotationId
   * will be assigned to the new annotation. If id is an AnnotationId, it will 
   * try the given id first, if it's taken, assign a new AnnotationId.
   * @return the AnnotationId of the new annotation.
   * @throw AGException IF (the id given is invalid)
   **/
  Id createAnnotation(Id id, Anchor* anchor1, Anchor* anchor2,
		      AnnotationType annotationType) throw(AGException);

  /**
   * Set feature value of the metadata or annotation.
   * @param id could be AGId or AnnotationId.
   * If it is AGId, then set the feature value of the metadata of the AG.
   * Or, if it is AnnotationId, set the feature value of the annotation.
   **/
  void setFeature(Id id, FeatureName featureName, FeatureValue featureValue);


  /**
   * Test if a feature exists in the metadata or annotation.
   * @param id could be AGId or AnnotationId.
   * If it is AGId, then test the existence of the feature of the metadata of the AG.
   * Or, if it is AnnotationId, test the existence of the feature of the annotation.
   **/
  bool existsFeature(Id id, FeatureName featureName);

  /**
   * Delete the specified feature from the metadata or annotation.
   * @param id could be AGId or AnnotationId.
   * If it is AGId, then delete the feature from the metadata of the AG.
   * Or, if it is AnnotationId, delete the feature from the annotation.
   **/
  void deleteFeature(Id id, FeatureName featureName);

  /** Get the value of specified feature in the metadata or annotation.
   * @param id could be AGId or AnnotationId.
   * If it is AGId, then get the value of specified feature from the metadata of the AG.
   * Or, if it is AnnotationId, get the value of specified feature from the annotation.
   **/
  string getFeature(Id id, FeatureName featureName);

  /**
   * Get all feature names from the metadata or annotation.
   * @param id could be AGId or AnnotationId.
   * If it is AGId, then get all feature names from the metadata of the AG.
   * Or, if it is AnnotationId, get all feature names from the annotation.
   * @return string set which contains all the feature names
   **/
  set<string>
  getFeatureNames(Id id);

  /**
   * Get all feature names of all annotations in this AG.
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
  getAnnotationFeatureNames(set<string>& S, const AnnotationType& type="");

  /**
   * Get types of all annotations in this AG.
   *
   * @param S
   *   The container for this method to put the results.
   *   Note that this method doesn't clean S before it puts
   *   the results.
   */
  void getAnnotationTypes(set<string>& S);

  /**
   * Set multiple features in a hash table of feature-value pairs.
   *
   * @note id could be AGId or AnnotationId.
   * If it is AGId, then set features of the metadata of the AG.
   * Or, if it is AnnotationId, set features of the annotation.
   */
  void
  setFeatures(Id id, map<string,string>& features);

  /**
   * Get all the features.
   * Returns all features in a hash table of feature-value pairs.
   */
  map<string,string>
  getFeatures(Id id);
  
  /**
   * Unset all features in the metadata or annotation.
   * Set all the features to empty string.
   * @param id could be AGId or AnnotationId.
   * If it is AGId, then unset features of the metadata of the AG.
   * Or, if it is AnnotationId, unset features of the annotation.
   **/
  void unsetFeatures(Id id);
 
  /** Create an anchor with specified offset, unit and signals.
   * @param id might be AGId or AnchorId. If id is a AGId, an AnchorId
   * will be assigned the new anchor. If id is a AnchorId, it will try the 
   * given id first, if it's taken, assign a new AnchorId.
   * @return AnchorId of the new anchor.
   * @throw AGException IF (the id given is invalid)
   **/
  AnchorId
  createAnchor(Id id, Offset offset, Unit unit, set<string> const &ignals);

  /** Create an anchor with specified unit and signals.
   * @param id might be AGId or AnchorId. If id is a AGId, an AnchorId
   * will be assigned the new anchor. If id is a AnchorId, it will try the 
   * given id first, if it's taken, assign a new AnchorId.
   * @return AnchorId of the new anchor.
   * @throw AGException IF (the id given is invalid)
   **/
  AnchorId createAnchor(Id id, Unit unit, set<string> const &signals); 

  /// Get the offset of the specified anchor
  Offset getAnchorOffset(Anchor* anchor);

  /** Clone an annotation.
   * Make a copy of the specified annotation, assign the new 
   * annotation a new AnnotationId.
   * @param annotation the original annotation
   * @return the pointer to the clone
   **/
  Annotation* copyAnnotation(const Annotation* annotation);

  /**
   * Split an annotation a in two.
   * Split an annotation a in two, creating a new annotation a'
   * having the same label data as the original one.
   * The two annotations a, a' connect head-to-tail at a
   * new anchor.  The new annotation and anchor have identifiers
   * taken from the specified identifier spaces. The new anchor is
   * unanchored, i.e. has no offset.
   * @param original, the annotation to be split.
   * @return a pointer to the new annotation a'.
   **/
  Annotation* splitAnnotation(Annotation* original);

  /**
   * Split an annotation to n annotations.
   * A version of split which does the split operation n-1 times, i.e.
   * split the original annotation into n annotations.
   * @param original, the annotation to be split.
   * @param n an integer specifying the number of annotations to split
   * into.
   * @return the set of new annotations, including the original one.
   * @see splitAnnotation
   **/
  AnnotationSet nSplitAnnotation(Annotation* original, int n);

  /// Delete the annotation from the AG.
  void deleteAnnotation(Annotation* annotation);

  /// Set the start anchor of an annotation to the specified anchor.
  void setStartAnchor(Annotation* annotation, Anchor* anchor);

  /// Set the end anchor of an annotation to the specified anchor.
  void setEndAnchor(Annotation* annotation, Anchor* anchor);

  /// Unset the offset of the specified anchor.
  void unsetAnchorOffset(Anchor* a);

  /// Set the specified feature of the annotation to this value.
  void setFeature(Annotation*, const String&, const String&);

  /// Get the set of anchors, sorted by offsets.
  AnchorSeq* getAnchorSet() { return &I.anchorSet; }

  /// Get the set of annotations
  IdSet& getAnnotationSet() { return annotationIds->getIssuedIds(); }

  /// Get the annotation reference by its id.
  Annotation* getById(const Id& id) throw (AGException);

  /// Get the anchor reference by its id.
  Anchor* getAnchorById(const Id& id) throw (AGException);

  /// Get anchors with the specified offset.
  AnchorSet getAnchorSetByOffset(Offset offset, double epsilon=0.0);

  /// Get the annotations of type t.
  AnnotationSet getAnnotationSetByType(const String& t) {return I.byType[t];}

  /**
   * Get the annotations that overlap a particular time offset.
   * Get all annotations whose start anchor offset is smaller than or
   * equal to the given offset AND end anchor offset is greater than 
   * or equal to the given offet.
   **/
  AnnotationSet getAnnotationSetByOffset(Offset o) {return I.getByOffset(o);}

  /// Get the annotations with value of feature f equals to v
  AnnotationSet getAnnotationSetByFeature(const String& f, const String& v) {return I.byFeature[make_pair(f,v)];}

  /// Get the incoming annotations to the specified node
  AnnotationSet getIncomingAnnotationSet(Anchor* a) {return I.incoming[a];}

  /// Get the incoming annotations of this type to the specified node
  AnnotationSet getIncomingAnnotationSetByType(Anchor* a, AnnotationType t);

  /// Get the outgoing annotations from the specified node
  AnnotationSet getOutgoingAnnotationSet(Anchor* a) {return I.outgoing[a];}

  /// Get the outgoing annotations of this type from the specified node
  AnnotationSet getOutgoingAnnotationSetByType(Anchor* a, AnnotationType t);

  /// Get the nearest used offset to the specified offset
  Offset getNearestOffset(Offset o) {return I.getNearestOffset(o);}

  /**
   * Get all annotations with its start anchor offset in between the specified values.
   * Get all annotations with its start anchor offset in between the specified values.
   * If both values are 0, return all annotations in the AG.
   * @param begin, the lower bound, default is 0.0.
   * @param end, the upper bound, default is 0.0.
   * @return all qualified annotations, sorted by start anchor offset.
   **/
  AnnotationSeq getAnnotationSeqByOffset(Offset begin=0.0, Offset end=0.0);

  /**
   * Get one of the annotations which overlap a particular time offset.
   * Same as getByOffset except that getAnnotationByOffset returns only
   * one qualified annotation while getByOffset returns all of them.
   * getAnnotationByOffset is signaficantly faster than getByOffset.
   * Should be favored over getAnnotationSetByOffset whenever possible.
   * @see getAnnotationSetByOffset
   **/
  Annotation*
  getAnnotationByOffset(Offset offset, const AnnotationType& type="")
  { return I.getAnnotationByOffset(offset, type); }

  /// Get all anchors whose offset is the nearest to the specified offset
  AnchorSet getAnchorSetNearestOffset(Offset offset);

  /// Create list of SQLs used to save the metadata to the DB server
  list<string> storeSQLs(AGSetId agSetId,FeatureNameSet features);

  /// Dump the AG in AIF format
  string toString();

};


#endif

