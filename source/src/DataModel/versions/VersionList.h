/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  @class VersionList
 *  Annotation file versions list
 */


#ifndef _HAVE_VERSION_LIST
#define _HAVE_VERSION_LIST

#include <vector>
#include <ostream>
#include <glibmm.h>
#include <glibmm/objectbase.h>

#include "DataModel/versions/Version.h"

using namespace std;

namespace tag {

/**
 *  @class VersionList
 *  @ingroup DataModel
 *  Annotation file versions list
 */

class VersionList {
	public:
	  /*! default constructor */
	VersionList() {};
	  /*! copy constructor */
	VersionList(const VersionList& copy)
		: a_versions(copy.getVersions()){}

  /* Define version list iterator */
  typedef std::vector<Version>::iterator iterator;
  typedef std::vector<Version>::const_iterator const_iterator;

  /*! affectation operator */
  VersionList& operator=(const VersionList& copy) {
		a_versions.clear();
		a_versions = copy.getVersions();
		return *this;
	}

  /**
   * get version information from list
   * @param id version id
   * @return version descriptor
   */
 	Version& getVersion(const string& id);

  /**
   * add new version to list
   * @param version version descriptor
   * @return true if version added / false if already exists
   */
  bool addVersion(const Version& version);


  /**
   * update version definition in version list
   * @param version version descriptor
   * @param auto_add if true and version not found, add version to version list (default=false)
   * @return true if version updated / false if not
   */
  bool updateVersion(const Version& version, bool auto_add=false);

  /**
   * delete version definition from version list
   * @param id version id
   * @return true if version deleted / false if not found
   */
  bool deleteVersion(string id);

  /**
  * print out versions version list as xml-formatted string
  * @param out ostream where to print
  * @param delim string delimiter between items
  * @return ostream passed as "out" parameter
  */
  std::ostream& toXML(std::ostream& out, const char* delim="\n") const;


	/**
   * read versions version list from xml-formatted string
   * @param in source string
   */
	void fromXML(const std::string& in) throw (const char*);


	/* version list size */
	int size() { return a_versions.size(); }	/**< nb of versions in list */
	bool empty() { return (a_versions.size() == 0); }  /**< true if no version */
	void clear() { a_versions.clear(); }	/**< reset version list */

  /*! browse through version list - begin iterator */
  const_iterator begin() const { return a_versions.begin(); }
  /*! browse through version list - non-const begin iterator */
  iterator begin() { return a_versions.begin(); }
  /*! browse through version list - end iterator */
  const_iterator end() const { return a_versions.end(); }
  /*! browse through version list - non-const end iterator */
  iterator end() { return a_versions.end(); }
  /*! last version */
   Version& back() { return a_versions.back(); }
   /*! first version */
   Version& front() { return a_versions.front(); }

   /**
    *  create new version id string from major and minor numbers
    * @param major major version number
    * @param minor minor version number
    * @return formatted version id
    */
   string newVersionId(int major=-1, int minor=-1);

   const std::vector<Version>& getVersions() const { return a_versions; }  /**< get versions list */
   std::vector<Version>& getVersions() { return a_versions; }  /**< get versions list - non const */


private:
	std::vector<Version> a_versions; /**< all versions */
	static Version novers;
};


} /* namespace tag */

#endif // _HAVE_VERSION_LIST
