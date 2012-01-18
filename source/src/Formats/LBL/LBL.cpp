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
* LBL file loader class definition
*/

#define _LBL_IMPL

#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include <ag/AGAPI.h>
#include "LBL.h"
#include "Common/util/StringOps.h"
#include "DataModel/speakers/Speaker.h"
#include "Common/FileInfo.h"
#include "Common/iso639.h"
#include "Common/globals.h"

#include <ag/AGException.h>

#define DEFAULT_CONVENTIONS "transag_default"


#ifdef EXTERNAL_LOAD
list<AGId>
LBL::plugin_load(const string& filename,
	 const Id& id,
	 map<string,string>* signalInfo,
	 map<string,string>* options)
  throw (agfio::LoadError)
{
  try {
    load(filename, id, signalInfo, options);
  }
  catch (const agfioError& e) {
    throw agfio::LoadError(string("LBL:") + e.what());
  }
}


string
LBL::plugin_store(const string& filename,
	  const string& id,
	  map<string,string>* options)
  throw (agfio::StoreError)
{
  try {
    store(filename, id, options);
  }
  catch (const agfioError& e) {
    throw agfio::StoreError(string("LBL:") + e.what());
  }
}

#endif


LBL::LBL()
{
	m_speakerRE	= new RE("[^,]*,[^,]*, *([^ ]*)");
}

LBL::~LBL()
{
	delete m_speakerRE;
}


list<AGId> LBL::load(const string& filename,
							const Id& id,
							map<string,string>* signalInfo,
							map<string,string>* options)
throw (agfio::LoadError)
{
	m_options = options;
	init_done=false;
	signalFilename = "";

	LBLfile B;
	m_fullmode = true;

	if (!B.open(filename)) {
		throw agfio::LoadError("LBL: can't open " + filename);
	}

	list<AGId> res;
	tag::DataModel* data = NULL;
	bool do_del = false;

	m_lastTime = 0.0;

	try {
		string addr= "";
		corpusName = "";
		map<string,string>::iterator it;

		if ( (it = options->find("&datamodel")) != options->end() )  addr = it->second;
		if ( (it = options->find("corpusName")) != options->end() )  corpusName = it->second;
		if ( (it = options->find("fullmode")) != options->end() )  m_fullmode = (it->second == "true" || it->second == "1");


		if ( corpusName.empty() ) corpusName="TransAG";


		if (  addr != "" ) {
			// data model adress passed by caller -> use it
			data = (tag::DataModel*)strtoul(addr.c_str(), NULL, 16);
		}
		else {
			data = new tag::DataModel();
			data->setKeepAG(true); // do not delete graph upon datamodel deletion
			do_del = true;
		}

		while (B.read_record()) {
			if ( B.get_type() == "turn" )
				processTurn(B, data);
			else // unexpected record type
				throw agfio::LoadError("LBL: invalid record found at line " + B.get_lineno() + " when reading " + filename);
		}
		// set signal duration (which also sets last anchor offset)
		data->setSignalDuration(m_lastTime);

		// last line hold only signal duration -> remove elements added for lastTime

		if ( !m_unitId.empty() && m_unitId[0]!="" && fabs(m_lastTime - data->getStartOffset(m_unitId[0])) < 0.00001 )
			data->deleteMainstreamElement(m_unitId[0]);

		data->updateVersionInfo("LBL2TAG", "1");
		res.push_back(data->getAGTrans() );
	}
	catch (const char* msg) {
		throw agfio::LoadError("\nLBL format: " + string(msg));
	}
	catch (const string& msg) {
		throw agfio::LoadError("\nLBL format: " + msg);
	}
	catch (AGException& e) {
		throw agfio::LoadError("\nLBL format: " + e.error());
	}

	if ( do_del )
		delete data;

	return res;
}

