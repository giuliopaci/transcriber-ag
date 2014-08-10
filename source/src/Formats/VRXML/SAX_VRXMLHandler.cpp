/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */
/**
*  @file SAX_VRXMLHandler.cc
*  VecsysResearch XML result file to AG converter implementation
*/

#include <sstream>


#include <iostream>
#include <ag/AGAPI.h>
#include <ag/AGException.h>
#include <ag/agfio.h>
#include <ag/Utilities.h>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include "SAX_VRXMLHandler.h"
#include "agfXercesUtils.h"
#include "Common/iso639.h"
#include "Common/FileInfo.h"
#include "Common/util/StringOps.h"
#include "Common/util/FormatTime.h"

using namespace std;
using namespace tag;

#define atof myatof

#ifndef TRACE
#define TRACE cout
#endif


static string no_blank_after = "'-";
static string no_blank_before = "-,.";

//#define TRACE(a)

SAX_VRXMLHandler::SAX_VRXMLHandler(tag::DataModel& out, const string& encoding)
  : m_dataModel(out), m_threshold(99.0), m_keepSegmentation(true), localDTD("")
{
  formatter = new XMLFormatter(encoding.c_str(),
#if _XERCES_VERSION >= 20300
                               "1.0",
#endif
                               this,
                               XMLFormatter::NoEscapes,
                               XMLFormatter::UnRep_CharRef);

  m_minSectSize = out.conventions().minSegmentSize("section", 5.);
  m_minTurnSize = out.conventions().minSegmentSize("turn", 2.);
  m_minSegSize = out.conventions().minSegmentSize("segment", 2.);
  m_with_sect = false;
  m_prev_is_special = false;
  m_CTSMode = false;
  m_firstSpeaker =true;

  // first tag will be handled by AGSet handler
  StartStack.push(&SAX_VRXMLHandler::AudioDocStart);
  EndStack.push(&SAX_VRXMLHandler::dummyEnd);
}

SAX_VRXMLHandler::~SAX_VRXMLHandler()
{ delete formatter; }

void SAX_VRXMLHandler::set_encoding(const string& encoding)
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

void SAX_VRXMLHandler::startElement
(const XMLCh* const name, AttributeList& attr)
{
//	cerr << "<<<<<<           " << trans(name) << endl;
  // execute the handler at the top of the stack
  (this->*StartStack.top())(name, attr);
}

void SAX_VRXMLHandler::endElement(const XMLCh* const name)
{
//		cerr<< ">>>>>>>>>          " << trans(name) << endl;
  // execute the handler at the top of the stack
  (this->*EndStack.top())(name);
}

void SAX_VRXMLHandler::dummyStart
(const XMLCh* const name, AttributeList& attr)
{
  // if start-of-element is reported,
  // do nothing, and push do-nothing handlers
  StartStack.push(&SAX_VRXMLHandler::dummyStart);
  EndStack.push(&SAX_VRXMLHandler::dummyEnd);
}

void SAX_VRXMLHandler::dummyEnd(const XMLCh* const name)
{
  // if end-of-element is reported
  // do nothing, and pop both stacks
  StartStack.pop();
  EndStack.pop();
}



/*
* start transcription file
*/
void SAX_VRXMLHandler::AudioDocStart
(const XMLCh* const name, AttributeList& attr)
{

	m_audio = trans(attr.getValue("name"));
	m_path_hint = FileInfo(trans(attr.getValue("path"))).dirname();

	StartStack.push(&SAX_VRXMLHandler::TransSubStart);
	EndStack.push(&SAX_VRXMLHandler::AudioDocEnd);
}

void SAX_VRXMLHandler::AudioDocEnd (const XMLCh* const name)
{
	if ( m_CTSMode ) {
		// CTS Mode -> suppress section info
		const list<AnnotationId>& lsect =  GetAnnotationSeqByOffset(m_dataModel.getAG("transcription_graph"),0.0, 0.0, "section");
		list<AnnotationId>::const_iterator it;
		for ( it=lsect.begin(); it != lsect.end(); ++it ) {
			DeleteAnnotation(*it);
		}
		m_dataModel.setAGSetProperty("speech_type", "CTS");
	}
	else
		m_dataModel.setAGSetProperty("speech_type", "BN");
}

