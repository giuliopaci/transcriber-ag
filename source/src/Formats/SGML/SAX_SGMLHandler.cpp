/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/* $Id$ */
/**
 *  @file SAX_SGMLHandler.cc
 *  Thomson R&D XML result file to AG converter implementation
 */
#include "SAX_SGMLHandler.h"

#include <sstream>


#include <iostream>
#include <ag/AGAPI.h>
#include <ag/AGException.h>
#include <ag/agfio.h>
#include <ag/Utilities.h>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include "agfXercesUtils.h"
#include "Common/iso639.h"
#include "Common/FileInfo.h"
#include "Common/util/StringOps.h"
#include "Common/util/FormatToUTF8.h"

using namespace std;

#define atof myatof

#if defined(TRACE)
#undef TRACE
#endif

#define TRACE(a)
//#define TRACE(a) trace((a), __FILE__, __LINE__ )

static void trace(string a, string f, int l)
{
	Log::err() << "At " <<  f << "," << l << ": " << a << endl << flush;
}

SAX_SGMLHandler::SAX_SGMLHandler(tag::DataModel& out, SGMLobjects* parseModel, const string& encoding)
: m_dataModel(out), localDTD("")
{

	formatter = new XMLFormatter(encoding.c_str(),
#if _XERCES_VERSION >= 20300
				"1.0",
#endif
				this,
				XMLFormatter::NoEscapes,
				XMLFormatter::UnRep_CharRef);

	current_SGMLspeaker = NULL ;
	sgmlModel = parseModel ;

	// first tag will be handled by AGSet handler
	StartStack.push(&SAX_SGMLHandler::SGMLSubStart);
	EndStack.push(&SAX_SGMLHandler::dummyEnd);
}

SAX_SGMLHandler::~SAX_SGMLHandler()
{ delete formatter; }

void SAX_SGMLHandler::set_encoding(const string& encoding)
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

void SAX_SGMLHandler::startElement
(const XMLCh* const name, AttributeList& attr)
{
	TRACE(string("{{{START-ELEM} ")+trans(name));
	// execute the handler at the top of the stack
	(this->*StartStack.top())(name, attr);
}

void SAX_SGMLHandler::endElement(const XMLCh* const name)
{
	TRACE(string("{END-ELEM}}}  ")+trans(name));
	// execute the handler at the top of the stack
	(this->*EndStack.top())(name);
}

void SAX_SGMLHandler::dummyStart(const XMLCh* const name, AttributeList& attr)
{
	// if start-of-element is reported,
	// do nothing, and push do-nothing handlers
	StartStack.push(&SAX_SGMLHandler::dummyStart);
	EndStack.push(&SAX_SGMLHandler::dummyEnd);
}

void SAX_SGMLHandler::dummyEnd(const XMLCh* const name)
{
	// if end-of-element is reported
	// do nothing, and pop both stacks
	EndBranch() ;
}

