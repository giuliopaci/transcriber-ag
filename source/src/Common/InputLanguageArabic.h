/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef INPUTLANGUAGEARABIC_H
#define INPUTLANGUAGEARABIC_H

#include "InputLanguage.h"

class AnnotationBuffer;

namespace tag
{

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/**
* @typedef	unicode_val_flag
* @see		_unicode_val_flag
*/
/**
 * @enum	_unicode_val_flag
 * Bit flag indicating available forms for any arabic char
 */
typedef enum _unicode_val_flag
{
	AR_NONE 	= 0, 	/**< AR_NONE */      //!< AR_NONE
	AR_ORIGINAL = 1,	/**< AR_ORIGINAL *///!< AR_ORIGINAL
	AR_ISOLATED = 2,	/**< AR_ISOLATED *///!< AR_ISOLATED
	AR_INITIAL 	= 4,	/**< AR_INITIAL */ //!< AR_INITIAL
	AR_MEDIAL 	= 8, 	/**< AR_MEDIAL */  //!< AR_MEDIAL
	AR_FINAL 	= 16  	/**< AR_FINAL */   //!< AR_FINAL
} unicode_val_flag;


/**
 * @typedef gunichar_type
 * @see		_gunichar_type
 */
/**
* @enum _gunichar_type
* Arabic unicode character type
*/
typedef enum _gunichar_type
{
	G_UNI_ARABIC			= 1,//!< G_UNI_ARABIC
	G_UNI_SPACE 			= 2,//!< G_UNI_SPACE
	G_UNI_OTHER 			= 3 //!< G_UNI_OTHER
} gunichar_type;


/**
 * @typedef		Unicode_Ar_Form
 * @see			_uni_ar_form
 */
/**
* @struct 	_uni_ar_form
* Structure defining the properties of an arabic character
*/
typedef struct _uni_ar_form
{
	_uni_ar_form()
	{
		available_types = AR_NONE;
		original_val = isolated_val = initial_val = final_val = medial_val = 0;
		unlinked_char = false;
		is_vowel = 0 ;
		no_check = 0 ;
		is_punctuation = false;
	}
	gunichar			original_val; 		/**< Basic form unicode value */
	gunichar			isolated_val;		/**< Isolated form unicode value */
	gunichar 			initial_val;		/**< Initial form unicode value */
	gunichar			final_val;			/**< Final form unicode value */
	gunichar			medial_val;			/**< Medial form unicode value */
	unicode_val_flag	available_types;	/**< Available types */
	bool				unlinked_char; 		/**< Indicates whether the character can be ligatured with next character  */
	/**
	* @var 	is_vowel
	* Indicates whether the character is a vowel.\n
	* There are 3 values:\n
	* 	0 : not a vowel\n
	*  1 : vowel\n
	*  2 : special char Shadda\n
	*/
	int 				is_vowel;
	int 				no_check ; 			/**<  1 if the letter hasn't to be checked by speller  */
	bool 				is_punctuation;		/**<  Indicates if the character can be ligatured with next character  */
	bool		 		is_composed ;		/**<  Indicates if the character is composed by several unicodes character  */
}Unicode_Ar_Form;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
* @class 		InputLanguageArabic
* @ingroup		Common
*
*	Specialization of InputLanguage for Arabic Input Method.
*
*/

class InputLanguageArabic :	public InputLanguage
{
	public:

		static const gunichar LRM = 0x200E;  	/**< Left to Right Mark */
		static const gunichar RLM = 0x200F;		/**< Right to Left Mark */
		static const gunichar LRE = 0x202A;		/**< Left to Right Embedding */
		static const gunichar RLE = 0x202B;		/**< Right to Left Embedding */
		static const gunichar PDF = 0x202C;		/**< Pop Directionnal Formatting */
		static const gunichar RLO = 0x202E;		/**< Right to Left Override */
		// static const gunichar LRO = 0x202D;

		/**
		* Constructor
		*/
		InputLanguageArabic(std::string display, std::string lan, std::string shortcut, Glib::ustring mappingMode, bool modeLeft2Right, bool spaceSeparated, bool isActivated, bool modifyMapping=true);