// invoked at the start of a subelement of Trans :
//  -> topics list
//  -> speakers list
//  -> episode start
void SAX_VRXMLHandler::TransSubStart
(const XMLCh* const name, AttributeList& attr)
{
  const string& tag = trans(name);


  // if ProcList  element is found
  if (tag == "ProcList") {
    StartStack.push(&SAX_VRXMLHandler::ProcStart);
    EndStack.push(&SAX_VRXMLHandler::dummyEnd);
  }

  // if ChannelList  element is found
  else if (tag == "ChannelList") {
    StartStack.push(&SAX_VRXMLHandler::ChannelStart);
    EndStack.push(&SAX_VRXMLHandler::dummyEnd);
  }

  // if Speakers element is found
  else if (tag == "SpeakerList") {
    StartStack.push(&SAX_VRXMLHandler::SpeakerStart);
    EndStack.push(&SAX_VRXMLHandler::dummyEnd);
  }

  // if SegmentList element is found
  else if (tag == "SegmentList") {
    StartStack.push(&SAX_VRXMLHandler::SegmentStart);
    EndStack.push(&SAX_VRXMLHandler::dummyEnd);
  }
  else {
	cerr << "UNKNOWN TAG " << tag << endl;
	   StartStack.push(&SAX_VRXMLHandler::TransSubStart);
	EndStack.push(&SAX_VRXMLHandler::dummyEnd);
  }

}

// invoked at the start of a Proc definition
void
SAX_VRXMLHandler::ProcStart
(const XMLCh* const name, AttributeList& attr)
{
	/* <Proc name="diva_trans" version="1.0"/>  */
	ostringstream version_id ;
	m_scribe = trans(attr.getValue("name"));
	version_id << m_scribe  << "-" << trans(attr.getValue("version"));
	m_versionInfo.push_back(version_id.str());

	if ( ! m_CTSMode )
		m_CTSMode = (m_scribe.compare(0, 5, "vrcts") == 0);
	if ( m_CTSMode ) m_minTurnSize = m_minSegSize;

	StartStack.push(&SAX_VRXMLHandler::dummyStart);
    EndStack.push(&SAX_VRXMLHandler::dummyEnd);
}

// invoked at the start of a Channel definition
void SAX_VRXMLHandler::ChannelStart(const XMLCh* const name,
		AttributeList& attr) {
	/* <Channel num="1" sigdur="223.99" spdur="201.26" nw="382"/> */
	int notrack, idtrack;
	float duration;

	istringstream is(trans(attr.getValue("num")));
	is >> idtrack;
	m_notrack[idtrack] = m_dataModel.getNbTracks();
	notrack = m_notrack[idtrack];

	istringstream is2(trans(attr.getValue("sigdur")));
	is2 >> duration;

	m_dataModel.addSignal(m_audio, "audio", "", "", m_dataModel.getNbTracks()
			+ 1, idtrack);
	m_duration = duration;
	m_dataModel.setSignalProperty(notrack, "path_hint", m_path_hint);
	istringstream is3(trans(attr.getValue("spdur")));
	is3 >> duration;
	m_dataModel.setSignalProperty(notrack, "speech_duration", FormatTime(
			duration, true, true));
	m_dataModel.setSignalProperty(notrack, "nb_word",
			trans(attr.getValue("nw")));

	m_dataModel.initAnnotationGraphs("", m_lang, m_scribe);
	m_dataModel.setSignalDuration(m_duration);
	m_firstSeg[notrack] = true;
	m_with_sect = m_dataModel.hasElementsWithType("section");
	if ( m_with_sect && m_CTSMode ) {
		m_with_sect = false;
	}
	list<string>::iterator it;

	for (it = m_versionInfo.begin(); it != m_versionInfo.end(); ++it)
		m_dataModel.updateVersionInfo(*it);

	m_prevTime[notrack] = 0.0;
	m_speaker[notrack] = "";
	m_text[notrack] = "";
	m_confidence[notrack] = "";


	StartStack.push(&SAX_VRXMLHandler::dummyStart);
	EndStack.push(&SAX_VRXMLHandler::dummyEnd);
}

