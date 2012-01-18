/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// VRXML.cc: VRXML (VecsysResearch XML format) loader class implementation
// Paule Lecuyer
// Copyright (C) 2007 Bertin Technologies
// Web: http://www.bertin.fr/

/* $Id */

/**
 * @file	VRXML.h
 */

#ifndef _VRXML_H_
#define _VRXML_H_

#include <ag/agfio_plugin.h>
#include <xercesc/parsers/SAXParser.hpp>

/**
* @class 	VRXML
* @ingroup	Formats
*
* VRXML format plug-in for loading and saving VRXML format files.\n
* This plug-in is used by the AG-LIB API.
*/
class VRXML : public agfio_plugin
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


AGFIO_PLUGIN(VRXML);

#endif
