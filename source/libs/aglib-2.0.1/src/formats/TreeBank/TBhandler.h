// TBhandler.h: TreeBank file handler class for TreeBank parser
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _TBHANDLER_H_
#define _TBHANDLER_H_

#include <stack>
#include <map>
#include <vector>
#include <ag/AGAPI.h>
#include <ag/agtree.h>
#include "TBEscapeHandler.h"

/// TreeBank parentheses handler class.
class TBhandler
{
private:
  AGId agId;
  AnchorId prevAncrId;
  int prevIdx;

  TBEscapeHandler* escHandler;

  stack<AnchorId> AnchorStack;
  stack<string> TagStack;
  stack<string> AnnTypeStack;
  stack<AnnotationId> ChildStack;
  vector<AnnotationId> traces;

  bool eos_flag;    // indicate end-of-sentence

  
public:
  TBhandler(const AGId &id, TBEscapeHandler& eh):
    agId(id), escHandler(&eh), eos_flag(true) {
    AnnotationId ann = tree_last_tree(id);
    if (! ann.empty()) prevAncrId = GetEndAnchor(ann);
  }
  
  /**
   * Called whenever the parser detects left parenthesis in the file.
   */
  void start_paren(const string &tag, const string &label);

  /** 
   * Called whenever the parser detects right parenthesis in the file.
   */
  void end_paren();
};

#endif
