// ATLAS_load.cc: AIF Level 1 loader implementation
// Haejoong Lee, Kazuaki Maeda, Steven Bird, Xiaoyi Ma
// Copyright (C) 2001,2002 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <assert.h>
#include <ag/Hash.h>
#include <ag/AGAPI.h>
#include <ag/RE.h>
#include <ag/Utilities.h>
#include "ATLAS.h"
#include "SAX_ATLASHandlers.h"
#include "agfXercesUtils.h"
#include <cstdio>
/* (( BT Patch -- */
#include <errno.h>
/* -- BT Patch )) */

AGFIO_PLUGIN(ATLAS);

static list<AGId>
toAG(AGSetId, const string&, string, string, const string&);

list<AGId>
ATLAS::load(const string& filename,
            const Id& agsetId,
            map<string,string>* signalInfo,
            map<string,string>* options)
  throw (agfio::LoadError)
{
  try {
    xercesc_open();
  }
  catch (const agfioError& e) {
    throw agfio::LoadError(string("ATLAS:") + e.what());
  }

  list<AGId> result = (options==NULL) ?
    toAG(agsetId, filename, "", "", ""):
    toAG(agsetId,
	 filename,
	 (*options)["MAIA"],
	 (*options)["agann"],
	 (*options)["DTDvalidation"]);

  xercesc_close();

  return result;
}

namespace ATLAS_load {
  #define INIT -1
  #define FRST 1
  #define SCND 2
  struct anntab_entry_t {
    AnnotationId annoid;
    AnchorId start;
    AnchorId end;
    short int mark;
    anntab_entry_t(): mark(INIT) {}
  };

  class hashDouble {
  public:
    size_t operator()(const double& x) const {
      static hash<char*> h;
      static char s[36];
      sprintf(s, "%.15e", x);
      return h(s);
    }
  };
  class doubleEqual {
  public:
    bool operator()(const double& x, const double& y) const {
      return x == y;
    }
  };
  typedef hash_map<string,anntab_entry_t,hashString,StringEqual> anntab_t;
};
using namespace ATLAS_load;

static map<string,map<string,DOMNode*> > MAIAMap;
static short int CURR;
static hash_map<string,anntab_entry_t,hashString,StringEqual> AnnTab;
static hash_map<AGId,hash_map<string,AnchorId,hashString,StringEqual>,hashString,StringEqual> AncPool;
// can't use GetAnchorSetByOffset because there no order between selected
// anchors
static SAX_ATLASHandlers* dp;


