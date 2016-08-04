// TBhandler.cc: TreeBank file handler implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <iostream>
#include <cstdio>
#include <ag/RE.h>
#include "TBhandler.h"

static string
itoa(int x)
{
  char s[36];
  sprintf(s, "%d", x);
  return string(s);
}

typedef struct {
  string tag;       // tag name
  string roles;     // list of roles
  string trace;     // trace ref. #
} tag_info;

static tag_info* tag_anal(const string&);

void
TBhandler::start_paren(const string &tag, const string &label)
{
  AnchorId ancrId;
  AnnotationId annoId;
  //  RE ttype("^([^/]+)/([^/]+)$");
  RE trace("^(.+)-([0-9]+)$");
  string word, pos;

  if (label != "") {     // tag != "" too (word node with pos tag)
    /*
    if (ttype.match(label)) {
      // for ATIS data
      AnchorStack.push(prevAncrId);
      AnnTypeStack.push("syn");

      ancrId = CreateAnchor(agId);
      word = ttype.get_matched(1);
      pos = ttype.get_matched(2);
      annoId = CreateAnnotation(agId, prevAncrId, ancrId, "wrd");

      if (pos == "XXX") {
	TagStack.push(pos);
	SetFeature(annoId, "depth", itoa(AnchorStack.size()));
	if (trace.match(word)) {
	  SetFeature(annoId, "trace_type", trace.get_matched(1));
	  SetFeature(annoId, "trace_ref", trace.get_matched(2));
	}
	else
	  SetFeature(annoId, "trace_type", word);
      }
      else {
	TagStack.push(tag);
	word = escHandler->decode("wrd", word);
	pos = escHandler->decode("pos", pos);
	prevIdx += word.size() + 1;
	SetFeature(annoId, "label", word);
	SetFeature(annoId, "depth", itoa(AnchorStack.size()+1));
	annoId = CreateAnnotation(agId, prevAncrId, ancrId, "pos");
	SetFeature(annoId, "label", pos);
	SetFeature(annoId, "depth", itoa(AnchorStack.size()));
      }
    }

    else {
    */
      // for WSJ data
      AnchorStack.push(prevAncrId);
      AnnTypeStack.push("pos");

      ancrId = CreateAnchor(agId);

      if (tag == "-NONE-") {    // process trace
        annoId = CreateAnnotation(agId, prevAncrId, ancrId, "syn");
	TagStack.push(tag);
	if (trace.match(label)) {
	  SetFeature(annoId, "trace_type", trace.get_matched(1));
	  SetFeature(annoId, "trace_ref", trace.get_matched(2));
	}
	else
	  SetFeature(annoId, "trace_type", label);
	//	SetFeature(annoId, "depth", itoa(AnchorStack.size()-1));
      }
      else {                    // normal node (pos_tag + word)
        annoId = CreateAnnotation(agId, prevAncrId, ancrId, "wrd");
	TagStack.push(escHandler->decode("pos", tag));
	word = escHandler->decode("wrd", label);
	prevIdx += word.size() + 1;
	SetFeature(annoId, "label", word);
	//	SetFeature(annoId, "depth", itoa(AnchorStack.size()));
	ChildStack.push("-");
      }

      ChildStack.push(annoId);
      /*
    }
      */
    SetAnchorOffset(ancrId, (double) prevIdx);
    prevAncrId = ancrId;
  }
  else if (tag != "") {  // label == "" (non-terminal)
    //if (eos_flag == false) {
      AnchorStack.push(prevAncrId);
      TagStack.push(tag);
      AnnTypeStack.push("syn");
      ChildStack.push("-");
      //}
  }
  else {                 // the very start of a sentence
    if (!ExistsAnchor(prevAncrId)) {
      prevAncrId = CreateAnchor(agId); 
      SetAnchorOffset(prevAncrId, 0.0);
      prevIdx = 0;
    }

    AnchorStack.push(prevAncrId);
    TagStack.push("");
    AnnTypeStack.push("dummy_root");
    ChildStack.push("-");
    //eos_flag = false;
  }
}

void
TBhandler::end_paren()
{
  //  if (!AnchorStack.empty()) {
    if (TagStack.top() != "-NONE-") {  // if not a trace
      AnnotationId anno;
      string type = AnnTypeStack.top();

      anno = CreateAnnotation(agId, AnchorStack.top(), prevAncrId, type);

      // analyze tag
      tag_info* ti = tag_anal(TagStack.top());
      
      // set "label"
      SetFeature(anno, "label", ti->tag);

      // set "roles" feature
      if (!ti->roles.empty())
      SetFeature(anno, "roles", ti->roles);

      // "trace_ref"
      if (!ti->trace.empty())
	SetFeature(anno, "trace_ref", ti->trace);
      
      // set "depth" feature
      //      SetFeature(anno, "depth", itoa(AnchorStack.size()-1));

      // set "parent" feature
      while (ChildStack.top() != "-") {
	SetFeature(ChildStack.top(), "parent", anno);
	ChildStack.pop();
      }
      ChildStack.pop();
      if (ChildStack.empty())
	SetFeature(anno, "parent", "");
      else
	ChildStack.push(anno);
    }
    AnchorStack.pop();
    TagStack.pop();
    AnnTypeStack.pop();
    //  }
  /*
  else {
    // the top node has no parent
    AnnotationId anno = ChildStack.top();
    SetFeature(anno, "parent", "");
    ChildStack.pop();   // the stack should be empty by now

    // signal end_of_sentence
    //eos_flag = true;
  }
  */
}
  
static tag_info*
tag_anal(const string& tag)
{
  static tag_info info;
  RE headtail("^([^ \t\n-]+)-(.*)$");
  RE trace("^[0-9]+$");

  string s = tag;

  info.roles = "";
  info.trace = "";

  if (headtail.match(s)) {
    info.tag = headtail.get_matched(1);

    s = headtail.get_matched(2);
    while (headtail.match(s)) {
      info.roles += headtail.get_matched(1) + " ";
      s = headtail.get_matched(2);
    }

    if (trace.match(s)) {
      info.trace = s;
      int n = info.roles.length();
      if (n > 0)
	info.roles.erase(n-1);
    }
    else
      info.roles += s;
  }

  else
    info.tag = s;

  return &info;
}
