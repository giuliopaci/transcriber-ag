// ATLAS_store.cc: AIF Level 1 writer implementation
// Haejoong Lee, Kazuaki Maeda, Steven Bird, Xiaoyi Ma
// Copyright (C) 2001,2002 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <fstream>
#include <vector>
#include <assert.h>
#include <ag/AGAPI.h>
#include <ag/AGException.h>
#include <ag/RE.h>
#include "ATLAS.h"
#include "agfXercesUtils.h"
#include <cstdio>
#include <cstdlib>
/* (( BT Patch -- */
#include <errno.h>
/* -- BT Patch -- */

using namespace std;

namespace ATLAS_store {

  /////////////////
  // global data //
  /////////////////
  
  // region  by Anc     by Reg  by Ann  ag
  // name    start end                  annos
  // ------  ----- ---  ------  ------  -----
  //                    map:    map:    set:
  //                    agann/  agann/  anno
  //                    role    role
  //
  // - method 1: longest span (assume no cross between regions)
  // - method 2: pointer (_RC_)
  
  // per RegionType in MAIA
  struct regtab_entry_t {
    string anc_start_role;
    string anc_end_role;
    map<AnnotationType,string> sub_reg_map;   // ag anno, role
    map<AnnotationType,string> sub_ann_map;   // ag anno, role
    set<AnnotationType> annos;
  };
  
  typedef map<string,regtab_entry_t> regtab_t;
  
  
  // ann   reg   para   children
  // name  role        
  // ----  ----  ----   --------
  //             map:   children_t
  //             type/ 
  //             role  
  //
  // - method 1: longest span (assume no cross between regions)
  // - method 2: pointer (_AC_)
  
  // per AnnotationType in MAIA
  struct children_entry_t {
    string ag_type;   // for efficiency
    string type;      // containedType
    string role;
    bool defines_region;
    bool defines_content;
    children_entry_t(): defines_region(false), defines_content(false) {}
  };
  
  typedef vector<children_entry_t> children_t;
  
  struct anntab_entry_t {
    string reg_type;
    string con_type;
    map<string,string> para_map;  // role, parameter type
    children_t children;
  };
  
  typedef map<string,anntab_entry_t> anntab_t;

  struct offset_comp {
    bool operator()(const AnnotationId& x, const AnnotationId& y) {
      return (GetStartOffset(x) < GetStartOffset(y));
    }
  };  
      
};

using namespace ATLAS_store;


/////////////////////
static map<string,map<string,DOMNode*> > MAIAMap;
static regtab_t RegTab;
static anntab_t AnnTab;
static map<string,AnnotationType> AnnMemberMap;
static map<AnnotationType,string> AnnoNameMap;
static string Corpus_type;
static string Signal_type;
static string Anchor_type;
static string Anchor_Signal_role;
static string Anchor_Parameter_type;
static string Anchor_Parameter_role;
static string Anchor_Parameter_unit;
static string AnchorSet_containedType;
static map<string,string> IdMap;
static map<string,map<string,string> > AnaIdMap;
/////////////////////

static string
next_id(string& ids)
{
  int i, j;
  string s;

  i = ids.find_first_not_of(" ");
  if (i == string::npos)
    return "";

  j = ids.find_first_of(" ", i);
  if (j == string::npos) {
    s = ids.substr(i);
    ids = "";
  }
  else {
    s = ids.substr(i, j-i);
    ids = ids.substr(j);
  }
  return s;
}

static string
itoa(int x)
{
  char s[36];
  sprintf(s, "%d", x);
  return string(s);
}

static void
build_MAIAMap(const string& maia_file, bool val_opt)
{
  DOMDocument* doc = agfDOMParse(maia_file, val_opt);
  if (doc == NULL)
    throw agfio::StoreError("ATLAS_store.cc:build_MAIAMap():parsing error at" +
                            maia_file);

  MAIAMap.clear();

  DOMNodeList* N =
    elts_by_name(doc,"TypeDefinitions")->item(0)->getChildNodes();

  for (int i=0; i < N->getLength(); ++i) {
    DOMNode* node = N->item(i);
    if (! is_text(node))
      MAIAMap[node_name(node)][node_att(node,"name")] = node;
  }
}

