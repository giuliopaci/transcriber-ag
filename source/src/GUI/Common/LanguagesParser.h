/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef LANGUAGESPARSER_H_
#define LANGUAGESPARSER_H_

#include <gtkmm.h>

namespace tag {
/**
 * @class 			LanguagesParser
 * @ingroup			GUI
 * @deprecated		not used anymore
 */
class LanguagesParser
{
	public:
		LanguagesParser();
		virtual ~LanguagesParser();

		/**
		 * @deprecated not used anymore
		 */
		static void makeConfigFile();

		/**
		 * @param path
		 * @param vect_gen
		 * @deprecated not used anymore
		 * @return
		 */
		static int read_ctl(Glib::ustring path, std::vector<Glib::ustring>* vect_gen) ;
};

} //namespace

#endif /*LANGUAGESPARSER_H_*/
