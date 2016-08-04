// Annotation.cc: Validate the content and structure of an AG
// Author: Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

#include <ag/Validation.h>
        
// Utility functions, defined at the end
void markForwards(AG *ag, Annotation *a, AnnotationType type, AnnotationSet& marked);
void markBackwards(AG *ag, Annotation *a, AnnotationType type, AnnotationSet& marked);
void markConnected(AG *ag, Annotation *a, AnnotationType type, AnnotationSet& marked);
bool checkBoundedPath(AG *ag, AnnotationType type, Anchor *start, Anchor *end, AnnotationSet& visited);
bool checkCoterminousAnnotation(AG *ag, Annotation *a, AnnotationType type, AnnotationSet& visited);

void printAS(string s, AnnotationSet as) {
  cout << s << " = {";
  for (AnnotationSet::iterator i = as.begin(); i != as.end(); i++)
    cout << (*i)->getId() << ",";
  cout << "}" << endl;
}

// VALIDATION FOR ANCHORS

// all anchors used by annotations of this type have an offset
// B&L term: totally anchored
bool Validation::checkAnchorOffsetTotal(AG *ag, AnnotationType type) {
  AnnotationSet as = ag->getAnnotationSetByType(type);
  for (AnnotationSet::iterator i = as.begin(); i != as.end(); i++)
    if (! ((*i)->getStartAnchor()->getAnchored())
	&& ((*i)->getEndAnchor()->getAnchored()))
      return false;
  return true;
}

// all anchors used by annotations of this type which lack an offset
// have a path forward and backwards along annotations of this type
// to an anchor having an offset
// B&L term: anchored
bool Validation::checkAnchorOffsetBounded(AG *ag, AnnotationType type) {
  AnnotationSet as = ag->getAnnotationSetByType(type);
  AnnotationSet right, left;
  AnnotationSet::iterator i;
  for (i = as.begin(); i != as.end(); i++) {
    if ((*i)->getStartAnchor()->getAnchored())
      left.insert(*i);
    if ((*i)->getEndAnchor()->getAnchored())
      right.insert(*i);
  }
  for (i = as.begin(); i != as.end(); i++) {
    markForwards(ag, *i, type, right);
    markBackwards(ag, *i, type, left);
  }
  return ((as == right) && (as == left));
}

// VALIDATE CONTENT

// Search annotations of this type to see if all have the feature
bool Validation::checkFeatureExists(AG *ag, AnnotationType type, FeatureName featureName) {
  AnnotationSet as = ag->getAnnotationSetByType(type);
  for (AnnotationSet::iterator i = as.begin(); i != as.end(); i++)
    if (! (*i)->existsFeature(featureName))
      return false;
  return true;  
}

// Search annotations of this type to see if all of them have the
// feature and the value of the feature is always an existing AnnotationId
bool Validation::checkFeatureIsAnnotationId(AG *ag, AnnotationType type, FeatureName featureName) {
  AnnotationSet as = ag->getAnnotationSetByType(type);
  for (AnnotationSet::iterator i = as.begin(); i != as.end(); i++) {
    if (! (*i)->existsFeature(featureName))
      return false;
    AnnotationId aid = (AnnotationId) (*i)->getFeature(featureName);
    if (! Identifiers::existsAnnotation(aid))
      return false;
  }
  return true;  
}

// VALIDATE STRUCTURE

// Check that the annotations of this type form a connected, linear
// sequence with no branching.  We expect that every annotation has a
// unique preceding and following annotation, except for the initial
// and final annotation in the sequence.  There is no need to navigate
// the sequence in order to do the test.
bool Validation::checkLinear(AG *ag, AnnotationType type) {
  AnnotationSet as = ag->getAnnotationSetByType(type);
  bool foundStart = false, foundEnd = false;
  for (AnnotationSet::iterator i = as.begin(); i != as.end(); i++) {
    Anchor* start = (*i)->getStartAnchor();
    Anchor* end = (*i)->getEndAnchor();
    int isize = ag->getIncomingAnnotationSetByType(start, type).size();
    int osize = ag->getOutgoingAnnotationSetByType(end, type).size();
    if ( (isize > 1 || osize > 1) // there is a branch in the path
	 || (foundStart && isize == 0) // >1 annotation lacks a preceeding annotation
	 || (foundEnd && osize == 0) ) // >1 annotation lacks a following annotation
      return false;
    if (isize == 0)
      foundStart = true;
    if (osize == 0)
      foundEnd = true;
  }
  return true;
}

// Check that the annotations of this type are connected.  A set of
// annotations "as" is connected iff by picking any annotation in this
// set, and navigating to all reachable annotations, we visit the
// entire set.
bool Validation::checkConnected(AG *ag, AnnotationType type) {
  AnnotationSet as = ag->getAnnotationSetByType(type);
  Annotation *a = *(as.begin()); // get an annotation
  AnnotationSet visited;
  markConnected(ag, a, type, visited);
  return (visited == as);
}

// Two useful set operations - are these defined elsewhere?

// void Utilities::union(set& s1, const set& s2) {
//  if (s1.size() < s2.size) { set tmp = s1; s1 = s2; s2 = tmp; }
//  for (set::iterator i = s2.begin(); i != s2.end(); i++)
//    s1.insert(*i);
//}
//
//void Utilities::intersection(set& s1, const set& s2) {
//  if (s1.size() < s2.size) { set tmp = s1; s1 = s2; s2 = tmp; }
//  for (set::iterator i = s1.begin(); i != s1.end(); i++)
//    if (s2.find(*i) == s2.end())
//      s1.erase(*i);
//}

