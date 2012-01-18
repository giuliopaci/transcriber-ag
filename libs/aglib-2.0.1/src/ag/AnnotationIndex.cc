// AnnotationIndex.cc: This class maintains all the indexes over annotations.
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/AnnotationIndex.h>


AnnotationIndex::AnnotationIndex() {
  byId.resize(500);
  anchorById.resize(1000);
  byFeature.resize(1000);
  incoming.resize(1000);
  outgoing.resize(1000);
}

AnnotationIndex::~AnnotationIndex() {
  for (AnchorSet::iterator pos = anchorSet.begin(); pos != anchorSet.end(); ++pos) {
    Identifiers::deleteAnchorRef((*pos)->getId());
    delete *pos;
  }
}

// Functions to update the indexes

// Add an anchor into the index
// do nothing if it exists
// otherwise add it
void AnnotationIndex::addAnchor(Anchor* anchor) {
  Offset offset = anchor->getOffset();

  if (anchorSet.find(anchor) == anchorSet.end()) {
    anchorSet.insert(anchor);
    anchorById[anchor->id] = const_cast<Anchor*>(anchor);
  }

  if (anchor->getAnchored()) {
    anchorSetByOffset[offset].insert(anchor);
    
#ifdef _MSC_VER
    {
#endif
    for (AnnotationSet::const_iterator pos = incoming[anchor].begin();
	 pos != incoming[anchor].end(); pos++) {
      byEndAnchorOffset[offset].insert(*pos);
    }
#ifdef _MSC_VER
    }
#endif
    
#ifdef _MSC_VER
    {
#endif
    for (AnnotationSet::const_iterator pos = outgoing[anchor].begin();
	 pos != outgoing[anchor].end(); pos++) {
      byStartAnchorOffset[offset].insert(*pos);
    }
#ifdef _MSC_VER
    }
#endif
  }
  
  return;
}

void AnnotationIndex::deleteAnchor(Anchor* anchor) {
  Offset offset = anchor->getOffset();

  if (anchorSet.find(anchor) == anchorSet.end())
    throw AGException("Anchor not found!");

  anchorSet.erase(anchor);
  anchorById.erase(anchor->id);
  anchorSetByOffset[offset].erase(anchor);

  if (anchor->getAnchored()) {
#ifdef _MSC_VER
    {
#endif
    for (AnnotationSet::iterator pos = incoming[anchor].begin();
	 pos != incoming[anchor].end(); pos++) {
      byEndAnchorOffset[offset].erase(*pos);
      if (byEndAnchorOffset[offset].empty())
	byEndAnchorOffset.erase(offset);
    }
#ifdef _MSC_VER
    }
#endif
    
#ifdef _MSC_VER
    {
#endif
    for (AnnotationSet::iterator pos = outgoing[anchor].begin();
	 pos != outgoing[anchor].end(); pos++) {
      byStartAnchorOffset[offset].erase(*pos);
     if (byStartAnchorOffset[offset].empty())
	byStartAnchorOffset.erase(offset);
    }
#ifdef _MSC_VER
    }
#endif
  }
}

void AnnotationIndex::add(Annotation* a) {
  Anchor* startAnchor = a->getStartAnchor();
  Anchor* endAnchor = a->getEndAnchor();

  byId[a->id] = (Annotation*)a;      // insert arc at this location
  byType[a->type].insert(a);         // index on type

  FeatureMap fm = a->getFeatureMap();     // get the features
  for (FeatureMap::iterator i = fm.begin(); i != fm.end(); i++)
    addFeature(a, i->first, i->second);  // index on this feature

  outgoing[a->start].insert(a);  // start node points to this arc
  incoming[a->end].insert(a);    // end node points to this arc

  addAnchor(a->start);  // add start anchor to the anchorSet
  addAnchor(a->end);    // add end anchor to the anchorSet

}

// TODO: REPORT ERROR IF ANNOTATION IS NOT PRESENT, done
void AnnotationIndex::deleteAnnotation(Annotation* a) {
  // delete start anchor and end anchor
  Anchor* startAnchor = a->getStartAnchor();
  Anchor* endAnchor = a->getEndAnchor();

  // delete annotation from annotation set and index
  byId.erase(a->id);
  byType[a->type].erase(a);

  FeatureMap fm = a->getFeatureMap();
  for (FeatureMap::iterator i = fm.begin(); i != fm.end(); i++)
    deleteFeature(a, i->first);

  if (startAnchor->getAnchored())
    byStartAnchorOffset[startAnchor->getOffset()].erase(a);
  if (startAnchor->getAnchored())
    byEndAnchorOffset[endAnchor->getOffset()].erase(a);

  outgoing[startAnchor].erase(a);
  incoming[endAnchor].erase(a);
}

