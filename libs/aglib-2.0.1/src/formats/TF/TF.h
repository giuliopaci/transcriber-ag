// TF.h: Table format loader class
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _TF_H_
#define _TF_H_

#include <string>
#include <vector>
#include <ag/agfio_plugin.h>
#include <ag/Record.h>

/// Table Format loader class.
class DllExport TF : public agfio_plugin
{
private:

  vector<string> header;
  string         separator;
  string         smplrate;
  string         annotation_type;

  void
  set_header(const string& header, const string& separator);

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

AGFIO_PLUGIN(TF);

#endif
