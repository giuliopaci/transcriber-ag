/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 *  @class  AnchorLinks_XMLHandler
 *  @brief  Read anchor links format
 */

#include "AnchorLinks_XMLHandler.h"

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


namespace tag {

AnchorLinks_XMLHandler::AnchorLinks_XMLHandler(AnchorLinks* anchorLNK)
: anchorLinks(anchorLNK)
{
	m_anchorLinksTag = XMLString::transcode("anchorlink");
}

AnchorLinks_XMLHandler::~AnchorLinks_XMLHandler()
{
	if ( m_anchorLinksTag != NULL )
		XMLString::release (&m_anchorLinksTag);
}

/* start XML element */
void AnchorLinks_XMLHandler::startElement (const XMLCh * const uri,
			  const XMLCh * const localname,
			  const XMLCh * const qname, const Attributes & attrs)
{
	std::map < std::string, std::string > attmap;
	std::map < std::string, std::string >::iterator it;
	getAttributes (attrs, attmap);

	if ( XMLString::compareIString(localname, m_anchorLinksTag) == 0)
		anchorLinks->loadLinks(attmap["id"], attmap["links"]) ;
}

void AnchorLinks_XMLHandler::warning(const SAXParseException& e)
{
	std::cerr << "WARNING: " << getString(e.getMessage()) << std::endl;
	std::cerr << " at line " << e.getLineNumber () << " col " << e.getColumnNumber () << std::endl;
}

void AnchorLinks_XMLHandler::error(const SAXParseException& e)
{
	std::cerr << "WARNING: " << getString(e.getMessage()) << std::endl;
	std::cerr << " at line " << e.getLineNumber () << " col " << e.getColumnNumber () << std::endl;
}

void AnchorLinks_XMLHandler::fatalError(const SAXParseException& e)
{
	std::cerr << "ERROR: " << getString(e.getMessage()) << std::endl;
	std::cerr << " at line " << e.getLineNumber () << " col " << e.getColumnNumber () << std::endl;
	throw getString(e.getMessage()).c_str();
}

} // namespace
