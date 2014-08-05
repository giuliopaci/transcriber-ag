/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// SAX_TRSHandlers.h: TRS format handler class
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

/* $Id */

/**
 * @file	SAX_TRSHandlers.h
 */

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

#include "Common/Parameters.h"
#include "DataModel/DataModel.h"

#ifdef XERCES_HAS_CPP_NAMESPACE
XERCES_CPP_NAMESPACE_USE
#endif

/**
* @class 	SAX_TRSHandlers
* @ingroup	Formats
*
* SAX TRS handler class for Xerces C++ XML parser.
*/
class SAX_TRSHandlers : public HandlerBase, private XMLFormatTarget
{
	public:
		/**
		 * Constructor
		 * @param outEncoding		Output encoding
		 * @param dtd				Dtd
		 */
		SAX_TRSHandlers(const string& outEncoding="UTF-8", const string& dtd="");
		~SAX_TRSHandlers();

		/**
		 * Specifies output encoding.
		 * @param encoding		Encoding name
		 */
		void set_encoding(const string& encoding);

		/**
		 * Specifies local dtd.
		 * @param dtd		Local dtd file
		 */
		void set_localDTD(const string& dtd) { localDTD = dtd; }

		/**
		 * Accessor to all AG elements created while parsing file.
		 * @return		A list containing identifiers of all created AG elements.
		 */
		list<AGId> get_agids() { return agIds; }

		/**
		 * Specifies Parameters object loaded from mapping file.\n
		 * Mapping file indicates which convention should be used for the
		 * corresponding format, and how representing some data from input format
		 * to internal format.
		 * @param p_mapping		Pointer on parameters object use
		 */
		void set_mapping(tag::Parameters* p_mapping) {mapping = p_mapping ; }

		/** Start element handler */
		void startElement(const XMLCh* const name, AttributeList& attr);

		/** End element handler */
		void endElement(const XMLCh* const name);

		/** Characters element handler */
		void characters(const XMLCh* const chars, const unsigned int length);

		/** Warning handler */
		void warning(const SAXParseException& exception);

		/** Error handler */
		void error(const SAXParseException& exception);

		/** Fatal error handler */
		void fatalError(const SAXParseException& exception);

		/**
		 * Specifies the model to use.
		 * @param model		Pointer on the AG model
		 * @note			This model should be filled while file parsing.\n
		 * 					In new plug-in version, this is how it works.
		 */
		void setTAGmodel(tag::DataModel* model) { dataModel = model ;}

		/**
		 * Accessor to the warnings encountered during the parsing
		 * @return			Vector with all the warnings found.
		 */
		const vector<string>& getMappingWarnings() { return mapping_qual_error ; }

	private:
		/**
		* @class 	SyncContent
		* @ingroup	Formats
		*
		* Sync tag content representation.\n
		* This class is dedicated to store all content found since the previous SYNC tag. It can be
		* event, text, word.s
		*/
		class SyncContent
		{
			public:
				typedef enum { TXT, WORD, EVENT_INSTANT, EVENT_BEGIN, EVENT_END, EVENT_PREVIOUS, EVENT_NEXT } SyncContentMode ;

				/**< Constructor for event*/
				SyncContent(int who, SyncContentMode mode, string type, string desc) ;

				/**< Constructor for text*/
				SyncContent(int who, string text) ;

				/**< Constructor for word*/
				SyncContent(int who, string text, string score, float start, float end) ;

				/** < debug purpose **/
				string print() ;

 	#ifndef DOXYGEN_SHOULD_SKIP_THIS
				// text(-1) - instantaneous(0) - start(1) - end(2)
				SyncContentMode mode ;
				// speaker
				int who ;
				// event qual
				string type ;
				string desc ;
				// text
				string txt ;
				// word
				string score ;
				float start ;
				float end ;
//				bool nextWord;
	#endif /* DOXYGEN_SHOULD_SKIP_THIS */

			private :
				void init() ;

		} ;


		/**
		* @class 	stack
		* @ingroup	Formats
		*
		* The reason why we don't use STL stack class is that
		* STL stack didn't work with function pointers
		*/
		template<class T> class stack
		{
			private:
				int itop;
				T array[12];  // the depth of TRS may be 9,  so 12 should do it

			public:
				/**
				 * Constructor
				 */
				stack(): itop(0) {}

				/**
				 * Accessor to stack top element
				 * @return		Top element
				 */
				T top()
				{
					return array[itop];
				}

				/**
				 * Adds an element to the stack
				 * @param f
				 */
				void push(T f)
				{
					if (itop < 12)
						array[++itop] = f;
					else
						throw ("stack overflow");
				}

				/**
				 * Removes the top element from the stack
				 */
				void pop()
				{
					if (itop > 0)
						itop--;
					else
						throw ("stack underflow");
				}
		};

		/** qualifiers mapping **/
		tag::Parameters* mapping ;
		std::vector<string> mapping_qual_error ;

		tag::DataModel* dataModel ;

