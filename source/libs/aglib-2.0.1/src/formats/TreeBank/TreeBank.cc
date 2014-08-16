// TreeBank.cc: TreeBank loader implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <set>
#include <fstream>
#include <cstdio>
#include <ag/AGAPI.h>
#include <ag/agtree.h>
#include <ag/RE.h>
#include "TreeBank.h"
#include "TBparser.h"
#include "TBhandler.h"

static string
itoa(int x)
{
  char s[36];
  sprintf(s, "%d", x);
  return string(s);
}

list<AGId>
TreeBank::load(const string& filename,
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
    throw agfio::LoadError("TreeBank:" + msg);
  }

  TBhandler handler(agId, *escHandler);
  TBparser parser(handler);
  bool is_string = false;
  if (options != NULL && (*options)["input type"] == "string")
    is_string = true;

  try {
    parser.parse(filename, is_string);
  }
  catch (TBparser::ParseError e) {
    DeleteAG(agId);
    throw agfio::LoadError("TreeBank:parsing " + filename + " failed");
  }

  list<AGId> res;
  res.push_back(agId);
  return res;
}


void
TreeBank::output_tree(ostream& OUT,
		      const AnnotationId& node,
		      const string& indent)
{
  static RE headtail("^[ \t]*([^ \t]+)(.*)$");

  list<AnnotationId> children = tree_children(node);
  list<AnnotationId>::iterator child;
  AnnotationType annoType = GetAnnotationType(node);

  if (children.empty()) {       // terminal node
    if (annoType == "syn") {    // trace (terminal && "syn")
      OUT << endl << indent
	  << "(-NONE- " << GetFeature(node,"trace_type");
      if (ExistsFeature(node,"trace_ref"))
	OUT << "-" << GetFeature(node,"trace_ref");
      OUT << ")";
    }
    else {                      // word (terminal && not "syn")
      string label = escHandler->encode("",GetFeature(node, "label"));
      OUT << " " << label;
    }
  }
  else {                        // internal node
    string label="";
    if (ExistsFeature(node,"label"))
      label = escHandler->encode("",GetFeature(node, "label"));
    OUT << endl << indent
	<<"(" << label;
    if (ExistsFeature(node, "roles")) {
      string roles = GetFeature(node, "roles");
      while (headtail.match(roles)) {
	OUT << "-" << headtail.get_matched(1);
	roles = headtail.get_matched(2);
      }
    }
    if (ExistsFeature(node,"trace_ref")) {
      OUT << "-" << GetFeature(node, "trace_ref");
    }

    for (child=children.begin(); child != children.end(); ++child)
      output_tree(OUT, *child, indent+"  ");

    OUT << ")";
  }
}

void
TreeBank::output_forest(ostream& OUT, const AGId& agId)
{
  AnnotationId aRoot = tree_first_tree(agId);
  for (; !aRoot.empty(); aRoot=tree_right(aRoot))
    output_tree(OUT, aRoot);
}

string
TreeBank::store(const string& filename,
		list<string>* const ids,
		map<string,string>* options)
  throw (agfio::StoreError)
{
  // open a file
  ofstream output(filename.c_str());
  if (!output)
    throw agfio::StoreError("TreeBank::store():can't open " +
			    filename + "for writing");

  for (list<AGId>::iterator pos=ids->begin(); pos != ids->end(); ++pos)
    output_forest(output, *pos);

  return "";
}

string
TreeBank::store(const string& filename,
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
    throw agfio::StoreError("TreeBank format:store:no object by the id, " + id);
  }
}

