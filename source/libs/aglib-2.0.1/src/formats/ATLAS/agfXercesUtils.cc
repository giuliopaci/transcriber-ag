// agfXercesUtils.cc: Xerces util functions
// Haejoong Lee
// Copyright (C) 2001,2002 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.



#include "agfXercesUtils.h"
#include <xercesc/framework/LocalFileInputSource.hpp>
#ifdef PluginUsesDOM
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOM.hpp>
static XercesDOMParser* DOMPARSER;
#endif

const unsigned short StrX::Qsize = 16;
static bool xercesc_opened = false;
static StrX* TRANS;

void
xercesc_open()
  throw(agfioError)
{
  if (xercesc_opened)
    return;

  try {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& e) {
    string message;
    message = "agfXercesUtils.cc:xercesc_open():\n";
    message += "[fatal] Error during Xerces-c Initialization\n";
    message += "Message:" + trans(e.getMessage()) + "\n";
    throw agfioError(message);
  }

  TRANS = new StrX;
#ifdef PluginUsesDOM
  DOMPARSER = new XercesDOMParser;
#endif
  xercesc_opened = true;
}

void
xercesc_close()
{
  if (! xercesc_opened)
    return;

#ifdef PluginUsesDOM
  delete DOMPARSER;
#endif
  delete TRANS;
  //XMLPlatformUtils::Terminate();
  xercesc_opened = false;
}

string
trans(const XMLCh* s)
{ return (*TRANS)(s); }

XMLCh*
trans(const string& s)
{ return (*TRANS)(s); }

void agfSAXParse(HandlerBase* handler,
		 const string& xml_file,
		 const bool& xml_validation,
		 const string& encoding)
{
  LocalFileInputSource inputSrc(trans(xml_file));
  if (! encoding.empty())
    inputSrc.setEncoding(trans(encoding));

  SAXParser parser;
  parser.setDocumentHandler(handler);
  parser.setErrorHandler(handler);
  if (xml_validation) {
    parser.setValidationScheme(SAXParser::Val_Auto);
    parser.setDoValidation(true);
  }
  else {
    parser.setLoadExternalDTD(false);
    parser.setValidationScheme(SAXParser::Val_Never);
    parser.setDoValidation(false);
  }
  parser.setDoNamespaces(false);
  parser.setEntityResolver(handler);

  parser.parse(inputSrc);
}

#ifdef PluginUsesDOM
DOMDocument*
agfDOMParse(const string& xml_file,
	    const bool& xml_validation,
	    const string& encoding)
{
  LocalFileInputSource inputSrc(trans(xml_file));
  if (! encoding.empty())
    inputSrc.setEncoding(trans(encoding));

  XercesDOMParser* parser = DOMPARSER;
  agfDOMErrorHandler error_handler;
  if (xml_validation) {
    parser->setValidationScheme(XercesDOMParser::Val_Auto);
    parser->setDoValidation(true);
  }
  else {
    parser->setLoadExternalDTD(false);
    parser->setValidationScheme(XercesDOMParser::Val_Never);
    parser->setDoValidation(false);
  }
  parser->setDoNamespaces(false);
  parser->setIncludeIgnorableWhitespace(false);
  parser->setCreateCommentNodes(false);
  parser->setErrorHandler(&error_handler);

  try {
    parser->parse(inputSrc);
  }
  catch (const XMLException& e) {
    cerr << "agfXercesUtils.cc:agfDOMParse():" << endl
	 << "[fatal error] An error occurred during parsing" << endl
	 << "Message: " << trans(e.getMessage()) << endl;
    return NULL;
  }
  catch (const DOMException& e) {
    cerr << "agfXercesUtils.cc:agfDOMParse():" << endl
	 << "[fatal error] A DOM error occurred during parsing" << endl
	 << "DOMException code: " << e.code << endl;
    return NULL;
  }
  catch (...) {
    cerr << "agfXercesUtils.cc:agfDOMParse():" << endl
	 << "[fatal error] An unknown error occurred during parsing" << endl;
    return NULL;
  }

  if (error_handler.getSawErrors()) {
    cerr << "agfXercesUtils.cc:agfDOMParse():" << endl
	 << "[fatal error] An error occurred during parsing" << endl;
    return NULL;
  }

  return parser->getDocument();
}

DOMElement*
dom_elt(DOMNode* node)
{ return (DOMElement*) node; }

string
node_name(DOMNode* node)
{ return trans(node->getNodeName()); }

string
node_att(DOMNode* node, const string& att)
{ return trans(dom_elt(node)->getAttribute(trans(att))); }

string
node_val(DOMNode* node)
{
  node = node->getFirstChild();
  return (node==NULL) ? "" : trans(node->getNodeValue());
}

bool
is_text(DOMNode* node)
{ return (node->getNodeType() == DOMNode::TEXT_NODE); }

DOMNodeList*
elts_by_name(DOMNode* node, const string& name)
{ return ((DOMElement*)node)->getElementsByTagName(trans(name)); }

DOMNodeList*
elts_by_name(DOMDocument* doc, const string& name)
{ return doc->getElementsByTagName(trans(name)); }

DOMNode*
next_node(DOMNode* node)
{
  DOMNode* n = node->getNextSibling();
  if (n == NULL)
    return NULL;
  return is_text(n) ? n->getNextSibling() : n;
}

DOMNode*
first_node(DOMNode* node)
{
  DOMNode* n = node->getFirstChild();
  if (n == NULL)
    return NULL;
  return is_text(n) ? n->getNextSibling() : n;
}

void
att2map(DOMNode* node, map<string,string>& m)
{
  DOMNamedNodeMap* A = node->getAttributes();
  for (int i=0; i < A->getLength(); ++i) {
    DOMNode* a = A->item(i);
    m[trans(a->getNodeName())] = trans(a->getNodeValue());
  }
}

DOMNode*
prev_node(DOMNode* node)
{
  DOMNode* n = node->getPreviousSibling();
  if (n == NULL)
    return NULL;
  return is_text(n) ? n->getPreviousSibling() : n;
}

DOMNode*
last_node(DOMNode* node)
{
  DOMNode* n = node->getLastChild();
  if (n == NULL)
    return NULL;
  return is_text(n) ? n->getPreviousSibling() : n;
}
#endif
