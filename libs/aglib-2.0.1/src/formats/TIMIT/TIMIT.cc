// TIMIT.cc: load() method implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <iostream>
#include <string>
#include <fstream>
#include <ag/AGAPI.h>
#include "TIMIT.h"
#include "TIMITfile.h"

/* (( BT Patch -- */
#define atof myatof
/* -- BT Patch )) */

static void
build_base(Paired &P,
	   const string& type,
	   const AGId &agId, 
	   const string& unit,
	   set<SignalId>& signalIds);

list<AGId>
TIMIT::load(const string& filename,
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
    throw agfio::LoadError("TIMIT:" + msg);
  }
  set<SignalId> emptySet;
  set<SignalId> sigSet;
  if (! sigId.empty())
    sigSet.insert(sigId);

  TIMITfile T;
  string prefix = filename;

  // .phn
  if (T.open(prefix + ".phn")) {
    if (signalInfo != NULL)
      build_base(T, "phn", agId, (*signalInfo)["unit"], sigSet);
    else
      build_base(T, "phn", agId, "", emptySet);
  }
  else
  {
    DeleteAG(agId);
    throw agfio::LoadError("TIMIT:can't open " + T.get_filename());
  }

  // .wrd
  if (T.open(prefix + ".wrd")) {
    if (signalInfo != NULL)
      add_annotation(T, "wrd", agId, (*signalInfo)["unit"], sigSet);
    else
      add_annotation(T, "wrd", agId, "", emptySet);
  }

  // .txt
  if (T.open(prefix + ".txt")) {
    if (signalInfo != NULL)
      add_annotation(T, "txt", agId, (*signalInfo)["unit"], sigSet);
    else
      add_annotation(T, "txt", agId, "", emptySet);
  }

  list<AGId> res;
  res.push_back(agId);
  return res;
}


void
TIMIT::add_annotation(Paired &T, const string& type, const AGId &agId,
                      const Unit& unit, set<SignalId>& signalIds)
{
  AnchorId prev_ancrId, ancrId;
  AnnotationId annoId;
  string ref;

  // find the first anchor
  // if impossible, create new one
  ref = T.get_time0();
  list<AnchorId> tmp = GetAnchorSetByOffset(agId, atoi(ref.c_str()), 5.0);
  if (! tmp.empty())
    prev_ancrId = *tmp.begin();
  else {
    if (! signalIds.empty())
      prev_ancrId = CreateAnchor(agId, atof(ref.c_str()), unit, signalIds);
    else {
      prev_ancrId = CreateAnchor(agId);
      SetAnchorOffset(prev_ancrId, atof(ref.c_str()));
    }
  }

  while ((ref = T.get_time()) != "")
  {
    // find an anchor
    // if impossible, create new one
    tmp = GetAnchorSetByOffset(agId, atof(ref.c_str()), 5.0);
    if (! tmp.empty())
      ancrId = *tmp.begin();
    else {
      if (! signalIds.empty())
	ancrId = CreateAnchor(agId, atof(ref.c_str()), unit, signalIds);
      else {
	ancrId = CreateAnchor(agId);
	SetAnchorOffset(ancrId, atof(ref.c_str()));
      }
    }

    annoId = CreateAnnotation(agId, prev_ancrId, ancrId, type);
    SetFeature(annoId, "label", T.get_label());

    prev_ancrId = ancrId;  // update prev_ancrId
  }
}

static void
build_base(Paired &P, const string& type, const AGId &agId, 
	   const string &unit, set<SignalId>& signalIds)
{
  AnchorId prev_ancrId, ancrId;
  AnnotationId annoId;
  string ref;

  ref = P.get_time0();
  if (! signalIds.empty())
    prev_ancrId = CreateAnchor(agId, atof(ref.c_str()), unit, signalIds);
  else {
    prev_ancrId = CreateAnchor(agId);
    SetAnchorOffset(prev_ancrId, atof(ref.c_str()));
  }

  while ((ref = P.get_time()) != "") {
    if (! signalIds.empty())
      ancrId = CreateAnchor(agId, atof(ref.c_str()), unit, signalIds);
    else {
      ancrId = CreateAnchor(agId);
      SetAnchorOffset(ancrId, atof(ref.c_str()));
    }

    annoId = CreateAnnotation(agId, prev_ancrId, ancrId, type);
      
    SetFeature(annoId, "label", P.get_label());
    prev_ancrId = ancrId;
  }
}
