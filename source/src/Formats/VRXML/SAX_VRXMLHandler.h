/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// SAX_VRXMLHandler.h: VRXML format handler class
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

/* $Id */

/**
 * @file	SAX_VRXMLHandler.h
 */

#ifndef _SAX_VRXMLHANDLERS_H_
#define _SAX_VRXMLHANDLERS_H_

#include <list>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <glibmm.h>

#include <ag/AGTypes.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include "DataModel/DataModel.h"


#ifdef XERCES_HAS_CPP_NAMESPACE
XERCES_CPP_NAMESPACE_USE
#endif

/**
* @class 	SAX_VRXMLHandler
* @ingroup	Formats
*
* SAX VRXML handler class for Xerces C++ XML parser.
*/
class SAX_VRXMLHandler : public HandlerBase, private XMLFormatTarget
{

	public:
		/**
		 * Constructor
		 * @param data				Reference on the model AG used (will be filled while parsing)
		 * @param outEncoding		Output encoding
		 */
		SAX_VRXMLHandler(tag::DataModel& data, const string& outEncoding="UTF-8");
		~SAX_VRXMLHandler();

		/**
		 * Specifies output encoding.
		 * @param encoding		Encoding name
		 */
		void set_encoding(const string& encoding);


		void setScoreThreshold(float threshold) { m_threshold = threshold; }
		void setKeepSegmentation(bool b) { m_keepSegmentation=b; }

		void setCTSMode(bool b) { m_CTSMode = b; }

		/** Start element handler */
		void startElement(const XMLCh* const name, AttributeList& attr);

		/** End element handler */
		void endElement(const XMLCh* const name);

		/** Characters element handler */
		void characters(const XMLCh* const chars, const XMLSize_t length);

		/** Warning handler */
		void warning(const SAXParseException& exception);

		/** Error handler */
		void error(const SAXParseException& exception);

		/** Fatal error handler */
		void fatalError(const SAXParseException& exception);

	private:
		// stack class
		// The reason why we don't use STL stack class is that
		// STL stack didn't work with function pointers
		template<class T>
		class stack
		{
		private:
			int itop;
			T array[12];  // the depth of VRXML may be 9,  so 12 should do it

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

		tag::DataModel& 	m_dataModel;	// resulting AG-format data model
		float m_threshold;		// for high score tagging
		bool m_keepSegmentation; // keep "original" segmentation
		string m_lang;			// audio file name
		string m_audio;			// audio file name
		string m_path_hint;		// audio file path hint
		string m_scribe;		// current scribe
		int m_curtrack;			// current segment track
		map<string, string> m_spkids ; // spk ids map
		list<string> m_versionInfo;

		bool 	m_CTSMode;		// set CTS mode

		std::map<int, bool> m_firstWord;  //
		std::map<int, bool> m_firstSeg;  //
		std::map<int, string> m_sectId;	// current section id
		std::map<int, string> m_turnId;	// current turn id
		std::map<int, string> m_segmentId;	// current segment id
		std::map<int, string> m_unitId;	// current word id
		std::map<int, string> m_speaker;	// current speaker
		std::map<int, string> m_text;		// current segment text
		std::map<int, float> m_prevTime;	// last time offset seen
		std::map<int, float> m_lastWord;	// last time offset seen for word
		std::map<int, int> m_notrack;		// notrack associated to track id
		std::map<int, string> m_confidence; // high confidence terms offsets

		float m_minSectSize;
		float m_minTurnSize;
		float m_minSegSize;
		float m_duration;
		bool m_with_sect;
		bool m_enhanced;
		bool m_prev_enhanced;
		bool m_prev_is_special;
		bool m_firstSpeaker;


		// AGlib stuff
		XMLFormatter* formatter;
		string targetString;
		string localDTD;
		void writeChars(const XMLByte* const, const XMLSize_t, XMLFormatter* const);
		string& set_string(string&, const XMLCh* const);

		// StartFP: a type for function pointers to element handlers which are
		//     invoked when the start tag is detected
		// EndFP: a type for function pointers to element handlers which are
		//     invoked when the end tag is detected
		typedef void (SAX_VRXMLHandler::*StartFP)(const XMLCh* const, AttributeList&);
		typedef void (SAX_VRXMLHandler::*EndFP)(const XMLCh* const);


		// stacks for element handlers
		stack<StartFP> StartStack;
		stack<EndFP>   EndStack;

		// do-nothing handlers
		void dummyStart(const XMLCh* const name, AttributeList& attr);
		void dummyEnd(const XMLCh* const name);

		// element handlers
		void AudioDocStart(const XMLCh* const name, AttributeList& attr);
		void AudioDocEnd(const XMLCh* const name);
		void TransSubStart(const XMLCh* const name, AttributeList& attr);
		void ProcStart(const XMLCh* const name, AttributeList& attr);
		void ChannelStart(const XMLCh* const name, AttributeList& attr);
		void SpeakerStart(const XMLCh* const name, AttributeList& attr);
		void SegmentStart(const XMLCh* const name, AttributeList& attr);
		void SegmentEnd(const XMLCh* const name);
		void WordStart(const XMLCh* const name, AttributeList& attr);
		void WordEnd(const XMLCh* const name);

		virtual InputSource* resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId);
};

#endif
