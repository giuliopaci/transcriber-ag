/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef TAGCOMMANDLINE_H_
#define TAGCOMMANDLINE_H_

#include <glibmm.h>
#include "Common/util/Utils.h"

namespace tag {

/**
 * @class 	TAGCommandLine
 * @ingroup GUI
 *	Basic command line wrapper for TranscriberAG
 *	Based on Glib::OptionContext and Glib::OptionEntry mechanism
 */

class TAGCommandLine
{
	public:
		TAGCommandLine();
		virtual ~TAGCommandLine();

		/**
		 * Wrapper for parse method of Glib::OptionContext
		 * @param argc	argc parameter of "main" program
		 * @param argv	argv parameter of "main" program
		 * @return		true if the command line has been successfully parsed, false
		 * 				otherwise (no Glib::OptionContext available, Glib error, ...)
		 */
		bool parse(int argc, char *argv[]) ;

		/**
		 * Prints TranscriberAG version
		 */
		void print_version() ;

		/**
		 * Prints last command line error on standard output
		 */
		void print_error() ;

		/**
		 * Prints error corresponding to given code on standard output
		 * @param code		Glib::OptionError::Code of the message to be printed
		 */
		void print_error(Glib::OptionError::Code code) ;

		/**
		 * Returns Pointer of the Glib::OptionContext used
		 * @return	Glib::OptionContext used by class
		 */
		Glib::OptionContext *getContext() const
		{
			return context;
		}

		/**
		 * Returns the annotation offset passed with the -o option
		 * @return	annotation timecode passed in command line
		 */
		float getOffset() const
		{
			if (m_offset.empty())
				return -1 ;
			else
				return my_atof(m_offset.c_str()) ;
		}

		/**
		 * Cancel use of offset information
		 */
		void resetOffset()
		{
			m_offset = "0" ;
		}

		/**
		 * Cancel use of offset information
		 */
		void resetFilename()
		{
			m_filename = "" ;
		}

		/**
		 * Returns the annotation file path passed with the -f option
		 * @return	file path passed in command line
		 */
		Glib::ustring getFilename() const
		{
			return m_filename;
		}

		/**
		 * Returns TranscriberAG version
		 * @return	application version
		 */
		bool getVersion() const
		{
			return m_version;
		}

		/**
		 * Returns the report level
		 * @return	0: no report\n
		 * 			1: information, warnings and errors\n
		 * 			2: errors only
		 */
		int getReportLevel() const
		{
			if (m_reportLevel.empty())
				return -1 ;
			else
				return string_to_number<int>(m_reportLevel) ;
		}

	private:
		Glib::OptionError::Code error ;
		Glib::ustring m_offset ;
		Glib::ustring m_filename ;
		Glib::ustring m_reportLevel ;

		bool m_version ;

		Glib::OptionContext* context ;
		Glib::OptionGroup* group ;
		std::vector<Glib::OptionEntry*> entries ;

		Glib::ustring get_error(Glib::OptionError::Code code) ;
		Glib::OptionEntry* createEntry(Glib::ustring longName, gchar shortName, int flag,
										Glib::ustring description, Glib::ustring description_arg) ;
};

} //namespace

#endif /* TAGCOMMANDLINE_H_ */
