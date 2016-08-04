// agtree.h: treebanking functions (declaration)
// Haejoong Lee, Steven Bird, Scott Cotton
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _AGTREE_H_
#define _AGTREE_H_

#include <ag/AGTypes.h>
#include <list>
#include <string>
using namespace std;

#ifdef _MSC_VER
#define DllExport __declspec( dllexport )
#else
#define DllExport
#endif

/// Create an empty tree, a tree with only one node.
/**
 * @param type
 *    Type of the annotation (node) to be created.
 * @param id
 *    Agset, ag or annotation id.  It's an error to give an id of
 *    an existing annotation.  If the specified ag already exists,
 *    new annotation will be attached to the last annotation in
 *    the ag.  If objects (agset, ag, annotation) specified in the
 *    id don't exist, appropriate objects will be created.
 * @return
 *    Id of created annotation (node) upon success.
 */
DllExport AnnotationId
tree_init_tree(const AnnotationType& type, const Id& id="AGTree");

/// Move the given node down, creating a new node above it.
/**
 * @param anno
 *    Id of the annotation (node) to be moved.
 * @param type
 *    Type of the newly created annotation (node).
 * @param new_anno
 *    Specifies the id of created annotation.
 * @return
 *    true on success, false on fail
 */
DllExport bool
tree_move_down(const AnnotationId& anno,
	       const AnnotationType& type="syn",
	       AnnotationId new_anno="");

/// Move the given node up, deleting the parent node.
/**
 * @param anno
 *    Id of the annotation (node) to be moved.  The node must be
 *    an only child.  Otherwise the operation fails.
 * @return
 *    true on success, false on fail
 */
DllExport bool
tree_move_up(const AnnotationId& anno);

/// Make the given node a right sibling of parent node.
/**
 * @param anno
 *    Id of the annotation (node) to be moved.  The node must be
 *    the rightmost child, and it can't be an only child.  Otherwise
 *    the operation fails.
 * @return
 *    true on success, false on fail
 */
DllExport bool
tree_promote_right(const AnnotationId& anno);

/// Make the given node a left sibling of parent node.
/**
 * @param anno
 *    Id of the annotation (node) to be moved.  The node must be
 *    the leftmost child, and it can't be an only child.  Otherwise
 *    the operation fails.
 * @return
 *    true on success, false on fail
 */
DllExport bool
tree_promote_left(const AnnotationId& anno);

/// Make the given node a rightmost child of the left sibling.
/**
 * @param anno
 *    Id of the annotation (node) to be moved.  The node must
 *    have a left sibling.  Otherwise the operation fails.
 * @return
 *    true on success, false on fail
 */ 
DllExport bool
tree_demote_right(const AnnotationId& anno);

/// Make the given node a leftmost child of the right sibling.
/**
 * @param anno
 *    Id of the annotation (node) to be moved.  The node must
 *    have a right sibling.  Otherwise the operation fails.
 * @return
 *    true on success, false on fail
 */
DllExport bool
tree_demote_left(const AnnotationId& anno);

/// Find the first tree (root) in a given annotation graph.
/**
 * @param agId
 *    Id of an annotation graph.
 * @return
 *    The id of the root node of the first tree in the annotation graph.
 */
DllExport AnnotationId
tree_first_tree(const AGId& agId);

/// Find the last tree (root) in a given annotation graph.
/**
 * @param agId
 *    Id of an annotation graph.
 * @return
 *    The id of the root node of the last tree in the annotation graph.
 */
DllExport AnnotationId
tree_last_tree(const AGId& agId);

/// Move a subtree under a certain node
/**
 * @param x
 *    Id of the annotation (node) to be moved.  Actually
 *    the subtree rooted at x is moved.  x must be a leftmost
 *    or rightmost child, and can't be an only child.
 * @param y
 *    x is moved under y.  x should be able to reach under y
 *    by some promote and demote operations.
 * @return
 *    true on success, false on fail
 */
DllExport bool
tree_move(const AnnotationId& x, const AnnotationId& y);

/// Insert a new terminal node on the left.
/**
 * @param x
 *    Id of the annotation (node) on the left of which the
 *    new node is inserted.  x itself should be a terminal
 *    node.
 * @param type
 *    Type of the new annotation (node).
 * @param annoId
 *    Specifies the id of the new annotation (node).
 * @return
 *    Id of the new annotation (node).
 */
