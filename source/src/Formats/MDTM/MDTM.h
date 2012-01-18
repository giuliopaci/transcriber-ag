/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	MDTM.h
 */

#ifndef MDTM_H
#define MDTM_H

#include <map>
#include <string>
#include <ag/agfio_plugin.h>

#define MIN_TURN_SIZE 1.0  // 1 second
#define MIN_SEG_SIZE  0.010	// 10 ms

/**
* @class 	MDTM
* @ingroup	Formats
*
* MDTM format plug-in for loading and saving MDTM format files.\n
* This plug-in is used by the AG-LIB API.
*/
class DllExport MDTM : public agfio_plugin
{
	public:
		MDTM();
		~MDTM();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
//> 	Test methods
		virtual list<AGId>	pubload(const string& filename,
									const Id& id = "",
									map<string,string>* signalInfo = NULL,
									map<string,string>* options = NULL)
		throw (agfio::LoadError)
		{ return load(filename, id, signalInfo, options); }

		/*
		virtual string	pubstore(const string& filename,
								const string& id,
								map<string,string>* options = NULL)
		throw (agfio::StoreError)
		{ return store(filename, id, options); }
		*/
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

	private:
		virtual list<AGId>	load(const string& filename,
								const Id& id = "",
								map<string,string>* signalInfo = NULL,
								map<string,string>* options = NULL)
		throw (agfio::LoadError);

		/*
		virtual string	store(const string& filename,
								const Id& id,
								map<string,string>* options = NULL)
		throw (agfio::StoreError);
		*/

	  /**
		 * Same as <tt>%store()</tt> below except that this one accepts
		 * a list of AGId's instead of just one AGId/AGSetId.
		 */

		/*
		virtual string	store(const string& filename,
							list<string>* const ids,
							map<string,string>* options = NULL)
		throw (agfio::StoreError)
		{ throw agfio::StoreError("agfio_plugin::store:Not supported by this format"); }
		*/
};


#ifdef _MDTM_IMPL
AGFIO_PLUGIN(MDTM);
#endif // _MDTM_IMPL

#endif

