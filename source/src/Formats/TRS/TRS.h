/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// TRS.h: TRS loader class definition
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

/* $Id */

/**
 * @file	TRS.h
 */

#ifndef _TRS_H_
#define _TRS_H_

#include <ag/agfio_plugin.h>
#include <xercesc/parsers/SAXParser.hpp>
#include "Common/Parameters.h"
#include "DataModel/DataModel.h"
#include "Formats/TRS/SAX_TRSHandlers.h"

/**
* @class 	TRS
* @ingroup	Formats
*
* TRS format plug-in for loading and saving TRS format files.\n
* This plug-in is used by the AG-LIB API.
*/
class TRS : public agfio_plugin
{

	private:
		tag::DataModel* dataModel ;
		tag::Parameters* mapping  ;

	private:

		virtual list<AGId> load(const string& filename, const Id& id = "",
									map<string,string>* signalInfo = NULL, map<string,string>* options = NULL)
									throw (agfio::LoadError);

		virtual string store(const string& filename, const string& id, map<string,string>* options = NULL)
								throw (agfio::StoreError);

		void setFormatOptions(map<string,string>* options, SAX_TRSHandlers& handler, bool& val_opt, string& encoding) ;
		void initializeDataModel(string agsetid) ;

		void prepare_error_user(tag::DataModel* model, SAX_TRSHandlers* handler, tag::Parameters* mapping) ;
};


AGFIO_PLUGIN(TRS);

#endif
