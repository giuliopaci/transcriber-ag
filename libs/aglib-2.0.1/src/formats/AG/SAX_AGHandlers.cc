// SAX_AGHandlers.cc: AIF element handler implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <iostream>
#include <ag/AGAPI.h>
#include <ag/AGException.h>
#include <ag/agfio.h>
#include <ag/Utilities.h>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include "SAX_AGHandlers.h"
#include "agfXercesUtils.h"

/* (( BT Patch -- */
#define atof myatof
/* -- BT Patch )) */
using namespace std;


SAX_AGHandlers::SAX_AGHandlers(const string& encoding,
			       const string& dtd)
  : localDTD(dtd), prevValue(""), 
/* (( BT Patch -- */
  inMetadata(false)
/* -- BT Patch )) */
{
  formatter = new XMLFormatter(encoding.c_str(),
#if _XERCES_VERSION >= 20300
                               "1.0",
#endif
                               this,
                               XMLFormatter::NoEscapes,
                               XMLFormatter::UnRep_CharRef);

  // first tag will be handled by AGSet handler
  StartStack.push(&SAX_AGHandlers::AGSetStart);
  EndStack.push(&SAX_AGHandlers::AGSetEnd);
}

SAX_AGHandlers::~SAX_AGHandlers()
{ delete formatter; }

void SAX_AGHandlers::set_encoding(const string& encoding)
{
  delete formatter;
  formatter = new XMLFormatter(encoding.c_str(),
#if _XERCES_VERSION >= 20300
                               "1.0",
#endif
                               this,
                               XMLFormatter::NoEscapes,
                               XMLFormatter::UnRep_CharRef);
}

void SAX_AGHandlers::startElement
(const XMLCh* const name, AttributeList& attr)
{
  // execute the handler at the top of the stack
  (this->*StartStack.top())(name, attr);
}

void SAX_AGHandlers::endElement(const XMLCh* const name)
{
  // execute the handler at the top of the stack
  (this->*EndStack.top())(name);
}

void SAX_AGHandlers::dummyStart
(const XMLCh* const name, AttributeList& attr)
{
  // if start-of-element is reported,
/* (( BT Patch -- */
  // if in metadata -> store element in prevVal
	if ( inMetadata ) {
  string s;
  prevValue += "<";
  set_string(s, name);
  prevValue += s;
  prevValue += " ";
	for ( int i =0; i < attr.getLength (); ++i ) {
  	  set_string(s, attr.getName(i));
	  prevValue += s;
	  prevValue += "=\"";
	  set_string(s, attr.getValue(i));
	  prevValue += s;
	  prevValue += "=\" ";
	}
  prevValue += ">";
	}
  // push do-nothing handlers
/* -- BT Patch )) */
  
  StartStack.push(&SAX_AGHandlers::dummyStart);
  EndStack.push(&SAX_AGHandlers::dummyEnd);
}

void SAX_AGHandlers::dummyEnd(const XMLCh* const name)
{
  // if end-of-element is reported
  // do nothing, and pop both stacks
/* (( BT Patch -- */
	if ( inMetadata ) 
	{
  		string s;
		set_string(s, name);
  		prevValue += "</";
  		prevValue += s;
  		prevValue += ">";
	}
/* -- BT Patch )) */
  	StartStack.pop();
 	EndStack.pop();
}

void SAX_AGHandlers::AGSetStart
(const XMLCh* const name, AttributeList& attr)
{
  prevId = CreateAGSet(trans(attr.getValue("id")));
  agIds.clear();    // erase ids of previous load

  StartStack.push(&SAX_AGHandlers::AGSetSubStart);
  EndStack.push(&SAX_AGHandlers::dummyEnd);
}

void SAX_AGHandlers::AGSetEnd
(const XMLCh* const name)
{}

// invoked at the start of a subelement of AGSet
void SAX_AGHandlers::AGSetSubStart
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);

  // if Metadata element is found
  if (tag == "Metadata") {
    StartStack.push(&SAX_AGHandlers::MetadataSubStart);
    EndStack.push(&SAX_AGHandlers::dummyEnd);
  }
  // if Timeline element is found
  else if (tag == "Timeline") {
    prevId = CreateTimeline(trans(attr.getValue("id")));
    StartStack.push(&SAX_AGHandlers::TimelineSubStart);
    EndStack.push(&SAX_AGHandlers::dummyEnd);
  }
  // if AG element is found
  else if (tag == "AG") {
    prevId = CreateAG(trans(attr.getValue("id")),
                      trans(attr.getValue("timeline")));
    agIds.push_back(prevId);
    StartStack.push(&SAX_AGHandlers::AGSubStart);
    EndStack.push(&SAX_AGHandlers::dummyEnd);
  }
}

// invoked at the start of a subelement of Metadata
void
SAX_AGHandlers::MetadataSubStart
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);

  if (tag == "MetadataElement" || tag == "OtherMetadata")
    set_string(prevFeature, attr.getValue("name"));
  else
    set_string(prevFeature, name);

  prevValue.erase();
  /* (( BT Patch -- */
  inMetadata = true;
  /* -- BT Patch )) */
  StartStack.push(&SAX_AGHandlers::dummyStart);
  EndStack.push(&SAX_AGHandlers::MetadataSubEnd);
}

