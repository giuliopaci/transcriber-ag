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
* CTM file loader class definition
*/

#define _CTM_IMPL

#include <iostream>
#include <string>
#include <fstream>
#include <ag/AGAPI.h>
#include "CTM.h"
#include "Common/util/StringOps.h"
#include "Common/util/FormatToUTF8.h"
#include "DataModel/speakers/Speaker.h"
#include "Common/FileInfo.h"
#include "Common/iso639.h"
#include "Common/globals.h"

#include <ag/AGException.h>

#define DEFAULT_CONVENTIONS "transag_default"


#ifdef EXTERNAL_LOAD
list<AGId>
CTM::plugin_load(const string& filename,
	 const Id& id,
	 map<string,string>* signalInfo,
	 map<string,string>* options)
  throw (agfio::LoadError)
{
  try {
    load(filename, id, signalInfo, options);
  }
  catch (const agfioError& e) {
    throw agfio::LoadError(string("CTM:") + e.what());
  }
}


string
CTM::plugin_store(const string& filename,
	  const string& id,
	  map<string,string>* options)
  throw (agfio::StoreError)
{
  try {
    store(filename, id, options);
  }
  catch (const agfioError& e) {
    throw agfio::StoreError(string("CTM:") + e.what());
  }
}

#endif


CTM::CTM()
{
	speakerRE	= new RE("[^,]*,[^,]*, *([^ ]*)");
	nospeech	= tag::Speaker::NO_SPEECH;
	need_new_segment = false;
}

CTM::~CTM()
{
	delete speakerRE;
}


list<AGId> CTM::load(const string& filename,
							const Id& id,
							map<string,string>* signalInfo,
							map<string,string>* options)
throw (agfio::LoadError)
{
	p_options = options;
	init_done=false;
	signalFilename = "";
	startTime = 0.0;
	maxSegSize = 50; // 50 secondes max par seg -> dès atteints, on tente de couper si inter-word gap > minDiff
	minDiff = .300;

	CTMfile B;
	fullmode = true;

	if (!B.open(filename)) {
		throw agfio::LoadError("CTM: can't open " + filename);
	}

	list<AGId> res;
	tag::DataModel* data = NULL;
	bool do_del = false;

	try {
		string addr= "";
		corpusName = "";
		map<string,string>::iterator it;

		if ( (it = options->find("&datamodel")) != options->end() )  addr = it->second;
		if ( (it = options->find("corpusName")) != options->end() )  corpusName = it->second;
		if ( (it = options->find("fullmode")) != options->end() )  fullmode = (it->second == "true" || it->second == "1");

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

		data->setInhibateChecking(true);
		data->setGraphType("transcription_graph");
		while (B.read_record()) {
			if ( B.get_type() == "turn" ) {
				processTurn(B, data);
			} else { // unexpected record type
				throw agfio::LoadError("CTM: invalid record found at line " + B.get_lineno() + " when reading " + filename);
			}
		}
		// set signal duration (which also sets last anchor offset)
		data->setSignalDuration(startTime);
		data->setInhibateChecking(false);
		data->setGraphType("");

		data->updateVersionInfo("CTM2TAG", "1");
		res.push_back(data->getAGTrans() );
	}
	catch (const char* msg) {
		throw agfio::LoadError("\nCTM format: " + string(msg));
	}
	catch (const string& msg) {
		throw agfio::LoadError("\nCTM format: " + msg);
	}
	catch (AGException& e) {
		throw agfio::LoadError("\nCTM format: " + e.error());
	}

	if (data)
		data->setImportWarnings(import_warning) ;

	if ( do_del )
		delete data;

	return res;
}

