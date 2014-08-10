/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/


#ifndef _HAVE_VERSION_LIST_XMLHANDLER
#define _HAVE_VERSION_LIST_XMLHANDLER

#include <map>
#include <list>
#include <ostream>

// XERCES SAX

#include <xercesc/sax2/DefaultHandler.hpp>

#include "Common/CommonXMLReader.h"
#include "VersionList.h"

using namespace std;

namespace tag {


/**
 *  @class VersionList_XMLHandler
 *  @ingroup DataModel
 *  SAX-based XML parsing handler for annotation file versions
 */
class VersionList_XMLHandler : public CommonXMLHandler
{
  public:

	  /**
	   * constructor
	   * @param l pointer on version list where to store loaded versions data
	   */
  VersionList_XMLHandler(VersionList* l) ;
	~VersionList_XMLHandler();

	/**
	 * handler for XML element start
	 * @param uri
	 * @param localname
	 * @param qname
	 * @param attrs
	 */
  void startElement( const   XMLCh* const    uri,
		     const   XMLCh* const    localname,
		     const   XMLCh* const    qname,
		     const   Attributes&     attrs);
	/**
	 * handler for XML element end
	 * @param uri
	 * @param localname
	 * @param qname
	 */
  void endElement( const XMLCh* const uri,
		   const XMLCh* const localname,
		   const XMLCh* const qname);

	/**
	 * handler for XML element character data
	 * @param chars
	 * @param length
	 */
  void characters(const XMLCh* const chars, const XMLSize_t length);

  private:
	  VersionList* m_versionList;
	  XMLCh*	m_versionTag;
	  std::string	m_curId;
	bool _inversion;
	string a_comment;

};

}

#endif // _HAVE_VERSION_LIST_XMLHANDLER
