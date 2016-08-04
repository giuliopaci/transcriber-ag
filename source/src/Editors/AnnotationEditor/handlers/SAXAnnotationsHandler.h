/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		SAXAnnotationsHandler.h
 */

#ifndef __HAVE_SAXANNOTATIONSHANDLER__
#define __HAVE_SAXANNOTATIONSHANDLER__

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLNumber.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>
#include <list>

XERCES_CPP_NAMESPACE_USE
using namespace std;

#ifndef __HAVE_PROPERTY__
#define __HAVE_PROPERTY__
/**
 * @struct	Property
 * Structure used for representing a dynamic property
 */
struct Property
{
	string label;			/**< Property label */
	int type;				/**< Property type */
	string choiceList;		/**< Property choice list */
};

const int PROPERTY_TEXT = 0;			/**< Property text type */
const int PROPERTY_CHOICELIST = 1;		/**< Property choice list type */
#endif

/**
 * @class SAXAnnotationsHandler
 *
 * SAX handler for parsing annotation XML representation.\n
 * Parses a specific properties files defined in XML languages, placed
 * in the TranscriberAG configuration folder.
 */
class SAXAnnotationsHandler : public DefaultHandler
{
	public :
		/**
		 * Constructor
		 */
		SAXAnnotationsHandler() { };

		/**
		 * Constructor
		 * @param p_annotationProperties	Pointer on map into which all parsed annotation properties will be placed
		 * @param p_fileProperties			Pointer on map into which all parsed file properties will be placed
		 * @param p_choiceLists				Pointer on map into which all parsed choice list will be placed
		 */
		SAXAnnotationsHandler(std::map<string, std::list<Property> >* p_annotationProperties, std::list<Property>* p_fileProperties, std::map<std::string, std::list<std::string> >* p_choiceLists);

		/**
		 * Destructor
		 */
		virtual ~SAXAnnotationsHandler() { };

		/**! Start element handler */
		void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs);

		/**! End element handler */
		void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname);

		/**! Character element handler */
		void characters(const XMLCh* const chars, const XMLSize_t length);

	private :
		std::map<string, list<Property> >* a_annotationProperties;
		list<Property>* a_fileProperties;
		std::map<string, list<string> >* a_choiceLists;

		string a_choiceListID;
		list<string> a_choiceListValues;
		list<Property>* a_properties;
};

#endif // __HAVE_SAXANNOTATIONSHANDLER__
