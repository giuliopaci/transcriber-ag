/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#ifndef _HAVE_VERSION_HH
#define _HAVE_VERSION_HH

#include <string>
#include <string.h>

#include <vector>
#include <map>
#include "Common/globals.h"

using namespace std;

namespace tag {

/**
 *  @class Version
 *  @ingroup DataModel
 *  Annotation file version information
 */
class Version {

public:
  /*! default constructor */
  Version();

  /**
   * constructor
   * @param id file version id, formatted as [major].[minor] string
   * @param date version date (formatted as string)
   * @param author	version author
   * @param wid	version work effort indication (time spent for edition, measured in seconds, formatted as hexadecimal string)
   * @param tagversion	version of TranscriberAG annotation editor used to produce file version
   * @param comment version comment
   */
  Version(string id, string date, string author, string wid, string tagversion="", string comment="");

  /*! copy constructor */
  Version(const Version& copy) ;

  const std::string& getId()  const { return a_id; } /**< get version id */
  void setId(const std::string& id); /**< set version id, formatted as [major].[minor] string*/
  const std::string& getDate()  const { return a_date; } /**< get version date */
  void setDate(const std::string& date); /**< set version date */
  const std::string& getAuthor()  const { return a_author; }  /**< get version author */
  void setAuthor(const std::string& author) { a_author = author; }  /**< set version author */
  const std::string& getWid()  const { return a_wid; } /**< get version wid */
  void setWid(const std::string& wid) { a_wid = wid; } /**< set version wid */
  const std::string& getComment()  const { return a_comment; }  /**< get version comment */
  void setComment(const std::string& comment) { a_comment = comment; } /**< set version comment */
	int getMajor() { return a_major; }	/**< get version major number */
	int getMinor() { return a_minor; } 	/**< get version minor number */
	const std::string& getTagVersion() const { return a_tag_version; } /**< get TranscriberAG version  */
	void setTagVersion(const std::string& tid) { a_tag_version = tid; } /**< set TranscriberAG version  */

	/*! check for equality : true if same id */
  bool operator ==(const Version& cmp) const { return (strcmp(a_id.c_str(), cmp.getId().c_str()) == 0);}

  /**
   * print out version as xml-formatted string
   * @param out destination ostream
   * @param delim string delimiter between items
   * @return destination ostream
   */
	std::ostream& toXML(std::ostream& out, const char* delim="") const;


private:
	string a_id; 	/* version id */
	int 	a_major, a_minor; 	/* version major & minor numbers */
	string a_date;			/* version date */
	string a_author;		/* version author */
	string a_wid;			/* version elapsed time */
	string a_comment;		/* version comment */
	string a_tag_version;	/* transcriber AG version used for this version */

};

}
#endif  // _HAVE_VERSION_HH