// Check that all annotations of these types are coextensive
bool Validation::checkCoextensive(AG *ag, AnnotationType type1, AnnotationType type2) {
  AnnotationSet as1 = ag->getAnnotationSetByType(type1);
  AnnotationSet as2 = ag->getAnnotationSetByType(type2);
  AnnotationSet visited;
  for (AnnotationSet::iterator i = as1.begin(); i != as1.end(); i++)
    if (checkCoterminousAnnotation(ag, *i, type2, visited) == false)
      return false;
  return (as2 == visited);
}

// Check that all annotations of type spanType span a path of
// annotations of type spannedType, and that all annotations of type
// spannedType are spanned.
bool Validation::checkSpan(AG *ag, AnnotationType spanType, AnnotationType spannedType) {
  AnnotationSet as = ag->getAnnotationSetByType(spanType);
  AnnotationSet visited;
  for (AnnotationSet::iterator i = as.begin(); i != as.end(); i++) {
    cout << "Checking " << (*i)->getId() << endl;;
    Anchor* start = (*i)->getStartAnchor();
    Anchor* end = (*i)->getEndAnchor();
    if (! checkBoundedPath(ag, spannedType, start, end, visited))
      return false;
  }
  // ensure that the above process visited every annotation of type spannedType
  if (visited != ag->getAnnotationSetByType(spannedType))
    return false;
  return true;
}

// UTILITY FUNCTIONS


// Search forwards along annotations of this type, marking the reachable ones.
// To be reachable the existing annotation must be marked
void markForwards(AG *ag, Annotation *a, AnnotationType type, AnnotationSet& marked) {
  // has it been visited already?
  if (marked.find(a) != marked.end()) {
    // mark this annotation
    marked.insert(a);
    // get annotations connected to the right
    AnnotationSet as = ag->getOutgoingAnnotationSetByType(a->getEndAnchor(), type);
    for (AnnotationSet::iterator i = as.begin(); i != as.end(); i++)
      markForwards(ag, *i, type, marked);
  }
}

// Search backwards along annotations of this type, marking the reachable ones
// To be reachable the existing annotation must be marked
void markBackwards(AG *ag, Annotation *a, AnnotationType type, AnnotationSet& marked) {
  if (marked.find(a) != marked.end()) {
    marked.insert(a);
    AnnotationSet as = ag->getIncomingAnnotationSetByType(a->getStartAnchor(), type);
    for (AnnotationSet::iterator i = as.begin(); i != as.end(); i++)
      markBackwards(ag, *i, type, marked);
  }
}

// Search forwards and backwards along annotations of this type, marking the reachable ones
void markConnected(AG *ag, Annotation *a, AnnotationType type, AnnotationSet& marked) {
  // skip this node if it was already visited
  if (marked.find(a) == marked.end()) {
    marked.insert(a);
    //printAS("marked", marked);
    AnnotationSet as;
    // find a more efficient way...
    as = ag->getOutgoingAnnotationSetByType(a->getEndAnchor(), type);
	AnnotationSet::iterator i;
    for (i = as.begin(); i != as.end(); i++)
      markConnected(ag, *i, type, marked);
    as = ag->getOutgoingAnnotationSetByType(a->getStartAnchor(), type);
    for (i = as.begin(); i != as.end(); i++)
      markConnected(ag, *i, type, marked);
    as = ag->getIncomingAnnotationSetByType(a->getEndAnchor(), type);
    for (i = as.begin(); i != as.end(); i++)
      markConnected(ag, *i, type, marked);
    as = ag->getIncomingAnnotationSetByType(a->getStartAnchor(), type);
    for (i = as.begin(); i != as.end(); i++)
      markConnected(ag, *i, type, marked);
  }
}

// Test if there is a path of annotations of this type, between the given anchors
bool checkBoundedPath(AG *ag, AnnotationType type, Anchor *start, Anchor *end, AnnotationSet& visited) {
  Anchor* anchor = start;
  do {
    AnnotationSet oas = ag->getOutgoingAnnotationSetByType(anchor, type);
    if (oas.size() != 1)
      return false;
    Annotation *a = *(oas.begin()); // get the unique following annotation
    visited.insert(a);
    cout << "Following " << a->getId() << endl;
    anchor = a->getEndAnchor();
  } while (anchor != end);
  return true;
}

// Does the annotation have coterminous annotations of the given type?
bool checkCoterminousAnnotation(AG *ag, Annotation *a, AnnotationType type, AnnotationSet& visited) {

  AnnotationSet c1 = ag->getOutgoingAnnotationSetByType(a->getStartAnchor(), type);
  AnnotationSet c2 = ag->getIncomingAnnotationSetByType(a->getEndAnchor(), type);

  // compute c1 intersect c2 and save in c1
  AnnotationSet::iterator i;
  for (i = c1.begin(); i != c1.end(); i++)
    if (c2.find(*i) == c2.end()) c1.erase(*i);

  // accumulate the coterminous annotations    
  for (i = c1.begin(); i != c1.end(); i++)
    visited.insert(*i);

  // if c1 is empty then we did not find a coterminous annotation
  return (c1.size() != 0);
}
