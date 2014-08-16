// AGAPI.cc: impletation of AG API
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

/* (( BT Patch -- */
#include <stdlib.h>
#include <math.h>
/* -- BT Patch )) */

#include <queue>
#include <ag/Utilities.h>
#include <ag/AGSet.h>
#include <ag/AGAPI.h>
#include <ag/Validation.h>
#include <ag/agfio.h>


void logError(string callingFun, AGException e) throw (AGException) {
  throw AGException("AG Exception caught when calling " + callingFun + ":\n" + e.error());
}

//// AGSet ////
bool ExistsAGSet(AGSetId id) {
  return Identifiers::existsAGSet(id);
}

AGSetId CreateAGSet(Id id) {
  try {
    if (id.empty() || !Identifiers::getNamespace(id).empty())
      throw AGException(id+ " is not a valid AGSet id!");

    if (ExistsAGSet(id))
      throw AGException("AGSet \'" + id + "` already exists!");

    Identifiers::addAGSetRef(id,new AGSet(id));
    return id;
  } catch (AGException e) {
    logError("CreateAGSet",e);
  }
}

void DeleteAGSet(AGSetId id) {
  try {
    delete Identifiers::getAGSetRef(id);
    Identifiers::deleteAGSetRef(id);
  } catch (AGException e) {
    logError("DeleteAGSet",e);
  }
}

// Id is AGSetId or AGId
AGId CreateAG(Id id, TimelineId timelineId) {
  try {
    Id nspace = Identifiers::getNamespace(id);
    if (timelineId.empty())
      if (ExistsAGSet(id)) // if id is an AGSetId
	return Identifiers::getAGSetRef(id)->createAG(id);
      else if (ExistsAGSet(nspace)) // if id is an AGId
	return Identifiers::getAGSetRef(nspace)->createAG(id);
      else
	throw AGException(id + " is not a valid Id!");
    else {
      Timeline* timeline = Identifiers::getTimelineRef(timelineId);
      if (ExistsAGSet(id))
	return Identifiers::getAGSetRef(id)->createAG(id,timeline);
      else if (ExistsAGSet(nspace))
	return Identifiers::getAGSetRef(nspace)->createAG(id,timeline);
      else
	throw AGException(id + " is not a valid Id!");
    }
  } catch (AGException e) {
    logError("CreateAG",e);
  }
}

bool ExistsAG(AGId agId) {
  return Identifiers::existsAG(agId);
}

void DeleteAG(AGId agId) {
  try {
    Identifiers::getAGSetRef(Identifiers::getNamespace(agId))->deleteAG(agId);
  } catch (AGException e) {
    logError("DeleteAG",e);
  }
}

set<AGId>
GetAGIds(AGSetId agSetId)
{
  try {
    return Identifiers::getAGSetRef(agSetId)->getAGIds();
  }
  catch (AGException e) {
    logError("GetAGIds",e);
  }
}

TimelineId CreateTimeline(Id id) {
  try {
    if (ExistsAGSet(id))
      return Identifiers::getAGSetRef(id)->createTimeline(id);
    else
      return Identifiers::getAGSetRef(Identifiers::getNamespace(id))->createTimeline(id);
  } catch (AGException e) {
    logError("CreateTimeline",e);
  }
}

bool ExistsTimeline(TimelineId timelineId) {
  return Identifiers::existsTimeline(timelineId);
}
 
void DeleteTimeline(TimelineId timelineId) {
  try {
    Identifiers::getAGSetRef(Identifiers::getNamespace(timelineId))->deleteTimeline(timelineId);
  } catch (AGException e) {
    logError("DeleteTimeline",e);
  }
}  
 
// Id may be TimelineId or SignalId
SignalId CreateSignal(Id id, URI uri, MimeClass mimeClass,
		      MimeType mimeType, Encoding encoding,
		      Unit unit, Track track) {
  try {
    Id nspace = Identifiers::getNamespace(id);
    if (ExistsTimeline(id))
      return Identifiers::getTimelineRef(id)->createSignal(id, uri, mimeClass, mimeType, encoding, unit, track);
    else if (ExistsTimeline(nspace))
      return Identifiers::getTimelineRef(nspace)->createSignal(id, uri, mimeClass, mimeType, encoding, unit, track);
    else
      throw AGException(id + " is not a valid TimelineId or SignalId!");
  } catch (AGException e) {
    logError("CreateSignal",e);
  }
}

bool ExistsSignal(SignalId signalId) {
  return Identifiers::existsSignal(signalId);
}

void DeleteSignal(SignalId signalId) {
  try {
    Identifiers::getTimelineRef(Identifiers::getNamespace(signalId))->deleteSignal(Identifiers::getSignalRef(signalId));
  } catch (AGException e) {
    logError("DeleteSignal",e);
  }
}

set<SignalId>
GetSignals(TimelineId timelineId)
{
  try {
    return Identifiers::getTimelineRef(timelineId)->getSignals();
  }
  catch (AGException e) {
    logError("GetSignals",e);
  }
}


