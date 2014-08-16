// SAX_AGHandlers.h: AG format handler class
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _SAX_AGHANDLERS_H_
#define _SAX_AGHANDLERS_H_

#include <list>
#include <ag/AGTypes.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#ifdef XERCES_HAS_CPP_NAMESPACE
XERCES_CPP_NAMESPACE_USE
#endif

/// SAX AG handler class for Xerces C++ XML parser.
class SAX_AGHandlers : public HandlerBase, private XMLFormatTarget
{
private:
  list<AGId> agIds;  // contains id's of loaded AG's
  // they are needed by AIF::load(),
  // because it needs to return id's of AG's it has loaded
  string localDTD;

  string prevId;        // These are need to set features or medatada for
  string prevFeature;   // objects such as AGSet, Timeline, Signal and AG.
  string prevValue;     // They provide temporary storage.

/* (( BT Patch -- */
  bool inMetadata;	// to allow XML-formatted items in metadata
/* -- BT Patch )) */

  XMLFormatter* formatter;
  string targetString;
  void writeChars(const XMLByte* const, const XMLSize_t, XMLFormatter* const);
  void set_string(string&, const XMLCh* const);

  // StartFP: a type for function pointers to element handlers which are
  //     invoked when the start tag is detected
  // EndFP: a type for function pointers to element handlers which are
  //     invoked when the end tag is detected
  typedef void (SAX_AGHandlers::*StartFP)(const XMLCh* const, AttributeList&);
  typedef void (SAX_AGHandlers::*EndFP)(const XMLCh* const);

  // stack class
  // The reason why we don't use STL stack class is that
  // STL stack didn't work with function pointers
  template<class T>
  class stack
  {
  private:
    int itop;
    T array[8];  // the depth of AIF is 5, so 8 is enough

  public:
    stack(): itop(0) {}

    T top()
    {
      return array[itop];
    }

    void push(T f)
    {
      if (itop < 8)
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

public:
  SAX_AGHandlers(const string& outEncoding="UTF-8", const string& dtd="");
  ~SAX_AGHandlers();

  void set_encoding(const string&);
  void set_localDTD(const string& dtd) { localDTD = dtd; }
  list<AGId> get_agids() { return agIds; }

  void startElement(const XMLCh* const name, AttributeList& attr);
  void endElement(const XMLCh* const name);
  void characters(const XMLCh* const chars, const unsigned int length);
  virtual InputSource* resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId);

  void warning(const SAXParseException& exception);
  void error(const SAXParseException& exception);
  void fatalError(const SAXParseException& exception);
};

#endif

