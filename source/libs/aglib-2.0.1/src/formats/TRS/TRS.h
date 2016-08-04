// TRS.h: TRS loader class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _TRS_H_
#define _TRS_H_

#include <ag/agfio_plugin.h>
#include <xercesc/parsers/SAXParser.hpp>

/// TRS/AIF format loader class.

class TRS : public agfio_plugin
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


AGFIO_PLUGIN(TRS);

#endif
