/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/*
 * CHATWriter.cpp: CHILDES CHAT native format : class definition
 */

#include <iterator>
#include <algorithm>
#include <set>
#include <vector>
#include <ag/AGAPI.h>
#include <libgen.h>

#include "CHATWriter.h"
#include "DataModel/speakers/SpeakerDictionary.h"
#include "Common/VersionInfo.h"
#include "Common/util/StringOps.h"
#include <glib.h>

using namespace tag;


// --- HasOverlappingSegments ---
bool CHATWriter::hasOverlappingSegments(DataModel* data, SignalSegment& s)
{
	set<string> v;
	set<string>::iterator it;

	if ( data->conventions().hasGraphType("background_graph") )
		data->getOverlappingSegmentsIds(s, v, "background") ;

	for(it = v.begin(); it != v.end() ; it++)
	{
		if(*it != s.getId())
		{
			return true;
		}
	}
	return false;
}

// get the 3 last characters of a speaker ID to respect the CHAT format
string CHATWriter::formatSPKid(string spkID)
{
	int spkLength = spkID.length();

	if ( ( spkLength == 0 ) || ( spkID == "no_speech" ) )
		return "NSP"; // no speaker
	if ( spkLength > 2 )
		return StringOps( spkID.substr(spkLength - 3, spkLength) ).toUpper();
	else
		return spkID;
}


// --- GetParticipants ---
string CHATWriter::getParticipants(DataModel* data)
{
	string participants = "";
	SpeakerDictionary dict = data->getSpeakerDictionary();

	// -- Browsing Speakers --
	map<string, Speaker>::iterator it;

	for(it = dict.begin(); it != dict.end(); it++)
	{
		participants += formatSPKid(it->first) + " ";

		Speaker spk = it->second;
		participants += StringOps(spk.getFirstName() + "~" + spk.getLastName()).replace(" ", "_", true) + " ";
	
		// -- Gender --
		string spk_gender = spk.getGenderStr();
		if (spk_gender == "")
			participants +=	"unidentified";
		else
			participants +=	spk_gender;

		participants += ",";
	}

	// -- Chop Last Char --
	if (participants[ participants.size() - 1 ] == ',')
		participants.erase( participants.size() - 1, 1 );

	return participants;
}


// --- GetParticipantsIDs ---
string CHATWriter::getParticipantsIDs(DataModel* data)
{
	string part_ids = "";
	SpeakerDictionary dict = data->getSpeakerDictionary();

	// -- Browsing Speakers --
	map<string, Speaker>::iterator it;
	

	for(it = dict.begin(); it != dict.end(); it++)
	{
		Speaker spk = it->second;

		part_ids += "@ID: ";
		part_ids += data->getTranscriptionLanguage().substr(0, 2) + "||";	// no corpus label

		part_ids += formatSPKid(it->first) + "||";	// no age specified

		if (spk.getGenderStr() == "")
			part_ids +=	"unidentified";
		else
			part_ids +=	spk.getGenderStr();

		part_ids += "|||||\n";
	}

	return part_ids;
}


// --- ProcessSegmentText ---
string CHATWriter::processSegmentText(DataModel* data, string segmentID)
{
	string partial_text = "";
	string text			= "";
	string lab			= "";
	string final_text	= "";
	
	vector<string> vs2;
	vector<string> v3;
	vector<string>::iterator is2;
	vector<string>::iterator it3;

	// -- Head Qualifiers --
	data->getQualifiers(segmentID, v3, "", true, false);

	// -- Text grabbed from segment inner units --
	data->getChilds(vs2, "unit", segmentID, trackID);

	for(is2 = vs2.begin(); is2 != vs2.end(); ++is2)
		text += data->getElementProperty(*is2, "value");

	for(it3 = v3.begin(); it3 != v3.end(); ++it3)
	{
		partial_text += "[" + data->getElementType(*it3);
		const string& desc = data->getElementProperty(*it3, "desc");
		// check if instantaneous or start of event
		const string& start_id = data->getMainstreamStartElement(*it3);
		bool instantaneous = data->isInstantaneous(*it3);

		if (!instantaneous)
		{
			const string& end_id = data->getMainstreamEndElement(*it3);
			if ( end_id.empty() || (start_id == end_id) )
			{
				string text =  data->getElementProperty(start_id, "value");
				instantaneous = text.empty() ;
			}
		}

		if(!desc.empty())
		{
			partial_text +=  "=" + desc;
		}
	
		if (!instantaneous)
			partial_text += "-] ";
		else
			partial_text += "] ";
	}

	partial_text += text;

	// -- Tail Qualifiers --
	data->getQualifiers(segmentID, v3, "", false, false);
	for(it3 = v3.begin(); it3 != v3.end(); ++it3)
	{
		// check if instantaneous or start of event
		const string& start_id = data->getMainstreamStartElement(*it3);
		const string& end_id = data->getMainstreamEndElement(*it3);
		bool instantaneous = false;
		if ( end_id.empty() || (start_id == end_id) )
		{
			string text =  data->getElementProperty(start_id, "value");
			instantaneous = text.empty() ;
		}
		
		if (!instantaneous)
		{
			partial_text += " [-" + data->getElementType(*it3);
			const string& desc = data->getElementProperty(*it3, "desc");
	
			if(!desc.empty())
				partial_text += "=" + desc;
			
			partial_text += "] ";
		}
	}

	final_text += partial_text;

	return final_text;
}


