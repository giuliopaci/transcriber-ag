// tree_kernel.cc: core treebanking functions
// Haejoong Lee, Steven Bird, Scott Cotton
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

#include <ag/AG.h>
#include <ag/Annotation.h>

#include <ag/AGAPI.h>
#include <ag/AGException.h>
#include <ag/RE.h>
#include <ag/agtree.h>
#include <string>
#include <set>
#include <queue>
using namespace std;

//////////////////////
// helper functions //
//////////////////////

static bool
legal_promote_left(const AnnotationId& x, const AnnotationId& y)
{
  AnnotationId z = GetFeature(x, "parent");
  AnchorId x_start = GetStartAnchor(x);
  while (z != y) {
    if (GetStartAnchor(z) != x_start)
      return false;
    z = GetFeature(z, "parent");
  }
  return true;
}

static bool
legal_promote_right(const AnnotationId& x, const AnnotationId& y)
{
  AnnotationId z = GetFeature(x, "parent");
  AnchorId x_end = GetEndAnchor(x);
  while (z != y) {
    if (GetEndAnchor(z) != x_end)
      return false;
    z = GetFeature(z, "parent");
  }
  return true;
}

//////////////////////
// kernel functions //
//////////////////////

// only kernel functions know how to manipulate ag

AnnotationId
tree_parent(const AnnotationId& x)
{
  return GetFeature(x, "parent");
}

AnnotationId
tree_left(const AnnotationId& x)
{
  queue<AnnotationType> TQ;
  TQ.push("syn");
  TQ.push("pos");
  TQ.push("wrd");
  TQ.push("dummy_root");
  
  AnnotationId x_parent = GetFeature(x, "parent");
  while (! TQ.empty()) {
    AnnotationType& t = TQ.front();
    set<AnnotationId> S = GetIncomingAnnotationSet(GetStartAnchor(x), t);
    set<AnnotationId>::iterator pos = S.begin();
    for (; pos != S.end(); ++pos)
      if (GetFeature(*pos,"parent") == x_parent)
	return *pos;
    TQ.pop();
  }
  return "";
}

AnnotationId
tree_right(const AnnotationId& x)
{
  queue<AnnotationType> TQ;
  TQ.push("syn");
  TQ.push("pos");
  TQ.push("wrd");
  TQ.push("dummy_root");
  
  AnnotationId x_parent = GetFeature(x, "parent");
  while (! TQ.empty()) {
    AnnotationType& t = TQ.front();
    set<AnnotationId> S = GetOutgoingAnnotationSet(GetEndAnchor(x), t);
    set<AnnotationId>::iterator pos = S.begin();
    for (; pos != S.end(); ++pos)
      if (GetFeature(*pos,"parent") == x_parent)
	return *pos;
    TQ.pop();
  }
  return "";
}

list<AnnotationId>
tree_children(const AnnotationId& x)
{
  list<AnnotationId> result;
  map<AnchorId,AnnotationId> m;
  AGId agId = GetAGId(x);
  set<AnnotationId> as = GetAnnotationSetByFeature(agId, "parent", x);
  // make sure that "parent" feature is not used by annotations of
  // types other than "syn", "pos", "wrd", "dummy_root"

  for (set<AnnotationId>::iterator i=as.begin(); i != as.end(); ++i)
    m[GetStartAnchor(*i)] = *i;

  list<AnnotationId>::iterator here;
  map<AnchorId,AnnotationId>::iterator p;
  while (! m.empty()) {
    p = m.begin();
    here = result.begin();
    while (p != m.end()) {
      AnnotationId anno = p->second;    // get an annotation
      result.insert(here, anno);        // insert it at the front
      m.erase(p);                       // no use anymore
      p = m.find(GetEndAnchor(anno));   // find a following annotation
    }
  }

  return result;
}


bool
tree_move_down(const AnnotationId& anno,
	       const AnnotationType& type,
	       AnnotationId new_anno)
{
  if (new_anno.empty()) {
    new_anno = CreateAnnotation(GetAGId(anno),
				GetStartAnchor(anno),
				GetEndAnchor(anno),
				type);
  }
  else {
    CreateAnnotation(new_anno,
		     GetStartAnchor(anno),
		     GetEndAnchor(anno),
		     type);
  }
    
  SetFeature(new_anno, "parent", GetFeature(anno,"parent"));
  SetFeature(anno, "parent", new_anno);

  return true;
}

bool
tree_move_up(const AnnotationId& anno)
{
  AnnotationId parent = GetFeature(anno, "parent");
  if (parent.empty() ||
      GetStartAnchor(parent) != GetStartAnchor(anno) ||
      GetEndAnchor(parent) != GetEndAnchor(anno))
    return false;
  SetFeature(anno, "parent", GetFeature(parent,"parent"));
  DeleteAnnotation(parent);

  return true;
}

