/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
*   @file portabilite.h
*  defines for UNIX/WINDOWS portability
*/

#ifndef _HAVE_PORTABILITE_H
#define _HAVE_PORTABILITE_H

#if defined(WIN32) && !defined(__MINGW32__)
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#define USLEEP(a) SleepEx((a <= 1000 ? 1 : a/1000), false)
#define strcasecmp stricmp
#else
#include <unistd.h>
#define USLEEP(a) usleep(a)
#endif

#endif /* _HAVE_PORTABILITE_H */