MimeClass GetSignalMimeClass(SignalId signalId) {
  try {
    return Identifiers::getSignalRef(signalId)->getMimeClass();
  } catch (AGException e) {
    logError("GetSignalMimeClass",e);
  }
}

MimeType GetSignalMimeType(SignalId signalId) {
  try {
    return Identifiers::getSignalRef(signalId)->getMimeType();
  } catch (AGException e) {
    logError("GetSignalMimeType",e);
  }
}

Encoding GetSignalEncoding(SignalId signalId) {
  try {
    return Identifiers::getSignalRef(signalId)->getEncoding();
  } catch (AGException e) {
    logError("GetSignalEncoding",e);
  }
}
string GetSignalXlinkType(SignalId signalId) {
  try {
    return Identifiers::getSignalRef(signalId)->getXlinkType();
  } catch (AGException e) {
    logError("GetSignalXlinkType",e);
  }
}

string GetSignalXlinkHref(SignalId signalId) {
  try {
    return Identifiers::getSignalRef(signalId)->getXlinkHref();
  } catch (AGException e) {
    logError("GetSignalXlinkHref",e);
  }
}

string GetSignalUnit(SignalId signalId){
  try {
    return Identifiers::getSignalRef(signalId)->getUnit();
  } catch (AGException e) {
    logError("GetSignalUnit",e);
  }
}

Track GetSignalTrack(SignalId signalId) {
  try {
    return Identifiers::getSignalRef(signalId)->getTrack();
  } catch (AGException e) {
    logError("GetSignalTrack",e);
  }
}

  
//// Annotation ////

// Id may be AGId or AnnotationId
AnnotationId CreateAnnotation(Id id,AnchorId anchorId1,AnchorId anchorId2,AnnotationType annotationType) {
  try {
    Anchor* anchor1 = Identifiers::getAnchorRef(anchorId1);
    Anchor* anchor2 = Identifiers::getAnchorRef(anchorId2);
    Id nspace;

    if (Identifiers::existsAG(id))
      return Identifiers::getAGRef(id)->createAnnotation(id, anchor1, anchor2, annotationType);
    else if (Identifiers::existsAG(nspace = Identifiers::getNamespace(id)))
      return Identifiers::getAGRef(nspace)->createAnnotation(id, anchor1, anchor2, annotationType);
  } catch (AGException e) {
    logError("CreateAnnotation",e);
  }
}

bool ExistsAnnotation(AnnotationId annotationId) {
  return Identifiers::existsAnnotation(annotationId);
}

void DeleteAnnotation(AnnotationId annotationId) {
  try {
    Identifiers::getAGRef(Identifiers::getNamespace(annotationId))->deleteAnnotation(Identifiers::getAnnotationRef(annotationId));
  } catch (AGException e) {
    logError("DeleteAnnotation",e);
  }
}

AnnotationId CopyAnnotation(AnnotationId annotationId) {
  try {
    return Identifiers::getAGRef(Identifiers::getNamespace(annotationId))->copyAnnotation(Identifiers::getAnnotationRef(annotationId))->getId();
  } catch (AGException e) {
    logError("CopyAnnotation",e);
  }
}

list<AnnotationId>
SplitAnnotation(AnnotationId annotationId)
{
  try {
    list<AnnotationId> res;
    res.push_back(annotationId);
    res.push_back(Identifiers::getAGRef(Identifiers::getNamespace(annotationId))->splitAnnotation(Identifiers::getAnnotationRef(annotationId))->getId());
    return res;
  }
  catch (AGException e) {
    logError("SplitAnnotation",e);
  }
}

list<AnnotationId>
NSplitAnnotation(AnnotationId annotationId,short n)
{
  try {
    AnnotationSet annotationIdSet = Identifiers::getAGRef(Identifiers::getNamespace(annotationId))->nSplitAnnotation(Identifiers::getAnnotationRef(annotationId),n);
    // put the original annotation first
    list<AnnotationId> res;
    res.push_back(annotationId);

    for (AnnotationSet::iterator pos = annotationIdSet.begin(); pos != annotationIdSet.end(); ++pos)
      res.push_back((*pos)->getId());

    return res;

  }
  catch (AGException e) {
    logError("NSplitAnnotation",e);
  }
}

String GetAnnotationInfo(AnnotationId annotationId) {
  try {
    return Identifiers::getAnnotationRef(annotationId)->getAnnotationInfo();
  } catch (AGException e) {
    logError("GetAnnotationInfo",e);
  }
} 

AnnotationType GetAnnotationType(AnnotationId annotationId) {
  try {
    return Identifiers::getAnnotationRef(annotationId)->getType();
  } catch (AGException e) {
    logError("GetAnnotationType",e);
  }
} 

AnchorId GetStartAnchor(AnnotationId annotationId) {
  try {
    return Identifiers::getAnnotationRef(annotationId)->getStartAnchor()->getId();
  } catch (AGException e) {
    logError("GetStartAnchor",e);
  }
}

AnchorId GetEndAnchor(AnnotationId annotationId) {
  try {
    return Identifiers::getAnnotationRef(annotationId)->getEndAnchor()->getId();
  } catch (AGException e) {
    logError("GetEndAnchor",e);
  }
}

