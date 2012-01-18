// tree_others.cc: high level treebanking functions
// Haejoong Lee, Steven Bird, Scott Cotton
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

#include <ag/AGTypes.h>
#include <ag/AGException.h>
#include <ag/agtree.h>
#include <string>
#include <list>
#include <set>
using namespace std;

///////////////////////////
// treebanking functions //
///////////////////////////

list<AnnotationId>
tree_path(AnnotationId x)
{
  list<AnnotationId> path;
  path.push_front(x);
  x = tree_parent(x);
  while (!x.empty()) {
    path.push_front(x);
    x = tree_parent(x);
  }
  return path;
}

AnnotationId
tree_root(AnnotationId x, int depth)
{
  list<AnnotationId> path = tree_path(x);
  if (path.size() < depth) {
    string msg = "tree_root: ";
    throw AGException(msg + "too deep!");
  }

  list<AnnotationId>::iterator pos = path.begin();
  for (; depth > 0; --depth) ++pos;
  return *pos;
}

AnnotationId
tree_common_ancestor(const AnnotationId& x, const AnnotationId& y)
{
  if (x == y)
    return x;

  list<AnnotationId> xpath = tree_path(x);
  list<AnnotationId> ypath = tree_path(y);
  list<AnnotationId>::iterator xi = xpath.begin();
  list<AnnotationId>::iterator yi = ypath.begin();
  list<AnnotationId>::iterator i = xi;

  if (*xi++ != *yi++)
    return "";  // different trees

  while (*xi == *yi && xi != xpath.end() && yi != ypath.end()) {
    i = xi;
    ++xi, ++yi;
  }
  return *i;
}

AnnotationId
tree_insert_node(const AnnotationId& x,
		 const AnnotationId& y,
		 const AnnotationType& type,
		 const AnnotationId& annoId)
{
  bool SPrecedes(const AnnotationId&, const AnnotationId&);

  string msg = "insert_node: ";

  if (tree_parent(x) != tree_parent(y))
    throw AGException(msg+"the given annotations are not in the same level");

  if (!SPrecedes(x, y))
    throw AGException(msg+"the first annotation doesn't precede the second");

  // make a list of nodes to demote
  set<AnnotationId> ids;
  AnnotationId id = x;
  while ((id=tree_right(id)) != y)
    ids.insert(id);
  ids.insert(id);

  tree_move_down(x, type, annoId);
  for (set<AnnotationId>::iterator pos=ids.begin(); pos != ids.end(); ++pos)
    tree_demote_left(*pos);

  return tree_parent(x);
}

bool
tree_delete_node(const AnnotationId& x)
{
  string msg = "delete_node: ";

  list<AnnotationId> ids = tree_children(x);
  list<AnnotationId>::iterator id;

  if (ids.empty())
    return false;

  for (id=ids.begin(); id != ids.end(); ++id)
    if (!tree_promote_left(*id)) {
      if (!tree_move_up(*id))
	throw AGException(msg+"cat't move the rightmost child up???");
      break;
    }

  return true;
}

