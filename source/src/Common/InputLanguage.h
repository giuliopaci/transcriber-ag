/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef INPUTLANGUAGE_H_
#define INPUTLANGUAGE_H_

#include <map>
#include <string>
#include <gtkmm.h>
#include <xercesc/dom/DOM.hpp>

#define XML_LANGUAGES 				"Input_Languages"
#define XML_LANGUAGE 				"Language"
#define XML_NAME 					"Name"
#define XML_TYPE					"Type"
#define XML_VALUE					"value"
#define XML_KEY_MAP					"KeyMap"
#define XML_LANGUAGE_GUNICHAR_RANGE	"Language_gunichar_range"
#define XML_KEY_MAP_LABEL			"Label"
#define XML_KEY_VAL					"gdk_keyval"
#define XML_KEY_HARDWARE			"gdk_hardware_code"
#define XML_UNICODE_VALUE			"unicode_value"
#define XML_UNICODE_REPLACE			"unicode_replace"
#define XML_MODIFIER				"modifier"
#define XML_START 					"start"
#define XML_END						"end"
#define XML_MODE_LEFT_2_RIGHT 		"ModeLeft2Right"
#define XML_SHORTCUT				"Shortcut"
#define XML_SPACESEPARATED			"SpaceSeparated"
#define XML_ISACTIVATED				"IsActivated"
#define XML_MAPPINGMODE				"MappingMode"

#define TRANS_MODIFIER_ALT 			"alt"
#define TRANS_MODIFIER_ALTGR 		"altgr"
#define TRANS_MODIFIER_MAJ 			"maj"
#define TRANS_MODIFIER_CTRLSHIFT 	"ctrl+shift"

#define LANGUAGE_HARDWARE_MAPPING 	"Hardware"
#define LANGUAGE_LOGIC_MAPPING		"Logic"

namespace tag
{

class AnnotationView;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/**
* @struct 	_GunicharRange
* Structure defining the range of unicode value for a given InputLanguage.\n
* @note 	Any unicode value outside of this range does not belong to the InputLanguage
*/
/**
 * @typedef		GunicharRange
 * @see			_GunicharRange
 */
typedef struct _GunicharRange
{
	/**
	 * Constructor
	 * @param st	Start unicode character
	 * @param en	End unicode character
	 */
	_GunicharRange(gunichar st, gunichar en) { start = st; end = en; }
	gunichar start;	/**< Start unicode character */
	gunichar end;   /**< End unicode character */
} GunicharRange;


/**
 * @struct 		_ILKeyMap
 * Structure defining the mapping between a gdk_key_val (ofr Logic mode) or a hardware_key_code (for Hardware mode)
 * and a the unicode unicode_value to be used
 */
/**
 * @typedef		ILKeyMap
 * @see			_ILKeyMap
 */
typedef struct _ILKeyMap
{
	/**
	 * Constructor
	 * @param key_val		Logic key value
	 * @param hardware		Hardware key value
	 * @param mapped		Corresponding unicode character
	 * @param replace		Corresponding sequence of unicode character
	 * @param _modifier		Key modifier
	 */
	_ILKeyMap(gunichar key_val, gunichar hardware, gunichar mapped, const Glib::ustring& replace, std::string _modifier)
	{
		gdk_key_val = key_val;
		hardware_code = hardware;
		mapped_value = mapped;
		replace_value = replace ;
		modifier = _modifier ;
	}
	gunichar gdk_key_val ; 			/**< Logic key value */
	gunichar hardware_code ;		/**< Hardware key value */
	gunichar mapped_value ;			/**< Corresponding unicode character */
	Glib::ustring replace_value ;	/**< Corresponding sequence of unicode character */
	std::string modifier ;			/**< Key modifier */
} ILKeyMap;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/**
* @class 		InputLanguage
* @ingroup		Common
*
*	Base class processing input methods.\n
*   Provides method to process most available languages.\n
*   To define new behaviors , create a new derived class and override virtual functions.\n\n
*
*   All the information is loaded from an input configuration file.
*/
class InputLanguage
{
	public:
		/**
		* constructor
		*/
		InputLanguage(std::string display, std::string lan, std::string shortcut, Glib::ustring MappingMode, bool modeLeft2Right = true, bool spaceSeparated = true, bool isActivated=true, bool modifyMapping=true);

		virtual ~InputLanguage();

		/**
		 * Add an element to the key mapping.
		 * @param gdk_key_value		Logic gdk key value
		 * @param hardware_code		Hardware gdk key value
		 * @param mapped_value		Corresponding character
		 * @param replace_value		Corresponding string (used for replacement by several characters)
		 * @param modifier			Gdk modifier used to enter the value
		 * @return					True if the mapping has been updated, False otherwise
		 * @note					If replace_value is defined, mapped_value isn't used
		 */
		bool addKeyMap(gunichar gdk_key_value, gunichar hardware_code, gunichar mapped_value, const Glib::ustring& replace_value, std::string modifier);

		/**
		 * Accessor to the mapped value corresponding to the given GdkEventKey (uses the key and  the hardware members of the event)
		 * @param 	   event		Pointer on GdkEventKey used when entering the letter
		 * @param[out] res			Mapped string
		 * @return					True if a mapped value has been found, False otherwise
		 */
		bool  hasKeyMap(GdkEventKey* event, Glib::ustring& res);