void SetStartAnchor(AnnotationId annotationId,AnchorId anchorId) {
  try {
    Annotation* annotation = Identifiers::getAnnotationRef(annotationId);
    Anchor* anchor = Identifiers::getAnchorRef(anchorId);
    Identifiers::getAGRef(Identifiers::getNamespace(annotationId))->setStartAnchor(annotation, anchor);
  } catch (AGException e) {
    logError("SetStartAnchor",e);
  }
}

void SetEndAnchor(AnnotationId annotationId,AnchorId anchorId) {
  try {
    Annotation* annotation = Identifiers::getAnnotationRef(annotationId);
    Anchor* anchor = Identifiers::getAnchorRef(anchorId);
    Identifiers::getAGRef(Identifiers::getNamespace(annotationId))->setEndAnchor(annotation, anchor);
  } catch (AGException e) {
    logError("SetEndAnchor",e);
  }
}
 
Offset GetStartOffset(AnnotationId annotationId) {
  try {
    return Identifiers::getAnnotationRef(annotationId)->getStartAnchor()->getOffset();
  } catch (AGException e) {
    logError("GetStartOffset",e);
  }
}

Offset GetEndOffset(AnnotationId annotationId) {
  try {
    return Identifiers::getAnnotationRef(annotationId)->getEndAnchor()->getOffset();
  } catch (AGException e) {
    logError("GetEndOffset",e);
  }
}

void SetStartOffset(AnnotationId annotationId,Offset offset) {
  try {
    Identifiers::getAGRef(Identifiers::getNamespace(annotationId))->setAnchorOffset(Identifiers::getAnnotationRef(annotationId)->getStartAnchor(), offset);
  } catch (AGException e) {
    logError("SetStartOffset",e);
  }
}

void SetEndOffset(AnnotationId annotationId,Offset offset) {
  try {
    Identifiers::getAGRef(Identifiers::getNamespace(annotationId))->setAnchorOffset(Identifiers::getAnnotationRef(annotationId)->getEndAnchor(), offset);
  } catch (AGException e) {
    logError("SetEndOffset",e);
  }
}

set<AnnotationType>
GetAnnotationTypes(Id id)
{
  set<AnnotationType> S;

  try {
    if (Identifiers::existsAGSet(id))
      Identifiers::getAGSetRef(id)->getAnnotationTypes(S);
    else if (Identifiers::existsAG(id))
      Identifiers::getAGRef(id)->getAnnotationTypes(S);
    else if (Identifiers::existsAnnotation(id))
      S.insert(Identifiers::getAnnotationRef(id)->getType());
    else
      throw AGException(id + " is not a valid AGSetId, AGId or AnnotationId!");
  }
  catch (AGException& e) {
    logError("GetAnnotationTypes", e);
  }

  return S;
}

set<FeatureName>
GetAnnotationFeatureNames(Id id, const AnnotationType& type)
{
  set<FeatureName> S;

  try {
    if (Identifiers::existsAGSet(id))
      Identifiers::getAGSetRef(id)->getAnnotationFeatureNames(S, type);
    else if (Identifiers::existsAG(id))
      Identifiers::getAGRef(id)->getAnnotationFeatureNames(S, type);
    else if (Identifiers::existsAnnotation(id)) {
      if (type.empty() || type==GetAnnotationType(id))
	S = Identifiers::getAnnotationRef(id)->getFeatureNames();
    }
    else
      throw AGException(id + " is not a valid AGSetId, AGId or AnnotationId!");
  }
  catch (AGException& e) {
    logError("GetAnnotationFeatureNames", e);
  }

  return S;
}

// this might be necessary to package up an id into a durable reference
// AnnotationRef GetRef(Id id);
 
 
//// Features ////
 
// this is for both the content of an annotation, and for the metadata
// associated with AGSets, AGs, Timelines and Signals.
 
void SetFeature(Id id,FeatureName featureName,FeatureValue featureValue) {
  try {
    if (ExistsAnnotation(id)) {
      Identifiers::getAGSetRef(Identifiers::getNamespace(Identifiers::getNamespace(id)))->addFeatureName(featureName);
      Identifiers::getAGRef(Identifiers::getNamespace(id))->setFeature(Identifiers::getAnnotationRef(id),featureName,featureValue);
    } else if (ExistsAG(id))
      Identifiers::getAGRef(id)->setFeature(id,featureName,featureValue);
    else if (ExistsAGSet(id))
      Identifiers::getAGSetRef(id)->setFeature(featureName,featureValue);
    else if (ExistsTimeline(id))
      Identifiers::getTimelineRef(id)->setFeature(featureName, featureValue);
    else if (ExistsSignal(id))
      Identifiers::getSignalRef(id)->setFeature(featureName, featureValue);
    else
      throw AGException(id + " is not a valid AnnotationId, AGSetId, AGId, TimelineId or SignalId!");
  } catch (AGException e) {
    logError("SetFeature",e);
  }
}

