// Identifiers.cc: Identifiers generates unique identifiers for anchors
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/Identifiers.h>


inline static string int60(int n)
{
  static char* CHAR_MAP = 
/* (( BT Patch -- */
    //"0123456789ABCDFGHIJKLMNOPQRSUVWXYZabcdefghijklmnopqrstuvwxyz";
  	(char*)"0123456789ABCDFGHIJKLMNOPQRSUVWXYZabcdefghijklmnopqrstuvwxyz";
/* -- BT Patch )) */
  string tmp;
  while (n >= 60) {
    tmp += CHAR_MAP[n%60];
    n /= 60;
  }
  tmp += CHAR_MAP[n];
  string out;
  string::reverse_iterator pos;
  for (pos=tmp.rbegin(); pos != tmp.rend(); ++pos)
    out += *pos;
  return out;
}

AGSetRefs Identifiers::agSetRefs;
AGRefs Identifiers::agRefs;
TimelineRefs Identifiers::timelineRefs;  
SignalRefs Identifiers::signalRefs;
AnnotationRefs Identifiers::annotationRefs(500);
AnchorRefs Identifiers::anchorRefs(1000);

Identifiers::Identifiers(const Id& nameSpace, string type)
  : nameSpace(nameSpace),
    type(type)
{
  cur = 1;
}

const Id
Identifiers::new_id()
{
  string id = nameSpace.empty()
              ? type + int60(cur)
              : nameSpace + ":" + type + int60(cur);

  // look for an unused identifier
  while (issued.find(id) != issued.end()) { // present?
    cur++;
    id =  nameSpace.empty() 
          ? type + int60(cur)
          : nameSpace + ":" + type + int60(cur);
  }

  issued.insert(id);
  return id;
}

// try to use the provided id as a new identifier
const Id Identifiers::new_id(const Id& id) throw (AGException) { 
  // Test if the namespace of the Identfiers and the given id agree
  if(nameSpace != getNamespace(id))
    throw AGException(id + " is not a member in namespace" + nameSpace);

  if (issued.find(id) != issued.end())  // present?
    return new_id();   // used already, return a new one
  else {
    issued.insert(id); // otherwise just insert it
    return id;
  }
}

bool Identifiers::existsId(const Id& id) {
  return issued.find(id) != issued.end();
}

void Identifiers::reclaim_id(const Id& id) {
  if (issued.find(id) != issued.end())
    issued.erase(id);
  return;
}

Id Identifiers::getNamespace(const Id& id) {
  string::size_type idx;

  idx = id.find_last_of(":");
  if (idx == string::npos)
    return "";
  else
    return id.substr(0,idx);
}

AGSet* Identifiers::getAGSetRef(const AGSetId& id) {
  if (agSetRefs.find(id) == agSetRefs.end())
    throw AGException(id + " is an invalid AGSetId");
  else
    return agSetRefs[id];
}

AG* Identifiers::getAGRef(const AGId& id) {
  if (agRefs.find(id) == agRefs.end())
    throw AGException(id + " is an invalid AGId");
  else
    return agRefs[id];
}

Timeline* Identifiers::getTimelineRef(const TimelineId& id) {
  if (timelineRefs.find(id) == timelineRefs.end())
    throw AGException(id + " is an invalid TimelineId");
  else
    return timelineRefs[id];
}

Signal* Identifiers::getSignalRef(const SignalId& id) {
  if (signalRefs.find(id) == signalRefs.end())
    throw AGException(id + " is an invalid SignalId");
  else
    return signalRefs[id];
}

Annotation* Identifiers::getAnnotationRef(const AnnotationId& id) {
  if (annotationRefs.find(id) == annotationRefs.end())
    throw AGException(id + " is an invalid AnnotationId");
  else
    return annotationRefs[id];
}

Anchor* Identifiers::getAnchorRef(const AnchorId& id) {
  if (anchorRefs.find(id) == anchorRefs.end())
    throw AGException(id + " is an invalid AnchorId");
  else
    return anchorRefs[id];
}

void Identifiers::deleteAGSetRef(const AGSetId& id) {
  agSetRefs.erase(id);
}

void Identifiers::deleteAGRef(const AGId& id) {
  agRefs.erase(id);
}

void Identifiers::deleteTimelineRef(const TimelineId& id) {
  timelineRefs.erase(id);
}

void Identifiers::deleteSignalRef(const SignalId& id) {
  signalRefs.erase(id);
}

void Identifiers::deleteAnnotationRef(const AnnotationId& id) {
  annotationRefs.erase(id);
}

void Identifiers::deleteAnchorRef(const AnchorId& id) {
  anchorRefs.erase(id);
}

void Identifiers::addAGSetRef(const AGSetId& id, AGSet* ref) {
  agSetRefs.insert(make_pair(id,ref));
}

void Identifiers::addAGRef(const AGId& id, AG* ref) {
  agRefs.insert(make_pair(id,ref));
}

void Identifiers::addTimelineRef(const TimelineId& id, Timeline* ref) {
  timelineRefs.insert(make_pair(id,ref));
}

void Identifiers::addSignalRef(const SignalId& id, Signal* ref) {
  signalRefs.insert(make_pair(id,ref));
}

void Identifiers::addAnnotationRef(const AnnotationId& id, Annotation* ref) {
  annotationRefs.insert(make_pair(id,ref));
}

void Identifiers::addAnchorRef(const AnchorId& id, Anchor* ref) {
  anchorRefs.insert(make_pair(id,ref));
}

bool Identifiers::existsAGSet(const AGSetId& id) {
  return agSetRefs.find(id) != agSetRefs.end();
}

bool Identifiers::existsAG(const AGId& id) {
  return agRefs.find(id) != agRefs.end();
}

bool Identifiers::existsTimeline(const TimelineId& id) {
  return timelineRefs.find(id) != timelineRefs.end();
}

bool Identifiers::existsSignal(const SignalId& id) {
  return signalRefs.find(id) != signalRefs.end();
}

bool Identifiers::existsAnnotation(const AnnotationId& id) {
  return annotationRefs.find(id) != annotationRefs.end();
}

bool Identifiers::existsAnchor(const AnchorId& id) {
  return anchorRefs.find(id) != anchorRefs.end();
}