void AnnotationIndex::addFeature(Annotation* a, const String& f, const String& v) {
    byFeature[make_pair(f,v)].insert(a);   // insert the feature-value pair
}

void AnnotationIndex::deleteFeature(Annotation* a, const String& f) {
  if (a->existsFeature(f)) {
    String v = a->getFeature(f);  // get the value of this feature
    byFeature[make_pair(f,v)].erase(a);     // delete the feature-value pair
  }
}

// Functions to access the indexes

// find the nearest existing offset to a given value
Offset AnnotationIndex::getNearestOffset(Offset offset) {
  if (anchorSetByOffset.empty())
    return 0;

  AnchorSetByOffset::iterator pos = anchorSetByOffset.lower_bound(offset);
  if (pos == anchorSetByOffset.begin())
    return pos->first;
  else if (pos == anchorSetByOffset.end())
    return (--pos)->first;
  else {
    Offset d1 = pos->first - offset;
    Offset d2 = -((--pos)->first - offset); ++pos;
    
    if (d1 < d2)
      return pos->first;
    else
      return (--pos)->first;
  }
}
// Get the annotations that overlap a particular time offset
AnnotationSet AnnotationIndex::getByOffset(Offset o) {
  AnnotationSet greaterStartSet,tempSet,curSet;
  AnnotationSet lessEndSet,spanSet;

  // if the offset is less than 0, return an empty set 
  if (o < 0)
    return spanSet;

  ByOffset::iterator startBound = byStartAnchorOffset.upper_bound(o);
  ByOffset::iterator endBound = byEndAnchorOffset.lower_bound(o);
  
  // union all the sets whose start anchor offset is greater than offset o
  // deal with unanchored anchor, depends on wheather annotations with 
  // anchors should be returned in the return set.
#ifdef _MSC_VER
  {
#endif
  for(ByOffset::iterator pos = byStartAnchorOffset.begin(); pos != startBound; pos++) {
    curSet = pos->second;
    tempSet = greaterStartSet;
    set_union(curSet.begin(),curSet.end(),
	      tempSet.begin(),tempSet.end(),
	      inserter(greaterStartSet,greaterStartSet.end()));
  }
#ifdef _MSC_VER
  }
#endif

  // union all the sets whose end anchor offset is less then offset o
#ifdef _MSC_VER
  {
#endif
  for(ByOffset::iterator pos = endBound; pos != byEndAnchorOffset.end(); pos++) {
    curSet = pos->second;
    tempSet = lessEndSet;
    set_union(curSet.begin(),curSet.end(),
	      tempSet.begin(),tempSet.end(),
	      inserter(lessEndSet,lessEndSet.end()));
  }
#ifdef _MSC_VER
  }
#endif

  // find the span set
  set_intersection(greaterStartSet.begin(),greaterStartSet.end(),
		   lessEndSet.begin(),lessEndSet.end(),
		   inserter(spanSet,spanSet.end()));
  
  return spanSet;

}

Annotation*
AnnotationIndex::getAnnotationByOffset(Offset offset,
				       const AnnotationType& type)
{
  ByOffset::iterator pos = byStartAnchorOffset.upper_bound(offset);
  if (pos == byStartAnchorOffset.begin())
    return NULL;
  else {
    --pos;
    while (pos->second.empty())
      if (pos == byStartAnchorOffset.begin())
	return NULL;
      else
	--pos;

    if (type.empty()) {
      for (AnnotationSet::iterator pos2 = pos->second.begin(); pos2 != pos->second.end(); ++pos2)
	if((*pos2)->getEndAnchor()->getOffset() >= offset)
	  return *pos2;
    }
    else {
      for (AnnotationSet::iterator pos2 = pos->second.begin(); pos2 != pos->second.end(); ++pos2)
	if((*pos2)->getType() == type &&
	   (*pos2)->getEndAnchor()->getOffset() >= offset)
	  return *pos2;
    }
    return NULL;
  }
}
    
bool AnnotationIndex::existsAnnotation(const Id& id) {
  return byId.find(id) != byId.end();
}

bool AnnotationIndex::existsAnchor(const Id& id) {
  return anchorById.find(id) != anchorById.end();
}
