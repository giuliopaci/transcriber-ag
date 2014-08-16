// agfXercesUtils.h: Xerces util functions
// Haejoong Lee
// Copyright (C) 2001,2002 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _AGFXERCESUTILS_H_
#define _AGFXERCESUTILS_H_

#define PluginUsesDOM

#include <ag/agfio.h>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#ifdef PluginUsesDOM
#include <xercesc/dom/DOM.hpp>
#endif
#include <xercesc/parsers/SAXParser.hpp>
#include <iostream>
#include <string>
#include <deque>

using namespace std;
#ifdef XERCES_HAS_CPP_NAMESPACE
XERCES_CPP_NAMESPACE_USE
#endif

class StrX: public XMLFormatter, public MemBufFormatTarget
{
private:
  const static unsigned short Qsize;
  deque<XMLCh*> Q;

public:
  StrX(const char* encoding = "UTF-8")
#if _XERCES_VERSION >= 20300
    : XMLFormatter(encoding, "1.0", this), Q(Qsize,NULL)
#else
    : XMLFormatter(encoding, this), Q(Qsize,NULL)
#endif
  {}

  ~StrX()
  { for (int i=0; i < Qsize; ++i) delete Q[i]; }

  string
  operator()(const XMLCh* src) {
    reset();
    (*this) << src;
    return string((char*) getRawBuffer());
  }

  XMLCh*
  operator()(const string& src) {
    delete Q.back();
    Q.pop_back();
    Q.push_front(XMLString::transcode(src.c_str()));
    return Q.front();
  }
};

void xercesc_open() throw(agfioError);
void xercesc_close();
string trans(const XMLCh*);
XMLCh* trans(const string&);

void
agfSAXParse(HandlerBase* handler,
	    const string& xml_file,
	    const bool& xml_validation,
	    const string& encoding="");

#ifdef PluginUsesDOM
class agfDOMErrorHandler : public ErrorHandler
{
private:
  bool fSawErrors;

  void print_error(const SAXParseException& toCatch) {
    cerr << "Error at file \""
	 << trans(toCatch.getSystemId()) << "\"" << endl
	 << "L" << toCatch.getLineNumber()
	 << ",C" << toCatch.getColumnNumber()
         << ": " << trans(toCatch.getMessage()) << endl;
  }

public:
  agfDOMErrorHandler():
    fSawErrors(false)
  {}

  ~agfDOMErrorHandler()
  {}


  void warning(const SAXParseException& toCatch) {}
  void error(const SAXParseException& toCatch) {
    cerr << "agfDOMErrorHandler::warning():" << endl
	 << "[warning] ";
    print_error(toCatch);
  }

  void fatalError(const SAXParseException& toCatch) {
    fSawErrors = true;
    cerr << "agfDOMErrorHandler::fatalError():" << endl
	 << "[fatal error] ";
    print_error(toCatch);
  }

  void resetErrors() {}

  bool getSawErrors() const
  { return fSawErrors; }

};

DOMDocument*
agfDOMParse(const string& xml_file,
	    const bool& xml_validation,
	    const string& encoding="");

DOMElement*
dom_elt(DOMNode* node);

string
node_name(DOMNode* node);

string
node_att(DOMNode* node, const string& att);

string
node_val(DOMNode* node);

bool
is_text(DOMNode* node);

DOMNodeList*
elts_by_name(DOMNode* node, const string& name);

DOMNodeList*
elts_by_name(DOMDocument* doc, const string& name);

DOMNode*
next_node(DOMNode* node);

DOMNode*
first_node(DOMNode* node);

void
att2map(DOMNode* node, map<string,string>& m);

DOMNode*
prev_node(DOMNode* node);

DOMNode*
last_node(DOMNode* node);

#endif // PluginUsesDOM
#endif