void SAX_SGMLHandler::ElementCDATAStart(const XMLCh* const name, AttributeList& attr) {
	//just clear for next PCDATA
	currentCDATAvalue.clear() ;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void SAX_SGMLHandler::SGMLSubStart(const XMLCh* const name, AttributeList& attr)
{
	const string& tag = trans(name);
	TRACE(string("IN SGMLSubStart ") + tag);

	if (tag == "SYSTEM") {
		string title, ref_fname, hyp_fname, creation_date, format, frag_corr, opt_del, weight_ali, weight_filename ;

		title = trans(attr.getValue("id")) ;
		ref_fname = trans(attr.getValue("ref_fname")) ;
		hyp_fname = trans(attr.getValue("hyp_fname")) ;
		creation_date = trans(attr.getValue("creation_date")) ;
		format = trans(attr.getValue("format")) ;
		frag_corr = trans(attr.getValue("frag_corr")) ;
		opt_del = trans(attr.getValue("opt_del")) ;
		weight_ali = trans(attr.getValue("weight_ali")) ;
		weight_filename = trans(attr.getValue("weight_filename")) ;

		if (sgmlModel)
			sgmlModel->setData(title, ref_fname, hyp_fname, creation_date, format, frag_corr, opt_del, weight_ali, weight_filename) ;
		else
			Log::err() << "SGML: no model handler... Error" << std::endl ;

		StartStack.push(&SAX_SGMLHandler::SystemStart) ;
		EndStack.push(&SAX_SGMLHandler::dummyEnd) ;
	}
	else {
		TRACE("UNKNOWN TAG: " + tag);
		StartStack.push(&SAX_SGMLHandler::SGMLSubStart);
		EndStack.push(&SAX_SGMLHandler::dummyEnd);
	}
	TRACE("OUT SGMLSubStart");
}

void SAX_SGMLHandler::SystemStart(const XMLCh* const name, AttributeList& attr)
{
	const string& tag = trans(name);
	TRACE(string("IN SystemStart ") + tag);

	if (tag == "LABEL") {
		string id, title, desc ;
		id = trans(attr.getValue("id")) ;
		title = trans(attr.getValue("title")) ;
		desc = trans(attr.getValue("desc")) ;

		if (sgmlModel)
			sgmlModel->addLabel(id, title, desc) ;
		else
			Log::err() << "no SGML parser model... Error" << std::endl ;
		StartStack.push(&SAX_SGMLHandler::LabelStart) ;
		EndStack.push(&SAX_SGMLHandler::LabelEnd) ;
	}
	else if (tag == "CATEGORY") {
		string id, title, desc ;
		id = trans(attr.getValue("id")) ;
		title = trans(attr.getValue("title")) ;
		desc = trans(attr.getValue("desc")) ;
		if (sgmlModel)
			sgmlModel->addCategory(id, title, desc) ;
		else
			Log::err() << "no SGML parser model... Error" << std::endl ;
		StartStack.push(&SAX_SGMLHandler::CategoryStart) ;
		EndStack.push(&SAX_SGMLHandler::CategoryEnd) ;
	}
	else if (tag== "SPEAKER") {
		string id = trans(attr.getValue("id")) ;
		if (sgmlModel)
			current_SGMLspeaker = sgmlModel->addSpeaker(id) ;
		StartStack.push(&SAX_SGMLHandler::SpeakerStart) ;
		EndStack.push(&SAX_SGMLHandler::SpeakerEnd) ;
	}
	else {
		TRACE("UNKNOWN TAG: " + tag);
		StartStack.push(&SAX_SGMLHandler::SystemStart);
		EndStack.push(&SAX_SGMLHandler::dummyEnd);
	}
}

void SAX_SGMLHandler::CategoryStart(const XMLCh* const name, AttributeList& attr)
{
	const string& tag = trans(name);
	TRACE(string("IN CategoryStart ") + tag);
}

void SAX_SGMLHandler::CategoryEnd(const XMLCh* const name)
{
	EndBranch() ;
}


void SAX_SGMLHandler::LabelStart(const XMLCh* const name, AttributeList& attr)
{
	const string& tag = trans(name);
	TRACE(string("IN LabelStart ")+ tag);

}
void SAX_SGMLHandler::LabelEnd(const XMLCh* const name)
{
	EndBranch() ;
}

void SAX_SGMLHandler::SpeakerStart(const XMLCh* const name, AttributeList& attr)
{
	const string& tag = trans(name);
	TRACE(string("IN SpeakerStart ") + tag);

	if (tag == "PATH") {

		string id, word_cnt, labels, file, chanel, sequence, r_t1, r_t2, word_aux ;
		id = trans(attr.getValue("id")) ;
		word_cnt = trans(attr.getValue("word_cnt")) ;
		labels = trans(attr.getValue("labels")) ;
		file = trans(attr.getValue("file")) ;
		chanel = trans(attr.getValue("chanel")) ;
		sequence = trans(attr.getValue("sequence")) ;
		r_t1 = trans(attr.getValue("R_T1")) ;
		r_t2 = trans(attr.getValue("R_T2")) ;
		word_aux = trans(attr.getValue("word_aux")) ;

		if (current_SGMLspeaker) {
			current_SGMLpath = current_SGMLspeaker->addPath(id, word_cnt, labels, file, chanel, sequence, r_t1, r_t2, word_aux) ;
			sgmlModel->addPath(current_SGMLpath) ;
		}

		StartStack.push(&SAX_SGMLHandler::PathStart) ;
		EndStack.push(&SAX_SGMLHandler::PathEnd) ;
	}
	else {
		TRACE("UNKNOWN TAG: " + tag);
		StartStack.push(&SAX_SGMLHandler::SpeakerStart) ;
		EndStack.push(&SAX_SGMLHandler::dummyEnd) ;
	}
}
void SAX_SGMLHandler::SpeakerEnd(const XMLCh* const name)
{
	current_SGMLspeaker = NULL ;
	EndBranch() ;
}

void SAX_SGMLHandler::PathStart(const XMLCh* const name, AttributeList& attr)
{
	const string& tag = trans(name);
	TRACE(string("IN PathStart ") + tag) ;

	currentCDATAvalue.clear() ;
}

void SAX_SGMLHandler::PathEnd(const XMLCh* const name)
{
	if (current_SGMLpath)
		current_SGMLpath->set_entries(currentCDATAvalue) ;

	current_SGMLpath = NULL ;
	EndBranch() ;
}

// invoked when PCDATA encountered
void SAX_SGMLHandler::characters(const XMLCh* const chars, const unsigned int length)
{
	currentCDATAvalue.clear() ;
	string s;
	set_string(s, chars);
	if ( !s.empty() ) {
		if ( ! currentCDATAvalue.empty() )
			currentCDATAvalue += " ";
		currentCDATAvalue += s ;
	}
}

//******************************************************************************
//****************************    Commun methods    ****************************
//******************************************************************************

void SAX_SGMLHandler::EndBranch()
{
	StartStack.pop();
	EndStack.pop();
	//currentCDATAvalue.clear() ;
}


void SAX_SGMLHandler::warning(const SAXParseException& e)
{
	Log::err() << "WARNING: " << trans(e.getMessage()) << endl;
	Log::err() << " at line " << e.getLineNumber ()
	<< " col " << e.getColumnNumber () << endl;
}

void SAX_SGMLHandler::error(const SAXParseException& e)
{
	Log::err() << "WARNING: " << trans(e.getMessage()) << endl;
	Log::err() << " at line " << e.getLineNumber ()
	<< " col " << e.getColumnNumber () << endl;
}

void SAX_SGMLHandler::fatalError(const SAXParseException& e)
{
	// DIRTY HACK
	// Stop on FatalError is disabled, we have to check wether the current
	// error is "correct" due to SGML format parsed with XML handler, or
	// if we need to throw an exception

	string allowed_error = "A '<' character cannot be used in attribute 'labels'" ;
	string error = trans(e.getMessage()) ;
	bool allowed = ( error.find(allowed_error.c_str(), 0) != std::string::npos ) ;

	if (!allowed)
	{
		Log::err() << "SAX-SGML-PARSER ERROR: " << trans(e.getMessage()) << endl;
		Log::err() << " at line " << e.getLineNumber () << " col " << e.getColumnNumber () << endl;
		throw agfio::LoadError(trans(e.getMessage()));
	}
}

void SAX_SGMLHandler::writeChars(const XMLByte* const toWrite,
			const XMLSize_t count,
			XMLFormatter* const formatter)
{
	targetString.assign((char*) toWrite, count);
}

string& SAX_SGMLHandler::set_string(string& s, const XMLCh* const chars)
{
	targetString.erase();
	(*formatter) << chars;
	int err ;
	s = tag::FormatToUTF8::checkUTF8(s, "ISO-8859-1", false, err) ;
	s = StringOps(targetString).trim();
	return s;
}

InputSource* SAX_SGMLHandler::resolveEntity(const XMLCh* const publicId,
			const XMLCh* const systemId)
{
	if (! localDTD.empty()) {
		LocalFileInputSource* is = new LocalFileInputSource(trans(localDTD));
		localDTD = ""; // dirty hack to prevent from loading the dtd for any entity
		return is;
	}

	return NULL;
}

