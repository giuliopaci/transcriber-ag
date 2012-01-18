/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  @class SpeakerDictionary_XMLHandler
 *  @brief  Read dictionary from XML file or buffer
 */

// xerces
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <sstream>
#include <map>
#include <string.h>
#include <errno.h>
#include <time.h>

// STL en fonction de XERCES
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif

#include "Common/FileInfo.h"
#include "Common/util/StringOps.h"
#include "SpeakerDictionary_XMLHandler.h"
#include "SpeakerDictionary.h"

using namespace std;

namespace tag {


SpeakerDictionary_XMLHandler::SpeakerDictionary_XMLHandler(SpeakerDictionary* dic)
	: m_speakerDic(dic), m_curId("")
{
	m_speakerTag = NULL ;
	m_langTag = NULL ;
}

SpeakerDictionary_XMLHandler::~SpeakerDictionary_XMLHandler()
{
	if ( m_speakerTag != NULL ) {
		XMLString::release (&m_speakerTag);
		XMLString::release (&m_langTag);
	}
}


/* start XML element */
void
SpeakerDictionary_XMLHandler::startElement (const XMLCh * const uri,
			  const XMLCh * const localname,
			  const XMLCh * const qname, const Attributes & attrs)
{
	map < string, string > attmap;
	map < string, string >::iterator it;
	getAttributes (attrs, attmap);

	if ( m_speakerTag == NULL ) {
		m_speakerTag = XMLString::transcode("speaker");
		m_langTag = XMLString::transcode("spokenlanguage");
	}

	if ( XMLString::compareIString(localname, m_speakerTag) == 0) {
		Speaker speaker;
		for ( it=attmap.begin(); it != attmap.end(); ++it )
			speaker.setProperty(it->first, it->second);
		if ( m_speakerDic->addSpeaker(speaker) ) m_curId = attmap["id"];
			else m_curId = "";
	} else if ( XMLString::compareIString(localname, m_langTag) == 0 && !m_curId.empty()) {
		Speaker::Language lang(attmap["code"], getBoolValue(attmap["isnative"]),
					getBoolValue(attmap["isusual"]), attmap["accent"],
					attmap["dialect"]);
		Speaker& speaker = m_speakerDic->getSpeaker(m_curId);
		speaker.addLanguage(lang);
	}
}

void SpeakerDictionary_XMLHandler::warning(const SAXParseException& e)
{
	cerr << "WARNING: " << getString(e.getMessage()) << endl;
	cerr << " at line " << e.getLineNumber () << " col " << e.getColumnNumber () << endl;
}

void SpeakerDictionary_XMLHandler::error(const SAXParseException& e)
{
	cerr << "WARNING: " << getString(e.getMessage()) << endl;
	cerr << " at line " << e.getLineNumber () << " col " << e.getColumnNumber () << endl;
}

void SpeakerDictionary_XMLHandler::fatalError(const SAXParseException& e)
{
	cerr << "ERROR: " << getString(e.getMessage()) << endl;
	cerr << " at line " << e.getLineNumber () << " col " << e.getColumnNumber () << endl;
	throw getString(e.getMessage()).c_str();
}


} //namespace
