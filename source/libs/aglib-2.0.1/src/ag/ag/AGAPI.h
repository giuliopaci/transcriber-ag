// AGAPI.h: AG API
// Author: Xiaoyi Ma, Steven Bird, Haejoong Lee
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef AGAPI_h
#define AGAPI_h

#include <string>
#include <list>
#include <set>
#include <map>
#include <ag/AGTypes.h>

using namespace std;

//string chomp (string s);

#ifdef _MSC_VER
#define DllExport __declspec( dllexport )
#else
#define DllExport
#endif

//// AGSet ////
DllExport AGSetId CreateAGSet(AGSetId id);
DllExport bool ExistsAGSet(AGSetId id);
DllExport void DeleteAGSet(AGSetId id);
 
// Id is AGSetId or AGId
DllExport AGId CreateAG(Id id, TimelineId timelineId);
DllExport bool ExistsAG(AGId agId); 
DllExport void DeleteAG(AGId agId);
DllExport set<AGId> GetAGIds(AGSetId agSetId);
 
 
//// Signals ////
// Id may be AGSetId or TimelineId
DllExport TimelineId CreateTimeline(Id id);
DllExport bool ExistsTimeline(TimelineId timelineId);
DllExport void DeleteTimeline(TimelineId timelineId);
 
// Id may be TimelineId or SignalId
DllExport SignalId CreateSignal(Id id,URI uri,MimeClass mimeClass,MimeType mimeType,
		      Encoding encoding,Unit unit,Track track);
DllExport bool ExistsSignal(SignalId signalId);
DllExport void DeleteSignal(SignalId signalId);
DllExport set<SignalId> GetSignals(TimelineId timelineId);

DllExport MimeClass GetSignalMimeClass(SignalId signalId); 
DllExport MimeType GetSignalMimeType(SignalId signalId); 
DllExport Encoding GetSignalEncoding(SignalId signalId); 
DllExport string GetSignalXlinkType(SignalId signalId); 
DllExport string GetSignalXlinkHref(SignalId signalId); 
DllExport string GetSignalUnit(SignalId signalId);
DllExport Track GetSignalTrack(SignalId signalId);

//// Annotation ////

// Id may be AGId or AnnotationId
DllExport AnnotationId CreateAnnotation(Id id,AnchorId anchorId1,AnchorId anchorId2,AnnotationType annotationType);
DllExport bool ExistsAnnotation(AnnotationId annotationId);
DllExport void DeleteAnnotation(AnnotationId annotationId);
DllExport AnnotationId CopyAnnotation(AnnotationId annotationId);
DllExport list<AnnotationId> SplitAnnotation(AnnotationId annotationId);
DllExport list<AnnotationId> NSplitAnnotation(AnnotationId annotationId,short N);
DllExport AnnotationType GetAnnotationType(AnnotationId annotationId);
DllExport String GetAnnotationInfo(AnnotationId annotationId);
DllExport AnchorId GetStartAnchor(AnnotationId annotationId);
DllExport AnchorId GetEndAnchor(AnnotationId annotationId);
DllExport void SetStartAnchor(AnnotationId annotationId,AnchorId anchorId);
DllExport void SetEndAnchor(AnnotationId annotationId,AnchorId anchorId);

DllExport Offset GetStartOffset(AnnotationId annotationId);
DllExport Offset GetEndOffset(AnnotationId annotationId);
DllExport void SetStartOffset(AnnotationId annotationId,Offset offset);
DllExport void SetEndOffset(AnnotationId annotationId,Offset offset);

// Id may be AGSetId, AGId or AnnotationId
DllExport set<AnnotationType> GetAnnotationTypes(Id id);
DllExport set<FeatureName>
GetAnnotationFeatureNames(Id id, const AnnotationType& type="");

// this might be necessary to package up an id into a durable reference
DllExport AnnotationRef GetRef(Id id);
 
 
//// Features ////
 
// this is for both the content of an annotation, and for the metadata
// associated with AGSets, AGs, Timelines and Signals.
 
DllExport void SetFeature(Id id,FeatureName featureName,FeatureValue featureValue);
DllExport bool ExistsFeature(Id id, FeatureName featureName);
DllExport void DeleteFeature(Id id, FeatureName featureName);
DllExport string GetFeature(Id id, FeatureName featureName);
DllExport void UnsetFeature(Id id,FeatureName featureName);

DllExport set<FeatureName>
GetFeatureNames(Id id);

DllExport void
SetFeatures(Id id, map<string,string>& features);

DllExport map<string,string>
GetFeatures(Id id);

DllExport void UnsetFeatures(Id id);
 
 
//// List-Valued Features ////
 
// ?? do we need to permit features to have list values?
 
 
//// Anchor ////
 
 // Id may be AGId or AnchorId
