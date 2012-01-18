/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	CHAT.h
 */



#ifndef CHAT_H
#define CHAT_H

#include <map>
#include <string>
#include <ag/agfio_plugin.h>

#define MIN_TURN_SIZE 1.0  // 1 second
#define MIN_SEG_SIZE  0.010	// 10 ms

/**
* @class 	CHAT
* @ingroup	Formats
*
* Chat format plug-in for loading and saving CHAT format files.\n
* This plug-in is used by the AG-LIB API.
*/
class DllExport CHAT : public agfio_plugin
{
	public:
		CHAT();
		~CHAT();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
//> 	Test methods
		virtual list<AGId>	pubload(const string& filename,
									const Id& id = "",
									map<string,string>* signalInfo = NULL,
									map<string,string>* options = NULL)
		throw (agfio::LoadError)
		{ return load(filename, id, signalInfo, options); }

		virtual string	pubstore(const string& filename,
								const string& id,
								map<string,string>* options = NULL)
		throw (agfio::StoreError)
		{ return store(filename, id, options); }
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

	private:
		virtual list<AGId>	load(const string& filename,
								const Id& id = "",
								map<string,string>* signalInfo = NULL,
								map<string,string>* options = NULL)
		throw (agfio::LoadError);

		virtual string	store(const string& filename,
								const Id& id,
								map<string,string>* options = NULL)
		throw (agfio::StoreError);

	  /**
		 * Same as <tt>%store()</tt> below except that this one accepts
		 * a list of AGId's instead of just one AGId/AGSetId.
		 */
		virtual string	store(const string& filename,
							list<string>* const ids,
							map<string,string>* options = NULL)
		throw (agfio::StoreError)
		{ throw agfio::StoreError("agfio_plugin::store:Not supported by this format"); }
};


#ifdef _CHAT_IMPL
AGFIO_PLUGIN(CHAT);
#endif // _CHAT_IMPL

#endif
