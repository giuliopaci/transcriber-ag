/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	TransAG_StringParser.h
 */

#ifndef TRANSAGSTRINGPARSER_H_
#define TRANSAGSTRINGPARSER_H_

#include <string>
#include <ag/agfio.h>

using namespace std;
/**
* @class 	TransAG_StringParser
* @ingroup	Formats
*
* Class allowing to load a TAG format from a buffer (and not from a file as classic plug-in)
*/
class TransAG_StringParser
{
	public:
		/**
		 * Constructor
		 * @param str		Format AG in buffer representation
		 */
		TransAG_StringParser(const string& str)	: m_agId("") { parseBuffer(str); }

		/**
		 * Accessor to the graph id corresponding to the given graph
		 */
		const string& getGraphId() { return m_agId; }

	private:
		  void parseBuffer(const string& str) throw (agfio::LoadError);
		  string m_agId;
};
#endif /* TRANSAGSTRINGPARSER_H_ */