DllExport AnchorId CreateAnchor(Id id,Offset offset,Unit unit,set<SignalId>& signalIds);
DllExport AnchorId CreateAnchor(Id id, set<SignalId>& signalId);
DllExport AnchorId CreateAnchor(Id id);
DllExport bool ExistsAnchor(AnchorId anchorId);
DllExport void DeleteAnchor(AnchorId anchorId);
DllExport void SetAnchorOffset(AnchorId anchorId,Offset offset);
DllExport Offset GetAnchorOffset(AnchorId anchorId);
DllExport void SetOffsetUnit(AnchorId anchorId,Unit unit);
DllExport Unit GetOffsetUnit(AnchorId anchorId);
DllExport void SetAnchorSignalIds(AnchorId anchorId,set<SignalId>& signalIds);
DllExport set<SignalId> GetAnchorSignalIds(AnchorId anchorId);
DllExport bool GetAnchored(AnchorId anchorId);
DllExport void UnsetAnchorOffset(AnchorId anchorId);
DllExport AnchorId SplitAnchor(AnchorId anchorId);
 
DllExport set<AnnotationId> GetIncomingAnnotationSet(AnchorId anchorId, const AnnotationType& type="");
DllExport set<AnnotationId> GetOutgoingAnnotationSet(AnchorId anchorId, const AnnotationType& type="");

//// Index ////
 
DllExport list<AnchorId> GetAnchorSet(AGId agId);
DllExport list<AnchorId> GetAnchorSetByOffset(AGId agId,Offset offset, double epsilon=0.0);
DllExport set<AnchorId> GetAnchorSetNearestOffset(AGId agId,Offset offset);
DllExport set<AnnotationId> GetAnnotationSet(const AGId& agId,const AnnotationType& type="");
DllExport set<AnnotationId> GetAnnotationSetByFeature(AGId agId,FeatureName featureName, FeatureValue featureValue, const AnnotationType& type="");
/**
 * Get all annotations with its start anchor offset in between the specified values.
 * Get all annotations with its start anchor offset in between the specified values.
 * If both values are 0, return all annotations in the AG.
 * @param begin, the lower bound, default is 0.0.
 * @param end, the upper bound, default is 0.0.
 * @return all qualified annotations, sorted by start anchor offset.
 **/
DllExport list<AnnotationId> GetAnnotationSetByOffset(AGId agId,Offset offset, const AnnotationType& type="");
DllExport list<AnnotationId> GetAnnotationSeqByOffset(AGId agId, Offset begin=0.0, Offset end=0.0, const AnnotationType& type="");
DllExport AnnotationId GetAnnotationByOffset(AGId agId,Offset offset, const AnnotationType& type="");
 
//// Ids ////
 
 // Id may be AGId, AnnotationId, AnchorId
DllExport AGSetId GetAGSetId(Id id="");

// Id may be AnnotationId or AnchorId
DllExport AGId GetAGId(Id id);

// Id may be AGId or SignalId
DllExport TimelineId GetTimelineId(Id id);

// Id maybe AGSetId or AGId
DllExport string toXML(Id id);

// load an AGSet from db, returns true if succeed, 
// false if it cann't connect to the server
DllExport bool LoadFromDB(string connStr, AGSetId agsetId);

// Store the AGSet to db, returns true if succeed, 
// false if it cann't connect to the server
DllExport bool StoreToDB(string connStr, AGSetId agSetId);

// Return SQL queries needed to store the kernel to the database server
DllExport list<string> StoreSQLs(AGSetId agSetId);

// structural precedes
DllExport bool SPrecedes(const AnnotationId& x, const AnnotationId& y);

// file io
DllExport list<AGId>
Load(const string&       format,
     const string&       annotationFile,
     const string&       id = "",
     map<string,string>* signalInfo = NULL,
     map<string,string>* options = NULL);

DllExport string
Store(const string&       format,
      const string&       filename,
      const string&       id,
      map<string,string>* options = NULL);

DllExport string
Store2(const string&       format,
       const string&       filename,
       list<string>* const ids,
       map<string,string>* options = NULL);


// validation

DllExport bool
CheckAnchorOffsetTotal(Id agId,
                       AnnotationType type);
DllExport bool
CheckAnchorOffsetBounded(Id agId,
                       AnnotationType type);

DllExport bool
CheckFeatureExists(Id agId, AnnotationType type, FeatureName featureName);

DllExport bool
CheckFeatureIsAnnotationId(Id agId, AnnotationType type, FeatureName featureName);

DllExport bool
CheckLinear(Id agId, AnnotationType type);

DllExport bool
CheckConnected(Id agId, AnnotationType type);

DllExport bool
CheckCoextensive(Id agId, AnnotationType type1, AnnotationType type2);

DllExport bool
CheckSpan(Id agId, AnnotationType spanType, AnnotationType spannedType);

/*(( BT Patch -- */
DllExport float
myatof(const char* s);
/*-- BT Patch )) */
#endif
