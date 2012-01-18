/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/


#ifndef _HAVE_SPEAKER_DICT
#define _HAVE_SPEAKER_DICT

#include <map>
#include <ostream>
#include <glibmm.h>
#include <glibmm/objectbase.h>

#include "DataModel/speakers/Speaker.h"

using namespace std;

namespace tag {

/**
 *  @class 		SpeakerDictionary
 *  @ingroup	DataModel
 *  Speakers dictionary
 */
class SpeakerDictionary : public Glib::ObjectBase {
	public:
		/**
		 *  @class NotFoundException
		 *  Speakers not found exception
		 */

		class NotFoundException {
			private :
					std::string _msg;
			public:
				/**
				 * constructor
				 * @param key speaker search key item
				 * @param val speaker search key value
				 */
				NotFoundException(std::string key, std::string val) {
					_msg = std::string("No speaker with ") + key + std::string(" ") + val;
				}
				/*! exception error message */
				const char* what() const throw() { return _msg.c_str(); }
		};
public:
	  /*! default constructor */
	SpeakerDictionary() {};
	  /*! copy constructor */
	SpeakerDictionary(const SpeakerDictionary& copy)
		: m_speakers(copy.getSpeakers()), m_desc(copy.getDescription()){}


  typedef map<string, Speaker>::iterator iterator;
  typedef map<string, Speaker>::const_iterator const_iterator;

  /**
   * copy operator
   * @param copy copied dictionary
   * @return reference on current speaker dictionary
   */
  SpeakerDictionary& operator =(const SpeakerDictionary& copy) {
	  m_speakers = copy.getSpeakers();
	  m_desc = copy.getDescription();
	  return (*this);
  }

  /**
   * load dictionary from given url
   * @param url string holding url to load from
   * @note  for the time beeing, only file protocol supported
   */
  void loadDictionary(string url) throw (const char*);

  /**
   * save dictionary to given url
   * @param url string holding url to save to
   * @return boolean value stating if saving was successful or not
   * @note  for the time beeing, only file protocol supported
   */
  bool saveDictionary(string url);

  /**
   *  get speaker with given id
   * @param id string holding searched speaker id
   * @return pointer on found speaker / NULL if no speaker with that id exists
   */
  Speaker& getSpeaker(const string& id) throw (NotFoundException);

  /**
   *  check that speaker with given id exists
   * @param id string holding searched speaker id
   * @return true if exists, else false
   */
  bool existsSpeaker(const string& id) ;

  /**
   *  get speaker with given name
   * @param lastname string holding searched speaker last name
   * @return pointer on found speaker / NULL if no speaker with that id exists
   */
   Speaker& getSpeakerByName(const string& lastname) throw (NotFoundException);

  /**
   * add new speaker to dictionary
   * @param speaker speaker definition
   * @return true if speaker added / false if already exists
   */
  bool addSpeaker(const Speaker& speaker);


  /**
   * update speaker definition in dictionary
   * @param speaker speaker definition
   * @param auto_add if true and speaker not found, add speaker to dictionary (default=false)
   * @return true if speaker updated / false if not
   */
  bool updateSpeaker(const Speaker& speaker, bool auto_add=false);

  /**
   * delete speaker definition from dictionary
   * @param id speaker id
   * @return true if speaker deleted / false if not found
   */
  bool deleteSpeaker(const string& id);

  /**
   * get speaker definition with default name and gender
   * @param lang speaker default language
   * @return defaut speaker
   */
  const Speaker& defaultSpeaker(string lang="");

  /**
  * print out speakers dictionary as XML-formatted string
  * @param out ostream where to print
  * @param delim string to print between 2 dictionary entries
  * @return ostream passed as "out" parameter
  */
  std::ostream& toXML(std::ostream& out, const char* delim="\n") const;

  /**
   * Load a speaker dictionary from an XML stream
   * @param in	  Stream to be read
   * @param dtd	  Dtd to apply
   */
	void fromXML(const std::string& in, const std::string& dtd="") throw (const char*);

	/* dictionary size */
	int size() { return m_speakers.size(); } /**< nb of slangpeakers in dictionary */
	bool empty() { return (m_speakers.size() == 0); }  /**< true if no entry in dictionary */

	/**
	 * Clear speaker dictionary
	 */
	void clear() { m_speakers.clear(); }

  /*! browse through dictionary - begin iterator */
  const_iterator begin() const { return m_speakers.begin(); }
  /*! browse through dictionary - non-const begin iterator */
  iterator begin() { return m_speakers.begin(); }
  /*! browse through dictionary - end iterator */
  const_iterator end() const { return m_speakers.end(); }
  /*! browse through dictionary - non-const end iterator */
  iterator end() { return m_speakers.end(); }
  /**
   * find speaker identified by id in dictionary
   * @param id target speaker id
   * @return matching iterator / end() if not found
   */
  const_iterator find(const std::string& id) const { return m_speakers.find(id); }


  /*! return indicative speaker rank in dictionary */
  int getSpeakerRank(const string& id);

  /*! compute and return unique speaker id  */
  string getUniqueId(char* name=NULL);

  /*! compute and return default name for given speaker id  */
  string getDefaultName(const string& id);

  /*! return dictionary speakers map */
   const std::map<std::string, Speaker>& getSpeakers() const { return m_speakers; }

   // configuration:
   /*! set format for default speaker id generation */
   static void setDefaultSpeakerFormat(string format) { defaultFormat = format; }

   /**
    * speaker updated signal. Emitted upon speaker modification
    * <b>string parameter:</b>	updated speaker id\n
    */
   sigc::signal<void, std::string> signalSpeakerUpdated() { return m_signalSpeakerUpdated ; }
   /**
    * speaker deleted signal. Emitted upon speaker deletion
    * <b>string parameter:</b>	deleted speaker id\n
    */
   sigc::signal<void, std::string> signalSpeakerDeleted() { return m_signalSpeakerDeleted ; }

   /*! get dictionary description */
    const std::string& getDescription() const { return m_desc; }

private:
	map<string, Speaker> m_speakers; /**< all speakers */
	string m_desc;         /**< brief description */
	Speaker m_defaultSpeaker;  /**< default speaker definition */

public:
	static string defaultFormat; /**< default speaker name format */
private:
	static string idFormat; /**< speaker id format */

   sigc::signal<void, std::string> m_signalSpeakerUpdated ;
   sigc::signal<void, std::string> m_signalSpeakerDeleted ;
};


} /* namespace tag */

#endif // _HAVE_SPEAKER_DICT
