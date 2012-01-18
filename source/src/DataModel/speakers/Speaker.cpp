/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class Speaker
 *  @brief speaker definition
 *
 */

#include <map>
#include <string.h>
#include "Speaker.h"
#include "Common/util/StringOps.h"
#include "Common/globals.h"
#include "Common/util/Log.h"

using namespace std;

namespace tag {

const string Speaker::NO_SPEECH = "no_speech";  // no speech id
const string Speaker::noPropertyValue = ""; // no property value

const Speaker::Language Speaker::NO_LANGUAGE;


/*
*  constructors
*/
Speaker::Speaker()  {
		 setGender(UNDEF_GENDER);
		 setScope(LOCAL_SCOPE);
}

Speaker::Speaker(string id, string lastname, string firstname, Gender gender, Scope scope)
{
	setId(id);
	setFirstName(firstname);
	setLastName(lastname);
	setGender(gender);
	setScope(scope);
}

Speaker::Speaker(const Speaker& copy)
{
	m_id = copy.getId();
	m_properties = copy.getProperties(); m_spokenLanguages = copy.getLanguages();
	m_gender = copy.getGender();
	m_scope = copy.getScope();

}

/*
* generic properties setter
*/
void Speaker::setProperty(const std::string& name, const std::string& value)
{
	if ( name == "id" ) setId(value);
	else if ( name == "gender" ) setGender(value);
	else if ( name == "scope") setScope(value);
	else {
		m_properties[name] = value;
		mkFullName();
	}
}

const std::string& Speaker::getProperty(const std::string& name) const
{
	map<string, string>::const_iterator it = m_properties.find(name);
	if ( it == m_properties.end() ) return noPropertyValue;
	return it->second;
}

/*
*  combine name.last + name.first to make speaker full namespace
*	if still empty, full name = id
*/
void Speaker::mkFullName()
{
	m_properties["name.full"] = m_properties["name.last"];
	if ( m_properties["name.full"] != "" )
		m_properties["name.full"] += " ";
	m_properties["name.full"] += m_properties["name.first"];
	if ( m_properties["name.full"] == "" )
		m_properties["name.full"] = m_properties["id"];
	StringOps(m_properties["name.full"]).trim();

/*	m_properties["name.full"] = m_properties["name.first"];
	if ( m_properties["name.full"] != "" ) m_properties["name.full"] += " ";
	m_properties["name.full"] += m_properties["name.last"];
	if ( m_properties["name.full"] == "" ) m_properties["name.full"] = m_properties["id"];*/
}

/* set speaker gender from gender code */
void Speaker::setGender(Gender gender)
{
	m_gender = gender;
	switch (gender) {
	case MALE_GENDER :	m_properties["gender"] = "male";  break;
	case FEMALE_GENDER : m_properties["gender"] = "female";  break;
	default : m_properties["gender"] = "";  break;
	}
}


  /*
   * set speaker gender
   * @param gender string stating speaker gender
   */
void Speaker::setGender(const char* gender)
{
	m_gender = UNDEF_GENDER;
	if ( strcasecmp(gender, "1") == 0 ) m_gender = MALE_GENDER;
	else if ( strcasecmp(gender, "m") == 0 ) m_gender = MALE_GENDER;
	else if ( strcasecmp(gender, "male") == 0 ) m_gender = MALE_GENDER;
	else if ( strcasecmp(gender, "man") == 0 ) m_gender = MALE_GENDER;
	else if ( strcasecmp(gender, "homme") == 0 ) m_gender = MALE_GENDER;
	else if ( strcasecmp(gender, "2") == 0 ) m_gender = FEMALE_GENDER;
	else if ( strcasecmp(gender, "f") == 0 ) m_gender = FEMALE_GENDER;
	else if ( strcasecmp(gender, "female") == 0 ) m_gender = FEMALE_GENDER;
	else if ( strcasecmp(gender, "woman") == 0 ) m_gender = FEMALE_GENDER;
	else if ( strcasecmp(gender, "femme") == 0 ) m_gender = FEMALE_GENDER;
	setGender(m_gender);
}


  /*
   * get speaker gender
   * @return pointer on static string stating localized speaker gender
   */
const char* Speaker::getGenderLocalizedCStr() const
{
		switch ( m_gender ) {
			case UNDEF_GENDER : return "";
			case MALE_GENDER : return _("male");
			case FEMALE_GENDER : return _("female");
			case NO_SPEECH_GENDER: return "";
		}
		return "";
}


