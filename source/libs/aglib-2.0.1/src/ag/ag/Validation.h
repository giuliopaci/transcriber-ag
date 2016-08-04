// Validation.h: Validate the content and structure of an AG
// Author: Steven Bird
// Copyright (C) 2002 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

#ifndef Validate_h
#define Validate_h

#include <string>
#include <iostream>

#include <ag/AGTypes.h>
#include <ag/AG.h>
#include <ag/Anchor.h>
#include <ag/Annotation.h>
#include <ag/AGException.h>
#include <ag/Identifiers.h>

class Validation {
public:

/**
 * Check that all anchors of annotations of a given type have an offset.
 * @param ag, an annotation graph
 * @param type, an annotation type
 * @return boolean indicating whether all anchors have an offset
 **/
static bool checkAnchorOffsetTotal(AG *ag, AnnotationType type);

/**
 * Check that all anchors of annotations of a given type are bounded
 * by anchors that have an offset, following paths of this type.
 * @param ag, an annotation graph
 * @param type, an annotation type
 * @return boolean indicating whether all anchors are bounded
 **/
static bool checkAnchorOffsetBounded(AG *ag, AnnotationType type);

/**
 * Check that all annotations of a given type have the specified feature.
 * @param ag, an annotation graph
 * @param type, an annotation type
 * @param featureName, the name of the feature
 * @return boolean indicating whether all annotations have the feature
 **/
static bool checkFeatureExists(AG *ag, AnnotationType type, FeatureName featureName);

/**
 * Check that all annotations of a given type have the specified feature and
 * that its value is a valid AnnotationId.
 * @param ag, an annotation graph
 * @param type, an annotation type
 * @param featureName, the name of the feature
 * @return boolean indicating whether all instances of the feature are AnnotationIds
 **/
static bool checkFeatureIsAnnotationId(AG *ag, AnnotationType type, FeatureName featureName);

/**
 * Check that all annotations of a given type form a connected linear sequence.
 * @param ag, an annotation graph
 * @param type, an annotation type
 * @return boolean indicating whether the set of annotations is linear
 **/
static bool checkLinear(AG *ag, AnnotationType type);

/**
 * Check that all annotations of a given type form a connected subgraph.
 * @param ag, an annotation graph
 * @param type, an annotation type
 * @return boolean indicating whether the set of annotations is connected
 **/
static bool checkConnected(AG *ag, AnnotationType type);

/**
 * Check that all annotations of type type1 are coextensive with an
 * annotation of type type2, and vice versa (an existence not uniqueness test).
 * @param ag, an annotation graph
 * @param type1, an annotation type
 * @param type2, an annotation type
 * @return boolean indicating whether the types are coextensive
 **/
static bool checkCoextensive(AG *ag, AnnotationType type1, AnnotationType type2);

/**
 * Check that all annotations of type spanType span annotations of
 * type spannedType and that all annotations of type spannedType are
 * spanned by annotations of type spanType.
 * @param ag, an annotation graph
 * @param spanType, the spanning type
 * @param spanedType, the spanned type
 * @return boolean indicating whether the spanning relationship holds
 **/
static bool checkSpan(AG *ag, AnnotationType spanType, AnnotationType spannedType);

};

#endif