static void
build_MAIAMap(const string& maia_file, bool validation)
{
  DOMDocument* doc = agfDOMParse(maia_file, validation);
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

static void
get_top_ann_types(const string& cor_type, set<string>& S)
{
  map<string,bool> AnnTypes;
  DOMNode* corpus = MAIAMap["CorpusType"][cor_type];
  DOMNodeList* N;   // general use

  N = elts_by_name(corpus, "AnalysisType");
  for (int i=0; i < N->getLength(); ++i) {
    string ana_name = node_att(N->item(i), "ref");
    DOMNodeList* M =
      elts_by_name(MAIAMap["AnalysisType"][ana_name], "AnnotationType");
    for (int j=0; j < M->getLength(); ++j) {
      string ann_name = node_att(M->item(j), "ref");
      AnnTypes[ann_name] = false;
    }
  }

  map<string,bool>::iterator pos;
  for (pos=AnnTypes.begin(); pos != AnnTypes.end(); ++pos) {
    N = elts_by_name(MAIAMap["AnnotationType"][pos->first],"ChildrenType");
    for (int i=0; i < N->getLength(); ++i) {
      DOMNode* ChType = MAIAMap["ChildrenType"][node_att(N->item(i),"ref")];
      AnnTypes[node_att(first_node(ChType),"ref")] = true;
    }
  }

  for (pos=AnnTypes.begin(); pos != AnnTypes.end(); ++pos)
    if (pos->second == false)
      S.insert(pos->first);
}

/*
static AnchorId
get_anchor(const AGId& ag,
	   const double& offset,
	   const string& unit,
	   const string& sig,
	   bool dup = false)
{
  AnchorId& a = AncPool[ag][offset];
  if (a.empty() || dup) {
    set<SignalId> sigset;
    Utilities::string2set(sig, sigset);
    a = CreateAnchor(ag, offset, unit, sigset);
  }
  return a;
}
*/


inline static AnchorId&
min_anchor(AnchorId& a, AnchorId& b)
{
  assert(!a.empty() || !b.empty());
  if (a.empty())
    return b;
  else if (b.empty())
    return a;
  else if (GetAnchorOffset(a) < GetAnchorOffset(b))
    return a;
  else
    return b;
}

inline static AnchorId&
max_anchor(AnchorId& a, AnchorId& b)
{
  assert(!a.empty() || !b.empty());
  if (a.empty())
    return b;
  else if (b.empty())
    return a;
  else if (GetAnchorOffset(a) < GetAnchorOffset(b))
    return b;
  else
    return a;
}

inline static void
copy_content(const string& annID, const AnnotationId& anno)
{
  map<string,string>& feature = dp->getAnnContent(annID);
  map<string,string>::iterator pos = feature.begin();
  for (; pos != feature.end(); ++pos)
    SetFeature(anno, pos->first, pos->second);
}

static AnnotationId
create_anno(const string& annID,
	    const AGId& ag)
{
  anntab_entry_t& anntab = AnnTab[annID];
  AnchorId& start = anntab.start;
  AnchorId& end = anntab.end;

  if (start.empty() || end.empty() || anntab.mark!=INIT)
    return "";
  if (GetAnchored(start) && GetAnchored(end) &&
      GetAnchorOffset(start) > GetAnchorOffset(end))
      cerr << "WARNING: reversed offsets: " << annID << endl;

  AnnotationId anno =
    CreateAnnotation(ag, start, end, dp->getAnnType(annID));
  copy_content(annID, anno);
  SetFeature(anno, "_AtlasAnnID_", annID);
  string& regid = dp->getAnnRegID(annID);
  SetFeature(anno, "_AtlasRegID_", regid);
  SetFeature(anno, "_AtlasStartAncID_", dp->getRegStartID(regid));
  SetFeature(anno, "_AtlasEndAncID_", dp->getRegEndID(regid));
  anntab.mark = CURR;
  anntab.annoid = anno;
  return anno;
}

static SignalId
create_signal(const string& sigID, const TimelineId& tl)
{
  map<string,string>& att = dp->getSigAtt(sigID);
  SignalId ag_sig = CreateSignal(tl,
				 att["xlink:href"],
				 att["mimeClass"],
				 att["mimeType"],
				 att["encoding"],
				 "null",
				 att["track"]);
  SetFeature(ag_sig, "_AtlasSignalID_", sigID);
  map<string,string>& md = dp->getSigMetadata(sigID);
  map<string,string>::iterator pos = md.begin();
  for (; pos != md.end(); ++pos)
    SetFeature(ag_sig, pos->first, pos->second);
  return ag_sig;
}

static AnchorId
create_anchor(const string& anc_id,
	      const AGId& ag,
	      const TimelineId& tl,
	      map<string,SignalId>& sig_map)
  // create an ag anchor using the information of an ATLAS Anchor
{
  AnchorId& a = AncPool[ag][anc_id];
  if (a.empty()) {
    SignalId sig;
    set<SignalId> sigset;
    string& atlas_sig = dp->getAncSigID(anc_id);
    map<string,SignalId>::iterator pos = sig_map.find(atlas_sig);
    if (pos != sig_map.end())
      Utilities::string2set(pos->second, sigset);
    else {
      sig = create_signal(atlas_sig, tl);
      sig_map[atlas_sig] = sig;
      Utilities::string2set(sig, sigset);
    }

    if (dp->getAnchored(anc_id))
      a = CreateAnchor(ag, dp->getAncOffset(anc_id), dp->getAncUnit(anc_id), sigset);
    else
      a = CreateAnchor(ag, sigset);
  }
  return a;
}

static string
process_ann(const string& annID,
	    const AGId& ag,
	    const TimelineId& tl,
	    map<string,SignalId>& sig_map);

static string
process_children(SAX_ATLASHandlers::children_t& children,
		 const AGId& ag,
		 const TimelineId& tl,
		 map<string,SignalId>& sig_map)
{
  vector<string>& chlist = children.refs;
  string ch_str_local;
  for (int i=0; i < chlist.size(); ++i) {
    string& annid = chlist[i];
    anntab_entry_t& child = AnnTab[annid];

    AnnotationId anno = process_ann(annid, ag, tl, sig_map);

    // anno is empty if process_ann fails
    // process_ann fails if the ann has no region defined
    if (! anno.empty())
      ch_str_local += " " + anno;
  }

  if (ch_str_local.empty())
    return "";
  else
    return " ; " + children.containedType + " " + children.role + " " + ch_str_local.substr(1);
}

static AnnotationId
process_ann(const string& annID,
	    const AGId& ag,
	    const TimelineId& tl,
	    map<string,SignalId>& sig_map)
{
  if (AnnTab[annID].mark != INIT)
    return AnnTab[annID].annoid;

  vector<SAX_ATLASHandlers::children_t>& childrens =
    dp->getAnnChildrens(annID);
  string ch_str;
  for (int i=0; i < childrens.size(); ++i) {
    SAX_ATLASHandlers::children_t& children = childrens[i];
    if (children.refs.size() == 0)
      continue;
    string ch_str_local = process_children(children, ag, tl, sig_map);
    ch_str += ch_str_local;
  }

  string anc1, anc2;
  if (dp->getAnnSpan(annID, anc1, anc2) == false)
    return "";
  AnchorId reg_start = create_anchor(anc1, ag, tl, sig_map);
  AnchorId reg_end = create_anchor(anc2, ag, tl, sig_map);
  if (reg_start == reg_end) {
    reg_end = SplitAnchor(reg_start);
    AncPool[ag][anc2] = reg_end;
  }
  anntab_entry_t& anntab = AnnTab[annID];
  anntab.start = reg_start;
  anntab.end = reg_end;
  AnnotationId anno = create_anno(annID, ag);
  if (! ch_str.empty())
    SetFeature(anno, "_AtlasAnnChil_", ch_str.substr(3));
  return anno;
}

static list<AGId>
toAG(AGSetId agsetid,
     const string& atlas_file,
     string maia_file,
     string agann,
     const string& validation)
{
  bool val_opt;
  if (validation == "true")
    val_opt = true;
  else if (validation == "false")
    val_opt = false;
  else
    val_opt = true;

  SAX_ATLASHandlers handler;
  agfSAXParse(&handler, atlas_file, val_opt);
  dp = &handler;

  if (maia_file.empty()) {
    maia_file = dp->getSchemeLocation();
    if (maia_file.empty())
      throw agfio::LoadError("ATLAS_load.cc:toAG():no MAIA file given");
  }
  build_MAIAMap(maia_file, val_opt);

  // agset
  if (agsetid.empty())
    agsetid = CreateAGSet("ATLAS");
  else if (! ExistsAGSet(agsetid))
    CreateAGSet(agsetid);

  {
  map<string,string>& CorpusMetadata = dp->getCorpusMetadata();
  map<string,string>::iterator pos = CorpusMetadata.begin();
  for (; pos != CorpusMetadata.end(); ++pos)
    SetFeature(agsetid, pos->first, pos->second);
  }

  set<string> top_ann_types;
  get_top_ann_types(dp->getCorpusType(), top_ann_types);
  top_ann_types.erase(agann);


  CURR = FRST;
  list<AGId> ags;

  vector<string>& aganns = dp->getAnnSetByType(agann);
  for (int i=0; i < aganns.size(); ++i) {
    string& annid = aganns[i];
    TimelineId tl = CreateTimeline(agsetid);
    AGId ag = CreateAG(agsetid, tl);
    map<string,SignalId> sig_map;
    process_ann(annid, ag, tl, sig_map);
    ags.push_back(ag);
  }

  CURR = SCND;

  for (set<string>::iterator pos = top_ann_types.begin();
       pos != top_ann_types.end(); ++pos) {

    const AnnotationType& ann_type = *pos;
    vector<string>& annset = dp->getAnnSetByType(*pos);
    
    for (int i=0; i < annset.size(); ++i) {
      
      string& annid = annset[i];
      TimelineId tl = CreateTimeline(agsetid);
      AGId ag = CreateAG(agsetid, tl);
      map<string,SignalId> sig_map;
      process_ann(annid, ag, tl, sig_map);
      ags.push_back(ag);
    }
  }

  return ags;
}