inline static void
init_global_data(list<AGId>& ags,
		 const string& maia_file,
		 const string& corpus_type,
		 const string& type_rename,
		 string anamapstr,
		 bool val_opt)
{
  RegTab.clear();
  AnnTab.clear();
  AnnMemberMap.clear();
  //AnnoNameMap.clear();      // it's been already initialized
  MAIAMap.clear();

  build_MAIAMap(maia_file, val_opt);

  DOMNode* corpus = MAIAMap["CorpusType"][corpus_type];
  DOMNode* node, *node1, *node2;
  DOMNodeList* N, *N1, *N2, *N3;
  map<string,string> att;
  map<string,DOMNode*> dommap;
  map<string,DOMNode*>::iterator dommap_p;
  string tag;

  // Corpus_type
  Corpus_type = corpus_type;
  // Signal_type
  N = ((DOMElement*)corpus)->getElementsByTagName(trans("SignalType"));
  if (N->getLength() != 1)
    throw agfio::StoreError("ATLAS_store.cc:init_global_data():there should be one and only one SignalType element in Corpus element");
  att2map(N->item(0), att);
  Signal_type = att["ref"];

  // AnchorSet_containedType
  // Anchor_type
  N = ((DOMElement*)corpus)->getElementsByTagName(trans("AnchorType"));
  if (N->getLength() != 1)
    throw agfio::StoreError("ATLAS_store.cc:init_global_data():there should be one and only one AnchorType element in Corpus element");
  att2map(N->item(0), att);
  AnchorSet_containedType = Anchor_type = att["ref"];

  // Anchor_Signal_role
  dommap = MAIAMap["AnchorType"];
  dommap_p = dommap.find(att["ref"]);
  if (dommap_p == dommap.end())
    throw agfio::StoreError("ATLAS_store.cc:init_global_data():error in the MAIA file -- AnchorType " + att["ref"] + "is not defined");
  node = dommap_p->second;
  N = ((DOMElement*)node)->getElementsByTagName(trans("SignalType"));
  assert(N->getLength() == 1);
  att2map(N->item(0), att);
  Anchor_Signal_role = att["ref"];
  // Anchor_Parameter_type
  // Anchor_Parameter_role
  N = ((DOMElement*)node)->getElementsByTagName(trans("ParameterType"));
  if (N->getLength() != 1)
    throw agfio::StoreError("ATLAS_store.cc:init_global_data():there should be one and only one ParameterType element in AnchorType element");
  att2map(N->item(0), att);
  Anchor_Parameter_type = att["ref"];
  Anchor_Parameter_role = att["role"];

  // AnnoNameMap
  if (!type_rename.empty()) {
    string s1, s2, ss = type_rename;
    while ((s1=next_id(ss))!="" && (s2=next_id(ss))!="")
      AnnoNameMap[s1] = s2;
  }

  // AnnMemberMap
  for (map<AnnotationType,string>::iterator pos = AnnoNameMap.begin();
       pos != AnnoNameMap.end(); ++pos)
    AnnMemberMap[pos->second] = pos->first;

  // RegTab
  N = ((DOMElement*)corpus)->getElementsByTagName(trans("RegionType"));
  for (int i=0; i < N->getLength(); ++i) {
    att2map(N->item(i), att);
    node = MAIAMap["RegionType"][att["ref"]];
    assert(node != NULL);
    regtab_entry_t& r = RegTab[att["ref"]];
    // base region?
    N1 = ((DOMElement*)node)->getElementsByTagName(trans("AnchorType"));
    N2 = ((DOMElement*)node)->getElementsByTagName(trans("RegionType"));
    N3 = ((DOMElement*)node)->getElementsByTagName(trans("AnnotationType"));
    if (N1->getLength() == 2 &&
	N2->getLength() == 0 &&
	N3->getLength() == 0) {
      att2map(N1->item(0), att);
      assert(att["ref"] == Anchor_type);
      r.anc_start_role = att["role"];
      att2map(N1->item(1), att);
      assert(att["ref"] == Anchor_type);
      r.anc_end_role = att["role"];
    }
    else if (N1->getLength() == 0) {
      int j;
      for (j=0; j < N2->getLength(); ++j) {
	att2map(N2->item(j), att);
	r.sub_reg_map[AnnMemberMap[att["ref"]]] = att["role"];
      }
      for (j=0; j < N3->getLength(); ++j) {
	att2map(N3->item(j), att);
	r.sub_ann_map[AnnMemberMap[att["ref"]]] = att["role"];
      }
    }
    else {
      throw agfio::StoreError("ATLAS_store.cc:init_global_data():this RegionType (" + att["ref"] + ") is not supported");
    }
  }

  // AnnTab
  dommap = MAIAMap["AnnotationType"];
  dommap_p = dommap.begin();
  for (; dommap_p != dommap.end(); ++dommap_p) {
    node = dommap_p->second;
    anntab_entry_t& r = AnnTab[dommap_p->first];

    N = node->getChildNodes();
    for (int i=0; i < N->getLength(); ++i) {
      node1 = N->item(i);      // RegionType/ContentType/ChildrenType
      if (is_text(node1))
	continue;
      tag = node_name(node1);
      att2map(node1, att);
      if (tag == "RegionType") {
	r.reg_type = att["ref"];
      }
      else if (tag == "ContentType") {
	r.con_type = att["ref"];
	N1 = elts_by_name(MAIAMap[tag][att["ref"]], "ParameterType");
	for (int j=0; j < N1->getLength(); ++j) {
	  att2map(N1->item(j), att);
	  r.para_map[att["role"]] = att["ref"];
	}
      }
      else if (tag == "ChildrenType") {
	children_entry_t c;
	att2map(node1, att);
	c.role = att["role"];
	node2 = first_node(MAIAMap[tag][att["ref"]]);
	// node2 := AnnotationType
	att2map(node2, att);
	c.type = att["ref"];
	c.ag_type = AnnMemberMap[c.type];
	r.children.push_back(c);
      }
      else if (tag == "RegionDefinedAs") {
	att2map(node1, att);
	for (int i=0; i < r.children.size(); ++i) {
	  children_entry_t& c = r.children[i];
	  if (c.role == att["byChildrenWithRole"]) {
	    c.defines_region = true;
	    break;
	  }
	}
      }
      else if (tag == "ContentDefinedAs") {
	att2map(node1, att);
	for (int i=0; i < r.children.size(); ++i) {
	  children_entry_t& c = r.children[i];
	  if (c.role == att["byChildrenWithRole"]) {
	    c.defines_content = true;
	    break;
	  }
	}
      }
    }
  }

  // RegTab.annos
  {
  map<AnnotationType,string>::iterator pos;
  for (pos=AnnoNameMap.begin(); pos != AnnoNameMap.end(); ++pos) {
    // pos->first: ag anno type
    // pos->second: atlas anno type
    // AnnTab[pos->first].reg_type: atlas reg type
    string& reg_type = AnnTab[pos->second].reg_type;
    if (!reg_type.empty())
      RegTab[reg_type].annos.insert(pos->first);
  }
  }

  // IdMap
  list<AGId>::iterator ag_p;
  int sn;

  sn=0;
  for (ag_p=ags.begin(); ag_p != ags.end(); ++ag_p) {
    // anchor ids
    list<string> ids = GetAnchorSet(*ag_p);
    list<string>::iterator id;
    for (id=ids.begin(); id != ids.end(); ++id)
      IdMap[*id] = "Anc" + itoa(++sn);
    // signal ids
    set<string> sigids = GetSignals(GetTimelineId(*ag_p));
    set<string>::iterator sigid = sigids.begin();
    for (; sigid != sigids.end(); ++sigid)
      IdMap[*sigid] = GetFeature(*sigid, "_AtlasSignalID_");
  }

  // AnaIdMap
  string anatype, anarole, anaid;
  anatype = next_id(anamapstr);
  anarole = next_id(anamapstr);
  anaid = next_id(anamapstr);
  while (!anatype.empty()) {
    AnaIdMap[anatype][anarole] = anaid;
    anatype = next_id(anamapstr);
    anarole = next_id(anamapstr);
    anaid = next_id(anamapstr);
  }
}




