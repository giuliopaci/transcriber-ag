/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  @file SpeakerDictionary.cpp
 *  @brief SpeakerDictionary class implementation
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLString.hpp>

#include "Common/globals.h"
#include "Common/util/StringOps.h"

#include "DataModel/speakers/SpeakerDictionary.h"
#include "DataModel/speakers/SpeakerDictionary_XMLHandler.h"

using namespace std;

namespace tag {

string SpeakerDictionary::defaultFormat(_("speaker#%d"));	/* default speaker name format */
string SpeakerDictionary::idFormat(_("spk%d"));	/* default speaker name format */

/*
 *  load dictionary from given url
 *    for the time beeing, only file protocol supported
 */
void SpeakerDictionary::loadDictionary(string url) throw(const char *)
		{
	try
	{
		clear();
		SpeakerDictionary_XMLHandler handler(this);
		CommonXMLReader reader(&handler);
		 reader.parseFile(url);
	} catch(const char *msg) {
		throw msg;
	}
}


/*
*  save dictionary from given url
*    for the time beeing, only file protocol supported
*/
bool SpeakerDictionary::saveDictionary(string url)
{
	ofstream fo(url.c_str());
	if ( ! fo.good() ) return false;
	fo << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
	fo << "<Speakers>" << endl;
	toXML(fo, "\n");
	fo << "</Speakers>" << endl;
	fo.close();
	return true;
}

/*
*  check that speaker with given id exists in dictionary
*     return true if ok, else false
*/
bool SpeakerDictionary::existsSpeaker(const string & id)
{
	map < string, Speaker >::iterator it = m_speakers.find(id);

	return (it != m_speakers.end());
}


/*
*  get speaker with given id
*     return pointer on speaker, NULL if not found
*/
Speaker& SpeakerDictionary::getSpeaker(const string & id) throw(NotFoundException)
{
	map < string, Speaker >::iterator it = m_speakers.find(id);

	if (it == m_speakers.end())
		throw NotFoundException("id", id);
	return it->second;
}

/*
*  get speaker with given last name
*     return pointer on 1st speaker found, NULL if not found
*/
Speaker & SpeakerDictionary::getSpeakerByName(const string & lastname) throw(NotFoundException)
{
	map < string, Speaker >::iterator it;

	for (it = m_speakers.begin(); it != m_speakers.end(); ++it)
		if (lastname == it->second.getLastName())
			break;

	if (it == m_speakers.end())
		throw NotFoundException("lastname", lastname);
	return it->second;
}

/*
* add new speaker to dictionary
* return true if speaker added / false if already exists
*/
bool SpeakerDictionary::addSpeaker(const Speaker & speaker)
{
	map < string, Speaker >::iterator it =
		m_speakers.find(speaker.getId());

	if (it != m_speakers.end())
		return false;

	m_speakers[speaker.getId()] = speaker;
	if ( speaker.getFirstName() == "" && speaker.getLastName() == "" )
	{
		int i;
		char name[40];
		sscanf(speaker.getId().c_str(), idFormat.c_str(), &i);
		sprintf(name, defaultFormat.c_str(), i);
		m_speakers[speaker.getId()].setLastName(name);
	}

	return true;
}

/*
* update speaker definition in dictionary
* return true if speaker updated / false if not
*/
bool SpeakerDictionary::updateSpeaker(const Speaker & speaker, bool auto_add)
{
	string id = speaker.getId() ;
	if ( id == "" )
	{
		if ( ! auto_add ) return false;
		id = getUniqueId();
	}
	else
	{
		map < string, Speaker >::iterator it =
			m_speakers.find(id);
		if (it == m_speakers.end() && !auto_add)
			return false;
	}
	m_speakers[id] = speaker;
	m_speakers[id].setId(id);
	signalSpeakerUpdated().emit(id);
	return true;
}


/*
* delete speaker definition from dictionary
*/
bool SpeakerDictionary::deleteSpeaker(const string& id)
{
	map < string, Speaker >::iterator it = m_speakers.find(id);
	/**
	* @note special treatment
	*/
	if (it == m_speakers.end())
		return false;
	m_speakers.erase(it);
	signalSpeakerDeleted().emit(id) ;
	return true;
}

/* return indicative speaker rank in dictionary */

int SpeakerDictionary::getSpeakerRank(const string& id)
{
	map < string, Speaker >::iterator it = m_speakers.find(id);
	if (it == m_speakers.end())
		return -1;
	return distance(m_speakers.begin(), it);
}

/*
* defaultSpeaker
*  fill default speaker information
*/

const Speaker& SpeakerDictionary::defaultSpeaker(string lang)
{
	char name[40];

	map < string, Speaker >::iterator it;

	const string& id = getUniqueId(name);
	m_defaultSpeaker.setId(id);
	m_defaultSpeaker.setLastName(name);
	if (!lang.empty())
		m_defaultSpeaker.addLanguage(lang, true, true);
	return m_defaultSpeaker;
}

/*! return unique speaker id  */
std::string SpeakerDictionary::getUniqueId(char* name)
{
	char id[40];
	int i;
	bool ok = false;

	for (i = size() + 1; !ok; ++i)	{
		sprintf(id, idFormat.c_str(), i);
		ok = (m_speakers.find(id) == m_speakers.end());
		if (ok) {
			if ( name != NULL )
				sprintf(name, defaultFormat.c_str(), i);
			break;
		}
	}
	return id;
}

/*! return default name for given speaker id  */
std::string SpeakerDictionary::getDefaultName(const string& id)
{
	int i=0;
	char name[40];
	sscanf(id.c_str(), idFormat.c_str(), &i);
	sprintf(name, defaultFormat.c_str(), i);
	return name;
}

/*
* print out speakers dictionary as XML-formatted string
*/
std::ostream& SpeakerDictionary::toXML(std::ostream & out, const char *delim) const
{
	map < string, Speaker >::const_iterator it;
	out << "<Speakers>" << delim;
	for (it = m_speakers.begin(); it != m_speakers.end(); ++it)
		it->second.toXML(out, delim);
	out << "</Speakers>" << delim;
	return out;
}

/*
* read speaker def from xml-formatted string
*/
void SpeakerDictionary::fromXML(const std::string& in, const std::string& dtd) throw(const char *)
{
	try
	{
		SpeakerDictionary_XMLHandler handler(this);
		CommonXMLReader reader(&handler);

		if ( !dtd.empty() )
		{
			std::string head = "<?xml version=\"1.0\" encoding=\"utf-8\"?>" ;
			head.append("\n") ;
			head.append("<!DOCTYPE Speakers SYSTEM ") ;
			head.append("\"") ;
			head.append(dtd) ;
			head.append("\">") ;

			ostringstream os;
			os << head << endl ;
			unsigned int pos = in.find("<");
			if (in.compare(pos, 10, "<Speakers>") != 0)
			{
				os << "<Speakers>" << in << "</Speakers>" << endl;;
			} else
				os << in << endl;
			reader.parseBuffer(os.str());
		}
		else
		{
			unsigned int pos = in.find("<");
			if (in.compare(pos, 10, "<Speakers>") != 0)
			{
				ostringstream os;
				os << "<Speakers>" << in << "</Speakers>";
				reader.parseBuffer(os.str());
			}
			else
				reader.parseBuffer(in);
		}
	}
	catch(const char *msg)
	{
		throw msg;
	}
}

}	/* namespace tag */