// do some inits
void LBL::initialize(LBLfile& B, tag::DataModel* data)
{
	map<string,string>::iterator it;
	lang = "";
	conventions=DEFAULT_CONVENTIONS;


	// some options may be passed as options by calling program
	string item;

	if ( (it = m_options->find("lang")) != m_options->end() )  item = it->second;
	if ( item.length() == 2 ) {
		const char* pl = ISO639::get3LetterCode(item.c_str());
		if ( pl != NULL ) lang = pl;
	} else if (item.length() == 3) { // check validity
		const char* pl = ISO639::get2LetterCode(item.c_str());
		if ( pl != NULL ) lang=item;
	}
	if (lang == "" ) {
		Log::err() << _("Unknown transcription language, using 'eng' as default");
		lang = "eng";
	}
	if ( (it = m_options->find("conventions")) != m_options->end() ) conventions = it->second;

	// configure data model
	try {
		data->setConventions(conventions, lang, m_fullmode);
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

	data->initAGSet(corpusName);
	addSignal(data, 1);

	// initialize transcription graph
	data->initAnnotationGraphs("", lang, "lbl2tag");

	item = B.get_item("author");
	if ( item.empty() ) item = "(unknown)";
	string date = B.get_item("date");
	if ( date.empty() ) date = "(unknown)";
	string comment = "original LBL file format";
	data->initVersionInfo(item, date, comment);

	//> LBL format has no speaker, create a default one
	string speaker = _("speaker") ;
	speaker += "1" ;
	m_spkid = getDefaultSpeaker(speaker, data) ;

	init_done=true;
}


void LBL::addSignal(tag::DataModel* data, int notrack)
{
	data->addSignal(signalFilename, "audio", "", "", notrack);
	m_unitId.push_back("") ;
	m_segmentId.push_back("") ;
	m_turnId.push_back("") ;
	m_sectId.push_back("") ;
	m_prevSpkid.push_back("") ;
}


/**
*  processTurn : add new speech turn to data model
*/
void LBL::processTurn(LBLfile& B,
	       tag::DataModel* data)
{
	signalFilename = "unknown-file" ;
	bool first = true ;
	if ( !init_done )
		initialize(B, data);
	else
		first = false ;

	//DEFAULT
	int notrack = 0 ;
	float start = atof(B.get_matched(1).c_str()) ;

	bool nontrans = false ;
	bool empty_seg = false ;
	string new_sect("");

	if (first) {
		//> first turn, get basis section/turn/seg
		m_unitId[notrack] = data->getByOffset(data->mainstreamBaseType(), 0.0, notrack);
		m_segmentId[notrack] =  data->getParentElement(m_unitId[notrack]) ;
		m_turnId[notrack] =  data->getParentElement(m_segmentId[notrack]);
		m_sectId[notrack] =  data->getParentElement(m_turnId[notrack]);
		//> here start isn't 0.0, so from 0.0 to real start, no speaker and no trans
		data->setElementProperty(m_turnId[notrack], "speaker", tag::Speaker::NO_SPEECH);
		data->setElementProperty(m_sectId[notrack], "type", "nontrans");
		m_prevSpkid[notrack] = tag::Speaker::NO_SPEECH ;
		first = false;
	}


	//> FILL DATAMODEL FOR CAUGHT TURN
	if ( start == 0.0 ) {
		data->setElementProperty(m_turnId[notrack], "speaker", m_spkid, false) ;
		//> LBL format has no section, create section trans by default
		data->setElementProperty(m_sectId[notrack], "type", "report", false) ;
		m_prevSpkid[notrack] = m_spkid;
	}
	else {
		//> First turn found but doesn't start at 0.00
		if (first) {
			//> from real start to next time, prepare creation of a report section
			new_sect = "report";
			nontrans = empty_seg = false ;
			first = false;
		}

		//> create segment for current matching text found
		m_unitId[notrack] = data->insertMainstreamBaseElement(m_unitId[notrack], start, true, false);
		m_segmentId[notrack] = data->insertParentElement(m_unitId[notrack], false);

	}

	// finally add text to current text segment
	if ( ! (nontrans || empty_seg) ) {
		empty_seg = processText(B, data, notrack);
	}

	string spkid = m_spkid;
	if ( empty_seg ) {
		spkid = tag::Speaker::NO_SPEECH;
	}

	//> change of speaker
	if ( m_prevSpkid[notrack] != spkid ) {
		//> if non-empty segment & previous speaker not a nospeech & turn duration is ok
		m_turnId[notrack] = data->insertParentElement(m_segmentId[notrack], false);
		data->setElementProperty(m_turnId[notrack], "speaker", spkid);
		m_prevSpkid[notrack] = spkid ;
	}

	// insert new section or set current section type
	if ( ! new_sect.empty() ) {
		m_sectId[notrack] = data->insertParentElement(m_turnId[notrack], false);
		data->setElementProperty(m_sectId[notrack], "type", new_sect);
	}

	m_lastTime = start ;
}


/* @return true if segment contains text */
bool LBL::processText(LBLfile& B,
	       tag::DataModel* data, int notrack)
{
	string text = B.get_matched(2);

	//> check encoding
	if (!text.empty())
	{
		string encoding = B.get_item("encoding") ;
		int err;
		text = tag::FormatToUTF8::checkUTF8(text, encoding, false, err) ;

		// TODO -> analyse text string to extract event tags
		data->setElementProperty(m_unitId[notrack], "subtype", "unit_text", false);
		data->setElementProperty(m_unitId[notrack], "value", text, false);
		return false;
	}
	return true;
}

const string& LBL::getDefaultSpeaker(const string& name, tag::DataModel* data)
{
	tag::Speaker spk = data->getSpeakerDictionary().defaultSpeaker();
	spk.setLastName(name);
	data->getSpeakerDictionary().addSpeaker(spk);
	return spk.getId();
}

int LBL::getNoTrack(Glib::ustring parsed_track)
{
	Glib::ustring::iterator it = parsed_track.begin() ;

	if (parsed_track.size()>1)
		return 0 ;

	for (it= parsed_track.begin(); it!=parsed_track.end(); it++) {
		//> numbers
		if ((*it)>=49 && (*it)<=57) {
			return (*it)-49 ;
		}
		//> Upper letters
		else if ((*it)>=65 && (*it)<=90) {
			return (*it)-65 ;
		}
		//> Lower case
		else if ((*it)>=97 && (*it)<=122) {
			return (*it)-97 ;
		}

	}
	return 0 ;
}

