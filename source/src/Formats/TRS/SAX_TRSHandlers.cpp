/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// SAX_TransAGHandlers.cc:
// based on AIF element handler implementation by  Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.



#include <iostream>
#include <ag/AGAPI.h>
#include <ag/AGException.h>
#include <ag/agfio.h>
#include <ag/Utilities.h>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include "SAX_TRSHandlers.h"
#include "agfXercesUtils.h"
#include "Common/iso639.h"
#include "Common/util/Utils.h"

using namespace std;

#define atof myatof

/* comment to enable trace */
#define O_TRACE(a)
/* uncomment to enable trace */
//#define O_TRACE(a) trace((a), __FILE__, __LINE__ )

static void trace(string a, string f, int l)
{
	Log::err() << "At " <<  f << "," << l << ": " << a << endl << flush;
}

static const char* itoa(int i)
{
	static char buf[10];
	sprintf(buf, "%d", i);
	return buf;
}

SAX_TRSHandlers::SAX_TRSHandlers(const string& encoding, const string& dtd)
: localDTD(dtd), lastOffset(0.0), nbSections(0), nbTurns(0),
nbSyncOverlap(0), nbSyncs(0), turnNeeded(false), nbBackground(0),
sectionNeeded(false), who(1)
{
	dataModel = NULL;
	formatter = new XMLFormatter(encoding.c_str(),
#if _XERCES_VERSION >= 20300
				"1.0",
#endif
				this,
				XMLFormatter::NoEscapes,
				XMLFormatter::UnRep_CharRef);

	// first tag will be handled by AGSet handler
	StartStack.push(&SAX_TRSHandlers::TransStart);
	EndStack.push(&SAX_TRSHandlers::dummyEnd);
	arm_next = 0;
	isAutomatic=false;
	mapping = NULL ;
	mapping_qual_error.clear() ;
//	isLastEvent = false;
//	insideEvent = false;


	//TODO do it dynamically if more than 2 speakers
	// -- Prepare vector for keeping data
	vector<SyncContent*> sc1, sc2 ;
	syncStack[1] = sc1 ;
	syncStack[2] = sc2 ;
}

SAX_TRSHandlers::~SAX_TRSHandlers()
{
	delete formatter;
}

void SAX_TRSHandlers::set_encoding(const string& encoding)
{

	delete formatter;
	formatter = new XMLFormatter(encoding.c_str(),
#if _XERCES_VERSION >= 20300
				"1.0",
#endif
				this,
				XMLFormatter::NoEscapes,
				XMLFormatter::UnRep_CharRef);
}

void SAX_TRSHandlers::startElement
(const XMLCh* const name, AttributeList& attr)
{
	O_TRACE(trans(name));
	// execute the handler at the top of the stack
	(this->*StartStack.top())(name, attr);
}

void SAX_TRSHandlers::endElement(const XMLCh* const name)
{
	O_TRACE(string(">> ")+trans(name));
	// execute the handler at the top of the stack
	(this->*EndStack.top())(name);
}

void SAX_TRSHandlers::dummyStart
(const XMLCh* const name, AttributeList& attr)
{
	// if start-of-element is reported,
	// do nothing, and push do-nothing handlers
	StartStack.push(&SAX_TRSHandlers::dummyStart);
	EndStack.push(&SAX_TRSHandlers::dummyEnd);
}

void SAX_TRSHandlers::dummyEnd(const XMLCh* const name)
{
	// if end-of-element is reported
	// do nothing, and pop both stacks
	StartStack.pop();
	EndStack.pop();
}

/*
 * start trans
 */
void SAX_TRSHandlers::TransStart(const XMLCh* const name, AttributeList& attr)
{
	O_TRACE("IN TransStart");

	// -- Annotation
	if (!dataModel)
	{
		tag::DataModel::initEnviron("");
		dataModel = new tag::DataModel("TRS");
		dataModel->setConventions("trs_import");
	}

	dataModel->setAGSetProperty("annotations", "transcription") ;

	// -- Version
	ostringstream vers;
	string scribe = trans(attr.getValue("scribe")) ;
	dataModel->initVersionInfo(scribe , trans(attr.getValue("version_date")) ) ;

	// -- Status
	ostringstream status;
	status << "elapsed=" << trans(attr.getValue("elapsed")) << ";";
	dataModel->setAGSetProperty("status", status.str()) ;

	// -- Signal
	signalIds = dataModel->addSignal(trans(attr.getValue("audio_filename")), "audio", "wav", "PCM", dataModel->getNbTracks() + 1, 1);
	signalId = signalIds.back() ;

	// -- Language
	lang = trans(attr.getValue("xml:lang"));
	if ( lang.length() == 2 )
	{
		const char* plang= ISO639::get3LetterCode(lang.c_str());
		if ( *plang )
			lang=plang;
	}

	// -- Keep value of all initialized elements.
	// We'll use them to cut the element timelines each time we want to create an element
	string prevId = dataModel->getAGTrans() ;
	dataModel->initAnnotationGraphs("", lang, scribe);
	m_sectId[0] = dataModel->getByOffset("section", 0.0, 0, "transcription_graph") ;
	m_turnId[0] = dataModel->getByOffset("turn", 0.0, 0, "transcription_graph") ;
	m_segmentId[0] = dataModel->getByOffset("segment", 0.0, 0, "transcription_graph") ;
	m_unitId[0] = dataModel->getByOffset("unit", 0.0, 0, "transcription_graph") ;
	prevBackgroundId  = dataModel->getByOffset("background", 0.0, 0, "background_graph") ;
	if (!prevBackgroundId.empty())
	{
		dataModel->setElementProperty(prevBackgroundId, "type", "none") ;
		dataModel->setElementProperty(prevBackgroundId, "level", "low") ;
	}

	// -- Values used for keeping the creation order
	turnIds.push_back(m_turnId[0]);
	segIds.push_back(m_segmentId[0]);

	StartStack.push(&SAX_TRSHandlers::TransSubStart);
	EndStack.push(&SAX_TRSHandlers::TransEnd);
	O_TRACE("OUT TransStart") ;
}

