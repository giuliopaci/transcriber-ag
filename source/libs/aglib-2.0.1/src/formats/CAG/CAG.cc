// CAG.cc: Compact AG format io class implementation
// Haejoong Lee, Steven Bird, Kazuaki Maeda
// Copyright (C) 2001-2002 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <fstream>
#include <ag/AGAPI.h>
#include <ag/RE.h>
#include <ag/Utilities.h>
#if HAVE_ZLIB_H && HAVE_LIBZ
#include "gzstream.h"
#endif
#include "CAG.h"

/* (( BT Patch -- */
#define atof myatof
/* -- BT Patch )) */
using namespace std;

char* const CAG::SEC_AGSET  = "#A";
char* const CAG::SEC_TL     = "#T";
char* const CAG::SEC_SIG    = "#S";
char* const CAG::SEC_AG     = "#G";
char* const CAG::SEC_ANC    = "#V";
char* const CAG::SEC_ANN    = "#E";
char* const CAG::SEC_INC    = "#I";
char* const CAG::SEC_LMAP   = "#ML";    // label map
char* const CAG::SEC_TMAP   = "#MT";    // type map

char* IdMap::charMap = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

string
IdMap::int2id(int n)
{
  string tmp;
  while (n >= 62) {
    tmp += charMap[n%62];
    n /= 62;
  }
  tmp += charMap[n];
  string out;
  string::reverse_iterator pos;
  for (pos=tmp.rbegin(); pos != tmp.rend(); ++pos)
    out += *pos;
  return out;
}

IdMap::IdMap(list<AGId>& agIds)
{
  list<AGId>::iterator agId;
  int agCount = 0;
  for (agId=agIds.begin(); agId != agIds.end(); ++agId) {
    // ag map
    mapFromAG[*agId] = int2id(agCount++);
    // anchor map
    list<AnchorId> ancIds = GetAnchorSet(*agId);
    list<AnchorId>::iterator ancId = ancIds.begin();
    int ancCount = 0;
    for (; ancId!=ancIds.end(); ++ancId)
      mapFromAnc[*ancId] = int2id(ancCount++);
    // feature name map
    set<string> feas = GetAnnotationFeatureNames(*agId);
    set<string>::iterator fea;
    for (fea=feas.begin(); fea != feas.end(); ++fea)
      mapFromFea[*fea] = "";
    // ann type map
    set<string> types = GetAnnotationTypes(*agId);
    set<string>::iterator type;
    for (type=types.begin(); type != types.end(); ++type)
      mapFromAnnType[*type] = "";
  }

  map<FeatureName,string>::iterator pos;
  int feaCount = 0;
  for (pos=mapFromFea.begin(); pos != mapFromFea.end(); ++pos)
    pos->second = int2id(feaCount++);
  int typeCount = 0;
  for (pos=mapFromAnnType.begin(); pos != mapFromAnnType.end(); ++pos)
    pos->second = int2id(typeCount++);
}

/*
IdMap::IdMap(istream& in)
{
  string line, prefix;
  getline(in, line);
  getline(in, prefix);
  prefix += ":";
  while (getline(in, line)) {
    int i = line.find(" ");
    if (i == string::npos)
      break;
    string agid = line.substr(0,i);
    string id = line.substr(i+1);
    mapToAG[id] = prefix + agid;
  }

  getline(in, prefix);
  AGId agId = mapToAG[prefix];
  prefix = agId + ":";
  while (getline(in,line)) {
    int i = line.find(" ");
    if (i == string::npos) {
      if (line != CAG::SEC_ID_ANC)
	break;
      getline(in, prefix);
      agId = mapToAG[prefix];
      prefix = agId + ":";
      continue;
    }
    string ancid = line.substr(0,i);
    string id = line.substr(i+1);
    mapToAnc[agId][id] = prefix + ancid;
  }

  while (getline(in,line)) {
    int i = line.find(" ");
    if (i == string::npos)
      break;
    string agid = line.substr(0,i);
    string id = line.substr(i+1);
    mapToFea[id] = agid;
  }
}

AGIds
IdMap::getAGIds2()
{
  string out;
  map<string,AGId>::iterator pos;
  for (pos=mapToAG.begin(); pos != mapToAG.end(); ++pos)
    out += " " + pos->second;
  return out.substr(1);
}
*/

