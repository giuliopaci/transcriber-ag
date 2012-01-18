/*
 * AnnotationIndex.cpp
 *
 *  Created on: 6 mai 2010
 *      Author: plecuyer
 */
#include <iostream>
#include "AnnotationIndex.h"

#include "Common/util/Utils.h"

namespace tag {

string AnnotationIndex::_IDNotFound("");
#define MKLONG(a) (long)(((a)+0.00001)*1000.0)

AnnotationIndex::AnnotationIndex(float duration)
{
	m_grain = 10;
}

AnnotationIndex::~AnnotationIndex() {}

void AnnotationIndex::clear()
{
	time_iterator itt;
	for (itt = m_index.begin(); itt != m_index.end(); ++itt)
		itt->second.clear();
	m_ids.clear();
	m_index.clear();
}


void AnnotationIndex::add(const string& id, float startTime, int order)
{
	if ( order == 0 )
	{
		int key=(((int)startTime)/m_grain)*m_grain;
		long t = MKLONG(startTime);
		m_index[key][t] = id;
	}
	m_ids[id] = ItemData(startTime, order);
}

void AnnotationIndex::remove(const string& id)
{
	id_iterator it = m_ids.find(id);
	if ( it != m_ids.end() ) {
		float startTime = it->second.startTime;
		m_ids.erase(it);
		int key=(((int)startTime)/m_grain)*m_grain;
		time_iterator itt = m_index.find(key);
		if ( itt != m_index.end() ) {
			std::map<long, std::string>& the_map = itt->second;
			std::map<long, std::string>::iterator it2;
			for (it2 = the_map.begin(); it2 != the_map.end(); ++it2 ) {
				if ( it2->second == id ) {
					the_map.erase(it2);
					break;
				}
			}
			if ( the_map.size() == 0 )
				m_index.erase(itt);
		} else
			Log::out() << "AnnotationIndex::remove : key not found id=" << id << " key=" << key << endl;
	}
}

void AnnotationIndex::update(const string& id, float startTime)
{
	int order=0;
	id_iterator it = m_ids.find(id);
	if ( it != m_ids.end() ) {
		order = it->second.order;
		remove(id);
	}
	add(id, startTime, order);
}


bool AnnotationIndex::contains(const string& id) {
	id_iterator it = m_ids.find(id);
	return ( it != m_ids.end() );
}

float AnnotationIndex::getStartTime(const string& id) {
	id_iterator it = m_ids.find(id);
	if ( it != m_ids.end() ) {
		return it->second.startTime;
	}
}

const string& AnnotationIndex::getIdAtTime(float f)
{
	if ( m_index.size() == 0 )
		return _IDNotFound;

	long comp = MKLONG(f);

	//> -- Compute key
	int key=(((int)f)/m_grain)*m_grain;

	//> -- Search for corresponding index
	time_iterator itt=m_index.begin();
	while ( itt!=m_index.end() && (itt->first < key) )
			itt++ ;

	//> -- Check wheter we are placed 1 index after matching one, and go backward if needed
	if ( itt == m_index.end() || itt->first > key )
		if ( itt != m_index.begin() )
			itt--;

	//> -- Empty index ? exit
	std::map<long, std::string>& the_map = itt->second;
	std::map<long, std::string>::iterator it2 = the_map.begin();
	if ( the_map.size() == 0 )
		return _IDNotFound ;

	//> -- First indexed time is after the time we search : we are 1 index too much ahead
	if ( it2->first > comp )
	{
		// -- Go backward
		if ( itt != m_index.begin() )
			itt -- ;

		// -- If no index, exit
		if ( itt->second.size()==0 )
			return _IDNotFound ;

		// -- only 1 solution remainig, it's the last.
		//    otherwise, exit
		it2 = itt->second.end();
		it2-- ;
		if ( it2->first <= comp )
			return it2->second;
		else
			return _IDNotFound;
	}
	//> -- First indexed time is before the time we search : we're in good index, keep on searching :)
	else
	{
		//> -- Go ahead until we find the indexed time superior to the time candidate (the limit)
		for (it2=the_map.begin(); it2 != the_map.end() && it2->first < comp; ++it2 );

		//> -- Find it ? The previous is the good one :)
		if ( it2 == the_map.end() || it2->first > comp)
			if ( it2 != the_map.begin() )
				it2--;
		return it2->second;
	}
}

//-------------------------------------------------------------------------------------
//										MONITORING
//-------------------------------------------------------------------------------------

void AnnotationIndex::printIndexes()
{
	time_iterator it ;
	for (it=m_index.begin(); it!=m_index.end(); it++)
	{
		Log::out() << "\tKEY=" << number_to_string(it->first) << std::endl ;
		printIndex(it->first) ;
	}
}

void AnnotationIndex::printIndex(int key)
{
	std::map< long, std::string> index = m_index[key] ;
	std::map< long, std::string>::iterator it ;
	for (it=index.begin(); it!=index.end(); it++)
		Log::out() << "\t\t time=" << number_to_string(it->first) << " - id=" << it->second << std::endl ;

	std::map< int, std::map< long, std::string > > m_index;
}

} //namespace
