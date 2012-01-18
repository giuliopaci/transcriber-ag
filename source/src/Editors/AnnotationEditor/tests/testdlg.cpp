/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include <iostream>
#include <gtkmm.h>
#include <libintl.h>
#include <time.h>

#include <map>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "Common/portabilite.h"
#include "SignalSegment.h"
#include "DialogTurnProperties.h"
#include "DialogFileProperties.h"
#include "SAXAnnotationsHandler.h"
#include "Common/Parameters.h"
#include "DialogMore.h"
#include "DialogEditQualifierProperties.h"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLNumber.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <stdio.h>

using namespace std;
using namespace tag;
XERCES_CPP_NAMESPACE_USE

Gtk::Window* mainwin;

int main(int argc, char **argv) {
try {
	bindtextdomain("TranscriberAG", "../../../locales");
//	bindtextdomain("dactilo", "../../../locales");
	g_thread_init(NULL);
	gdk_threads_init();

	Gtk::Main kit(argc, argv);
	
	mainwin = Gtk::manage(new Gtk::Window());

///////////////////////////////////////////////////////////////////////////////
/*
	Parameters* params = new Parameters();
	params->load(string("test2.xml"));
	if (params->existsParameter("annotation_editor", "edit,max")) printf("A oui\n");
	else printf("A non\n");
	printf("B %s\n", params->getParameterLabel("annotation_editor", "display,no").c_str());
	printf("C %s\n", params->getParameterValue("audio_component", "tools,tool2").c_str());
	params->save();
*/
///////////////////////////////////////////////////////////////////////////////

	try {
		XMLPlatformUtils::Initialize ();
	} catch (const XMLException & toCatch) {
		throw "Caught exception when initializing xerces-SAX";
	}

	std::map<string, std::list<Property> > annotationProperties;
	std::list<Property> fileProperties;
	std::map<std::string, std::list<std::string> > choiceLists;

	SAXAnnotationsHandler* handler = new SAXAnnotationsHandler(&annotationProperties, &fileProperties, &choiceLists);

	SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
	parser->setFeature(XMLUni::fgSAX2CoreValidation, false);
	parser->setFeature(XMLUni::fgXercesLoadExternalDTD, false);
	parser->setContentHandler((DefaultHandler*)handler);
	parser->setErrorHandler((DefaultHandler*)handler);

	parser->parse("test1.xml");

	XMLPlatformUtils::Terminate();

printf("----- choiceLists -----\n");
	map<string, list<string> >::iterator it = choiceLists.begin();
	while (it != choiceLists.end()) {
		const char* a = it->first.c_str();
		list<string> b = it->second;
		printf("=> %s\n", a);
		list<string>::iterator it2 = b.begin();
		while (it2 != b.end()) {
			const char* c = it2->c_str();
			printf("+ %s\n", c);
			it2++;
		}
		it++;
	}

printf("----- fileProperties -----\n");
	list<Property>::iterator it3 = fileProperties.begin();
	while (it3 != fileProperties.end()) {
		Property p = *it3;
		if (p.type == PROPERTY_TEXT) printf("+ %s text\n", p.label.c_str());
		else if (p.type == PROPERTY_CHOICELIST) printf("+ %s choice_list %s\n", p.label.c_str(), p.choiceList.c_str());
		it3++;
	}

printf("----- annotationProperties -----\n");
	map<string, list<Property> >::iterator it4 = annotationProperties.begin();
	while (it4 != annotationProperties.end()) {
		const char* a = it4->first.c_str();
		list<Property> b = it4->second;
		printf("=> %s\n", a);
		list<Property>::iterator it5 = b.begin();
		while (it5 != b.end()) {
			Property p = *it5;
			if (p.type == PROPERTY_TEXT) printf("+ %s text\n", p.label.c_str());
			else if (p.type == PROPERTY_CHOICELIST) printf("+ %s choice_list %s\n", p.label.c_str(), p.choiceList.c_str());
			it5++;
		}
		it4++;
	}

///////////////////////////////////////////////////////////////////////////////

	tag::SpeakerDictionary mydic;
	mydic.loadDictionary("../GUI/Arborescence/etage1/speakerdic.xml");

	const std::string* id = new std::string("ceci est un test");
	tag::TurnSegment* ss = new tag::TurnSegment(*id, 12, 13.5, 1);
	ss->setIsSpeech(TRUE);
	ss->setProperty("speaker", "spk2");
	ss->setProperty("language", "ENG");
	ss->setProperty("dialect", "Texan");
	ss->setProperty("speech_type", "Spontaneous");
	ss->setProperty("overlapping_speech", "1");
	ss->setProperty("overlapping_noise", "Music");
	ss->setProperty("environment", "Studio");
	ss->setProperty("no_speech_type", "Noise");

	tag::DialogTurnProperties* dialog1 = new tag::DialogTurnProperties(*mainwin, ss, &mydic, 2, annotationProperties["turn"], choiceLists);
	int r1 = dialog1->run();
	dialog1->hide();
/*
	std::map<string, string> values;
	values["file"] = "/tmp/file.pouf";
	values["corpus"] = "CNN";
	values["version"] = "2.0";
	values["creation_date"] = "11/11/07";
	values["creation_by"] = "Bob ROGER";
	values["creation_comment"] = "ceci est un commentaire";
	values["lastmodification_date"] = "12/12/07";
	values["lastmodification_by"] = "Jean-Claude DUSS";
	values["lastmodification_comment"] = "ceci est également un commentaire";
	values["comment"] = "ça aussi";
	values["property,hop"] = "ceci est un hop";
	values["property,zioup"] = "test2";
*/
/*
	std::map<string, string> signal1;
	signal1["file"] = "/tmp/signal1.wav";
	signal1["format"] = "WAV";
	signal1["channels"] = "Stereo";
	signal1["length"] = "623,1250 sec";
	signal1["type"] = "Telephone";
	signal1["source"] = "CNN";
	signal1["date"] = "10/10/07";
	signal1["comment"] = "ah bah d'accord";
	signal1["Test1"] = "test2";
	list<std::map<string, string> > signals;
	signals.push_back(signal1);
*/
/*
	list<std::map<string, string> > more;
	std::map<string, string> more1;
	more1["date"] = "10/10/07";
	more1["by"] = "Moi";
	more1["wid"] = "F";
	more1["comment"] = "";
	more.push_back(more1);
	std::map<string, string> more2;
	more2["date"] = "11/11/07";
	more2["by"] = "Toi";
	more2["wid"] = "FF";
	more2["comment"] = "ah bah d'accord";
	more.push_back(more2);
*/

	string title = "ceci est un titre";
	string s1 = "t2";
	string s2 = "u34";
	vector<string> p_types;
	p_types.push_back(string("t1"));
	p_types.push_back(string("t2"));
	p_types.push_back(string("t3"));
	vector<string> p_descs;
	p_descs.push_back(string("u1"));
	p_descs.push_back(string("u2"));
	p_descs.push_back(string("u3"));
	tag::DialogEditQualifierProperties* dialog3 = new tag::DialogEditQualifierProperties(*mainwin, title, s1, s2, p_types, p_descs, true);
	int r3 = dialog3->run();
	dialog3->hide();

printf("s1 = %s\n", s1.c_str());
printf("s2 = %s\n", s2.c_str());

	DataModel model("TransAG");
	model.loadFromFile("/bali/croatie/DACTILO/DEVEL/Transcriber/src/TEST/frint980428.tag");
	bool displayAnnotationTime = true;

	Parameters params;
	params.load("/home/mopicoreau/.TransAG/transcriberAG.rc");

	while (true) {
		tag::DialogFileProperties* dialog2 = new tag::DialogFileProperties(*mainwin, model, displayAnnotationTime, params);
		int r2 = dialog2->run();
		dialog2->hide();
	}

	gdk_threads_enter();
	Gtk::Main::run(*mainwin);
	gdk_threads_leave();

}
catch(const char* msg) {
cout<<msg<<endl;
}

	return 1;

}
