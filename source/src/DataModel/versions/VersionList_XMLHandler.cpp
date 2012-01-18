/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  @class VersionList_XMLHandler
 *  @brief  Read list from XML file or buffer
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
#include "VersionList_XMLHandler.h"

using namespace std;

namespace tag {


VersionList_XMLHandler::VersionList_XMLHandler(VersionList* l)
	: m_versionList(l), m_curId("")
{
	m_versionTag = NULL ;
	_inversion = false;
}

VersionList_XMLHandler::~VersionList_XMLHandler()
{
	if ( m_versionTag != NULL ) {
		XMLString::release (&m_versionTag);
	}
}


/* start XML element */
void
VersionList_XMLHandler::startElement (const XMLCh * const uri,
			  const XMLCh * const localname,
			  const XMLCh * const qname, const Attributes & attrs)
{
	map < string, string > attmap;
	map < string, string >::iterator it;
	getAttributes (attrs, attmap);

	if ( m_versionTag == NULL ) {
		m_versionTag = XMLString::transcode("version");
	}
	a_comment = "";

	if ( XMLString::compareIString(localname, m_versionTag) == 0) {
		m_versionList->addVersion(Version(attmap["id"], attmap["date"], attmap["author"], attmap["wid"], attmap["tag_version"]));
		_inversion = true;
	}
}

void
VersionList_XMLHandler::endElement (const XMLCh * const uri,
			const XMLCh * const localname,
			const XMLCh * const qname)
{
  char *message = XMLString::transcode (localname);
  if (strcasecmp (message, "version") == 0 ) {
	_inversion = false;
	m_versionList->back().setComment(a_comment);
	  a_comment="";
}
  XMLString::release (&message);

}

void
VersionList_XMLHandler::characters (const XMLCh * const chars, const unsigned int length)
{

  if (length != 0)
    {
      if (_inversion)
	{
	  char *strContenu = XMLString::transcode (chars);
	  if (*strContenu) 	    {
		a_comment += strContenu;
	    }
	  XMLString::release (&strContenu);
	}
    }
}

}
