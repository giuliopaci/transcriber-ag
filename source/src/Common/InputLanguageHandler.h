/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef INPUTLANGUAGEHANDLER_H_
#define INPUTLANGUAGEHANDLER_H_

#include <string>
#include <map>
#include <glibmm.h>
#include <gtkmm/icontheme.h>
#include <gdk/gdk.h>
#include <gtk/gtkimcontext.h>


#define DEFAULT_LANGUAGE _("Default")
#ifdef WIN32
#define IME_LANGUAGE "Windows IME"
#else
#define IME_LANGUAGE "SCIM"
#endif


namespace tag
{

class InputLanguage;

/**
* @class 		InputLanguageHandler
* @ingroup		Common
*
*	Static class which manages available language 4 application.\n
*	It uses the corresponding InputLanguage for loading a language.
*/
class InputLanguageHandler
{
	public:

		/**
		 * Constructor
		 */
		InputLanguageHandler();

		/**
		 * Loads all input languages defined in input language configuration file.
		 * @param filePath		Path of the input language configuration file
		 * @param externalIME	Whether or not enabling external IME use
		 * @return				True if external IME can be chosen in input languages possibility,
		 * 						False otherwise
		 */
		static bool load_language_map(const std::string & filePath, bool externalIME);

		/**
		 * Switch to next or previous InputLanguage
		 * @param current	Currently used input language
		 * @param offset	Switch direction (-1 for previous, 1 for next)
		 * @return			Next or previous InputLanguage, or current if none was found
		 */
		static InputLanguage* change_current_language(InputLanguage* current, int offset);

		/**
		 * Set or unset focus and presentation on the external IME.\n
		 *
		 * @param context		Gdk context in use
		 * @param window		GdkWindon in use
		 * @param activate		True for presenting the IME, False to hide him
		 * @note		MultiOS usage:\n
		 * 					unix:     using context (win can be set to NULL)\n
		 * 					win32:    using win (context can be set to NULL)\n
		 * 					activate: activate or not the IME\n
		 */
		static void activate_external_IME(GtkIMContext* context, GdkWindow* window, bool activate) ;

		/**
		 * @return  System default input language (no mapping)
		 */
		static InputLanguage* defaultInputLanguage();

		/**
		 * @return	Iterator pointing on the first InputLanguage
		 */
		static const std::map<std::string, InputLanguage*>::iterator get_first_language_iter() { return m_ilMap.begin(); }

		/**
		 * @return	Iterator pointing on the last InputLanguage
		 */
		static const std::map<std::string, InputLanguage*>::iterator get_last_language_iter() { return m_ilMap.end(); }

		/**
		 * Uses language name to find the corresponding InputLanguage
		 * @param	name	Language name
		 * @return			Pointer on the corresponding InputLanguage, or NULL if none found
		 */
		static InputLanguage* get_input_language_by_name(std::string name);

		/**
		 * Uses a unicode value to find the corresponding InputLanguage
		 * @param  c	Unicode value
		 * @return		Pointer on the corresponding InputLanguage, or NULL if none found
		 */
		static InputLanguage* get_input_language_by_char(gunichar c);

		/**
		 * Uses a unicode value to find the corresponding InputLanguage
		 * @param  shortcut		Unicode value
		 * @return				Pointer on the corresponding InputLanguage, or NULL if none found
		 */
		static InputLanguage* get_input_language_by_shortcut(std::string shortcut);

	private:

		/**
		*  returns according to input language type the right class instance
		*/
		static InputLanguage* createInputLanguage(std::string display, std::string name, std::string ilType, bool modeLeft2Right, std::string shortcut, Glib::ustring MappingMode, bool spaceSeparated, bool isActivated, bool modifyMapping=true);
		static void free_resources();

		static std::map<std::string, InputLanguage*> m_ilMap; //!< mapping between key value and unicode char

};


}

#endif /*INPUTLANGUAGEHANDLER_H_*/
