/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	SAX_SGMLHandler.h
 */

#ifndef _SAX_SGMLHANDLERS_H_
#define _SAX_SGMLHANDLERS_H_

#include <list>
#include <map>
#include <string>
#include <vector>
#include <sstream>

#include <ag/AGTypes.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/Locator.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include "DataModel/DataModel.h"

#include "SGMLobjects.h"

#ifdef XERCES_HAS_CPP_NAMESPACE
XERCES_CPP_NAMESPACE_USE
#endif

/**
* @class 	SAX_SGMLHandler
* @ingroup	Formats
*
* SAX SGML handler class for Xerces C++ XML parser.
*/
class SAX_SGMLHandler : public HandlerBase, private XMLFormatTarget
{
	public:
		/**
		 * Constructor
		 * @param data				Reference on the model AG used (will be filled while parsing)
		 * @param SGMLmodel			SGMLmodel that will be filled by parser
		 * @param outEncoding		Output encoding
		 * @return
		 */
		SAX_SGMLHandler(tag::DataModel& data, SGMLobjects* SGMLmodel, const string& outEncoding="UTF-8");
		~SAX_SGMLHandler();

		/**
		 * Specifies output encoding.
		 * @param encoding		Encoding name
		 */
		void set_encoding(const string& encoding);

		/** Start element handler */
		void startElement(const XMLCh* const name, AttributeList& attr);

		/** End element handler */
		void endElement(const XMLCh* const name);

		/** Characters element handler */
		void characters(const XMLCh* const chars, const unsigned int length);

		/** Warning handler */
		void warning(const SAXParseException& exception);

		/** Error handler */
		void error(const SAXParseException& exception);

		/** Fatal error handler */
		void fatalError(const SAXParseException& exception);

		/**
		 * Specifies the SGML model into which all parsed data will be stored.
		 * @param model		Pointer on an allocated SGML model
		 */
		void setSGMLmodel(SGMLobjects* model) { sgmlModel = model ;}

	private:
		// stack class
		// The reason why we don't use STL stack class is that
		// STL stack didn't work with function pointers
		template<class T>
		class stack
		{
			private:
				int itop;
				T array[12];  // the depth of VRXML may be 9,  so 12 should do it

			public:
				stack(): itop(0) {}

				T top()
				{
					return array[itop];
				}

				void push(T f)
				{
					if (itop < 12)
						array[++itop] = f;
					else
						throw ("stack overflow");
				}

				void pop()
				{
					if (itop > 0)
						itop--;
					else
						throw ("stack underflow");
				}
		};

		virtual InputSource* resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId);

		/* Deals with encoding */
		string input_encoding ;
//		Locator2 myLocator ;

		tag::DataModel& 	m_dataModel;	// resulting AG-format data model

		SGMLobjects* sgmlModel ;

		string currentCDATAvalue ;

		SGMLobjects::SGMLspeaker* current_SGMLspeaker ;
		SGMLobjects::SGMLpath* current_SGMLpath ;

		// AGlib stuff
		XMLFormatter* formatter;
		string targetString;
		string localDTD;

		void writeChars(const XMLByte* const, const unsigned int, XMLFormatter* const);
		string& set_string(string&, const XMLCh* const);

		// StartFP: a type for function pointers to element handlers which are
		//     invoked when the start tag is detected
		// EndFP: a type for function pointers to element handlers which are
		//     invoked when the end tag is detected
		typedef void (SAX_SGMLHandler::*StartFP)(const XMLCh* const, AttributeList&);
		typedef void (SAX_SGMLHandler::*EndFP)(const XMLCh* const);


		// stacks for element handlers
		stack<StartFP> StartStack;
		stack<EndFP>   EndStack;

		// do-nothing handlers
		void dummyStart(const XMLCh* const name, AttributeList& attr);
		void dummyEnd(const XMLCh* const name);

		// element handlers

		void ElementCDATAStart(const XMLCh* const name, AttributeList& attr) ;
		void EndBranch() ;

		void SGMLSubStart(const XMLCh* const name, AttributeList& attr) ;
			void SystemStart(const XMLCh* const name, AttributeList& attr) ;
				void CategoryStart(const XMLCh* const name, AttributeList& attr) ;
				void LabelStart(const XMLCh* const name, AttributeList& attr) ;
				void SpeakerStart(const XMLCh* const name, AttributeList& attr) ;
					void PathStart(const XMLCh* const name, AttributeList& attr) ;
					void PathEnd(const XMLCh* const name) ;
				void CategoryEnd(const XMLCh* const name) ;
				void LabelEnd(const XMLCh* const name) ;
				void SpeakerEnd(const XMLCh* const name) ;
};

#endif