// --- Write ---
int CHATWriter::write(ostream& out, DataModel* data, const string& name)
{
	vector<SignalSegment> v1;
	vector<SignalSegment> v2;
	vector<string> vs1;
	vector<string> v3;
	vector<SignalSegment>::iterator it0;
	vector<SignalSegment>::iterator it1;
	vector<SignalSegment>::iterator it2;
	vector<string>::iterator is1;
	vector<string>::iterator it3;

	// -- AG Properties --
	std::map<string, string> agProps = data->getAGSetProperties();
	std::map<string, string>::iterator it;

	bool overlap = false;
	bool already_done = false;

	string track="";
	string defname = SpeakerDictionary::defaultFormat;
	unsigned long pos = defname.find("%");
	if ( pos != string::npos ) defname.erase(pos);


	// -- CHAT export only turn / segments --
	data->getSegments("turn", v1, 0.0, 0.0, trackID);

	// -- CHAT : Headers --
	out << "@Begin" << endl;
	out << "@Languages: " << data->getTranscriptionLanguage().substr(0, 2) << endl;	// 2-letter code
	out << "@Participants: " << getParticipants(data) << endl;
	out << getParticipantsIDs(data);

	// -- TrackID --
	if (trackID != -1)
		out << "@Track: " << trackID << endl;

	// -- Segment Parsing --
	for(it1 = v1.begin(); it1 != v1.end(); ++it1)
	{
		double start_backup = -1;
		string final_text = "";
		string labels = "";
		string spkgender;

		StringOps(track).fromInt(1 + it1->getTrack());

		// -- Track ID check --
		if ( (trackID != -1) && (it1->getTrack() != trackID) )
			continue;

		v2.clear();
		string spkid = data->getElementProperty(it1->getId(), "speaker");
		string spkname=spkid;
		try
		{
			const Speaker& spk=data->getSpeakerDictionary().getSpeaker(spkid) ;
			spkgender = spk.getGenderStr();
			spkname = spk.getFullName();
			if ( spkname.compare(0, defname.length(), defname) == 0 )
				spkname = spkid;
			else
				for ( int i=0; i<spkname[i]; ++i)
					if ( spkname[i] == ' ' ) spkname[i] = '_';
		}
		catch(...)
		{
			spkgender = "unknown";
		}
		string segment1 = "";
		
		// New algo 
		vs1.clear();
		data->getChilds(vs1, "segment", it1->getId(), trackID);


		for(is1 = vs1.begin(); is1 != vs1.end(); ++is1)
		{
			// -- Loop through units to obtain text --
			final_text += processSegmentText(data, *is1);

			// -- Offsets --
			float s_Offset = data->getElementOffset(*is1, true);
			float e_Offset = data->getElementOffset(*is1, false);

			// -- Anchored Segments --
			if (s_Offset >= 0)
			{
				if (e_Offset >= 0)
				{
					startOffset = (int)(s_Offset * 1000.0);
					endOffset	= (int)(e_Offset * 1000.0);

					// -- Header / Text / Timecode --
					out << "*" << formatSPKid(spkid) << ": ";
					
					// -- Lazy Overlap Marker : +< --
					if (data->getOrder(*is1) > 0)
						out << "+< ";

					out << final_text;
					out	<< " " << startOffset << "_" << endOffset << endl;
					
					// -- Reset --
					final_text = "";
					labels = "";
					start_backup = -1;
				}
				else
				{
					start_backup = s_Offset;
					final_text += " ";
				}
			}
			else
			{
				// -- Non-anchored --
				if(e_Offset >= 0)
				{
					startOffset = (int)(s_Offset * 1000.0);
					
					if (startOffset == -1000)
						startOffset = endOffset;	// previous offset end
				
					endOffset	= (int)(e_Offset * 1000.0);
				

					// -- Header / Text / Timecode --
					out << "*" << formatSPKid(spkid) << ": ";
					
					// -- Lazy Overlap Marker : +< --
					if (data->getOrder(*is1) > 0)
						out << "+< ";
					
					out << final_text;
					out	<< " " << startOffset << "_" << endOffset << endl;

					// -- Reset --
					start_backup = -1;
					labels = "";
					final_text = "";
				}
				else
				{
					final_text += " ";
				}
			}
		}
	}

	// -- CHAT : End Footer --
	out << "@End" << endl;

	return 0;
}