// invoked at the end of a subelement of Metadata
void SAX_AGHandlers::MetadataSubEnd(const XMLCh* const name)
{
  SetFeature(prevId, prevFeature, prevValue);
  /* (( BT Patch -- */
  inMetadata = false;
  /* -- BT Patch )) */
  prevValue.erase();
  StartStack.pop();
  EndStack.pop();
}

// invoked at the start of a subelement of Timeline
void SAX_AGHandlers::TimelineSubStart
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);

  if (tag == "Metadata") {
    StartStack.push(&SAX_AGHandlers::MetadataSubStart);
    EndStack.push(&SAX_AGHandlers::dummyEnd);
  }
  else if (tag == "Signal") {
    string href, mimeClass, mimeType, unit, track;
    set_string(href, attr.getValue("xlink:href"));
    set_string(mimeClass, attr.getValue("mimeClass"));
    set_string(mimeType, attr.getValue("mimeType"));
    set_string(unit, attr.getValue("unit"));
    set_string(track, attr.getValue("track"));

    prevId =
      CreateSignal(trans(attr.getValue("id")),
                   href, mimeClass, mimeType,
                   trans(attr.getValue("encoding")),
                   unit, track);
    StartStack.push(&SAX_AGHandlers::SignalSubStart);
    EndStack.push(&SAX_AGHandlers::dummyEnd);
  }
}

// invoked at the start of a subelement of Signal
void SAX_AGHandlers::SignalSubStart
(const XMLCh* const name, AttributeList& attr)
{
  StartStack.push(&SAX_AGHandlers::MetadataSubStart);
  EndStack.push(&SAX_AGHandlers::dummyEnd);
}

// invoked at the start of a subelement of AG
void SAX_AGHandlers::AGSubStart
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);

  if (tag == "Metadata") {
    StartStack.push(&SAX_AGHandlers::MetadataSubStart);
    EndStack.push(&SAX_AGHandlers::dummyEnd);
  }
  else if (tag == "Anchor") {
    string unit = trans(attr.getValue("unit"));
    string off_str = trans(attr.getValue("offset"));
    set<SignalId> sigset;
    Utilities::string2set(trans(attr.getValue("signals")), sigset);
    if (off_str.empty())
      CreateAnchor(trans(attr.getValue("id")), sigset);
    else
      CreateAnchor(trans(attr.getValue("id")),
		   atof(off_str.c_str()),
		   unit,
		   sigset);
    StartStack.push(&SAX_AGHandlers::dummyStart);
    EndStack.push(&SAX_AGHandlers::dummyEnd);
  }
  else if (tag == "Annotation") {
    prevId =
      CreateAnnotation(trans(attr.getValue("id")),
                       trans(attr.getValue("start")),
                       trans(attr.getValue("end")),
                       trans(attr.getValue("type")));
    StartStack.push(&SAX_AGHandlers::AnnotationSubStart);
    EndStack.push(&SAX_AGHandlers::dummyEnd);
  }
}

// invoked at the start of a subelement of Annotation
void SAX_AGHandlers::AnnotationSubStart
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);

  if (tag == "Feature") {
    set_string(prevFeature, attr.getValue("name"));
    prevValue.erase();
    StartStack.push(&SAX_AGHandlers::dummyStart);
    EndStack.push(&SAX_AGHandlers::FeatureEnd);
  }
}

// invoked at the end of Feature
void SAX_AGHandlers::FeatureEnd(const XMLCh* const name)
{
  SetFeature(prevId, prevFeature, prevValue);
  prevValue.erase();
  StartStack.pop();
  EndStack.pop();
}

void SAX_AGHandlers::characters
(const XMLCh* const chars, const unsigned int length)
{
  string s;
  set_string(s, chars);
  prevValue += s;
}

void SAX_AGHandlers::warning(const SAXParseException& e)
{
  cerr << "WARNING: " << trans(e.getMessage()) << endl;
}

void SAX_AGHandlers::error(const SAXParseException& e)
{
  cerr << "WARNING: " << trans(e.getMessage()) << endl;
}

void SAX_AGHandlers::fatalError(const SAXParseException& e)
{
  throw agfio::LoadError(trans(e.getMessage()));
}

void SAX_AGHandlers::writeChars(const XMLByte* const toWrite,
                                 const unsigned int count,
                                 XMLFormatter* const formatter)
{
  targetString.assign((char*) toWrite, count);
}

void SAX_AGHandlers::set_string(string& s, const XMLCh* const chars)
{
  targetString.erase();
  (*formatter) << chars;
  s = targetString;
}

InputSource*
SAX_AGHandlers::resolveEntity(const XMLCh* const publicId,
			      const XMLCh* const systemId)
{
  if (! localDTD.empty()) {
    LocalFileInputSource* is = new LocalFileInputSource(trans(localDTD));
    localDTD = ""; // dirty hack to prevent from loading the dtd for any entity
    return is;
  }

  return NULL;
}