  /*
   * set speaker scope
   * @param scope speaker scope
   */
void Speaker::setScope(Scope scope)
{
	m_scope = scope;
	switch ( m_scope ) {
		case LOCAL_SCOPE : m_properties["scope"] = "local"; break;
		case GLOBAL_SCOPE : m_properties["scope"] = "global";break;
	}
}

/*
* set speaker scope
* @param scope string stating speaker scope
*/
void Speaker::setScope(const char* scope)
{
	m_scope = LOCAL_SCOPE;
	if ( strcasecmp(scope, "1") == 0 ) m_scope = GLOBAL_SCOPE;
	else if ( strcasecmp(scope, "local") == 0 ) m_scope = LOCAL_SCOPE;
	else if ( strcasecmp(scope, "global") == 0 ) m_scope = GLOBAL_SCOPE;
	else if ( *scope ) { m_scope = GLOBAL_ID; m_properties["scope"] = scope; }
	setScope(m_scope);
}


  /*
   * get speaker scope
   * @return pointer on static string stating localized speaker scope
   */
const char* Speaker::getScopeLocalizedCStr() const
{
		switch ( m_scope ) {
			case LOCAL_SCOPE : return _("local");
			case GLOBAL_SCOPE : return _("global");
			default:
			{
				map<string, string>::const_iterator it = m_properties.find("scope");
				if ( it != m_properties.end() ) return it->second.c_str();
			}
		}
		return "";
}

/**
* add language to list of spoken languages for speaker
* @param lang language definition
* @return true if language added or updated, else false
*
* @note:
*  check first if already exists for speaker (same lang code & same dialect)
*  if yes -> update language properties (accent, native, usual)
*  else add language to spoken languages list.
*/
bool Speaker::addLanguage(const Language& lang)
{
	vector<Language>::iterator it;
	if ( lang.getCode().empty() ) return false;
	for ( it=m_spokenLanguages.begin(); it != m_spokenLanguages.end(); ++it )
		if ( it->getCode() == lang.getCode() && it->getDialect() == lang.getDialect() ) {
			*it = lang;
			return true;
		}
	m_spokenLanguages.push_back(lang);
	return true;
}


/*
 * remove language from list of spoken languages (same lang code & same dialect)
*/
bool Speaker::removeLanguage(const std::string& iso639_2_code, const std::string& dialect)
{
	bool done = false;
	vector<Language>::iterator it;
	for ( it=m_spokenLanguages.begin(); it != m_spokenLanguages.end(); ++it )
		if ( it->getCode() == iso639_2_code && it->getDialect() == dialect ) {
			m_spokenLanguages.erase(it);
			done = true;
			break;
		}
	return done;
}

/*
* return language description
*/
const Speaker::Language& Speaker::getLanguage(const std::string& iso639_2_code, const std::string& dialect) const
{
	vector<Language>::const_iterator it;
	for ( it=m_spokenLanguages.begin(); it != m_spokenLanguages.end(); ++it )
			if ( it->getCode() == iso639_2_code && it->getDialect() == dialect )
				return *it;

	return NO_LANGUAGE;
}

/*
* return speaker usual language
*/
const Speaker::Language& Speaker::getUsualLanguage() const
{
	vector<Language>::const_iterator it;
	for ( it=m_spokenLanguages.begin(); it != m_spokenLanguages.end(); ++it )
			if ( it->isUsual() )  return *it;
	if ( m_spokenLanguages.size() > 0 ) return m_spokenLanguages.front();
	return NO_LANGUAGE;
}

  /**
   * print out speaker as xml-formatted string
   * @param out destination ostream
   * @param delim print-out delimiter (eg. "\n")
   * @return destination ostream
   */
std::ostream& Speaker::toXML(std::ostream& out, const char* delim) const
{
	const char* items[] =
		{ "gender", "name.first", "name.last", "scope", "desc", NULL };
	int i;

	if ( m_id.empty() ) {    // should never happen !!
		MSGOUT << "Warning : found empty speaker id ! " << std::endl;
		return out;
	}

	out << "<Speaker id=\"" << m_id << "\"";
	for ( i=0; items[i] != NULL ; ++i )
		out << " " << items[i] << "=\"" << getProperty(string(items[i])) << "\"";

	if ( m_spokenLanguages.size() == 0 ) out <<"/>";
	else  {
		out << ">" << delim;
		for ( unsigned int i=0; i < m_spokenLanguages.size(); ++i )
			if ( ! m_spokenLanguages[i].getCode().empty() ) // print only if language code set
				m_spokenLanguages[i].toXML(out, delim);

		out << "</Speaker>" ;

	}
	out << delim;
	return out;
}


  /*
   * print out spoken language definition as xml-formatted string
   * @param out destination ostream
   * @return destination ostream
   */
std::ostream& Speaker::Language::toXML(std::ostream& out, const char* delim) const
{
	out << "<SpokenLanguage code=\"" << m_code << "\" dialect=\"" << m_dialect
		<< "\" isnative=\"" << m_isNative << "\" isusual=\"" << m_isUsual
		<< "\" accent=\"" << m_accent
		<< "\" />" << delim;

	return out;
}

} // namespace tag