// deal with "Trans" end items
void SAX_TRSHandlers::TransEnd
(const XMLCh* const name)
{
	char buf[20];
	int sec, min, hour;
	sec = (int)lastOffset;
	hour =  (int)sec / 3600;
	sec -=  (hour * 3600);
	min = (int)sec / 60;
	sec -= (min * 60);
	sprintf(buf, "%02d:%02d:%02d", hour, min, sec);

	//> -- Set some transcription value
	if ( isAutomatic )
	{
		dataModel->setAGSetProperty("type", "automatic transcription") ;
		dataModel->setAGSetProperty("convention_id", "transag_tap") ;
	}
	else
		dataModel->setAGSetProperty("type", "detailed transcription") ;

	//> -- Now we can set the last offset value
	dataModel->setSignalDuration(lastOffset) ;

	//>  -- Deals with last background
	float last_backgroundOffset = atof(lastBackgroundTime.c_str());
	if ( nbBackground > 1 )
	{
		float lastBgTime = atof(lastBackgroundTime.c_str()) ;

		prevBackgroundId = dataModel->insertMainstreamBaseElement(prevBackgroundId, lastBgTime);
		dataModel->setElementProperty(prevBackgroundId, "type", formatBackgrounds(lastBackgroundType), false);
		dataModel->setElementProperty(prevBackgroundId, "level", lastBackgroundLevel, false);
	}

	vector<string> prevUnits ;
	prevUnits.push_back(m_unitId[0]) ;

	//> -- Deals with the last overlapping segments
	addOverlapSegments(m_unitId[0], prevUnits) ;

	//> -- Deals with the last section
	if (nbSections > 1)
		addSection(m_unitId[0], currentSectionType, currentSectionTopic) ;

	//> -- Deals with the last turn
	if (nbTurns > 1)
		addTurns(m_unitId[0], currentTurnSpeakerStr, currentTurnAttribute) ;

	//> -- Deals with the last content
	addSyncContent(prevUnits) ;

	//> -- Clean room
	resetSyncStack() ;

	//> -- Links anchors
	linkBackgroundAnchors() ;

	// pop both stacks
	StartStack.pop();
	EndStack.pop();
}

// invoked at the start of a subelement of Trans :
//  -> topics list
//  -> speakers list
//  -> episode start
void SAX_TRSHandlers::TransSubStart(const XMLCh* const name, AttributeList& attr)
{
	const string& tag = trans(name);

	O_TRACE(string("IN TransSubStart ") + tag);

	// if Topics  element is found
	if (tag == "Topics") {
		StartStack.push(&SAX_TRSHandlers::TopicStart);
		EndStack.push(&SAX_TRSHandlers::dummyEnd);
	}

	// if Speakers element is found
	else if (tag == "Speakers")
	{
		StartStack.push(&SAX_TRSHandlers::SpeakerStart);
		EndStack.push(&SAX_TRSHandlers::dummyEnd);
	}

	// if Episode element is found
	else if (tag == "Episode")
	{
		O_TRACE(tag);
		dataModel->setSignalProperty( signalId, "source", trans(attr.getValue("program")) ) ;
		dataModel->setSignalProperty( signalId, "date", trans(attr.getValue("air_date")) ) ;

		O_TRACE(tag);

		agId = dataModel->getAG("transcription_graph") ;

		dataModel->setTranscriptionLanguage(lang) ;

		if ( ! speakers.str().empty() )
		{
			// clean default dictionary for enable total laoding
			dataModel->getSpeakerDictionary().clear() ;
			// add trs dictioanry to model
			dataModel->setGraphProperty("transcription_graph", "speakers", speakers.str()) ;
		}

		//if ( ! topics.str().empty() )
		//		SetFeature(agId, "topics", topics.str());

		//> create associated background
		background_ids[agId] = dataModel->getAG("background_graph") ;
		lastBackgroundTime = "" ;
		lastBackgroundLevel = "low" ;
		lastBackgroundType = "none" ;

		O_TRACE(tag);
		StartStack.push(&SAX_TRSHandlers::SectionStart);
		EndStack.push(&SAX_TRSHandlers::dummyEnd);
	}
	else
	{
		O_TRACE("UNKNOWN TAG");
		StartStack.push(&SAX_TRSHandlers::TransSubStart);
		EndStack.push(&SAX_TRSHandlers::dummyEnd);
	}
	O_TRACE(string("OUT TransSubStart ") + tag);

}

// invoked at the start of a speaker definition
void
SAX_TRSHandlers::SpeakerStart
(const XMLCh* const name, AttributeList& attr)
{
	O_TRACE("IN SpeakerStart ");
	string scope =trans(attr.getValue("scope"));
	if ( scope.empty() )
		scope = "local";

	speakers << "<Speaker id=\"" << trans(attr.getValue("id"))
	<< "\" name.last=\"" << trans(attr.getValue("name"))
	<< "\" gender=\"" << trans(attr.getValue("type"))
	<< "\" scope=\"" << scope ;

	string slang = trans(attr.getValue("lang"));
	if ( slang == "" )
		slang = lang;

	string dialect = trans(attr.getValue("dialect"));
	int isNative =0;
	if ( dialect == "native" )
	{
		isNative=1;
		dialect = "";
	}
	else if ( dialect == "nonnative" )
	{
		isNative=0;
		dialect = "";
	}

	if ( slang != "" )
	{
		speakers  << "\">" << endl;
		speakers << "<SpokenLanguage code=\"" << slang
		<< "\" dialect=\"" << dialect
		<< "\" isnative=\"" << isNative << "\" isusual=\"" << 1
		<< "\" accent=\"" << trans(attr.getValue("accent"))
		<< "\" />" << endl;
		speakers << "</Speaker>" << endl;
	}
	else
		speakers  << "\"/>" << endl;

	StartStack.push(&SAX_TRSHandlers::dummyStart) ;
	EndStack.push(&SAX_TRSHandlers::dummyEnd) ;

	O_TRACE("OUT SpeakerStart ") ;
}

// invoked at the start of a topic definition
void SAX_TRSHandlers::TopicStart(const XMLCh* const name, AttributeList& attr)
{
	O_TRACE("IN TopicStart ");

	topics[trans(attr.getValue("id"))] = trans(attr.getValue("desc"));

	O_TRACE("OUT TopicStart ") ;

	StartStack.push(&SAX_TRSHandlers::dummyStart) ;
	EndStack.push(&SAX_TRSHandlers::dummyEnd) ;
}

/*
 * invoked at the start of a section
 *
 * We can't create the section because the children are not created yet.
 * So we keep the section data and we'll create it as soon as the children are
 * built.
 *
 * Take care:
 * We can create base element (sync tag) only when we arrive at the following
 * element tag (to get the split timestamp). As we need to wait for the base
 * element to be created for creating all parents (like sections), we always create
 * the previous section after encountering a section tag.
 * Therefore, at a section tag, we need to keep data of the previous section and data
 * of the current section.
 */
void SAX_TRSHandlers::SectionStart (const XMLCh* const name, AttributeList& attr)
{
	O_TRACE("IN SectionStart ");

	nbSections++ ;

	// -- As soon as we are at the 2nd section, we can indicates that we'll need
	//	  to create the previous one as soon as the children are built.
	if (nbSections > 1)
		sectionNeeded = true ;


	//>  keep data for the turn that will be created
	lastSectionType = currentSectionType ;
	lastSectionTopic = currentSectionTopic ;

	//>  Keep current data
	const string& topicId = trans(attr.getValue("topic")) ;
	currentSectionType = trans(attr.getValue("type")) ;
	currentSectionTopic = topics[topicId] ;

	if (nbSections==1)
	{
		dataModel->setElementProperty(m_sectId[0], "type", currentSectionType, false) ;
		dataModel->setElementProperty(m_sectId[0], "desc", currentSectionTopic, false) ;

		lastSectionType = currentSectionType ;
		lastSectionTopic = currentSectionTopic ;
	}

	StartStack.push(&SAX_TRSHandlers::TurnStart);
	EndStack.push(&SAX_TRSHandlers::dummyEnd);
	O_TRACE("OUT SectionStart ");
}


