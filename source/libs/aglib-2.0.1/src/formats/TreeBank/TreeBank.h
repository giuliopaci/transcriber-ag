// TreeBank.h: TreeBank file loader class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _TREEBANK_H_
#define _TREEBANK_H_

#include <fstream>
#include <ag/agfio_plugin.h>
#include "TBEscapeHandler.h"
#include "TB_UPenn_WSJ_EscapeHandler.h"

/// TreeBank loader class.
class DllExport TreeBank : public agfio_plugin
{

private:

  TBEscapeHandler* escHandler;
  bool usingDefault;

  virtual list<AGId>
  load(const string& filename,
       const Id& id = "",
       map<string,string>* signalInfo = NULL,
       map<string,string>* options = NULL)
    throw (agfio::LoadError);

  void
  output_forest(ostream& OUT, const AGId& agId);

  void
  output_tree(ostream& OUT, const AGId& agId, const string& indent="");

public:

  TreeBank() {
    escHandler = new TB_UPenn_WSJ_EscapeHandler;
    usingDefault = true;
  }

  TreeBank(TBEscapeHandler& eh): escHandler(&eh), usingDefault(false) {}

  ~TreeBank()
  { if (usingDefault) delete escHandler; }


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

AGFIO_PLUGIN(TreeBank);

#endif