// invoked at the start of a speaker definition
void
SAX_VRXMLHandler::SpeakerStart
(const XMLCh* const name, AttributeList& attr)
{
	/* <Speaker ch="1" dur="122.26" gender="2" spkid="1" spkname="" lang="ara" lconf="1.00" nw="249"/> */

	int gender;

	istringstream is(trans(attr.getValue("gender")));
	is >> gender;

	switch ( gender ) {
	case 1: gender = Speaker::MALE_GENDER; break;
	case 2: gender = Speaker::FEMALE_GENDER; break;
	default: gender = Speaker::UNDEF_GENDER; break;
	}


        if ( m_lang.empty() ) {
                m_lang = trans(attr.getValue("lang"));
        		StringOps(m_lang).toLower();
        		m_lang=m_lang.substr(0,3);
                // some VR-specific language codes.
        		if ( m_lang == "rej" ) m_lang = "unk";
        		if ( m_lang == "us" || m_lang == "usa" ) m_lang="eng"; //lang="eng-usa";
        		if ( m_lang == "man" ) m_lang = "chi";
        		if ( m_lang == "cmn" ) m_lang = "chi";

        		m_lang = ISO639::get3LetterCode(m_lang.c_str(), true);
        }

    string spkid=  trans(attr.getValue("spkid"));
    const string& spkname=  trans(attr.getValue("spkname"));

    Speaker speaker ;

    if ( m_firstSpeaker && ! m_dataModel.getSpeakerDictionary().empty() ) {
    	speaker = m_dataModel.getSpeakerDictionary().begin()->second;  // use default speaker
    } else {
    	speaker = m_dataModel.getSpeakerDictionary().defaultSpeaker(m_lang); // new speaker
    }
    if ( !spkname.empty() ) {
    	speaker.setLastName(spkname);  // spkname should uniquely identify a given speaker and can be used as id
        speaker.setScope(spkname);
    }

	speaker.addLanguage(m_lang, true, true);
    speaker.setGender((Speaker::Gender)gender);

    ostringstream extra;
    extra << "ch=" << trans(attr.getValue("ch")) << ";spkid=" << spkid;
    speaker.setProperty("extra", extra.str());
    if ( m_firstSpeaker && ! m_dataModel.getSpeakerDictionary().empty() )
    	m_dataModel.getSpeakerDictionary().updateSpeaker(speaker);
    else
    	m_dataModel.getSpeakerDictionary().addSpeaker(speaker);
	m_spkids[spkid] = speaker.getId();
	m_firstSpeaker = false;

	StartStack.push(&SAX_VRXMLHandler::dummyStart);
    EndStack.push(&SAX_VRXMLHandler::dummyEnd);
}

