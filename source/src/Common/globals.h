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
#ifndef _HAVE_COMMON_GLOBALS_H
#define _HAVE_COMMON_GLOBALS_H 1

#include <libintl.h>
#include <iostream>
#include <fstream>

#include "Common/util/FormatToUTF8.h"

// --- GetText Settings ---
#ifdef GETTEXT_PACKAGE
#undef GETTEXT_PACKAGE
#endif
#define GETTEXT_PACKAGE "TranscriberAG"

// --- Cross-platform Definition : _ --
//#ifdef WIN32
//	#define _(a) (char*)tag::FormatToUTF8::checkUTF8(dgettext(GETTEXT_PACKAGE,a), "", true).c_str()
//#else
//	#define _(a) dgettext(GETTEXT_PACKAGE,a)
//#endif
#define _(a) (char*)tag::FormatToUTF8::checkUTF8(dgettext(GETTEXT_PACKAGE,a), "", true).c_str()

#ifdef ASSERT
#undef ASSERT
#endif

#include <iostream>
#define ASSERT(a) if (!(a)) { Log::err() << __FILE__ << ":" << __LINE__ << " : Assertion failed" << std::endl; exit(1); }

#include "util/Log.h"

#endif /* _HAVE_COMMON_GLOBALS_H */

