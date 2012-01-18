/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */


#ifndef _HAVE_SPEAKER_HH
#define _HAVE_SPEAKER_HH

#include <string>
#include <vector>
#include <map>
#include "Common/globals.h"

using namespace std;

namespace tag {

/**
*  @class Speaker
*  @ingroup DataModel
*  Speaker properties.
*/

class Speaker {

public:
	static const string NO_SPEECH;  /**< no speaker tag */

	/*! speaker gender  */
	typedef enum { UNDEF_GENDER, MALE_GENDER, FEMALE_GENDER, NO_SPEECH_GENDER } Gender;

	/*! speaker scope : local / global / global id */
	typedef enum { LOCAL_SCOPE, GLOBAL_SCOPE, GLOBAL_ID } Scope;

	/**
	*	@class Language
	*	Speaker spoken language
	*   A speaker may use several languages, and several dialects.
	*   For each spoken language, his accent, whether it's his native and his usual language must be stated.
	*	<br> a speaker may be bilingual, and so have many native & usual languages
	*/
	class Language {
	public:
		/*! default constructor */
		Language()
			: m_code(""), m_dialect(""), m_accent(""), m_isNative(false), m_isUsual(false) {}

		/**
		 *  alternate constructor
		 * @param lang language iso-639-2 3-letter code
		 * @param isNative true if language is speaker native language
		 * @param isUsual true if language is speaker usual language (which may differ from its native language)
		 * @param accent accent specification
		 * @param dialect dialect specification
		 */
		Language(const std::string& lang, bool isNative, bool isUsual, const std::string& accent="", const std::string& dialect="")
			: m_code(lang), m_dialect(dialect), m_accent(accent),
				m_isNative(isNative), m_isUsual(isUsual) {}
		/**
		 * copy constructor
		 * @param c copied language
		 */
		Language(const Language& c)
			: m_code(c.getCode()), m_dialect(c.getDialect()), m_accent(c.getAccent()),
				m_isNative(c.isNative()), m_isUsual(c.isUsual()) {}


		const string& getCode() const { return m_code; } /**< get language iso-639-2 3-letter code */
		void setCode(const string& iso639_2_code) { m_code = iso639_2_code; } /**< set language iso-639-2 3-letter code */
		const string& getDialect() const { return m_dialect; } /**< get dialect for current language */
		void setDialect(const string& dialect) { m_dialect = dialect; } /**< set dialect for current language */
		const string& getAccent() const { return m_accent; } /**< get accent for current language */
		void setAccent(const string& accent) { m_accent = accent; } /**< set accent for current language */
		bool isNative() const { return m_isNative; } /**< true if is speaker native language */
		void setNative(bool isNative) { m_isNative = isNative; }  /**< set as speaker native language */
		bool isUsual() const { return m_isUsual; }  /**< true if is speaker usual language */
		void setUsual(bool isUsual) { m_isUsual = isUsual; }  /**< set as speaker usual language */

		  /**
		   * print out language definition as xml-formatted string
		   * @param out destination ostream
		   * @param delim string delimiter between items
		   * @return destination ostream
		   */
		std::ostream& toXML(std::ostream& out, const char* delim="") const;

	private:
		string m_code;		/**< iso-639-2 language code */
		string m_dialect;	/**< specific dialect */
		string m_accent;	/**< accent in spoken language */
		bool m_isNative;	/**< is speaker native language or not */
		bool m_isUsual;		/**< is speaker usual language or not */
	} ;


	static const Language NO_LANGUAGE; // no language

public:
  /*! default constructor */
  Speaker();

  /**
   * constructor
   * @param id speaker id
   * @param lastname speaker lastname
   * @param firstname speaker firstname
   * @param gender speaker gender
   * @param scope speaker scope
   */
  Speaker(string id, string lastname="", string firstname = "", Gender gender = UNDEF_GENDER, Scope scope=LOCAL_SCOPE) ;


  /*! copy constructor */
  Speaker(const Speaker& copy) ;

	/**
	 * generic speaker property accessor
	 * @param name property name
	 * @return property value
	 */
 	const std::string& getProperty(const std::string& name) const;

 	/**
 	 * generic speaker properties accessor
 	 * @return speaker properties map
 	 */
	const std::map<std::string, std::string>& getProperties() const { return m_properties; }

	/**
	 * generic speaker property setter
	 * @param name property name
	 * @param value property value
	 */
	void setProperty(const std::string& name, const std::string& value);