static void
write_opentag(ostream& out,
	      const string& tag,
	      const map<string,string>& att,
	      bool changeline=false)
{
  map<string,string>::const_iterator pos;

  out << "<" << tag << " ";
  for (pos=att.begin(); pos != att.end(); ++pos)
    out << pos->first << "=\"" << pos->second << "\" ";
  out << ">";
  if (changeline)
    out << endl;
}

inline static void
write_opentag(ostream& out, const string& tag)
{ out << "<" << tag << ">" << endl; }

static void
write_singletag(ostream& out, const string& tag, const map<string,string>& att)
{
  map<string,string>::const_iterator pos;

  out << "<" << tag << " ";
  for (pos=att.begin(); pos != att.end(); ++pos)
    out << pos->first << "=\"" << pos->second << "\" ";
  out << "/>" << endl;
}

inline static void
write_closetag(ostream& out, const string& tag)
{ out << "</" << tag << ">" << endl; }




static void
write_Metadata(ostream& out, const Id& id)
{
  map<string,string> att;
  att["type"] = "AG";

  write_opentag(out, "Metadata", att);

  att["type"] = "other";
  set<FeatureName> names = GetFeatureNames(id);
  set<FeatureName>::iterator name = names.begin();
  for (; name != names.end(); ++name) {
    att["name"] = *name;
    write_opentag(out, "OtherMetadata", att);
    out << GetFeature(id, *name);
    write_closetag(out, "OtherMetadata");
  }

  write_closetag(out, "Metadata");
}