ostream&
operator<<(ostream& out, IdMap& idMap)
{
  map<string,string>::iterator pos;
  /*
  string prefix = "";
  for (pos=idMap.mapFromAG.begin(); pos != idMap.mapFromAG.end(); ++pos) {
    int i = pos->first.find_last_of(':');
    string ns = pos->first.substr(0,i);
    string id = pos->first.substr(i+1);
    if (prefix != ns) {
      out << CAG::SEC_ID_AG << endl;
      out << ns << endl;
      prefix = ns;
    }
    out << id << " " << pos->second << endl;
  }
  prefix = "";
  for (pos=idMap.mapFromAnc.begin(); pos != idMap.mapFromAnc.end(); ++pos) {
    int i = pos->first.find_last_of(':');
    string ns = pos->first.substr(0,i);
    string id = pos->first.substr(i+1);
    if (prefix != ns) {
      out << CAG::SEC_ID_ANC << endl;
      out << idMap.mapFromAG[ns] << endl;
      prefix = ns;
    }
    out << id << " " << pos->second << endl;
  }
  */
  out << CAG::SEC_LMAP << endl;
  for (pos=idMap.mapFromFea.begin(); pos != idMap.mapFromFea.end(); ++pos)
    out << pos->first << " " << pos->second << endl;
  out << CAG::SEC_TMAP << endl;
  for (pos=idMap.mapFromAnnType.begin();
       pos != idMap.mapFromAnnType.end(); ++pos)
    out << pos->first << " " << pos->second << endl;
  return out;
}

static string
encode(const string& in)
{
  string out;
  for (int i=0; i < in.size(); ++i) {
    char c = in.at(i);
    if (c == '\\')
      out.append("\\\\");
    else if (c == '\n')
      out.append("\\n");
    else if (c == ';')
      out.append("\\;");
    else
      out += c;
  }
  return out;
}

static string
decode(const string& in)
{
  string out;
  for (int i=0; i < in.size(); ++i) {
    char c = in.at(i);
    if (c == '\\') {
      char d = in.at(++i);
      if (d == c)
	out += c;
      else if (d == 'n')
	out += '\n';
      else if (d == ';')
	out += ';';
      else
	throw string("corrupted?");
    }
    else
      out += c;
  }
  return out;
}

inline static string
local_id(const string& id)
{
  return id.substr(id.find_last_of(":")+1);
}

static void
writeMetadata(const string& id, ostream& out)
{
  map<FeatureName,FeatureValue> features = GetFeatures(id);
  map<FeatureName,FeatureValue>::iterator feature = features.begin();
  for (; feature != features.end(); ++feature)
    out << feature->first << " "
	<< encode(feature->second) << endl;
}

static void
writeSignals(const TimelineId& tlId, ostream& out)
{
  set<SignalId> sigIds = GetSignals(tlId);
  set<SignalId>::iterator sigId = sigIds.begin();
  for (; sigId != sigIds.end(); ++sigId) {
    const string& s = *sigId;
    out << CAG::SEC_SIG << endl
	<< local_id(s) << " "
	<< GetSignalXlinkHref(s) << " "
	<< GetSignalMimeClass(s) << " "
	<< GetSignalMimeType(s) << " "
	<< GetSignalEncoding(s) << " "
	<< GetSignalUnit(s) << " "
	<< GetSignalTrack(s) << endl;
    writeMetadata(s, out);
  }
}

