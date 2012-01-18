/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  @file Topics.cpp
 *  @brief Topic class implementation
 *
 */

#include "Topics.h"

#include <iostream>
#include <fstream>
#include <sstream>

// xerces
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <string.h>
#include <errno.h>
#include <time.h>

// STL en fonction de XERCES
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif

#include "Topics.h"
#include "Common/FileInfo.h"
#include "Common/globals.h"
#include "Common/util/StringOps.h"

using namespace std;

namespace tag {


Glib::ustring Topics::getTopicLabel(Glib::ustring id, std::map<Glib::ustring,Topics*>& l)
{
	if (id.compare("")==0)
		return TAG_TOPIC_NULL_LABEL ;
	else {
		Topic* res = Topics::getTopicFromAll(id, l) ;
		if (res)
			return res->getLabel() ;
		else
			return TAG_TOPIC_UNKNOWN_LABEL ;
	}
}

Topic* Topics::getTopicFromAll(Glib::ustring id, const std::map<Glib::ustring,Topics*>& l)
{
	Topic* res = NULL ;
	std::map<Glib::ustring, Topics*>::const_iterator it ;
	for (it=l.begin(); (it!=l.end())&&(res==NULL); it++) {
		if (it->second)
			res = it->second->getTopic(id) ;
	}
	return res ;
}

Topic* Topics::getTopic(Glib::ustring id)
{
	Topic* res = NULL ;
	std::map<Glib::ustring, Topic*>::iterator it = m_topics.find(id) ;
	if ( it != m_topics.end() )
		res = it->second ;
	return  res ;
}

Glib::ustring Topics::getTopicLabel(Glib::ustring id)
{
	Topic* res = getTopic(id) ;
	if (res)
		return res->getLabel() ;
	else
		return "" ;
}

//void Topics::print()
//{
//	Log::err() << "----------------- Topics Printer--------------" << std::endl ;
//
//	Log::err() << "GROUP id= " << m_id
//		<< " - label= " << m_label << std::endl ;
//
//	std::map<Glib::ustring,Topic*>::iterator it ;
//	for (it=m_topics.begin(); it!= m_topics.end(); it++) {
//		if (it->second)
//			(it->second)->print() ;
//	}
//}



//******************************************************************************
//***************************** READ METHODS **********************************
//******************************************************************************


/*
 * read topic list from xml-formatted string
 */
void Topics::loadTopics(Glib::ustring in, std::map<Glib::ustring,Topics*>& l)
throw(const char *)
{
	try
	{
		Topics_XMLHandler handler(l);
		CommonXMLReader reader(&handler);
		reader.parseFile(in);
	}
	catch(const char *msg) {
		throw msg;
	}
}


Topics_XMLHandler::Topics_XMLHandler(std::map<Glib::ustring,Topics*>& l)
	: m_topicsGroups(l)
{
	m_detailsTag = NULL ;
	m_detailTag = NULL ;
	m_contextTag = NULL ;
	m_topicTag = NULL ;
	m_topicgroupTag = NULL ;
	m_current_group = NULL ;
	m_current_topic = NULL ;
	in_context = false ;
	in_detail_value= false ;
}

Topics_XMLHandler::~Topics_XMLHandler()
{
	if ( m_detailsTag != NULL )
		XMLString::release (&m_detailsTag);
	if ( m_detailTag != NULL )
		XMLString::release (&m_detailTag);
	if ( m_contextTag != NULL )
		XMLString::release (&m_contextTag);
	if ( m_topicTag != NULL )
		XMLString::release (&m_topicTag);
	if ( m_topicgroupTag != NULL )
		XMLString::release (&m_topicgroupTag);
}

/* start XML element */
void
Topics_XMLHandler::startElement (const XMLCh * const uri,
			  const XMLCh * const localname,
			  const XMLCh * const qname, const Attributes & attrs)
{
	map < string, string > attmap;
	map < string, string >::iterator it;
	getAttributes (attrs, attmap);

	if ( m_topicgroupTag == NULL ) {
		m_detailTag = XMLString::transcode("detail");
		m_contextTag = XMLString::transcode("context");
		m_topicTag = XMLString::transcode("topic");
		m_topicgroupTag = XMLString::transcode("topicgroup");
	}

	if (XMLString::compareIString(localname, m_topicgroupTag) == 0) {
		m_topicsGroups[attmap["id"]] = new Topics(attmap["id"], attmap["label"]) ;
		m_current_group = m_topicsGroups[attmap["id"]] ;
	}
	else if (XMLString::compareIString(localname, m_topicTag) == 0) {
 		if (m_current_group) {
			m_current_topic = m_current_group->addTopic(attmap["id"], attmap["label"]) ;
 		}
	}
	else if (XMLString::compareIString(localname, m_contextTag) == 0) {
		if (m_current_topic) {
			in_context = true ;
			current_context = "" ;
		}
	}
	else if (XMLString::compareIString(localname, m_detailTag) == 0) {
		if (m_current_topic) {
			m_current_detail = m_current_topic->addDetail(attmap["type"], attmap["label"]) ;
			in_detail_value = true ;
			current_detail_value = "" ;
		}
	}
}

void
Topics_XMLHandler::endElement (const XMLCh * const uri,
			const XMLCh * const localname,
			const XMLCh * const qname)
{
	if (m_current_topic && in_context) {
		in_context = false ;
		if (!current_context.empty()) {
			m_current_topic->setContext(current_context) ;
		}
	}
	else if (m_current_detail && in_detail_value) {
		in_detail_value = false ;
		if (!current_detail_value.empty()) {
			m_current_detail->setValue(current_detail_value) ;
		}
	}
}

void
Topics_XMLHandler::characters (const XMLCh * const chars, const unsigned int length)
{
	if (length != 0 && in_context )
	  	current_context += getString(chars);
	else if (length != 0 && in_detail_value )
		current_detail_value += getString(chars);
}



//******************************************************************************
//***************************** WRITE METHODS **********************************
//******************************************************************************


/**
 * print out Topic details as xml-formatted string
 * <!> WARNING: it doesn't generate the format of the topics xml file,
 * 				but the format needed by TAG file to indicate
 * 				which topics has been used in current transcription
 * @param out destination ostream
 * @param delim print-out delimiter (eg. "\n")
 * @return destination ostream
 */
std::ostream& TopicDetails::toXML(std::ostream& out, const char* delim) const
{
	out << "<detail" << " " << "type" << "=\"" << m_type << "\"";
	out << " " << "label" << "=\"" << m_label << "\"";
	out << ">" << delim;
	out << m_value << delim ;
	out << "</detail>" << delim ;
	return out;
}

/**
 * print out Topic as xml-formatted string
 * <!> WARNING: it doesn't generate the format of the topics xml file,
 * 				but the format needed by TAG file to indicate
 * 				which topics has been used in current transcription
 * @param out destination ostream
 * @param delim print-out delimiter (eg. "\n")
 * @return destination ostream
 */
std::ostream& Topic::toXML(std::ostream& out, const char* delim) const
{
	out << "<Topic" ;
	out << " " << "id" << "=\"" << m_id << "\"";
	out << " " << "desc" << "=\"" << m_label << "\"";
	out << "/>" << delim ;

	return out ;
}


/**
 * print out Topic as xml-formatted string
 * @param delim print-out delimiter (eg. "\n")
 * @return destination ostream
 */
std::ostream& Topics::toXML(std::ostream& out, const char* delim) const
{
	//TODO if NECESSARY
}

/**
 * print out Topic as xml-formatted string
 * <!> WARNING: it doesn't generate the format of the topics xml file,
 * 				but the format needed by TAG file to indicate
 * 				which topics has been used in current transcription
 * @param delim print-out delimiter (eg. "\n")
 * @return destination ostream
 */
std::ostream& Topics::toXML(std::ostream& out, const std::map<Glib::ustring,Topics*>& topics, const std::set<Glib::ustring>& to_store, const char* delim)
{
	out << "<Topics>" << delim;
	std::set<Glib::ustring>::const_iterator itt ;
	for (itt=to_store.begin(); itt!=to_store.end(); itt++) {
		Topic* t = Topics::getTopicFromAll(*itt, topics); ;
		if (t)
			t->toXML(out, delim) ;
	}
	out << "</Topics>" << delim;

}



} /* namespace tag */
