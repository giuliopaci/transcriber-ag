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

#define _VSFT_IMPL

#include <iostream>
#include <string>
#include <fstream>
#include <ag/AGAPI.h>

#include "Formats/VSFT/VSFT.h"
#include "Common/util/StringOps.h"
#include "DataModel/speakers/Speaker.h"
#include "Common/iso639.h"
#include "Common/globals.h"
#include "Common/FileInfo.h"


#define DEFAULT_CONVENTIONS "transag_default"


#ifdef EXTERNAL_LOAD
list<AGId>
VSFT::plugin_load(const string& filename,
	 const Id& id,
	 map<string,string>* signalInfo,
	 map<string,string>* options)
  throw (agfio::LoadError)
{
  try {
    load(filename, id, signalInfo, options);
  }
  catch (const agfioError& e) {
    throw agfio::LoadError(string("VSFT:") + e.what());
  }
}

string
VSFT::plugin_store(const string& filename,
	  const string& id,
	  map<string,string>* options)
  throw (agfio::StoreError)
{
  try {
    store(filename, id, options);
  }
  catch (const agfioError& e) {
    throw agfio::StoreError(string("VSFT:") + e.what());
  }
}

#endif

VSFT::VSFT()
{
  funcMap["turn"] = &VSFT::processTurn;
  funcMap["text"] = &VSFT::processText;
	try {
 		speakerRE = new RE("\\( *([MF]) *([ -] *(.*) *\\))?");
	} catch (const RE::CompError& e) {
    		throw agfioError(string("VSFT:") + e.what());
	}
}

list<AGId>
VSFT::load(const string& filename,
	  const Id& id,
	  map<string,string>* signalInfo,
	  map<string,string>* options)
  throw (agfio::LoadError)
{
	p_options = options;
	init_done=false;

  VSFTfile B;

  if (!B.open(filename)) {
    throw agfio::LoadError("VSFT:can't open " + filename);
  }
	filesz = FileInfo(filename).size();

  list<AGId> res;
	tag::DataModel* data = NULL;
	bool do_del = false;
	fullmode = true;

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
	} else {
		data = new tag::DataModel();
		data->setKeepAG(true); // do not delete graph upon datamodel deletion
		do_del = true;
	}

	data->setInhibateSignals(true);

  	while (B.read_record()) {
	    (this->*funcMap[B.get_type()])(B, data);
		if ( elapsed < minsz ) elapsed = minsz;
		m_startTime += elapsed;
	}

	if ( m_segmentId != "" && m_endTime < m_duration ) {
		// add empty segment at end
		m_unitId = data->insertMainstreamBaseElement(m_unitId, m_endTime, true, false);
		if ( (m_endTime - m_duration) >= data->conventions().minSegmentSize("segment",1.0) ) {
			m_segmentId = data->insertParentElement(m_unitId, false);
			if ( (m_endTime - m_duration) >= data->conventions().minSegmentSize("turn",1.0) ) {
				m_turnId = data->insertParentElement(m_segmentId, false);

			}
		}
//ICI NOSPEECH

		data->insertParentElement(m_turnId, "nontrans" , false);
	}

	data->updateVersionInfo("VSFT2TAG", "1");
	data->setInhibateSignals(false);

  	res.push_back(data->getAGTrans() );
  }
  catch (const char* msg) {
    throw agfio::LoadError("VSFT:" + string(msg));
  }
  catch (const string& msg) {
    throw agfio::LoadError("VSFT:" + msg);
  }

	if (data)
		data->setImportWarnings(import_warning) ;

	if ( do_del ) delete data;

  return res;
}