static void
writeTimelines(list<AGId>& agIds, ostream& out)
{
  set<TimelineId> tlSet;
  list<AGId>::iterator agId;
  for (agId=agIds.begin(); agId != agIds.end(); ++agId)
    tlSet.insert(GetTimelineId(*agId));
  set<TimelineId>::iterator tlSetIt;
  for (tlSetIt=tlSet.begin(); tlSetIt != tlSet.end(); ++tlSetIt) {
    out << CAG::SEC_TL << endl
	<< tlSetIt->substr(tlSetIt->find_last_of(":")+1) << endl;
    writeMetadata(*tlSetIt, out);
    writeSignals(*tlSetIt, out);
  }
}

static void
writeAnchors(list<AGId>& agIds, ostream& out)
{
  list<AGId>::iterator agId;
  for (agId=agIds.begin(); agId != agIds.end(); ++agId) {
    out << CAG::SEC_AG << endl;
    out << local_id(*agId) << " "
	<< local_id(GetTimelineId(*agId)) << endl;
    writeMetadata(*agId, out);

    list<AnchorId> ids = GetAnchorSet(*agId);
    list<AnchorId>::iterator id = ids.begin();
    map<string,map<set<string>,set<AnchorId> > > byUNS; // by Unit aNd Signals
    for (; id != ids.end(); ++id)
      byUNS[GetOffsetUnit(*id)][GetAnchorSignalIds(*id)].insert(*id);
    map<string,map<set<string>,set<AnchorId> > >::iterator byUNSIt;
    for (byUNSIt = byUNS.begin(); byUNSIt != byUNS.end(); ++byUNSIt) {
      map<set<string>,set<AnchorId> >::iterator bySigIt;
      for (bySigIt = byUNSIt->second.begin();
	   bySigIt != byUNSIt->second.end();
	   ++bySigIt) {
	out << CAG::SEC_ANC << endl;
	out << byUNSIt->first << " ";
	set<SignalId>::iterator sigIt = bySigIt->first.begin();
	if (sigIt != bySigIt->first.end()) {
	  out << *sigIt;
	  for (++sigIt; sigIt != bySigIt->first.end(); ++sigIt) 
	    out << " " << *sigIt;
	}
	out << endl;
	set<AnchorId>::iterator ancSetIt;
	for (ancSetIt = bySigIt->second.begin();
	     ancSetIt != bySigIt->second.end();
	     ++ancSetIt) {
	  out << local_id(*ancSetIt) << " ";
	  if (GetAnchored(*ancSetIt))
	    out << GetAnchorOffset(*ancSetIt);
	  out << endl;
	}
      }
    }
  }
}

static void
writeFeature(const string& id, ostream& out, IdMap& idMap)
{
  set<string> names = GetFeatureNames(id);
  set<string>::iterator name = names.begin();
  for (; name != names.end(); ++name) {
    out << idMap.fromFeatureName(*name) << " "
	<< encode(GetFeature(id,*name)) << endl;
  }
}

