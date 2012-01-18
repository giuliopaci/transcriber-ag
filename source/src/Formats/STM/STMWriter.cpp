/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include <iterator>
#include <algorithm>
#include <set>
#include <vector>
#include <ag/AGAPI.h>
#include <libgen.h>

#include "STMWriter.h"
#include "DataModel/speakers/SpeakerDictionary.h"
#include "DataModel/DataModel.h"
#include "Common/VersionInfo.h"
#include "Common/util/StringOps.h"
#include <glib.h>
#include <iostream>

using namespace tag;


string STMWriter::header () {
	return	";; CATEGORY \"0\" \"\" \"\" \n"
		";; LABEL \"O\" \"Overall\" \"Overall\" \n"
		";; \n"
		";; CATEGORY \"1\" \"Hub4 Focus Conditions\" \"\" \n"
		";; LABEL \"F0\" \"Baseline//Broadcast//Speech\" \"\" \n"
		";; LABEL \"F1\" \"Spontaneous//Broadcast//Speech\" \"\" \n"
		";; LABEL \"F2\" \"Speech Over//Telephone//Channels\" \"\" \n"
		";; LABEL \"F3\" \"Speech in the//Presence of//Background Music\" \"\" \n"
		";; LABEL \"F4\" \"Speech Under//Degraded//Acoustic Conditions\" \"\" \n"
		";; LABEL \"F5\" \"Speech from//Non-Native//Speakers\" \"\" \n"
		";; LABEL \"F6\" \"Overlapping Speech\" \"\" \n"
		";; LABEL \"FX\" \"All other speech\" \"\" \n"
		";; CATEGORY \"2\" \"Speaker Sex\" \"\" \n"
		";; LABEL \"female\" \"Female\" \"\" \n"
		";; LABEL \"male\"   \"Male\" \"\" \n"
		";; LABEL \"unknown\"   \"Unknown\" \"\" \n";
}

string STMWriter::trailer()
{
	return "\n";
}

string STMWriter::noSpeaker()
{
	return "** " + string(_("(No speaker)")) + " **";
}


string STMWriter::section_beg() {
	return "";
}

string STMWriter::section_end() {
	return "" ;
}

string STMWriter::speaker_beg(Speaker::Gender gender) {
	return "" ;
}

string STMWriter::speaker_end() {
	return "\t";
}




int STMWriter::write(ostream& out, DataModel* data)
{
	m_data = data;
	m_graphtype = "transcription_graph";
	m_graphId = m_data->getAG(m_graphtype);
	const vector<string>& mainstream_types = m_data->getMainstreamTypes(m_graphtype);

	int itype;

	// Ecriture du header
	out << header() ;
		for ( itype=0; itype < mainstream_types.size() && mainstream_types[itype] != "turn" ;++itype );

		if ( itype < mainstream_types.size() ) {
			for (m_track=0; m_track < m_data->getNbTracks(); ++m_track ) {
				vector<string> v;
				m_data->getChilds(v, "background", "", m_track);
				// filter "inactive" backgrounds, ie. those with no "type" or "type=none" feature
				vector<string>::iterator itv;
				for (itv = v.begin(); itv != v.end(); itv++) {
					const string& bgtype = m_data->getElementProperty(*itv, "type", "none");
					if ( bgtype != "none" ) {
						BgDef def ;
						def.id = *itv;
						def.type = bgtype;
						def.so = m_data->getStartOffset(*itv);
						def.eo = m_data->getEndOffset(*itv);
						m_backgrounds.push_back(def);
					}
				}
				renderAll(out, mainstream_types, itype, "", m_track);
			}
		}

	out << trailer() << endl;
	return 0;
}


