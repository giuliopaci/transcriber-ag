/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	LBL.h
 */

#ifndef _LBL_H_
#define _LBL_H_

#include <map>
#include <string>
#include <ag/agfio_plugin.h>
#include "Formats/LBL/LBLfile.h"
#include "DataModel/DataModel.h"

#define MIN_TURN_SIZE 1.0  // 1 second
#define MIN_SEG_SIZE  0.010	// 10 ms

/**
* @class 	LBL
* @ingroup	Formats
*
* LBL format plug-in for loading and saving LBL format files.\n
* This plug-in is used by the AG-LIB API.
*/
class DllExport LBL : public agfio_plugin
{
	public:
		LBL();
		~LBL();

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

		void initialize(LBLfile& B, tag::DataModel* data);

		void processTurn(LBLfile&, tag::DataModel*);
		bool processText(LBLfile&, tag::DataModel*, int notrack);
		const string& getDefaultSpeaker(const string& name, tag::DataModel* data);
		void addSignal(tag::DataModel* data, int notrack);

		map<string,string>* m_options;

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
		std::vector<string> m_unitId ;
		std::vector<string> m_segmentId ;
		std::vector<string> m_turnId ;
		std::vector<string> m_sectId ;
		std::vector<string> m_prevSpkid ;
		float m_lastTime ;
		float m_minSegSize ;
		float m_minTurnSize ;
		bool m_fullmode;
		string m_spkid;

		std::vector<string> import_warning ;

		RE* m_speakerRE;
};

#ifdef _LBL_IMPL
AGFIO_PLUGIN(LBL);
#endif // _LBL_IMPL

#endif
