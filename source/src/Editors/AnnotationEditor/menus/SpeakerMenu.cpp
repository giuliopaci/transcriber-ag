/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @file SpeakerMenu.cpp
 *   @brief speakers popup menu implementation
 */

#include <iostream>
#include <map>
#include <iterator>
#include <algorithm>
#include <gdk/gdk.h>
#include <gtkmm.h>

#include "SpeakerMenu.h"

#include "Common/globals.h"
#include "Common/util/Utils.h"

using namespace Gtk::Menu_Helpers;


namespace tag {


class SpeakerIsLower : public binary_function < const string&, const string&, bool >
{
	private :
		SpeakerDictionary& m_dic;
	public:
		SpeakerIsLower(SpeakerDictionary& dic) : m_dic(dic) {}
		bool operator() (const string& id1, const string& id2) {
			const Speaker& spk1 = m_dic.getSpeaker(id1);
			const Speaker& spk2 = m_dic.getSpeaker(id2);
			const string& s1 = spk1.getFullName();
			const string& s2 = spk2.getFullName();
			int i1, i2;
			bool ok1, ok2;
			const char* fmt = SpeakerDictionary::defaultFormat.c_str();
			ok1=(sscanf(s1.c_str(), fmt, &i1) == 1);
			ok2=(sscanf(s2.c_str(), fmt, &i2) == 1) ;
			if ( ok1 && ok2 )
				return ( i1 < i2 );
			else {
				if ( ok1 ) return false;
				if ( ok2 ) return true;
				return (s1.compare(s2) < 0);
			}
		}
};

std::map<string, string> SpeakerMenu::m_nospeechItems ;

void SpeakerMenu::initItems () {
	if ( m_nospeechItems.size() == 0 ) {
		m_nospeechItems["silence"] = _("(Silence)");
		m_nospeechItems["noise"] = _("(Noise)");
		m_nospeechItems["music"] = _("(Music)");
	}

}

/* constructor */
SpeakerMenu::SpeakerMenu(SpeakerDictionary& dic, string lang, bool overlap, bool editable)
  : AnnotationMenu(_("turn"), true, editable), m_speakersDict(dic), m_defaultLang(lang), m_editable(editable)
{
	if ( m_editable )
	{
		SpeakerMenu::initItems();

	//	for ( it=m_nospeechItems.begin(); it != m_nospeechItems.end(); ++it )
	//		m_noSpeakerSubmenu.items().push_back(MenuElem(it->second, sigc::bind<string>(sigc::mem_fun(*this, &SpeakerMenu::onSelectNoSpeech), it->first)));

		items().push_back(MenuElem("last speaker", sigc::bind<string>(sigc::mem_fun(*this, &SpeakerMenu::onSelectSpeaker), "")));

		m_nbLast = 1;

		items().push_back(MenuElem(_("New speaker"),  sigc::mem_fun(*this, &SpeakerMenu::onNewSpeaker)));

		items().push_back(SeparatorElem());

		m_nospeechIndex = items().size() - m_nbLast;
		items().push_back(MenuElem(Speaker::noSpeaker().getLastName(),
			sigc::bind<string>(sigc::mem_fun(*this, &SpeakerMenu::onSelectNoSpeech), Speaker::noSpeaker().getId())));

		items().push_back(SeparatorElem());

		if ( overlap ) {
			items().push_back(MenuElem(_("Overlapping speech"),
				sigc::bind<string>(sigc::mem_fun(*this, &SpeakerMenu::onSelectSpeaker), "overlap")));

			items().push_back(SeparatorElem());
		}

		items().push_back(MenuElem(_("Set speaker"), m_speakerSubmenu));
		items().push_back(MenuElem(_("Speaker properties"),
			sigc::mem_fun(*this, &SpeakerMenu::onEditSpeakerProperties)));
	}
	else
	{
		items().push_back(MenuElem(_("_Display speaker properties"),
			sigc::mem_fun(*this, &SpeakerMenu::onEditSpeakerProperties)));
	}
//	items().push_back(SeparatorElem());
}

/* destructor */
SpeakerMenu::~SpeakerMenu()
{
}


/* update menu item vs speakers dictionnary */
void
SpeakerMenu::updateMenu(bool can_create, bool can_edit, bool can_delete, bool can_be_unanchored)
{
	TRACE << " IN SpeakerMenu::updateMenu - editable=" << m_editable << endl << flush;
	if ( m_editable )
	{
		m_speakerSubmenu.items().clear();

	  SpeakerDictionary::const_iterator it;

		while ( m_nbLast > 0 ) {
			items().erase(items().begin());
			--m_nbLast;
		}

		insertLastSpeakerItem(getLastSelectedSpeaker());

		if ( m_lastSelectedSpeaker.size() > 1 )
			insertLastSpeakerItem(m_lastSelectedSpeaker.front());

		// set menu sensitivity
		for ( int i =0; i < (m_nbLast + 6); ++i )
			items()[i].set_sensitive(can_create);

		vector<string> sids;
		vector<string>::iterator its;

		/* display named speakers first, default speakers after */
		for ( it = m_speakersDict.begin(); it !=  m_speakersDict.end(); ++it ) {
			sids.push_back(it->first);
		}

		SpeakerIsLower cmp_func(m_speakersDict);
		std::sort(sids.begin(), sids.end(), cmp_func);

	//	if ( m_speakersDict.size() > 10 ) {
		bool is_default=false;
		const char* fmt = SpeakerDictionary::defaultFormat.c_str();

		string n1;
		int prev_range=0;
		int i1, cnt;

		its = sids.begin();
		Gtk::Menu*  submenu = &m_speakerSubmenu;

		while ( its != sids.end() )
		{
			if ( m_speakersDict.size() > 10 )
				submenu = Gtk::manage(new class Gtk::Menu() );  // speaker submenu
			for (cnt = 0 ; cnt < 9 && its != sids.end(); ++its ) {
				it = m_speakersDict.find(*its);
				const string& s1 =  it->second.getFullName() ;
				if ( cnt == 0 ) n1 = s1;
				if ( !is_default ) {
					is_default =(sscanf(s1.c_str(), fmt, &i1) == 1);
					if ( is_default ) {
						prev_range = (i1/10);
						if ( submenu->items().size() > 0 ) break;
					}
					++cnt;
				} else {
					cnt=1;
					sscanf(s1.c_str(), fmt, &i1);
					int nr = (i1/10);
					if ( nr > prev_range ) { prev_range=nr; break; }
				}
				submenu->items().push_back(MenuElem(it->second.getFullName(),
					sigc::bind<string>(sigc::mem_fun(*this, &SpeakerMenu::onSelectSpeaker), it->second.getId())));
			}
			if ( m_speakersDict.size() > 10 ) {
				m_speakerSubmenu.items().push_back(MenuElem(n1, *submenu));
				m_speakerSubmenu.items().back().set_sensitive(can_edit);
			}
		}
	/*
		}
		else {
			for ( it = m_speakersDict.begin(); it != m_speakersDict.end(); ++it ) {
				string name = it->second.getFullName();
				if ( name == "" ) name = it->second.getId() ;
				m_speakerSubmenu.items().push_back( MenuElem(name,
					  sigc::bind<string>(sigc::mem_fun(*this, &SpeakerMenu::onSelectSpeaker), it->second.getId())));
				m_speakerSubmenu.items().back().set_sensitive(can_edit);
			}
		}
	*/
	m_speakerSubmenu.items().push_back(SeparatorElem());
	}

	AnnotationMenu::updateMenu(can_create, can_edit, can_delete, can_be_unanchored);

}

/* Inherited method specialization: set active option to given hint */
void
SpeakerMenu::setHint(const std::string& hint)
{
	guint index=0;
	if ( ! hint.empty() ) {
		if ( hint == Speaker::NO_SPEECH ) {
			const string& lbl = Speaker::noSpeaker().getLastName();
			index = m_nospeechIndex + m_nbLast - 1;
		}
	}
	set_active(index);
}
//
// insert "last speaker" menu itemgetTextIter()
void
SpeakerMenu::insertLastSpeakerItem(const string& id)
{
	try {
	const Speaker& last_speaker = m_speakersDict.getSpeaker(id);
	items().push_front(MenuElem(last_speaker.getFullName(),
				sigc::bind<string>(sigc::mem_fun(*this, &SpeakerMenu::onSelectSpeaker), last_speaker.getId())));
	m_nbLast++;
	} catch (...) {
		MSGOUT << " getSpeaker " << id << endl;
	}
}


void SpeakerMenu::onSelectNoSpeech(string kind)
{
	if ( kind == Speaker::noSpeaker().getId() )
		m_signalSetSpeaker.emit(getTextIter(), kind, Speaker::noSpeaker().getFullName(), m_selectionStart, m_selectionEnd);
	else
		m_signalSetSpeaker.emit(getTextIter(), Speaker::noSpeaker().getId(), m_nospeechItems[kind], m_selectionStart, m_selectionEnd);
}

void SpeakerMenu::onSelectSpeaker(string id)
{
	if ( id != "overlap" ) 
	{
		setLastSelectedSpeaker(id);
		m_signalSetSpeaker.emit(getTextIter(), id, m_speakersDict.getSpeaker(id).getFullName(), m_selectionStart, m_selectionEnd);
	} 
	else 
	{
		m_signalSetSpeaker.emit(getTextIter(), id, "", m_selectionStart, m_selectionEnd);
	}
}

void SpeakerMenu::onNewSpeaker()
{
	const Speaker& new_speaker = m_speakersDict.defaultSpeaker(m_defaultLang);
    m_speakersDict.addSpeaker(new_speaker);
	setLastSelectedSpeaker(new_speaker.getId());
	m_signalSetSpeaker.emit(getTextIter(), new_speaker.getId(), new_speaker.getFullName(), m_selectionStart, m_selectionEnd);
}

void SpeakerMenu::setLastSelectedSpeaker(const string& id)
{
	string popid = m_lastSelectedSpeaker.front();
	if ( m_lastSelectedSpeaker.size() > 1 )
		m_lastSelectedSpeaker.pop();
	if ( id == m_lastSelectedSpeaker.back() ) {
		if ( id != popid ) m_lastSelectedSpeaker.push(popid);
	} else m_lastSelectedSpeaker.push(id);
}

void SpeakerMenu::onEditSpeakerProperties()
{
	 m_signalEditSpeaker.emit(getTextIter());
}

string SpeakerMenu::getLastSelectedSpeaker(bool alternate)
{
	if ( m_lastSelectedSpeaker.size() < 2 ) {
		if ( m_lastSelectedSpeaker.size() == 0 && m_speakersDict.empty() ) {
			const Speaker& speaker = m_speakersDict.defaultSpeaker();
			m_speakersDict.addSpeaker(speaker);
			m_lastSelectedSpeaker.push(speaker.getId());
		} else {
			SpeakerDictionary::iterator it = m_speakersDict.begin();
			if (m_lastSelectedSpeaker.size() == 0)
				m_lastSelectedSpeaker.push(it->second.getId());
			if ( m_speakersDict.size() > 1 ) {
				if ( m_lastSelectedSpeaker.front() == it->second.getId() )	++it;
				m_lastSelectedSpeaker.push(it->second.getId());
			}
		}
	}
	if (  m_lastSelectedSpeaker.size()  == 0 ) return "";

	if ( alternate ) return m_lastSelectedSpeaker.front();
	return m_lastSelectedSpeaker.back();
}

} /* namespace tag */