// invoked at the start of a SpeechSegment
void
SAX_VRXMLHandler::SegmentStart
(const XMLCh* const name, AttributeList& attr)
{
	/* <SpeechSegment ch="1" sconf="1.00" stime="0.93" etime="7.80" spkid="1" lang="ara" lconf="1.00" trs="1"> */
	istringstream is(trans(attr.getValue("ch")));
	int idtrack;
	is >> idtrack;
	m_curtrack = m_notrack[idtrack];

	istringstream is1(trans(attr.getValue("stime")));
	istringstream is2(trans(attr.getValue("etime")));
	float stime, etime;
	is1 >> stime;
	is2 >> etime;
	string new_sect("");

	string spkid = trans(attr.getValue("spkid"));
	if ( m_spkids.find(spkid) == m_spkids.end() ) {
		Speaker speaker = m_dataModel.getSpeakerDictionary().defaultSpeaker(m_lang);
		 m_dataModel.getSpeakerDictionary().addSpeaker(speaker);
		m_spkids[spkid] = speaker.getId();
	}

	spkid = m_spkids[spkid];

	bool new_seg=false;
	bool do_new_seg = m_keepSegmentation;


	float diff = ( stime - m_prevTime[m_curtrack] );

	if ( m_firstSeg[m_curtrack]) {
		do_new_seg = false;
		m_dataModel.setTranscriptionLanguage(m_lang);
		m_unitId[m_curtrack] = m_dataModel.getByOffset(m_dataModel.mainstreamBaseType(), 0.0, m_curtrack);
		m_segmentId[m_curtrack] = m_dataModel.getParentElement(m_unitId[m_curtrack]);
		m_turnId[m_curtrack] =  m_dataModel.getParentElement(m_segmentId[m_curtrack]);
		m_dataModel.deleteElementProperty(m_turnId[m_curtrack], "speaker", false);
		m_speaker[m_curtrack] = Speaker::NO_SPEECH;

		if ( m_with_sect) {
			m_sectId[m_curtrack] =  m_dataModel.getParentElement(m_turnId[m_curtrack]);
			m_dataModel.setElementProperty(m_sectId[m_curtrack], "type", "nontrans", false);
			new_sect="report";
		}
	} else {

		if ( diff > m_minSegSize && m_firstSeg[m_curtrack] == false ) {
			m_lastWord[m_curtrack] = m_prevTime[m_curtrack];
			m_unitId[m_curtrack] = m_dataModel.insertMainstreamBaseElement(m_unitId[m_curtrack], m_lastWord[m_curtrack], false, false);
			// gap between segments -> add an empty segment
			m_segmentId[m_curtrack] = m_dataModel.insertParentElement(m_unitId[m_curtrack], false);
			new_seg=true;
			if ( diff > m_minTurnSize ) {
				m_speaker[m_curtrack] = Speaker::NO_SPEECH;
				// then create a "nospeaker" turn
				m_turnId[m_curtrack] = m_dataModel.insertParentElement(m_segmentId[m_curtrack], false);
				m_dataModel.setElementProperty(m_turnId[m_curtrack], "speaker", m_speaker[m_curtrack], false);
				if ( ! m_CTSMode && diff > m_minSectSize &&  m_with_sect ) {
					// set 1st section as "nontrans" and create a new section
					m_sectId[m_curtrack] = m_dataModel.insertParentElement(m_turnId[m_curtrack], false);
					m_dataModel.setElementProperty(m_sectId[m_curtrack], "type", "nontrans", false);
					new_sect="report";
				}
			}
		}
	}

	m_lastWord[m_curtrack] = stime;
	m_unitId[m_curtrack] = m_dataModel.insertMainstreamBaseElement(m_unitId[m_curtrack], m_lastWord[m_curtrack], false, false);
//	cerr << "line=" << __LINE__ << " === insertMainstreamBaseElement " <<  m_unitId[m_curtrack] << " AT" << m_lastWord[m_curtrack] << endl;

	if ( do_new_seg || (m_firstSeg[m_curtrack] == false  && m_speaker[m_curtrack] != spkid) ) {
		m_segmentId[m_curtrack] = m_dataModel.insertParentElement(m_unitId[m_curtrack], false);
		new_seg=true;
	}

	m_firstSeg[m_curtrack]=false;

	if (m_CTSMode || m_speaker[m_curtrack] != spkid) { // add new turn
		if ( m_dataModel.getStartAnchor(m_segmentId[m_curtrack]) != m_dataModel.getStartAnchor(m_turnId[m_curtrack]) ) {
			m_turnId[m_curtrack] = m_dataModel.insertParentElement(m_segmentId[m_curtrack], false);
		}
		m_dataModel.setElementProperty(m_turnId[m_curtrack], "speaker", spkid, false);
		m_speaker[m_curtrack] = spkid;
	}

	// insert new section or set current section type
	if ( m_with_sect && ! new_sect.empty() ) {
		if ( m_dataModel.getStartAnchor(m_turnId[m_curtrack]) != m_dataModel.getStartAnchor(m_sectId[m_curtrack]) ) {
			m_sectId[m_curtrack] = m_dataModel.insertParentElement(m_turnId[m_curtrack], false);
		}
		m_dataModel.setElementProperty(m_sectId[m_curtrack], "type", new_sect, false);
	}

	m_prevTime[m_curtrack] = etime;
	m_firstWord[m_curtrack] = true;


	m_enhanced = false;
	if ( new_seg ) {
		m_prev_enhanced = false;
		m_text[m_curtrack] = "";
		m_confidence[m_curtrack] = "";
	}

	StartStack.push(&SAX_VRXMLHandler::WordStart);
    EndStack.push(&SAX_VRXMLHandler::SegmentEnd);
}

// deal with "SpeechSegment" end items
void SAX_VRXMLHandler::SegmentEnd (const XMLCh* const name)
{
//	if ( !m_firstWord[m_curtrack] && (m_prevTime[m_curtrack] - m_lastWord[m_curtrack]) > m_minResol ) {
//		m_unitId[m_curtrack] = m_dataModel.insertMainstreamBaseElement(m_unitId[m_curtrack], m_lastWord[m_curtrack], false, false);
//	}
	m_lastWord[m_curtrack] == m_prevTime[m_curtrack];

  // pop both stacks
  StartStack.pop();
  EndStack.pop();
}