		/**
		 * @return 		The display label defined for the language
		 */
		const std::string & getLanguageDisplay() { return m_display; }

		/**
		* Accessor to the language type. Two main type, the IME_LANGUAGE type (for the use of external input language)
		* and the others
		* @return		Defined IME_LANGAGE value for languages using external input language, or language shortcut for others
		*/
		const std::string & getLanguageType() { return m_languageType; }

		/**
		 * Checks if given character belongs to language characters bound
		 * @param c		Unicode character to check
		 * @return
		 */
		bool is_gunichar_in_bound(gunichar c);

		/**
		 * Add a range of gunichar for this input language
		 * @param rg	Unichar range
		 */
		void addGunicharRange(GunicharRange *rg);

		/**
		*  @return 	True if language is writing from left to right, False otherwise
		*/
		bool ModeLeft2Right() const { return m_modeLeft2Right; }

		/**
		*  @return 	True if words in language are usually separated by space character, False otherwise
		*/
		bool isSpaceSeparated() const { return m_isSpaceSeparated ; }

		/**
		*  If the input language isn't activated, it is not possible to choose it in the editor.
		*  @return 	True if the input language is activated, False otherwise
		*/
		bool isActivated() const { return m_isActivated ; }

		/**
		 * Accessor to the language shortcut defined in configuration file
		 * @return
		 */
		std::string get_language_shortcut() const { return m_languageShortcut; }

		/**
		 * Launches some post processing after after text insertion
		 * @param view		Connected text view
		 * @param iter		Current text view position
		 */
		virtual void postProcessing(Gtk::TextView *view, Gtk::TextIter iter);

		/**
		 * Checks the insertion rules before processing text insertion at given buffer position (different for each languages)
		 * @param view		Connected text view
		 * @param iter		Current text view position
		 * @param s			String to be inserted
		 * @return			True if the insertion is allowed, False otherwise
		 */
		virtual bool check_insertion_rules(Gtk::TextView *view, const Gtk::TextIter& iter, const Glib::ustring& s) ;

		/**
		 * Checks the insertion rules before processing text insertion inside a given
		 * string at a given position (different for each languages)
		 * @param text		Text into which the insertion will be done
		 * @param pos		Position at which the insertion will be done
		 * @param s			Text inserted
		 * @return			True if the insertion is allowed, False otherwise
		 */
		virtual bool check_insertion_rules_str(const Glib::ustring& text, int pos, const Glib::ustring& s) ;

		/**
		 * Does special treatment (for instance adding information) on given node and unicode character after loading
		 * a keyMap from the input configuration file
		 * @param node		Node to modify
		 * @param c			Unicode character
		 */
		virtual void postLoadingKeyMap(xercesc::DOMNode *node, gunichar c);

		/**
		 * Manages backspace key event
		 * @param event		Pointer on GdkEventKey used when entering the letter
		 * @param view		Connected text view
		 * @param it		Current text view position
		 * @return			True if deletion has been proceeded, False otherwise
		 */
		virtual bool proceedDeletion(GdkEventKey *event, Gtk::TextView  *view, Gtk::TextIter it);

		/**
		 * Format the given string according to InputLanguage Engine
		 * @param s			String to be treated
		 * @return			Computed string
		 * @note			For instance, the InputLanguageArabic class uses this method
		 * 					for replacing unicode character by theirs correct arabic form
		 * @deprecated		Not used anymore
		 */
		virtual Glib::ustring processingString(Glib::ustring s)  ;

		/**
		 * For a given string returns the unformatted corresponding string
		 * @param s			String to be treated
		 * @return			The corresponding basic string
		 * @see				processingString()
		 * @deprecated		Not used anymore
		 */
		virtual Glib::ustring unprocessingString(Glib::ustring s)  ;

		/**
		 * Formats the given string in a specific language format
		 * @param s			String to be treated
		 * @note			For instance, the InputLanguageArabic class uses this method
		 * 					for removing diacritics
		 * @return			The formatted string
		 */
		Glib::ustring special_string_format(const Glib::ustring& s) ;

		/**
		 * Checks if the input language modifies or not the characters
		 * @return		True if the input language modifies the characters, false otherwise
		 */
		bool modifyMapping() { return m_modifyMapping ; }


	protected:
		std::vector<ILKeyMap*> m_keyMap; 				/**< Mapping of gdk_key - gunichar values */
		std::vector<GunicharRange*> m_gunichar_range;	/**< Vector of language unicode range */
		std::string m_languageType;						/**< Language name */
		std::string m_display;							/**< Language display */
		bool m_modeLeft2Right ;							/**< Left to right writing mode flag */
		std::string m_languageShortcut;					/**< Language shortcut (Iso639-3 code) */
		bool m_isSpaceSeparated ;						/**< Space separated words mode flag */
		bool m_isActivated ;							/**< Language input activation status */

		/**
		* @var	m_modifyMapping
		* In most of cases set to true
		* Only false for Default language and IME language
		* This value isn't read in configuration file, internal process
		*/
		bool m_modifyMapping ;

		/**
		* @var	m_mappingMode
		* Type of mapping defined in mapping language:
		* "Hardware" (hardware gdk code) or "Logic" (key gdk code)
		*/
		Glib::ustring m_mappingMode ;

	private:
		Glib::ustring get_transcriber_modifier(GdkEventKey* event) ;

};

}

#endif /*INPUTLANGUAGE_H_*/
