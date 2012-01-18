/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	HTML.h
 */

#ifndef _HTML_H_
#define _HTML_H_

#include <map>
#include <string>
#include <ag/agfio_plugin.h>

#define MIN_TURN_SIZE 1.0  // 1 second
#define MIN_SEG_SIZE  0.010	// 10 ms

/**
* @class 	HTML
* @ingroup	Formats
*
* HTML format plug-in for loading and saving HTML format files.\n
* This plug-in is used by the AG-LIB API.
*/
class DllExport HTML : public agfio_plugin
{
	private:
		/**
		* Load a HTML file, converting it to AG's.
		*
		* @param filename
		*   A path (absolute or relative) to the file.
		* @param id
		*   AGSetId or AGId into which the file will be loaded.
		* @param signalInfo
		*   A feature-value pair list for the signal information. Features that
		*   should be used are:
		*   <ul>
		*   <li><tt>uri</tt></li>
		*   <li><tt>mimeClass</tt></li>
		*   <li><tt>mimeType</tt></li>
		*   <li><tt>encoding</tt></li>
		*   <li><tt>unit</tt></li>
		*   <li><tt>track</tt> (optional)</li>
		*   </ul>
		*   The values shouldn't be empty.
		* @param options
		*   A option-value pair list that is used to change the behavior of
		*   <tt>%load()</tt> of the I/O module(plugin).
		* @return
		*   List of ids of created AG's
		*/
		virtual list<AGId>
		load(const string& filename,
				const Id& id = "",
				map<string,string>* signalInfo = NULL,
				map<string,string>* options = NULL)
		throw (agfio::LoadError);

		/**
		* Store AG's to a HTML file.
		*
		* @param filename
		*   The name of the file to be written.
		* @param id
		*   The id of AG or AGSet to be stored.
		* @param options
		*   A option-value pair list that is used to change the behavior of
		*   <tt>%store()</tt> of the I/O module(plugin).
		* @return
		*   An optional string.
		*/
		virtual string
		store(const string& filename,
				const Id& id,
				map<string,string>* options = NULL)
		throw (agfio::StoreError);

		/**
		* Same as <tt>%store()</tt> below except that this one accepts
		* a list of AGId's instead of just one AGId/AGSetId.
		*/
		virtual string
		store(const string& filename,
				list<string>* const ids,
				map<string,string>* options = NULL)
		throw (agfio::StoreError)
		{ throw agfio::StoreError("agfio_plugin::store:Not supported by this format"); }


	public:


		HTML();
		~HTML();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
//> 	Test methods
		virtual list<AGId>
		pubload(const string& filename,
				const Id& id = "",
				map<string,string>* signalInfo = NULL,
				map<string,string>* options = NULL)
		throw (agfio::LoadError) { return load(filename, id, signalInfo, options); }

		virtual string
		pubstore(const string& filename,
				const string& id,
				map<string,string>* options = NULL)
		throw (agfio::StoreError) { return store(filename, id, options); }
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
};

#ifdef _HTML_IMPL
AGFIO_PLUGIN(HTML);
#endif // _HTML_IMPL

#endif
