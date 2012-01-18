/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#ifndef ANCHORLINKS_XMLHANDLER_H_
#define ANCHORLINKS_XMLHANDLER_H_

#include "AnchorLinks.h"
#include "Common/CommonXMLReader.h"

namespace tag {
/**
 *  @class 		AnchorLinks_XMLHandler
 *  @ingroup 	DataModel
 *  SAX-based XML parsing handler for anchor links
 */
class AnchorLinks_XMLHandler : public CommonXMLHandler
{
	public:
		/**
		 * COnstructor
		 * @param anchorLNK	AnchorLinks pointer object
		 */
		AnchorLinks_XMLHandler(AnchorLinks* anchorLNK);
		virtual ~AnchorLinks_XMLHandler();

	private:
		AnchorLinks* anchorLinks;
		XMLCh*	m_anchorLinksTag ;

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
};

} //namespace

#endif /* ANCHORLINKS_XMLHANDLER_H_ */
