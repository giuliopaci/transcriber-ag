// SAX_TRSHandlers.h: TRS format handler class
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _SAX_TRSHANDLERS_H_
#define _SAX_TRSHANDLERS_H_

#include <list>
#include <map>
#include <string>
#include <vector>
#include <sstream>

#include <ag/AGTypes.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/framework/XMLFormatter.hpp>


#ifdef XERCES_HAS_CPP_NAMESPACE
XERCES_CPP_NAMESPACE_USE
#endif

/// SAX TRS handler class for Xerces C++ XML parser.
class SAX_TRSHandlers : public HandlerBase, private XMLFormatTarget
{
	class Event {
	public:
		Event(int w, const string& anchor="", const string& t="", const string& d="", bool next=false)
			: who(w), startAnchor(anchor), type(t), desc(d), nextWord(next) {}
		Event(const Event& ev) 
			: who(ev.who), startAnchor(ev.startAnchor), type(ev.type), desc(ev.desc), nextWord(ev.nextWord) {}
		int who;
		string startAnchor;
		string type;
		string desc;
		bool nextWord; 
	};
	
	  // stack class
  // The reason why we don't use STL stack class is that
  // STL stack didn't work with function pointers
  template<class T>
  class stack
  {
  private:
    int itop;
    T array[12];  // the depth of TRS may be 9,  so 12 should do it

  public:
    stack(): itop(0) {}

    T top()
    {
      return array[itop];
    }

    void push(T f)
    {
      if (itop < 12)
        array[++itop] = f;
      else
        throw ("stack overflow");
    }

    void pop()
    {
      if (itop > 0)
        itop--;
      else
        throw ("stack underflow");
    }
  };

	
private:
  list<AGId> agIds;  // contains id's of loaded AG's
  list<AGSetId> agSetIds;  // contains id's of loaded AGSets's
  set<SignalId> signalIds;  // contains id's of loaded AGSets's
	string timelineId;	// current timelineId
	string agsetId;	// current agsetId
	string agId;	// current agId
	string signalId;	// current signalId
	string lang;		// current transcription language
	vector<string> turnIds;  // overlapping speech -> one turn per speaker
	vector<string> segIds;  // last created speech/no_speech annotation id
	string turnEndAnchor;			// current end anchor
	string segStartTime;		// previous segment time
	string segEndTime;		// previous segment time
	ostringstream speakers;	// speakers
//	ostringstream topics;	// topics
std::map<std::string, std::string> topics;
	std::map<std::string, std::string> anchors;  // created timecoded anchors
	string bgId;   // last encountered background

	bool isDetailed;
	bool isAutomatic;
	int nbSegInTurn; 

  // they are needed by AIF::load(),
  // because it needs to return id's of AG's it has loaded
  string localDTD;

  string prevId;        // These are need to set features or medatada for
  string prevFeature;   // objects such as AGSet, Timeline, Signal and AG.
  string prevValue;     // They provide temporary storage.
	
	int who;		// current speaker for overlapping speech turns
	int arm_next;   // next-word events
	int nbwords;	// nb words in segment
		float prevScore;
	float lastOffset;	// last signal offset found

  unsigned int prevPos; //
  
  vector<Event> eventStack;

	std::map<string,string> eventDescCvt;

  XMLFormatter* formatter;
  string targetString;
  void writeChars(const XMLByte* const, const unsigned int, XMLFormatter* const);
  string& set_string(string&, const XMLCh* const);

  // StartFP: a type for function pointers to element handlers which are
  //     invoked when the start tag is detected
  // EndFP: a type for function pointers to element handlers which are
  //     invoked when the end tag is detected
  typedef void (SAX_TRSHandlers::*StartFP)(const XMLCh* const, AttributeList&);
  typedef void (SAX_TRSHandlers::*EndFP)(const XMLCh* const);


  // stacks for element handlers
  stack<StartFP> StartStack;
  stack<EndFP>   EndStack;

  // do-nothing handlers
  void dummyStart(const XMLCh* const name, AttributeList& attr);
  void dummyEnd(const XMLCh* const name);
  
  // element handlers
  void TransStart(const XMLCh* const name, AttributeList& attr);
  void TransEnd(const XMLCh* const name);
  void TransSubStart(const XMLCh* const name, AttributeList& attr);
  void EpisodeStart(const XMLCh* const name, AttributeList& attr);
  void SectionStart(const XMLCh* const name, AttributeList& attr);
  void TurnStart(const XMLCh* const name, AttributeList& attr);
  void TurnEnd(const XMLCh* const name);
  void TurnSubStart(const XMLCh* const name, AttributeList& attr);
  void SyncStart(const XMLCh* const name, AttributeList& attr);
  void LangStart(const XMLCh* const name, AttributeList& attr);
  void WordStart(const XMLCh* const name, AttributeList& attr);
  void WordEnd(const XMLCh* const name);
  void WhoStart(const XMLCh* const name, AttributeList& attr);
  void BackgroundStart(const XMLCh* const name, AttributeList& attr);
  void CommentStart(const XMLCh* const name, AttributeList& attr);
  void EventStart(const XMLCh* const name, AttributeList& attr);
  void SpeakerStart(const XMLCh* const name, AttributeList& attr);
  void TopicStart(const XMLCh* const name, AttributeList& attr);
  
  const string& addSegment(int spkno, const string& anchor="");
  void getLastWord(string& inbuf, string& word);
  void setLastSegmentText(int who);
  
	const string& getAnchor(const string& offset_str, int rank=1);
	void setElementFeatures(const string& id, AttributeList& attrs, const char* exclude);
	string getOrder(const string& id);


public:
  SAX_TRSHandlers(const string& outEncoding="UTF-8", const string& dtd="");
  ~SAX_TRSHandlers();

  void set_encoding(const string&);
  void set_localDTD(const string& dtd) { localDTD = dtd; }
  list<AGId> get_agids() { return agIds; }

  void startElement(const XMLCh* const name, AttributeList& attr);
  void endElement(const XMLCh* const name);
  void characters(const XMLCh* const chars, const unsigned int length);
  virtual InputSource* resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId);

  void warning(const SAXParseException& exception);
  void error(const SAXParseException& exception);
  void fatalError(const SAXParseException& exception);
};

#endif