inline static void
write_SimpleSignals(ostream& out, list<AGId>& ags)
{
  set<SignalId> sig_set, atlas_sigid_set;
  map<string,string> att;

  // collect signals
  list<AGId>::iterator ag_p = ags.begin();
  for (; ag_p != ags.end(); ++ag_p) {
    set<SignalId> ids = GetSignals(GetTimelineId(*ag_p));
    for (set<SignalId>::iterator id=ids.begin(); id != ids.end(); ++id)
      sig_set.insert(*id);
  }

  // write SimpleSignals
  set<SignalId>::iterator sig_p = sig_set.begin();
  att["type"] = Signal_type;
  for (; sig_p != sig_set.end(); ++sig_p) {
    const string& id = *sig_p;
    string atlas_sigid;
    if (ExistsFeature(id, "_AtlasSignalID_")) {
      atlas_sigid = GetFeature(id, "_AtlasSignalID_");
      if (atlas_sigid_set.insert(atlas_sigid).second == false)
        // already processed?
        continue;
    }
    else {
      // no atlas signal id?  use ag signal id
      atlas_sigid = id;
    }

    att["id"] = atlas_sigid;
    att["mimeClass"] = GetSignalMimeClass(id);
    att["mimeType"] = GetSignalMimeType(id);
    att["encoding"] = GetSignalEncoding(id);
    att["xlink:href"] = GetSignalXlinkHref(id);
    att["track"] = GetSignalTrack(id);

    //if (GetFeatureNames(id).empty())
      write_singletag(out, "SimpleSignal", att);
    //else {
      //write_opentag(out, "SimpleSignal", att);
      //write_Metadata(out, id);
      //write_closetag(out, "SimpleSignal");
    //}
    // FIXME: need to filter out _ATLAS features from the metadata 
  }
}
    
inline static void
write_AnchorSet(ostream& out, list<AGId>& ags)
{
  map<string,string> att;
  set<AnchorId> anc_set;

  // collect anchor ids
  list<AGId>::iterator ag_p = ags.begin();
  for (; ag_p != ags.end(); ++ag_p) {
    list<AnchorId> ids = GetAnchorSet(*ag_p);
    for (list<AnchorId>::iterator id=ids.begin(); id != ids.end(); ++id)
      anc_set.insert(*id);
  }

  if (anc_set.empty())
    return;

  att["containedType"] = AnchorSet_containedType;
  write_opentag(out, "AnchorSet", att, true);

  att.clear();
  map<string,string> sig_att, para_att;
  att["type"] = Anchor_type;
  sig_att["role"] = Anchor_Signal_role;
  para_att["type"] = Anchor_Parameter_type;
  para_att["role"] = Anchor_Parameter_role;
  set<AnchorId>::iterator anc_p = anc_set.begin();
  for (; anc_p != anc_set.end(); ++anc_p) {
    const string& anc_id = *anc_p;
    att["id"] = IdMap[anc_id];
    write_opentag(out, "Anchor", att, true);

    set<SignalId> sigs = GetAnchorSignalIds(anc_id);
    if (sigs.size() != 1)
      throw agfio::LoadError("ATLAS_store.cc:write_AnchorSet():anchor "
			     + anc_id + " has more than one signals");
    sig_att["xlink:href"] = "#" + IdMap[*sigs.begin()];
    out << "  ";
    write_singletag(out, "SignalRef", sig_att);

    string anc_unit = GetOffsetUnit(anc_id);
    para_att["unit"] = anc_unit.empty() ? "NULL_UNIT" : anc_unit;
    out << "  ";
    write_opentag(out, "Parameter", para_att);
    if (GetAnchored(anc_id))
      out << GetAnchorOffset(anc_id);
    write_closetag(out, "Parameter");

    write_closetag(out, "Anchor");
  }

  write_closetag(out, "AnchorSet");
}





static bool
includes(const AnnotationId& x, const AnnotationId& y)
{
  double xs = GetStartOffset(x);
  double xe = GetEndOffset(x);
  double ys = GetStartOffset(y);
  double ye = GetEndOffset(y);

  return ((xs <= ys && ye < xs) || (xs < ys && ye <= xs));
}

static bool
identical(const AnnotationId& x, const AnnotationId& y)
{
  return (GetStartOffset(x)==GetStartOffset(y) &&
	  GetEndOffset(x)==GetEndOffset(y));
}

static bool
cross(const AnnotationId& x, const AnnotationId& y)
{
  double xs = GetStartOffset(x);
  double xe = GetEndOffset(x);
  double ys = GetStartOffset(y);
  double ye = GetEndOffset(y);

  return ((xs < ys && ys < xe && xe < ye) ||
	  (ys < xs && xs < ye && ye < xe));
}

/*
static vector<AnnotationId>
get_children_by_inclusion(const AnnotationId& anno,
			  set<AnnotationType>& subtypes)
{
  set<AnnotationId,offset_comp> annos;
  set<AnnotationId> filtered;
  vector<AnnotationId> result;

  double x = GetEndOffset(anno);
  set<AnnotationId> ids =
    GetAnnotationSeqByOffset(GetAGId(anno), GetStartOffset(anno), x);
  for (set<AnnotationId>::iterator id=ids.begin(); id != ids.end(); ++id)
    if (subtypes.find(GetAnnotationType(*id)) != subtypes.end() &&
	GetEndOffset(*id) <= x)
      annos.insert(*id);

  set<AnnotationId>::iterator i, j;
  for (i=annos.begin(); i != annos.end(); ++i) {
    if (filtered.find(*i) != filtered.end())
      continue;
    for (j=i,++j; j != annos.end(); ++j) {
      if (includes(*j, *i)) {
	filtered.insert(*i);
	break;
      }
      else if (includes(*i, *j))
	filtered.insert(*j);
      else if (identical(*i, *j)) {
	throw agfio::StoreError("ATLAS_store.cc:get_children_by_inclusion():annotations " +
				*i + " and " + *j + " have the same span");
      }
      else if (cross(*i, *j)) {
	throw agfio::StoreError("ATLAS_store.cc:get_children_by_inclusion():annotations " +
				*i + " and " + *j + " cross each other");
      }
      // both *i and *j survive
    }
	
    if (j == annos.end())
      result.push_back(*i);
  }

  return result;
}
*/