// do some inits
void VSFT::initialize(VSFTfile& B, tag::DataModel* data)
{
	map<string,string>::iterator it;
	m_startTime = 0.0;
	m_endTime = 0.0;
	m_duration = 0.0;
	lang = "";
	conventions=DEFAULT_CONVENTIONS;
	minsz = 0.5;  // min seg size = 1 second.
	ok_duration = false;
	do_pi = false;
	signalFormat="";
	signalEncoding="";
	signalNbTracks=1;
	signalFilename = "";

	// some options may be passed as options by calling program
	string item;

	if ( (it = p_options->find("start")) != p_options->end() ) item = it->second;
	else item=B.get_item("start");
	if ( ! item.empty() ) m_startTime=StringOps(item).parseTimeStr();

	if ( (it = p_options->find("end")) != p_options->end() ) item=it->second;
	else item=B.get_item("end");
	if ( ! item.empty() ) m_endTime=StringOps(item).parseTimeStr();

	if ( (it = p_options->find("duration")) != p_options->end() ) item=it->second;
	else item=B.get_item("duration");
	if ( ! item.empty() ) m_duration=StringOps(item).parseTimeStr();

	if ( m_endTime > m_duration ) {
		Log::err() << _("Transcription end time set to signal duration ") << duration << " secs" << endl;
		m_endTime = m_duration;
	} else if ( m_endTime == 0 ) m_endTime = m_duration;

	if ( (it = p_options->find("lang")) != p_options->end() )  item = it->second;
	else item=B.get_item("lang");
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

	if ( (it = p_options->find("signalFile")) != p_options->end() ) signalFilename = it->second;
	else signalFilename=B.get_item("signalFile");

	if ( (it = p_options->find("signalFormat")) != p_options->end() ) signalFormat = it->second;
	if ( (it = p_options->find("signalEncoding")) != p_options->end() ) signalEncoding = it->second;
	if ( (it = p_options->find("signalNbTracks")) != p_options->end() ) signalNbTracks = atoi(it->second.c_str());
	if ( (it = p_options->find("conventions")) != p_options->end() ) conventions = it->second;
	if ( (it = p_options->find("with_pi")) != p_options->end() ) do_pi=true;
	if ( signalFilename == "" ) {
		FileInfo info(B.get_filename());
		info.setTail("wav");
		signalFilename = info.Basename();
	}

	// configure data model
	try {
		data->setConventions(conventions, lang, fullmode);
	} catch ( const char* msg ) {
		Log::err() << msg << endl;
		data->setConventions("", "", fullmode);
	}

	data->initAGSet(corpusName);
	data->addSignal(signalFilename, "audio", signalFormat, signalEncoding, signalNbTracks);
	// initialize transcription graph
	data->initAnnotationGraphs("", lang, "fft2tag");
	data->setSignalDuration(m_duration);

//Log::err() << " SIGNAL = " << signalFilename << " m_duration=" << m_duration << " start=" << m_startTime << " end=" << m_endTime << endl;
	item = B.get_item("author");
	if ( ! item.empty() ) {
		string comment = "original VSFT (Vecsys fast transcript) file format";
		data->initVersionInfo(item, B.get_item("date"), comment);
	}

	// final inits
	m_segmentId = "";
	m_turnId = "";
	add_nospeech_sect=false;
	elapsed=0;;
	prev_is_pi = false;
	prev_spkid = "";

	init_done=true;
}