/*
 * invoked at the start of a turn
 *
 * We can't create the turn because the children are not created yet.
 * So we keep the turn data and we'll create it as soon as the children are
 * built.
 *
 * Take care:
 * We can create base element (sync tag) only when we arrive at the following
 * element tag (to get the split timestamp). As we need to wait for the base
 * element to be created for creating all parents (like turns), we always create
 * the previous turn after encountering a turn tag.
 * Therefore, at a turn tag, we need to keep data of the previous turn and data
 * of the current turn.
 */
void SAX_TRSHandlers::TurnStart(const XMLCh* const name, AttributeList& attr)
{
	O_TRACE("IN TurnStart ");
	nbTurns++ ;

	turnIds.clear();
	segIds.clear();

	// -- Keep the end time because next tags won't have this information
	string endTime = trans(attr.getValue("endTime")) ;
	lastOffset =  atof(endTime.c_str()) ;

	// -- As soon as we are at the 2nd turn, we can indicate that we'll need
	//	  to create the previous one as soon as the children are built.
	if (nbTurns > 1)
		turnNeeded = true ;

	//>  Keep data for the turn(s) that will be created
	lastTurnSpeakersStr = currentTurnSpeakerStr ;
	lastTurnAttribute = currentTurnAttribute ;

	//>  Keep the current turn data
	currentTurnSpeakerStr = trans(attr.getValue("speaker")) ;
	saveElementFeatures(attr, currentTurnAttribute) ;

	//>  Update global values
//	prevValue.erase();
	segStartTime = "";
	segEndTime = "";
	who = 1 ;
	nbwords = 0;
	prevScore=0.0;

	StartStack.push(&SAX_TRSHandlers::TurnSubStart);
	EndStack.push(&SAX_TRSHandlers::TurnEnd);
	O_TRACE("OUT TurnStart ");
}



// invoked at the start of a subelement of Turn : sync / who / background / event / word
void SAX_TRSHandlers::TurnSubStart
(const XMLCh* const name, AttributeList& attr)
{
	string tag = trans(name);
	O_TRACE(string("IN TurnSubStart ") + tag);

//	if (tag == "Word")
//	{
//		WordStart(name, attr);
//		isAutomatic=true;
//		EndStack.push(&SAX_TRSHandlers::WordEnd);
//	}
//	else
//	{
		if (tag == "Sync")
			SyncStart(name, attr);
		else if (tag == "lang")
			LangStart(name, attr);
		else if (tag == "Who")
			WhoStart(name, attr);
		else if ( tag == "Background" )
			BackgroundStart(name, attr);
		else if (tag == "Event" )
			EventStart(name, attr);
		else if (tag == "Comment" )
			CommentStart(name, attr);
		else if (tag == "Vocal")
			VocalStart(name, attr);

/*		else if (tag == "Vocal" )
		{
			const string& order = getOrder(segIds[who-1]);
			prevId = CreateAnnotation(agId,
						GetStartAnchor(segIds[who-1]), GetStartAnchor(segIds[who-1]),
						"vocal");
			string desc = trans(attr.getValue("desc"));
			SetFeature(prevId, "desc", desc);
			if ( order != "" ) SetFeature(prevId, "order", order);
		}
		else if (tag == "Comment" )
		{
			const string& order = getOrder(segIds[who-1]);
			prevId = CreateAnnotation(agId,
						GetStartAnchor(segIds[who-1]), GetStartAnchor(segIds[who-1]),
						"comment");
			string desc = trans(attr.getValue("desc"));
			SetFeature(prevId, "desc", desc);
			if ( order != "" ) SetFeature(prevId, "order", order);
		}
*/
		EndStack.push(&SAX_TRSHandlers::dummyEnd);
//	}
	StartStack.push(&SAX_TRSHandlers::TurnSubStart);
	O_TRACE(string("OUT TurnSubStart ") + tag);
}

// deal with "Word" start items
//void SAX_TRSHandlers::WordStart
//(const XMLCh* const name, AttributeList& attr)
//{
//	++nbwords;
//
//	segStartTime = trans(attr.getValue("startTime")) ;
//	segEndTime = trans(attr.getValue("endTime")) ;
//	float start = atof(segStartTime) ;
//	float end = atof(segEndTime) ;
//	const string& score = trans(attr.getValue("conf")) ;
//
//	syncStack[who].push_back( new SyncContent(who, "", score, start, end) ) ;
//
//	/*
//	//todo do it
//	const string& startAnchor = getAnchor(segStartTime);
//	const string& endAnchor = getAnchor(segEndTime);
//	const string& order = getOrder(segIds[who-1]);
//
//	if ( nbwords == 1 && startAnchor != GetStartAnchor(segIds.back()) ) {
//		// create empty "word" seg between segment start && word start
//		prevId = CreateAnnotation(agId,
//					GetStartAnchor(segIds.back()), startAnchor,
//					"word");
//		if ( order != "" ) SetFeature(prevId, "order", order);
//	}
//	prevId = CreateAnnotation(agId,
//				startAnchor, endAnchor,
//				"word");
//	string conf =  trans(attr.getValue("conf"));
//	if ( ! conf.empty() ) SetFeature(prevId, "score", conf);
//	if ( order != "" ) SetFeature(prevId, "order", order);
//*/
//	//	if ( nbwords == 1 ) SetFeature(segIds[who-1], "type", "speech");
//}

// deal with "Word" end items
void SAX_TRSHandlers::WordEnd(const XMLCh* const name)
{
//	dataModel->setElementProperty(m_unitId[0], "value", prevValue) ;
	//	SetFeature(prevId, "value", prevValue);
//	prevValue.erase();

	// pop both stacks
	StartStack.pop();
	EndStack.pop();
}


/*
 *  Deals with "sync" items
 *
 *  We can't create a base element before we know the timestamp where the
 *  following element begins. Moreover, after graphe initialization a default
 *  base element is created over all the timeline.
 *
 *  Therefore, at a sync tag we split the existing base element at the time
 *  given by the tag. After that, we can create all parents that needed to be
 *  created over PREVIOUS base element(s).
 */
