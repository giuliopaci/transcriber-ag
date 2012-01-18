/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/*
 * CHATParser.cpp: CHILDES CHAT native format : class definition
 */

#define _CHAT_IMPL

#include <iostream>
#include <string>
#include <fstream>
#include <ag/AGAPI.h>
#include "CHATParser.h"
#include "Common/util/StringOps.h"
#include "DataModel/speakers/Speaker.h"
#include "Common/FileInfo.h"
#include "Common/iso639.h"
//#include "MediaComponent/base/Guesser.h"
#include <ag/AGException.h>

#define DEFAULT_CONVENTIONS "transag_default"

using namespace tag;

// --- CHATParser ---
CHATParser::CHATParser(const string& corpus, map<string,string>* options, bool mode)
: corpusName(corpus), p_options(options), fullmode(mode), init_done(false)
{
	timecodeRE	= new RE("([0-9]+)_([0-9]+)");
	lastSPK = "";
	spkids["NSP"] = "no_speech"; //init no_speaker id
}


// --- ~CHATParser ---
CHATParser::~CHATParser()
{
	delete speakerRE;
	delete badSpeakerRE;
	delete timecodeRE;
}


// --- Parse ---
list<AGId> CHATParser::parse(CHATfile& B, tag::DataModel* data)
{

	signalFilename	= "";
	eof				= false;
	startTime		= 0.0;
	endTime			= 0.0;
	nbSegments		= 0;
	segmentOrder	= 0;
	list<AGId>		res;

	try
	{
		if (!init_done)
			initialize(B, data);

		// -- Headers Processing --
		B.read_headers();

		processHeaders(B, data);
		processSpeakers(B, data);


		// -- Turn Processing --
		while (B.read_record() && !eof)
		{
			if (B.get_type() == "turn")
				processTurn(B, data);
			else
			if (B.get_type() == "eof")
			{
				processTurn(B, data);
				eof = true;
			}
			else
				throw agfio::LoadError("CHAT: Invalid record found at line " + B.get_lineno() );
		}

		data->setSignalDuration(endTime);
		data->updateVersionInfo("CHAT2TAG", "1");
		res.push_back(data->getAGTrans() );
	}
	catch (const char* msg)
	{
		throw agfio::LoadError("\nCHAT format: " + string(msg));
	}

	catch (const string& msg)
	{
		throw agfio::LoadError("\nCHAT format: " + msg);
	}

	catch (AGException& e)
	{
		throw agfio::LoadError("\nCHAT format: " + e.error());
	}

	return res;
}


// --- Initialize ---
void CHATParser::initialize(CHATfile& B, tag::DataModel* data)
{
	map<string,string>::iterator it;

	// -- Defaults --
	lang		= "";
	conventions	= DEFAULT_CONVENTIONS;
	minTurnSize = MIN_TURN_SIZE;

	// -- Options Check --
	string item;

	if ( (it = p_options->find("min_turn_size")) != p_options->end() )
	{
		minTurnSize = atof(it->second.c_str());

		if ( minTurnSize <= 0 )
			minTurnSize = MIN_TURN_SIZE;
	}

	if ( (it = p_options->find("lang")) != p_options->end() )
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

	if ( (it = p_options->find("conventions")) != p_options->end() )
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
	data->initAnnotationGraphs("", lang, "chat2tag");

	TRACE << "CHAT ----> LOADED CONVENTIONS : " << data->conventions().name() << std::endl ;

	item = B.get_item("author");
	if ( item.empty() )
		item = "(unknown)";
	string date = B.get_item("date");

	if ( date.empty() )
		date = "(unknown)";

	string comment = "original CHAT file format";
	data->initVersionInfo(item, date, comment);
	data->setTranscriptionLanguage("fra");

	init_done=true;
}


// --- AddSignal ---
void
CHATParser::addSignal(tag::DataModel* data, int notrack)
{
	data->addSignal(signalFilename, "audio", "", "", notrack);

	segmentId	= "";
	turnId		= "";
	sectId		= "";
	prev_spkid	= "";
}


// --- ProcessHeaders ---
void
CHATParser::processHeaders(CHATfile &B, tag::DataModel* data)
{}


