// BU.cc: BU corpus loader implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/AGAPI.h>
#include <ag/Paired.h>
#include "BU.h"
#include "XLabelFile.h"
#include "BUpos.h"

/* (( BT Patch -- */
#define atof myatof
/* -- BT Patch ) */

static void
build_base(Paired &P, const string& type, const AGId &agId, 
	   const string &unit, set<SignalId>& signalIds);


void
BU::check_files(const string &prefix)
{
  // some files are not needed to be checked
  const string ext[] = { ".lba", ".lbl", ".wrd", ".brk", ".pos",
			 ".ton", ".msc" };
  ifstream test;
  for (int i=LBA; i < MSC; i++) {
    test.open((prefix + ext[i]).c_str());
    if (test.good())
      exist[i] = true;
    else
      exist[i] = false;
    test.close();
  }

  if (lbl_option && exist[LBL]) {
    exist[LBA] = false;
    exist[LBL] = true;
  }
}

list<AGId>
BU::load(const string& filename,
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
    throw agfio::LoadError("BU:" + msg);
  }

  if (options != NULL && (*options)["base"] == "lbl")
    lbl_option = true;
  else
    lbl_option = false;

  check_files(filename);

  XLabelFile X, wrd, brk;
  BUpos pos;

  // .lba
  set<SignalId> sigSet;
  if (! sigId.empty())
    sigSet.insert(sigId);
  string unit("sec");
  if (exist[LBA] && X.open(filename + ".lba"))
    build_base(X, "lba", agId, unit, sigSet);
  else if (exist[LBL] && X.open(filename + ".lbl"))
    build_base(X, "lbl", agId, unit, sigSet);
  else {
    DeleteAG(agId);
    throw agfio::LoadError("BU:can't access lb? file. Loading failed: "
			   + filename);
  }

  // .wrd, .brk, .pos
  if (wrd.open(filename + ".wrd"))
  {
    brk.open(filename + ".brk");
    pos.open(filename + ".pos");
    build_wbp(wrd, brk, pos, agId);
  }

  // .ton
  if (X.open(filename + ".ton"))
    build_base(X, "ton", agId, unit, sigSet);

  // .msc
  if (X.open(filename + ".msc"))
    build_base(X, "msc", agId, unit, sigSet);

  list<AGId> res;
  res.push_back(agId);
  return res;
}

void
BU::build_wbp(Paired& wrd, Paired& brk, Paired& pos, const AGId& agId)
{
  AnchorId prev_ancrId, ancrId;
  AnnotationId annoId;
  string ref;

  // find the first Anchor or create an Anchor with offset 0.0
  ref = wrd.get_time0();
  brk.get_time0();
  pos.get_time0();
  list<AnchorId> tmp = GetAnchorSetByOffset(agId, atof(ref.c_str()), 0.01);
  if (! tmp.empty())
    prev_ancrId = *tmp.begin();
  else {
    cerr << "WARNING: alignment error found" << endl
	 << "WARNING: .wrd, .brk, .pos files are not loaded" << endl;
    return;
  }
  
  while ((ref = wrd.get_time()) != "") {

    tmp = GetAnchorSetByOffset(agId, atof(ref.c_str()), 0.01);
    if (! tmp.empty())
      ancrId = *tmp.begin();
    else {
      cerr << "WARNING: alignment error found" << endl
	   << "WARNING: .wrd, .brk, .pos annotations will be abandoned" << endl
	   << "WARNING: deleting loaded annotations..." << endl;
      set<AnnotationId> ids = GetAnnotationSet(agId, "wrd");
      for (set<AnnotationId>::iterator id=ids.begin(); id!=ids.end(); ++id)
	DeleteAnnotation(*id);
      return;
    }

    // create an Annotation
    annoId = CreateAnnotation(agId, prev_ancrId, ancrId, "wrd");
    SetFeature(annoId, "label", wrd.get_label());
    
    // need to find a way to avoid good() checks
    if (brk.good())
      SetFeature(annoId, "brk", brk.get_label());
    
    if (pos.good())
      SetFeature(annoId, "pos", pos.get_label());
      
    prev_ancrId = ancrId;  // update prev_ancrId
  }
}


static void
build_base(Paired &P, const string& type, const AGId &agId, 
	   const string &unit, set<SignalId>& signalIds)
{
  RE first("[^ \t]+");
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
