/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	VSFTfile.h
 */

#ifndef _VSFTFILE_H_
#define _VSFTFILE_H_

#include <string>
#include <fstream>
#include <map>
#include <ag/RE.h>
#include <ag/Record.h>
#include "Common/util/StringOps.h"

/**
* @class 	VSFTfile
* @ingroup	Formats
*
* Parsing handler for VSFT format.\n
* Defines regular expressions for identifying all format entities.
*/
class VSFTfile : public Record
{
	public:
		VSFTfile();
		/**
		* Constructor
		* @param filename		Path of the file to be parsed
		*/
		VSFTfile(const string& filename);
		~VSFTfile();

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
		const string& get_item(string key) { return items[key] ; }

	private:
		map<string, RE*> headerRE;
		map<string, RE*> turnRE;
		map<string, int> nsub;
		map<string, string> items;
		string filename;
		string record_line;
		string record_type;
		RE* record_re;
		int nmax;
		bool check_headers ;
		int	record_no;
		void init();
		virtual void read_entry();
		static string nomatch;
};

#endif
