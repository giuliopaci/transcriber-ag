/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	MDTMParser.h
 */

#ifndef _MDTMPARSER_H_
#define _MDTMPARSER_H_

#include <map>
#include <string>
#include <ag/agfio_plugin.h>
#include "Formats/MDTM/MDTMfile.h"
#include "DataModel/DataModel.h"

#define MIN_TURN_SIZE 1.0  // 1 second
#define MIN_SEG_SIZE  0.010	// 10 ms

/**
* @class 	MDTMParser
* @ingroup	Formats
*
* Specific text parser for MDTM format.\n
*/
class MDTMParser
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
		MDTMParser(const string& corpus, map<string,string>* options, bool fullmode);
		~MDTMParser();

		/**
		 * Parses a MDTM file.
		 * @param B			Specific handler
		 * @param data		Annotation model (using aglib API)
		 * @return			List of all AGids (Identifiers of AG-LIB elements
		 * 					created while parsing file)
		 */
		list<AGId> parse(MDTMfile& B, tag::DataModel* data);

	private:
		void initialize(MDTMfile& B, tag::DataModel* data);

		void processTurn(MDTMfile&, tag::DataModel*);
		void addSignal(tag::DataModel* data, int notrack);
		void checkSpeakerID(const string& spkID,
							const string& spkGender,
							tag::DataModel* data);

		map<string, string>*	m_options;
		map<string, string>		m_spkids;

		bool	init_done;
		bool	eof;
		bool	fullmode;

		string	lang;
		string	conventions;
		string	corpusName;
		string	signalFilename;
		string	m_unitId;
		string	m_segmentId;
		string	m_turnId;
		string	m_sectId;
		string	m_prevSpkid;
		float	m_lastTime;


		int		signalNbTracks;
		int		nbSegments;
};


#endif
