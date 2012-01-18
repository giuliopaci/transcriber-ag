/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	CHATfile.h
 */

#ifndef _CHATFILE_H_
#define _CHATFILE_H_

#include <string>
#include <fstream>
#include <map>
#include <ag/RE.h>
#include <ag/Record.h>
#include "Common/util/StringOps.h"

/**
* @class 	CHATfile
* @ingroup	Formats
*
* Parsing handler for CHAT format.\n
* Defines regular expressions for identifying all format entities.
*/
class CHATfile : public Record
{
	public:
		/**
		 * Constructor
		 */
		CHATfile();

		/**
		 * Constructor
		 * @param fileName		Path of the file to be parsed
		 */
		CHATfile(const string& fileName);

		/**
		 * Destructor
		 * @return
		 */
		~CHATfile();

		// -- Reading Single Record --
		/**
		 * Reads format header
		 * @return		True
		 */
		bool read_headers();

		/**
		 * Reads a single record.
		 * @return		True if a known record type has been found, False otherwise.
		 */
		bool read_record();

		/**
		 * Accessor to record type
		 * @return		Record type
		 */
		const string& get_type()		{ return record_type; }

		/**
		 * Accessor to record line
		 * @return		Record line
		 */
		const string& get_line()		{ return record_line; }

		/**
		 * Accessor to record number
		 * @return		Current record number
		 */
		string get_lineno()				{ return StringOps().fromInt(record_no); }

		/**
		 * Accessor to the item corresponding to the given key
		 * @param key		Item key
		 * @return			The corresponding item if found, or empty if unfound
		 */
		string get_item(string key)		{ return (items.find(key) == items.end() ? "" : items[key]) ; }

		/**
		 * Accessor to the option value corresponding to the given key
		 * @param key		Option key
		 * @return			The corresponding option value if found, or empty if unfound
		 */
		string get_option(string key)	{ return (options.find(key) == options.end() ? "" : options[key]); }

		/**
		 * Accessor to the iNth element defined in the regular expression the
		 * CHATfile object represents.
		 * @param i			Element number
		 * @return			Value of the iNth element
		 */
		string get_matched(int i);

		// -- Turn accessors --
		/**
		 * Accessor to the current turn speaker.
		 * @return		Speaker id of the current turn.
		 */
		string get_turn_speaker()						{ return turnSpeaker; }

		/**
		 * Accessor to the current turn text.
		 * @return		Text of the current turn.
		 */
		string get_turn_text()							{ return turnText; }

		/**
		 * Accessor to the current turn options.
		 * @return		Map with all current turn options
		 */
		multimap<string, string>& get_turn_options()	{ return turnOptions; }

	private:
		RE*					optionRE;
		RE*					optionValueRE;
		RE*					turnRE;
		RE*					turnOptionsRE;
		RE*					record_re;

		string				filename;
		string				record_line;
		string				record_type;
		string				current_option;

		// -- Turn --
		string				turnSpeaker;
		string				turnText;

		int		nmax;
		bool	check_headers;
		bool	turn_matched;
		int		record_no;

		map<string, string> items;
		map<string, string>	options;
		multimap<string, string>	turnOptions;

		void init();
		virtual void read_entry();

		static string nomatch;
};

#endif

