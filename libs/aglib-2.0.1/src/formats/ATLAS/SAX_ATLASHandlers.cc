// SAX_ATLASHandlers.cc: AIF1 element handler implementation
// Haejoong Lee
// Copyright (C) 2001,2002 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <iostream>
/* (( BT Patch -- */
#include <ag/AGAPI.h>
/* -- BT Patch ) */
#include <ag/agfio.h>
#include <ag/AGException.h>
#include <ag/RE.h>
#include "SAX_ATLASHandlers.h"
#include "agfXercesUtils.h"

/* (( BT Patch -- */
#define atof myatof
/* -- BT Patch )) */

using namespace std;


SAX_ATLASHandlers::SAX_ATLASHandlers(const string& encoding)
{
  RDBCflag = CDBCflag = false;
  shMap["Corpus"] = &SAX_ATLASHandlers::CorpusS;
  shMap["SimpleSignal"] = &SAX_ATLASHandlers::SimpleSignalS;
  shMap["SignalRef"] = &SAX_ATLASHandlers::SignalRefS;
  shMap["Anchor"] = &SAX_ATLASHandlers::AnchorS;
  shMap["Parameter"] = &SAX_ATLASHandlers::ParameterS;
  shMap["Region"] = &SAX_ATLASHandlers::RegionS;
  shMap["AnchorRef"] = &SAX_ATLASHandlers::AnchorRefS;
  shMap["Analysis"] = &SAX_ATLASHandlers::AnalysisS;
  shMap["AnnotationSet"] = &SAX_ATLASHandlers::AnnotationSetS;
  shMap["Annotation"] = &SAX_ATLASHandlers::AnnotationS;
  shMap["RegionRef"] = &SAX_ATLASHandlers::RegionRefS;
  shMap["RegionDefinedByChildren"] = &SAX_ATLASHandlers::RegionDefinedByChildrenS;
  shMap["Content"] = &SAX_ATLASHandlers::ContentS;
  shMap["ContentDefinedByChildren"] = &SAX_ATLASHandlers::ContentDefinedByChildrenS;
  shMap["Children"] = &SAX_ATLASHandlers::ChildrenS;
  shMap["AnnotationRef"] = &SAX_ATLASHandlers::AnnotationRefS;
  shMap["OtherMetadata"] = &SAX_ATLASHandlers::OtherMetadataS;
  shMap["dc:title"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:creator"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:subject"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:description"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:publisher"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:contributor"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:date"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:type"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:format"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:identifier"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:source"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:language"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:relation"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:coverage"] = &SAX_ATLASHandlers::dcElementS;
  shMap["dc:rights"] = &SAX_ATLASHandlers::dcElementS;

  ehMap["Corpus"] = &SAX_ATLASHandlers::CorpusE;
  ehMap["Anchor"] = &SAX_ATLASHandlers::AnchorE;
  ehMap["Parameter"] = &SAX_ATLASHandlers::ParameterE;
  ehMap["Children"] = &SAX_ATLASHandlers::ChildrenE;
  ehMap["OtherMetadata"] = &SAX_ATLASHandlers::OtherMetadataE;
  ehMap["dc:title"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:creator"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:subject"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:description"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:publisher"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:contributor"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:date"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:type"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:format"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:identifier"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:source"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:language"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:relation"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:coverage"] = &SAX_ATLASHandlers::dcElementE;
  ehMap["dc:rights"] = &SAX_ATLASHandlers::dcElementE;

}

void
SAX_ATLASHandlers::CorpusS(const string& name, AttributeList& attr)
{
  CorpusMD["_AtlasCorpusID_"] = trans(attr.getValue("id"));
  CorpusMD["_AtlasCorpusType_"] = trans(attr.getValue("type"));
  string sl = trans(attr.getValue("schemeLocation"));
  static RE re("^[ \t]*file:(/*)([^ \t]+)");
  if (re.match(trans(attr.getValue("schemeLocation")))) {
    string slash = re.get_matched(1);
    if (slash.size() >= 2)
      CorpusMD["_AtlasSchemeLocation_"] = slash.substr(2) + re.get_matched(2);
    else
      CorpusMD["_AtlasSchemeLocation_"] = slash + re.get_matched(2);
  }
  currMD = &CorpusMD;
}

void
SAX_ATLASHandlers::CorpusE(const string& name)
{
  CorpusMD["_AtlasAnaMapStr_"] = AnaMapStr.substr(1);
}

void
SAX_ATLASHandlers::OtherMetadataS(const string& name, AttributeList& attr)
{
  currFeature = trans(attr.getValue("name")) + "/" + trans(attr.getValue("type"));
  currValue.erase();
}

void
SAX_ATLASHandlers::OtherMetadataE(const string& name)
{
  (*currMD)[currFeature] = currValue;
}

void
SAX_ATLASHandlers::dcElementS(const string& name, AttributeList& attr)
{
  currFeature = name;
  currValue.erase();
}

void
SAX_ATLASHandlers::dcElementE(const string& name)
{
  (*currMD)[currFeature] = currValue;
}

void
SAX_ATLASHandlers::SimpleSignalS(const string& name, AttributeList& attr)
{
  string id = trans(attr.getValue("id"));
  sigtab_entry_t& sigtab = SigTab[id];
  sigtab.MD["_AtlasSignalID_"] = id;
  currMD = &sigtab.MD;
  for (int i=0; i < attr.getLength(); ++i)
    sigtab.FMap[trans(attr.getName(i))] = trans(attr.getValue(i));
}

void
SAX_ATLASHandlers::SignalRefS(const string& name, AttributeList& attr)
{
  currID = trans(attr.getValue("xlink:href")).substr(1);
}

void
SAX_ATLASHandlers::AnchorS(const string& name, AttributeList& attr)
{
  currAnctab = &AncTab[trans(attr.getValue("id"))];
  prevOpenTag = name;
}

void
SAX_ATLASHandlers::AnchorE(const string& name)
{
  currAnctab->sigid = currID;
}

void
SAX_ATLASHandlers::ParameterS(const string& name, AttributeList& attr)
{
  if (prevOpenTag == "Anchor")
    currAnctab->unit = trans(attr.getValue("unit"));
  else if (prevOpenTag == "Content")
    currFeature = trans(attr.getValue("role"));

  currValue.erase();
}

void
SAX_ATLASHandlers::ParameterE(const string& name)
{
  if (prevOpenTag == "Anchor") {
    if (currValue.empty())
      currAnctab->anchored = false;
    else {
      currAnctab->anchored = true;
      currAnctab->offset = atof(currValue.c_str());
    }
  }
  else if (prevOpenTag == "Content")
    currAnntab->content[currFeature] = currValue;
}

void
SAX_ATLASHandlers::RegionS(const string& name, AttributeList& attr)
{
  currRegtab = &RegTab[trans(attr.getValue("id"))];
}

void
SAX_ATLASHandlers::AnchorRefS(const string& name, AttributeList& attr)
{
  if (trans(attr.getValue("role")) == "start")
    currRegtab->startid = trans(attr.getValue("xlink:href")).substr(1);
  else
    currRegtab->endid = trans(attr.getValue("xlink:href")).substr(1);
}

void
SAX_ATLASHandlers::AnalysisS(const string& name, AttributeList& attr)
{
  AnaMapStr +=
    " " + trans(attr.getValue("type")) +
    " " + trans(attr.getValue("role")) +
    " " + trans(attr.getValue("id"));
}

void
SAX_ATLASHandlers::AnnotationSetS(const string& name, AttributeList& attr)
{
  currAnnSet = &AnnSetMap[trans(attr.getValue("containedType"))];
}

void
SAX_ATLASHandlers::AnnotationS(const string& name, AttributeList& attr)
{
  string id = trans(attr.getValue("id"));
  currAnnSet->push_back(id);
  currAnntab = &AnnTab[id];
  currAnntab->type = trans(attr.getValue("type"));
  RDBCType.erase();
  RDBCRole.erase();
}

void
SAX_ATLASHandlers::RegionRefS(const string& name, AttributeList& attr)
{
  currAnntab->regid = trans(attr.getValue("xlink:href")).substr(1);
}

void
SAX_ATLASHandlers::RegionDefinedByChildrenS(const string& name, AttributeList& attr)
{
  RDBCType = trans(attr.getValue("withContainedType"));
  RDBCRole = trans(attr.getValue("withRole"));
}

void
SAX_ATLASHandlers::ContentS(const string& name, AttributeList& attr)
{
  prevOpenTag = name;
  CDBCType.erase();
  CDBCRole.erase();
}

void
SAX_ATLASHandlers::ContentDefinedByChildrenS(const string& name, AttributeList& attr)
{
  CDBCType = trans(attr.getValue("withContainedType"));
  CDBCRole = trans(attr.getValue("withRole"));
}

void
SAX_ATLASHandlers::ChildrenS(const string& name, AttributeList& attr)
{
  string type = trans(attr.getValue("containedType"));
  string role = trans(attr.getValue("role"));

  vector<children_t>& V = currAnntab->childrens;
  int n = V.size();
  V.resize(n+1);
  currChildren = &V[n].refs;
  V[n].containedType = type;
  V[n].role = role;
  
  if (type == RDBCType && role == RDBCRole) {
    currAnntab->rchidx = n;
    RDBCflag = true;
  }
  else if (type == CDBCType && role == CDBCRole) {
    currAnntab->cchidx = n;
    CDBCflag = true;
  }
}

void
SAX_ATLASHandlers::ChildrenE(const string& name)
{
  if (currChildren->empty()) {
    currAnntab->childrens.pop_back();
    if (RDBCflag)
      currAnntab->rchidx = -1;
    else if (CDBCflag)
      currAnntab->cchidx = -1;
  }
  RDBCflag = CDBCflag = false;
}
    
void
SAX_ATLASHandlers::AnnotationRefS(const string& name, AttributeList& attr)
{
  currChildren->push_back(trans(attr.getValue("xlink:href")).substr(1));
}

void
SAX_ATLASHandlers::startElement
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);
  StartHandlerMap::iterator pos = shMap.find(tag);
  if (pos != shMap.end())
    (this->*pos->second)(tag, attr);
}

