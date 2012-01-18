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

#include "HTMLWriter.h"
#include "Common/VersionInfo.h"
#include "Common/util/Utils.h"
#include "Common/util/StringOps.h"
#include "Common/util/FormatTime.h"
#include "DataModel/speakers/Speaker.h"
#include "DataModel/speakers/SpeakerDictionary.h"
#include <glib.h>

using namespace tag;


string HTMLWriter::header() {
	return "<style type=\"text/css\">\n"
	"td\n"
	"{\n"
	"/*	background-color: #EBEBEB ;\n*/"
	"}\n\n"	
	"#Section\n"
	"{\n"
	"	background-color: #CD5C5C ;\n"
	"	color: black ;\n"
	"	font-weight: bold\n"
	"}\n\n"
	"#Speaker\n"
	"{\n"
	"	background-color: #A2A2D5 ;\n"
	"	color: #000000 ;\n"
	"	font-weight: bold\n"
	"}\n\n"
	"#NoSpeaker\n"
	"{\n"
	"	color: #615050;\n"
	"	background-color: #bfbfbf;\n"
	"	font-weight: bold\n"
	"}\n\n"
	"#MaleSpeaker\n"
	"{\n"
	"	color: #8A0A0A;\n"
	"	background-color: #B2C0F3;\n"
	"	font-weight: bold\n"
	"}\n\n"
	"#FemaleSpeaker\n"
	"{\n"
	"	color: #8A0A0A;\n"
	"	background-color: #D5AEE2;\n"
	"	font-weight: bold\n"
	"}\n\n"
	"#track1\n"
	"{\n"
	"	float:left;\n"
	"	width:49%;\n"
	"	padding: 8px;\n"
	"/*	color: #B3B3B3;\n*/"
	"	background-color: #EBEBEB;\n"
	"	border: 1px solid black;\n"
	"}\n\n"
	"#track2\n"
	"{\n"
	"	margin-left: 51%;\n"
	"	width:50%;\n"
	"	padding: 8px;\n"
	"/*	color: #B3B3B3;\n*/"
	"	background-color: #EBEBEB;\n"
	"	border: 1px solid black;\n"
	"}\n\n"
	"</style>\n\n"
	"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\n"
	"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
	"<head>\n"
	"\t<title>TranscriberAG</title>\n"
	"\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />"
	"</head>\n"
	"<body>\n";
}

string HTMLWriter::trailer() {
	return "</body>\n</html>";
}

string HTMLWriter::noSpeaker() {
	return "<br/><font id=\"NoSpeaker\">&nbsp &nbsp " +
		string(_("(No speaker)")) +
		"&nbsp &nbsp </font>";
}

string HTMLWriter::section_beg() {
	return "<br/><br/><div align=\"center\"><font id=\"Section\">&nbsp &nbsp ";
}

string HTMLWriter::section_end() {
	return "&nbsp &nbsp </div></font>\n";
}

string HTMLWriter::speaker_beg(Speaker::Gender gender) {
	switch ( gender ) {
	case Speaker::MALE_GENDER :
		return "<br/>\n<font id=\"MaleSpeaker\">&nbsp &nbsp ";
	case Speaker::FEMALE_GENDER :
		return "<br/>\n<font id=\"FemaleSpeaker\">&nbsp &nbsp ";
	default:
		return  "<br/>\n<font id=\"Speaker\">&nbsp &nbsp ";
	}
}

string HTMLWriter::speaker_end() {
	return "&nbsp &nbsp </font>\n" ;
}


int HTMLWriter::write(ostream& out, DataModel* data)
{
	m_data = data;
	m_graphtype = "transcription_graph";
	m_graphId = m_data->getAG(m_graphtype);
	const vector<string>& mainstream_types = m_data->getMainstreamTypes(m_graphtype);


	// Ecriture du header
	out << header() << endl;

	// Ecriture des propriétés du tag dans un tableau
	const std::map<string, string>& items = m_data->getAGSetProperties();
	std::map<string, string>::const_iterator it;

	out << "<table summary=\"TransAGProperties\" border>" << endl << "\t<tbody>" << endl;
	out << "<br />" << endl;

	// NOMBRE DE PISTE
	int nb_tracks = m_data->getNbTracks();
	out << "\t\t<tr>" << endl << "\t\t\t<td><strong> Nombre de Pistes </strong></td>" << endl;
	out << "\t\t\t<td> " << nb_tracks << " </td>" << endl << "\t\t</tr>" << endl;


	// FICHIER TRAITE
//	out << "\t\t<tr>" << endl << "\t\t\t<td><strong> Fichier traité </strong></td>" << endl;
//	out << "\t\t\t<td> " << m_data->getPath() << " </td>" << endl << "\t\t</tr>" << endl;


	// DUREE
	out << "\t\t<tr>" << endl << "\t\t\t<td><strong> Durée </strong></td>" << endl;
	out << "\t\t\t<td> " << FormatTime::FormatTime(m_data->getSignalDuration(), true, true);
	out << " </td>" << endl << "\t\t</tr>" << endl;


	// NB DE SPEAKER
//	out << "\t\t<tr>" << endl << "\t\t\t<td><strong> Nombre de Speakers </strong></td>" << endl;
//	out << "\t\t\t<td> " << m_data->getSpeakerDictionary().size() << " </td>" << endl << "\t\t</tr>" << endl;


	for (it= items.begin(); it != items.end(); ++it)
	{
		// LANGUE DU TEXTE
		if (it->first == "lang") {
			out << "\t\t<tr>" << endl << "\t\t\t<td><strong> " << it->first << " </strong></td>" << endl;
			out << "\t\t\t<td> " << it->second << " </td>" << endl << "\t\t</tr>" << endl;
		}
	}
	out << "\t</tbody>" << endl << "</table>" << endl << "<br />" << endl << "<br />" << endl;

	int itype = 0;

	if ( nb_tracks == 1 )
	{
		//MONO


		for ( itype=0;
				itype < mainstream_types.size() && ! m_data->hasElementsWithType(mainstream_types[itype], m_graphId);
				++itype);  // eventually skip missing top levels
		if ( itype < mainstream_types.size() )
			renderAll(out, mainstream_types, itype, "", 0);
		else
			out << "<br/> NO ELEMENT TO PRINT !! <br/>" << endl;
	}
	else
	{
		//STEREO -> don't render eventual sections, and interlace turns


		for ( itype=0; itype < mainstream_types.size() && mainstream_types[itype] != "turn" ;++itype );
		if ( itype < mainstream_types.size() ) {

			int notrack;
			for (notrack=0; notrack < m_data->getNbTracks(); ++notrack ) {
				out << "<div id=track" << notrack << ">" << endl;
				renderAll(out, mainstream_types, itype, "", notrack);
				out << endl << "</div>" << endl << endl;
			}
		}
	}

	out << trailer() << endl;
	return 0;
}