void STMWriter::renderAll(ostream& out, const vector<string>& mainstream_types, int itype, const string& parent, int notrack)
{
	const string& curtype = mainstream_types[itype];
	bool render_next_level = (itype+1) < mainstream_types.size() ;
	vector<string> childs;

	int renderer = 0;
	if ( curtype == "section") renderer = 1;
	else if ( curtype == "turn" ) renderer = 2;
	else if (curtype == "segment" ) renderer = 3;


	//> -- Rendering all mainstreams except mainstream base type
	if ( curtype != m_data->mainstreamBaseType(m_graphtype) )
	{
		m_data->getChilds(childs, curtype, parent, notrack);
		vector<string>::iterator itc;
		for ( itc = childs.begin(); itc != childs.end(); ++itc )
		{
			switch (renderer) {
			case 1:		render_next_level = renderSectionStart(out, *itc); break;
			case 2:		render_next_level = renderTurnStart(out, *itc); break;
			case 3:		render_next_level = renderSegmentStart(out, *itc); break;
			default: 	out << "# No renderer for type " << curtype << endl; break;
			}

			if ( render_next_level )
			{
				if ( notrack == -1 && !parent.empty() )
					notrack = m_data->getElementSignalTrack(parent);

				if ( renderer == 3 ) {
					ostringstream final_text;
					renderAll(final_text, mainstream_types, itype+1, *itc, notrack);
					m_final_text = final_text.str();
				} else {
					renderAll(out, mainstream_types, itype+1, *itc, notrack);
				}
			}
			switch (renderer) {
			case 1:		renderSectionEnd(out, *itc); break;
			case 2:		renderTurnEnd(out, *itc); break;
			case 3:		renderSegmentEnd(out, *itc); break;
			}
		}
	}
	//> -- Rendering mainstream base type
	else
	{
		m_data->getChilds(childs, curtype, parent, notrack);
		vector<string>::iterator itc;
		for ( itc = childs.begin(); itc != childs.end(); ++itc )
		{
			renderBaseElement(out, *itc);
		}
	}
}

bool STMWriter::renderSectionStart(ostream& out, const string& id)
{
	const string& sectType = m_data->getElementProperty(id, "type");

	if ( sectType == "nontrans" )
	{
		out << m_name << " " << m_track+1 << " excluded_region ";
		out << m_data->getElementOffset(id, true) << " " << m_data->getElementOffset(id, false);
		out << " <o,,unknown> ignore_time_segment_in_scoring" << endl;
		return true;
	}
	return false;
}

void STMWriter::renderSectionEnd(ostream& out, const string& id)
{
}

bool STMWriter::renderTurnStart(ostream& out, const string& id)
{
	if ( m_data->getOrder(id) > 0 ) return false;; // overlapping speech

	//> ChECK OVERLAP
	const vector<string>& over = m_data->getElementsWithSameStart(id, "turn") ;
	if ( over.size() > 0 )
		// alors il y a de la parole superposee
	{
		out << m_name << " " << m_track+1 << " excluded_region ";
		out << m_data->getElementOffset(id, true) << " " << m_data->getElementOffset(id, false);
		out << " <o,f6,unknown> ignore_time_segment_in_scoring" << endl;
		return false;
	}

	const string& spkid = m_data->getElementProperty(id, "speaker");
	string spkname= spkid;
	Speaker::Gender gender = Speaker::UNDEF_GENDER;

	m_lab = "";
	m_spkgender = "unknown";
	m_nospeechTurn = ((spkid == "") || ( spkid == tag::Speaker::NO_SPEECH));

	if ( ! m_nospeechTurn ) {
		try
		{
			const Speaker& spk=m_data->getSpeakerDictionary().getSpeaker(spkid);
			spkname = spk.getFullName();
			if ( spkname == m_data->getSpeakerDictionary().getDefaultName(spkid) )
					spkname = spkid;
			else
				for ( int i=0; i<spkname[i]; ++i)
					if ( spkname[i] == ' ' ) spkname[i] = '_';
			gender=spk.getGender();
			m_lab = "f0";	//default speech segment label
			m_spkgender = spk.getGenderStr();
		}
		catch(...) {}
	}

	ostringstream os;
	if( (spkid == "") || ( spkid == tag::Speaker::NO_SPEECH))
		os <<  m_name << " " << m_track+1 << " inter_segment_gap ";
	else
		os <<  m_name << " " << m_track+1 << " " << m_name << "_" << spkname << " ";
	m_prefix = os.str();

	return true;
}

