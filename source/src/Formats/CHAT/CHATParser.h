/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	CHATParser.h
 */

#ifndef _CHATPARSER_H_
#define _CHATPARSER_H_

#include <map>
#include <string>
#include <ag/agfio_plugin.h>
#include "Formats/CHAT/CHATfile.h"
#include "DataModel/DataModel.h"

#define MIN_TURN_SIZE 1.0	// 1s
#define MIN_SEG_SIZE  0.010	// 10ms

/**
* @class 	CHATParser
* @ingroup	Formats
*
* Specific text parser for CHAT format.\n
*/
class CHATParser
{
	public:
		/**
		 * Constrcutor
		 * @param corpus		Default annotation corpus
		 * @param options		Annotation options
		 * @param fullmode		True for complete loading (presentation, layout, etc...),
		 * 						False for only loading model data.
		 * @note 				Except for external use, <em>fullmode</em> will
		 * 						mostly be set to True.
		 */
		CHATParser(const string& corpus, map<string,string>* options, bool fullmode);
		~CHATParser();

		/**
		 * Parses a CHAT file.
		 * @param B			Specific handler
		 * @param data		Annotation model (using aglib API)
		 * @return			List of all AGids (Identifiers of AG-LIB elements
		 * 					created while parsing file)
		 */
		list<AGId> parse(CHATfile& B, tag::DataModel* data);

	private:
		void	initialize(CHATfile& B, tag::DataModel* data);
		void	processHeaders(CHATfile&, tag::DataModel*);
		void	processSpeakers(CHATfile&, tag::DataModel*);
		void	processTurn(CHATfile&, tag::DataModel*);
		void	addSignal(tag::DataModel* data, int notrack);

		map<string,string>*	p_options;
		map<string, string> spkids;

		bool	init_done;
		bool	eof;
		bool	fullmode;

		string	lang;
		string	conventions;
		string	corpusName;
		string	signalFilename;
		string	segmentId;
		string	unitId;
		string	turnId;
		string	sectId;
		string	prev_spkid;
		string	lastSPK;

		int		signalNbTracks;
		int		nbSegments;
		int		segmentOrder;

		float	startTime;
		float	endTime;
		float	minTurnSize;
		RE*		speakerRE;
		RE*		badSpeakerRE;
		RE*		timecodeRE;
};

#endif

