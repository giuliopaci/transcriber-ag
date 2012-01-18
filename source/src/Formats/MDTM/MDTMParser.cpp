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

#define _MDTM_IMPL

#include <iostream>
#include <string>
#include <fstream>
#include <ag/AGAPI.h>
#include "MDTMParser.h"
#include "Common/util/StringOps.h"
#include "DataModel/speakers/Speaker.h"
#include "Common/FileInfo.h"
#include "Common/iso639.h"

#include <ag/AGException.h>

#define DEFAULT_CONVENTIONS "transag_default"


MDTMParser::MDTMParser(const string& corpus, map<string,string>* options, bool mode)
: corpusName(corpus), m_options(options), fullmode(mode), init_done(false)
{
}


// --- ~CharParser ---
MDTMParser::~MDTMParser()
{}


// --- Parse ---
list<AGId> MDTMParser::parse(MDTMfile& B, tag::DataModel* data)
{

	signalFilename	= "";
	eof				= false;
	nbSegments		= 0;

	list<AGId>		res;

	try
	{
		if (!init_done)
			initialize(B, data);

		// -- Turn Processing --
		while ( B.read_record() )
		{
			if (B.get_type() == "turn")
				processTurn(B, data);
			else
				throw agfio::LoadError("MDTM: Invalid record found at line " + B.get_lineno() );
		}

		data->setSignalDuration(m_lastTime);
		data->updateVersionInfo("MDTM2TAG", "1");
		res.push_back(data->getAGTrans() );
	}
	catch (const char* msg)
	{
		throw agfio::LoadError("\nMDTM format: " + string(msg));
	}

	catch (const string& msg)
	{
		throw agfio::LoadError("\nMDTM format: " + msg);
	}

	catch (AGException& e)
	{
		throw agfio::LoadError("\nMDTM format: " + e.error());
	}

	return res;
}


// --- Initialize ---
void MDTMParser::initialize(MDTMfile& B, tag::DataModel* data)
{
	map<string,string>::iterator it;

	// -- Defaults --
	lang		= "";
	conventions	= DEFAULT_CONVENTIONS;


	// -- Options Check --
	string item;


	if ( (it = m_options->find("lang")) != m_options->end() )
		item = it->second;

	if ( item.length() == 2 )
	{
		const char* pl = ISO639::get3LetterCode(item.c_str());

		if ( pl != NULL )
			lang = pl;
	}
	else
	if (item.length() == 3)
	{
		// check validity
		const char* pl = ISO639::get2LetterCode(item.c_str());
		if ( pl != NULL )
			lang=item;
	}

	if (lang == "" )
	{
		Log::err() << _("Unknown transcription language, using 'eng' as default");
		lang = "eng";
	}

	if ( (it = m_options->find("conventions")) != m_options->end() )
		conventions = it->second;

	// configure data model
	// If conventions haven't been previously set let's configure
	// (could have been set before plugin call when identifying import format)
	if ( !data->conventions().loaded() )
	{
		try {
				data->setConventions(conventions, lang);
		}
		catch ( const char* msg ) {
			Log::err() << msg << endl;
			data->setConventions("", "", fullmode);
		}
	}

	data->initAGSet(corpusName);
	addSignal(data, 1);

	// initialize transcription graph
	data->initAnnotationGraphs("", lang, "mdtm2tag");

	TRACE << "MDTM ----> LOADED CONVENTIONS : " << data->conventions().name() << std::endl ;

	item = B.get_item("author");
	if ( item.empty() )
		item = "(unknown)";
	string date = B.get_item("date");

	if ( date.empty() )
		date = "(unknown)";

	string comment = "original MDTM file format";
	data->initVersionInfo(item, date, comment);
	data->setTranscriptionLanguage("fra");

	init_done=true;
}


// --- AddSignal ---
void
MDTMParser::addSignal(tag::DataModel* data, int notrack)
{
	data->addSignal(signalFilename, "audio", "", "", notrack);

	m_unitId	= "";
	m_segmentId	= "";
	m_turnId		= "";
	m_sectId		= "";
	m_prevSpkid	= "";
}


/**
 *  processTurn : add new speech turn to data model
 */
void
MDTMParser::processTurn(MDTMfile& B,
			tag::DataModel* data)
{
	vector<string> turnItems = B.get_turn();

	// -- Speaker ID --
	checkSpeakerID(turnItems[7], turnItems[6], data);


	// -- Timestamps --
	string speakerID	= m_spkids[ turnItems[7] ];
	float startTime	= atof( turnItems[2].c_str() );
	m_lastTime		= startTime + atof( turnItems[3].c_str() );

;	// -- Empty Turns --
	if (m_unitId == "")
	{
		m_unitId	= data->getByOffset(data->mainstreamBaseType(), 0.0);
		m_segmentId	= data->getParentElement(m_unitId);
		m_turnId	= data->getParentElement(m_segmentId);

		data->setElementProperty(m_turnId, "speaker", speakerID, false);
	}
	else
	{
		m_unitId	= data->insertMainstreamBaseElement(m_unitId, 0.0, true, false);
		m_segmentId	= data->insertParentElement(m_unitId, false);
		m_turnId	= data->insertParentElement(m_segmentId, false);
	}
}


/**
 * checkSpeakerID : if speaker doesn't exist, we add it to datamodel
 */
void
MDTMParser::checkSpeakerID( const string& spkID,
							const string& spkGender,
							tag::DataModel* data)
{
	// -- Existing Speaker ID --
	if (m_spkids.find(spkID) != m_spkids.end())
		return;

	// -- Adding Speaker to DataModel --
	tag::Speaker spk = data->getSpeakerDictionary().defaultSpeaker();
	spk.setFirstName(spkID);

	// -- Gender --
	if (spkGender == "adult_male")
		spk.setGender(tag::Speaker::MALE_GENDER);
	else
	if (spkGender == "adult_female")
		spk.setGender(tag::Speaker::FEMALE_GENDER);

	data->getSpeakerDictionary().addSpeaker(spk);

	m_spkids[spkID] = spk.getId();
}