// deal with "Word" start items
void SAX_VRXMLHandler::WordStart
(const XMLCh* const name, AttributeList& attr)
{

		istringstream is1(trans(attr.getValue("stime")));
		istringstream is2(trans(attr.getValue("dur")));
		float stime, dur;
		is1 >> stime;
		is2 >> dur;

		m_text.clear();
		if ( !m_firstWord[m_curtrack] ) {
			if ( dur > 0 ) {
				m_lastWord[m_curtrack] = stime;
				m_unitId[m_curtrack] = m_dataModel.insertMainstreamBaseElement(m_unitId[m_curtrack], stime, false, false);
			} else
				m_text[m_curtrack] = m_dataModel.getElementProperty(m_unitId[m_curtrack],"value");

		}
		if ( dur > 0 ) {
			m_dataModel.setElementProperty(m_unitId[m_curtrack], "score", trans(attr.getValue("conf")));
		}

		m_firstWord[m_curtrack] = false;
		m_lastWord[m_curtrack] = stime + dur;

	// for this version, do nothing. for future versions -> will create "word" annotations
    StartStack.push(&SAX_VRXMLHandler::WordStart);
    EndStack.push(&SAX_VRXMLHandler::WordEnd);
}

// deal with "SpeechSegment" end items
void SAX_VRXMLHandler::WordEnd(const XMLCh* const name)
{
	m_dataModel.setElementProperty(m_unitId[m_curtrack], "subtype", "unit_text", false);
	m_dataModel.setElementProperty(m_unitId[m_curtrack], "value", m_text[m_curtrack], false);
  // pop both stacks
  StartStack.pop();
  EndStack.pop();
}

// invoked when PCDATA encountered
void SAX_VRXMLHandler::characters
(const XMLCh* const chars, const XMLSize_t length)
{
	string s;
	set_string(s, chars);
	int l = 0;
	char buf[12];
	if ( !s.empty() ) {
		if ( ! (m_text[m_curtrack].empty() ) ) {
			l = m_text[m_curtrack].length();
			if ( s == "--" || m_prev_is_special
					|| (l <= 1 || no_blank_after.find(m_text[m_curtrack][l-1]) == string::npos)
				&& ( s.length() <= 1 || no_blank_before.find(s[0]) == string::npos) ) {
				m_text[m_curtrack] += " ";
				++l;
			}
			m_prev_is_special= (s == "--");
		}
		m_text[m_curtrack] += s ;
	}
}

void SAX_VRXMLHandler::warning(const SAXParseException& e)
{
	Log::err() << "WARNING: " << trans(e.getMessage()) << endl;
   Log::err() << " at line " << e.getLineNumber ()
       << " col " << e.getColumnNumber () << endl;
}

void SAX_VRXMLHandler::error(const SAXParseException& e)
{
  Log::err() << "WARNING: " << trans(e.getMessage()) << endl;
   Log::err() << " at line " << e.getLineNumber ()
       << " col " << e.getColumnNumber () << endl;
}

void SAX_VRXMLHandler::fatalError(const SAXParseException& e)
{
  Log::err() << "ERROR: " << trans(e.getMessage()) << endl;
   Log::err() << " at line " << e.getLineNumber ()
       << " col " << e.getColumnNumber () << endl;
  throw agfio::LoadError(trans(e.getMessage()));
}

void SAX_VRXMLHandler::writeChars(const XMLByte* const toWrite,
                                 const XMLSize_t count,
                                 XMLFormatter* const formatter)
{
  targetString.assign((char*) toWrite, count);
}

string& SAX_VRXMLHandler::set_string(string& s, const XMLCh* const chars)
{
  targetString.erase();
  (*formatter) << chars;
  s = StringOps(targetString).trim();
  return s;
}

InputSource*
SAX_VRXMLHandler::resolveEntity(const XMLCh* const publicId,
			      const XMLCh* const systemId)
{
  if (! localDTD.empty()) {
    LocalFileInputSource* is = new LocalFileInputSource(trans(localDTD));
    localDTD = ""; // dirty hack to prevent from loading the dtd for any entity
    return is;
  }

  return NULL;
}

