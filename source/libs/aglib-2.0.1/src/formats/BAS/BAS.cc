// BAS.cc: BAS Partitur format loader implementation.
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <iostream>
#include <string>
#include <fstream>
#include <ag/AGAPI.h>
#include "BAS.h"


BAS::BAS()
{
  funcMap["KAN"] = &BAS::ag_build1;
  funcMap["ORT"] = &BAS::ag_build1;
  funcMap["TRL"] = &BAS::ag_build1TR;
  funcMap["TR2"] = &BAS::ag_build1TR;
  funcMap["SUP"] = &BAS::ag_build1SUP;
  funcMap["PHO"] = &BAS::ag_build4;
  funcMap["SAP"] = &BAS::ag_build4Gab;
  funcMap["MAU"] = &BAS::ag_build4MAU;
  funcMap["WOR"] = &BAS::ag_build4WOR;
  funcMap["DAS"] = &BAS::ag_build1;
  funcMap["PRB"] = &BAS::ag_build5_begin;
  funcMap["PRS"] = &BAS::ag_build1;
  funcMap["NOI"] = &BAS::ag_build1NOI;
}

list<AGId>
BAS::load(const string& filename,
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
    throw agfio::LoadError("BAS:" + msg);
  }

  BASfile B;

  if (!B.open(filename)) {
    DeleteAG(agId);
    throw agfio::LoadError("BAS:can't open " + filename);
  }

  set<SignalId> sigSet;
  if (!sigId.empty())
    sigSet.insert(sigId);
  string unit = GetSignalUnit(sigId);
  while (B.read_record())
    (this->*funcMap[B.get_type()])(B, agId, unit, sigSet);


  //
  // Merge MAU into class 1
  //
  AnnotationId prev_anno, anno;
  AnchorId a, x;
  int i = 0;
  string ref;

  // find the first (non -1) annotation of MAU
  set<AnnotationId> tmp = GetAnnotationSet(agId, "MAU");
  prev_anno = tmp.empty() ? "" : *tmp.begin();
  while (prev_anno != "") {
    anno = prev_anno;
    tmp = GetIncomingAnnotationSet(GetStartAnchor(anno));
    prev_anno = tmp.empty() ? "" : *tmp.begin();
  }

  while (GetFeature(anno, "bas_ref") == "-1") {
    prev_anno = anno;
    tmp = GetOutgoingAnnotationSet(GetEndAnchor(anno));
    anno = tmp.empty() ? "" : *tmp.begin();
  }

  while (anno != "") {
    ref = GetFeature(anno, "bas_ref");

    if (ref == "-1") {
      x = anchorPool["class1"][i];
      anchorPool["class1"][i] = SplitAnchor(x);
    }
    else if (atoi(ref.c_str()) != i) {
      prev_anno = anno;
      tmp = GetOutgoingAnnotationSet(GetEndAnchor(anno));
      anno = tmp.empty() ? "" : *tmp.begin();
      continue;
    }
    else {
      x = anchorPool["class1"][i];
      i++;
    }

    a = GetStartAnchor(anno);

    SetEndAnchor(prev_anno, x);
    SetStartAnchor(anno, x);
    
    SetStartOffset(anno, GetAnchorOffset(a));

    DeleteAnchor(a);

    prev_anno = anno;
    tmp = GetOutgoingAnnotationSet(GetEndAnchor(anno));
    anno = tmp.empty() ? "" : *tmp.begin();
  }

  list<AGId> res;
  res.push_back(agId);
  return res;
}

void
BAS::ag_build1(BASfile& B,
	       const AGId& agId,
	       const string& unit,
	       set<SignalId>& signalIds)
{
  static RE head("[0-9]+");
  static RE tail("[0-9]+$");
  string ref = B.get_ref();

  head.match(ref);
  tail.match(ref);
  
  int ref1 = atoi(head.get_matched(0).c_str());
  int ref2 = atoi(tail.get_matched(0).c_str()) + 1;

  AnchorId ancr1, ancr2;

  if ((ancr1 = anchorPool["class1"][ref1]) == "") {
    if (! signalIds.empty())
      ancr1 = CreateAnchor(agId, -1, unit, signalIds);
    else {
      ancr1 = CreateAnchor(agId);
      SetAnchorOffset(ancr1, -1);
    }
    anchorPool["class1"][ref1] = ancr1;
  }

  if ((ancr2 = anchorPool["class1"][ref2]) == "") {
    if (! signalIds.empty())
      ancr2 = CreateAnchor(agId, 0, unit, signalIds);
    else {
      ancr2 = CreateAnchor(agId);
      SetAnchorOffset(ancr2, 0);
    }
    anchorPool["class1"][ref2] = ancr2;
  }

  AnnotationId anno = CreateAnnotation(agId, ancr1, ancr2, B.get_type());
  SetFeature(anno, "label", B.get_label());
  SetFeature(anno, "bas_ref", ref);
}