void
SAX_ATLASHandlers::endElement(const XMLCh* const name)
{
  string tag = trans(name);
  EndHandlerMap::iterator pos = ehMap.find(tag);
  if (pos != ehMap.end())
    (this->*pos->second)(tag);
}

void SAX_ATLASHandlers::characters
(const XMLCh* const chars, const unsigned int length)
{
  currValue += trans(chars);
}

void SAX_ATLASHandlers::warning(const SAXParseException& e)
{
  cerr << "WARNING: " << trans(e.getMessage()) << endl;
}

void SAX_ATLASHandlers::error(const SAXParseException& e)
{
  cerr << "WARNING: " << trans(e.getMessage()) << endl;
}

void SAX_ATLASHandlers::fatalError(const SAXParseException& e)
{
  throw agfio::LoadError(trans(e.getMessage()));
}

bool
SAX_ATLASHandlers::getAnnSpan(const string& annID,
			      string& ancID1,
			      string& ancID2)
{
  anntab_entry_t& anntab = AnnTab[annID];
  if (!anntab.startid.empty()) {
    ancID1 = anntab.startid;
    ancID2 = anntab.endid;
    return true;
  }

  string& regID = anntab.regid;
  if (!regID.empty()) {
    regtab_entry_t& regtab = RegTab[regID];
    ancID1 = regtab.startid;
    ancID2 = regtab.endid;
  }
  else if (anntab.rchidx >= 0) {
    vector<string>& rch = anntab.childrens[anntab.rchidx].refs;
    if (rch.size() != 0) {
      string x;
      getAnnSpan(rch.front(), ancID1, x);
      getAnnSpan(rch.back(), x, ancID2);
    }
  }
  else {
    cerr << "WARNING: annotation " << annID << " has no region defined"
	 << endl;
    return false;
  }
  
  anntab.startid = ancID1;
  anntab.endid = ancID2;
  return true;
}