void SAX_TRSHandlers::SyncStart(const XMLCh* const name, AttributeList& attr)
{
	nbSyncs ++ ;

	segStartTime = trans(attr.getValue("time"));
	segEndTime = "";

	float start = atof(segStartTime.c_str()) ;
	string prevUnit = m_unitId[0] ;
	string prevSeg = m_segmentId[0] ;

	if (nbSyncs > 1)
	{
		string prev = m_unitId[0] ;

		// -- Keep last segment for adding text and event
		std::vector<string> prevUnits ;
		prevUnits.push_back(m_unitId[0]) ;

		// -- Split existing segment to inserting new
		m_unitId[0] = dataModel->insertMainstreamBaseElement(m_unitId[0], start, true, false) ;
		dataModel->setElementProperty(m_unitId[0] , "subtype", "unit_text", false) ;

		m_segmentId[0] = dataModel->insertMainstreamElement("segment", m_unitId[0], start, true, false) ;

		// We've just added the new segment of order 0, reset the storing list
		segIds.clear() ;
		segIds.push_back(m_segmentId[0]) ;

		//2 -Add eventual overlapping elements at previous position
		addOverlapSegments(prevUnit, prevUnits) ;
		nbSyncOverlap = 0 ;

		//3 - Add eventual terminated section
		if (sectionNeeded)
			addSection(prevUnit, lastSectionType, lastSectionTopic) ;

		//4 - Add eventual terminated turn
		if (turnNeeded)
			addTurns(prevUnit, lastTurnSpeakersStr, lastTurnAttribute) ;

		//5- Deals with text and events
		addSyncContent(prevUnits) ;

		//6- Deals with background links
		// Keep all unit anchors and we'll see after all the matching background
		const string& startAnchor = dataModel->getAnchor(prev, true) ;
		const string& endAnchor = dataModel->getAnchor(prev, false) ;
		anchorsToLink.insert(startAnchor) ;
		anchorsToLink.insert(endAnchor) ;
	}
	else
		dataModel->setElementProperty(m_unitId[0] , "subtype", "unit_text", false);
}

void SAX_TRSHandlers::addOverlapSegments(const string& prevUnit, vector<string>& prevUnits)
{
	int cpt = 1 ;
	while (cpt <= nbSyncOverlap)
	{
		string over_unit, over_seg ;
		const string& startA = dataModel->getAnchor(prevUnit, true) ;
		const string& endA = dataModel->getAnchor(prevUnit, false) ;

		// -- Create unit & segment
		over_unit = dataModel->createAnnotation(agId, dataModel->mainstreamBaseType(), startA, endA, false) ;
		dataModel->setElementProperty(over_unit , "order", cpt, false);
		dataModel->setElementProperty(over_unit , "subtype", "unit_text", false);

		over_seg = dataModel->createAnnotation(agId, dataModel->segmentationBaseType(), startA, endA, false) ;
		dataModel->setElementProperty(over_seg , "order", cpt, false);

		segIds.push_back(over_seg) ;

		// -- Keep created overlapping unit for adding text and event
		prevUnits.push_back(over_unit) ;

		cpt++ ;
	}
}

/*
 *  Once we have created the base elements, we apply on them the data we have
 *  encountered.
 *  prevUnits is the unit elements we have to use when creating new ones.
 */
