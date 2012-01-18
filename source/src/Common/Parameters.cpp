/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class Parameters
 *
 * Parameters...
 */

#include <fstream>
#include <iostream>
#include "Parameters.h"
#include "util/Log.h"
#include "util/StringOps.h"
#include <glibmm.h>

namespace tag {

#define TAG_PARAMETERS_DELIM ","

Parameters::Parameters() {
	a_path = "";
}

Parameters::~Parameters()
{
//	printf("DELETE PARAMETERS!\n");
}

void Parameters::load(const string& p_path) throw (const char*)
{
	a_path = p_path;
	reload();
}

/**
 *  Add all parameters specified in "users" in "system"
 *  if save true, save result in "system" file
 */
void Parameters::mergeUserParameters(Parameters* users, Parameters* system, bool save)
{
	std::map< std::string, std::map< std::string, std::string> >::iterator itU_comp ;
	string delim = TAG_PARAMETERS_DELIM ;
	int cpt_new = 0 ;
	int cpt_update = 0 ;
	bool _new ;

	for (itU_comp = (users->getMap()).begin() ; itU_comp != (users->getMap()).end() ; itU_comp++)
	{
		std::map<string, string> mapCompU = itU_comp->second ;
		std::map<string, string>::iterator itU_sectionParam ;

		for (itU_sectionParam = mapCompU.begin(); itU_sectionParam != mapCompU.end(); itU_sectionParam++ )
		{
			bool ok = true ;
			string secparam = itU_sectionParam->first ;
			//> don't accept line with label (with #) and line for only section (no ",")
			if (  secparam.find(",", 0) == string::npos  || secparam.find("#", 0)!= string::npos )
				ok = false ;

			if (ok) {
				if ( system->existsParameter(itU_comp->first, itU_sectionParam->first) ) {
					_new = false;
					cpt_update++ ;
				}
				else {
 					_new = true ;
					cpt_new++ ;
				}
				//TRACE << "> merging " << itU_comp->first << " - " << itU_sectionParam->first << " (nw:" << _new << ")" << std::endl ;
				string value = users->getParameterValue(itU_comp->first, itU_sectionParam->first) ;
				string label = users->getParameterLabel(itU_comp->first, itU_sectionParam->first) ;
				//TRACE << "Value= " << value << " - Label= " << label << std::endl ;
				system->setParameterValue(itU_comp->first, itU_sectionParam->first, value, true) ;
				system->setParameterLabel(itU_comp->first, itU_sectionParam->first, label, true) ;
			}
		}
	}
	if (save)
		system->save() ;
}

/**
 *  For each parameter, check if it exists in "old", and copy that value in "newp"
 */
void Parameters::updateUserParameters(Parameters* old, Parameters* newp, bool save)
{
	int cpt_updated = 0 ;

	//> go over all old parameters, and set their value in new map if they exist
	std::map< std::string, std::map< std::string, std::string> >::iterator itOld_comp ;

	for (itOld_comp = (old->getMap()).begin() ; itOld_comp != (old->getMap()).end() ; itOld_comp++)
	{
		std::map<string, string> mapCompOld = itOld_comp->second ;
		std::map<string, string>::iterator itOld_sectionParam ;

		for (itOld_sectionParam = mapCompOld.begin(); itOld_sectionParam != mapCompOld.end(); itOld_sectionParam++ )
		{
			bool ok = true ;
			string secparam = itOld_sectionParam->first ;
			//> don't accept line for displaying label (with #) and for displaying only section (no ",")
			if (  secparam.find(",", 0) == string::npos  || secparam.find("#", 0)!= string::npos )
				ok = false ;

			//> if exists in new param, get old values and replace them in new map
			if (ok && newp->existsParameter(itOld_comp->first, itOld_sectionParam->first)) {
				string value =  old->getParameterValue(itOld_comp->first, itOld_sectionParam->first) ;
				string label =  old->getParameterLabel(itOld_comp->first, itOld_sectionParam->first) ;
				newp->setParameterValue(itOld_comp->first, itOld_sectionParam->first, value, false) ;
				newp->setParameterLabel(itOld_comp->first, itOld_sectionParam->first, label, false) ;
				cpt_updated ++ ;
			}
		}
	}
	if (save)
		newp->save() ;
}

/** Return paramerer map pointer that can be modified (not const) **/
std::map<string, string>* Parameters::getAndModifyParametersMap(const string& p_idComponent)
{
	return &a_components[p_idComponent];
}

/**
 *  Print a parameter map in standard out
 *  if only_param is true, skip lines with only label or section description
 */
void Parameters::print(bool only_param)
{
	std::map< string, std::map< string, string> >::iterator itU_comp ;
	TRACE << "--------------------->> Print parameters from " << a_path << " ---------------------" << std::endl ;
	string delim = TAG_PARAMETERS_DELIM ;
	int cpt = 0 ;

	for (itU_comp = a_components.begin() ; itU_comp != a_components.end() ; itU_comp++)
	{
		std::map<string, string> mapCompU = itU_comp->second ;
		std::map<string, string>::iterator itU_sectionParam ;

		for (itU_sectionParam = mapCompU.begin(); itU_sectionParam != mapCompU.end(); itU_sectionParam++ )
		{
			bool ok = true ;
			//> just display lines with parameters, disabling only section or label lines
			if (only_param) {
				string secparam = itU_sectionParam->first ;
				//> don't accept line with label (with #) and line for only section (no ",")
				if (  secparam.find(",", 0) == string::npos  || secparam.find("#", 0)!= string::npos )
					ok = false ;
			}

			if (ok) {
				TRACE << "- " << itU_comp->first << ":" << itU_sectionParam->first << ":" << itU_sectionParam->second << std::endl ;
				cpt++ ;
			}
		}
	}
	TRACE << "> Printed (" << cpt << ")" << std::endl ;
}

void Parameters::reload() throw (const char*)
{
	if (!Glib::file_test(a_path, Glib::FILE_TEST_EXISTS))
	{
		string msg = "Could not find file " + a_path ;
		Log::err() << msg << std::endl ;
		throw msg.c_str() ;
	}

	try
	{
		a_components.clear();
		SAXParametersHandler handler(&a_components);
		CommonXMLReader reader(&handler);
		reader.parseFile(a_path);
	}
	catch (const char* msg )
	{
		MSGOUT << msg << endl;
		throw msg;
	}
}


bool Parameters::save()
{
	ofstream file(a_path.c_str());
	if ( ! file.good() ) {
		Log::err() << "Could not open file " << a_path << " for writing" << endl;
		file.close() ;
		return false ;
	}

	file << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
	file << "<!DOCTYPE parameters SYSTEM \"configurationAG.dtd\">" << endl;
	file << "<parameters>\n";

	std::map<string, std::map<string, string> >::iterator itA = a_components.begin();
	while (itA != a_components.end())
	{
		const string& s1 = itA->first;
		file << "\t<component id=\"" << s1 << "\">\n";
		std::map<string, string> labelsSections;
		std::map<string, std::map<string, std::map<string, string> > > parametersSections;
		std::map<string, string>& s2 = itA->second;
		std::map<string, string>::iterator itB = s2.begin();
		while (itB != s2.end()) {
			const string& s3 = itB->first;
			const string& s4 = itB->second;
			int virg = 0;
			int pos1 = s3.find(TAG_PARAMETERS_DELIM);
			int pos2 = -1;
			if (pos1 != -1) {
				virg++;
				//pos2 = s3.find(',', pos1+1);
				pos2 = s3.find(",#label");
				if (pos2 != -1) {
					virg++;
				} else pos2 = s3.length();
			}
			if (virg == 0) {
				labelsSections[s3] = s4;
				std::map<string, std::map<string, string> > m;
				parametersSections[s3] = m;
			}
			else if (virg >= 1) {
				string s5 = s3.substr(0, pos1);
				string s6 = s3.substr(pos1+1, pos2-pos1-1);
				//parametersSections[s5][s6]["label"] = a_components[s1][s3+",#label"];
				if ( virg == 2 )
				parametersSections[s5][s6]["label"] = s4;
				else
				parametersSections[s5][s6]["value"] = s4;
			}
			itB++;
		}
		itB = labelsSections.begin();
		while (itB != labelsSections.end()) {
			const string& s3 = itB->first;
			const string& s4 = itB->second;
			file <<  "\t\t<section id=\"" << s3 << "\" label=\"" << s4 << "\">\n";
			std::map<string, std::map<string, string> >::iterator itC = parametersSections[s3].begin();
			while (itC != parametersSections[s3].end()) {
				const string& s5 = itC->first;
				std::map<string, string>& s6 = itC->second;
				file <<  "\t\t\t<parameter id=\"" << s5 << "\" label=\"" << s6["label"] << "\" value=\"" << s6["value"] << "\"/>\n";
				itC++;
			}
			file <<  "\t\t</section>\n";
			itB++;
		}
		file <<  "\t</component>\n";
		itA++;
	}
	file <<  "</parameters>\n";

	file.close();

	return true ;
}

bool Parameters::existsParametersMap(const string& p_idComponent) {
	return (a_components.find(p_idComponent) != a_components.end());
}

const std::map<string, string>& Parameters::getParametersMap(const string& p_idComponent)
{
	return a_components[p_idComponent];
}

bool Parameters::existsParameter(const string& p_idComponent, const string& p_idSectionParam) {
	return (a_components[p_idComponent].count(p_idSectionParam) > 0);
}

const string& Parameters::getParameterLabel(const string& p_idComponent, const string& p_idSectionParam) {
	return a_components[p_idComponent][p_idSectionParam + string(",#label")];
}

const string& Parameters::getParameterValue(const string& p_idComponent, const string& p_idSectionParam) {
	return a_components[p_idComponent][p_idSectionParam];
}

bool Parameters::setParameterLabel(const string& p_idComponent, const string& p_idSectionParam, const string& p_label, bool p_create) {
	if (existsParameter(p_idComponent, p_idSectionParam)) {
		a_components[p_idComponent][p_idSectionParam + string(",#label")] = p_label;
		return true;
	}
	else {
		if (p_create) {
			a_components[p_idComponent][p_idSectionParam] = string("");
			a_components[p_idComponent][p_idSectionParam + string(",#label")] = p_label;
			return true;
		}
		else return false;
	}
	return false;
}

bool Parameters::setParameterValue(const string& p_idComponent, const string& p_idSectionParam, const string& p_value, bool p_create) {
	if (existsParameter(p_idComponent, p_idSectionParam)) {
		a_components[p_idComponent][p_idSectionParam] = p_value;
		return true;
	}
	else {
		if (p_create) {
			a_components[p_idComponent][p_idSectionParam] = p_value;
			a_components[p_idComponent][p_idSectionParam + string(",#label")] = string("");
			return true;
		}
		else return false;
	}
	return false;
}

void Parameters::addSection(const string& p_idComponent, const string& p_idSection, const string& p_label) {
	a_components[p_idComponent][p_idSection] = p_label;
}

} //namespace

/**
*  SAX Parameters handler
*/
void SAXParametersHandler::startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs) {

	string name;
	getString(localname, name);

	map<string, string> attmap;
	getAttributes(attrs, attmap);

	if (name == "component") {
		std::map<string, string> m;
		(*a_parameters)[attmap["id"]] = m;
		a_tmpComponent = &((*a_parameters)[attmap["id"]]);
	}
	else if (name == "section" ) {
		(*a_tmpComponent)[attmap["id"]] = attmap["label"];
		a_tmpSectionID = attmap["id"];
	}
	else if (name== "parameter") {
		(*a_tmpComponent)[a_tmpSectionID + string(TAG_PARAMETERS_DELIM) + attmap["id"]] = attmap["value"];
		(*a_tmpComponent)[a_tmpSectionID + string(TAG_PARAMETERS_DELIM) + attmap["id"] + string(",#label")] = attmap["label"];
	}
}
