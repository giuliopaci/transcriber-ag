/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	SAX_SGMLHandler.h
 */

#ifndef _SGML_H_
#define _SGML_H_

#include <ag/agfio_plugin.h>
#include <xercesc/parsers/SAXParser.hpp>
#include "Formats/SGML/SGMLobjects.h"

namespace tag {
class DataModel;
}

/**
* @class 	SGML
* @ingroup	Formats
*
* SGML format plug-in for loading and saving SGML format files.\n
* This plug-in is used by the AG-LIB API.
*/
class SGML : public agfio_plugin
{
	private:
		virtual list<AGId> load(const string& filename, const Id& id = "",
										map<string,string>* signalInfo = NULL,
										map<string,string>* options = NULL)
							throw (agfio::LoadError);

		virtual string store(const string& filename, const string& id,
									map<string,string>* options = NULL)
							throw (agfio::StoreError);

		void getFiles(tag::DataModel* data, string filepath) ;

		bool fillModels(tag::DataModel* data, SGMLobjects* objects, string& err) ;
		bool fillDisplayModel(tag::DataModel* data, string& err) ;
		bool fillAlignModel(tag::DataModel* data, SGMLobjects* structure, string& err) ;
		bool initializeFromScratch(SGMLobjects* pars, tag::DataModel* data) ;
		bool addGraph(tag::DataModel* data, const string& format, const string& graphtype, const string& lang, const string& scribe);


		bool checkTime(SGMLobjects::SGMLentry* entry, SGMLobjects::SGMLpath* path) ;
		string createAlignment(const string& alignType, float start, float end, const string& text, const string& graphType, const string& annotType, tag::DataModel* data) ;
		string createTranscriptionAnnotation(const string& prevId, float start, const string& text, tag::DataModel* data) ;
		bool isHandledType(const string& type, const string& mode) ;
		bool adjustPreviousEnd(const string last_id, float current_start, tag::DataModel* data) ;
		std::string getEncoding(string filepath) ;

	private:
		string m_audioFile ;
		string m_displayFile ;
		string m_displayType ;
		string m_sgmlFile ;
		float m_duration ;
		float m_lastOffset ;
		bool m_fromScratch ;
};


AGFIO_PLUGIN(SGML);

#endif