void SAX_TRSHandlers::addSyncContent(vector<string>& prevUnits)
{
	//> -- For each speaker
	std::map<int, vector< SyncContent* > >::iterator it ;
	for (it=syncStack.begin(); it!=syncStack.end(); it++)
	{
		// We'll use the stack for keeping BEGIN event in memory, and
		// we'll create them at the END event
		std::deque<string> stack ;

		// who's speaking ?
		int currentWho = it->first ;

		//> -- For each content let's proceed
		vector<SyncContent*>::iterator itsync ;
		SyncContent* syc = NULL ;
		SyncContent* previous = NULL ;
		for (itsync = it->second.begin(); itsync !=it->second.end(); itsync ++)
		{
			previous = syc ;
			syc = *itsync ;
			if ( syc )
			{
				const string& order = getOrder( prevUnits[currentWho-1] );

				//> -- Set text ? add property to last element (the end frontier will be created if another element starts)
				if (syc->mode == SyncContent::TXT && !syc->txt.empty() )
				{
					// -- Previous was text, let's merged (should not happen except in bad formed doc, let's heal)
					if ( previous && previous->mode==SyncContent::TXT)
					{
						string txt = dataModel->getElementProperty(prevUnits[currentWho-1], "value", "") ;
						txt += syc->txt ;
						dataModel->setElementProperty(prevUnits[currentWho-1], "value", txt) ;
					}
					// -- The last content was an event with NEXT extent ? we need to apply it now
					else if ( previous && previous->mode==SyncContent::EVENT_NEXT )
					{
						string remain ;
						string first = getFirstLastWord(syc->txt, remain, true) ;

						// -- create first unit whose text is the first word
						prevUnits[currentWho-1] = dataModel->insertMainstreamBaseElement(prevUnits[currentWho-1], -1, true, false) ;
						if ( order != "" )
							dataModel->setElementProperty(prevUnits[currentWho-1], "order", order) ;
						dataModel->setElementProperty(prevUnits[currentWho-1], "value", first) ;

						// -- create second unit with eventual remaining text
						string bak = prevUnits[currentWho-1] ;
						prevUnits[currentWho-1] = dataModel->insertMainstreamBaseElement(prevUnits[currentWho-1], -1, true, false) ;
						if ( order != "" )
							dataModel->setElementProperty(prevUnits[currentWho-1], "order", order) ;
						dataModel->setElementProperty(prevUnits[currentWho-1], "value", remain) ;

						// -- apply the waiting qualifier on the first unit
						createQualifier(previous->type, previous->desc, bak, order) ;
					}
					// -- Otherwise just add text property to the current segment
					else
					{
						dataModel->setElementProperty(prevUnits[currentWho-1], "value", syc->txt) ;
						dataModel->setElementProperty(prevUnits[currentWho-1], "subtype", "unit_text") ;
					}
				}
				//> -- Add event related to the last word ?
				if (syc->mode == SyncContent::EVENT_PREVIOUS )
				{
					// should be only happened if last one was a text content
					if (previous && previous->mode==SyncContent::TXT)
					{
						// get last text and cut it
						const string& txt = dataModel->getElementProperty(prevUnits[currentWho-1], "value", "") ;
						string remain ;
						string lastWord = getFirstLastWord(txt, remain, false) ;

						// cut existing element
						string back = prevUnits[currentWho-1] ;
						prevUnits[currentWho-1] = dataModel->insertMainstreamBaseElement(prevUnits[currentWho-1], -1, true, false) ;
						if ( order != "" )
							dataModel->setElementProperty(prevUnits[currentWho-1], "order", order) ;

						// if the last text has been splitted
						if (!remain.empty())
						{
							// the previous created unit get the text without the last word
							dataModel->setElementProperty(back, "value", remain) ;
							// we add a new element
							back = prevUnits[currentWho-1] ;
							prevUnits[currentWho-1] = dataModel->insertMainstreamBaseElement(prevUnits[currentWho-1], -1, true, false) ;
							if ( order != "" )
								dataModel->setElementProperty(prevUnits[currentWho-1], "order", order) ;
						}
						// the last created received the lastWord and the qualifier
						dataModel->setElementProperty(back, "value", lastWord) ;
						createQualifier(syc->type, syc->desc, back, order) ;
					}
				}
				//> -- Add instantaneous event ? create the element
				else if ( syc->mode == SyncContent::EVENT_INSTANT )
				{
					bool firstElem = false ;
					bool lastElem = false ;
					vector<SyncContent*>::iterator tmpit = itsync ;
					if (tmpit == it->second.begin())
						firstElem = true ;
					if ( tmpit+1 == it->second.end() )
						lastElem = true ;

					/*
					 * Unique element of segment: don't create anything, use existing one
					 */
					if (firstElem && lastElem)
						dataModel->setEventMainstreamElement(prevUnits[currentWho-1], syc->type, syc->desc, false) ;
					/*
					 * If we're adding an instant event (i.e foreground event) at segment start,
					 * -current unit is the first of segment, has text type but no text value-
					 * don't split at start or we'll have 2 anchors at same position !!
					 * 	==> Split the existing unit, the first part of split becomes the event,
					 *  	the next part becomes the current unit
					 */
					else if ( firstElem )
					{
						string prev = prevUnits[currentWho-1] ;
						prevUnits[currentWho-1] = dataModel->insertMainstreamBaseElement(prevUnits[currentWho-1], -1, true, false) ;
						dataModel->setEventMainstreamElement(prev, syc->type, syc->desc, false) ;
						if ( order != "" )
							dataModel->setElementProperty(prevUnits[currentWho-1], "order", order) ;
					}
					/*
					 * If we're adding a fg event at segment end, split existing one as new event
					 */
					else if ( lastElem)
					{
						prevUnits[currentWho-1] = dataModel->insertMainstreamBaseElement(prevUnits[currentWho-1], -1, true, false) ;
						dataModel->setEventMainstreamElement(prevUnits[currentWho-1], syc->type, syc->desc, false) ;
						if ( order != "" )
							dataModel->setElementProperty(prevUnits[currentWho-1], "order", order) ;
					}
					/*
					 * If we're adding between several units, split at start and end
					 */
					else
					{
						string newId = dataModel->insertEventMainstreamElement(prevUnits[currentWho-1], -1, -1, syc->type, syc->desc, true, false) ;
						const string& nextId = dataModel->getNextElementId(newId) ;
						prevUnits[currentWho-1] = nextId ;
						if ( order != "" )
							dataModel->setElementProperty(newId, "order", order) ;
					}
				}
				//> -- Beginning a qualifier ? let's split the last element
				// 	   Keep the start anchor, we'll create the qualifier at the end
				else if (syc->mode == SyncContent::EVENT_BEGIN)
				{
					prevUnits[currentWho-1] = dataModel->insertMainstreamBaseElement(prevUnits[currentWho-1], -1, true, false) ;
					if ( order != "" )
						dataModel->setElementProperty(prevUnits[currentWho-1], "order", order) ;

					//-- Keep start anchor, will be used when tag element is encountered and we'll create the qualifier
					stack.push_front( dataModel->getAnchor(prevUnits[currentWho-1], true) ) ;
				}
				//> -- Ending a qualifier ? Let's use the stack to see where the qualifier should be attached
				else if (syc->mode == SyncContent::EVENT_END)
				{
					// stack should always be filled at this point
					if (stack.size()!=0)
					{
						//> -- split segment
						string bak = prevUnits[currentWho-1] ;
						prevUnits[currentWho-1] = dataModel->insertMainstreamBaseElement(prevUnits[currentWho-1], -1, false, false) ;
						if ( order != "" )
							dataModel->setElementProperty(prevUnits[currentWho-1], "order", order) ;

						//> -- Create Qualifer
						const string& endAnchor = dataModel->getAnchor(bak, false) ;
						const string& startAnchor = stack.front() ;

						createQualifier(syc->type, syc->desc, startAnchor, endAnchor, order) ;
						stack.pop_front() ;
					}
					// otherwise, it's an error
					else
						std::cerr << "Trying to end an event: no start in stack... Aborted" << std::endl ;
				}
				//> Missing NEXT element ? no, we'll proceed it at next TEXT element
			}
		}
	}
	// -- We've treated all contents, don't forget to clean room :)
	resetSyncStack() ;
}

void SAX_TRSHandlers::addTurns(const string& prevUnit, const string& speakers, const map<string,string>& attributes)
{
	string startAnchor ;
	string endAnchor = dataModel->getAnchor(prevUnit, false) ;

	//-- We at the 2nd turn element: we don't need to create the previous turn (i.e the 1st one)
	//   because it has been done when initializing the graphe.
	//   Just move its end anchor whe it must be
	if (nbTurns==2)
	{
		//justMove anchor
		startAnchor = dataModel->getAnchor(m_turnId[0], true) ;
		dataModel->setAnchor(m_turnId[0], endAnchor, false) ;
	}
	//-- Other case, let's create the turn
	else
	{
		// -- Its tail must match the last head
		startAnchor = dataModel->getAnchor(m_turnId[0], false) ;
		m_turnId[0] = dataModel->createAnnotation(agId, "turn", startAnchor, endAnchor, false) ;
		turnIds.push_back(m_turnId[0]) ;
	}

	// -- Create previous overlapping turns if some exist
	vector<string> v = Utilities::splitString(speakers, ' ') ;
	for ( int i=0; i < v.size(); ++i )
	{
		string over_turn ;
		if (i > 0)
		{
			over_turn = dataModel->createAnnotation(agId, "turn", startAnchor, endAnchor, false) ;
			startAnchor = dataModel->getAnchor(over_turn, false) ;
			dataModel->setElementProperty(over_turn , "order", i, false);
			turnIds.push_back(over_turn);
		}
		else
			over_turn = m_turnId[0] ;

		// -- Set properties (for order 0 too)
		dataModel->setElementProperty(over_turn , "speaker", v[i], false);
		setElementFeatures(over_turn , attributes, " speakers startTime endTime ");
		nbwords=0;
	}

	//-- If no speaker, delete property
	if (speakers.empty())
		dataModel->deleteElementProperty(m_turnId[0], "speaker") ;

	turnNeeded = false ;
}

