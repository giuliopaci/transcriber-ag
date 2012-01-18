/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * $Id*
 *
 * VS Fast Transcription file loader class definition
 */

#define _STM_IMPL

#include <iostream>
#include <string>
#include <fstream>
#include <ag/AGAPI.h>
#include "STMParser.h"
#include "Common/util/StringOps.h"
#include "DataModel/speakers/Speaker.h"
#include "Common/FileInfo.h"
#include "Common/iso639.h"

#include <ag/AGException.h>

#define DEFAULT_CONVENTIONS "transag_default"


STMParser::STMParser(const string& corpus, map<string,string>* options, bool mode)
: m_corpusName(corpus), m_options(options), m_fullmode(mode), m_initDone(false)
{
	m_speakerRE	= new RE("[^,]*,*([^, ]*),*([^ ]*)");
}

STMParser::~STMParser()
{
	delete m_speakerRE;
}


list<AGId> STMParser::parse(STMfile& B, tag::DataModel* data)
{

	m_signalFilename = "";
	list<AGId> res;

	try {
		while (B.read_record()) {
			if ( B.get_type() == "turn" )
				processTurn(B, data);
			else // unexpected record type
				throw agfio::LoadError("STM: invalid record found at line " + B.get_lineno() );
		}
		// set signal duration (which also sets last anchor offset)
		data->setSignalDuration(m_prevTime[0]);

		data->updateVersionInfo("STM2TAG", "1");
		res.push_back(data->getAGTrans() );
	}
	catch (const char* msg) {
	throw agfio::LoadError("\nSTM format: " + string(msg));
	}
	catch (const string& msg) {
	throw agfio::LoadError("\nSTM format: " + msg);
	}
	catch (AGException& e) {
	throw agfio::LoadError("\nSTM format: " + e.error());
	}
	return res;
}


// do some inits
void STMParser::initialize(STMfile& B, tag::DataModel* data)
{
	map<string,string>::iterator it;
	m_lang = "";
	m_prevBackgroundId = "";
	m_conventions=DEFAULT_CONVENTIONS;

	// some options may be passed as options by calling program
	string item;


	if ( (it = m_options->find("lang")) != m_options->end() )  item = it->second;
	if ( item.length() == 2 ) {
		const char* pl = ISO639::get3LetterCode(item.c_str());
		if ( pl != NULL ) m_lang = pl;
	} else if (item.length() == 3) { // check validity
		const char* pl = ISO639::get2LetterCode(item.c_str());
		if ( pl != NULL ) m_lang=item;
	}
	if (m_lang == "" ) {
		Log::err() << _("Unknown transcription language, using 'eng' as default");
		m_lang = "eng";
	}
	if ( (it = m_options->find("conventions")) != m_options->end() ) m_conventions = it->second;

	// configure data model
	try {
		data->setConventions(m_conventions, m_lang, m_fullmode);
	} catch ( const char* msg ) {
		Log::err() << msg << endl;
		data->setConventions("", "", m_fullmode);
	}

	m_minTurnSize = data->conventions().minSegmentSize("turn",MIN_TURN_SIZE);
	if ( (it = m_options->find("min_turn_size")) != m_options->end() )  {
		float sz = atof(it->second.c_str());
		if ( sz > 0 ) {
			m_minTurnSize = sz;
		}
	}
	m_minSegSize = data->conventions().minSegmentSize("segment",MIN_SEG_SIZE);
	if ( m_minSegSize > m_minTurnSize )  m_minSegSize = m_minTurnSize;

	data->initAGSet(m_corpusName);
	addSignal(data, 1);

	// initialize transcription graph
	data->initAnnotationGraphs("", m_lang, "stm2tag");

	item = B.get_item("author");
	if ( item.empty() ) item = "(unknown)";
	string date = B.get_item("date");
	if ( date.empty() ) date = "(unknown)";
	string comment = "original STM file format";
	data->initVersionInfo(item, date, comment);

	m_initDone=true;
}
void
STMParser::addSignal(tag::DataModel* data, int notrack)
{
	data->addSignal(m_signalFilename, "audio", "", "", notrack);
	m_unitId.push_back("") ;
	m_segmentId.push_back("") ;
	m_turnId.push_back("") ;
	m_sectId.push_back("") ;
	m_bgId.push_back("") ;
	m_prevSpkid.push_back("") ;
	m_prevTime.push_back(0.0);
}

/**
 *  processTurn : add new speech turn to data model
 */