static void
writeAnnotations(list<AGId>& agIds, ostream& out, string types="")
{
  IdMap idMap(agIds);
  out << idMap;
  out << CAG::SEC_ANN << endl;

  list<AGId>::iterator agId;
  for (agId=agIds.begin(); agId != agIds.end(); ++agId) {
    string agId2 = local_id(*agId);
    string type;
    string ids;
    if (types.empty()) {
      list<AnnotationId> ids = GetAnnotationSeqByOffset(*agId);
      list<AnnotationId>::iterator idp = ids.begin();
      for (; idp != ids.end(); ++idp) {
	const string& id = *idp;
	out << agId2 << " "
	    << local_id(id) << " "
	    << local_id(GetStartAnchor(id)) << " "
	    << local_id(GetEndAnchor(id)) << " "
	    << idMap.fromAnnotationType(GetAnnotationType(id));
	set<string> names = GetFeatureNames(id);
	set<string>::iterator name = names.begin();
	for (; name != names.end(); ++name) {
	  out << ";" << idMap.fromFeatureName(*name) << " "
	      << encode(GetFeature(id,*name));
	}
	out << endl;
      }
    }
    else {
      while ((type=Utilities::next_tok(types)) != "") {
	set<AnnotationId> ids = GetAnnotationSet(*agId, type);
	set<AnnotationId>::iterator idp = ids.begin();
	for (; idp != ids.end(); ++idp) {
	  const string& id = *idp;
	  out << agId2 << " "
	      << local_id(id) << " "
	      << local_id(GetStartAnchor(id)) << " "
	      << local_id(GetEndAnchor(id)) << " "
	      << idMap.fromAnnotationType(GetAnnotationType(id));
	  set<string> names = GetFeatureNames(id);
	  set<string>::iterator name = names.begin();
	  for (; name != names.begin(); ++name) {
	    out << ";" << idMap.fromFeatureName(*name) << " "
		<< encode(GetFeature(id,*name));
	  }
	  out << endl;
	}
      }
    }
  }
}

/*
static void
writeAG(const AGId& agId, ostream& out, IdMap& idMap)
{
  string t = GetTimelineId(agId);
  out << CAG::SEC_AG << endl;
  out << agId.substr(agId.find_last_of(":")+1) << " "
      << t.substr(t.find_last_of(":")+1) << endl;
  writeAnchors(agId, out, idMap);
  writeAnnotations(agId, out, idMap);
}
*/

static void
writeDictionary(list<AGId>& agIds, ostream& out)
{
  list<AGId>::iterator first = agIds.begin();
  AGSetId agsetId = first->substr(0,first->find(":"));
  out << CAG::SEC_AGSET << endl;
  out << agsetId << endl;
  writeMetadata(agsetId, out);
  writeTimelines(agIds, out);
  writeAnchors(agIds, out);
}
  
string
CAG::store(const string& filename,
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
    throw agfio::StoreError("CAG format:store:no object by the id, " + id);
  }
}

string
CAG::store(const string& filename,
	   list<string>* const ids,
	   map<string,string>* options)
  throw (agfio::StoreError)
{
  if (!ids) return "";

  ostream *out, *out1, *out2;

  string dic, ann, inc, types;
  if (options) {
    dic = (*options)["dictionary"];
    ann = (*options)["annotation"];
    inc = (*options)["include"];
    types = (*options)["types"];
  }

  if (dic.empty()) {
    if (ann.empty()) {          // (1) ~dic & ~ann : one file
#if HAVE_ZLIB_H && HAVE_LIBZ
      if (options != NULL && (*options)["compress"] == "true")
	out = new ogzstream(filename.c_str());
      else
#endif
	out = new ofstream(filename.c_str());
      if (out->bad())
	throw agfio::StoreError("CAG::store():can't open file: " + filename);

      writeDictionary(*ids, *out);
      writeAnnotations(*ids, *out, types);
      delete out;
    }
    else if (inc.empty())       // (2) ~dic & ann & ~inc : error
      throw agfio::StoreError("CAG::store():include file should be specified in the options");
    else {                      // (3) ~dic & ann & inc : ann only
      // open file
#if HAVE_ZLIB_H && HAVE_LIBZ
      if (options != NULL && (*options)["compress"] == "true")
	out = new ogzstream(ann.c_str());
      else
#endif
	out = new ofstream(ann.c_str());
      if (out->bad())
	throw agfio::StoreError("CAG::store():can't open file: " + ann);
      // write
      *out << CAG::SEC_INC << endl
	   << inc << endl;
      writeAnnotations(*ids, *out, types);
      delete out;
    }
  }
  else if (ann.empty()) {       // (4) dic && ~ann : dic only
    // open file
#if HAVE_ZLIB_H && HAVE_LIBZ
    if (options != NULL && (*options)["compress"] == "true")
      out = new ogzstream(dic.c_str());
    else
#endif
      out = new ofstream(dic.c_str());
    if (out->bad())
      throw agfio::StoreError("CAG::store():can't open file: " + dic);
    // write
    writeDictionary(*ids, *out);
    delete out;
  }
  else {                        // (5) dic && ann : two files
    // open files
#if HAVE_ZLIB_H && HAVE_LIBZ
    if (options != NULL && (*options)["compress"] == "true") {
      out1 = new ogzstream(dic.c_str());
      out2 = new ogzstream(ann.c_str());
    }
    else {
#endif
      out1 = new ofstream(dic.c_str());
      out2 = new ofstream(ann.c_str());
#if HAVE_ZLIB_H && HAVE_LIBZ
    }
#endif
    if (out1->bad())
      throw agfio::StoreError("CAG::store():can't open file: " + dic);
    if (out2->bad())
      throw agfio::StoreError("CAG::store():can't open file: " + ann);
    // write
    writeDictionary(*ids, *out1);
    *out2 << CAG::SEC_INC << endl
	  << dic << endl;
    writeAnnotations(*ids, *out2, types);
    delete out1, out2;
  }

  return "";
}

