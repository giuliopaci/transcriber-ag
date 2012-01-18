/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @file globals.h
 * @brief global defines for TranscriberAG application
 */

#ifndef _HAVE_COMMON_GLOBDEF_H
#define _HAVE_COMMON_GLOBDEF_H 1

// -- Old automake code --
/*
#ifndef PACKAGE
#include "config.h"
#endif
*/

#ifdef FUTUR
#if defined WITH_FTP && WITH_FTP == 0
#undef WITH_FTP
#endif

#if defined WITH_DES && WITH_DES == 0
#undef WITH_DES 
#endif

#define WITH_FTP 
#define WITH_DES 
#endif

#endif /* _HAVE_COMMON_GLOBDEF_H */
