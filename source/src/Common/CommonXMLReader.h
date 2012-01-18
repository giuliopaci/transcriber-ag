/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef _HAVE_COMMON_XML_READER
#define _HAVE_COMMON_XML_READER

#include <map>
#include <string>

// XERCES SAX
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>

#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

XERCES_CPP_NAMESPACE_USE
/**
* @class 		CommonXMLHandler
* @ingroup		Common
*
* Common base class for SAX-based XML elements handler
*
*/
class CommonXMLHandler : public DefaultHandler, public XMLFormatTarget
{
	public:
		CommonXMLHandler();
		~CommonXMLHandler();
		/**
		 * Converts SAX Attributes into string items map
		 * @param 		attrs		SAX attributes
		 * @param[out]	attmap 		Map where items will be placed
		 */
		virtual void getAttributes(const Attributes& attrs, std::map<std::string, std::string>& attmap);

		/**
		 * Parsing fatal error handler
		 * @param exception		Reference on the SAXParseException received
		 */
		virtual void fatalError(const SAXParseException& exception);

		/*!  */

		/**
		 * Compares XMLCh tag value with a string value
		 * @param xmltag			XMLCh tag value
		 * @param val				String value
		 * @param caseSensitive		Whether or not the comparison has to be case sensitive.
		 * @return					Result of comparison
		 */
		bool compare(const XMLCh* xmltag, const char* val, bool caseSensitive=true);

		/**
		 * Boolean value accessor
		 * @param val		Value in string representation
		 * @param defval	Default value if error
		 * @return			The corresponding boolean value
		 */
		bool getBoolValue(const std::string& val, bool defval=false)
		{ return getBoolValue(val.c_str(), defval); }
		/**
		 * Boolean value accessor
		 * @param val		Value in const char* representation
		 * @param defval	Default value if error
		 * @return			The corresponding boolean value
		 */
		bool getBoolValue(const char* val, bool defval=false);
		/**
		 * Boolean value accessor
		 * @param val		Value in const XMLCh* representation
		 * @param defval	Default value if error
		 * @return			The corresponding boolean value
		 */
		bool getBoolValue(const XMLCh* val, bool defval=false);

		/**
		 * String representation accessor
		 * @param chars		const XMLCh* string representation
		 * @return			The corresponding string representation
		 */
		const std::string& getString(const XMLCh* const chars);

		/**
		 * String representation accessor
		 * @param 		chars		const XMLCh* string representation
		 * @param[out]	s			The corresponding string representation
		 */
		void getString(const XMLCh* const chars, std::string& s);

	private:
		XMLFormatter* m_formatter;
		std::string* m_targetString;
		std::string m_buffer;
		void writeChars(const XMLByte* const toWrite,
					const unsigned int count,
					XMLFormatter* const formatter);
};

/**
* @class 		CommonXMLReader
* @ingroup		Common
*
* Common base class for SAX-based XML file readers
*
*/
class CommonXMLReader {

	public:
		/**
		 * Constructor
		 * @param handler		XML handler
		 * @param systemId		---
		 */
		CommonXMLReader(CommonXMLHandler* handler, const char* systemId=NULL) throw (const char*);
		~CommonXMLReader();

		/**
		 * Parses given file
		 * @param path	Path of the file to be parsed
		 */
		void parseFile(std::string path) throw (const char*);

		/**
		 * Parses given buffer
		 * @param buffer		String buffer
		 * @param systemId		---
		 */
		void parseBuffer(std::string buffer, const char* systemId=NULL) throw (const char*)
		{ parseBuffer(buffer.c_str(), buffer.size(), systemId); }

		/**
		 * Parses given buffer
		 * @param buffer		String buffer
		 * @param sz			Size buffer
		 * @param systemId		---
		 */
		void parseBuffer(const char* buffer, int sz, const char* systemId=NULL) throw (const char*);

	private:
		SAX2XMLReader* m_parser;
		XMLCh* m_fakeId;
};

#endif // _HAVE_COMMON_XML_READER
