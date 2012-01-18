/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// SAX_TransAGHandler.cc: AIF element handler implementation
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

#include "SAX_TransAGHandler.h"
#include "agfXercesUtils.h"

#include "FileInfo.h"

using namespace std;

#define atof myatof

SAX_TransAGHandler::SAX_TransAGHandler(const string& encoding,
			       const string& dtd)
  : localDTD(dtd), prevValue("")
{
  formatter = new XMLFormatter(encoding.c_str(),
#if _XERCES_VERSION >= 20300
                               "1.0",
#endif
                               this,
                               XMLFormatter::NoEscapes,
                               XMLFormatter::UnRep_CharRef);

  a_agId = "" ;
  a_version = "" ;

  // first tag will be handled by AGSet handler
  StartStack.push(&SAX_TransAGHandler::AGSetStart);
  EndStack.push(&SAX_TransAGHandler::AGSetEnd);
}

SAX_TransAGHandler::~SAX_TransAGHandler()
{ delete formatter; }

void SAX_TransAGHandler::set_encoding(const string& encoding)
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

void SAX_TransAGHandler::startElement
(const XMLCh* const name, AttributeList& attr)
{
  // execute the handler at the top of the stack
  (this->*StartStack.top())(name, attr);
}

void SAX_TransAGHandler::endElement(const XMLCh* const name)
{
  // execute the handler at the top of the stack
  (this->*EndStack.top())(name);
}

void SAX_TransAGHandler::dummyStart
(const XMLCh* const name, AttributeList& attr)
{
  // if start-of-element is reported,
  // do nothing, and push do-nothing handlers
  StartStack.push(&SAX_TransAGHandler::dummyStart);
  EndStack.push(&SAX_TransAGHandler::dummyEnd);
}

void SAX_TransAGHandler::dummyEnd(const XMLCh* const name)
{
  // if end-of-element is reported
  // do nothing, and pop both stacks
  StartStack.pop();
  EndStack.pop();
}

void SAX_TransAGHandler::structuredMetaStart
(const XMLCh* const name, AttributeList& attr)
{
  string s;
  // if start-of-element is reported,
  // store structured element as XML string in feature value
  // and push structuredMetaEnd handler
  //  storeMetaValueStart(name, attr);
  if ( ! prevValue.empty() ) prevValue += " ";
  prevValue += "<";
  prevValue += set_string(s, name);
  for ( int i = 0; i < attr.getLength(); ++i ) {
    prevValue += " ";
    prevValue += set_string(s, attr.getName(i));
    prevValue += "=\"";
    prevValue += set_string(s, attr.getValue(i));
    prevValue += "\"";
  }

  prevValue += ">";
  prevPos = prevValue.length();

  StartStack.push(&SAX_TransAGHandler::structuredMetaStart);
  EndStack.push(&SAX_TransAGHandler::structuredMetaEnd);
}


void SAX_TransAGHandler::structuredMetaEnd(const XMLCh* const name)
{
  string s;
  // if end-of-element is reported
  // do nothing, and pop both stacks
  if ( prevPos == prevValue.length() ) { // no CDATA
    prevValue.erase(prevPos-1);
    prevValue += "/>";
  } else {
    prevValue += "</";
    prevValue += set_string(s, name);
    prevValue += ">";
  }
  StartStack.pop();
  EndStack.pop();
}

void SAX_TransAGHandler::AGSetStart
(const XMLCh* const name, AttributeList& attr)
{
	string id = trans(attr.getValue("id")) ;
	string version = trans(attr.getValue("version")) ;

	try  {
		prevId = CreateAGSet(id);
		agIds.clear();    // erase ids of previous load
		agSetIds.push_back(prevId);

		a_version = version ;
		a_agId =  id ;

		StartStack.push(&SAX_TransAGHandler::AGSetSubStart);
		EndStack.push(&SAX_TransAGHandler::dummyEnd);
	}
	catch ( AGException e ) {
		throw agfio::LoadError(e.error());
	}
	catch ( ... ) {
		string msg = "AGSetStart : id '" + id + "'already exists ";
		throw agfio::LoadError(msg);
	}

}

void SAX_TransAGHandler::AGSetEnd
(const XMLCh* const name)
{

}

// invoked at the start of a subelement of AGSet
void SAX_TransAGHandler::AGSetSubStart
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);

  if (!a_agId.empty() && !a_version.empty())
  	SetFeature(a_agId, "version", a_version) ;

  // if Metadata element is found
  if (tag == "Metadata") {
    StartStack.push(&SAX_TransAGHandler::MetadataSubStart);
    EndStack.push(&SAX_TransAGHandler::dummyEnd);
  }
  // if Timeline element is found
  else if (tag == "Timeline") {
    prevId = CreateTimeline(trans(attr.getValue("id")));
    StartStack.push(&SAX_TransAGHandler::TimelineSubStart);
    EndStack.push(&SAX_TransAGHandler::dummyEnd);
  }
  // if AG element is found
  else if (tag == "AG") {
    prevId = CreateAG(trans(attr.getValue("id")),
                      trans(attr.getValue("timeline")));

    agIds.push_back(prevId);
    StartStack.push(&SAX_TransAGHandler::AGSubStart);
    EndStack.push(&SAX_TransAGHandler::dummyEnd);
  }
}

