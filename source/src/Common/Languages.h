/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef LANGUAGES_H_
#define LANGUAGES_H_

#include <string>
#include <glibmm.h>

namespace tag {

/**
* @class 		Languages
* @ingroup		Common
*
*	Singleton that loads the language configuration. \n
*/
class Languages
{
	public:
		/**
		 * Creates a unique instance if it doesn't already exist, or returns the existing one.
		 * @return	Pointer on the unique Languages instance
		 */
		static Languages* getInstance();

		/**
		 * Deletes the unique instance.
		 * @return	Pointer on the unique Languages instance
		 */
		static void kill();

		/**
		 * Load the language configuration from file
		 * @param path		Path of the language configuration file
		 * @note 			By default the file is languagesAG.xml contained in the configuration directory
		 * @param mode		Component name in the configuration file for enabling a partial loading only.
		 */
		static void configure(const Glib::ustring& path, const Glib::ustring& mode);

		/**
		 * Print all languages loaded and theirs characteristics
		 */
		void print()  ;

		/**
		 * Get the Iso639-3 language code corresponding to the given language name
		 * @param name		Language name
		 * @return			The corresponding Iso639-3 code
		 */
		Glib::ustring get_code(Glib::ustring name) const ;

		/**
		 * Get the language name corresponding to the given Iso639-3 language code
		 * @param code		A Iso639-3 language code
		 * @return			The corresponding language name
		 */
		Glib::ustring get_name(Glib::ustring code) const ;

		/**
		 * @return 		All Iso639-3 language code
		 */
		const std::vector<Glib::ustring>& get_codes() ;

		/**
		 * @return 		All Iso639-3 language name
		 */
		const std::vector<Glib::ustring>& get_names() ;

		/**
		 * Gets all accents defined for given language. If no language is given
		 * in parameter, gets all accents.
		 * @param lang		A Iso639-3 language code
		 * @return			All accents corresponding to the language code, or
		 * 					all accents if given code was an empty value
		 */
		const std::vector<Glib::ustring>& get_accents(Glib::ustring lang="") ;

		/**
		 * Gets all dialects defined for given language. If no language is given
		 * in parameter, gets all dialects.
		 * @param lang		A Iso639-3 language code
		 * @return			All dialects corresponding to the language code, or
		 * 					all dialects if given code was an empty value
		 */
		const std::vector<Glib::ustring>& get_dialects(Glib::ustring lang="") ;

	private:
		static Languages* m_instance;
		Languages(Glib::ustring path, Glib::ustring mode="");
		virtual ~Languages();

		Glib::ustring path ;

		std::map<Glib::ustring, Glib::ustring> name_from_code ;
		std::map<Glib::ustring, Glib::ustring> code_from_name ;

		std::vector<Glib::ustring> codes ;
		std::vector<Glib::ustring> names ;
		std::map<Glib::ustring, std::vector<Glib::ustring> > accents ;
		std::map<Glib::ustring, std::vector<Glib::ustring> > dialects ;

		// to return empty vec.
		std::vector<Glib::ustring> empty ;

		// current locale
		bool initialised ;
		std::string m_locale;

		int initialise(Glib::ustring mode) ;
		void add_vect(std::map<Glib::ustring, std::vector<Glib::ustring> >& store, Glib::ustring code, Glib::ustring val);
};

} //namespace

#endif /*LANGUAGES_H_*/
