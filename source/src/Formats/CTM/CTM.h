/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	CTM.h
 */


#ifndef _CTM_H_
#define _CTM_H_

#include <map>
#include <string>
#include <ag/agfio_plugin.h>
#include "CTMfile.h"
#include "DataModel/DataModel.h"

#define MIN_TURN_SIZE 2.  // 1 second
#define MIN_SEG_SIZE 2.  // 1 second
#define MIN_RESOL  .80	// 800 ms

/**
* @class 	CTM
* @ingroup	Formats
*
* CTM format plug-in for loading and saving CTM format files.\n
* This plug-in is used by the AG-LIB API.
*/
class DllExport CTM : public agfio_plugin
{
	public:
		CTM();
		~CTM();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
//> 	Test methods
		virtual list<AGId>
		pubload(const string& filename,
					const Id& id = "",
					map<string,string>* signalInfo = NULL,
					map<string,string>* options = NULL)
		throw (agfio::LoadError) { return load(filename, id, signalInfo, options); }
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

	private:
		int getNoTrack(Glib::ustring parsed_track)  ;

		void initialize(CTMfile& B, tag::DataModel* data);

		void processTurn(CTMfile&, tag::DataModel*);
		void processText(CTMfile&, tag::DataModel*, int notrack);
		void checkSpeakerId(const string& spkid, const string& spkinfo,
					tag::DataModel* data);
		void addSignal(tag::DataModel* data, int notrack);

		map<string,string>* p_options;

		virtual list<AGId>
		load(const string& filename,
					const Id& id = "",
					map<string,string>* signalInfo = NULL,
					map<string,string>* options = NULL)
		throw (agfio::LoadError);

		string corpusName;
		bool init_done;

		string lang;
		string conventions;
		int signalNbTracks;
		string signalFilename;
		map <string, string> spkids ;
		std::vector<string> wordId ;
		std::vector<string> segmentId ;
		std::vector<string> turnId ;
		std::vector<string> sectId ;
		std::vector<string> prev_spkid ;
		float startTime ;
		float prevTime ;
		float minSegmentSize ;
		float minTurnSize ;
		float maxSegSize ;	// maximum seg size -> will terminate current segment as soon as inter-word gap > minDiff is found
		float minDiff;		// min inter-word gap
		bool fullmode;
		bool need_new_segment ;

		std::vector<string> import_warning ;
		std::string nospeech;
		RE* speakerRE;
};

#ifdef _CTM_IMPL
AGFIO_PLUGIN(CTM);
#endif // _CTM_IMPL

#endif