void SAX_TRSHandlers::addSection(const string& prevUnit, const string& type, const string& topic)
{
	const string& endAnchor = dataModel->getAnchor(prevUnit, false) ;

	//-- We  the 2nd section element: we don't need to create the previous section (i.e the 1st one)
	//   because it has been done when initializing the graph.
	//   Just move its end anchor where it must be
	if (nbSections==2)
		dataModel->setAnchor(m_sectId[0], endAnchor, false) ;
	//-- Other case, let's create the turn
	else
	{
		// -- Its tail must match the last head
		const string& startAnchor = dataModel->getAnchor(m_sectId[0], false ) ;
		m_sectId[0] = dataModel->createAnnotation(agId, "section", startAnchor, endAnchor, false) ;
		dataModel->setElementProperty(m_sectId[0], "type", type, false) ;
		if ( !topic.empty() )
				dataModel->setElementProperty(m_sectId[0], "desc", topic, false) ;
	}

	sectionNeeded = false ;
}

// deal with "Lang" items
void SAX_TRSHandlers::LangStart(const XMLCh* const name, AttributeList& attr)
{
	float score = atof(trans(attr.getValue("score")).c_str());
	if ( score > 0.0 && score > prevScore )
	{
		dataModel->setElementProperty(turnIds[who-1], "lang", trans(attr.getValue("name")), false) ;
		prevScore = score;
	}
}

// deal with "who" items
void SAX_TRSHandlers::WhoStart
(const XMLCh* const name, AttributeList& attr)
{

	who = atoi(trans(attr.getValue("nb")).c_str());

	// -- For all overlapping speeches, let's count number of level
	//    It will be used for creating turn
	if (who>1)
		nbSyncOverlap++ ;
}


// deal with "background" items
void SAX_TRSHandlers::BackgroundStart(const XMLCh* const name, AttributeList& attr)
{
	nbBackground ++ ;

	string startTime = trans(attr.getValue("time"));
	float start = atof(startTime.c_str());

	//> Create background if not at 0 time
	//  if 0 time, will be created at next background border
	float lastTime = atof(lastBackgroundTime.c_str());
	if (lastTime!=0)
	{
//		prevBackgroundId = dataModel->addBackgroundSegment(0, lastTime, -1, formatBackgrounds(lastBackgroundType), lastBackgroundLevel, tag::DataModel::ADJUST_PREVIOUS, false) ;
		prevBackgroundId = dataModel->insertMainstreamBaseElement(prevBackgroundId, lastTime);
		dataModel->setElementProperty(prevBackgroundId, "type", formatBackgrounds(lastBackgroundType), false);
		dataModel->setElementProperty(prevBackgroundId, "level", lastBackgroundLevel, false);
	}

	lastBackgroundTime = startTime ;
	string level = trans(attr.getValue("level"));
	string type = trans(attr.getValue("type"));

	if (level!="off")
	{
		lastBackgroundLevel = level ;
		lastBackgroundType = type ;
	}
	else
	{
		lastBackgroundLevel = "low" ;
		lastBackgroundType = "none" ;
	}
}


// deal with "comment" items
void SAX_TRSHandlers::CommentStart
(const XMLCh* const name, AttributeList& attr)
{
	string type = "comment";
	string desc = trans(attr.getValue("desc"));

	syncStack[who].push_back( new SyncContent(who, SyncContent::EVENT_INSTANT, type, desc) ) ;
}

// deal with "vocal" items
void SAX_TRSHandlers::VocalStart (const XMLCh* const name, AttributeList& attr)
{
	string type = "vocal";
	string desc = trans(attr.getValue("desc"));
	syncStack[who].push_back( new SyncContent(who, SyncContent::EVENT_INSTANT, type, desc) ) ;
}

// deal with "event" items
void SAX_TRSHandlers::EventStart(const XMLCh* const name, AttributeList& attr)
{
	string type = trans(attr.getValue("type"));
	string extent = trans(attr.getValue("extent"));
	string desc = trans(attr.getValue("desc"));

	bool is_entity = false ;
	SyncContent::SyncContentMode mode ;

	//> type empty ? let's consider it as a noise
	if ( type.empty() )
		type = "noise";
	//> type is entities ? we have an entity type too
	else if ( type == "entities" )
		is_entity = true ;

	//> Format it
	formatAGqualifier(is_entity, type, desc) ;

	//> Event has not specific extent ? let's say it's instantaneous
	//  --> Instantaneous = foreground event
	if ( extent.empty() || extent == "instantaneous" )
		mode = SyncContent::EVENT_INSTANT ;
	else if ( extent == "begin" )
		mode = SyncContent::EVENT_BEGIN ;
	else if (extent == "end")
		mode = SyncContent::EVENT_END ;
	else if (extent == "next")
		mode = SyncContent::EVENT_NEXT ;
	else if (extent == "previous")
		mode = SyncContent::EVENT_PREVIOUS ;

	syncStack[who].push_back( new SyncContent(who, mode, type, desc) ) ;
}


// invoked at the end of Turn
// 		if non-empty text -> store it in last created transcription segment
void SAX_TRSHandlers::TurnEnd(const XMLCh* const name)
{
//TODO do it for words

//	if ( GetAnnotationType(prevId) == "word" )
//	{
//		// check if missing word segment at turn end
//		const string& turn_end = GetEndAnchor(turnIds.back() );
//		const string& word_end = GetEndAnchor(prevId);
//		if ( GetAnchorOffset(word_end) < GetAnchorOffset(turn_end) )
//		{
//			// create empty "word" seg between word end && turn end
//			prevId = CreateAnnotation(agId, word_end, turn_end, "word");
//		}
//		else if ( GetAnchorOffset(word_end) > GetAnchorOffset(turn_end) )
//		{
//			if ( GetStartOffset(prevId) <  GetAnchorOffset(turn_end) )
//				SetEndAnchor(prevId, turn_end);
//			else
//				Log::err() << "wrong timecode for word at " << GetStartOffset(prevId) <<endl;
//		}
//	}

//	if ( ! prevValue.empty() )
//		setLastSegmentText(who);

	StartStack.pop() ;
	EndStack.pop() ;
}

/*
 * As all N elements are created at N+1 SYNC start tag, when characters tag is found we
 * can't set the property yet.
 *
 * Let's keep them in a stack, they will be applied at the next SYNC start tag
 */
void SAX_TRSHandlers::characters (const XMLCh* const chars, const unsigned int length)
{
	string current_chars;
	set_string(current_chars, chars);

	// get rid of leading & terminating newlines
	unsigned long pos = strspn(current_chars.c_str(), " \t\n");
	unsigned long pos2 = current_chars.find_last_of(" \t\n");

	if ( pos2 == pos || pos2 == string::npos )
		pos2 = current_chars.size() ;

	if ( pos > 0 || pos2 < current_chars.size() )
		current_chars = current_chars.substr(pos, (pos2-pos)) ;


	if (current_chars.empty())
		return ;

	bool merged = false ;
	if ( syncStack[who].size() > 0 )
	{
		// -- If the last action was text, let's merge
		SyncContent* sc = syncStack[who].back() ;
		if (sc && sc->mode==SyncContent::TXT && !sc->txt.empty())
		{
			sc->txt = sc->txt + " " + current_chars ;
			merged = true ;
		}
	}
	// -- No merge was done, let's add a new SyncContent
	if (!merged)
		syncStack[who].push_back( new SyncContent(who, current_chars) ) ;
}

