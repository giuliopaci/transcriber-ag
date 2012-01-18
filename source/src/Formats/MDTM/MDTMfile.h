/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	MDTMfile.h
 */

#ifndef _MDTMFILE_H_
#define _MDTMFILE_H_

#include <string>
#include <fstream>
#include <map>
#include <ag/RE.h>
#include <ag/Record.h>
#include "Common/util/StringOps.h"

/**
* @class 	MDTMfile
* @ingroup	Formats
*
* Parsing handler for MDTM format.\n
* Defines regular expressions for identifying all format entities.
*/
class MDTMfile : public Record
{
	public:
		MDTMfile();
		/**
		* Constructor
		* @param fileName		Path of the file to be parsed
		*/
		MDTMfile(const string& fileName);
		~MDTMfile();

		/**
		* Reads a single record.
		* @return		True if a known record type has been found, False otherwise.
		*/
		bool	read_record();

		/**
		* Accessor to record type
		* @return		Record type
		*/
		const	string& get_type()		{ return record_type; }

		/**
		* Accessor to record line
		* @return		Record line
		*/
		const	string& get_line()		{ return record_line; }

		/**
		* Accessor to record number
		* @return		Current record number
		*/
		string	get_lineno()			{ return StringOps().fromInt(record_no); }

		/**
		* Accessor to the item corresponding to the given key
		* @param key		Item key
		* @return			The corresponding item if found, or empty if unfound
		*/
		string	get_item(string key)	{ return (items.find(key) == items.end() ? "" : items[key]) ; }

		/**
		* Accessor to the iNth element defined in the regular expression the
		* CHATfile object represents.
		* @param i			Element number
		* @return			Value of the iNth element
		*/
		string	get_matched(int i);

		// -- Turn accessors --
		/**
		 * Accessor to all turn
		 * @return		Vector with turn items
		 */
		vector<string>	get_turn()		{ return turn_items; }

	private:
		RE*		turnRE;
		RE*		record_re;

		string	filename;
		string	record_line;
		string	record_type;

		int		nmax;
		int		record_no;

		map<string, string> items;
		vector<string>		turn_items;

		void init();
		virtual void read_entry();

		static string nomatch;
};

#endif