void
STMParser::processTurn(STMfile& B,
			tag::DataModel* data)
{
	m_signalFilename = B.get_matched(1);
	if ( !m_initDone ) initialize(B, data);

	int notrack = atoi(B.get_matched(2).c_str()) - 1;
	if ( notrack >= data->getNbTracks() )
		addSignal(data, notrack);

	string spkid = B.get_matched(3);
	float start = atof(B.get_matched(4).c_str());
	float end = atof(B.get_matched(5).c_str());
	string generalInfo = B.get_matched(6);
	bool nontrans, empty_seg;
	string new_sect("");

	setBackground(notrack, start, generalInfo, data);

	if ( spkid == "excluded_region" )
	{
		nontrans=true;
		new_sect = "nontrans";
	}
	else
	{
		if ( spkid == "inter_segment_gap" )
			empty_seg=true;
		else
		{
			nontrans = empty_seg = false;
			if ( spkid.compare(0, m_signalFilename.length(), m_signalFilename) == 0 )
				spkid = spkid.substr(m_signalFilename.length()+1);
			checkSpeakerId(spkid, generalInfo, data);
		}
		if ( m_prevSpkid[notrack] == "excluded_region" )
			new_sect = "report";
		else
			new_sect = "";
	}

	if ( start == 0.0 )
	{
		m_unitId[notrack] = data->getByOffset(data->mainstreamBaseType("transcription_graph"), 0.0, notrack);
		m_segmentId[notrack] = data->getParentElement(m_unitId[notrack]);
		m_turnId[notrack] =  data->getParentElement(m_segmentId[notrack]);
		m_sectId[notrack] =  data->getParentElement(m_turnId[notrack]);
		if (empty_seg || nontrans )
			data->setElementProperty(m_turnId[notrack], "speaker", tag::Speaker::NO_SPEECH, false);
		else
			data->setElementProperty(m_turnId[notrack], "speaker", m_spkids[spkid], false);

		if ( nontrans )
			data->setElementProperty(m_sectId[notrack], "type", "nontrans", false);

		new_sect = "";
	}
	else
	{
		if ( (start - m_prevTime[notrack]) > MIN_RESOLUTION )
			m_unitId[notrack] = data->insertMainstreamBaseElement(m_unitId[notrack], m_prevTime[notrack], false, false);
		if ( (start - m_prevTime[notrack]) >= m_minSegSize )
		{
			// gap between segments -> add an empty segment
			m_segmentId[notrack] = data->insertParentElement(m_unitId[notrack], false);
			if ( start - m_prevTime[notrack] >= m_minTurnSize )
			{
				// then create a "nospeaker" turn
				m_turnId[notrack] = data->insertParentElement(m_segmentId[notrack], false);
				data->setElementProperty(m_turnId[notrack], "speaker", tag::Speaker::NO_SPEECH, false);
				m_prevSpkid[notrack] = tag::Speaker::NO_SPEECH;
			}
			m_prevTime[notrack] = start;
		}
		m_unitId[notrack] = data->insertMainstreamBaseElement(m_unitId[notrack], start, false, false);
		m_segmentId[notrack] = data->insertParentElement(m_unitId[notrack], false);
		if ( m_prevSpkid[notrack] != m_spkids[spkid] )
		{
			m_turnId[notrack] = data->insertParentElement(m_segmentId[notrack], false);
			if ( empty_seg || nontrans)
				data->setElementProperty(m_turnId[notrack], "speaker",tag::Speaker::NO_SPEECH, false);
			else
				data->setElementProperty(m_turnId[notrack], "speaker", m_spkids[spkid], false);
			m_prevSpkid[notrack] = m_spkids[spkid] ;
		}
	}

	// insert new section or set current section type
	if ( ! new_sect.empty() )
	{
		m_sectId[notrack] = data->insertParentElement(m_turnId[notrack], false);
		data->setElementProperty(m_sectId[notrack], "type", new_sect);
	}

	m_prevTime[notrack] = end;

	// finally add text to current text segment
	if ( ! (nontrans || empty_seg) )
		processText(B, data, notrack);
}


void
STMParser::processText(STMfile& B,
			tag::DataModel* data, int notrack)
{
	string text = B.get_matched(7) ;
	unsigned int pos = text.find_first_not_of(' ');
	if ( pos > 0 )
		text.erase(0, pos);


	//> check encoding
	if (!text.empty())
	{
		string encoding = B.get_item("encoding") ;
		int err;
		text = tag::FormatToUTF8::checkUTF8(text, encoding, false, err) ;
	}

	// TODO -> analyse text string to extract event tags
	data->setElementProperty(m_unitId[notrack], "subtype", "unit_text", false);
	data->setElementProperty(m_unitId[notrack], "value", text, false);
}

void STMParser::checkSpeakerId(const string& spkid, const string& spkinfo,
			tag::DataModel* data)
{
	if ( m_spkids.find(spkid) == m_spkids.end() ) {
		string dicoId = "spk"+StringOps().fromInt(m_spkids.size()+1);
		tag::Speaker spk ;
		if ( data->getSpeakerDictionary().existsSpeaker(String(dicoId)) ) {
			spk = data->getSpeakerDictionary().getSpeaker(dicoId);
		} else
		{
			spk = data->getSpeakerDictionary().defaultSpeaker();
			data->getSpeakerDictionary().addSpeaker(spk);
		}
		if ( ! spkinfo.empty() && m_speakerRE->match(spkinfo) )
			spk.setGender(m_speakerRE->get_matched(2).c_str());
		spk.setLastName(spkid);
		data->getSpeakerDictionary().updateSpeaker(spk, false);
		m_spkids[spkid] = spk.getId();
	}
}

/*
 * Background mapping
 * F4    -> noise
 * F3    -> music
 * FX    -> music;noise
 */
string STMParser::formatBackground(const string& background)
{
    if (background == "f3") return "music";
	else if (background == "fx") return "music;noise";
	else if (background == "f4") return "noise";
	else return "none";
}


void STMParser::setBackground(int notrack, float start, const string& generalInfo,
			tag::DataModel* data)
{
	const string& basetype = data->mainstreamBaseType("background_graph");
	if ( m_bgId[notrack].empty() )
		m_bgId[notrack] = data->getByOffset(basetype, 0.0, notrack);

	if ( ! generalInfo.empty() && m_speakerRE->match(generalInfo) )
	{
		string backID = m_speakerRE->get_matched(1).c_str();

		if ( m_prevBackgroundId != backID)
		{
			const string& bgtype = formatBackground(backID);
			if ( start > data->getStartOffset(m_bgId[notrack]) )
				m_bgId[notrack] = data->insertMainstreamElement(basetype, m_bgId[notrack], start, false, false);

			data->setElementProperty(m_bgId[notrack], "type", bgtype , false);
			if ( bgtype == "none" )
				data->setElementProperty(m_bgId[notrack], "level", "low" , false);
			else
				data->setElementProperty(m_bgId[notrack], "level", "medium", false );
			m_prevBackgroundId = backID;
		}
	}
}

