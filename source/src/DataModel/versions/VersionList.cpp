/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  @file VersionList.cpp
 *  @brief VersionList class implementation
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLString.hpp>

#include "Common/globals.h"
#include "Common/util/StringOps.h"
#include "VersionList.h"
#include "VersionList_XMLHandler.h"

using namespace std;

namespace tag {

Version VersionList::novers("","","","");

Version& VersionList::getVersion(const string & id) {
	iterator it;
	for ( it=a_versions.begin(); it != a_versions.end(); ++it )
		if ( it->getId() == id ) {
			return *it;
		}
	return novers;
}

/*
 * add new version to dictionary
 * return true if version added / false if already exists
 */
bool VersionList::addVersion(const Version & version) {
	a_versions.push_back(version);
	return true;
}

/*
 * update version definition in dictionary
 * return true if version updated / false if not
 */
bool VersionList::updateVersion(const Version & version, bool auto_add) {
	string id = version.getId() ;
	iterator it = a_versions.end();
	if ( id == "" ) {
		if ( ! auto_add ) return false;
		id = newVersionId();
	} else {
		for ( it=a_versions.begin(); it != a_versions.end(); ++it )
			if ( it->getId() == id ) {
				*it = version;
				break;
			}
	}

	if ( it == a_versions.end() )
		a_versions.push_back(version);
		a_versions.back().setId(id);
		return true;
	}


	std::string VersionList::newVersionId(int major, int minor)
	{
		if ( major == -1 ) {
			if ( a_versions.size() == 0 ) {
				major = 1; minor=0;
			} else {
				major = a_versions.back().getMajor();
				minor = a_versions.back().getMinor();
				minor++;
			}
		}
		char id[20];
		sprintf(id, "%d.%d", major, minor);
		return id;
	}

/*
 * print out Version list as XML-formatted string
*/
std::ostream & VersionList::toXML(std::ostream & out, const char *delim) const {
		const_iterator it;
		out << "<Versions>" << delim;
		for (it = a_versions.begin(); it != a_versions.end(); ++it)
			it->toXML(out, delim);
		out << "</Versions>" << delim;
		return out;
	}
/*
   * read version list from xml-formatted string
   */
void VersionList::fromXML(const std::string & in)
	 throw(const char *) {
		try
		{
			VersionList_XMLHandler handler(this);
			CommonXMLReader reader(&handler);

			unsigned int pos = in.find("<");
			if (in.compare(pos, 10, "<Versions>") != 0)
			{
				ostringstream os;
				os << "<Versions>\n" << in << "\n</Versions>\n";
				reader.parseBuffer(os.str());
			} else
				reader.parseBuffer(in);
		}
		catch(const char *msg) {
			throw msg;
		}
	}

}								/* namespace tag */
