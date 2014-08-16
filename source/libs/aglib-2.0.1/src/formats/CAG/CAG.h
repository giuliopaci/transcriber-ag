// CAG.h: Compact AG format io class
// Haejoong Lee, Steven Bird, Kazuaki Maeda
// Copyright (C) 2001-2002 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _CAG_H_
#define _CAG_H_

#if defined(__GNUC__) && __GNUC__ < 3
  #include <ostream.h>
#else
  #include <ostream>
#endif
#include <ag/agfio_plugin.h>

/// Compact AG class
class DllExport CAG : public agfio_plugin
{

private:

  set<string> incSet;
  int callLevel;

public:

  static char* const SEC_AGSET;
  static char* const SEC_TL;
  static char* const SEC_SIG;
  static char* const SEC_AG;
  static char* const SEC_ANC;
  static char* const SEC_ANN;
  static char* const SEC_INC;
  static char* const SEC_LMAP;
  static char* const SEC_TMAP;

  CAG(): callLevel(0) {}

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

class IdMap
{
private:
  map<AGId,string> mapFromAG;
  map<AnchorId,string> mapFromAnc;
  map<FeatureName,string> mapFromFea;
  map<AnnotationType,string> mapFromAnnType;
  static char* charMap;
  string int2id(int n);

  map<string,AGId> mapToAG;
  map<AGId,map<string,AnchorId> > mapToAnc;
  map<string,string> mapToFea;

public:
  IdMap(list<AGId>& agIds);

  IdMap(istream& in);

  string fromAGId(const AGId& agId)
  { return mapFromAG[agId]; }

  string fromAnchorId(const AnchorId& anchorId)
  { return mapFromAnc[anchorId]; }

  string fromFeatureName(const string& featureName)
  { return mapFromFea[featureName]; }

  string fromAnnotationType(const string& AnnotationType)
  { return mapFromAnnType[AnnotationType]; }

  friend ostream& operator<<(ostream& out, IdMap& idMap);

  AGId toAGId(const string& id)
  { return mapToAG[id]; }

  AnchorId toAnchorId(const AGId& agId, const string& id)
  { return mapToAnc[agId][id]; }

  FeatureName toFeatureName(const string& id)
  { return mapToFea[id]; }

  //AGIds getAGIds2();
};

AGFIO_PLUGIN(CAG);

#endif
