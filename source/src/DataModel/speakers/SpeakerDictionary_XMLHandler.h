/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/



#ifndef _HAVE_SPEAKER_DICT_XMLHANDLER
#define _HAVE_SPEAKER_DICT_XMLHANDLER

#include <map>
#include <ostream>

#include <xercesc/sax2/DefaultHandler.hpp>

#include "Common/CommonXMLReader.h"

using namespace std;

namespace tag {

class SpeakerDictionary;

/**
 *  @class SpeakerDictionary_XMLHandler
 *  @ingroup DataModel
 *  SAX-based XML parsing handler for speaker dictionary
 */
class SpeakerDictionary_XMLHandler : public CommonXMLHandler
{
  public:

	  /**
	   * constructor
	   * @param dic speaker dictionary where to store file contents
	   */
  SpeakerDictionary_XMLHandler(SpeakerDictionary* dic) ;
  /*! desctructor */
	~SpeakerDictionary_XMLHandler();

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

	/** Warning handler */
	void warning(const SAXParseException& exception);

	/** Error handler */
	void error(const SAXParseException& exception);

	/** Fatal error handler */
	void fatalError(const SAXParseException& exception);

  private:
	  SpeakerDictionary* m_speakerDic;
	  XMLCh*	m_speakerTag;
	  XMLCh*	m_langTag;
	  std::string	m_curId;
};

}

#endif // _HAVE_SPEAKER_DICT_XMLHANDLER