inline static vector<AnnotationId>
get_children(const AnnotationId& anno,
	     children_entry_t& chrec)
{
  // check _AtlasAnnChil_ feature first
  vector<AnnotationId> result;
  if (ExistsFeature(anno, "_AtlasAnnChil_")) {
    string search = chrec.type + " " + chrec.role + " ";
    string id, ids = GetFeature(anno, "_AtlasAnnChil_");    
    int d = ids.find(search);
    if (d != string::npos) {
      ids = ids.substr(d + search.size());
      id = next_id(ids);
      while (id != "" && id != ";") {
	result.push_back(id);
	id = next_id(ids);
      }
    }
  }

  return result;
  
  /*
  set<AnnotationType> subtypes;
  subtypes.insert(chrec.ag_type);
  return get_children_by_inclusion(anno, subtypes);
  */
}

inline static void
write_RegionSets(ostream& out, list<AGId>& ags)
{
  regtab_t::iterator pos = RegTab.begin();
  for (; pos != RegTab.end(); ++pos) {
    const string& reg_type = pos->first;
    regtab_entry_t& rec = pos->second;

    set<AnnotationType>& anno_types = rec.annos;
    set<AnnotationType>::iterator type_p = anno_types.begin();

    // collect annotations
    set<AnnotationId> anno_set;
    for (; type_p != anno_types.end(); ++type_p) {
      assert(! type_p->empty());
      list<AGId>::iterator ag_p = ags.begin();
      for (; ag_p != ags.end(); ++ag_p) {
	set<AnnotationId> ids = GetAnnotationSet(*ag_p, *type_p);
	for (set<AnnotationId>::iterator id=ids.begin(); id!=ids.end(); ++id)
	  anno_set.insert(*id);
      }
    }
      
    map<string,string> att;
    att["containedType"] = reg_type;
    write_opentag(out, "RegionSet", att, true);
    set<AnnotationId>::iterator anno_p;

    if (!rec.anc_start_role.empty()) {
      // base region
      att.clear();
      map<string,string> s_anc_att, e_anc_att;
      att["type"] = reg_type;
      s_anc_att["role"] = rec.anc_start_role;
      e_anc_att["role"] = rec.anc_end_role;
      for (anno_p=anno_set.begin(); anno_p != anno_set.end(); ++anno_p) {
	const AnnotationId& anno_id = *anno_p;
	//att["id"] = "Reg" + IdMap[anno_id];
	att["id"] = GetFeature(anno_id, "_AtlasRegID_");
	write_opentag(out, "Region", att, true);

	s_anc_att["xlink:href"] = "#" + IdMap[GetStartAnchor(anno_id)];
	out << "  ";
	write_singletag(out, "AnchorRef", s_anc_att);

	e_anc_att["xlink:href"] = "#" + IdMap[GetEndAnchor(anno_id)];
	out << "  ";
	write_singletag(out, "AnchorRef", e_anc_att);

	write_closetag(out, "Region");
      }
    }
    else {
      throw agfio::StoreError("ATLAS_store.cc:write_RegionSets():region type, " + reg_type + ", is not supported yet");
      /*
      att.clear();
      map<string,string> sub_att;
      att["type"] = reg_type;

      set<AnnotationType> rsubtypes, asubtypes;
      map<AnnotationType,string>::iterator pos = rec.sub_reg_map.begin();
      for (; pos != rec.sub_reg_map.end(); ++pos)
	rsubtypes.insert(pos->first);
      for (pos=rec.sub_ann_map.begin(); pos != rec.sub_ann_map.end(); ++pos)
	asubtypes.insert(pos->first);

      for (anno_p=anno_set.begin(); anno_p != anno_set.end(); ++anno_p) {
	const AnnotationId& anno_id = *anno_p;
	
	//att["id"] = "Reg" + IdMap[anno_id];
	att["id"] = GetFeature(anno_id, "_AtlasRegID_");
	write_opentag(out, "Region", att, true);

	vector<AnnotationId> subannos =
	  get_children_by_inclusion(anno_id, rsubtypes);
	vector<AnnotationId>::iterator sub_p;

	for (sub_p=subannos.begin(); sub_p != subannos.end(); ++sub_p) {
	  const AnnotationId& anno_id = *sub_p;
	  //sub_att["xlink:href"] = "#Reg" + IdMap[anno_id];
	  sub_att["xlink:href"] = "#" + GetFeature(anno_id, "_AtlasRegID_");
	  sub_att["role"] = rec.sub_reg_map[GetAnnotationType(anno_id)];
	  out << "  ";
	  write_singletag(out, "RegionRef", sub_att);
	}

	subannos = get_children_by_inclusion(anno_id, asubtypes);

	for (sub_p=subannos.begin(); sub_p != subannos.end(); ++sub_p) {
	  const AnnotationId& anno_id = *sub_p;
	  // sub_att["xlink:href"] = "#Ann" + IdMap[anno_id];
	  sub_att["xlink:href"] = "#" + GetFeature(anno_id, "_ID");
	  sub_att["role"] = rec.sub_ann_map[GetAnnotationType(anno_id)];
	  out << "  ";
	  write_singletag(out, "AnnotationRef", sub_att);
	}

	write_closetag(out, "Region");
      }
      */
    }
    write_closetag(out, "RegionSet");
  }
}