static string
mySetFeature(const string& id, istream& in)
{
  string line;
  while (getline(in,line)) {
    int i = line.find_first_of(" ");
    if (i == string::npos)
      return line;
    string name = line.substr(0,i);
    string val = decode(line.substr(i+1));
    SetFeature(id, name, val);
  }
  return "";
}

static void
mySetFeature2(const AnnotationId& id, const string& f, map<string,string>& m)
{
  string name, val;
  bool valmode = false;
  int s = 0;
  char d;
  for (int i=0; i < f.size(); ++i) {
    char c = f.at(i);

    if (valmode) {
      switch (c) {
      case '\\':
	d = f.at(++i);
	if (d == 'n')
	  val += '\n';
	else
	  val += d;
	break;
      case ';':
	SetFeature(id, m[name], val);
	val.erase();
	valmode = false;
	s = i+1;
	break;
      default:
	val += c;
      }
    }
    else {
      i = f.find(" ", s);
      name = f.substr(s, i-s);
      valmode = true;
    }
  }
  SetFeature(id, m[name], val);
}

static string currAGSetId;
static string currTLId;
static string currAGId;
static list<string> currAGIds;

static string
mkAGSet(istream& in)
{
  string line;
  getline(in, currAGSetId);
  CreateAGSet(currAGSetId);
  return mySetFeature(currAGSetId, in);
}

static string
mkSignals(istream& in)
{
  string line;
  getline(in, line);
  RE pat("^([^ ]+) ([^ ]+) ([^ ]+) ([^ ]+) ([^ ]+) ([^ ]+) ([^ ]*)$");
  if (!pat.match(line))
    throw ("format error: " + line);
  SignalId sigId = currTLId + ":" + pat.get_matched(1);
  CreateSignal(sigId,
	       pat.get_matched(2),
	       pat.get_matched(3),
	       pat.get_matched(4),
	       pat.get_matched(5),
	       pat.get_matched(6),
	       pat.get_matched(7));
  return mySetFeature(sigId, in);
}

static string
mkTimeline(istream& in)
{
  string line;
  getline(in, currTLId);
  currTLId = currAGSetId + ":" + currTLId;
  CreateTimeline(currTLId);
  return mySetFeature(currTLId, in);
}

static string
mkAG(istream& in)
{
  string line;
  getline(in, line);
  RE pat("^([^ ]+) ([^ ]*)$");
  if (!pat.match(line))
    throw ("format error: " + line);
  currAGId = currAGSetId + ":" + pat.get_matched(1);
  CreateAG(currAGId, currAGSetId+":"+pat.get_matched(2));
  currAGIds.push_back(currAGId);
  return mySetFeature(currAGId, in);
}