void HTMLWriter::renderAll(ostream& out, const vector<string>& mainstream_types, int itype, const string& parent, int notrack)
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
			case 1:		renderSectionStart(out, *itc); break;
			case 2:		renderTurnStart(out, *itc); break;
			case 3:		renderSegmentStart(out, *itc); break;
			default: 	out << "<br/>No renderer for type " << curtype << "<br/>" << endl; break;
			}

			if ( render_next_level )
			{
				if ( notrack == -1 && !parent.empty() )
					notrack = m_data->getElementSignalTrack(parent);
				renderAll(out, mainstream_types, itype+1, *itc, notrack);
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

void HTMLWriter::renderSectionStart(ostream& out, const string& id)
{
	const string& sectTopicID = m_data->getElementProperty(id, "topic");
	const string& sectDesc = m_data->getElementProperty(id, "desc");

	out << endl << endl << section_beg() << m_data->getElementProperty(id, "type")	;
	if (sectDesc != "")
		out << " - " << sectDesc;

	if (sectTopicID != "")
	{
		Topic* topic =  Topics::getTopicFromAll(sectTopicID, m_data->conventions().getTopics());
		out << " {" << topic->getLabel() << "} ";
	}

	out << section_end() << endl;
}

void HTMLWriter::renderSectionEnd(ostream& out, const string& id)
{
}

void HTMLWriter::renderTurnStart(ostream& out, const string& id)
{
	const string& spkid = m_data->getElementProperty(id, "speaker");
	string spkname= spkid;
	Speaker::Gender gender = Speaker::UNDEF_GENDER;
	try
	{
		const Speaker& spk=m_data->getSpeakerDictionary().getSpeaker(spkid);
		spkname = spk.getFullName();
		gender=spk.getGender();
	}
	catch(...) {}

	if( (spkid == "") || ( spkid == tag::Speaker::NO_SPEECH))
		out << noSpeaker() ;
	else {
		if ( m_data->getOrder(id) > 0 )
			out << endl << "<ul>" << endl ;
		out << speaker_beg(gender) << spkname << speaker_end() <<" ";
	}
}

void HTMLWriter::renderTurnEnd(ostream& out, const string& id)
{
	if ( m_data->getOrder(id) > 0 )
		out << "</ul>" << endl;
	out << endl;
}

void HTMLWriter::renderSegmentStart(ostream& out, const string& id)
{
		out << "</li> " ;

		if (m_printTimeCode)
				out << "{ " << m_data->getStartOffset(id) << " } ";
		else
				out << "{ " << m_data->getEndOffset(id) << " } ";
}

void HTMLWriter::renderSegmentEnd(ostream& out, const string& id)
{
		out << "</li>" << endl;
}

void HTMLWriter::renderBaseElement(ostream& out, const string& id)
{
	bool text_type = (m_data->getElementProperty(id, "subtype") == "unit_text");
	const string& value = m_data->getElementProperty(id, "value");
	if ( text_type ) 	{
		renderQualifiersAtStart(out, id, value.empty());
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

void HTMLWriter::renderQualifiersAtStart(ostream& out, const string& id, bool no_text)
{
	vector<string> ids;
	vector<string>::iterator it;

	m_data->getQualifiers(id, ids, "", true, false);
	for ( it=ids.begin(); it != ids.end(); ++it )
	{
		const string& qtype = m_data->getElementType(*it);
		const string& desc = m_data->getElementProperty(*it, "desc");
		bool instantaneous = (m_data->isInstantaneous(*it) || no_text);

		out << "[" << qtype;
		if (!desc.empty()) out << "=" << desc;
		if ( !instantaneous ) out << " -] ";
		else out << "] ";
	}
}

void HTMLWriter::renderQualifiersAtEnd(ostream& out, const string& id, bool no_text)
{
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
