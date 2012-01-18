/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 **  SccsId:	@(#)sig.h	2.1	4/1/93
 **
 **  Titre:
 **	defines, macros et prototypes pour le module de gestion de signaux
 **	systeme
 **
 **
 **
 **/

/**
* @file 		sig.h
* Defines, macros and prototypes for signals management module
* @deprecated 	Not used anymore
*/

#ifndef _HAVE_SIG_H
#define _HAVE_SIG_H

#include <signal.h>

typedef void (*exit_func_t)();
typedef void (*sig_handler_t)(int);

#ifdef __cplusplus
extern "C" {
#endif

  void sig_init(exit_func_t ptr);	/* initialisation des gestionnaires de signaux */
  void main_exit();
  void sig_sigchld_handler(sig_handler_t handler); /* handler pour SIGCHLD */

#ifdef __cplusplus
}
#endif

#endif	/* _HAVE_SIG_H */