inline static string
mkAnchors(istream& in)
{
  string line;
  getline(in, line);
  static RE pat("^([^ ]*) (.*)$");
  if (!pat.match(line))
    throw ("format error: " + line);
  string unit = pat.get_matched(1);
  string id, ids = pat.get_matched(2);
  set<SignalId> sigIds;
  while ((id=Utilities::next_tok(ids)) != "")
    sigIds.insert(id);
  while (getline(in,line)) {
    int i = line.find_first_of(" ");
    if (i == string::npos)
      return line;
    AnchorId ancId = currAGId + ":" + line.substr(0,i);
    string ostr = line.substr(i+1);
    if (ostr.empty())
      CreateAnchor(ancId, sigIds);
    else {
      double offset = atof(line.substr(i+1).c_str());
      CreateAnchor(ancId, offset, unit, sigIds);
    }
  }
  return "";
}

static string
mkAnnotations(istream& in)
{
  string line;
  map<string,string> lbMap;
  while (getline(in,line)) {
    int i = line.find(" ");
    if (i == string::npos)
      break;
    lbMap[line.substr(i+1)] = line.substr(0,i);
  }
  //  if (line != CAG::SEC_TMAP)
  //    throw "format error:no annotation type map";
  map<string,string> typeMap;
  while (getline(in,line)) {
    int i = line.find(" ");
    if (i == string::npos)
      break;
    typeMap[line.substr(i+1)] = line.substr(0,i);
  }
  RE pat("^([^ ]+) ([^ ]+) ([^ ]+) ([^ ]+) ([^ ]+)$");
  while (getline(in,line)) {
    int i = line.find(";");
    if (i == string::npos)
      return line;
    else if (!pat.match(line.substr(0,i)))
      throw ("format error: " + line);
    AGId agId = currAGSetId + ":" + pat.get_matched(1);
    AnnotationId annId = agId + ":" + pat.get_matched(2);
    AnchorId sancId = agId + ":" + pat.get_matched(3);
    AnchorId eancId = agId + ":" + pat.get_matched(4);
    AnnotationType annType = typeMap[pat.get_matched(5)];
    CreateAnnotation(annId, sancId, eancId, annType);
    mySetFeature2(annId, line.substr(i+1), lbMap);
  }
  return "";
}

list<AGId>
CAG::load(const string& filename,
	  const Id& id,
	  map<string,string>* signalInfo,
	  map<string,string>* options)
  throw (agfio::LoadError)
{
  if (callLevel == 0) {
    incSet.clear();
    currAGIds.clear();
  }

  istream* in;
#if HAVE_ZLIB_H && HAVE_LIBZ
  if (options != NULL && (*options)["compress"] == "true")
    in = new igzstream(filename.c_str());
  else
#endif
    in = new ifstream(filename.c_str());

  if (in->bad())
    throw agfio::LoadError("CAG::load():can't open " + filename);

  //IdMap idMap(*in);
  string secMark;
  if (!getline(*in, secMark)) {     // empty file
    delete in;
    return list<AGId>();
  }

  try {
    if (secMark == SEC_INC) {       // ann file
      string tok;
      while (getline(*in,tok) && tok != SEC_LMAP) {
	if (incSet.insert(tok).second) {
	  callLevel++;
	  load(tok);
	  callLevel--;
	}
      }
      mkAnnotations(*in);
      delete in;
      return currAGIds;
    }
    
    secMark = mkAGSet(*in);
    while (secMark == SEC_TL) {
      secMark = mkTimeline(*in);
      while (secMark == SEC_SIG)
	secMark = mkSignals(*in);
    }
    while (secMark == SEC_AG) {
      secMark = mkAG(*in);
      while (secMark == SEC_ANC)
	secMark = mkAnchors(*in);
    }
    if (secMark == SEC_LMAP)
      secMark = mkAnnotations(*in);

    delete in;
    return currAGIds;
  }
  catch (const string& err) {
    throw agfio::LoadError("CAG::load(): " + err);
  }
}
