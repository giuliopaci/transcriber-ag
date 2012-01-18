/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* @file AnchorLinks.cpp
* Link between graphs anchors
*/

#include "AnchorLinks.h"
#include "AnchorLinks_XMLHandler.h"

#include <ag/AGAPI.h>

#include "Common/util/Log.h"
#include "Common/util/Utils.h"
#include "DataModel/DataModel.h"

namespace tag {

//------------------------------------------------------------------------------
//									CONSTRUCTORS
//------------------------------------------------------------------------------

AnchorLinks::AnchorLinks()
{
	dataModel = NULL ;
}

AnchorLinks::~AnchorLinks()
{
}

void AnchorLinks::setModel(DataModel* model)
{
	dataModel = model ;
}

//------------------------------------------------------------------------------
//									  BUSINESS
//------------------------------------------------------------------------------

int AnchorLinks::link(const string& anchorId1, const string& anchorId2, bool check)
{
	if ( check && !canBeLinked(anchorId1, anchorId2) )
		return -1 ;

	if (existLink(anchorId1, anchorId2))
		return 0 ;

	dataModel->tagInsertIntoAnchorLinks(anchorId1, anchorId2) ;
	dataModel->tagInsertIntoAnchorLinks(anchorId2, anchorId1) ;

	return 1 ;
}

void AnchorLinks::unlink(const string& anchorId1, const string& anchorId2)
{
	if (!existLink(anchorId1, anchorId2))
		return ;

	dataModel->tagRemoveFromAnchorLinks(anchorId1, anchorId2) ;
	dataModel->tagRemoveFromAnchorLinks(anchorId2, anchorId1) ;
}

void AnchorLinks::unlink(const string& anchorId)
{
	if (!hasLinks(anchorId))
		return ;

	// -- Remove anchorId everywhere it's referenced
	AnchorLinksSet::iterator it ;
	for (it=linksMap[anchorId].begin(); it!=linksMap[anchorId].end() ; it++ )
		dataModel->tagRemoveFromAnchorLinks(*it, anchorId) ;

	// -- Remove all referenced objects from List
	AnchorLinksSet copyset = linksMap[anchorId] ;
	AnchorLinksSet::iterator itc ;
	for (itc=copyset.begin(); itc!=copyset.end(); itc++)
		dataModel->tagRemoveFromAnchorLinks(anchorId, *itc) ;

//
//	// -- Remove anchorId entry
//	linksMap.erase(anchorId) ;
}

void AnchorLinks::setLinksOffset(const std::string& anchorId, float offset)
{
	if (!dataModel)
		return ;

	AnchorLinksSet links = getLinks(anchorId) ;
	AnchorLinksSet::const_iterator it ;
	for (it=links.begin(); it!=links.end(); it++)
		dataModel->setAnchorOffset(*it, offset) ;
}



//------------------------------------------------------------------------------
//									  ACCESS
//------------------------------------------------------------------------------


bool AnchorLinks::existLink(const string& anchorId1, const string& anchorId2)
{
	if (!hasLinks(anchorId1))
		return false ;

	AnchorLinksSet::iterator itset = linksMap[anchorId1].find(anchorId2) ;
	if (itset==linksMap[anchorId1].end())
		return false ;
	else
		return true ;
}

bool AnchorLinks::hasLinks(const string& anchordId)
{
	AnchorLinksMap::iterator it1 = linksMap.find(anchordId) ;
	if (it1==linksMap.end())
		return false ;
	else
		return true ;
}

std::set<std::string> AnchorLinks::getLinks(const string& anchorId)
{
	std::set<std::string> set ;
	return linksMap[anchorId] ;
}

//------------------------------------------------------------------------------
//								  	VALIDATION
//------------------------------------------------------------------------------

bool AnchorLinks::canBeLinked(const string& anchorId1, const string& anchorId2)
{
	if (!dataModel)
		return false ;

	//> -- Existence
	if ( !ExistsAnchor(anchorId1) || !ExistsAnchor(anchorId2))
		return false ;

	//> -- Time anchored
	if ( !GetAnchored(anchorId1) || !GetAnchored(anchorId2))
		return false ;

	//> -- From different graph
	const string& graphtype1 = dataModel->getGraphType(anchorId1) ;
	const string& graphtype2 = dataModel->getGraphType(anchorId2) ;
	if ( graphtype1.compare(graphtype2)==0 || graphtype1.empty() || graphtype2.empty() )
	{
		Log::out() << "~) anchors cannot be linked: " << anchorId1 << " & " << anchorId2 << std::endl ;
		return false ;
	}

	return true ;
}

//------------------------------------------------------------------------------
//									  INTERNAL
//------------------------------------------------------------------------------

void AnchorLinks::insertIntoLinks(const string& anchorId, const string& toBeInserted)
{
	if ( hasLinks(anchorId) )
		linksMap[anchorId].insert(toBeInserted) ;
	else
	{
		AnchorLinksSet set ;
		set.insert(toBeInserted) ;
		linksMap[anchorId] = set ;
	}
}

void AnchorLinks::removeFromLinks(const string& anchorId, const string& toBeRemoved)
{
	linksMap[anchorId].erase(toBeRemoved) ;
	if (linksMap[anchorId].empty())
		linksMap.erase(anchorId) ;
}

//------------------------------------------------------------------------------
//									 	IN / OUT
//------------------------------------------------------------------------------

void AnchorLinks::loadLinks(const string& anchorId, const string& links)
{
	std::set<std::string> linksSet ;
	string2set(links, &linksSet) ;
	linksMap[anchorId] = linksSet ;
}

std::ostream& AnchorLinks::toXML(std::ostream& out, const char* delim) const
{
	AnchorLinksMap::const_iterator it ;
	string id = "" ;
	string linksString = "" ;
	out << "<AnchorLinks>" << delim;

	for ( it=linksMap.begin() ; it!=linksMap.end(); it++ )
	{
		id = it->first ;
		std::set<std::string> set = it->second ;
		linksString = set2string(set) ;
		out << "<AnchorLink id=\"" << id << "\" links=\"" << linksString << "\"/>" << delim ;
	}
	out << "</AnchorLinks>" << delim;
	return out ;
}

/*
   * read speaker def from xml-formatted string
   */
void AnchorLinks::fromXML(const std::string & in, const std::string& dtd) throw (const char *)
{
	try
	{
		AnchorLinks_XMLHandler handler(this);
		CommonXMLReader reader(&handler);

		if ( !dtd.empty() )
		{
			std::string head = "<?xml version=\"1.0\" encoding=\"utf-8\"?>" ;
			head.append("\n") ;
			head.append("<!DOCTYPE AnchorLinks SYSTEM ") ;
			head.append("\"") ;
			head.append(dtd) ;
			head.append("\">") ;

			ostringstream os;
			os << head << endl ;
			unsigned int pos = in.find("<");
			if (in.compare(pos, 10, "<AnchorLinks>") != 0)
			{
				os << "<AnchorLinks>" << in << "</AnchorLinks>" << endl;;
			} else
				os << in << endl;
			reader.parseBuffer(os.str());
		}
		else
		{
			unsigned int pos = in.find("<");
			if (in.compare(pos, 13, "<AnchorLinks>") != 0)
			{
				ostringstream os;
				os << "<AnchorLinks>" << in << "</AnchorLinks>";
				reader.parseBuffer(os.str());
			}
			else
				reader.parseBuffer(in);
		}
	}
	catch(const char *msg)
	{
		Log::err() << "Anchor links loading: failed. Aborted." << std::endl ;
		throw msg;
	}
}



//------------------------------------------------------------------------------
//								  	TOOL
//------------------------------------------------------------------------------

bool AnchorLinks::tmpFlagAnchor(const string& anchorId)
{
	if ( tmpIsFlaggedAnchor(anchorId) )
		return false;

	tmpCheck.insert(anchorId) ;
	return true ;
}

bool AnchorLinks::tmpIsFlaggedAnchor(const string& anchorId)
{
	std::set<std::string>::iterator it = tmpCheck.find(anchorId) ;
	return ( it!=tmpCheck.end() ) ;
}

void AnchorLinks::tmpClearBuffer()
{
	tmpCheck.clear() ;
}

//------------------------------------------------------------------------------
//										DEBUG
//------------------------------------------------------------------------------

std::string AnchorLinks::toString()
{
	std::string res = "\n----------------------------------- ANCHOR LINKS" ;

	AnchorLinksMap::const_iterator it ;
	string id = "" ;
	string linksString = "" ;
	for ( it=linksMap.begin() ; it!=linksMap.end(); it++ )
	{
		id = it->first ;
		res.append(string("\nid=") + id + " linked to :\n") ;
		AnchorLinksSet myset = it->second ;
		AnchorLinksSet::const_iterator itset ;
		for (itset=myset.begin(); itset!=myset.end(); itset++)
			res.append(string("\t") + *itset + "\n") ;
		linksString = set2string(myset) ;
		res.append(string("\tsumup: id=") + id + " linked to" + linksString) ;
	}
	res.append("\n---------------------------------------------------\n") ;
	return res ;
}

} // namespace