bool ExistsFeature(Id id, FeatureName featureName) {
  try {
    if (ExistsAnnotation(id))
      return Identifiers::getAGRef(Identifiers::getNamespace(id))->existsFeature(id,featureName);
    else if (ExistsAG(id))
      return Identifiers::getAGRef(id)->existsFeature(id,featureName);
    else if (ExistsAGSet(id))
      return Identifiers::getAGSetRef(id)->existsFeature(featureName);
    else if (ExistsTimeline(id))
      return Identifiers::getTimelineRef(id)->existsFeature(featureName);
    else if (ExistsSignal(id))
      return Identifiers::getSignalRef(id)->existsFeature(featureName);
    else
      throw AGException(id + " is not a valid AnnotationId, AGSetId, AGId, TimelineId or SignalId!");
  } catch (AGException e) {
    logError("ExistsFeature",e);
  }
}

void DeleteFeature(Id id, FeatureName featureName) {
  try {
    if (ExistsAnnotation(id))
      Identifiers::getAGRef(Identifiers::getNamespace(id))->deleteFeature(id,featureName);
    else if (ExistsAG(id))
      Identifiers::getAGRef(id)->deleteFeature(id,featureName);
    else if (ExistsAGSet(id))
      Identifiers::getAGSetRef(id)->deleteFeature(featureName);
    else if (ExistsTimeline(id))
      Identifiers::getTimelineRef(id)->deleteFeature(featureName);
    else if (ExistsSignal(id))
      Identifiers::getSignalRef(id)->deleteFeature(featureName);
    else
      throw AGException(id + " is not a valid AnnotationId, AGSetId, AGId, TimelineId or SignalId!");
  } catch (AGException e) {
    logError("DeleteFeature",e);
  }
}

string GetFeature(Id id, FeatureName featureName) {
  try {
    if (ExistsAnnotation(id))
      return Identifiers::getAGRef(Identifiers::getNamespace(id))->getFeature(id,featureName);
    else if (ExistsAG(id))
      return Identifiers::getAGRef(id)->getFeature(id,featureName);
    else if (ExistsAGSet(id))
      return Identifiers::getAGSetRef(id)->getFeature(featureName);
    else if (ExistsTimeline(id))
      return Identifiers::getTimelineRef(id)->getFeature(featureName);
    else if (ExistsSignal(id))
      return Identifiers::getSignalRef(id)->getFeature(featureName);
    else
      throw AGException(id + " is not a valid AnnotationId, AGSetId, AGId, TimelineId or SignalId!");
  } catch (AGException e) {
    logError("GetFeature",e);
  }
}

void UnsetFeature(Id id,FeatureName featureName) {
  try {
    if (ExistsAnnotation(id))
      Identifiers::getAGRef(Identifiers::getNamespace(id))->setFeature(id,featureName,"");
    else if (ExistsAG(id))
      Identifiers::getAGRef(id)->setFeature(id,featureName,"");
    else if (ExistsAGSet(id))
      Identifiers::getAGSetRef(id)->setFeature(featureName,"");
    else if (ExistsTimeline(id))
      Identifiers::getTimelineRef(id)->setFeature(featureName, "");
    else if (ExistsSignal(id))
      Identifiers::getSignalRef(id)->setFeature(featureName, "");
    else
      throw AGException(id + " is not a valid AnnotationId, AGSetId, AGId, TimelineId or SignalId!");
  } catch (AGException e) {
    logError("UnsetFeature",e);
  }
}

set<FeatureName>
GetFeatureNames(Id id)
{
  try {
    if (ExistsAnnotation(id))
      return Identifiers::getAGRef(Identifiers::getNamespace(id))->getFeatureNames(id);
    else if (ExistsAG(id))
      return Identifiers::getAGRef(id)->getFeatureNames(id);
    else if (ExistsAGSet(id))
      return Identifiers::getAGSetRef(id)->getFeatureNames();
    else if (ExistsTimeline(id))
      return Identifiers::getTimelineRef(id)->getFeatureNames();
    else if (ExistsSignal(id))
      return Identifiers::getSignalRef(id)->getFeatureNames();
    else
      throw AGException(id + " is not a valid AnnotationId, AGSetId, AGId, TimelineId or SignalId!");
    
  }
  catch (AGException e) {
    logError("GetFeatureNames",e);
  }
}

void
SetFeatures(Id id, map<string,string>& features)
{
  try {
    if (ExistsAnnotation(id))
      Identifiers::getAGRef(Identifiers::getNamespace(id))->setFeatures(id,features);
    else if (ExistsAG(id))
      Identifiers::getAGRef(id)->setFeatures(id,features);
    else if (ExistsAGSet(id))
      Identifiers::getAGSetRef(id)->setFeatures(features);
    else if (ExistsTimeline(id))
      Identifiers::getTimelineRef(id)->setFeatures(features);
    else if (ExistsSignal(id))
      Identifiers::getSignalRef(id)->setFeatures(features);
    else
      throw AGException(id + " is not a valid AnnotationId, AGSetId, AGId, TimelineId or SignalId!");
  } catch (AGException e) {
    logError("SetFeatures",e);
  }
}

