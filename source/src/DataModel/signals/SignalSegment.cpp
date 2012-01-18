/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* @file SignalSegment.cpp
* @brief  signal segments classes
*/

#include "SignalSegment.h"

using namespace std;

namespace tag {


bool SignalSegment::operator< (const SignalSegment& s) const
{
	if (getStartOffset() < s.getStartOffset()) return true;
	if (getStartOffset() == s.getStartOffset()) {
		if ( getOrder() < s.getOrder() ) return true;
		if ( getOrder() == s.getOrder() ) {
			if ( getTrack() < s.getTrack() ) return true;
			if ( getTrack() == s.getTrack() )
				return (getEndOffset() < s.getEndOffset());
		}
	}
	return false;
}


bool SignalSegment::hasProperty(const string& name)  const throw (const char*)
{
#ifdef CHECK_NAMES
	if ( ! validPropertyName(name) ) {
		ostringstream msg;
		msg << "Invalid property name for class " << getSegmentClass() << " : " << name ;
		throw msg.str().c_str();
	}
#endif
	std::map<std::string, std::string>::const_iterator it = m_properties.find(name);
	return (it != m_properties.end());
}

string SignalSegment::getProperty(const string& name)  const throw (const char*)
{
#ifdef CHECK_NAMES
	if ( ! validPropertyName(name) ) {
		ostringstream msg;
		msg << "Invalid property name for class " << getSegmentClass() << " : " << name ;
		throw msg.str().c_str();
	}
#endif
	std::map<std::string, std::string>::const_iterator it = m_properties.find(name);
	return ( it == m_properties.end() ? "" : it->second);
}


void SignalSegment::setProperty(const std::string& name, const std::string& value) throw (const char*)
{
#ifdef CHECK_NAMES
	if ( ! validPropertyName(name) ) {
		ostringstream msg;
		msg << "Invalid property name for class " << getSegmentClass() << " : " << name ;
			throw msg.str().c_str();
	}
#endif
	m_properties[name] = value;
}



}
