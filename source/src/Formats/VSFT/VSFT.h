/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	VSFT.h
 */

#ifndef _VSFT_H_
#define _VSFT_H_

#include <map>
#include <string>
#include <ag/agfio_plugin.h>
#include "VSFTfile.h"
#include "DataModel/DataModel.h"
#include <ag/RE.h>

/**
* @class 	VSFT
* @ingroup	Formats
*
* VSFT format plug-in for loading and saving VSFT format files.\n
* This plug-in is used by the AG-LIB API.
*/
class DllExport VSFT : public agfio_plugin
{
	public:
		VSFT();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
		//> 	Test methods
		virtual list<AGId> pubload(const string& filename, const Id& id = "",
										map<string,string>* signalInfo = NULL,
										map<string,string>* options = NULL)
										throw (agfio::LoadError)
		{ return load(filename, id, signalInfo, options); }
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

	private:
		void initialize(VSFTfile& B, tag::DataModel* data);

		void processTurn(VSFTfile&, tag::DataModel*);
		void processText(VSFTfile&, tag::DataModel*);

		map<string, void (VSFT::*)(VSFTfile&, tag::DataModel* )> funcMap;
		map<string,string>* p_options;

		virtual list<AGId> load(const string& filename,
				const Id& id = "",
				map<string,string>* signalInfo = NULL,
				map<string,string>* options = NULL)
		throw (agfio::LoadError);

		string corpusName;
		bool init_done;
		bool fullmode;

		string lang;
		string conventions;
		float minsz;
		bool ok_duration;
		bool do_pi;
		string signalFormat;
		string signalEncoding;
		int signalNbTracks;
		string signalFilename;

		float m_startTime;
		float m_endTime;
		float m_duration;
		map <string, string> m_spkids;
		string m_unitId;
		string m_segmentId;
		string m_turnId;

		bool add_nospeech_sect;
		float elapsed;
		bool prev_is_pi;
		string prev_spkid;
		float filesz;

		std::vector<string> import_warning ;
		RE* speakerRE;
};

#ifdef _VSFT_IMPL
		AGFIO_PLUGIN(VSFT);
#endif // _VSFT_IMPL

#endif