map<string,string>
GetFeatures(Id id)
{
  try {
    if (ExistsAnnotation(id))
      return Identifiers::getAGRef(Identifiers::getNamespace(id))->getFeatures(id);
    else if (ExistsAG(id))
      return Identifiers::getAGRef(id)->getFeatures(id);
    else if (ExistsAGSet(id))
      return Identifiers::getAGSetRef(id)->getFeatures();
    else if (ExistsTimeline(id))
      return Identifiers::getTimelineRef(id)->getFeatures();
    else if (ExistsSignal(id))
      return Identifiers::getSignalRef(id)->getFeatures();
    else
      throw AGException(id + " is not a valid AnnotationId, AGSetId, AGId, TimelineId or SignalId!");
  } catch (AGException e) {
    logError("GetFeatures",e);
  }
}

void UnsetFeatures(Id id) {
  try {
    if (ExistsAnnotation(id))
      Identifiers::getAGRef(Identifiers::getNamespace(id))->unsetFeatures(id);
    else if (ExistsAG(id))
      Identifiers::getAGRef(id)->unsetFeatures(id);
    else if (ExistsAGSet(id))
      Identifiers::getAGSetRef(id)->unsetFeatures();
    else if (ExistsTimeline(id))
      Identifiers::getTimelineRef(id)->unsetFeatures();
    else if (ExistsSignal(id))
      Identifiers::getSignalRef(id)->unsetFeatures();
    else
      throw AGException(id + " is not a valid AnnotationId, AGSetId, AGId, TimelineId or SignalId!");
  } catch (AGException e) {
    logError("UnsetFeatures",e);
  }
}
 
 
//// List-Valued Features ////
 
// ?? do we need to permit features to have list values?
 
 
//// Anchor ////
 
 // Id may be AGId or AnchorId
AnchorId
CreateAnchor(Id id, Offset offset, Unit unit, set<SignalId>& signalIds)
{
  try {
    Id nspace;
    if (ExistsAG(id))
      return Identifiers::getAGRef(id)->createAnchor(id,offset,unit,signalIds);
    else if (ExistsAG(nspace = Identifiers::getNamespace(id)))
      return Identifiers::getAGRef(nspace)->createAnchor(id,offset,unit,signalIds);
    else
      throw AGException("The AG of " + id + " doesn't exist!");
  } catch (AGException e) {
    logError("CreateAnchor",e);
  }
}

AnchorId
CreateAnchor(Id id, set<SignalId>& signalIds)
{
  try {
    Id nspace = Identifiers::getNamespace(id);

    if (ExistsAG(id))
      return Identifiers::getAGRef(id)->createAnchor(id,"",signalIds);
    else if (ExistsAG(nspace))
      return Identifiers::getAGRef(nspace)->createAnchor(id,"",signalIds);
    else
      throw AGException("The AG of " + id + " doesn't exist!");
  }
  catch (AGException e) {
    logError("CreateAnchor",e);
  }
}

AnchorId
CreateAnchor(Id id)
{
  try {
    Id nspace = Identifiers::getNamespace(id);

    if (ExistsAG(id))
      return Identifiers::getAGRef(id)->createAnchor(id,"",set<string>());
    else if (ExistsAG(nspace))
      return Identifiers::getAGRef(nspace)->createAnchor(id,"",set<string>());
    else
      throw AGException("The AG of " + id + " doesn't exist!");
  }
  catch (AGException e) {
    logError("CreateAnchor",e);
  }
}

bool ExistsAnchor(AnchorId anchorId) {
  return Identifiers::existsAnchor(anchorId);
}

void DeleteAnchor(AnchorId anchorId) {
  try {
    Identifiers::getAGRef(Identifiers::getNamespace(anchorId))->deleteAnchor(Identifiers::getAnchorRef(anchorId));
  } catch (AGException e) {
    logError("DeleteAnchor",e);
  }
}

void SetAnchorOffset(AnchorId anchorId,Offset offset) {
  try {
    Identifiers::getAGRef(Identifiers::getNamespace(anchorId))->setAnchorOffset(Identifiers::getAnchorRef(anchorId),offset);
  } catch (AGException e) {
    logError("SetAnchorOffset",e);
  }
}

Offset GetAnchorOffset(AnchorId anchorId) {
  try {
    return Identifiers::getAnchorRef(anchorId)->getOffset();
  } catch (AGException e) {
    logError("GetAnchorOffset",e);
  }
}

void SetOffsetUnit(AnchorId anchorId,Unit unit) {
  try {
    Identifiers::getAnchorRef(anchorId)->setUnit(unit);
  } catch (AGException e) {
    logError("SetOffsetUnit",e);
  }
}

Unit GetOffsetUnit(AnchorId anchorId) {
  try {
    return Identifiers::getAnchorRef(anchorId)->getUnit();
  } catch (AGException e) {
    logError("GetOffsetUnit",e);
  }
}

void
SetAnchorSignalIds(AnchorId anchorId,set<SignalId>& signalIds)
{
  try {
    Identifiers::getAnchorRef(anchorId)->setSignals(signalIds);
  }
  catch (AGException e) {
    logError("SetAnchorSignalIds",e);
  }
}

