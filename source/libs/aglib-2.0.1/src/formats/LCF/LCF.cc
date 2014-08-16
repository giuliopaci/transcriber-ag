// LCF.cc: LDC Callhome Format loader implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include "LCF.h"
#include "LCFfile.h"
#include <ag/AGException.h>
#include <ag/AGAPI.h>

/* (( BT Patch -- */
#define atof myatof
/* -- BT Patch )) */
	
list<AGId>
LCF::load(const string& filename,
	  const Id& id,
	  map<string,string>* signalInfo,
	  map<string,string>* options)
  throw (agfio::LoadError)
{
  AGSetId agsetId;
  TimelineId timelineId;
  SignalId signalId;
  AGId agId;
  auto_init(id, signalInfo, agsetId, timelineId, signalId, agId);
  set<SignalId> sigSet;
  if (! signalId.empty())
    sigSet.insert(signalId);

  LCFfile lcf(filename);
  if (! lcf.good()) {
    DeleteAG(agId);
    throw agfio::LoadError("LCF:can't open " + lcf.get_filename());
  }

  // Save comment lines of the format if there's any.
  SetFeature(agId, "Header", lcf.get_comment());


  AnchorId ancr1, ancr2;
  AnnotationId anno;

  while (lcf.read_record()) {
    if (signalInfo != NULL) {
      ancr1 = CreateAnchor(agId, atof(lcf.get_ith(0).c_str()),
			   (*signalInfo)["unit"], sigSet);
      ancr2 = CreateAnchor(agId, atof(lcf.get_ith(1).c_str()),
			   (*signalInfo)["unit"], sigSet);
    }
    else {
      ancr1 = CreateAnchor(agId);
      SetAnchorOffset(ancr1, atof(lcf.get_ith(0).c_str()));
      ancr2 = CreateAnchor(agId);
      SetAnchorOffset(ancr2, atof(lcf.get_ith(1).c_str()));
    }
     
    anno = CreateAnnotation(agId, ancr1, ancr2, "LCF");
    SetFeature(anno, "TURN", lcf.get_ith(2));
    SetFeature(anno, "TRANSCRIPTION", lcf.get_ith(3));
  }

  list<AGId> res;
  res.push_back(agId);
  return res;
}

string
LCF::store(const string& filename,
	   list<string>* const ids,
	   map<string,string>* options)
  throw (agfio::StoreError)
{
  // open a file
  ofstream output(filename.c_str());
  if (!output)
    throw agfio::StoreError("LCF:store():can't open " +
			    filename + "for writing");

  list<AnnotationId> annos;
  list<AnnotationId>::iterator annop;

  for (list<AGId>::iterator pos = ids->begin(); pos != ids->end(); ++pos) {

    const AGId& id = *pos;

    if (ExistsFeature(id, "Header"))
      output << GetFeature(id, "Header");

    annos = GetAnnotationSeqByOffset(id);

#ifdef _MSC_VER
    Offset s, e;
#endif
    for (annop=annos.begin(); annop != annos.end(); ++annop) {
      const AnnotationId& anno = *annop;

#ifdef _MSC_VER
      s = GetStartOffset(anno);
      e = GetEndOffset(anno);
      output << s;
      output << " ";
      output << e;
#else
      output << GetStartOffset(anno)
	     <<	" "
	     << GetEndOffset(anno);
#endif

      if (ExistsFeature(anno, "TURN")) {
	output << " "
	       << GetFeature(anno, "TURN") << ":";
      }

      if (ExistsFeature(anno, "TRANSCRIPTION")) {
	output << " "
	       << GetFeature(anno, "TRANSCRIPTION");
      }

      output << endl;
    }
  }

  return "";
}

string
LCF::store(const string& filename,
           const Id& id,
           map<string,string>* options)
  throw (agfio::StoreError)
{
  if (ExistsAG(id)) {
    list<AGId> l;
    l.push_back(id);
    return store(filename, &l, options);
  }
  else if (ExistsAGSet(id)) {
    list<AGId> l;
    set<AGId> s = GetAGIds(id);
    for (set<AGId>::iterator pos=s.begin(); pos!=s.end(); ++pos)
      l.push_back(*pos);
    return store(filename, &l, options);
  }
  else {
    throw agfio::StoreError("LCF format:store:no object by the id, " + id);
  }
}

