// ATLAS.h: AIF Level 1 loader/writer class
// Haejoong Lee, Kazuaki Maeda, Steven Bird, Xiaoyi Ma
// Copyright (C) 2001,2002 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _ATLAS_H_
#define _ATLAS_H_

#include <string>
#include <map>
#include <ag/agfio_plugin.h>
using std::string;
using std::map;


/**
 * \brief Atlas (AIF level 1) loader/writer
 *
 * <p> Atlas format extends annotation graph data model in various ways.  Most
 * of all, it generalizes the concept of anchor.  Anchor is not just a time
 * mark in Atlas.  It is a point in a multi-dimensional space.  Region
 * revives in Atlas.  It specifies a portion in a signal to be annotated.
 * Region can be expressed in a very complex way, using anchors, annotations
 * and even regions.
 *
 * <p> By restricting the target signals to linear ones, it is still possible
 * to model Atlas format as annotation graph.  The restriction coded in
 * ATLAS class is as follows:
 *
 * <ul>
 * <li> Anchor has one and only one parameter of numeric type.
 * <li> Region has exactly two anchor references.  The role of one anchor
 * reference must be \c start and the other must be \c end.  \c start anchor
 * specifies the start offset and \c end anchor specifies the end offset.
 * </ul>
 *
 * <p> Atlas also extends the content model of annotation of annotation graph,
 * and this also needs to be restricted.
 *
 * <ul>
 * <li> The content must be a list of parameters.  Role of a parameter
 * is mapped to annotation feature name.  The text content of parameter is
 * mapped to annotation feature value.
 * </ul>
 *
 * <p> Atlas makes the hierarchy of annotations explicit be specifying the
 * annotation's children.  In annotation graph, such hierarchy is implicit
 * and incomplete.  To solve this problem, Atlas ag model introduces a
 * special annotation feature, \c _AtlasAnnChil_.  There are other
 * special features, and they are summarized below.
 *
 * <ul>
 * <li> \c _AtlasAnnChil_ : Annotation feature. List of child annotations, e.g.
 *   <table border="1" cellpadding="0"><tr><td><small><tt>token token
 *   ATLAS:AG1:Annotation1 ATLAS:AG1:Annotation2 ATLAS:AG1:Annotation3 ;
 *   token interruptionPoint ATLAS:AG1:Annotation3</tt></small></td></tr>
 *   </table>
 *   This example means the annotation has 2 children group.  The first group
 *   contains \c token type children and their role is \c token.  The
 *   second group contains \c token type children and their role is
 *   \c interruptionPoint.
 * <li> \c _AtlasAnnID_ : Annotation feature.  Atlas allows external
 *   references to annotation.  Thus, it's important to keep original ids
 *   during conversion.  This feature keeps the original id.
 * <li> \c _AtlasRegID_ : Annotation feature.  Atlas region id.
 * <li> \c _AtlasStartAncID_ : Annotation feature.  Atlas start anchor id.
 * <li> \c _AtlasEndAncID_ : Annotation feature.  Atlas end anchor id.
 * <li> \c _AtlasCorpusID_ : AGSet feature (metadata).  Atlas corpus id.
 * <li> \c _AtlasCorpusType_ : AGSet feature (metadata).  Atlas corpus type.
 * <li> \c _AtlasSchemeLocation_ : AGSet feature (metadata).  The location
 *   of MAIA file.
 * <li> \c _AtlasAnaMapStr_ : AGSet feature (metadata).  Records original
 *   Atlas analysis ids.
 * <li> \c _AtlasSignalID_ : Signal feature (metadata).  Atlas signal id.
 * </ul>
 *
 * \attention Applications should carefully manupulate the \c _AtlasAnnChil_
 * feature.  The value of that feature is critical in generating Atlas output.
 *
 */
class ATLAS: public agfio_plugin
{
private:

  virtual list<AGId>
  load(const string& filename,
       const Id& id = "",
       map<string,string>* signalInfo = NULL,
       map<string,string>* options = NULL)
    throw (agfio::LoadError);

  virtual string
  store(const string& filename,
        list<string>* const ids,
        map<string,string>* options = NULL)
    throw (agfio::StoreError);

  virtual string
  store(const string& filename,
        const Id& id,
        map<string,string>* options = NULL)
    throw (agfio::StoreError);
};

#endif