// --- ProcessSpeakers ---
void
CHATParser::processSpeakers(CHATfile &B, tag::DataModel* data)
{
	// -- Regexps --
	speakerRE		= new RE("[\t ]*([^\t ]+)[\t ]+([^\t ]+)[\t ]+([^\t ]+)");	// ID, name, role
	badSpeakerRE	= new RE("[\t ]*([^\t ]+)[\t ]+([^\t ]+)");					// ID, role (incorrect)

	string spk = B.get_option("Participants");
	vector<string> spks;

	bool inParse	= true;
	unsigned int toIdx		= string::npos;

	// -- Looking for multiple speakers --
	while (inParse)
	{
		toIdx = spk.find_first_of(",");

		if (toIdx != string::npos)
		{
			spks.push_back( spk.substr(0, toIdx) );
			spk = spk.substr(toIdx + 1);
		}
		else
			inParse = false;
	}

	// -- Last / Single Speaker --
	if (!spk.empty())
		spks.push_back(spk);

	// -- Parsing Speaker Info --
	for(int i=0; i<spks.size(); i++)
	{
		Speaker	spk = data->getSpeakerDictionary().defaultSpeaker();
		string	speakerFullName	= "";
		string	speakerFirstName = "";
		string	speakerLastName	= "";
		string	speakerID	= "";
		string	speakerRole	= "";

		if (speakerRE->match(spks[i]))
		{
			speakerID	= speakerRE->get_matched(1);
			speakerFullName = speakerRE->get_matched(2);
			speakerRole	= speakerRE->get_matched(3);
		}
		else
		if (badSpeakerRE->match(spks[i]))
		{
			speakerID		= badSpeakerRE->get_matched(1);
			speakerRole		= badSpeakerRE->get_matched(2);
			speakerFullName	= speakerRole;
		}

		size_t sep_ptr = speakerFullName.find_first_of("~");

		if (sep_ptr > speakerFullName.length())
		{
			sep_ptr = 0;
		}
		else
		{
			speakerFirstName = StringOps(speakerFullName.substr(0, sep_ptr)).replace("_", " ", true);
			spk.setFirstName(speakerFirstName);
			sep_ptr++;
		}

		speakerLastName = StringOps(speakerFullName.substr(sep_ptr, speakerFullName.size())).replace("_", " ", true);;

		// -- Adding Speaker to DataModel --
		spk.setLastName(speakerLastName);
		spk.setGender(speakerRole);
		data->getSpeakerDictionary().addSpeaker(spk);


		// TODO -> Gender Detection

		spkids[speakerID] = spk.getId();
	}
}


/**
 *  processTurn : add new speech turn to data model
 */
void
CHATParser::processTurn(CHATfile& B,
			tag::DataModel* data)
{
	// -- Default : Single Track for CHA files --
	string turnText		= B.get_turn_text();
	string turnSpeaker	= B.get_turn_speaker();

	multimap<string, string> turnOptions = B.get_turn_options();

	// -- Timecodes ? --
	if (timecodeRE->match(turnText))
	{
		startTime	= atof( timecodeRE->get_matched(1).c_str() ) / 1000.0;
		endTime		= atof( timecodeRE->get_matched(2).c_str() ) / 1000.0;

		// -- Timecode Removal --
		int erasePos = turnText.find( timecodeRE->get_matched(1) );

		if (erasePos != string::npos)
			turnText.erase(erasePos);
	}
	else
	{
		startTime	+= MIN_TURN_SIZE;
		endTime		= startTime + MIN_TURN_SIZE;
	}


	// -- Lazy Overlap Marker : +< --
	if (turnText.find("+<") != string::npos)
		segmentOrder++;


	// -- New Text Segment --
	if (segmentId == "")
	{
		unitId		= data->getByOffset(data->mainstreamBaseType(), 0.0);
		segmentId	= data->getParentElement(unitId);
		turnId		= data->getParentElement(segmentId);

		// New Turn for NO_SPEAKER to take in account section mark
		if ( ( turnSpeaker != lastSPK )  || ( turnSpeaker == "NSP" ) )
		{
			lastSPK = turnSpeaker;
			data->setElementProperty(turnId, "speaker", spkids[turnSpeaker], false);
		}
	}
	else
	{
		if (segmentOrder > 0)
		{
			string tmp_unitId = data->insertOverlappingElement(turnId);

			turnId = data->getParentElement(tmp_unitId, "turn");

			data->setElementProperty(turnId, "speaker", spkids[turnSpeaker], false);
			lastSPK = "";
		}
		else
		{
			unitId		= data->insertMainstreamBaseElement(unitId, startTime, false, false);
			segmentId	= data->insertParentElement(unitId, false);

			if ( ( turnSpeaker != lastSPK )  || ( turnSpeaker == "NSP" ) )
			{
				if (segmentOrder == 0)
					turnId = data->insertParentElement(segmentId, false);

				lastSPK = turnSpeaker;
				data->setElementProperty(turnId, "speaker", spkids[turnSpeaker], false);
			}
		}
	}

	// -- Segment Text --
	if (!turnText.empty())
	{
		string encoding = B.get_item("encoding");
		int err;
		turnText = tag::FormatToUTF8::checkUTF8(turnText, encoding, false, err) ;
	}

	if (segmentOrder == 0)
	{
		data->setElementProperty(unitId, "subtype", "unit_text");
		data->setElementProperty(unitId, "value", turnText);
	}
	else
	{
		vector<string> units;
		vector<string>::iterator it;

		data->getChilds(units, "unit", turnId);

		for(it = units.begin(); it != units.end(); it++)
		{
			string textToPrint;
			if ( turnText.substr(0,3) == "+< " )
				textToPrint = turnText.substr(3, turnText.size() - 3);

			data->setElementProperty(*it, "subtype", "unit_text");
			data->setElementProperty(*it, "value", textToPrint);
		}
	}


	// -- Lazy Overlap Marker : +< --
	if (turnText.find("[>]") != string::npos)
		segmentOrder++;


	// -- Overlap Precedes Marker : [<] --
	if (turnText.find("[<]") != string::npos ||
		turnText.find("+<") != string::npos)
		segmentOrder--;
	nbSegments++;


	// -- Turn Options : Segments Qualifiers --
	map<string, string>::iterator it;

	for(it = turnOptions.begin(); it != turnOptions.end(); it++)
		data->addQualifier(it->first, segmentId,  "", it->second,  false);
}