// do some inits
void CTM::initialize(CTMfile& B, tag::DataModel* data)
{
	map<string,string>::iterator it;
	lang = "unk";
	conventions=DEFAULT_CONVENTIONS;
	minTurnSize = MIN_TURN_SIZE;
	minSegmentSize = MIN_SEG_SIZE;

	// some options may be passed as options by calling program
	string item;

	if ( (it = p_options->find("min_turn_size")) != p_options->end() )  {
		minTurnSize = atof(it->second.c_str());
		if ( minTurnSize <= 0 )
			minTurnSize = MIN_TURN_SIZE;
	}
	if ( (it = p_options->find("min_segment_size")) != p_options->end() )  {
		minSegmentSize = atof(it->second.c_str());
		if ( minSegmentSize <= 0 )
			minSegmentSize = MIN_SEG_SIZE;
	}

	if ( (it = p_options->find("lang")) != p_options->end() )  item = it->second;
	if ( item.length() == 2 ) {
		const char* pl = ISO639::get3LetterCode(item.c_str());
		if ( pl != NULL ) lang = pl;
	} else if (item.length() == 3) { // check validity
		const char* pl = ISO639::get2LetterCode(item.c_str());
		if ( pl != NULL ) lang=item;
	}
//	if (lang == "" ) {
//		Log::err() << _("Unknown transcription language, using 'eng' as default");
//		lang = "eng";
//	}
	if ( (it = p_options->find("conventions")) != p_options->end() ) conventions = it->second;

	// configure data model
	try {
		data->setConventions(conventions, lang, fullmode);
	} catch ( const char* msg ) {
		Log::err() << msg << endl;
		data->setConventions("", "", fullmode);
	}


	data->initAGSet(corpusName);
	addSignal(data, 1);

	// initialize transcription graph
	data->initAnnotationGraphs("", lang, "ctm2tag");

	item = B.get_item("author");
	if ( item.empty() ) item = "(unknown)";
	string date = B.get_item("date");
	if ( date.empty() ) date = "(unknown)";
	string comment = "original CTM file format";
	data->initVersionInfo(item, date, comment);

	init_done=true;
}


void CTM::addSignal(tag::DataModel* data, int notrack)
{
	data->addSignal(signalFilename, "audio", "", "", notrack);
	wordId.push_back("") ;
	segmentId.push_back("") ;
	turnId.push_back("") ;
	sectId.push_back("") ;
	prev_spkid.push_back("") ;
}


