/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	TXTParser.h
 */

#ifndef _TXTPARSER_H_
#define _TXTPARSER_H_

#include <map>
#include <string>
#include <ag/agfio_plugin.h>
#include "TXTfile.h"
#include "DataModel/DataModel.h"

#define MIN_TURN_SIZE 2.0  // 2 second
#define MIN_SEG_SIZE  1.0	// 1 second
#define MIN_RESOLUTION 0.001 // 1 ms

/**
* @class 	TXTParser
* @ingroup	Formats
*
* Specific text parser for TEXT format.\n
*/
class TXTParser
{
	public:
		/**
		 * Constructor
		 * @param corpus		Default annotation corpus
		 * @param options		Annotation options
		 * @param fullmode		True for complete loading (presentation, layout, etc...),
		 * 						False for only loading model data.
		 * @note 				Except for external use, <em>fullmode</em> will
		 * 						mostly be set to True.
		 */
		TXTParser(const string& corpus, map<string,string>* options, bool fullmode);
		~TXTParser();

		/**
		 * Parses a TXT file.
		 * @param B			Specific handler
		 * @param data		Annotation model (using aglib API)
		 * @return			List of all AGids (Identifiers of AG-LIB elements
		 * 					created while parsing file)
		 */
		list<AGId> parse(TXTfile& B, tag::DataModel* data);

	private:
		void initialize(TXTfile& B, tag::DataModel* data);
		void processTurn(TXTfile&, tag::DataModel*);
		void processText(TXTfile&, tag::DataModel*, int notrack);
		void checkSpeakerId(const string& spkid, const string& spkinfo,
				tag::DataModel* data);
		void addSignal(tag::DataModel* data, int notrack);


		map<string,string>* m_options;
		string m_corpusName;
		bool m_initDone;
		string m_lang;
		string m_conventions;
		string m_signalFilename;
		map <string, string> m_spkids ;
		std::vector<string> m_unitId ;
		std::vector<string> m_segmentId ;
		std::vector<string> m_turnId ;
		std::vector<string> m_sectId ;
		std::vector<string> m_bgId ;
		std::vector<string> m_prevSpkid ;
		std::vector<float> m_prevTime ;
		float m_minSegSize ;
		float m_minTurnSize ;
		bool m_fullmode;
		RE* m_speakerRE;
};


#endif
