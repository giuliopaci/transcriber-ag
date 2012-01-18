// AnnotationIndex.h: This class maintains all the indexes over annotations.
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


// The annotation data is indexed in a wide variety of ways.
// This class maintains all the indexes.

#ifndef AnnotationIndex_h
#define AnnotationIndex_h

#include <map>
#include <algorithm>

#include <ag/AGTypes.h>
#include <ag/Identifiers.h>
#include <ag/Anchor.h>
#include <ag/Annotation.h>
#include <ag/Hash.h>

using namespace std;

// Indexes on Anchors
typedef hash_map<Id, Anchor*, hashString, StringEqual > AnchorById;
typedef map<Offset, AnchorSet, less<Offset> > AnchorSetByOffset;

// Indexes on Annotations
// - by convention we leave "annotation" out of the name
typedef hash_map<Id, Annotation*, hashString, StringEqual > ById;
typedef hash_map<String, AnnotationSet, hashString, StringEqual > ByType;
typedef map<Offset, AnnotationSet>  ByOffset;
typedef hash_map<pair<string,string>, AnnotationSet, hashStringPair, StringPairEqual >  ByFeature;
typedef hash_map<Anchor*, AnnotationSet, hashPointer, PointerEqual> ByAnchor;

/** 
 * AnnotationIndex manages the indexes over anchors and annotations within an AG.
 * Anchors are indexed by their id and offset. 
 * Annotations are indexed by their id, type, feature, start anchor, end anchor,
 * start anchor offset and end anchor offset.
 * @author Xiaoyi Ma, Steven Bird
 **/
class AnnotationIndex {
  friend class AG;
 private:
  /// the anchors
  AnchorSeq anchorSet;

  /// anchors indexed by identifier
  AnchorById anchorById;
  /// anchors at a given offset
  AnchorSetByOffset anchorSetByOffset;

  /// annotations indexed by identifier
  ById byId;
  /// annotations indexed by type
  ByType byType;
  /// annotations indexed by the offset of start anchor
  ByOffset byStartAnchorOffset;
  /// annotations indexed by the offset of end anchor
  ByOffset byEndAnchorOffset;
  /// annotations indexed by feature-value pair
  ByFeature byFeature;

  /// annotations indexed by their end anchor
  ByAnchor incoming;
  /// annotations indexed by their start anchor
  ByAnchor outgoing;

  /// add an anchor to the indexes
  void addAnchor(Anchor*);
  /// delete an anchor from the indexes
  void deleteAnchor(Anchor*);

 public:
  /// A constructor. Create an AnnotationIndex with an empty set of indexes.
  AnnotationIndex();
  /// A destructor
  ~AnnotationIndex();


  /// add an annotation to the indexes
  void add(Annotation* a);
  /// delete an annotation from the indexes
  void deleteAnnotation(Annotation* a);
  /// add a feature to the indexes
  void addFeature(Annotation* a, const String& feature, const String& value);
  /// delete a feature from the indexes
  void deleteFeature(Annotation* a, const String& feature);
  /**
   * Get the annotations that overlap a particular time offset.
   * Get all annotations whose start anchor offset is smaller than or
   * equal to the given offset AND end anchor offset is greater than 
   * or equal to the given offet.
   **/
  AnnotationSet getByOffset(Offset offset);
  /**
   * Get one of the annotations which overlap a particular time offset.
   * Same as getByOffset except that getAnnotationByOffset returns only
   * one qualified annotation while getByOffset returns all of them.
   * getAnnotationByOffset is signaficantly faster than getByOffset.
   * Should be favored over getByOffset whenever possible.
   * @see getByOffset
   **/
  Annotation*
  getAnnotationByOffset(Offset offset, const AnnotationType& type="");

  /// Test if the specified annotation exists
  bool existsAnnotation(const Id& id);
  /// Test if the specified anchor exists
  bool existsAnchor(const Id& id);
  /// get the nearest used offset to the specified offset
  Offset getNearestOffset(Offset o);
};

#endif
