// AG.h: AG loader class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _AG_H_
#define _AG_H_

#include <ag/agfio_plugin.h>
#include <xercesc/parsers/SAXParser.hpp>

/// AIF format loader class.
namespace ns1 { // to prevent conflict (class AG already exists)

class AG : public agfio_plugin
{

private:

  virtual list<AGId>
  load(const string& filename,
       const Id& id = "",
       map<string,string>* signalInfo = NULL,
       map<string,string>* options = NULL)
    throw (agfio::LoadError);

  virtual string
  store(const string& filename,
        const string& id,
        map<string,string>* options = NULL)
    throw (agfio::StoreError);
};

}

AGFIO_PLUGIN(ns1::AG);

#endif