/**
*  processTurn : add new speech turn to data model
*/
void CTM::processTurn(CTMfile& B,
	       tag::DataModel* data)
{
	signalFilename = B.get_matched(1);
	bool first = true ;
	if ( !init_done )
		initialize(B, data);
	else
		first = false ;

	//int notrack = atoi(B.get_matched(2).c_str()) - 1;
	int notrack = getNoTrack( B.get_matched(2) ) ;

	if ( notrack >= data->getNbTracks() )
		addSignal(data, notrack);

	float start = atof(B.get_matched(3).c_str());
	float duration = atof(B.get_matched(4).c_str());
	float end = start + duration ;
	//float end = atof(B.get_matched(4).c_str());

	bool nontrans = false ;
	bool empty_seg = false ;
	string new_sect("");

	//> CTM format has no speaker, create a default one
	string speaker = _("speaker") ;
	string spkid = speaker + "1" ;
	checkSpeakerId(spkid, "", data) ;

	//> FILL DATAMODEL FOR CAUGHT TURN
	if ( start == 0.0 ) {
		//> first turn, get basis section/turn/seg
		wordId[notrack] = data->getByOffset(data->mainstreamBaseType(), 0.0, notrack);
		segmentId[notrack] = data->getParentElement(wordId[notrack]);
		turnId[notrack] =  data->getParentElement(segmentId[notrack]);
		sectId[notrack] =  data->getParentElement(turnId[notrack]);
		data->setElementProperty(turnId[notrack], "speaker", spkids[spkid]);
		//> CTM format has no section, create section trans by default
		data->setElementProperty(sectId[notrack], "type", "report");
		prev_spkid[notrack] = spkid ;

	}
	else {
		//> First turn found but doesn't start at 0.00 : add an empty seg.
		if (first) {
			//> first turn, get basis section/turn/seg
			wordId[notrack] = data->getByOffset(data->mainstreamBaseType(), 0.0, notrack);
			segmentId[notrack] = data->getParentElement(wordId[notrack]);
			turnId[notrack] =  data->getParentElement(segmentId[notrack]);
			sectId[notrack] =  data->getParentElement(turnId[notrack]);
			//> here start isn't 0.0, so from 0.0 to real start, no speaker and no trans
			data->setElementProperty(turnId[notrack], "speaker", nospeech);
			data->setElementProperty(sectId[notrack], "type", "nontrans");
			prev_spkid[notrack] = nospeech ;
			//> from real start to next time, trans is report
			new_sect = "report";
		}

		bool has_gap = false;
		float diff = (start - startTime);
		//> If gap between previous time and start
		if ( !first
				&& ( diff > MIN_RESOL
						|| ( (diff > minDiff) && (start - data->getStartOffset(segmentId[notrack])) > maxSegSize)) ) {
			has_gap = true;
			wordId[notrack] = data->insertMainstreamBaseElement(wordId[notrack], startTime, false, false);
			//> If gap between previous time and start can be a segment, create an empty one
			if ( (start - startTime) > MIN_SEG_SIZE ) {
				segmentId[notrack] = data->insertParentElement(wordId[notrack], false);
				//> if it can be a turn, create a nospeech one
				if ( start - startTime > minTurnSize ) {
					data->setSpeakerHint(nospeech, notrack);
					turnId[notrack] = data->insertParentElement(segmentId[notrack], false);
					prev_spkid[notrack] = nospeech;
				}
			}
		}

		wordId[notrack] = data->insertMainstreamBaseElement(wordId[notrack], start, false, false);

		if ( has_gap ||  prev_spkid[notrack] != spkid  || need_new_segment ) {
			//> create segment for corresponding text found
			segmentId[notrack] = data->insertParentElement(wordId[notrack], false);
			need_new_segment = false;

			//> change of speaker
			if ( prev_spkid[notrack] != spkid ) {
				//> if non-empty segment | previous speaker is a nospeech | turn duration is ok, create new turn
				if ( ! (empty_seg && spkids.find(prev_spkid[notrack]) != spkids.end() && ((end - start) < MIN_TURN_SIZE)) ) {
					data->setSpeakerHint((empty_seg || nontrans ? nospeech : spkids[spkid]), notrack);
					turnId[notrack] = data->insertParentElement(segmentId[notrack], false);
				}
			}
		}
	}

	// insert new section or set current section type
	if ( ! new_sect.empty() ) {
		data->setHint("section", "type", new_sect);
		sectId[notrack] = data->insertParentElement(turnId[notrack], false);
	}

	startTime = end;
	prev_spkid[notrack] = spkid;


	// finally add text to current text segment
	if ( ! (nontrans || empty_seg) ) {
		processText(B, data, notrack);
	}

}


void CTM::processText(CTMfile& B,
	       tag::DataModel* data, int notrack)
{
	string text = B.get_matched(5);

	//> check encoding
	if (!text.empty())
	{
		string encoding = B.get_item("encoding") ;
		int err;
		text = tag::FormatToUTF8::checkUTF8(text, encoding, false, err) ;
	}

	need_new_segment = (text == "<s/>") ;

	if ( need_new_segment  )
		data->setElementProperty(wordId[notrack], "value", ".");
	else
		data->setElementProperty(wordId[notrack], "value", text);  // put eol instead of <s/>
}

void CTM::checkSpeakerId(const string& spkid, const string& spkinfo,
	       tag::DataModel* data)
{
	if ( spkids.find(spkid) == spkids.end() ) {
		string dicoId = "spk"+StringOps().fromInt(spkids.size()+1);
		tag::Speaker spk ;
		if ( data->getSpeakerDictionary().existsSpeaker(String(dicoId)) ) {
			spk = data->getSpeakerDictionary().getSpeaker(dicoId);
		} else {
			spk = data->getSpeakerDictionary().defaultSpeaker();
			data->getSpeakerDictionary().addSpeaker(spk);
		}
		/*if ( ! spkinfo.empty() && speakerRE->match(spkinfo) )
			spk.setGender(speakerRE->get_matched(1).c_str());*/
		spk.setLastName(spkid);
		data->getSpeakerDictionary().updateSpeaker(spk, false);
		spkids[spkid] = spk.getId();
	}
}

int CTM::getNoTrack(Glib::ustring parsed_track)
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
		else
			return 0 ;
	}
}

