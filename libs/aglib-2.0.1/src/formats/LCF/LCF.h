// LCF.h: LCD Callhome Format loader class
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _LCF_H_
#define _LCF_H_

#include <ag/agfio_plugin.h>

/// LDC Callhome Format loader class.
class LCF : public agfio_plugin
{
public:

  virtual list<AGId>
  load(const string& filename,
       const Id& id = "",
       map<string,string>* signalInfo = NULL,
       map<string,string>* options = NULL)
    throw (agfio::LoadError);

  /// Annotation writer interface.
  /**
   * @param filename
   *    The name of the file where the AG's are stored.
   * @param agIds
   *    Ids of AG's to store.
   * @param options
   *    Not used.
   */
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

AGFIO_PLUGIN(LCF);

#endif
