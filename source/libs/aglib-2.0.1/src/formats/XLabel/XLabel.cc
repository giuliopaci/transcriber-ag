// XLabel.cc: XLabel loader implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/AGAPI.h>
#include "XLabel.h"
#include "XLabelFile.h"

/* (( BT Patch -- */
#define atof myatof
/* -- BT Patch )) */

list<AGId>
XLabel::load(const string& filename,
	     const Id& id,
	     map<string,string>* signalInfo,
	     map<string,string>* options)
  throw (agfio::LoadError)
{
  AGSetId agsetId;
  TimelineId tlId;
  SignalId sigId;
  AGId agId;
  try {
    auto_init(id, signalInfo, agsetId, tlId, sigId, agId);
  }
  catch (const string& msg) {
    throw agfio::LoadError("XLabel:" + msg);
  }
  set<SignalId> sigSet;
  if (! sigId.empty())
    sigSet.insert(sigId);

  // loading
  XLabelFile X(filename);
  if (! X.good()) {
    DeleteAG(agId);
    throw agfio::LoadError("XLabel:can't open " + X.get_filename());
  }

  AnchorId     prev_ancrId, ancrId;
  AnnotationId annoId;
  string       ref;
  string       unit;
  string       ann_type;

  if (signalInfo != NULL)
    unit = (*signalInfo)["unit"];

  if (options != NULL) {
    ann_type = (*options)["ann_type"];
    if (ann_type.empty())
      ann_type = "object";
  }

  ref = X.get_time0();
  if (!sigSet.empty())
    prev_ancrId = CreateAnchor(agId, atof(ref.c_str()), unit, sigSet);
  else {
    prev_ancrId = CreateAnchor(agId);
    SetAnchorOffset(prev_ancrId, atof(ref.c_str()));
  }
  
  while ((ref = X.get_time()) != "") {
    if (!sigSet.empty())
      ancrId = CreateAnchor(agId, atof(ref.c_str()), unit, sigSet);
    else {
      ancrId = CreateAnchor(agId);
      SetAnchorOffset(ancrId, atof(ref.c_str()));
    }
    
    annoId = CreateAnnotation(agId, prev_ancrId, ancrId, ann_type);
    
    SetFeature(annoId, "label", X.get_label());
    prev_ancrId = ancrId;
  }

  list<AGId> res;
  res.push_back(agId);
  return res;
}
