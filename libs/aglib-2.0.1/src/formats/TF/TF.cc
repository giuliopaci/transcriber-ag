// TF.cc: Table Format loader implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <cstdio>
#include <fstream>
#include <ag/AGAPI.h>
#include <ag/RE.h>
#include <ag/AGException.h>
#include "TF.h"
#include "TFfile.h"

/* (( BT Patch -- */
#define atof myatof
/* -- BT Patch )) */

static string
ftoa(double x)
{
//  stringstream ss;
//  ss << x;
//  return ss.str();
  char s[36];
  sprintf(s, "%.15e", x);
  return string(s);
}

list<AGId>
TF::load(const string& filename,
	 const Id& id,
	 map<string,string>* signalInfo,
	 map<string,string>* options)
  throw (agfio::LoadError)
{
  AGSetId agsetId;
  TimelineId tlId;
  SignalId sigId;
  AGId agId;
  auto_init(id, signalInfo, agsetId, tlId, sigId, agId);
  set<SignalId> sigSet;
  if (! sigId.empty())
    sigSet.insert(sigId);

  ////////////////////
  /// option check ///
  ////////////////////

  if ((*options)["header"] != "" && (*options)["separator"] != "")
    set_header((*options)["header"], (*options)["separator"]);
  else
    throw agfio::LoadError("TF: header and separator must be specified");

  if ((*options)["ann_type"] != "")
    annotation_type = (*options)["ann_type"];
  else
    annotation_type = "TF";

  /// END ///


  TFfile tf(header.size(), separator, filename);
  if (! tf.good()) {
    DeleteAG(agId);
    throw agfio::LoadError("TF:can't open " + tf.get_filename());
  }

  AnchorId a1, a2;
  AnnotationId anno;
  string unit;
  if (signalInfo != NULL)
    unit = (*signalInfo)["unit"];

  while (tf.read_record()) {
    if (signalInfo != NULL) {
      a1 = CreateAnchor(agId, atof(tf.get_ith(0).c_str()), unit, sigSet);
      a2 = CreateAnchor(agId, atof(tf.get_ith(1).c_str()), unit, sigSet);
    }
    else {
      a1 = CreateAnchor(agId);
      SetAnchorOffset(a1, atof(tf.get_ith(0).c_str()));
      a2 = CreateAnchor(agId);
      SetAnchorOffset(a2, atof(tf.get_ith(1).c_str()));
    }

    anno = CreateAnnotation(agId, a1, a2, annotation_type);
      
    for (int i=2; i < header.size(); i++)
      SetFeature(anno, header[i], tf.get_ith(i));
  }

  list<AGId> res;
  res.push_back(agId);
  return res;
}


void
TF::set_header(const string& header, const string& separator)
{
  this->header.clear();

  int i = 0;
  int j = header.find(separator);
  while (j != string::npos) {
    this->header.push_back(header.substr(i, j-i));
    i = j + separator.length();
    j = header.find(separator, i);
  }
  this->header.push_back(header.substr(i));

  this->separator = separator;
}


string
TF::store(const string& filename,
	  list<string>* const ids,
	  map<string,string>* options)
  throw (agfio::StoreError)
{
  // open a file
  ofstream output(filename.c_str());
  if (!output.good())
    throw agfio::StoreError("TF:can't open " + filename + " for writing");

  // check options
  if (options != NULL) {
    if (options->find("header") != options->end() &&
	options->find("separator") != options->end()) {
      set_header((*options)["header"], (*options)["separator"]);
    }
  }

  list<AnnotationId> annos;
  list<AnnotationId>::iterator annop;

  for (list<AGId>::iterator pos=ids->begin(); pos!=ids->end(); ++pos) {
    annos = GetAnnotationSeqByOffset(*pos);

#ifdef _MSC_VER
    Offset s, e;
#endif
    for (annop=annos.begin(); annop != annos.end(); ++annop) {
      const AnnotationId& anno = *annop;
#ifdef _MSC_VER
      // because of possible bugs in stlport or vc++
      s = GetStartOffset(anno);
      e = GetEndOffset(anno);
      output << s;
      output << " ";
      output << e;
#else
      output << GetStartOffset(anno)
	     <<	separator
	     << GetEndOffset(anno);
#endif

      for (int i=2; i < header.size(); i++) {
	if (ExistsFeature(anno, header[i])) {
	  output << separator
		 << GetFeature(anno, header[i]);
	}
	else
	  output << separator;
      }
      output << endl;
    }
  }

  return "";
}

string
TF::store(const string& filename,
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
    throw agfio::StoreError("TF format:store:no object by the id, " + id);
  }
}