DllExport AnnotationId
tree_insert_node_left(const AnnotationId& x,
		      const AnnotationType& type="wrd",
		      AnnotationId annoId="");

/// Insert a new terminal node on the right.
/**
 * @param x
 *    Id of the annotation (node) on the right of which the
 *    new node is inserted.  x itself should be a terminal
 *    node.
 * @param type
 *    Type of the new annotation (node).
 * @param annoId
 *    Specifies the id of the new annotation (node).
 * @return
 *    Id of the new annotation (node).
 */
DllExport AnnotationId
tree_insert_node_right(const AnnotationId& x,
		       const AnnotationType& type="wrd",
		       AnnotationId annoId="");

/// Delete a node on the left.
/**
 * @param x
 *    Id of the annotation (node) whose left sibling is deleted.
 *    x must be a terminal node.
 * @return
 *    true on success, false on fail.
 */
DllExport bool
tree_delete_node_left(const AnnotationId& x);

/// Delete a node on the left.
/**
 * @param x
 *    Id of the annotation (node) whose right sibling is deleted.
 *    x must be a terminal node.
 * @return
 *    true on success, false on fail.
 */
DllExport bool
tree_delete_node_right(const AnnotationId& x);

/// Find the root node.
/**
 * @param x
 *    Id of an annotation (node).  The root of the tree with x will
 *    be searched.
 * @param depth
 *    A positive integer.  Suppose a path from the root to x.
 *    Then the (depth+1)th node in the path will be returned.
 *    This is useful when the root of a sentence and the root
 *    of the tree are different.
 * @return
 *    Id of the root annotation (node).
 */
DllExport AnnotationId
tree_root(AnnotationId x, int depth=1);

/// Find the parent node.
/**
 * @param x
 *    Id of an annotation (node) whose parent will be searched.
 * @return
 *    Id of the parent annotation (node).
 */
DllExport AnnotationId
tree_parent(const AnnotationId& x);

/// Find children.
/**
 * @param x
 *    Id of an annotation (node) whose children will be searched.
 * @return
 *    A list of children from the leftmost to the rightmost child.
 */
DllExport list<AnnotationId>
tree_children(const AnnotationId& x);

/// Find a left sibling.
/**
 * @param x
 *    Id of an annotation (node) whose left sibling will be searched.
 * @return
 *    Id of the left sibling.
 */
DllExport AnnotationId
tree_left(const AnnotationId& x);

/// Find a right sibling
/**
 * @param x
 *    Id of an annotation (node) whose right sibling will be searched.
 * @return
 *    Id of the right sibling.
 */
DllExport AnnotationId
tree_right(const AnnotationId& x);

/// Find a path from the root to the given node.
/**
 * @param x
 *    An annotation (node) id.  The function finds the path from the root
 *    to x.
 * @return
 *    A list of annotations (nodes) in the path from the root.
 */
DllExport list<AnnotationId>
tree_path(AnnotationId x);

/// Find the nearest common ancestor of the two nodes.
/**
 * @param x, y
 *    Annotation (node) ids.
 * @return
 *    Id of the nearest common ancestor of x and y.
 */
DllExport AnnotationId
tree_common_ancestor(const AnnotationId& x, const AnnotationId& y);

/// Insert a non-terminal node.
/**
 * @param x, y
 *    Ids of annotations (nodes) which are children of a node.
 *    It's an error that y precedes x.  A non-terminal will be added
 *    in the following way.  x is moved down, creating a non-terminal x'.
 *    All nodes which are right siblings of x, and left siblings of y,
 *    including y, become the children of x'.
 * @param type
 *    Type of newly created annotation (node).
 * @param annoId
 *    Specifies the id of newly created annotation (node).
 */
DllExport AnnotationId
tree_insert_node(const AnnotationId& x,
                 const AnnotationId& y,
                 const AnnotationType& type="syn",
                 const AnnotationId& annoId="");

/// Delete a non-terminal node.
/**
 * @param x
 *    Id of an annotation (node) to be deleted.
 * @returns
 *    true on success, false on fail.
 */
DllExport bool
tree_delete_node(const AnnotationId& x);

#endif
