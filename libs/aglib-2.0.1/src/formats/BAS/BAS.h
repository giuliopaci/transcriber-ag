// BAS.h: BAS Partitur format file loader class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _BAS_H_
#define _BAS_H_

#include <map>
#include <string>
#include <ag/agfio_plugin.h>
#include "BASfile.h"

/// BAS Partitur format loader class.
class DllExport BAS : public agfio_plugin
{
private:

  void ag_build1(BASfile&, const AGId&, const string&, set<SignalId>&);
  void ag_build1TR(BASfile&, const AGId&, const string&, set<SignalId>&);
  void ag_build1SUP(BASfile&, const AGId&, const string&, set<SignalId>&);
  void ag_build1NOI(BASfile&, const AGId&, const string&, set<SignalId>&);
  void ag_build4(BASfile&, const AGId&, const string&, set<SignalId>&);
  void ag_build4Gab(BASfile&, const AGId&, const string&, set<SignalId>&);
  void ag_build4WOR(BASfile&, const AGId&, const string&, set<SignalId>&);
  void ag_build4MAU(BASfile&, const AGId&, const string&, set<SignalId>&);
  void ag_build5_begin(BASfile&, const AGId&, const string&, set<SignalId>&);
  void ag_build5(BASfile&, const AGId&, const string&, set<SignalId>&);

  map<string, void (BAS::*)(BASfile&, const AGId&, const string&, set<SignalId>&)> funcMap;
  map< string, map<int, AnchorId> > anchorPool;
  map< string, map<int, AnnotationId> > annotationPool;
  map<string, AnchorId> prevAnchor;

  virtual list<AGId>
  load(const string& filename,
       const Id& id = "",
       map<string,string>* signalInfo = NULL,
       map<string,string>* options = NULL)
    throw (agfio::LoadError);

public:

  BAS();

};

AGFIO_PLUGIN(BAS);

#endif
