/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class Version
 *  @brief version definition
 *
 */

#include <iostream>
#include <map>
#include <glib.h>
#include <string.h>

#include "Version.h"
#include "Common/util/Log.h"

using namespace std;
using namespace tag;

/*
*  constructors
*/
Version::Version()  {
}

Version::Version(string id, string date, string author, string wid, string tagversion, string comment)
{
	setId(id);
	setDate(date);
	setAuthor(author);
	setWid(wid);
	setComment(comment);
	setTagVersion(tagversion);
}

Version::Version(const Version& copy)
{
	setId(copy.getId());
	setDate(copy.getDate());
	setAuthor(copy.getAuthor());
	setWid(copy.getWid());
	setComment(copy.getComment());
	setTagVersion(copy.getTagVersion());
}

void Version::setId(const string& id)
{
	a_id = id;
	int nb = sscanf(id.c_str(), "%d.%d", &a_major, &a_minor);
	if ( nb == 0 ) a_major = 1;
	if ( nb < 2 ) {
		a_minor = 0;
		char buf[20];
		sprintf(buf, "%d.%d", a_major, a_minor);
		a_id = buf;
	}
}

void Version::setDate(const string& date)
{
	if ( date == "today" || date.empty() ) {
		// use today's date
		GDate  date;
		g_date_set_time (&date, time (NULL));
		char bufdate[20];
		sprintf(bufdate, "%04d/%02d/%02d", g_date_get_year(&date), g_date_get_month(&date), g_date_get_day(&date));
		a_date = bufdate;
	} else
		a_date = date;
}

  /*
   * print out version as xml-formatted string
   * @param out destination ostream
   * @return destination ostream
   */
std::ostream& Version::toXML(std::ostream& out, const char* delim) const
{
	if ( a_id.empty() ) {    // should never happen !!
		MSGOUT << "Warning : found empty version id ! " << std::endl;
		return out;
	}

	out << "<Version id=\"" << a_id << "\" date=\""<< a_date << "\" author=\""<< a_author << "\" wid=\""<< a_wid << "\" tag_version=\""<< a_tag_version;
	if ( a_comment.empty() )
		out << "\"/>" << delim;
	else out << "\">" << a_comment << "</Version>" << delim;
	return out;
}