// invoked at the start of a subelement of Metadata
void
SAX_TransAGHandler::MetadataSubStart
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);

  if (tag == "MetadataElement" || tag == "OtherMetadata")
    set_string(prevFeature, attr.getValue("name"));
  else
    set_string(prevFeature, name);

  prevValue.erase();
  StartStack.push(&SAX_TransAGHandler::structuredMetaStart);
  EndStack.push(&SAX_TransAGHandler::MetadataSubEnd);
}

// invoked at the end of a subelement of Metadata
void SAX_TransAGHandler::MetadataSubEnd(const XMLCh* const name)
{
  SetFeature(prevId, prevFeature, prevValue);
  prevValue.erase();
  StartStack.pop();
  EndStack.pop();
}

// invoked at the start of a subelement of Timeline
void SAX_TransAGHandler::TimelineSubStart
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);

  if (tag == "Metadata") {
    StartStack.push(&SAX_TransAGHandler::MetadataSubStart);
    EndStack.push(&SAX_TransAGHandler::dummyEnd);
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
    StartStack.push(&SAX_TransAGHandler::SignalSubStart);
    EndStack.push(&SAX_TransAGHandler::dummyEnd);
  }
}

// invoked at the start of a subelement of Signal
void SAX_TransAGHandler::SignalSubStart
(const XMLCh* const name, AttributeList& attr)
{
  StartStack.push(&SAX_TransAGHandler::MetadataSubStart);
  EndStack.push(&SAX_TransAGHandler::dummyEnd);
}

// invoked at the start of a subelement of AG
void SAX_TransAGHandler::AGSubStart
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);

  if (tag == "Metadata") {
    StartStack.push(&SAX_TransAGHandler::MetadataSubStart);
    EndStack.push(&SAX_TransAGHandler::dummyEnd);
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

    StartStack.push(&SAX_TransAGHandler::dummyStart);
    EndStack.push(&SAX_TransAGHandler::dummyEnd);
  }
  else if (tag == "Annotation") {
    prevId =
      CreateAnnotation(trans(attr.getValue("id")),
                       trans(attr.getValue("start")),
                       trans(attr.getValue("end")),
                       trans(attr.getValue("type")));
    StartStack.push(&SAX_TransAGHandler::AnnotationSubStart);
    EndStack.push(&SAX_TransAGHandler::dummyEnd);
  }
}

// invoked at the start of a subelement of Annotation
void SAX_TransAGHandler::AnnotationSubStart
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);

  if (tag == "Feature") {
    set_string(prevFeature, attr.getValue("name"));
    prevValue.erase();
    StartStack.push(&SAX_TransAGHandler::structuredMetaStart);
    EndStack.push(&SAX_TransAGHandler::FeatureEnd);
  }
}

// invoked at the end of Feature
void SAX_TransAGHandler::FeatureEnd(const XMLCh* const name)
{
  SetFeature(prevId, prevFeature, prevValue);
  prevValue.erase();
  StartStack.pop();
  EndStack.pop();
}

void SAX_TransAGHandler::characters
(const XMLCh* const chars, const unsigned int length)
{
  string s;
  set_string(s, chars);
  prevValue += s;
}

void SAX_TransAGHandler::warning(const SAXParseException& e)
{
  cerr << "WARNING: " << trans(e.getMessage()) << endl;
   cerr << " at line " << e.getLineNumber ()
       << " col " << e.getColumnNumber () << endl;
}

void SAX_TransAGHandler::error(const SAXParseException& e)
{
  cerr << "WARNING: " << trans(e.getMessage()) << endl;
   cerr << " at line " << e.getLineNumber ()
       << " col " << e.getColumnNumber () << endl;
}

void SAX_TransAGHandler::fatalError(const SAXParseException& e)
{
  cerr << "ERROR: " << trans(e.getMessage()) << endl;
   cerr << " at line " << e.getLineNumber ()
       << " col " << e.getColumnNumber () << endl;
  throw agfio::LoadError(trans(e.getMessage()));
}

void SAX_TransAGHandler::writeChars(const XMLByte* const toWrite,
                                 const unsigned int count,
                                 XMLFormatter* const formatter)
{
  targetString->assign((char*) toWrite, count);
}

string& SAX_TransAGHandler::set_string(string& s, const XMLCh* const chars)
{
	targetString = &s;
  targetString->erase();
  (*formatter) << chars;
  return s;
}

InputSource*
SAX_TransAGHandler::resolveEntity(const XMLCh* const publicId,
                  const XMLCh* const systemId)
{
    string sysid = trans(systemId);
//    cerr << " IN SAX_TransAGHandler::resolveEntity sysid = " << sysid << " publicId=" << trans(publicId) << endl;

    // no dtd found, use argument
    if (sysid.empty())
    	sysid = localDTD ;
    else
    {
    	// add extension if needed
    	if ( FileInfo(sysid).tail() == "" )
			sysid += ".dtd";
    	if (! FileInfo(sysid).exists() && ! localDTD.empty())
    	{
            string tmp = FileInfo(localDTD).dirname();
            sysid = FileInfo(tmp).join(sysid);
        }
    }

    if ( FileInfo(sysid).exists() )
    {
//        cerr << " IN SAX_TransAGHandler::resolveEntity DTD -> " << sysid <<  " exists" << endl;
    	LocalFileInputSource* is = new LocalFileInputSource(trans(sysid));
        //localDTD = ""; // dirty hack to prevent from loading the dtd for any entity
//        cerr << "OK:!" << endl;
        return is;
    }
    else
    	cerr << " IN SAX_TransAGHandler::resolveEntity DTD -> " << sysid <<  " doesn't exist" << endl;

  return NULL;
}
