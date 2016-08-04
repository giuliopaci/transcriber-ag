// TIMIT.h: TIMIT loader class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _TIMIT_H_
#define _TIMIT_H_

#include <ag/agfio_plugin.h>
#include <ag/Paired.h>
#include <set>
using namespace std;

/// TIMIT loader class.
class DllExport TIMIT : public agfio_plugin
{

private:

  // Add Annotataion to an exist AG.
  // Assume that Anchors needed already exist.
  void
  add_annotation(Paired &pair,
		 const string& type,
		 const AGId &agId,
		 const Unit& unit,
		 set<SignalId>& signalIds);

  virtual list<AGId>
  load(const string& filename,
       const Id& id = "",
       map<string,string>* signalInfo = NULL,
       map<string,string>* options = NULL)
    throw (agfio::LoadError);

};

AGFIO_PLUGIN(TIMIT);

#endif
