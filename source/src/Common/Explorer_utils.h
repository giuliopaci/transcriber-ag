/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  TransAGVideo				*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef EXPLORER_UTILS_H_
#define EXPLORER_UTILS_H_

#include <iostream>
#include <string>
#include <sstream>
#include <glibmm.h>
#include <gtkmm/icontheme.h>
#include <gtkmm.h>
#include "Common/globals.h"

namespace tag {

/**
 * @class 		Explorer_utils
 * @ingroup		GUI
 * Static methods for some useful GUI classes operation
 *
 */
class Explorer_utils
{
	public:
		Explorer_utils() ;
		virtual ~Explorer_utils(){} ;

		/**
		 * Write a line a text file
		 * @param path		path of the target file
		 * @param line		string to be written
		 * @param mode		file opening mode (<em>fopen</em> based)
		 * @return			1 for success, 0 for failure
		 */
		static int write_line(Glib::ustring path, Glib::ustring line, Glib::ustring mode) ;

		/**
		 * Write lines in a text file
		 * @param path		path of the target file
		 * @param lines		vector containing all string to be written\n
		 * 					(each vector element will be a line)
		 * @param mode		file opening mode (<em>fopen</em> based)
		 * @return			1 for success, 0 for failure
		 */
		static int write_lines(Glib::ustring path, std::vector<Glib::ustring> lines, Glib::ustring mode) ;

		/**
		 * Read lines in a text file
		 * @param path		path of the target file
		 * @param lines		Pointer on the resulting vector
		 * @return			1 for success, -1 for failure
		 */
		static int read_lines(Glib::ustring path, std::vector<Glib::ustring>* lines) ;

		/**
		 * Write a line in a text file
		 * @param file		FILE structure of file
		 * @param line		string to be written
		 * @return			1 for success, 0 for failure
		 * @note			<em>file</em> must be handled out of the method (open/close)
		 */
		static int write_line(FILE* file, Glib::ustring line) ;

		/**
		 * Format time from second value to HH/MM/SS vector value
		 * @param seconds_time		time in seconds
		 * @param result			vector representing HH/MM/SS values\n
		 * 							element1 : hours
		 * 							element2 : minutes
		 * 							element3 : seconds
		 */
		static void get_time(double seconds_time, std::vector<int>* result) ;

		/**
		 * Check if a given string is the substring of another given string
		 * @param sub		is that string a substring ?
		 * @param s			Pointer string of which <em>sub</em> could be a substring
		 * @return			positive integer if true\n
		 * 					0 if false\n
		 * 					-1 if sub.size > s.size
		 */
		static int is_substring(Glib::ustring sub, Glib::ustring s) ;

		/**
		 * @param s		string to be printed
		 * @param mode	print mode
		 */
		static void print_trace(Glib::ustring s, int mode) ;

		/**
		 * @param s		string to be printed
		 * @param arg	argument to be printed
		 * @param mode	mode of trace
		 */
		template <class ARGUMENT>
		static void print_trace(Glib::ustring s, ARGUMENT arg, int mode)
		{
			if (mode==1)
				Log::out() << s << " -> " << arg << std::endl ;
			else if (mode==0)
				Log::err() << s << " -> " << arg << std::endl ;
		}

		/**
		 * Format time depending on locale
		 * @param time		time in string representation
		 * @return			formatted time
		 * @note			not yet implemented, just return time
		 */
		static Glib::ustring format_date(Glib::ustring time) ;

		/**
		 * Format a string-size-representation {bytes} into
		 * a string-size-representation {mb kb bytes}
		 * @param size		string-size-representation {bytes}
		 * @return			string-size-representation {mb kb bytes}
		 */
		static Glib::ustring format_size(Glib::ustring size) ;

		/**
		 * Give the string representation of a float number
		 * @param i		float number
		 * @return		string representation
		 */
		static Glib::ustring float_to_string(float i) ;

		/**
		 * Format recent files path replacing all "_" characters by
		 * "__" string
		 * Indeed as recent files path are displayed in Gtk::Menu, all
		 * "_" characters become accelerators.
		 * @param path	file path
		 * @return		formatted file path
		 */
		static Glib::ustring format_recent_file(Glib::ustring path) ;
} ;

} //namespace

#endif /*EXPLORER_UTILS_H_*/