		list<AGId> agIds;  // contains id's of loaded AG's
		list<AGSetId> agSetIds;  // contains id's of loaded AGSets's
		vector<SignalId> signalIds;  // contains id's of loaded AGSets's
		string timelineId;	// current timelineId
		string agsetId;	// current agsetId
		string agId;	// current agId
		string signalId;	// current signalId
		string lang;		// current transcription language

		vector<string> turnIds;  // overlapping speech -> one turn per speaker
		vector<string> segIds;  // last created speech/no_speech annotation id
		vector<string> baseIds;  // last created base annotation id

		string segStartTime;		// previous segment time
		string segEndTime;		// previous segment time
		ostringstream speakers;	// speakers
		//	ostringstream topics;	// topics
		std::map<std::string, std::string> topics;

		/* MAP graphId - backgroundGraphId */
		std::map<std::string, std::string> background_ids ;
		string getBackgroundAnchor(const string& offset_str) ;
		string getBackgroundAnchor(float offset) ;


		bool isAutomatic;

		// they are needed by AIF::load(),
		// because it needs to return id's of AG's it has loaded
		string localDTD;

		int who;		// current speaker for overlapping speech turns
		int arm_next;   // next-word events
		int nbwords;	// nb words in segment
		float prevScore;
		float lastOffset;	// last signal offset found

		unsigned int prevPos; //

		std::map<int, string> m_backId ;	// current background id
		std::map<int, string> m_sectId ;	// current section id
		std::map<int, string> m_turnId ;	// current turn id
		std::map<int, string> m_segmentId ;	// current segment id
		std::map<int, string> m_unitId ; 	//

		/* stack for SYNC tag data storing (evants & text) */
		std::map<int, vector<SyncContent*> > syncStack ;

		/* previous & current turn data */
		string lastTurnSpeakersStr ;
		map<string,string> lastTurnAttribute ;
		string currentTurnSpeakerStr ;
		map<string,string> currentTurnAttribute ;

		/* previous background data */
		set<string> anchorsToLink ;
		string lastBackgroundLevel ;
		string lastBackgroundType ;
		string prevBackgroundId ;
		string lastBackgroundTime ;

		/* previous & current section data */
		string lastSectionType ;
		string lastSectionTopic ;
		string currentSectionType ;
		string currentSectionTopic ;

		/* number of overlapping segment encountered */
		int nbSyncOverlap ;
		/* number of Turn tags encountered */
		int nbTurns ;
		/* number of Section tags encountered */
		int nbSections ;
		/* number of Background tags encountered */
		int nbBackground ;

		int nbSyncs ;
		/* flag for indicating that a section should be terminated at next sync tag */
		bool sectionNeeded ;
		/* flag for indicating that a turn should be terminated at next sync tag */
		bool turnNeeded ;

		virtual InputSource* resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId);

		/* Background business  */
		string formatBackground(string& back_type) ;
		string formatBackgrounds(string& back_chain) ;
		void linkBackgroundAnchors() ;

		/* Qualifier business  */
		void formatAGqualifier(bool is_entity, string& typeToMap, string& descToMap) ;
//		void checkAGqualifier(const string& TAGtype, const string& TAGdesc, const string& TRStype, const string& TRSdesc) ;
		bool getEntityTypeAndDesc(const string& TRSdesc, string& TAGtype, string& TAGdesc) ;

		/* Model business */
		void addOverlapSegments(const string& prevUnit, vector<string>& prevUnits) ;
		void addTurns(const string& prevUnit, const string& speakers, const map<string,string>& attributes) ;
		void addSection(const string& prevUnit, const string& type, const string& topic) ;
		void addSyncContent(vector<string>& prevUnit) ;
		string createQualifier(const string& type, const string& desc, const string& startAnchor, const string& endAnchor, const string& order) ;
		string createQualifier(const string& type, const string& desc, const string& prevId, const string& order) ;

		/* Parsed data business */
		void printSyncStack() ;
		void resetSyncStack() ;

		/* Util */
		void saveElementFeatures(AttributeList& attr, map<string,string>& mapattr) ;
		void setElementFeatures(const string& id, const map<string,string>& attr, const char* exclude) ;
		string getOrder(const string& id);

 		template <class NUMBER>
 		static string number_to_string(NUMBER number)
		{
			string res ;
			std::ostringstream ostr ;
			ostr << number ;
			res = ostr.str() ;
			return res ;
		}

		/* SAX XML PARSER BUSINESS */
		XMLFormatter* formatter;
		string targetString;
		void writeChars(const XMLByte* const, const XMLSize_t, XMLFormatter* const);
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
//		void WordStart(const XMLCh* const name, AttributeList& attr);
		void WordEnd(const XMLCh* const name);
		void WhoStart(const XMLCh* const name, AttributeList& attr);
		void BackgroundStart(const XMLCh* const name, AttributeList& attr);
		void CommentStart(const XMLCh* const name, AttributeList& attr);
		void VocalStart(const XMLCh* const name, AttributeList& attr);
		void EventStart(const XMLCh* const name, AttributeList& attr);
		void SpeakerStart(const XMLCh* const name, AttributeList& attr);
		void TopicStart(const XMLCh* const name, AttributeList& attr);
};

#endif