		~InputLanguageArabic();

		/*** Herited methods ***/
		virtual void postProcessing(Gtk::TextView *view, Gtk::TextIter iter);
		virtual void postLoadingKeyMap(DOMNode *node, gunichar c);
		virtual bool proceedDeletion(GdkEventKey *event, Gtk::TextView  *view, Gtk::TextIter it);
		virtual Glib::ustring processingString(Glib::ustring s)  ;
		Glib::ustring unprocessingString(Glib::ustring s) ;
		bool check_insertion_rules(Gtk::TextView *view, const Gtk::TextIter& iter, const Glib::ustring& s) ;
		bool check_insertion_rules_str(const Glib::ustring& text, int pos, const Glib::ustring& s);
		Glib::ustring special_string_format(const Glib::ustring& s) ;

		/**
		 * @param c		Unicode character
		 * @return		The corresponding unicode character type
		 */
		gunichar_type get_gunichar_type(gunichar c);

		/**
		 *  Removes all vowels, presentation characters and characters marked as "nocheck".\n
		 *  Used for giving to the speller the string representation that it must check.
		 * @param 	  	s		String to format
		 * @param[out]  res		Resulting string
		 */
		void remove_vowel_and_nocheck_string(const Glib::ustring& s, Glib::ustring& res);

		/**
		 *  Removes all vowels, presentation characters and characters marked as "nocheck".\n
		 *  Used for giving to the speller the string representation that it must check.
		 * @param  s		String to format
		 * @return			Resulting string
		 */
		char* remove_vowel_and_nocheck(const char* s) ;

		/**
		 * Checks if the given unicode character is an arabic presentation one
		 * @param c		Unicode character
		 * @return		True if it matches one of the presentation character, False otherwise
		 * @note		Presentation characters recognized:\n
		 *				- LRM : Left to Right Mark\n
		 *				- RLM : Right to Left Mark\n
		 *				- LRE : Left to Right Embedding\n
		 *				- RLE : Right to Left Embedding\n
		 *				- PDF : Pop Directionnal Formatting\n
		 *				- RL0 : Right to Left Override
		 */
		static bool is_presentation_character(gunichar c) ;

		/**
		 * Trace debugging method for _uni_ar_form structure
		 * @param display	If set to False, only displays number of _uni_ar_form
		 * @return			Number of _uni_ar_form
		 */
		int printArForm(bool display) ;

	private:
		bool check_vowels_and_shadda(gunichar char_before, gunichar toBeInserted, bool at_start);

		/**
		* returns depending on char before and after...the needed type for a char
		*/
		static unicode_val_flag get_unicode_ar_type(gunichar_type before_type, gunichar_type after_type, bool is_char_before_linked);

		/**
		* update char at iterator according to char before and char after the one at iter
		*/
		void update_char_at_iter(Gtk::TextView *view, Gtk::TextIter it);

		/**
		* returns for a given char its original presentation form
		* @param is_char_before_linked : reference on a variable equal to true if the strBefore char can't be linked with its next one
		*/
		gunichar get_unicode_ar_original_unichar(gunichar c, bool &is_linked_char);

		/**
		* returns for a given char (original form) a specific form(initial, medial, final)
		*/
		gunichar get_unicode_ar_form(gunichar c, unicode_val_flag type);

		/**
		* returns true if the gunichar matches an arabic vowel
		*/
		bool is_vowel(gunichar c);

		/**
		* returns true if the gunichar has been set for not beeing watched in speller and search behaviour
		* By default, vowels and shadda aren't watched by speller because most of dictionary don't contain
		* diacritics
		* WARNING: is_nocheck doesn't return true for vowels and shadda, use is_vowel for them
		*/
		bool is_nocheck(gunichar c) ;

		/**
		* returns true if char is a punctuation(arabic one)
		*/
		bool is_punctuation(gunichar c);


	private:
		std::map<unsigned int, Unicode_Ar_Form*> m_formTable;
};

}

#endif /*INPUTLANGUAGEARABIC_H*/