bool
tree_promote_right(const AnnotationId& anno)
{
  AnnotationId parent = GetFeature(anno, "parent");
  AnchorId anno_start;

  if (parent.empty() ||
      GetEndAnchor(parent) != GetEndAnchor(anno) ||
      GetStartAnchor(parent) == (anno_start=GetStartAnchor(anno)))
    return false;

  SetEndAnchor(parent, anno_start);
  SetFeature(anno, "parent", GetFeature(parent,"parent"));

  return true;
}

bool
tree_promote_left(const AnnotationId& anno)
{
  AnnotationId parent = GetFeature(anno, "parent");
  AnchorId anno_end;

  if (parent.empty() ||
      GetStartAnchor(parent) != GetStartAnchor(anno) ||
      GetEndAnchor(parent) == (anno_end=GetEndAnchor(anno)))
    return false;

  SetStartAnchor(parent, anno_end);
  SetFeature(anno, "parent", GetFeature(parent,"parent"));

  return true;
}

bool
tree_demote_right(const AnnotationId& anno)
{
  // should have a right sibling that is not of "pos" type
  AnnotationId the_right = tree_right(anno);
  if (the_right.empty() || GetAnnotationType(the_right)=="pos")
    return false;

  SetStartAnchor(the_right, GetStartAnchor(anno));
  SetFeature(anno, "parent", the_right);
  return true;
}

bool
tree_demote_left(const AnnotationId& anno)
{
  // should have a left sibling that is not of "pos" type
  AnnotationId the_left = tree_left(anno);
  if (the_left.empty() || GetAnnotationType(the_left)=="pos")
    return false;

  SetEndAnchor(the_left, GetEndAnchor(anno));
  SetFeature(anno, "parent", the_left);
  return true;
}

AnnotationId
tree_first_tree(const AGId& agId)
{
  set<AnnotationId> ids = GetAnnotationSetByFeature(agId, "parent", "");
  if (ids.empty())
    return "";

  AnnotationId prevId = *ids.begin();
  AnnotationId id;
  while ((id=tree_left(prevId)) != "")
    prevId = id;

  return prevId;
}

AnnotationId
tree_last_tree(const AGId& agId)
{
  set<AnnotationId> ids = GetAnnotationSetByFeature(agId, "parent", "");
  if (ids.empty())
    return "";

  AnnotationId prevId = *ids.begin();
  AnnotationId id;
  while ((id=tree_right(prevId)) != "")
    prevId = id;

  return prevId;
}

AnnotationId
tree_init_tree(const AnnotationType& type, const Id& id)
{
  string msg("init_tree: ");

  // check if the ag id, ag, is in valid format
  RE idPat("^([^:]+)(:([^:]+)(:([^:]+))?)?$");
  if (!idPat.match(id))
    throw AGException(msg + "expecting an agset/ag/annotation id");

  // analysis the given id
  AGSetId agsetId;      bool agset_new = false;
  AGId agId;            bool ag_new = false;
  AnnotationId annoId;
  AnchorId ancrId;      bool ancr_new = false;

  agsetId = idPat.get_matched(1);
  if (!idPat.get_matched(2).empty()) {
    agId = agsetId + ":" + idPat.get_matched(3);
    if (!idPat.get_matched(4).empty()) {
      annoId = agId + ":" + idPat.get_matched(5);
    }
  }

  if (!ExistsAGSet(agsetId)) {
    CreateAGSet(agsetId);
    agset_new = true;
  }

  if (agId.empty()) {
    agId = CreateAG(agsetId, CreateTimeline(agsetId));
    ancrId = CreateAnchor(agId);
    ag_new = ancr_new = true;
  }
  else if (!ExistsAG(agId)) {
    CreateAG(agId, CreateTimeline(agsetId));
    ancrId = CreateAnchor(agId);
    ag_new = ancr_new = true;
  }
  else {
    // find the rightmost anchor
    set<AnnotationId> ids = GetAnnotationSetByFeature(agId,"parent","");
    AnnotationId id;
    if (!ids.empty()) {
      set<AnnotationId>::iterator id = ids.begin();
      for (; id != ids.end(); ++id)
	if (GetOutgoingAnnotationSet(GetEndAnchor(*id)).empty())
	  break;
      if (id == ids.end())
	throw AGException(msg+"found a circle");
      ancrId = GetEndAnchor(*id);
    }
    else {
      ancrId = CreateAnchor(agId);
      ancr_new = true;
    }
  }

  if (annoId.empty()) {
    annoId = CreateAnnotation(agId, ancrId, CreateAnchor(agId), type);
  }
  else if (!ExistsAnnotation(annoId)) {
    CreateAnnotation(annoId, ancrId, CreateAnchor(agId), type);
  }
  else {
    if (agset_new)
      DeleteAGSet(agsetId);
    else if (ag_new)
      DeleteAGSet(agId);
    else if (ancr_new)
      DeleteAnchor(ancrId);
    throw AGException(msg+"given annotation already exists");
  }

  SetFeature(annoId, "parent", "");
  return annoId;
}