void
BAS::ag_build1TR(BASfile& B,
		 const AGId& agId,
		 const string& unit,
		 set<SignalId>& signalIds)
{
  static RE head("[0-9]+");
  static RE tail("[0-9]+$");
  string ref = B.get_ref();

  head.match(ref);
  tail.match(ref);
  
  int ref1 = atoi(head.get_matched(0).c_str());
  int ref2 = atoi(tail.get_matched(0).c_str());

  AnchorId ancr1, ancr2;
  AnnotationId anno;
  string type = B.get_type();

  if (ref1 == ref2 && (anno = annotationPool[type][ref1]) != "")
  {
    anno = *(++SplitAnnotation(anno).begin());
    anno = anno.substr(anno.rfind(' ') + 1);  // get the second annotation
    ancr1 = GetStartAnchor(anno);             // which is newly created
    UnsetAnchorOffset(ancr1);
    annotationPool[type][ref1] = anno;
  }
  else
  // ref1 != ref2  -->  it's not the case to split the annotation
  // no annotation -->  there's no annotation to split  
  {
    ref2++;
    
    if ((ancr1 = anchorPool["class1"][ref1]) == "") {
      if (! signalIds.empty())
	ancr1 = CreateAnchor(agId, -1, unit, signalIds);
      else {
	ancr1 = CreateAnchor(agId);
	SetAnchorOffset(ancr1, -1);
      }
      anchorPool["class1"][ref1] = ancr1;
    }

    if ((ancr2 = anchorPool["class1"][ref2]) == "") {
      if (! signalIds.empty())
	ancr2 = CreateAnchor(agId, -1, unit, signalIds);
      else {
	ancr2 = CreateAnchor(agId);
	SetAnchorOffset(ancr2, -1);
      }
      anchorPool["class1"][ref2] = ancr2;
    }

    anno = CreateAnnotation(agId, ancr1, ancr2, type);

    if (ref1+1 == ref2)
      annotationPool[type][ref1] = anno;
  }
	
  SetFeature(anno, "label", B.get_label());
  SetFeature(anno, "bas_ref", ref);
}


void
BAS::ag_build1SUP(BASfile& B,
		  const AGId& agId,
		  const string& unit,
		  set<SignalId>& signalIds)
{
  static RE head("^[0-9]+");
  static RE tail("[0-9]+$");
  string ref = B.get_ref();

  head.match(ref);
  tail.match(ref);

  int ref1 = atoi(head.get_matched(0).c_str());
  int ref2 = atoi(tail.get_matched(0).c_str()) + 1;

  AnchorId ancr1, ancr2;

  if ((ancr1 = anchorPool["class1"][ref1]) == "") {
    if (! signalIds.empty())
      ancr1 = CreateAnchor(agId, -1, unit, signalIds);
    else {
      ancr2 = CreateAnchor(agId);
      SetAnchorOffset(ancr2, -1);
    }
    anchorPool["class1"][ref1] = ancr1;
  }

  if ((ancr2 = anchorPool["class1"][ref2]) == "") {
    if (! signalIds.empty())
      ancr2 = CreateAnchor(agId, -1, unit, signalIds);
    else {
      ancr2 = CreateAnchor(agId);
      SetAnchorOffset(ancr2, -1);
    }
    anchorPool["class1"][ref2] = ancr2;
  }

  AnnotationId anno = CreateAnnotation(agId, ancr1, ancr2, B.get_type());

  static RE suplbl("([^ \t]+)[ \t]+(.*)");
  suplbl.match(B.get_label());
  SetFeature(anno, "label", suplbl.get_matched(2));
  SetFeature(anno, "utterance_id", suplbl.get_matched(1));
  SetFeature(anno, "bas_ref", ref);  
}


