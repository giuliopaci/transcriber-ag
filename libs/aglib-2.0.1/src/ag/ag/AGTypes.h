// AGTypes.h: Declarations of AG Types
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef AGTypes_h
#define AGTypes_h

#include <string>

using namespace std;

typedef string String;
typedef string Id;              // generic identifier
// Id can be from any of AGSetId, AGId, AnnotationId, TimelineId, SignalId

typedef string AGSetId;         // AGSet identifier
typedef string AGId;            // AG identifier
typedef string AnnotationId;    // Annotation identifier
typedef string AnnotationType;  // Annotation type
typedef string AnchorId;        // Anchor identifier
typedef string TimelineId;      // Timeline identifier
typedef string SignalId;        // Signal identifier
typedef string FeatureName;     // feature name
typedef string FeatureValue;    // feature value
typedef string URI;             // a uniform resource identifier
typedef string MimeClass;       // the MIME class
typedef string MimeType;        // the MIME type
typedef string Encoding;        // the signal encoding
typedef string Unit;            // the unit for offsets
typedef string Track;           // the track
typedef string AnnotationRef;   // an annotation reference

typedef double Offset;

#endif
