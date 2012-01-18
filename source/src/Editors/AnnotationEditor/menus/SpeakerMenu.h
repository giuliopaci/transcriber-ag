/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
*  @file 		SpeakerMenu.h
*/

#ifndef _HAVE_SPEAKERS_MENU
#define _HAVE_SPEAKERS_MENU


#include <map>
#include <queue>
#include <string>

#include "AnnotationMenu.h"
#include "DataModel/speakers/SpeakerDictionary.h"

#define MAX_STACK_SIZE 2

namespace tag {
/**
* @class 		SpeakerMenu
* @ingroup		AnnotationEditor
*
* Contextual menu for editor speaker label.\n
*
*/
class SpeakerMenu : public AnnotationMenu
{
	public:
		/**
		 * Constructor
		 * @param dic		Reference on the speaker dictionary used by parent editor
		 * @param lang		Speaker default language
		 * @param overlap	True for allowing overlap creation, False otherwise
		 * @param editable	True for edition menu, false otherwise
		 */
		SpeakerMenu(SpeakerDictionary& dic, string lang="", bool overlap=true, bool editable=true) ;

		/**
		 * Destructor
		 */
		~SpeakerMenu();

		/**
		 * Sets the default language used for new speakers
		 * @param lang 		Iso639-3 language code
		 */
		void setDefaultLanguage(string lang) { m_defaultLang = lang; }

		/**
		 * Accessor to the default language used for new speakers
		 * @return			Default Iso639-3 language code used for new speakers
		 */
		const std::string getDefaultLanguage() { return m_defaultLang; }

		/*! get last selected speaker id
		* @param alternate
		* @return speaker id
		* @note : if no speaker yet selected, will return new default speaker; if only one speaker
		* created and alternate = true, will still return last selected speaker
		*/

		/**
		 * Accessor to the last selected speaker.
		 * @param alternate		If false returns the last selected speaker,
		 * 						else returns one-before-last selected speaker
		 * @return				Speaker id
		 * @note  				If no speaker yet selected, will return new default speaker.\n
		 * 						If only one created speaker and <em>alternate</em> set to True,
		 * 						will still return the last selected speaker.
		 */
		string getLastSelectedSpeaker(bool alternate=false);

		/**
		 * Sets the last selected speaker id.
		 * @param id		Speaker id
		 */
		void setLastSelectedSpeaker(const string& id);

		/**
		 * Signal emitted when a speaker is chosen in menu
		 * <b>const Gtk::TextIter& parameter:</b>		Text position
		 * <b>string:</b>								Speaker id
		 * <b>string:</b>								Speaker full name
		 */
		sigc::signal<void, const Gtk::TextIter&, string, string, float, float>& signalSetSpeaker() { return  m_signalSetSpeaker; }

		/**
		 * Signal emitted when a speaker has been updated
		 * <b>const Gtk::TextIter& parameter:</b>		Text position
		 */
		sigc::signal<void, const Gtk::TextIter&>& signalEditSpeaker() { return  m_signalEditSpeaker; }

		/* Inherited method specialization */
		virtual void updateMenu(bool can_create, bool can_edit=true, bool can_delete=true, bool can_be_unanchored=false);

		/* Inherited method specialization */
		virtual void setHint(const std::string& hint);

	private:
		void insertLastSpeakerItem(const string& id);

		void onSelectNoSpeech(std::string kind);
		void onSelectSpeaker(std::string id);
		void onNewSpeaker();

		void onEditSpeakerProperties();
		void onEditTurnProperties();


		static void initItems();
		/* update menu item vs speakers dictionnary */

		/* member attributes */
		bool m_editable;
		SpeakerDictionary& m_speakersDict;     /* parent annotation editor */
		string m_defaultLang;			/* default lang for speakers */
		int m_nbLast;
		int m_nospeechIndex;

		sigc::signal<void, const Gtk::TextIter&, string, string, float, float>  m_signalSetSpeaker;
		sigc::signal<void, const Gtk::TextIter&>  m_signalEditSpeaker;
		sigc::signal<void, const Gtk::TextIter&>  m_signalEditTurn;

		static std::map<std::string, std::string> m_nospeechItems;
		Gtk::Menu  m_speakerSubmenu;
		Gtk::Menu  m_noSpeakerSubmenu;

		std::queue<std::string> m_lastSelectedSpeaker;  /* last selected spk id */

};

} /* namespace tag */


#endif  // _HAVE_SPEAKERS_MENU