void STMWriter::renderTurnEnd(ostream& out, const string& id)
{
}

bool STMWriter::renderSegmentStart(ostream& out, const string& id)
{
	m_final_text = "";
	m_lab = "";
	m_nospeechSeg = m_nospeechTurn;

	if ( m_nospeechSeg ) return false;

	m_lab = "f0";

	float so = m_data->getStartOffset(id);
	float eo = m_data->getEndOffset(id);

	set<string> bgtypes;

	// find if overlapping backgrounds
	vector<BgDef>::iterator itv;
	for (itv = m_backgrounds.begin(); itv != m_backgrounds.end(); ++itv ) {
		if ( m_data->overlaps(itv->id, so, eo) ) {
			bgtypes.insert(itv->type);
		}
	}
	if ( bgtypes.size() == 1 ) {
		const string& s = *(bgtypes.begin());
		if ( s.find(";") != string::npos )
			m_lab = "fx";
		else if ( s == "music" )
			m_lab = "f3";
		else m_lab = "f4";
	} else if ( bgtypes.size() >= 1 )
		m_lab = "fx";

	return true;
}

void STMWriter::renderSegmentEnd(ostream& out, const string& id)
{
	string labels;
	if ( m_nospeechSeg || m_lab.empty() ) // if no trans event
		labels = "<o,,unknown>";
	else
		labels = "<o," + m_lab + "," + m_spkgender + "> ";

	out << m_prefix << m_data->getStartOffset(id) << " " << m_data->getEndOffset(id) << " " << labels;

	if ( m_lab.empty() )
	{
		out << " ignore_time_segment_in_scoring" << endl;
	}
	else
		out << m_final_text << endl;
}

void STMWriter::renderBaseElement(ostream& out, const string& id)
{
	bool text_type = m_data->isSpeechSegment(id, false);
	const string& value = m_data->getElementProperty(id, "value");
	if ( text_type ) 	{
		renderQualifiersAtStart(out, id, value.empty());
		if ( ! value.empty() )
			out <<  value  << " ";
		renderQualifiersAtEnd(out, id, value.empty());
	} else {
		renderQualifiersAtStart(out, id, false);
		const string& desc = m_data->getElementProperty(id, "desc");
		out << "[" << value;
		if ( !desc.empty() ) out << "=" << desc;
		out << "] ";
		renderQualifiersAtEnd(out, id, value.empty());
	}
}

void STMWriter::renderQualifiersAtStart(ostream& out, const string& id, bool no_text)
{
	vector<string> ids;
	vector<string>::iterator it;

	m_data->getQualifiers(id, ids, "", true, false);
	for ( it=ids.begin(); it != ids.end(); ++it )
	{
		const string& qtype = m_data->getElementType(*it);
		const string& desc = m_data->getElementProperty(*it, "desc");
		bool instantaneous = (m_data->isInstantaneous(*it) || no_text);

		if ( (qtype == "noise") && (desc == "notrans")) {
			m_nospeechSeg = true;
			return;
		}

		out << "[" << qtype;
		if (!desc.empty()) out << "=" << desc;
		if ( !instantaneous ) out << " -] ";
		else out << "] ";
	}
}

void STMWriter::renderQualifiersAtEnd(ostream& out, const string& id, bool no_text)
{
	if ( m_nospeechSeg ) return;

	vector<string> ids;
	vector<string>::iterator it;

	m_data->getQualifiers(id, ids, "", true, false);
	for ( it=ids.begin(); it != ids.end(); ++it )
	{
		const string& qtype = m_data->getElementType(*it);
		const string& desc = m_data->getElementProperty(*it, "desc");
		bool instantaneous = (m_data->isInstantaneous(*it) || no_text);

		if ( !instantaneous ) {
			out << "[-" << qtype ;
			if (!desc.empty()) out << "=" << desc;
			out << "] ";
		}
	}
}