static void
write_AnnotationSet(ostream& out, DOMNode* annref, list<AGId>& ags)
{
  string ann_type = trans(((DOMElement*)annref)->getAttribute(trans("ref")));
  string ag_ann_type = AnnMemberMap[ann_type];
  if (ag_ann_type.empty()) {
    map<string,string> att;
    att["containedType"] = ann_type;
    write_singletag(out, "AnnotationSet", att);
    return;
  }

  anntab_entry_t& annrec = AnnTab[ann_type];
  children_t& children = annrec.children;

  // collect annotations
  set<AnnotationId> annos;
  list<AGId>::iterator ag_p = ags.begin();
  for (; ag_p != ags.end(); ++ag_p) {
    set<AnnotationId> ids = GetAnnotationSet(*ag_p, ag_ann_type);
    for (set<AnnotationId>::iterator id=ids.begin(); id!=ids.end(); ++id)
      annos.insert(*id);
  }

  map<string,string> att;
  att["containedType"] = ann_type;
  write_opentag(out, "AnnotationSet", att, true);

  att.clear();
  att["type"] = ann_type;
  set<AnnotationId>::iterator anno_p;
  for (anno_p=annos.begin(); anno_p != annos.end(); ++anno_p) {
    map<string,string> reg_att, con_att, para_att;
    const AnnotationId& anno_id = *anno_p;
    // att["id"] = "Ann" + IdMap[anno_id];
    att["id"] = GetFeature(anno_id, "_AtlasAnnID_");
    write_opentag(out, "Annotation", att, true);

    if (annrec.reg_type.empty()) {
      // rdbc
      int i;
      for (i=0; i < children.size(); ++i) {
	children_entry_t& r = children[i];
	if (r.defines_region) {
	  reg_att["withContainedType"] = r.type;
	  reg_att["withRole"] = r.role;
	  out << "  ";
	  write_singletag(out, "RegionDefinedByChildren", reg_att);
	  break;
	}
      }
      assert(i < children.size());
    }
    else {
      //reg_att["xlink:href"] = "#Reg" + IdMap[anno_id];
      reg_att["xlink:href"] = "#" + GetFeature(anno_id, "_AtlasRegID_");
      reg_att["role"] = annrec.reg_type;
      out << "  ";
      write_singletag(out, "RegionRef", reg_att);
    }

    int i;
    for (i=0; i < children.size(); ++i) {
      children_entry_t& r = children[i];
      if (r.defines_content) {
	con_att["withContainedType"] = r.type;
	con_att["withRole"] = r.role;
	out << "  ";
	write_singletag(out, "ContentDefinedByChildren", con_att);
	break;
      }
    }
    if (i >= children.size()) {
      bool do_content = false;
      map<string,string>::iterator para_p = annrec.para_map.begin();
      /*
      for (;para_p != annrec.para_map.end(); ++para_p)
	if (ExistsFeature(anno_id, para_p->first)) {
	  do_content = true;
	  break;
	}
      */

      //if (do_content) {
	con_att["type"] = annrec.con_type;
	out << "  ";
	write_opentag(out, "Content", con_att, true);
	
	for (para_p = annrec.para_map.begin();
	     para_p != annrec.para_map.end(); ++para_p) {
	  if (! ExistsFeature(anno_id, para_p->first))
	    continue;
	  para_att["type"] = para_p->second;
	  para_att["role"] = para_p->first;
	  para_att["unit"] = "NULL_UNIT";
	  out << "    ";
	  write_opentag(out, "Parameter", para_att, false);
	  out << GetFeature(anno_id, para_p->first);
	  write_closetag(out, "Parameter");
	}

	out << "  ";
	write_closetag(out, "Content");
      //}
    }

    map<string,string> c_att;
    for (i=0; i < children.size(); ++i) {
      children_entry_t& r = children[i];
      vector<AnnotationId> C = get_children(anno_id, r);

      /*
      if (C.empty())
        continue;
      */

      c_att["containedType"] = r.type;
      c_att["role"] = r.role;
      out << "  ";
      write_opentag(out, "Children", c_att, true);

      vector<AnnotationId>::iterator pos;
      map<string,string> ann_att;
      ann_att["role"] = r.role;
      for (pos=C.begin(); pos != C.end(); ++pos) {
	// ann_att["xlink:href"] = "#Ann" + IdMap[*pos];
	ann_att["xlink:href"] = "#" + GetFeature(*pos, "_AtlasAnnID_");
	out << "    ";
	write_singletag(out, "AnnotationRef", ann_att);
      }
      out << "  ";
      write_closetag(out, "Children");
    }

    write_closetag(out, "Annotation");
  }

  write_closetag(out, "AnnotationSet");
}

