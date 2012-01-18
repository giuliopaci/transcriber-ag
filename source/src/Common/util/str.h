/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/*
**  $Revision: 1.1.1.1 $	$Date: 2001/03/19 14:33:35 $	$Author: lecuyer $
**  $Locker:  $
**
**	str.h : fonctions de manupulation de chaines de caracteres
*/

/**
* @file 		str.h
* String manipulation
*/

#ifndef _HAVE_STR_H
#define _HAVE_STR_H

#include <string>
#include <vector>
#include <string.h>
#include <stdint.h>
#include <sys/param.h>

using namespace std;

#define MAX_STR_LENGTH    65537

extern char ToAscii(char c, char repchar);

extern int StrEq(const char*, const char*);
extern int StrEq(const char*, const char*, int n);
extern int StrEq(const char*, const char*, const char* delims);
extern int StrEq_abbrev(const char* str, const char* ref, int minl=1);
extern int StrCpy(char*, const char*);
extern int StrCpy(char*, const char*, int n);
extern int StrCpy(char*, const char*, const char* delims);
extern int StrGt(const char*, const char*);
extern char* StrDup(const char* s, int l=-1);
extern char* StrDup(const char* s, const char *delims, int* l=NULL);
extern char* StrCat(char* s1, const char* s2, int l=-1);
extern char* StrCat(char* s1, const char* s2, const char *delims);
extern const char* StrSub(const char* s, int l);
extern const char* StrSub(const char* s, const char* delims);
extern const char* StrTok(const char* s, const char* delims, const char** ns);
extern string StrTrim(const string& s);
extern int StrSplit(const char* s, const char* delims, vector<string>& vec);
extern const char* StrToUpper(const char* s, int l=-1);
extern const char* StrToUpper(const char* s, const char *delims);
extern const char* StrToLower(const char* s, int l=-1);
extern const char* StrToLower(const char* s, const char *delims);
extern const char* StrPad(int n, char padcar);
extern int IsNumeric(const char*);
extern int IsNumeric(const char*, int l);
const char *Basename(const char*path);
const char *BasenameP(const char* path);
const char *Dirname(const char*path);
const char* NameLessSuffix(const char* filename);
void StrMakeFormat(char *fmt) ;
const char* StrParseFormatStr(const char *fmt) ;
char* MakeStr(const char* fmt, ...);
const char* MakeCStr(const char* fmt, ...);
const char* Adr2Str(uint64_t adr, int lg=4);
bool Str2Adr(const char* adrstr, uint64_t& offset);
const char* Itoa(int i);
uint64_t strtouint64(const char* str, const char **endptr, int base);
const char* JustifyToWord(const char* buf, int llen);
const char* JustifyToLen(const char* buf, int llen);
extern void vstr_printf(char *dest, const char *format, char *tab[], int nb_elem);

#endif /* _HAVE_STR_H */
