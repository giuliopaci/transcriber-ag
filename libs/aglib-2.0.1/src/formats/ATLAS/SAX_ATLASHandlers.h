// Sax_ATLASHandlers.h: AIF1 document handler class
// Haejoong Lee
// Copyright (C) 2001,2002 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _SAX_ATLASHANDLERS_H_
#define _SAX_ATLASHANDLERS_H_

#include <string>
#include <map>
#include <ag/Hash.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
using namespace std;
#ifdef XERCES_HAS_CPP_NAMESPACE
XERCES_CPP_NAMESPACE_USE
#endif

/// SAX ATLAS handler class for Xerces C++ XML parser.
class SAX_ATLASHandlers : public HandlerBase
{
public:
  struct sigtab_entry_t {
    map<string,string> MD;
    map<string,string> FMap;
  };

  struct anctab_entry_t {
    double offset;
    string unit;
    string sigid;
    bool anchored;
  };

  struct regtab_entry_t {
    string startid;
    string endid;
  };

  struct children_t {
    string containedType;
    string role;
    vector<string> refs;
  };

  struct anntab_entry_t {
    string type;
    string regid;
    int rchidx;
    int cchidx;
    vector<children_t> childrens;
    map<string,string> content;
    string startid;
    string endid;
    anntab_entry_t(): rchidx(-1), cchidx(-1) {}
  };

  typedef hash_map<string,sigtab_entry_t,hashString,StringEqual> sigtab_t;
  typedef hash_map<string,anctab_entry_t,hashString,StringEqual> anctab_t;
  typedef hash_map<string,regtab_entry_t,hashString,StringEqual> regtab_t;
  typedef hash_map<string,anntab_entry_t,hashString,StringEqual> anntab_t;

private:
  // atlas file info
  map<string,string> CorpusMD;
  sigtab_t SigTab;
  anctab_t AncTab;
  regtab_t RegTab;
  anntab_t AnnTab;
  map<string,vector<string> > AnnSetMap;
  string AnaMapStr;

  // temporary data
  string prevOpenTag;
  string prevCloseTag;
  string currFeature;
  string currValue;
  string currID;
  string RDBCType;
  string RDBCRole;
  bool RDBCflag;
  string CDBCType;
  string CDBCRole;
  bool CDBCflag;
  map<string,string>* currMD;
  anctab_entry_t* currAnctab;
  regtab_entry_t* currRegtab;
  anntab_entry_t* currAnntab;
  vector<string>* currChildren;
  vector<string>* currAnnSet;

  // callbacks
  typedef void (SAX_ATLASHandlers::*StartFP)(const string& name, AttributeList&);
  typedef void (SAX_ATLASHandlers::*EndFP)(const string& name);
  typedef hash_map<string,StartFP,hashString,StringEqual> StartHandlerMap;
  typedef hash_map<string,EndFP,hashString,StringEqual> EndHandlerMap;

  StartHandlerMap shMap;
  EndHandlerMap ehMap;

  void CorpusS(const string& name, AttributeList& attr);
  void OtherMetadataS(const string& name, AttributeList& attr);
  void dcElementS(const string& name, AttributeList& attr);
  void SimpleSignalS(const string& name, AttributeList& attr);
  void SignalRefS(const string& name, AttributeList& attr);
  void AnchorS(const string& name, AttributeList& attr);
  void ParameterS(const string& name, AttributeList& attr);
  void RegionS(const string& name, AttributeList& attr);
  void AnchorRefS(const string& name, AttributeList& attr);
  void AnalysisS(const string& name, AttributeList& attr);
  void AnnotationSetS(const string& name, AttributeList& attr);
  void AnnotationS(const string& name, AttributeList& attr);
  void RegionRefS(const string& name, AttributeList& attr);
  void RegionDefinedByChildrenS(const string& name, AttributeList& attr);
  void ContentS(const string& name, AttributeList& attr);
  void ContentDefinedByChildrenS(const string& name, AttributeList& attr);
  void ChildrenS(const string& name, AttributeList& attr);
  void AnnotationRefS(const string& name, AttributeList& attr);

  void CorpusE(const string& name);
  void OtherMetadataE(const string& name);
  void dcElementE(const string& name);
  void AnchorE(const string& name);
  void ParameterE(const string& name);
  void ChildrenE(const string& name);

public:

  SAX_ATLASHandlers(const string& encoding = "UTF-8");

  // SAX callbacks
  void startElement(const XMLCh* const name, AttributeList& attr);
  void endElement(const XMLCh* const name);
  void characters(const XMLCh* const chars, const unsigned int length);

  void warning(const SAXParseException& exception);
  void error(const SAXParseException& exception);
  void fatalError(const SAXParseException& exception);

  // data provider methods
  map<string,string>& getCorpusMetadata()
  { return CorpusMD; }
  string& getCorpusType()
  { return CorpusMD["_AtlasCorpusType_"]; }
  string& getSchemeLocation()
  { return CorpusMD["_AtlasSchemeLocation_"]; }
  vector<string>& getAnnSetByType(const string& type)
  { return AnnSetMap[type]; }
  bool getAnnSpan(const string& annID, string& ancID1, string& ancID2);
  string& getAnnType(const string& annID)
  { return AnnTab[annID].type; }
  map<string,string>& getAnnContent(const string& annID)
  { return AnnTab[annID].content; }
  vector<children_t>& getAnnChildrens(const string& annID)
  { return AnnTab[annID].childrens; }
  string& getAnnRegID(const string& annID)
  { return AnnTab[annID].regid; }
  string& getAnnStartID(const string& annID)
  { return AnnTab[annID].startid; }
  string& getAnnEndID(const string& annID)
  { return AnnTab[annID].endid; }
  string& getRegStartID(const string& regID)
  { return RegTab[regID].startid; }
  string& getRegEndID(const string& regID)
  { return RegTab[regID].endid; }
  string& getAncSigID(const string& ancID)
  { return AncTab[ancID].sigid; }
  double getAncOffset(const string& ancID)
  { return AncTab[ancID].offset; }
  string& getAncUnit(const string& ancID)
  { return AncTab[ancID].unit; }
  bool getAnchored(const string& ancID)
  { return AncTab[ancID].anchored; }
  map<string,string>& getSigAtt(const string& sigID)
  { return SigTab[sigID].FMap; }
  map<string,string>& getSigMetadata(const string& sigID)
  { return SigTab[sigID].MD; }
};

#endif
