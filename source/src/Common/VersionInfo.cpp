/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* FERRY Guillaume - LECUYER Paule - MONTILLA Jmarc							  	*/
/* 	         																	*/
/********************************************************************************/

#include "VersionInfo.h"
#include <glibmm.h>

string getVersionStamp()
{
	try
	{
		return APPLICATION_VERSION_DATE ;
	}
	catch ( ... )
	{
		return TRANSAG_VERSION_NO ;
	}
}

