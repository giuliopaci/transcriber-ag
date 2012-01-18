/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */


#include <set>
#include <list>

#include <ag/AGException.h>
#include <ag/RE.h>
#include <ag/AGAPI.h>

#include "TransAG_StringParser.h"
#include "SAX_TransAGHandler.h"
#include "agfXercesUtils.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
using namespace std;


/**
* parse UTF-8-encoded XML Buffer
*/
void
TransAG_StringParser::parseBuffer(const string& buffer)
throw (agfio::LoadError)
{
	try {
		xercesc_open();
	}
	catch (const agfioError& e) {
		throw agfio::LoadError(string("TransAG:") + e.what());
	}

	SAX_TransAGHandler handler("UTF-8");
	try {
		tagSAXParseBuffer(&handler, buffer, false, "UTF-8");
	}
	catch (const XMLException& e) {
		throw agfio::LoadError(string("TransAG:loading failed due to the following error\n") + trans(e.getMessage()));
	}
	catch (AGException& e) {
		throw agfio::LoadError("TransAG:loading failed due to the following error\n" +
				e.error());
	}

	list<AGId> result = handler.get_agids();

	xercesc_close();

	if ( result.size() > 0 )  m_agId = result.front();
}