void
BAS::ag_build1NOI(BASfile& B,
		  const AGId& agId,
		  const string& unit,
		  set<SignalId>& signalIds)
{
  static RE head("^[0-9]+");
  static RE tail("[0-9]+$");
  string ref = B.get_ref();

  head.match(ref);
  tail.match(ref);

  int ref1 = atoi(head.get_matched(0).c_str());
  int ref2 = atoi(tail.get_matched(0).c_str());

  AnchorId ancr1, ancr2;

  if (ref1 == ref2)
  {
    ref2++;
    
    if ((ancr1 = anchorPool["class1"][ref1]) == "") {
      if (! signalIds.empty())
	ancr1 = CreateAnchor(agId, -1, unit, signalIds);
      else {
	ancr1 = CreateAnchor(agId);
	SetAnchorOffset(ancr1, -1);
      }
      anchorPool["class1"][ref1] = ancr1;
    }
  }

  else  // if (ref1 != ref2)
  {
    ref1++;
  }

  if ((ancr2 = anchorPool["class1"][ref2]) == "") {
    if (! signalIds.empty())
      ancr2 = CreateAnchor(agId, -1, unit, signalIds);
    else {
      ancr2 = CreateAnchor(agId);
      SetAnchorOffset(ancr2, -1);
    }
    anchorPool["class1"][ref2] = ancr2;
  }

  AnnotationId anno = CreateAnnotation(agId, ancr1, ancr2, B.get_type());
  SetFeature(anno, "label", B.get_label());
  SetFeature(anno, "bas_ref", ref);
}


void
BAS::ag_build4(BASfile& B,
	       const AGId& agId,
	       const string& unit,
	       set<SignalId>& signalIds)
{
  int time1 = atoi(B.get_time().c_str());
  int time2 = time1 + atoi(B.get_dur().c_str());
  string type = B.get_type();
  AnchorId ancr1, ancr2;
  
  if ((ancr1 = anchorPool[type][time1]) == "") {
    if (! signalIds.empty())
      ancr1 = CreateAnchor(agId, time1, unit, signalIds);
    else {
      ancr1 = CreateAnchor(agId);
      SetAnchorOffset(ancr1, time1);
    }
    anchorPool[type][time1] = ancr1;
  }

  if ((ancr2 = anchorPool[type][time2]) == "") {
    if (! signalIds.empty())
      ancr2 = CreateAnchor(agId, time2, unit, signalIds);
    else {
      ancr2 = CreateAnchor(agId);
      SetAnchorOffset(ancr2, time2);
    }
    anchorPool[type][time2] = ancr2;
  }

  AnnotationId anno = CreateAnnotation(agId, ancr1, ancr2, type);
  SetFeature(anno, "label", B.get_label());
  string ref = B.get_ref();
  SetFeature(anno, "bas_ref", ref);
}


void
BAS::ag_build4Gab(BASfile& B,
		  const AGId& agId,
		  const string& unit,
		  set<SignalId>& signalIds)
{
  int time1 = atoi(B.get_time().c_str());
  int time2 = time1 + atoi(B.get_dur().c_str());
  string type = B.get_type();
  AnchorId ancr1, ancr2;
  
  if ((ancr1 = anchorPool[type][time1]) == "") {
    if (! signalIds.empty())
      ancr1 = CreateAnchor(agId, time1, unit, signalIds);
    else {
      ancr1 = CreateAnchor(agId);
      SetAnchorOffset(ancr1, time1);
    }
    anchorPool[type][time1] = ancr1;
    if (prevAnchor[type] != "")
    {
      CreateAnnotation(agId, prevAnchor[type], ancr1, type);
    }
  }

  if ((ancr2 = anchorPool[type][time2]) == "") {
    if (! signalIds.empty())
      ancr2 = CreateAnchor(agId, time2, unit, signalIds);
    else {
      ancr2 = CreateAnchor(agId);
      SetAnchorOffset(ancr2, time2);
    }
    anchorPool[type][time2] = ancr2;
  }

  prevAnchor[type] = ancr2;

  AnnotationId anno = CreateAnnotation(agId, ancr1, ancr2, type);
  SetFeature(anno, "label", B.get_label());
  string ref = B.get_ref();
  SetFeature(anno, "bas_ref", ref);
}