/**
*  processTurn : add new speech turn to data model
*/
void
VSFT::processTurn(VSFTfile& B,
	       tag::DataModel* data)
{
	if ( !init_done ) initialize(B, data);

	string spkid = B.get_matched(1);
	string spkinfo = B.get_matched(2);
	string text = B.get_matched(3);

	if ( m_spkids.find(spkid) == m_spkids.end() ) {
		string dicoId = "spk"+spkid;
		tag::Speaker spk ;
		if ( data->getSpeakerDictionary().existsSpeaker(String(dicoId)) ) {
			spk = data->getSpeakerDictionary().getSpeaker(dicoId);
		} else {
			spk = data->getSpeakerDictionary().defaultSpeaker();
			data->getSpeakerDictionary().addSpeaker(spk);
		}
		// parse speaker info
		if ( ! spkinfo.empty() ) {
			if ( speakerRE->match(spkinfo) ) {
				string gender = speakerRE->get_matched(1);
				spk.setGender(gender.c_str());
				string name = speakerRE->get_matched(3);
				if ( !name.empty() ) {
					unsigned long pos;
					if ( (pos=name.find(' ')) != string::npos ) {
						spk.setFirstName(name.substr(0,pos));
						spk.setLastName(name.substr(pos+1));
					} else
						spk.setLastName(name);
				}
			}
		}
		data->getSpeakerDictionary().updateSpeaker(spk, false);
		m_spkids[spkid] = spk.getId();
	}

	// insert speech segment
	// create turn
	if ( m_unitId == "" && m_startTime > 0.0 ) {
		// let no speaker turn at file start
		m_unitId = data->getByOffset(data->mainstreamBaseType(), 0.0, 0);
		m_segmentId = data->getParentElement(m_unitId);
		m_turnId =  data->getParentElement(m_segmentId);
		m_dataModel.setElementProperty(m_turnId, "speaker", Speaker::NO_SPEECH, false);
		if ( m_startTime > 30.) { // add section
			add_nospeech_sect=true;
		}
	}

	if ( m_unitId == "" ) {
		// set segment text
		m_unitId = data->getByOffset(data->mainstreamBaseType(), 0.0, 0);
		m_segmentId = data->getParentElement(m_unitId);
		m_turnId =  data->getParentElement(m_segmentId);
		data->setElementProperty(m_turnId, "speaker", m_spkids[spkid], false);
	} else {
		if ( ! prev_is_pi || prev_spkid != spkid ) {
			m_unitId = data->insertMainstreamBaseElement(m_unitId, m_startTime, false, false);
			m_segmentId = data->insertParentElement(m_unitId, false);
			m_turnId = data->insertParentElement(m_segmentId, false);
			data->setElementProperty(m_turnId, "speaker", m_spkids[spkid], false);
		}
		if ( do_pi && text == "&" ) {
			// add [pron_pi] to current segment start
			string qid = data->addQualifier("pronounce", m_unitId, "", "unintelligible", false);
			text="";
			prev_is_pi = true;

		} else prev_is_pi = false;
	}

	prev_spkid = spkid;

	if ( !text.empty() ) {
		data->setElementProperty(m_unitId, "subtype", "unit_text", false);
		data->setElementProperty(m_unitId, "value", text, false);
		// compute avg turn length
		elapsed =  (m_endTime-m_startTime) * ((float)(B.get_line().length())/filesz);
	} else elapsed = 0.0;

	if ( add_nospeech_sect )  {
		string secId =  data->getParentElement(m_turnId);
		data->setElementProperty(secId, "type", "nontrans", false);
		data->insertParentElement(m_turnId, "report" , false);
		add_nospeech_sect = false;
	}
}


void VSFT::processText(VSFTfile& B,
	       tag::DataModel* data)
{
	if ( m_segmentId == "" ) {
	    throw agfio::LoadError(string("VSFT:") + string("no turn found before line ") + B.get_lineno());
	}
	// plain text to be added to previous segment
	string curtext = data->getElementProperty(m_segmentId, "value");
	string record_line = B.get_line();
	if ( ! isspace(record_line[0]) )
		curtext += " ";
	curtext += StringOps(record_line).trim();

	//> check encoding
	if (!curtext.empty())
	{
		string encoding = B.get_item("encoding") ;
		if (!g_utf8_validate(curtext.c_str(), -1, NULL) && !encoding.empty())
		{
			try {
				Glib::ustring converted = Glib::convert_with_fallback(curtext, "UTF-8", encoding, "<?>") ;
				curtext = converted ;
			}
			catch (Glib::Error e) {

				string msg ;

				if (import_warning.empty())
					msg = "\n" + string(_("ENCODING Problem")) + "\n\n" ;

				msg = msg + string(_("Can't correctly import characters codeset")) + "\n" ; ;
				import_warning.push_back(msg) ;
				TRACE_D << "VSFT import: " << msg << "\n\t" << e.what() << std::endl ;
			}
		}
	}

	data->setElementProperty(m_unitId, "subtype", "unit_text", false);
	data->setElementProperty(m_unitId, "value", curtext, false);
	elapsed =  (m_endTime - m_startTime) * ((float)(record_line.length())/filesz);
}

