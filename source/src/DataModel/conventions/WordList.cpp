/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  @file WordList.cpp
 *  @brief WordList class implementation
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>

// xerces
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/PlatformUtils.hpp>

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
#include "Common/globals.h"
#include "Common/util/StringOps.h"
#include "WordList.h"

using namespace std;

namespace tag {

/*
 * read word list from xml-formatted string
 */
void WordList::loadLists(std::string in, list<WordList>& lists)
throw(const char *)
{
	try
	{
		WordList_XMLHandler handler(lists);
		CommonXMLReader reader(&handler);
		reader.parseFile(in);
	}
	catch(const char *msg) {
		throw msg;
	}
}


WordList_XMLHandler::WordList_XMLHandler(list<WordList>& l)
	: m_wordLists(l)
{
	m_wordTag = NULL ;
	m_wordlistTag = NULL ;
	m_inword = false;
}

WordList_XMLHandler::~WordList_XMLHandler()
{
	if ( m_wordTag != NULL ) {
		XMLString::release (&m_wordTag);
		XMLString::release (&m_wordlistTag);
		m_inword=false;
	}
}


/* start XML element */
void
WordList_XMLHandler::startElement (const XMLCh * const uri,
			  const XMLCh * const localname,
			  const XMLCh * const qname, const Attributes & attrs)
{
	map < string, string > attmap;
	map < string, string >::iterator it;
	getAttributes (attrs, attmap);

	if ( m_wordlistTag == NULL ) {
		m_wordTag = XMLString::transcode("word");
		m_wordlistTag = XMLString::transcode("wordlist");
	}
	if ( XMLString::compareIString(localname, m_wordTag) == 0) {
		m_inword=true;
		m_curWord = "";
	} else {
		if ( XMLString::compareIString(localname, m_wordlistTag) == 0) {
			m_wordLists.push_back(WordList(attmap["label"]));
		}
	}
}

void
WordList_XMLHandler::endElement (const XMLCh * const uri,
			const XMLCh * const localname,
			const XMLCh * const qname)
{
	if ( m_inword ) {
		m_inword=false;
		if ( ! m_curWord.empty() )
			m_wordLists.back().addWord(m_curWord);
	}
}

void
WordList_XMLHandler::characters (const XMLCh * const chars, const unsigned int length)
{
  if (length != 0 && m_inword ) {
		m_curWord += getString(chars);
	}
}

} /* namespace tag */
