/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef FORMATS_H_
#define FORMATS_H_

#include <glibmm.h>
#include "Parameters.h"

namespace tag {
/**
 * @class 		Formats
 * @ingroup		Common
 *
 *	Singleton defining the transcription formats supported by TranscriberAG\n
 *
 *  The information are loaded from a configuration file (by default : formatsAG.rc)\n
 *  All the information returned by the class methods are relative to the formatsAG.rc
 *  syntaxe.
 */
class Formats
{
	public:
		/***
		 * Constructor
		 */
		Formats();
		virtual ~Formats();

		/**
		 * Load configuration format file path
		 * @param configuration_path
		 */
		static void configure(string configuration_path) ;

		/**
		 * @return Unique instance of the Formats object
		 */
		static Formats* getInstance();

		/**
		 * Kills the formats instance
		 */
		static void kill() ;

		/**
		 * Accessor to the type corresponding to the given format
		 * @param format	A TranscriberAG format
		 * @return			The corresponding TranscriberAG format type
		 */
		string getTypeFromFormat(string format) ;
		/**
		 * Accessor to the format corresponding to the given type
		 * @param type	A file type
		 * @return			The corresponding TranscriberAG format
		 */
		string getFormatFromType(string type) ;

		/**
		 * Accessor to the display corresponding to the given format
		 * @param format	A TranscriberAG format
		 * @return			The corresponding format description
		 */
		string getDisplayFromFormat(string format) ;

		/**
		 * Accessor to the display corresponding to the given format
		 * @param format	A TranscriberAG format
		 * @return			The corresponding format description
		 */
		string getExtensionFromFormat(string format) ;

		/**
		 * Checks if the given format can be imported
		 * @param format	A TranscriberAG format
		 * @return			True if the application can import the corresponding file type
		 */
		bool isImport(string format);

		/**
		 * Checks if the given format can be exported
		 * @param format	A TranscriberAG format
		 * @return			True if the application can export the corresponding file type
		 */
		bool isExport(string format);

		/**
		 * Accessor to all TranscriberAG transcription file format supported
		 * @return			All TranscriberAG transcription format names define in file configuration
		 */
		const std::vector<string> getFormats() { return formats ; }

	private :
		/** Singleton **/
		static Formats* m_instance ;

		/** Business **/
		string file ;
		Parameters formats_param ;

		std::vector<string> formats ;
		std::map<string, string> formatTOextension ;
		std::map<string, string> formatTOio;
		std::map<string, string> formatFromType;

		bool load(string configuration_path) ;
		void loadMap() ;
} ;

}

#endif /* FORMATS_H_ */