AnnotationId
tree_insert_node_left(const AnnotationId& x,
		      const AnnotationType& type,
		      AnnotationId annoId)
{
  AGId agId = GetAGId(x);
  AnchorId a = GetStartAnchor(x);
  AnchorId c = CreateAnchor(agId);
  SetAnchorOffset(c, GetAnchorOffset(a));

  // find set of annotations to be moved
  set<AnnotationId> S = GetOutgoingAnnotationSet(a);
  AnnotationId id = x;
  while ((id=GetFeature(id,"parent")) != "")
    S.erase(id);

  set<AnnotationId>::iterator e;
  for (e=S.begin(); e != S.end(); ++e)
    SetStartAnchor(*e, c);

  if (annoId.empty())
    annoId = CreateAnnotation(agId, a, c, type);
  else
    CreateAnnotation(annoId, a, c, type);
  SetFeature(annoId, "parent", GetFeature(x,"parent"));

  return annoId;
}

AnnotationId
tree_insert_node_right(const AnnotationId& x,
		       const AnnotationType& type,
		       AnnotationId annoId)
{
  AGId agId = GetAGId(x);
  AnchorId b = GetEndAnchor(x);
  AnchorId c = CreateAnchor(agId);
  SetAnchorOffset(c, GetAnchorOffset(b));

  // find set of annotations to be moved
  set<AnnotationId> S = GetIncomingAnnotationSet(b);
  AnnotationId id = x;
  while ((id=GetFeature(id,"parent")) != "")
    S.erase(id);

  set<AnnotationId>::iterator e;
  for (e=S.begin(); e != S.end(); ++e)
    SetEndAnchor(*e, c);

  if (annoId.empty())
    annoId = CreateAnnotation(agId, c, b, type);
  else
    CreateAnnotation(annoId, c, b, type);
  SetFeature(annoId, "parent", GetFeature(x,"parent"));

  return annoId;
}

bool
tree_delete_node_left(const AnnotationId& x)
{
  AnnotationId x_left = tree_left(x);

  if (x_left.empty())
    return false;   // can't find a left sibling
  if (!tree_children(x_left).empty())
    return false;   // the left one is not a terminal

  AnchorId target = GetStartAnchor(x_left);
  AnchorId x_start = GetStartAnchor(x);
  set<AnnotationId> ids = GetOutgoingAnnotationSet(x_start);
  set<AnnotationId>::iterator id = ids.begin();
  for (; id != ids.end(); ++id)
    SetStartAnchor(*id, target);
  DeleteAnnotation(x_left);
  try {
    DeleteAnchor(x_start);
  }
  catch (const AGException& e) {
    cerr << "tree_kernel.cc: tree_delete_node_left: please check" << endl;
  }

  return true;
}
  
bool
tree_delete_node_right(const AnnotationId& x)
{
  AnnotationId x_right = tree_right(x);

  if (x_right.empty())
    return false;   // can't find a right sibling
  if (!tree_children(x_right).empty())
    return false;   // the right one is not a terminal

  AnchorId target = GetEndAnchor(x_right);
  AnchorId x_end = GetEndAnchor(x);
  set<AnnotationId> ids = GetIncomingAnnotationSet(x_end);
  set<AnnotationId>::iterator id = ids.begin();
  for (; id != ids.end(); ++id)
    SetEndAnchor(*id, target);
  DeleteAnnotation(x_right);
  try {
    DeleteAnchor(x_end);
  }
  catch (const AGException& e) {
    cerr << "tree_kernel.cc: tree_delete_node_right: please check" << endl;
  }

  return true;
}

bool
tree_move(const AnnotationId& x, const AnnotationId& y)
{
  AnnotationId x_parent = GetFeature(x, "parent");
  AnchorId x_start = GetStartAnchor(x);
  AnchorId x_end = GetEndAnchor(x);

  // is x an only child?
  if (x_start == GetStartAnchor(x_parent) && x_end == GetEndAnchor(x_parent))
      return false;

  // is there a legal move from x under y?
  // <=> x is no only child and one of the followings holds
  //     * adjacent(Tx,Ty), or
  //     * x is left-promotable upto child(y), or
  //     * x is right-promotable upto child(y)

  // Tx-Ty-adjacent
  if (x_end == GetStartAnchor(y)) {
    while (tree_promote_right(x));
    while (GetFeature(x,"parent") != y)
      tree_demote_right(x);
  }
  // Ty-Tx-adjacent
  else if (x_start == GetEndAnchor(y)) {
    while (tree_promote_left(x));
    while (GetFeature(x,"parent") != y)
      tree_demote_left(x);
  }
  // x is left-promotable upto child(y)
  else if (legal_promote_left(x,y)) {
    while (GetFeature(x,"parent") != y)
      tree_promote_left(x);
  }
  // x is right-promotable upto child(y)
  else if (legal_promote_right(x,y)) {
    while (GetFeature(x,"parent") != y)
      tree_promote_right(x);
  }
  // can't move
  else
    return false;

  return true;
}

