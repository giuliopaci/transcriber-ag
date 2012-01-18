/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// agdemo.cc: A demo of using AG API
// Steven Bird & Xiaoyi Ma
// Copyright (C) 2001-2003 Linguistic Data Consortium
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include <iostream>
#include <iterator>
#include <algorithm>
#include <set>
#include <vector>
#include <list>
#include <ag/AGAPI.h>
#include <sys/time.h>


#include "DataModel/DataModel.h"
using namespace tag;


int main(int argc, const char* argv[])
{
	if ( argc < 2 ) {
	cout << "Usage : " << argv[0] << " <filename>" << endl;
	return 1;
	}


	DataModel::initEnviron(argv[0]);

	DataModel data("TransAG");
	string format;

	format = data.guessFileFormat(argv[1]);
	if ( format == "" ) {
			 Log::err() << "Unknown file format" << endl;
				return 1;
	}

	data.loadFromFile(argv[1],format);
	string m_agTrans = data.getAGTrans() ;

  const set<string>& types = GetAnnotationTypes(m_agTrans);
  set<string>::const_iterator itt;
	for ( itt= types.begin(); itt != types.end(); ++itt )
		if ( *itt == "section" ) {
			Log::err() << "Le fichier contient déjà des sections ! " << endl;
			return 1;
		}


  const list<AnchorId>& l = GetAnchorSetByOffset(m_agTrans, 0, 0.001);
	int notrack;
  list<AnchorId>::const_iterator itl;
	 set<AnnotationId>::const_iterator it;
	string bid, oid;
  for ( itl=l.begin(); itl!=l.end() ; ++itl) {

		if ( ! GetAnchored(*itl) ) continue;

		notrack = data.getAnchorSignalTrack(*itl);
		bid = *itl;
		oid = *itl;

	while (1) {
     const set<AnnotationId>& ids =  GetOutgoingAnnotationSet(oid, "turn");
		if ( ids.size() > 0 ) {
      for ( it = ids.begin(); it != ids.end(); ++it ) {
				oid = GetEndAnchor(*it);
			}
	} else break;
	}

	data.createAnnotation(m_agTrans, "section", bid, oid, false);
	}

	data.saveToFile(argv[1],format, false);
}

