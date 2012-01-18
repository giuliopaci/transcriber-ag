/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**************************************************************************
 *            CommonXMLReader.cpp
 *
 ****************************************************************************/


// xerces
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <sstream>
#include <map>
#include <string.h>
#include <errno.h>
#include <time.h>
//#include <glib.h>

// STL en fonction de XERCES
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif
#include <xercesc/framework/LocalFileInputSource.hpp>
#include "CommonXMLReader.h"

using namespace std;

CommonXMLHandler::CommonXMLHandler()
{
  try
    {
      XMLPlatformUtils::Initialize ();
    } catch (const XMLException & toCatch)  {
      throw "Caught exception when initializing Xerces-SAX";
    }

  m_formatter = new XMLFormatter("UTF-8",
#if _XERCES_VERSION >= 20300
                               "1.0",
#endif
                               this,
                               XMLFormatter::NoEscapes,
                               XMLFormatter::UnRep_CharRef);
}

CommonXMLHandler::~CommonXMLHandler()
{
	delete m_formatter;
  XMLPlatformUtils::Terminate ();
}


/**
 *  CommonXMLHandler::getAttributes : transcode element attributes from XMLCh
 *          to strings.
 */

void
CommonXMLHandler::getAttributes (const Attributes & attrs, map < string, string >& attmap)
{
  int len = attrs.getLength ();

  attmap.clear ();
  for (int index = 0; index < len; index++)
  {
		string name;
		getString(attrs.getQName (index), name);
 		attmap[name] = getString(attrs.getValue (index));
 }
}

/**
 *  CommonXMLHandler::fatalError : intercepts XML Parse error and send back message
 */

void
CommonXMLHandler::fatalError (const SAXParseException & exception) 
{
	ostringstream err;
	err << "XML Parse Error at line " << exception.getLineNumber ()
       << " col " << exception.getColumnNumber () ;
	throw err.str().c_str();
}


bool
CommonXMLHandler::compare (const XMLCh* xmltag, const char* val, bool caseSensitive) 
{
	XMLCh* xmlval = XMLString::transcode (val);
	
	int ret = (caseSensitive ? 
				XMLString::compareString(xmltag, xmlval) :
				XMLString::compareIString(xmltag, xmlval) );
	
	XMLString::release(&xmlval);
	return (ret == 0);
}


bool
CommonXMLHandler::getBoolValue (const XMLCh* xmlval, bool defval) 
{
	char* val = XMLString::transcode (xmlval);
	bool ret = getBoolValue(val, defval);
	XMLString::release(&val);
	return ret;
}

bool
CommonXMLHandler::getBoolValue (const char* val, bool defval) 
{
	bool ret = false;

	if ( *val == 0 ) 
		ret = defval;
	else 
		if ( strcasecmp(val, "1") == 0 
			|| strcasecmp(val, "t") == 0 || strcasecmp(val, "true") == 0 
			|| strcasecmp(val, "y") == 0 || strcasecmp(val, "yes") == 0 
			|| strcasecmp(val, "o")  == 0 || strcasecmp(val, "oui") == 0 
			|| strcasecmp(val, "v")  == 0 || strcasecmp(val, "vrai") == 0 
		)
			ret = true;
	
	return ret;
}

const string& 
CommonXMLHandler::getString(const XMLCh* const chars)
{
	m_targetString = &m_buffer;
  m_targetString->erase();
  (*m_formatter) << chars;
	return m_buffer;
}

void
CommonXMLHandler::getString(const XMLCh* const chars, string& s)
{
  m_targetString = &s;
  m_targetString->erase();
  (*m_formatter) << chars;
}

void
CommonXMLHandler::writeChars(const XMLByte* const toWrite,
                                 const unsigned int count,
                                 XMLFormatter* const formatter)
{
  m_targetString->assign((char*) toWrite, count);
}


/**
 *   CommonXMLReader Constructor
 */

CommonXMLReader::CommonXMLReader (CommonXMLHandler* handler, const char* systemId) throw (const char *)
{
  m_parser = XMLReaderFactory::createXMLReader ();

  m_parser->setFeature (XMLUni::fgSAX2CoreValidation, true);
  m_parser->setFeature (XMLUni::fgXercesLoadExternalDTD, true);

  m_parser->setContentHandler (handler);
  m_parser->setErrorHandler (handler);

	m_fakeId = XMLString::transcode((systemId==NULL ? "" : systemId));

}

/**
 * CommonXMLReader destructor
 */
CommonXMLReader::~CommonXMLReader ()
{
  delete m_parser;
	XMLString::release (&m_fakeId);
}


/**
 *   parse XML file 
 */
void
CommonXMLReader::parseFile (string path)   throw (const char *)
{
	try    {
      m_parser->parse (path.c_str ());
    }  catch (const OutOfMemoryException &)  {
      throw "ERROR: caught OutOfMemoryException";
    } catch (const XMLException & toCatch)  {
      throw "ERROR :  caught XMLException" ;
    } catch (const char* msg)     {
      throw msg ;
    }
}


/**
 *   parse XML buffer
 */
void
CommonXMLReader::parseBuffer (const char* buffer, int sz, const char* systemId)   throw (const char *)
{
	try    {
		
		if ( systemId != NULL ) {
			XMLString::release (&m_fakeId);
			m_fakeId = XMLString::transcode(systemId);
		}
		
		MemBufInputSource source((const XMLByte*)buffer, sz, m_fakeId);
		m_parser->parse(source);
    }  catch (const OutOfMemoryException &)  {
      throw "ERROR: caught OutOfMemoryException";
    } catch (const XMLException & toCatch)  {
      throw "ERROR :  caught XMLException" ;
    } catch (const char* msg)     {
      throw msg ;
    }
}
