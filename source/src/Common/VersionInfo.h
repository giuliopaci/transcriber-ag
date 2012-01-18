/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* @file VersionInfo.h
*
* @note : holds TranscriberAG current version info
*/

#include "Common/util/Utils.h"


#ifndef	 _HAVE_VERSION_INFO
#define	 _HAVE_VERSION_INFO 1

#define	TRANSAG_DISPLAY_NAME			"TranscriberAG"			/**<  APPLICATION version name */
#define	TRANSAG_VERSION_NO 				"2.0.0-b1" 				/**<  APPLICATION version */

#define APPLICATION_VERSION_DATE  		"4 Jul 2011"

string getVersionStamp() ;

#endif //	 _HAVE_VERSION_INFO
