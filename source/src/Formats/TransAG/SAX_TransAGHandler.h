/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// SAX_TransAGHandler.h: TransAG format handler class
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

/* $Id */

/**
 * @file	SAX_TransAGHandler.h
 */

#ifndef _SAX_TransAGHANDLERS_H_
#define _SAX_TransAGHANDLERS_H_

#include <list>
#include <ag/AGTypes.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#ifdef XERCES_HAS_CPP_NAMESPACE
XERCES_CPP_NAMESPACE_USE
#endif

#define STACK_SIZE 10 // the depth of AIF is 5, so we use 10 to cope with structured metadata
/**
* @class 	SAX_TransAGHandler
* @ingroup	Formats
*
* SAX TransAG handler class for Xerces C++ XML parser.
*/
class SAX_TransAGHandler : public HandlerBase, private XMLFormatTarget
{
	public:
		/**
		 * Constructor
		 * @param outEncoding		Output encoding
		 * @param dtd				Dtd
		 */
		SAX_TransAGHandler(const string& outEncoding="UTF-8", const string& dtd="");
		~SAX_TransAGHandler();

		/**
		 * Specifies output encoding.
		 * @param encoding		Encoding name
		 */
		void set_encoding(const string& encoding);

		/**
		 * Specifies local dtd.
		 * @param dtd		Local dtd file
		 */
		void set_localDTD(const string& dtd) { localDTD = dtd; }

		/**
		 * Accessor to all AG elements created while parsing file.
		 * @return		A list containing identifiers of all created AG elements.
		 */
		list<AGId> get_agids() { return agIds; }

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


	private:
	  list<AGId> agIds;  // contains id's of loaded AG's
	  list<AGId> agSetIds;  // contains id's of loaded AGSets's
	  // they are needed by AIF::load(),
	  // because it needs to return id's of AG's it has loaded
	  string localDTD;

	  string prevId;        // These are need to set features or medatada for
	  string prevFeature;   // objects such as AGSet, Timeline, Signal and AG.
	  string prevValue;     // They provide temporary storage.
	  unsigned int prevPos; //

	  XMLFormatter* formatter;
	  string* targetString;
	  void writeChars(const XMLByte* const, const XMLSize_t, XMLFormatter* const);
	  string& set_string(string&, const XMLCh* const);

	  // StartFP: a type for function pointers to element handlers which are
	  //     invoked when the start tag is detected
	  // EndFP: a type for function pointers to element handlers which are
	  //     invoked when the end tag is detected
	  typedef void (SAX_TransAGHandler::*StartFP)(const XMLCh* const, AttributeList&);
	  typedef void (SAX_TransAGHandler::*EndFP)(const XMLCh* const);

	  // stack class
	  // The reason why we don't use STL stack class is that
	  // STL stack didn't work with function pointers
	  template<class T>
	  class stack
	  {
	  private:
		int itop;
		T array[STACK_SIZE];

	  public:
		stack(): itop(0) {}

		T top()
		{
		  return array[itop];
		}

		void push(T f)
		{
		  if (itop < STACK_SIZE)
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

	  // stacks for element handlers
	  stack<StartFP> StartStack;
	  stack<EndFP>   EndStack;

	  string a_version ;
	  string a_agId ;

	  // do-nothing handlers
	  void dummyStart(const XMLCh* const name, AttributeList& attr);
	  void dummyEnd(const XMLCh* const name);
	  // element handlers
	  void AGSetStart(const XMLCh* const name, AttributeList& attr);
	  void AGSetEnd(const XMLCh* const name);
	  void AGSetSubStart(const XMLCh* const name, AttributeList& attr);
	  void MetadataSubStart(const XMLCh* const name, AttributeList& attr);
	  void MetadataSubEnd(const XMLCh* const name);
	  void TimelineSubStart(const XMLCh* const name, AttributeList& attr);
	  void SignalSubStart(const XMLCh* const name, AttributeList& attr);
	  void AGSubStart(const XMLCh* const name, AttributeList& attr);
	  void AnnotationSubStart(const XMLCh* const name, AttributeList& attr);
	  void FeatureEnd(const XMLCh* const name);

	  void structuredMetaStart(const XMLCh* const name, AttributeList& attr);
	  void structuredMetaEnd(const XMLCh* const name);

	  virtual InputSource* resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId);

};

#endif

