/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	HTMLfile.h
 */

#ifndef _HTMLFILE_H_
#define _HTMLFILE_H_

#include <string>
#include <fstream>
#include <map>
#include <ag/RE.h>
#include <ag/Record.h>
#include "Common/util/StringOps.h"

/**
* @class 	HTMLfile
* @ingroup	Formats
*
* Parsing handler for HTML format.\n
* Defines regular expressions for identifying all format entities.
*/
class HTMLfile : public Record
{
	public:
	  HTMLfile();
	 /**
	  * Constructor
	  * @param filename		Path of the file to be parsed
	  */
	  HTMLfile(const string& filename);
	  ~HTMLfile();

	 /**
	  * Reads a single record.
	  * @return		True if a known record type has been found, False otherwise.
	  */
	  bool read_record();

	 /**
	  * Accessor to record type
	  * @return		Record type
	  */
	  const string& get_type() { return record_type; }

	  /**
	  * Accessor to record line
	  * @return		Record line
	  */
	  const string& get_line() { return record_line; }

	 /**
	  * Accessor to record number
	  * @return		Current record number
	  */
	  string get_lineno() { return StringOps().fromInt(record_no); }

	 /**
	  * Accessor to the iNth element defined in the regular expression the
	  * CHATfile object represents.
	  * @param i			Element number
	  * @return			Value of the iNth element
	  */
	  string get_matched(int i) ;

	 /**
	  * Accessor to the item corresponding to the given key
	  * @param key		Item key
	  * @return			The corresponding item if found, or empty if unfound
	  */
	  string get_item(string key) { return (items.find(key) == items.end() ? "" : items[key]) ; }

	private:
	  map<string, RE*> headerRE;
	  RE* turnRE;

		string filename;
		string record_line;
		string record_type;
		RE* record_re;
		int nmax;
		bool check_headers ;
		map<string, string> items;

		int	record_no;

	  void init();
	  virtual void read_entry();

		static string nomatch;
};

#endif
