// SwitchBoard.cc: SwitchBoard loader class implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <iostream>
#include <string>
#include <fstream>
#include <ag/AGAPI.h>
#include "SwitchBoard.h"
#include "SWBfile.h"
/* (( BT Patch -- */
#define atof myatof
/* -- BT Patch ) */

list<AGId>
SwitchBoard::load(const string& filename,
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
    throw agfio::LoadError("SwitchBoard:" + msg);
  }
  set<SignalId> sigSet;
  if (! sigId.empty())
    sigSet.insert(sigId);

  SWBfile S;

  if (!S.open(filename)) {
    DeleteAG(agId);
    throw agfio::LoadError("SwitchBoard:can't open " + filename);
  }

  AnchorId prev_ancrId, ancrId, turn_ancrId;
  AnnotationId annoId;
  string time, label;

  if (signalInfo != NULL)
    prev_ancrId = CreateAnchor(agId, atof(S.get_time0().c_str()),
			       (*signalInfo)["unit"], sigSet);
  else {
    prev_ancrId = CreateAnchor(agId);
    SetAnchorOffset(prev_ancrId, atof(S.get_time0().c_str()));
  }

  turn_ancrId = prev_ancrId;

  while (S.read_record())
  {
    time = S.get_time();

    if (signalInfo != NULL) {
      ancrId = (time.at(0) == '-') ?
	CreateAnchor(agId, sigSet) :
	CreateAnchor(agId, atof(time.c_str()),
		     (*signalInfo)["unit"], sigSet);
    }
    else {
      ancrId = CreateAnchor(agId);
      if (time.at(0) != '-')
	SetAnchorOffset(ancrId, atof(time.c_str()));
    }
      
    annoId = CreateAnnotation(agId, prev_ancrId, ancrId, "wrd");
    SetFeature(annoId, "label", S.get_label());

    if (S.turn_over()) {
      annoId = CreateAnnotation(agId, turn_ancrId, ancrId, "turn");
      SetFeature(annoId, "speaker", S.get_spkr());
      SetFeature(annoId, "utterance", S.get_uttn());

      if ((time = S.get_next_time()) != "") {
	if (signalInfo != NULL) {
	  ancrId = (time.at(0) == '-') ?
	    CreateAnchor(agId, sigSet) :
	    CreateAnchor(agId, atof(time.c_str()),
			 (*signalInfo)["unit"], sigSet);
	}
	else {
	  ancrId = CreateAnchor(agId);
	  if (time.at(0) != '-')
	    SetAnchorOffset(ancrId, atof(time.c_str()));
	}
	turn_ancrId = ancrId;
      }
    }

    prev_ancrId = ancrId;
  }

  list<AGId> res;
  res.push_back(agId);
  return res;
}
