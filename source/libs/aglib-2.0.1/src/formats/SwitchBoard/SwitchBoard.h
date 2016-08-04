// SwitchBoard.h: SwitchBoard loader class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _SWITCHBOARD_H_
#define _SWITCHBOARD_H_

#include <ag/agfio_plugin.h>

/// SwtchBoard loader class.
class DllExport SwitchBoard : public agfio_plugin
{
 
private:

  virtual list<AGId>
  load(const string& filename,
       const Id& id = "",
       map<string,string>* signalInfo = NULL,
       map<string,string>* options = NULL)
    throw (agfio::LoadError);

};

AGFIO_PLUGIN(SwitchBoard);

#endif