set<SignalId>
GetAnchorSignalIds(AnchorId anchorId)
{
  try {
    return Identifiers::getAnchorRef(anchorId)->getSignals();
  }
  catch (AGException e) {
    logError("GetAnchorSignalIds",e);
  }
}

bool GetAnchored(AnchorId anchorId) {
  try {
    return Identifiers::getAnchorRef(anchorId)->getAnchored();
  } catch (AGException e) {
    logError("GetAnchored",e);
  }
}

void UnsetAnchorOffset(AnchorId anchorId) {
  try {
    Identifiers::getAGRef(Identifiers::getNamespace(anchorId))->unsetAnchorOffset(Identifiers::getAnchorRef(anchorId));
  } catch (AGException e) {
    logError("UnsetAnchorOffset",e);
  }
}

AnchorId SplitAnchor(AnchorId anchorId) {
  try {
    return Identifiers::getAGRef(Identifiers::getNamespace(anchorId))->splitAnchor(Identifiers::getAnchorRef(anchorId))->getId();
  } catch (AGException e) {
    logError("SplitAnchor",e);
  }
}

set<AnnotationId>
GetIncomingAnnotationSet(AnchorId anchorId, const AnnotationType& type)
{
  try {
    set<AnnotationId> aids;
    AnnotationSet aidset =
      Identifiers::getAGRef(Identifiers::getNamespace(anchorId))
      ->getIncomingAnnotationSet(Identifiers::getAnchorRef(anchorId));
    AnnotationSet::iterator pos = aidset.begin();

    if (type.empty())
      for (; pos != aidset.end(); ++pos)
	aids.insert((*pos)->getId());
    else
      for (; pos != aidset.end(); ++pos)
	if ((*pos)->getType() == type)
	  aids.insert((*pos)->getId());

    return aids;

  }
  catch (AGException e) {
    logError("GetIncomingAnnotationSet",e);
  }
}

set<AnnotationId>
GetIncomingAnnotationSetByType(AnchorId anchorId, AnnotationType type)
{
  try {
    AnnotationSet aidset = Identifiers::getAGRef(Identifiers::getNamespace(anchorId))->getIncomingAnnotationSetByType(Identifiers::getAnchorRef(anchorId), type);

    set<AnnotationId> aids;
    for (AnnotationSet::iterator pos = aidset.begin(); pos != aidset.end(); ++pos)
      aids.insert((*pos)->getId());
    return aids;

  }
  catch (AGException e) {
    logError("GetIncomingAnnotationSetByType",e);
  }
}

set<AnnotationId>
GetOutgoingAnnotationSet(AnchorId anchorId, const AnnotationType& type)
{
  try {
    set<AnnotationId> aids;
    AnnotationSet aidset =
      Identifiers::getAGRef(Identifiers::getNamespace(anchorId))
      ->getOutgoingAnnotationSet(Identifiers::getAnchorRef(anchorId));
    AnnotationSet::iterator pos = aidset.begin();

    if (type.empty())
      for (; pos != aidset.end(); ++pos)
	aids.insert((*pos)->getId());
    else
      for (; pos != aidset.end(); ++pos)
	if ((*pos)->getType() == type)
	  aids.insert((*pos)->getId());

    return aids;

  }
  catch (AGException e) {
    logError("GetOutgoingAnnotationSet",e);
  }
}

set<AnnotationId>
GetOutgoingAnnotationSetByType(AnchorId anchorId, AnnotationType type)
{
  try {
    AnnotationSet aidset = Identifiers::getAGRef(Identifiers::getNamespace(anchorId))->getOutgoingAnnotationSetByType(Identifiers::getAnchorRef(anchorId), type);

    set<AnnotationId> aids;
    for (AnnotationSet::iterator pos = aidset.begin(); pos != aidset.end(); ++pos)
      aids.insert((*pos)->getId());
    return aids;

  }
  catch (AGException e) {
    logError("GetOutgoingAnnotationSet",e);
  }
}

//// Index ////
 
list<AnchorId>
GetAnchorSet(AGId agId)
{
  try {
    AnchorSeq* aidset = Identifiers::getAGRef(agId)->getAnchorSet();
    list<AnchorId> aids;

    for (AnchorSeq::iterator pos = aidset->begin(); pos != aidset->end(); ++pos)
      aids.push_back((*pos)->getId());
    return aids;

  } catch (AGException e) {
    logError("GetAnchorSet",e);
  }
}

list<AnchorId>
GetAnchorSetByOffset(AGId agId,Offset offset,double epsilon)
{
  try {
    AnchorSet aidset = Identifiers::getAGRef(agId)->getAnchorSetByOffset(offset,epsilon);
    list<AnchorId> aids;
  
    for (AnchorSet::iterator pos = aidset.begin(); pos != aidset.end(); ++pos)
      aids.push_back((*pos)->getId());
    return aids;

  } catch (AGException e) {
    logError("GetAnchorSetByOffset",e);
  }
}

