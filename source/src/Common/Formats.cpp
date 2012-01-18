/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#include "Formats.h"
#include "util/Utils.h"
#include "FileInfo.h"
#include "globals.h"

#define TAG_FORMAT_FILENAME "formatsAG.rc"
#define TAG_FORMAT_COMP "PluginAG"

namespace tag
{

Formats* Formats::m_instance = NULL;


Formats* Formats::getInstance()
{
	return m_instance;
}

Formats::Formats()
{
	file = "" ;
}

Formats::~Formats()
{

}

void Formats::kill()
{
	if ( m_instance != NULL ) {
		delete m_instance;
		m_instance = NULL ;
	}
}

//******************************************************************************
//									 INITIALIZING
//******************************************************************************



void Formats::configure(string configuration_path)
{
	if ( m_instance == NULL ) {
		m_instance = new Formats();
		m_instance->load(configuration_path) ;
	}
}

bool Formats::load(string configuration_path)
{
	if (!file.empty()) {
		TRACE << "file formats mapping already loaded." << std::endl ;
		return false ;
	}

	TRACE << "Looking for formats mapping from " << configuration_path << std::endl ;

	//> check directory constrainsts
	if (configuration_path.empty()
				|| !Glib::file_test(configuration_path, Glib::FILE_TEST_EXISTS)
				|| !Glib::file_test(configuration_path, Glib::FILE_TEST_IS_DIR)
	)
	{
		file = "" ;
		TRACE << "<!> couldn't load file formats mapping at [" << file << "] <!>" << std::endl ;
		return false ;
	}

	//> compute file path
//	file = FileInfo(configuration_path).join(TAG_FORMAT_FILENAME) ;
	file = Glib::build_filename(configuration_path, TAG_FORMAT_FILENAME) ;

	TRACE << "Loading formats mapping from " << file << std::endl ;

	//> check file constrainsts
	if ( ! FileInfo(file).exists("f") )
	{
		TRACE << "<!> couldn't load file formats mapping at [" << file << "] <!>" << std::endl ;
		file = "" ;
		return false ;
	}

	try {
		//> load parameters
		formats_param.load(file);

		//> add TranscriberAG default entry
		formats_param.addSection(TAG_FORMAT_COMP, "TransAG", "Plugin format") ;
		formats_param.setParameterValue(TAG_FORMAT_COMP, "TransAG,display", _("TAG Annotation file"), true) ;
		formats_param.setParameterValue(TAG_FORMAT_COMP, "TransAG,extension", ".tag", true) ;
		formats_param.setParameterValue(TAG_FORMAT_COMP, "TransAG,type", "AGSet", true) ;
		formats_param.setParameterValue(TAG_FORMAT_COMP, "TransAG,io", "both", true) ;

		//> map
		loadMap() ;
		return true ;
	}
	catch ( const char* msg ) {
		TRACE << "<!> couldn't load file formats mapping at [" << file << "] <!>" << std::endl ;
		MSGOUT << "Sax error = " << msg << endl;
		file = "" ;
		return false ;
	}
	catch (...) {
		TRACE << "<!> couldn't load file formats mapping at [" << file << "] <!>" << std::endl ;
		MSGOUT << "Unknown error catched" << std::endl;
		file = "" ;
		return false ;
	}
}

void Formats::loadMap()
{
	std::map<std::string,std::string> pluginAGcomp ;
	std::map<std::string,std::string>::const_iterator it ;
	pluginAGcomp = formats_param.getParametersMap("PluginAG") ;

	for (it = pluginAGcomp.begin(); it != pluginAGcomp.end(); it++ )
	{
		bool ok = true ;

		string secparam = it->first ;
		string value = it->second ;

		//> don't accept line with label (with #) and line for only section (no ",")
		if (  secparam.find(",", 0) == string::npos  || secparam.find("#", 0)!= string::npos )
			ok = false ;

		if (ok && !secparam.empty())
		{
			guint pos = secparam.find_first_of(',', 0) ;
			string section = secparam.substr(0, pos) ;
			string param = secparam.substr(pos+1, secparam.size()-1) ;

			if (param == "extension") {
				formatTOextension[section] = value ;
				formats.push_back(section) ;
			}
			else if (param == "io")
				formatTOio[section] = value;
			else if (param == "type")
				formatFromType[value] = section;
		}
	}
}




//******************************************************************************
//									 ACCESSORS
//******************************************************************************

string Formats::getDisplayFromFormat(string format)
{
	if (format.empty())
		return "" ;

	string key = format + ",display" ;

	if (!formats_param.existsParameter(TAG_FORMAT_COMP, key) )
		return "" ;
	else
		return formats_param.getParameterValue(TAG_FORMAT_COMP, key) ;
}

string Formats::getExtensionFromFormat(string format)
{
	if (format.empty())
		return "" ;

	string key = format + ",extension" ;

	if (!formats_param.existsParameter(TAG_FORMAT_COMP, key) )
		return "" ;
	else
		return formats_param.getParameterValue(TAG_FORMAT_COMP, key) ;
}

string Formats::getTypeFromFormat(string format)
{
	if (format.empty())
		return "" ;

	string key = format + ",type" ;

	if (!formats_param.existsParameter(TAG_FORMAT_COMP, key) )
		return "" ;
	else
		return formats_param.getParameterValue(TAG_FORMAT_COMP, key) ;
}


string Formats::getFormatFromType(string type)
{
	if (type.empty())
		return "" ;

	return formatFromType[type] ;
}



bool Formats::isImport(string format)
{
	string io = formatTOio[format];

	return ( (io == "import") || (io == "both") );
}


bool Formats::isExport(string format)
{
	string io = formatTOio[format];

	return ( (io == "export") || (io == "both") );
}


} // namespace