static void
write_Analyses(ostream& out, list<AGId>& ags)
{
  DOMNodeList* N =
    ((DOMElement*)MAIAMap["CorpusType"][Corpus_type]) ->
    getElementsByTagName(trans("AnalysisType"));

  map<string,DOMNode*> NMap;
  int i;
  for (i=0; i < N->getLength(); ++i) {
    DOMNode* anaref = N->item(i);
    string ananame = trans(((DOMElement*)anaref)->getAttribute(trans("ref")));
    NMap[ananame] = anaref;
  }

  map<string,DOMNode*>::iterator pos;
  for (pos=NMap.begin(); pos != NMap.end(); ++pos) {
    DOMNode* anaref = pos->second;
    string ana_type =
      trans(((DOMElement*)anaref)->getAttribute(trans("ref")));
    string ana_role =
      trans(((DOMElement*)anaref)->getAttribute(trans("role")));
    map<string,string> att;
    string ana_id = AnaIdMap[ana_type][ana_role];
    att["id"] = ana_id.empty() ? "agf:ATLAS:store:" + itoa(++i) : ana_id;
    att["type"] = ana_type;
    att["role"] = ana_role;
    write_opentag(out, "Analysis", att, true);

    DOMNodeList* anndefs =
      ((DOMElement*)MAIAMap["AnalysisType"][ana_type]) ->
      getElementsByTagName(trans("AnnotationType"));

    for (int j=0; j < anndefs->getLength(); ++j)
      write_AnnotationSet(out, anndefs->item(j), ags);

    write_closetag(out, "Analysis");
  }
}

    
string
ATLAS::store(const string& filename,
	     list<AGId>* const ids,
	     map<string,string>* options)
  throw (agfio::StoreError)
  // available options:
  //   encoding
  //   schemeLocation
  //   MAIA
  //   Corpus/type
  //   Corpys/id
  //   type_rename
  //   AnaIdMap
  //   DTDvalidation
{
  if (ids->empty())
    // nothing to do
    return "";

  AnnoNameMap.clear();
  list<AGId>::iterator ag_p = ids->begin();
  for (; ag_p != ids->end(); ++ag_p) {
    set<AnnotationType> s = GetAnnotationTypes(*ag_p);
    set<AnnotationType>::iterator s_p = s.begin();
    for (; s_p != s.end(); ++s_p)
      AnnoNameMap[*s_p] = *s_p;
  }

  // determine maia file and corpus id/type
  // options --> agset feature --> agset id
  string schemeLocation = (*options)["schemeLocation"];
  string maia_file = (*options)["MAIA"];
  string corpus_id = (*options)["Corpus/id"];
  string corpus_type = (*options)["Corpus/type"];
  string anamapstr = (*options)["AnaIdMap"];
  bool val_opt = ((*options)["DTDvalidation"]=="false") ? false : true;

  if (schemeLocation.empty()) {
    // let's get the value from agsets' metadata (_AtlasSchemeLocation_)
    // use the value only if
    //   1) all agsets have that metadata
    //   2) the values are the same
    ag_p = ids->begin();
    AGSetId agsetId_1 = GetAGSetId(*ag_p);
    if (ExistsFeature(agsetId_1,"_AtlasSchemeLocation_")) {
      schemeLocation = GetFeature(agsetId_1,"_AtlasSchemeLocation_");
      // let's check whether 1) & 2) are satisfied
      for (++ag_p; ag_p != ids->end(); ++ag_p) {
	agsetId_1 = GetAGSetId(*ag_p);
	if (! ExistsFeature(agsetId_1,"_AtlasSchemeLocation_") ||
	    schemeLocation != GetFeature(agsetId_1,"_AtlasSchemeLocation_")) {
	  schemeLocation.erase();
	  break;
	}
      }
    }
  }

  if (schemeLocation.empty())
    schemeLocation = "file:" + maia_file;
  
  if (maia_file.empty()) {
    RE slpat("file:(.*)$");
    if (slpat.match(schemeLocation))
      maia_file = slpat.get_matched(1);
    if (maia_file.empty()) {
      throw agfio::StoreError("ATLAS_store.cc:init_global_data(): can't determine MAIA file location");
    }
  }

  if (corpus_id.empty()) {
    // let's get the value from agsets' metadata (_AtlasCorpusID_)
    // use the value only if
    //   1) all agsets have that metadata
    //   2) the values are the same
    ag_p = ids->begin();
    AGSetId agsetId_1 = GetAGSetId(*ag_p);
    if (ExistsFeature(agsetId_1,"_AtlasCorpusID_")) {
      corpus_id = GetFeature(agsetId_1,"_AtlasCorpusID_");
      for (++ag_p; ag_p != ids->end(); ++ag_p) {
	// let's check whether 1) & 2) are satisfied
	agsetId_1 = GetAGSetId(*ag_p);
	if (! ExistsFeature(agsetId_1,"_AtlasCorpusID_") ||
	    corpus_id != GetFeature(agsetId_1,"_AtlasCorpusID_")) {
	  corpus_id.erase();
	  break;
	}
      }
    }
  }
  if (corpus_id.empty()) {
    // let's get the value from agsets' ids
    // use the value only if
    //   1) all agsets have the same id
    ag_p = ids->begin();
    corpus_id = GetAGSetId(*ag_p);
    for (++ag_p; ag_p != ids->end(); ++ag_p) {
      if (corpus_id != GetAGSetId(*ag_p)) {
	throw agfio::StoreError("ATLAS_store.cc:init_global_data: can't determine corpus id.");
      }
    }
  }

  if (corpus_type.empty()) {
    // let's get the value from agsets' metadata (_AtlasCorpusType_)
    // use the value only if
    //   1) all agsets have that metadata
    //   2) the values are the same
    ag_p = ids->begin();
    AGSetId agsetId_1 = GetAGSetId(*ag_p);
    if (ExistsFeature(agsetId_1,"_AtlasCorpusType_")) {
      corpus_type = GetFeature(agsetId_1,"_AtlasCorpusType_");
      for (++ag_p; ag_p != ids->end(); ++ag_p) {
	agsetId_1 = GetAGSetId(*ag_p);
	if (! ExistsFeature(agsetId_1,"_AtlasCorpusType_") ||
	    corpus_type != GetFeature(agsetId_1,"_AtlasCorpusType_")) {
	  corpus_type.erase();
	  break;
	}
      }
    }
  }
  if (corpus_type.empty()) {
    throw agfio::StoreError("ATLAS_store.cc:init_global_data: can't determine corpus type.");
  }

  if (anamapstr.empty()) {
    for (ag_p=ids->begin(); ag_p != ids->end(); ++ag_p) {
      AGSetId agsetId_1 = GetAGSetId(*ag_p);
      if (ExistsFeature(agsetId_1,"_AtlasAnaMapStr_"))
	anamapstr += GetFeature(agsetId_1,"_AtlasAnaMapStr_") + " ";
    }
  }
  
  // open output file
  ofstream out(filename.c_str());
  if (!out)
    throw agfio::StoreError("ATLAS::store():can't open " +
                            filename + " for writing");

  xercesc_open();
  init_global_data(*ids, maia_file, corpus_type,
		   (*options)["type_rename"], anamapstr, val_opt);

  string enc = (*options)["encoding"];
  if (enc.empty())
    out << "<?xml version='1.0'?>" << endl;
  else
    out << "<?xml version=\"1.0\" encoding=\"" << enc << "\"" << endl;
  out << "<!DOCTYPE Corpus SYSTEM \"http://www.nist.gov/speech/atlas/aif.dtd\">" << endl;

  map<string,string> att;
  att["id"] = corpus_id;
  att["AIFVersion"] = "1.1";
  att["type"] = corpus_type;
  att["xmlns"] = "http://www.nist.gov/speech/atlas";
  att["xmlns:xlink"] = "http://www.w3.org/1999/xlink";
  att["schemeLocation"] = schemeLocation;
  write_opentag(out, "Corpus", att, true);
  out << "<Metadata/>" << endl;

  write_SimpleSignals(out, *ids);
  write_AnchorSet(out, *ids);
  write_RegionSets(out, *ids);
  write_Analyses(out, *ids);

  write_closetag(out, "Corpus");
  xercesc_close();
  /* (( BT Patch -- */
  if ( !out.good() ) 
  {
	  string msg = "ATLAS::store(): write error on "+filename;
	  msg += strerror(errno);
	    throw agfio::StoreError(msg);
  }
  /* -- BT Patch ) */
  return "";
}

string
ATLAS::store(const string& filename,
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
    throw agfio::StoreError("ATLAS format:store:no object by the id, " + id);
  }
}

