// Identifiers.h: Identifiers generates unique identifiers for anchors
// and annotations
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef Identifiers_h
#define Identifiers_h

#include <cstdio>
#include <string>
#include <set>

#include <ag/AGTypes.h>
#include <ag/AGException.h>
#include <ag/Hash.h>

class AGSet;
class AG;
class Timeline;
class Signal;
class Annotation;
class Anchor;

using namespace std;

typedef string String;
typedef hash_set<Id,hashString,StringEqual> IdSet;
typedef hash_map<AGSetId,AGSet*,hashString,StringEqual > AGSetRefs;
typedef hash_map<AGId,AG*,hashString,StringEqual > AGRefs;  
typedef hash_map<TimelineId,Timeline*,hashString,StringEqual > TimelineRefs;  
typedef hash_map<SignalId,Signal*,hashString,StringEqual > SignalRefs;
typedef hash_map<AnnotationId,Annotation*,hashString,StringEqual > AnnotationRefs;
typedef hash_map<AnchorId,Anchor*,hashString,StringEqual > AnchorRefs;

/**
 * Identifiers manages the issuing of ids.
 * Identifiers generates unique identifiers for AGSets, AGs, timelines,
 * signals, annotations, and anchors. It also maintains Id to reference
 * maps for AGSet, AG, Signal, Timeline, Annotation and Anchor, to speed 
 * up reference lookup.
 * @author Xiaoyi Ma
 **/
class Identifiers {
 private:
  /// The name space of the identifier
  const Id nameSpace;

  /// The type of ids, eg. AG, Anchor, etc.
  const string type;

  /// The highest id issued so far
  long cur;

  /// The collection of identifiers already issued
  IdSet issued;

/* (( BT Patch -- */
public:
/* -- BT Patch )) */

  /// AGSetId to AGSet reference mapping
  static AGSetRefs agSetRefs;
  /// AGId to AG reference mapping
  static AGRefs agRefs;
  /// AGSetId to AGSet reference mapping
  static TimelineRefs timelineRefs;  
  /// SignalId to Signal reference mapping
  static SignalRefs signalRefs;
  /// AnnotationId to Annotation reference mapping
  static AnnotationRefs annotationRefs;
  /// AnchorId to Anchor reference mapping
  static AnchorRefs anchorRefs;

 public:
  /// Create an identifiers with the specified namespace and type
  Identifiers(const Id& nameSpace, string type);

  /// Generate a new identifier
  const Id new_id();

  /**
   * Try to register the given identifier.
   * @param id existing identifier
   * @return the identifier created
   * @note if the identifier already exists in #issued
   * then create a new identifier
   * @throw AGException IF (the namespace of id is different from the one of the Identifiers)
  */
  const Id new_id(const Id& id) throw (AGException);

  /// Test if an id has been issued.
  bool existsId(const Id& id);
  
  /// Reclaim an id.
  void reclaim_id(const Id& id);

  /// Return all issued ids.
  IdSet& getIssuedIds() { return issued; }

  /**
   * Get the namespace of a given id.
   * For example, the namespace of "TIMIT:AG122:Annotaion32" would be "TIMIT:AG122",
   * and the namespace of "TIMIT:AG122" would be "TIMIT".
   **/
  static Id getNamespace(const Id& id);

  /// Get AGSet reference by id
  static AGSet* getAGSetRef(const AGSetId& id);
  /// Get AG reference by id
  static AG* getAGRef(const AGId& id);
  /// Get Timeline reference by id
  static Timeline* getTimelineRef(const TimelineId& id);
  /// Get Signal reference by id
  static Signal* getSignalRef(const SignalId& id);
  /// Get Annotation reference by id
  static Annotation* getAnnotationRef(const AnnotationId& id);
  /// Get Anchor reference by id
  static Anchor* getAnchorRef(const AnchorId& id);

  /// Delete an AGSet id to reference mapping
  static void deleteAGSetRef(const AGSetId& id);
  /// Delete an AG id to reference mapping
  static void deleteAGRef(const AGId& id);
  /// Delete a Timeline id to reference mapping
  static void deleteTimelineRef(const TimelineId& id);
  /// Delete a Signal id to reference mapping
  static void deleteSignalRef(const SignalId& id);
  /// Delete an Annotation id to reference mapping
  static void deleteAnnotationRef(const AnnotationId& id);
  /// Delete an Anchor id to reference mapping
  static void deleteAnchorRef(const AnchorId& id);

  /// Add an AGSet id to reference mapping
  static void addAGSetRef(const AGSetId& id, AGSet* ref);
  /// Add an AG id to reference mapping
  static void addAGRef(const AGId& id, AG* ref);
  /// Add a Timeline id to reference mapping
  static void addTimelineRef(const TimelineId& id, Timeline* ref);
  /// Add a Signal id to reference mapping
  static void addSignalRef(const SignalId& id, Signal* ref);
  /// Add an Annotation id to reference mapping
  static void addAnnotationRef(const AnnotationId& id, Annotation* ref);
  /// Add an Anchor id to reference mapping
  static void addAnchorRef(const AnchorId& id, Anchor* ref);

  /// Test if an AGSet exists
  static bool existsAGSet(const AGSetId& id);
  /// Test if an AG exists
  static bool existsAG(const AGId& id);
  /// Test if a Timeline exists
  static bool existsTimeline(const TimelineId& id);
  /// Test if a Signal exists
  static bool existsSignal(const SignalId& id);
  /// Test if an Annotation exists
  static bool existsAnnotation(const AnnotationId& id);
  /// Test if an Anchor exists
  static bool existsAnchor(const AnchorId& id);

};

#endif
