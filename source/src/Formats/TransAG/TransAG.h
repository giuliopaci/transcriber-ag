/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// TransAG.cc: TransAG loader class implementation
// based on AG loader class definition by Haejoong Lee, Xiaoyi Ma, Steven Bird
// ( Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
//   Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
//   For license information, see the file `LICENSE' included with aglib  distribution)

/**
* @defgroup	Formats File formats Plugins
*/

/**
 *  @file 	TransAG.h
 */

#ifndef _TransAG_H_
#define _TransAG_H_

#include <ag/agfio_plugin.h>

/**
* @class 	TransAG
* @ingroup	Formats
*
* TransAG format plug-in for loading and saving TransAG format files.\n
* This plug-in is used by the AG-LIB API.
*/
class TransAG : public agfio_plugin
{

	#ifdef EXTERNAL_LOAD
	public:
		virtual list<AGId>
		plugin_load(const string& filename,
				const Id& id = "",
				map<string,string>* signalInfo = NULL,
				map<string,string>* options = NULL)
		throw (agfio::LoadError);

		virtual string
		plugin_store(const string& filename,
				const string& id,
				map<string,string>* options = NULL)
		throw (agfio::StoreError);
	#endif

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

#ifndef EXTERNAL_USE
AGFIO_PLUGIN(TransAG)
#endif

#endif