  /*! set speaker id */
  void setId(const std::string& id) { m_id = id; }
  /*! get speaker id */
  const std::string& getId()  const { return m_id; }

  /*! set speaker first name */
  void setFirstName(const std::string name) { m_properties["name.first"]=name; mkFullName(); }
  /*! get speaker first name */
  const std::string& getFirstName()  const { return getProperty("name.first"); }

  /*! set speaker last name */
  void setLastName(const std::string name) { m_properties["name.last"]=name; mkFullName(); }
  /*! get speaker last name */
  const std::string& getLastName()  const { return getProperty("name.last"); }

   /*! get speaker full name (first + last) */
  const std::string& getFullName() const {	 return getProperty("name.full"); }

   /*! set speaker gender */
  void setGender(Gender gender);
  /*! set speaker gender from gender label */
  void setGender(const char* gender) ;
  /*! set speaker gender from gender label */
  void setGender(const std::string& gender) { setGender(gender.c_str()); }
  /*! get speaker gender */
  Gender getGender() const {return m_gender; }
  /*! get speaker gender label */
  const std::string& getGenderStr() const { return getProperty("gender"); }
  /*! get speaker gender localized label */
  const char* getGenderLocalizedCStr() const ;

  /*! set speaker scope */
  void setScope(Scope scope) ;
  /*! set speaker scope from scope label */
  void setScope(const char* scope) ;
  /*! set speaker scope from scope label */
  void setScope(const std::string& scope) { setScope(scope.c_str()); }
  /*! get speaker scope */
  Scope getScope() const { return m_scope; }
  /*! get speaker scope label */
  const std::string& getScopeStr() const { return getProperty("scope"); }
  /*! get speaker scope label */
  const char* getScopeLocalizedCStr() const ;

  /**
  * add language for speaker; if lang/dialect already exists, modify it
  * @param lang language iso-639-2 3-letter code
  * @param isNative true if language is speaker native language
  * @param isUsual true if language is speaker usual language (which may differ from its native language)
  * @param accent accent specification
  * @param dialect dialect specification
  * @return true if language added, else false
  */
  bool addLanguage(const std::string& lang, bool isNative=true, bool isUsual=true, const std::string& accent="", const std::string& dialect="")
	  { return addLanguage(Language(lang, isNative, isUsual, accent, dialect)); }
  /**
   * add language from language definition
   * @param lang language definition
   * @return true if language added, else false
   */
  bool addLanguage(const Language& lang);

  /**
   * get spoken language properties
   * @param lang  target language iso-639-2 3-letter code
   * @param dialect target dialect
   * @return Language properties
   */
  const Language& getLanguage(const std::string& lang, const std::string& dialect="") const;
  /**
   * get all spoken languages properties
   * @return vector holding all speaker spoken Languages properties
   */
  const vector<Language>& getLanguages() const { return m_spokenLanguages; }
  /*! same as above, non-const */
  vector<Language> getLanguages() {return m_spokenLanguages; }
  /*! set languages list for current speaker */
  void setLanguages(vector<Language> v) {m_spokenLanguages.clear(); m_spokenLanguages = std::vector<Language>(v);}

  /**
    * get speaker usual language
	*   @note a speaker may be bilingual, and so have many native & usual languages;
	*   the first one in list is returned;
   */
  const Language& getUsualLanguage() const;

  /*! remove language for speaker */
  bool removeLanguage(const std::string& iso639_2_code, const std::string& dialect="");

  /*! set speaker description */
  void setDescription(const std::string desc) { m_properties["desc"]=desc; }
  /*! get speaker description */
  const std::string& getDescription() const { return getProperty("desc"); }

	/*! check for equality : true if same id */
  bool operator ==(const Speaker& cmp) const { return (m_id == cmp.getId()) ;}

  /**
   * print out speaker as xml-formatted string
   * @param out destination ostream
   * @param delim delimiter string between speaker entries
   * @return destination ostream
   */
	std::ostream& toXML(std::ostream& out, const char* delim="") const;


  /**
  *  no speaker entry
  */
  static Speaker& noSpeaker() { return  * ( new Speaker(Speaker::NO_SPEECH, _("(No speaker)"), "", NO_SPEECH_GENDER )) ; }


private:
	void mkFullName();


private:
	std::string m_id;
	std::map<std::string, std::string> m_properties;
	Speaker::Gender m_gender;
	Speaker::Scope m_scope;
	vector <Language> m_spokenLanguages;

	static const std::string noPropertyValue;

};

}
#endif  // _HAVE_SPEAKER_HH