set<AnchorId>
GetAnchorSetNearestOffset(AGId agId,Offset offset)
{
  try {
    AnchorSet aidset = Identifiers::getAGRef(agId)->getAnchorSetNearestOffset(offset);
    set<AnchorId> aids;
  
    for (AnchorSet::iterator pos = aidset.begin(); pos != aidset.end(); ++pos)
      aids.insert((*pos)->getId());
    return aids;

  } catch (AGException e) {
    logError("GetAnchorSetNearestOffset",e);
  }
}

set<AnnotationId>
GetAnnotationSet(const AGId& agId, const AnnotationType& annotationType)
{
  try {
    set<AnnotationId> aids;
    if (! annotationType.empty()) {
      AnnotationSet aidset = Identifiers::getAGRef(agId)->getAnnotationSetByType(annotationType);
  
      for (AnnotationSet::iterator pos = aidset.begin(); pos != aidset.end(); ++pos)
	aids.insert((*pos)->getId());
      return aids;
    }
    else {
      IdSet& idSet = Identifiers::getAGRef(agId)->getAnnotationSet();
      for (IdSet::iterator pos=idSet.begin(); pos != idSet.end(); ++pos)
	aids.insert(*pos);
      return aids;
    }
  } catch (AGException e) {
    logError("GetAnnotationSet",e);
  }
}


set<AnnotationId>
GetAnnotationSetByFeature(AGId agId,
			  FeatureName featureName,
			  FeatureValue featureValue,
			  const AnnotationType& type)
{
  try {
    set<AnnotationId> aids;
    AnnotationSet aidset =
      Identifiers::getAGRef(agId)
      ->getAnnotationSetByFeature(featureName,featureValue);
    AnnotationSet::iterator pos = aidset.begin();

    if (type.empty())
      for (; pos != aidset.end(); ++pos)
	aids.insert((*pos)->getId());
    else
      for (; pos != aidset.end(); ++pos)
	if ((*pos)->getType() == type)
	  aids.insert((*pos)->getId());

    return aids;

  }
  catch (AGException e) {
    logError("GetAnnotationSetByFeature",e);
  }
}

list<AnnotationId>
GetAnnotationSetByOffset(AGId agId, Offset offset, const AnnotationType& type)
{
  try {
    list<AnnotationId> aids;
    AnnotationSet aidset =
      Identifiers::getAGRef(agId)
      ->getAnnotationSetByOffset(offset);
    AnnotationSet::iterator pos = aidset.begin();

    if (type.empty())
      for (; pos != aidset.end(); ++pos)
	aids.push_back((*pos)->getId());
    else
      for (; pos != aidset.end(); ++pos)
	if ((*pos)->getType() == type)
	  aids.push_back((*pos)->getId());

    return aids;

  }
  catch (AGException e) {
    logError("GetAnnotationSetByOffset",e);
  }
}

list<AnnotationId>
GetAnnotationSeqByOffset(AGId agId,
			 Offset begin,
			 Offset end,
			 const AnnotationType& type)
{
  try {
    list<AnnotationId> aids;
    AnnotationSeq aseq =
      Identifiers::getAGRef(agId)->getAnnotationSeqByOffset(begin,end);
    AnnotationSeq::iterator pos = aseq.begin();

    if (type.empty())
      for (; pos != aseq.end(); ++pos)
	aids.push_back((*pos)->getId());
    else
      for (; pos != aseq.end(); ++pos)
	if ((*pos)->getType() == type)
	  aids.push_back((*pos)->getId());

    return aids;

  }
  catch (AGException e) {
    logError("GetAnnotationSeqByOffset",e);
  }
}


AnnotationId
GetAnnotationByOffset(AGId agId,
		      Offset offset,
		      const AnnotationType& type)
{
  try {
    return
      Identifiers::getAGRef(agId)->getAnnotationByOffset(offset,type)->getId();
  }
  catch (AGException e) {
    logError("GetAnnotationByOffset",e);
  }
}

//// Ids ////
 
 // Id may be AGId, AnnotationId, AnchorId
AGSetId GetAGSetId(Id id) {
  try {
    Id nspace = Identifiers::getNamespace(id);
    if (Identifiers::existsAG(id))
      if (Identifiers::existsAGSet(nspace))
	return nspace;
      else
	throw AGException(id + " is not a valid Id!");
    else if (Identifiers::existsAG(nspace)) {
      if (Identifiers::existsAGSet(nspace = Identifiers::getNamespace(nspace)))
	return nspace;
      else
	throw AGException(id + " is not a valid Id!");
    } else
	throw AGException(id + " is not a valid Id!");
  } catch (AGException e) {
    logError("GetAGSetId",e);
  }
}

// Id may be AnnotationId or AnchorId
AGId GetAGId(Id id) {
  try {
    Id nspace = Identifiers::getNamespace(id);
    if (Identifiers::existsAG(nspace))
      return nspace;
    else
      throw AGException(id + " is not a valid Id!");
  } catch (AGException e) {
    logError("GetAGId",e);
  }
}