void
BAS::ag_build4WOR(BASfile& B,
		  const AGId& agId,
		  const string& unit,
		  set<SignalId>& signalIds)
{
  int time1 = atoi(B.get_time().c_str());
  int time2 = time1 + atoi(B.get_dur().c_str()) + 1;
  string type = B.get_type();
  AnchorId ancr1, ancr2;
  
  if ((ancr1 = anchorPool[type][time1]) == "") {
    if (! signalIds.empty())
      ancr1 = CreateAnchor(agId, time1, unit, signalIds);
    else {
      ancr1 = CreateAnchor(agId);
      SetAnchorOffset(ancr1, time1);
    }
    anchorPool[type][time1] = ancr1;
    if (prevAnchor[type] != "")
    {
      CreateAnnotation(agId, prevAnchor[type], ancr1, type);
    }
  }

  if ((ancr2 = anchorPool[type][time2]) == "") {
    if (! signalIds.empty())
      ancr2 = CreateAnchor(agId, time2, unit, signalIds);
    else {
      ancr2 = CreateAnchor(agId);
      SetAnchorOffset(ancr2, time2);
    }
    anchorPool[type][time2] = ancr2;
  }

  prevAnchor[type] = ancr2;

  AnnotationId anno = CreateAnnotation(agId, ancr1, ancr2, type);
  SetFeature(anno, "label", B.get_label());
  string ref = B.get_ref();
  SetFeature(anno, "bas_ref", ref);
}


void
BAS::ag_build4MAU(BASfile& B,
		  const AGId& agId,
		  const string& unit,
		  set<SignalId>& signalIds)
{
  int time1 = atoi(B.get_time().c_str());
  int time2 = time1 + atoi(B.get_dur().c_str()) + 1;
  string type = B.get_type();
  AnchorId ancr1, ancr2;
  
  if ((ancr1 = anchorPool[type][time1]) == "") {
    if (! signalIds.empty())
      ancr1 = CreateAnchor(agId, time1, unit, signalIds);
    else {
      ancr1 = CreateAnchor(agId);
      SetAnchorOffset(ancr1, time1);
    }
    anchorPool[type][time1] = ancr1;
  }

  if ((ancr2 = anchorPool[type][time2]) == "") {
    if (! signalIds.empty())
      ancr2 = CreateAnchor(agId, time2, unit, signalIds);
    else {
      ancr2 = CreateAnchor(agId);
      SetAnchorOffset(ancr2, time2);
    }
    anchorPool[type][time2] = ancr2;
  }

  AnnotationId anno = CreateAnnotation(agId, ancr1, ancr2, type);
  SetFeature(anno, "label", B.get_label());
  string ref = B.get_ref();
  SetFeature(anno, "bas_ref", ref);
}


void
BAS::ag_build5_begin(BASfile& B,
		     const AGId& agId,
		     const string& unit,
		     set<SignalId>& signalIds)
{
  string type = B.get_type();
  if (! signalIds.empty())
    prevAnchor[type] = CreateAnchor(agId, 0, unit, signalIds);
  else {
    prevAnchor[type] = CreateAnchor(agId);
    SetAnchorOffset(prevAnchor[type], 0);
  }
  funcMap[type] = &BAS::ag_build5;
  ag_build5(B, agId, unit, signalIds);
}

void
BAS::ag_build5(BASfile& B,
	       const AGId& agId,
	       const string& unit,
	       set<SignalId>& signalIds)
{
  int time = atoi(B.get_time().c_str());
  string type = B.get_type();

  AnchorId ancr;

  if (! signalIds.empty())
    ancr = CreateAnchor(agId, time, unit, signalIds);
  else {
    ancr = CreateAnchor(agId);
    SetAnchorOffset(ancr, time);
  }
  AnnotationId anno = CreateAnnotation(agId, prevAnchor[type], ancr, type);
  prevAnchor[type] = ancr;
  
  string ref = B.get_ref();
  SetFeature(anno, "bas_ref", ref);

  static RE lblist("([A-Z]{3}):[ \t]*([^; \t]+)([ \t]*;[ \t]*(.*))?");
  string label = B.get_label();
  while (lblist.match(label))
  {
    SetFeature(anno, lblist.get_matched(1), lblist.get_matched(2));
    label = lblist.get_matched(4);
  }
}