void SAX_TRSHandlers::warning(const SAXParseException& e)
{
	Log::err() << "WARNING: " << trans(e.getMessage()) << endl;
	Log::err() << " at line " << e.getLineNumber ()
	<< " col " << e.getColumnNumber () << endl;
}

void SAX_TRSHandlers::error(const SAXParseException& e)
{
	Log::err() << "WARNING: " << trans(e.getMessage()) << endl;
	Log::err() << " at line " << e.getLineNumber ()
	<< " col " << e.getColumnNumber () << endl;
}

void SAX_TRSHandlers::fatalError(const SAXParseException& e)
{
	Log::err() << "ERROR: " << trans(e.getMessage()) << endl;
	Log::err() << " at line " << e.getLineNumber ()
	<< " col " << e.getColumnNumber () << endl;
	throw agfio::LoadError(trans(e.getMessage()));
}

void SAX_TRSHandlers::writeChars(const XMLByte* const toWrite,
			const XMLSize_t count,
			XMLFormatter* const formatter)
{
	targetString.assign((char*) toWrite, count);
}

string& SAX_TRSHandlers::set_string(string& s, const XMLCh* const chars)
{
	targetString.erase();
	(*formatter) << chars;
	s = targetString;
	return s;
}

InputSource*
SAX_TRSHandlers::resolveEntity(const XMLCh* const publicId,
			const XMLCh* const systemId)
{
	if (! localDTD.empty()) {
		LocalFileInputSource* is = new LocalFileInputSource(trans(localDTD));
		localDTD = ""; // dirty hack to prevent from loading the dtd for any entity
		return is;
	}

	return NULL;
}

//******************************************************************************
//								DataModel help
//******************************************************************************

void SAX_TRSHandlers::saveElementFeatures(AttributeList& attr, map<string,string>& mapattr)
{
	for ( int i = 0; i < attr.getLength(); ++i )
		mapattr[trans(attr.getName(i))] = trans(attr.getValue(i)) ;
}

void SAX_TRSHandlers::setElementFeatures(const string& id, const map<string,string>& attr, const char* exclude)
{
 	string tag ;
 	map<string,string>::const_iterator it ;
	for ( it=attr.begin(); it!=attr.end(); it++)
	{
		tag = it->first ;
		if ( strstr(exclude, tag.c_str()) == NULL )
 			dataModel->setElementProperty(id, tag, it->second);
	}
}

string SAX_TRSHandlers::getOrder(const string& id)
{
	return dataModel->getElementProperty(id, "order", "") ;
}


//******************************************************************************
//							SYNC tag STACK help
//******************************************************************************

/**< Constructor for event*/
SAX_TRSHandlers::SyncContent::SyncContent(int who, SyncContentMode mode, string type, string desc)
{
	init() ;

	this->who = who ;
	this->type = type ;
	this->desc = desc ;
	this->mode = mode ;

//	cout << "New SyncContent : " << print() << std::endl ;
}

/**< Constructor for text*/
SAX_TRSHandlers::SyncContent::SyncContent(int who, string text)
{
	init() ;

	this->who = who ;
	this->txt = text ;
	this->mode = TXT ;

//	cout << "New SyncContent : " << print() << std::endl ;
}

/**< Constructor for word*/
SAX_TRSHandlers::SyncContent::SyncContent(int who, string text, string score, float start, float end)
{
	init() ;

	this->who = who ;
	this->txt = text ;
	this->score = score ;
	this->mode = WORD ;
	this->start = start ;
	this->end = end ;

//	cout << "New SyncContent : " << print() << std::endl ;
}

void SAX_TRSHandlers::SyncContent::init()
{
	who = -1 ;
	txt = "" ;
	score = "" ;
	mode = TXT ;
	type = "" ;
	desc = "" ;
	start = -1 ;
	end = -1 ;
}

string SAX_TRSHandlers::SyncContent::print()
{
	string print = "who=" + number_to_string(who) ;
	if (mode==TXT)
	{
		print += " - mode=TXT" ;
		print += " - value=" + txt ;
	}
	else if (mode==WORD)
	{
		print += " - mode=WORD" ;
		print += " - value=" + txt ;
		print += " - score=" + score ;
	}
	else
	{
		if (mode==EVENT_BEGIN)
			print += " - mode=EVENT_BEGIN" ;
		else if (mode==EVENT_END)
			print += " - mode=EVENT_END" ;
		else if (mode==EVENT_INSTANT)
			print += " - mode=EVENT_INSTANT" ;
		print += " - type=" + type  + " - desc=" + desc ;
	}
	return print ;
}

void SAX_TRSHandlers::printSyncStack()
{
	std::cout << "\n ~~~~~PRINT SYNC STACK " << std::endl ;

	std::map<int, vector< SyncContent* > >::iterator it ;
	for (it=syncStack.begin(); it!=syncStack.end(); it++)
	{
		std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Track " << number_to_string(it->first) << std::endl ;
		vector<SyncContent*>::iterator itsync ;
		for (itsync = it->second.begin(); itsync !=it->second.end(); itsync ++)
		{
			if (*itsync)
				cout << (*itsync)->print() << std::endl ;
		}
	}

	std::cout << "~~~~~DONE\n " << std::endl ;
}

void SAX_TRSHandlers::resetSyncStack()
{
	std::map<int, vector< SyncContent* > >::iterator it ;
	for (it=syncStack.begin(); it!=syncStack.end(); it++)
	{
		vector<SyncContent*>::iterator itsync ;
		for (itsync = it->second.begin(); itsync !=it->second.end(); itsync ++)
		{
			if (*itsync)
				delete(*itsync) ;
		}
		it->second.clear() ;
	}
}


//******************************************************************************
//							Background help
//******************************************************************************

string SAX_TRSHandlers::getBackgroundAnchor(const string& offset_str)
{
	float offset = atof(offset_str.c_str());
	return dataModel->createAnchorIfNeeded(dataModel->getAG("background_graph"), 0, offset, false) ;
}

string SAX_TRSHandlers::getBackgroundAnchor(float offset)
{
	return dataModel->createAnchorIfNeeded(dataModel->getAG("background_graph"), 0, offset, false) ;
}