// Id may be AGId or SignalId
TimelineId GetTimelineId(Id id) {
  try {
    if (Identifiers::existsAG(id)) {
      Timeline* tl = Identifiers::getAGRef(id)->getTimeline();
      return (tl==NULL) ? "" : tl->getId();
    }
    else if (Identifiers::existsSignal(id))
      return Identifiers::getNamespace(id);
    else
      throw AGException(id + " is not a valid AGId or SignalId!");
  } catch (AGException e) {
    logError("GetTimelineId",e);
  }
}

string toXML(Id id) {
  try {
    if (Identifiers::existsAGSet(id)) {
      return Identifiers::getAGSetRef(id)->toString();
    } else if (Identifiers::existsAG(id))
      return Identifiers::getAGSetRef(Identifiers::getNamespace(id))->toXML(id);
    else
      throw AGException(id + " is not a valid AGId or AGSetId!");
  } catch (AGException e) {
    logError("toXML",e);
  }
}

list<string> StoreSQLs(AGSetId agSetId) {
  try {
    return Identifiers::getAGSetRef(agSetId)->storeSQLs();
  } catch (AGException e) {
    logError("StoreSQLs",e);
  }
}

bool
SPrecedes(const AnnotationId& x, const AnnotationId& y)
{
  set<AnnotationId> marked;
  queue<set<AnnotationId> > Q;
  Q.push(GetOutgoingAnnotationSet(GetEndAnchor(x)));
  while (! Q.empty()) {
    set<AnnotationId>& ids = Q.front();
    set<AnnotationId>::iterator id;
    for (id=ids.begin(); id != ids.end(); ++id) {
      if (marked.insert(*id).second) {
	if (*id == y)
	  return true;
	Q.push(GetOutgoingAnnotationSet(GetEndAnchor(*id)));
      }
    }
    Q.pop();
  }

  return false;
}

static agfio io;

list<AGId>
Load(const string&       format,
     const string&       annotationFile,
     const string&       id,
     map<string,string>* signalInfo,
     map<string,string>* options)
{
/* (( B Patch -- */
	try 
	{
/* -- BT Patch )) */
		return io.load(format, annotationFile, id, signalInfo, options);
/* (( BT Patch -- */
	}  
	catch (agfio::LoadError& ex) 
	{
		throw agfio::LoadError(ex.what());
	}  
	catch (...) 
	{
		throw agfio::LoadError("Unknown Exception in AGAPI::Load, id exists ???");
	}
/* -- BT Patch )) */	
}

string
Store(const string&       format,
      const string&       filename,
      const string&       id,
      map<string,string>* options)
{
  return io.store(format, filename, id, options);
}

string
Store2(const string&       format,
       const string&       filename,
       list<string>* const ids,
       map<string,string>* options)
{
  return io.store(format, filename, ids, options);
}

// Validation

AG* id2ref(Id agId) {
  if (Identifiers::existsAG(agId))
    return Identifiers::getAGRef(agId);
  else
    throw AGException(agId + " is not a valid AG id!");
}

bool CheckAnchorOffsetTotal(Id agId, AnnotationType type) {
  return Validation::checkAnchorOffsetTotal(id2ref(agId), type);
}

bool CheckAnchorOffsetBounded(Id agId, AnnotationType type) {
    return Validation::checkAnchorOffsetBounded(id2ref(agId), type);
}

bool CheckFeatureExists(Id agId, AnnotationType type, FeatureName featureName) {
  return Validation::checkFeatureExists(id2ref(agId), type, featureName);
}

bool CheckFeatureIsAnnotationId(Id agId, AnnotationType type, FeatureName featureName) {
  return Validation::checkFeatureIsAnnotationId(id2ref(agId), type, featureName);
}

bool CheckLinear(Id agId, AnnotationType type) {
  return Validation::checkLinear(id2ref(agId), type);
}

bool CheckConnected(Id agId, AnnotationType type) {
  return Validation::checkConnected(id2ref(agId), type);
}

bool CheckCoextensive(Id agId, AnnotationType type1, AnnotationType type2) {
  return Validation::checkCoextensive(id2ref(agId), type1, type2);
}

bool CheckSpan(Id agId, AnnotationType spanType, AnnotationType spannedType) {
  return Validation::checkSpan(id2ref(agId), spanType, spannedType);
}

/* (( BT Patch -- */
#ifdef WIN32
float exp10f(int n) {
	float r = 1 ;
	int i = 0 ;
	for (i=0; i<n; i++) {
		r*=10 ;
	}
	return r ;
}
#endif

#ifdef __APPLE__
float exp10f(int n) {
	float r = 1 ;
	int i = 0 ;
	for (i=0; i<n; i++) {
		r*=10 ;
	}
	return r ;
}
#endif


float myatof(const char* s)
{
	float f;
	int n=0;
	char* pt;
	f = (float)strtol(s, &pt, 10);
	if ( (*pt == '.' || *pt == ',') ) {
		++pt;
		while ( pt[n] != 0 && isdigit(pt[n]) ) ++n;
		if ( n > 0 ) 
			f += ((float)strtol(pt, NULL, 10) / exp10f(n));
	}
	return f;
}
/* -- (( BT Patch )) */
