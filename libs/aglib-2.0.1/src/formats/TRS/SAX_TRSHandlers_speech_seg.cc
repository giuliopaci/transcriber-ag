// SAX_TransAGHandlers.cc: AIF element handler implementation
// Haejoong Lee, Xiaoyi Ma, Steven Bird
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

using namespace std;

#define atof myatof

#define TRACE(a) 

//#define TRACE(a) trace((a), __FILE__, __LINE__ )
static void trace(string a, string f, int l)
{
	cerr << "At " <<  f << "," << l << ": " << a << endl << flush;
}

static const char* itoa(int i)
{
	static char buf[10];
	sprintf(buf, "%d", i);
	return buf;
}

SAX_TRSHandlers::SAX_TRSHandlers(const string& encoding,
			       const string& dtd)
  : localDTD(dtd), prevValue("")
{
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
}

SAX_TRSHandlers::~SAX_TRSHandlers()
{ delete formatter; }

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
	TRACE(trans(name));
  // execute the handler at the top of the stack
  (this->*StartStack.top())(name, attr);
}

void SAX_TRSHandlers::endElement(const XMLCh* const name)
{
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
void SAX_TRSHandlers::TransStart
(const XMLCh* const name, AttributeList& attr)
{
	int no = 1;
	string prefix = "trs_";
	TRACE("IN TransStart");

	// make a unique Set ID
	agsetId = prefix; agsetId += itoa(no);
	while ( ExistsAGSet(agsetId) ) {
		++no;
		agsetId = prefix; agsetId += itoa(no);
	};
	
  prevId = CreateAGSet(agsetId);
  agIds.clear();    // erase ids of previous load
  agSetIds.push_back(agsetId);
	
      SetFeature(agsetId, "annotations", "transcription");
	
	ostringstream vers;
	vers << "<Version id=\"" << trans(attr.getValue("version")) 
		<< "\" date=\"" << trans(attr.getValue("version_date"))
		<< "\" author=\"" << trans(attr.getValue("scribe"))
		<< "\"/>";
    SetFeature(agsetId, "versions", vers.str());

	ostringstream status;
	status << "elapsed=" << trans(attr.getValue("elapsed")) << ";";
	
    SetFeature(agsetId, "status", status.str());
		
    timelineId = CreateTimeline(agsetId);
    signalId = CreateSignal(timelineId,trans(attr.getValue("audio_filename")),"audio","wav","PCM","secs","1");
	
	signalIds = GetSignals(timelineId);
	
    agId = CreateAG(agsetId,timelineId);
	lang = trans(attr.getValue("xml:lang"));
	
	SetFeature(agId, "lang", lang);
	prevId = agsetId;

	StartStack.push(&SAX_TRSHandlers::TransSubStart);
	EndStack.push(&SAX_TRSHandlers::dummyEnd);
	TRACE("OUT TransStart");


}


// invoked at the start of a subelement of Trans :
//  -> topics list
//  -> speakers list
//  -> episode start
void SAX_TRSHandlers::TransSubStart
(const XMLCh* const name, AttributeList& attr)
{
  string tag = trans(name);
	
		TRACE(string("IN TransSubStart ") + trans(name));

  // if Topics  element is found
  if (tag == "Topics") {
    StartStack.push(&SAX_TRSHandlers::TopicStart);
    EndStack.push(&SAX_TRSHandlers::TopicsEnd);
  }
  
  // if Speakers element is found
  else if (tag == "Speakers") {
    StartStack.push(&SAX_TRSHandlers::SpeakerStart);
    EndStack.push(&SAX_TRSHandlers::SpeakersEnd);
  }
  
  // if Episode element is found
  else if (tag == "Episode") {
 
  	TRACE(trans(name));
	SetFeature(signalId, "source", trans(attr.getValue("program")));
	SetFeature(signalId, "date", trans(attr.getValue("air_date")));
	  
  	TRACE(trans(name));
    prevId = CreateAG(agsetId, timelineId);
//	setElementFeatures(prevId, attrs, " program air_date ");
 
  	TRACE(trans(name));
    agIds.push_back(prevId);
    StartStack.push(&SAX_TRSHandlers::SectionStart);
    EndStack.push(&SAX_TRSHandlers::dummyEnd);
  }
  else {
	  TRACE("UNKNOWN TAG");
	   StartStack.push(&SAX_TRSHandlers::TransSubStart);
	EndStack.push(&SAX_TRSHandlers::dummyEnd);
  }
  	TRACE(string("OUT TransSubStart ") + trans(name));

}

// invoked at the start of a speaker definition
void
SAX_TRSHandlers::SpeakerStart
(const XMLCh* const name, AttributeList& attr)
{
	TRACE("IN SpeakerStart ");
	string scope =trans(attr.getValue("scope"));
	if ( scope.empty() ) scope = "local";
	speakers << "<Speaker id=\"" << trans(attr.getValue("id"))
       << "\" name.last=\"" << trans(attr.getValue("name"))
       << "\" gender=\"" << trans(attr.getValue("type"))
       << "\" scope=\"" << scope ;
	string slang = trans(attr.getValue("lang"));
	if ( slang == "" ) slang = lang;
	string dialect = trans(attr.getValue("dialect"));
	int isNative =0;
	if ( dialect == "native" ) { isNative=1; dialect = ""; }
	if ( dialect == "nonnative" ) { isNative=0; dialect = ""; }
	if ( slang != "" ) {
		speakers  << "\">" << endl;
		speakers << "<SpokenLanguage code=\"" << slang
			<< "\" dialect=\"" << dialect
			<< "\" isnative=\"" << isNative << "\" isusual=\"" << 1
			<< "\" accent=\"" << trans(attr.getValue("accent"))
			<< "\" />" << endl;
		speakers << "</Speaker>" << endl;
	} else 
		speakers  << "\"/>" << endl;

	StartStack.push(&SAX_TRSHandlers::dummyStart);
    EndStack.push(&SAX_TRSHandlers::dummyEnd);
	
	TRACE("OUT SpeakerStart ");

}

// invoked at the end of speakers list
// 		set agset speakers feature
void SAX_TRSHandlers::SpeakersEnd(const XMLCh* const name)
{
	TRACE("IN SpeakersEnd ");
	SetFeature(agsetId, "speakers", speakers.str());

	StartStack.pop();
	EndStack.pop();
	TRACE("OUT SpeakersEnd ");


}

// invoked at the start of a topic definition
void
SAX_TRSHandlers::TopicStart
(const XMLCh* const name, AttributeList& attr)
{
		TRACE("IN TopicStart ");

	topics << "<Topic id=\"" << trans(attr.getValue("id"))
	       << "\" desc=\"" << trans(attr.getValue("desc"))
			<< "\" />" << endl;
			TRACE("OUT TopicStart ");
	StartStack.push(&SAX_TRSHandlers::dummyStart);
    EndStack.push(&SAX_TRSHandlers::dummyEnd);

}

// invoked at the end of topics list
// 		set agset topics feature
void SAX_TRSHandlers::TopicsEnd(const XMLCh* const name)
{
			TRACE("IN TopicsEnd ");
	SetFeature(agsetId, "topics", topics.str());

	StartStack.pop();
	EndStack.pop();
				TRACE("OUT TopicsEnd ");

}




// invoked at the start of a section
void
SAX_TRSHandlers::SectionStart
(const XMLCh* const name, AttributeList& attr)
{
				TRACE("IN SectionStart ");
	prevId =
      CreateAnnotation(agId,
	                   getAnchor(trans(attr.getValue("startTime"))),
                       getAnchor(trans(attr.getValue("endTime"))),
                       "section");
	SetFeature(prevId, "type", trans(attr.getValue("type")));
	string topic = trans(attr.getValue("topic"));
	if ( ! topic.empty() ) {
		string topicId = CreateAnnotation(agId,
	                   getAnchor(trans(attr.getValue("startTime"))),
                       getAnchor(trans(attr.getValue("endTime"))),
					"topic");
		SetFeature(prevId, "topic", topic);  // back-info only
		SetFeature(topicId, "topic", topic);
	}
	
//	setElementFeatures(prevId, attrs, " time topic startTime endTime ");
 	
    StartStack.push(&SAX_TRSHandlers::TurnStart);
    EndStack.push(&SAX_TRSHandlers::dummyEnd);	
					TRACE("OUT SectionStart ");
}


// invoked at the start of a turn
void
SAX_TRSHandlers::TurnStart
(const XMLCh* const name, AttributeList& attr)
{
				TRACE("IN TurnStart ");
	string speakers = trans(attr.getValue("speaker"));
	turnIds.clear();
	string startAnchor, endAnchor;		
	
	if ( ! speakers.empty() ) {
		vector<string> v = Utilities::splitString(speakers, ' ');
		int i;

		// create a new turn per speakers
		for ( i=0; i < v.size(); ++i ) {
			startAnchor = getAnchor(trans(attr.getValue("startTime")), i+1);
			endAnchor = getAnchor(trans(attr.getValue("endTime")), i+1);
			prevId = CreateAnnotation(agId,
						   startAnchor, endAnchor,
						   "turn");
			turnIds.push_back(prevId );
			SetFeature(prevId , "speaker", v[i]);
			if ( v.size() > 1 ) SetFeature(prevId , "order", itoa(i+1));
			setElementFeatures(prevId , attr, " speakers startTime endTime ");
			prevId = CreateAnnotation(agId,
						   startAnchor, endAnchor,
						   "no_speech");
			segIds.push_back(prevId);
		}
		
	} else {
		startAnchor = getAnchor(trans(attr.getValue("startTime")), 1);
		endAnchor = getAnchor(trans(attr.getValue("endTime")), 1);
		prevId = CreateAnnotation(agId,
						   startAnchor, endAnchor,
						   "turn");
		setElementFeatures(prevId, attr, " speakers startTime endTime ");
		turnIds.push_back(prevId);
		prevId = CreateAnnotation(agId,
						   startAnchor, endAnchor,
						   "no_speech");
		segIds.push_back(prevId);

	}
	
	prevValue.erase();
	segStartTime = "";
	segEndTime = "";
	who = 1;
	nbwords = 0;
	
    StartStack.push(&SAX_TRSHandlers::TurnSubStart);
    EndStack.push(&SAX_TRSHandlers::TurnEnd);
				TRACE("OUT TurnStart ");	
}



// invoked at the start of a subelement of Turn : sync / who / background / event / word
void SAX_TRSHandlers::TurnSubStart
(const XMLCh* const name, AttributeList& attr)
{
	string tag = trans(name);
			TRACE(string("IN TurnSubStart ") + tag);
	
	if (tag == "Word") { 
		WordStart(name, attr);
		EndStack.push(&SAX_TRSHandlers::WordEnd);
	} else {
		if (tag == "Sync") SyncStart(name, attr);
		else if (tag == "Who") WhoStart(name, attr);
		else if ( tag == "Background"  ) BackgroundStart(name, attr);
		else if (tag == "Event" ) EventStart(name, attr);
		else if (tag == "Vocal" ) {
			prevId = CreateAnnotation(agId,
						GetStartAnchor(segIds[who-1]), GetStartAnchor(segIds[who-1]), 
						"vocal");
			string desc = trans(attr.getValue("desc"));
			SetFeature(prevId, "desc", desc);
		}	else if (tag == "Comment" ) {
			prevId = CreateAnnotation(agId,
						GetStartAnchor(segIds[who-1]), GetStartAnchor(segIds[who-1]), 
						"comment");
			string desc = trans(attr.getValue("desc"));
			SetFeature(prevId, "desc", desc);
		}
		EndStack.push(&SAX_TRSHandlers::dummyEnd);
	}
    StartStack.push(&SAX_TRSHandlers::TurnSubStart);
	TRACE(string("OUT TurnSubStart ") + tag);
}

// deal with "Word" start items
void SAX_TRSHandlers::WordStart
(const XMLCh* const name, AttributeList& attr)
{
	++nbwords;
	segStartTime = trans(attr.getValue("startTime"));
	segEndTime = trans(attr.getValue("endTime"));
	const string& startAnchor = getAnchor(segStartTime, who);
	const string& endAnchor = getAnchor(segEndTime, who);
	prevId = CreateAnnotation(agId,
						   startAnchor, endAnchor,
						   "word");
	if ( nbwords == 1 ) 
		setAnnotationType(segIds[who-1], "speech");
}

// deal with "Word" end items
void SAX_TRSHandlers::WordEnd
(const XMLCh* const name)
{
	SetFeature(prevId, "text", prevValue);
	prevValue.erase();
}


// deal with "sync" items
void SAX_TRSHandlers::SyncStart
(const XMLCh* const name, AttributeList& attr)
{
		if ( ! prevValue.empty() ) setLastSegmentText(who);		 
		segStartTime = trans(attr.getValue("startTime"));

		segEndTime = trans(attr.getValue("endTime"));
		if ( segStartTime.empty() ) {
			segStartTime = trans(attr.getValue("time"));
			segEndTime = "";
		}
		string startAnchor = getAnchor(segStartTime, who);
		string segStartAnchor = GetStartAnchor(turnIds[who-1]);
		if ( startAnchor != segStartAnchor ) 
			addSegment(who, startAnchor);
}


// deal with "who" items
void SAX_TRSHandlers::WhoStart
(const XMLCh* const name, AttributeList& attr)
{
	if ( ! prevValue.empty() ) setLastSegmentText(who);	
	who = atoi(trans(attr.getValue("nb")).c_str());
}
	

// deal with "background" items
void SAX_TRSHandlers::BackgroundStart
(const XMLCh* const name, AttributeList& attr)
{
	if ( ! prevValue.empty() ) setLastSegmentText(who);	
		
	string startTime = trans(attr.getValue("startTime"));
	string prevAnchor = GetStartAnchor(segIds[who-1]);
	string startAnchor = getAnchor(startTime);
		
	if ( startAnchor != prevAnchor ) 
		prevId = addSegment(who, startAnchor);
	else prevId = segIds[who-1];
		
	string endAnchor = GetEndAnchor(prevId);
	// if another background annotation has been created for current segment
	//  offsets range -> set its end anchor to new background
	const list<std::string>& bgids = GetAnnotationSeqByOffset(agId,
					atof(segStartTime.c_str()), GetAnchorOffset(endAnchor),
					"background");
	if ( bgids.size() > 0 ) 
		SetEndAnchor(bgids.back(), startAnchor);
	
	prevId = CreateAnnotation(agId,
					startAnchor, endAnchor, 
                    "background");
	SetFeature(prevId, "type", trans(attr.getValue("type")));
	SetFeature(prevId, "level", trans(attr.getValue("level")));
}


// deal with "event" items

void SAX_TRSHandlers::EventStart
(const XMLCh* const name, AttributeList& attr)
{
		string type = trans(attr.getValue("type"));
		string extent = trans(attr.getValue("extent"));
		string desc = trans(attr.getValue("desc"));
		string prevId = segIds[who-1];
		string remaining("");
		
		if ( extent == "previous" && ! prevValue.empty() ) {
			// we will immediately add corresponding text segment.
			// -> remove last word from prevValue;
			getLastWord(prevValue, remaining);
		}

		
		if ( ! prevValue.empty() ) {
			setLastSegmentText(who);	
			addSegment(who, string(""));
		}
		
		if ( extent == "begin" ) {
			// push event desc on "to be added" events stack
			eventStack.push_back(Event(who, GetStartAnchor(segIds[who-1]), type, desc, false));
			
		} else if ( extent == "previous" ) {
			// add corresponding text segment.
			if ( ! remaining.empty() ) {
				prevValue = remaining;
				setLastSegmentText(who);	
				addSegment(who, string(""));
			} else {
				// eventually split prev. text segment
				const string& endAnchor = GetStartAnchor(segIds[who-1]);
				const set<AnnotationId>& ids = GetIncomingAnnotationSet(endAnchor, "segment");
				if ( ids.size() == 0 ) 
					TRACE("No incoming annotations for previous event");
				else {
					set<AnnotationId>::const_iterator it ;
					for ( it =ids.begin(); it != ids.end(); ++it ) {
						if ( ExistsFeature(*it, "text") ) {
							prevValue = GetFeature(*it, "text");
							getLastWord(prevValue, remaining);
							if ( prevValue == "" ) { // no need to split seg
								prevId = CreateAnnotation(agId,
									GetStartAnchor(*it), GetEndAnchor(*it), 
									type);
							} else {  // need to split seg
								string curid = segIds[who-1];
								segIds[who-1] = *it;
								addSegment(who, ""); // to split seg
								SetFeature(*it, "text", prevValue);
								prevValue = remaining;
								setLastSegmentText(who);
								segIds[who-1] = curid;
							}
							break;
							if ( !desc.empty() ) SetFeature(prevId, "desc", desc);
						}
					}
				}
			}
			
		} else if ( extent == "next" ) {
			// arm next word event segment creation
			arm_next++;
			eventStack.push_back(Event(who, GetStartAnchor(segIds[who-1]), type, desc, true));
			
		} else {
	
			if ( extent == "end" ) {
				// "unstack" and apply corresponding event
				

				int itr;
				for ( itr = eventStack.size() -1 ; itr >= 0; --itr ) {
					if ( eventStack[itr].type == type 
							&& eventStack[itr].desc == desc && eventStack[itr].nextWord == false ) {
						prevId = CreateAnnotation(agId,
								eventStack[itr].startAnchor, GetStartAnchor(segIds[who-1]), 
								eventStack[itr].type);
						if ( !(eventStack[itr].desc.empty()) ) 
							SetFeature(prevId, "desc", eventStack[itr].desc);
						break;
					}
				}


				if ( itr == -1 )
					TRACE("Unmatched event type/desc");
				else {
					vector<Event>::iterator it = eventStack.begin();	
					while (itr > 0 ) { ++it; --itr; }
					eventStack.erase(it);
				}
					
			}
			
			else if ( extent == "instantaneous" ) {
				prevId = CreateAnnotation(agId,
						GetStartAnchor(segIds[who-1]), GetStartAnchor(segIds[who-1]), 
						type);
				if ( !desc.empty() ) SetFeature(prevId, "desc", desc);
			}
		}

}


// get last transcription word
void SAX_TRSHandlers::getLastWord(string& inbuf, string& word)
{
	unsigned int endpos = inbuf.size()-1;
	
	word="";
	while ( endpos > 0 && strchr(" \t\n", inbuf[endpos]) != NULL ) --endpos;
	unsigned int pos = inbuf.find_last_of(" \t\n", 0, endpos);
	if ( pos != string::npos ) {
		word = inbuf.substr(pos+1);
		inbuf.erase(pos+1);
	}
	pos = inbuf.find_first_not_of(" \t\n");
	if ( pos == string::npos ) inbuf = "";
}


//
// add new segment; split current segment and set new intermediate
//   anchor to startanchor if given
const string& SAX_TRSHandlers::addSegment(int who, const string& startAnchor)
{
	string& splitId = segIds[who-1]; 
	
	list<AnnotationId> ids = SplitAnnotation(splitId);
	list<AnnotationId>::iterator it = ids.begin();
	++it; splitId = *it;
	
	if ( !startAnchor.empty() ) {
			it = ids.begin(); SetEndAnchor(*it, startAnchor);
			SetStartAnchor(splitId, startAnchor);
	}
	setAnnotationType(splitId, "no_speech");
	if ( ExistsFeature(splitId, "text") ) DeleteFeature(splitId, "text");
	return splitId;
}

void  SAX_TRSHandlers::setAnnotationType(string& id, const char* type)
{
	if ( GetAnnotationType(id) != type ) {
		// we must delete and recreate annotation with appropriate new type
		// thus we lose any feature associated to it
		const AnchorId& start = GetStartAnchor(id);
		const AnchorId& end = GetEndAnchor(id);
		DeleteAnnotation(id);
      	CreateAnnotation(agId, start, end, type);
	}
}
 
//
// set transcription segment text
void SAX_TRSHandlers::setLastSegmentText(int who)
{
	string& id = segIds[who-1];
	
	if ( !prevValue.empty() ) {
		setAnnotationType(id, "speech");
		SetFeature(id, "text", prevValue);
	} else {
		setAnnotationType(id, "no_speech");
		if ( ExistsFeature(id, "text") ) DeleteFeature(id, "text");
	}

	prevValue.erase();
}


// invoked at the end of Turn
// 		if non-empty text -> store it in last created transcription segment
void SAX_TRSHandlers::TurnEnd(const XMLCh* const name)
{
	if ( ! prevValue.empty() ) setLastSegmentText(who);

	StartStack.pop();
	EndStack.pop();
}

// invoked when PCDATA encountered
void SAX_TRSHandlers::characters
(const XMLCh* const chars, const unsigned int length)
{
	string s;
	set_string(s, chars);
	
	if ( arm_next > 0 ) {	// apply-on-next-word events
		unsigned int pos = strspn(s.c_str(), " \t\n");
		unsigned int pos2 = s.find_first_of(" \t\n", pos);
		string back_prev = prevValue;

		if ( pos2 == string::npos ) {  // single word
			prevValue = s; s=""; 
		} else {
			prevValue = s.substr(pos, (pos2-pos));
			s = s.substr(pos2+1);
		}
		
		if ( prevValue.empty() ) { prevValue = back_prev; return; }
		setLastSegmentText(who);
		addSegment(who);
		prevValue = s;
		
		// "unstack" and apply corresponding event
		int i;
		for ( i = eventStack.size()-1; arm_next > 0 && i >= 0; --i ) {
			if ( eventStack[i].nextWord == true ) {
				prevId = CreateAnnotation(agId,
						eventStack[i].startAnchor, GetStartAnchor(segIds[who-1]), 
						eventStack[i].type);
				if ( !(eventStack[i].desc.empty()) )
					SetFeature(prevId, "desc", eventStack[i].desc);
				--arm_next;
			}
		}
		if ( arm_next > 0 ) {
				TRACE("Unmatched event type/desc");
		} else {
			vector<Event>::iterator it ;
			for ( it = eventStack.begin(); it != eventStack.end(); )
				if ( it->nextWord == true ) it=eventStack.erase(it);
				else ++it;
		}
	} else {
		 prevValue += s;
}
}

void SAX_TRSHandlers::warning(const SAXParseException& e)
{
	cerr << "WARNING: " << trans(e.getMessage()) << endl;
   cerr << " at line " << e.getLineNumber ()
       << " col " << e.getColumnNumber () << endl;
}

void SAX_TRSHandlers::error(const SAXParseException& e)
{
  cerr << "WARNING: " << trans(e.getMessage()) << endl;
   cerr << " at line " << e.getLineNumber ()
       << " col " << e.getColumnNumber () << endl;
}

void SAX_TRSHandlers::fatalError(const SAXParseException& e)
{
  cerr << "ERROR: " << trans(e.getMessage()) << endl;
   cerr << " at line " << e.getLineNumber ()
       << " col " << e.getColumnNumber () << endl;
  throw agfio::LoadError(trans(e.getMessage()));
}

void SAX_TRSHandlers::writeChars(const XMLByte* const toWrite,
                                 const unsigned int count,
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



/**
 *  setElementFeatures : store "variable" list of element attributes as 
 *          element features;
 */

void
SAX_TRSHandlers::setElementFeatures(const string& id, AttributeList& attr, const char* exclude)
{
	string tag;
	for ( int i = 0; i < attr.getLength(); ++i ) {
		set_string(tag, attr.getName(i));
		if ( strstr(exclude, tag.c_str()) == NULL )
			SetFeature(id, tag, trans(attr.getValue(i)));
	}
}

const string& SAX_TRSHandlers::getAnchor(const string& offset_str, int rank)
{
	float offset = atof(offset_str.c_str());
	char buf[20];
	sprintf(buf, "%5.3f  %d", offset, rank);
	map<string, string>::iterator it = anchors.find(buf);
	
	if (it == anchors.end() ) {
		anchors[buf] = CreateAnchor(agId, offset, "sec", signalIds);
		return anchors[buf];
	} else
		return it->second;
}
