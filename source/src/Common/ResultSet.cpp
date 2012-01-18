/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file	ResultSet.h
 */
#include "ResultSet.h"
#include "util/Utils.h"

namespace tag {

//******************************************************************************
// 									Constructor
//******************************************************************************

ResultSet::ResultSet()
{
	loaded = false ;
}

ResultSet::~ResultSet()
{
}


//******************************************************************************
// 									LOADING
//******************************************************************************

bool ResultSet::loadResultSetFile(Glib::ustring path)
{
	if ( !Glib::file_test(path, Glib::FILE_TEST_EXISTS) )
		loaded = false ;
	else
	{
		filepath = path ;
		loaded = parseResultSetFile() ;
	}
	return loaded ;
}

bool ResultSet::loadResultSetString(Glib::ustring resultset)
{
	resultset_string = base64decode(resultset) ;
	Log::out() << "Received resultset string: " << resultset_string << std::endl ;
	loaded = parseResultSetString();
	return loaded ;
}


//******************************************************************************
// 									PARSING
//******************************************************************************

bool ResultSet::parseResultSetFile()
{
	//TODO hehe
	return true ;
}

bool ResultSet::parseResultSetString()
{
	if (resultset_string.empty())
		return false ;

	int nb = mini_parser(';', resultset_string, &resultset_vector) ;
	return true ;
}


} // namespace