// Format a chain of background type thanks to formatBackground function
string SAX_TRSHandlers::formatBackgrounds(string& back_chain)
{
	string resBack = "";
	set<string> backgroundSet;
	set<string>::iterator it_back;

	int startPos = 0;
	
	back_chain += " ";

	int endPos = back_chain.find(" ");
	string currBack;
	while ( endPos != -1)
	{
		currBack = back_chain.substr(startPos, endPos - startPos);
		backgroundSet.insert(SAX_TRSHandlers::formatBackground(currBack));
		startPos = endPos + 1;
		endPos = back_chain.find(" ", startPos);
	}

	if ( backgroundSet.size() > 1)
		for (it_back = backgroundSet.begin() ; it_back != backgroundSet.end() ; it_back++) {
			resBack += *it_back + ";";
		}
	else
	{
		it_back = backgroundSet.begin();
		resBack += *it_back;
	}

	return resBack;
}

// Format a background type in a wellknown type
string SAX_TRSHandlers::formatBackground(string& back_type)
{
	if ( back_type == "none" ) return "none";

	string typeRes;
	if (mapping)
	{
		typeRes = mapping->getParameterValue("background", "type," + back_type) ;
	}
	else
		typeRes = "noise" ;


	if (typeRes != "")
		return typeRes;
	else
		return "noise";
}

void SAX_TRSHandlers::linkBackgroundAnchors()
{
	set<string>::iterator it ;
	for (it=anchorsToLink.begin(); it!=anchorsToLink.end(); it++)
	{
		float offset = dataModel->getAnchorOffset(*it) ;
		const string& anchor = dataModel->getAnchorAtOffset(background_ids[agId], 0, offset) ;
		if ( !anchor.empty() && !(*it).empty() )
			dataModel->anchorLinks().link(*it, anchor, true) ;
	}
}

//******************************************************************************
//								Qualifier help
//******************************************************************************

string SAX_TRSHandlers::createQualifier(const string& type, const string& desc, const string& prevId, const string& order)
{
	const string& startA = dataModel->getAnchor(prevId, true) ;
	const string& endA = dataModel->getAnchor(prevId, false) ;
	return createQualifier(type, desc, startA, endA, order) ;
}

string SAX_TRSHandlers::createQualifier(const string& type, const string& desc, const string& startAnchor, const string& endAnchor, const string& order)
{
	string qual = dataModel->createAnnotation(agId, type, startAnchor, endAnchor, false);

	if ( !desc.empty() )
		dataModel->setElementProperty(qual, "desc", desc, false);

	if ( order != "" )
		dataModel->setElementProperty(qual, "order", order, false);

	return qual ;
}

void SAX_TRSHandlers::formatAGqualifier(bool is_entity, string& typeToMap, string& descToMap)
{
	string orig_type = typeToMap ;
	string orig_desc = descToMap ;

	string qualifier_type ;
	if (is_entity)
		qualifier_type = "entity" ;
	else
		qualifier_type = "event" ;

	O_TRACE(string("SAX_TRSHandlers::formatAGqualifier class_type=") + qualifier_type + string(" - [") + typeToMap + string(" - ") + descToMap + string("]")) ;

	string typekey, desckey ;
	string cutted_type = "" ;
	string cutted_desc = "" ;

	//> For entity, trs type=entity and trs desc contains data for tag type
	//  => type becomes desc for further computation
	if (is_entity)
	{
		// cut TRSdesc into TAGtype and TAGdesc
		getEntityTypeAndDesc(descToMap, cutted_type, cutted_desc) ;
		typekey = "type," + cutted_type ;
	}
	else
		typekey = "type," + typeToMap ;

	if (is_entity)
		desckey = "desc," + descToMap ;
	// For other events than entity, desc doesn't get any information about the type it
	// corresponds to. Add it
	else
		desckey = "desc," + typeToMap +"."+descToMap ;

	string typeRes ;
	string descRes ;

	if (mapping)
	{
		typeRes = mapping->getParameterValue(qualifier_type, typekey) ;
		descRes = mapping->getParameterValue(qualifier_type, desckey) ;
	}
	else
	{
		typeRes = "" ;
		descRes = "" ;
	}

	O_TRACE("--> found: type=" + typeRes + " - desc=" + descRes) ;

	if (!typeRes.empty())
		typeToMap = typeRes ;
	else if (is_entity)
		typeToMap = cutted_type ;

	if (!descRes.empty())
		descToMap = descRes ;
	else if (is_entity)
		descToMap = cutted_desc ;

	O_TRACE("--> returned: type=" + typeToMap + " - desc=" + descToMap) ;

//	checkAGqualifier(typeToMap, descToMap, orig_type, orig_desc) ;
}

//void SAX_TRSHandlers::checkAGqualifier(const string& TAGtype, const string& TAGdesc, const string& TRStype, const string& TRSdesc)
//{
//	bool typeOK, descOK ;
//	bool isInConventions = dataModel->conventions().isQualifier(TAGtype, TAGdesc, "transcription_graph", typeOK, descOK) ;
//	if (!isInConventions)
//	{
//		bool display = false ;
//		bool first = mapping_qual_error.empty() ;
//
//		string warning = "TRS[ type(" + TRStype + ") - desc(" + TRSdesc + ") ]"
//							+ "  ==>  TAG[ type(" + TAGtype + ") - subtype(" + TAGdesc + ") ]" ;
//
//		//> if type not in conv, always display warning
//		if (!typeOK) {
//			warning = warning + "\n\t\t <" + TAGtype + ">: " + string("TAG type doesn't exist in conventions") ;
//			O_TRACE(warning) ;
//			mapping_qual_error.push_back(warning) ;
//			display = true ;
//		}
//		//> for descritpion, only display warning if subtype can't be edited
//		//  (if subtype can be, desc will often be out of conv)
//		else if (!descOK) {
//			bool subtype_editable = dataModel->conventions().subtypeCanBeEdited(TAGtype) ;
//			if (!subtype_editable) {
//				warning = warning + "\n\t\t <" + TAGdesc +">: " + string("TAG subtype doesn't exist in conventions") ;
//				O_TRACE(warning) ;
//				mapping_qual_error.push_back(warning) ;
//				display = true ;
//			}
//		}
//
//		//> first entry: add the header
//		if (display && first) {
//			string head = "\n" ;
//			head = head + _("Following annotations are not (correctly) defined in conventions.") ;
//			head = head + "\n" + _("They may not be visible in text editor.") ;
//			head = head + "\n" + _("Check the mapping conversion file or the convention file.") +"\n" ;
//			mapping_qual_error.insert(mapping_qual_error.begin(), head) ;
//		}
//	}
//}

bool SAX_TRSHandlers::getEntityTypeAndDesc(const string& TRSdesc, string& TAGtype, string& TAGdesc)
{
	bool done = false ;
	size_t pos = TRSdesc.find_first_of(".", 0) ;
	if (pos!=string::npos)
	{
		TAGtype = TRSdesc.substr(0, pos) ;
		TAGdesc = TRSdesc.substr(pos+1, TRSdesc.size()-pos) ;
		done = true ;
	}
	else
	{
		TAGtype = TRSdesc ;
		TAGdesc = "" ;
		done = false ;
	}
	return done ;
}

