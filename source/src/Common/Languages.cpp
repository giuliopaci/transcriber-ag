/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Languages.h"
#include "Parameters.h"
#include "util/StringOps.h"
#include <string>
#include <iostream>
#include "iso639.h"
#include "globals.h"

namespace tag {

// the Languages instance
Languages* Languages::m_instance = NULL;

Languages* Languages::getInstance()
{
	return m_instance;
}

void Languages::configure(const  Glib::ustring& path, const  Glib::ustring& mode)
{
	if ( m_instance != NULL ) delete m_instance;
	m_instance = new Languages(path, mode);
}

void Languages::kill()
{
	if ( m_instance != NULL ) {
		delete m_instance;
		m_instance = NULL ;
	}
}

Languages::Languages(Glib::ustring file_path, Glib::ustring mode)
{
	path = file_path ;
	initialised = false ;

	m_locale = ISO639::getLocale();

	if ( !mode.empty() ) initialise(mode);
}

Languages::~Languages()
{
}

int Languages::initialise(Glib::ustring mode)
{
	int res = 1 ;
	Glib::ustring languages_path = path ;

	Parameters param ;
	try {
		param.load(languages_path) ;
		const std::map<string, string>& m = param.getParametersMap(mode) ;

		std::map<string, string>::const_iterator it;
		for ( it = m.begin() ; it != m.end(); ++it) {
			vector<string> v;
			StringOps(it->first).split(v, ",;");
			if (v.size() == 1) {
				Glib::ustring code3 = v[0];
				Glib::ustring name = param.getParameterValue(mode, code3+","+m_locale);
				if ( name.empty() ) {// non-existent locale, use "eng"
					m_locale = "eng";
					name = param.getParameterValue(mode, code3+","+m_locale);
				}
				v.clear();
				StringOps(name).split(v, ";");
				name_from_code[code3]=v[0];
				code_from_name[v[0]]=code3 ;

				name = param.getParameterValue(mode, code3+",accents,"+m_locale);
				if ( !name.empty() ) {
					add_vect(accents, code3, name);
				}

				name = param.getParameterValue(mode, code3+",dialects,"+m_locale);
				if ( !name.empty() ) {
					add_vect(dialects, code3, name);
				}
			}
		}
		initialised=true ;
	}
	catch (const char* e) {
		Log::err() << "Languages::Languages:> PROBLEM LOADING FILE: " << e << std::endl ;
		res = -1 ;
	}

	return res ;
}
void Languages::add_vect(std::map<Glib::ustring, std::vector<Glib::ustring> >& store, Glib::ustring code3, Glib::ustring val)
{
	vector<string> v;
	vector<string>::iterator itv;
	vector<Glib::ustring> v2;
	StringOps(val).split(v, ",;");
	for ( itv=v.begin(); itv != v.end(); ++itv) v2.push_back(*itv);
	store[code3] = v2;
}

Glib::ustring Languages::get_code(Glib::ustring name) const
{
	std::map<Glib::ustring, Glib::ustring>::const_iterator it = code_from_name.find(name);
	if ( it !=  code_from_name.end() )
		return it->second;
	return "";
}

Glib::ustring Languages::get_name(Glib::ustring code) const
{
	std::map<Glib::ustring, Glib::ustring>::const_iterator it = name_from_code.find(code);
	if ( it !=  name_from_code.end() )
		return it->second;
	return "";
}

const std::vector<Glib::ustring>& Languages::get_codes()
{
	if (initialised && codes.empty() ) {
		std::map<Glib::ustring, Glib::ustring>::const_iterator it = name_from_code.begin() ;
		while (it != name_from_code.end()) {
			codes.push_back(it->first) ;
			it++ ;
		}
	}
	return codes;
}

const std::vector<Glib::ustring>& Languages::get_names()
{
	if (initialised && names.empty() ) {
		std::map<Glib::ustring, Glib::ustring>::const_iterator it = name_from_code.begin() ;
		while (it != name_from_code.end()) {
			names.push_back(it->second) ;
			it++ ;
		}
	}
	return names;
}

const std::vector<Glib::ustring>& Languages::get_accents(Glib::ustring lang)
{
	std::map< Glib::ustring, std::vector<Glib::ustring> >::iterator it;
	if ( (it=accents.find(lang)) != accents.end() )
		return it->second;
	return empty ;
}

const std::vector<Glib::ustring>& Languages::get_dialects(Glib::ustring lang)
{
	std::map< Glib::ustring, std::vector<Glib::ustring> >::iterator it;
	if ( (it=dialects.find(lang)) != dialects.end() )
		return it->second;
	return empty ;
}

void Languages::print()
{
	TRACE << "LANGUES::PRINT - nb= " << code_from_name.size() << std::endl ;
	std::map<Glib::ustring, Glib::ustring>::iterator it ;
	for (it = code_from_name.begin(); it!=code_from_name.end(); it++) {
		TRACE << "name= " << it->first << " -  code= " << it->second << std::endl ;
	}
}

} //namespace
