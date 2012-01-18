/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
//------------------------------------------------------------------------
// File : str.cc
//
//------------------------------------------------------------------------

#include <string>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>

using namespace std;

#include "str.h"

char ToAscii(char c, char repchar) {
    switch ((unsigned char)c) {
    case 130:
    case 147:
    case 224:
    case 232:
    case 233:
    case 234:
    case 244:
      return c;
    default:
      return ((c > 31) && (c < 127) ? c : repchar);
    }
}

//------------------------------------------------------------------------
// StrEq
//------------------------------------------------------------------------

int StrEq(const char *p, const char *q)
{
  if ( !p || !q ) return 0;
  while (*p && *q && (*p == *q)) p++, q++;
  return (*p == *q) ;
} // StrEq 

int StrEq(const char *p, const char *q, int n)
{
  if ( !p || !q ) return 0;
  while (n && *p && *q && (*p == *q)) p++, q++, n--;
  return ( !n ) ;
} // StrEq 

int StrEq(const char *p, const char *q, const char* delims)
{
  if ( !p || !q ) return 0;
  while ( *p && *q && !strchr(delims, (int)*q) && (*p == *q)) p++, q++;
  return ( *p == 0 && ( *q == 0  || strchr(delims,(int)*q) ) ) ;
} // StrEq 

int StrEq_abbrev(const char* str, const char* ref, int minl)
{
  register int l;
  for (l=0; str[l] && ref[l] && str[l] == ref[l]; ++l);
  if (l<minl) return 0;
  else return (str[l]==0 || str[l] == ref[l] ? l : 0);
}


//------------------------------------------------------------------------
// StrCpy : retourne le nb de caracteres copies
//------------------------------------------------------------------------

int StrCpy(char *p, const char *q)
{
  const char* qo=q;
  if ( !p || !q ) return 0;
  while ((*p = *q)) p++, q++;
  return ((q-qo)) ;
} // StrCpy 

int StrCpy(char *p, const char *q, int n)
{
  const char* qo=q;
  if ( !p || !q ) return 0;
  while (n && (*p = *q))  p++, q++, n--;
  *p = 0;
  return ((q-qo)) ;
} // StrCpy 

int StrCpy(char *p, const char *q, const char* delims)
{
  const char* qo=q;
  if ( !p || !q ) return 0;
  while ( !strchr(delims, (int)*q) && (*p = *q)) p++, q++;
  *p = 0;
  return ((q-qo)) ;
} // StrCpy 

//------------------------------------------------------------------------
// StrGt 
//------------------------------------------------------------------------

int StrGt(const char *p, const char *q)
{
  while (*p && *q && (*p == *q)) p++, q++;
  return (*p > *q) ;
} // StrGt 

//------------------------------------------------------------------------
// StrDup
//------------------------------------------------------------------------

char* StrDup(const char* s, int l)
{
	if ( ! s ) return NULL;
	int	ls=strlen(s);
	if ( l > -1 && l < ls ) ls = l;
	
	char *pt=new char[ls+1];
	if ( ls) strncpy(pt, s, ls);
	pt[ls]=0;
	return pt;
}

char* StrDup(const char* s, const char* delims, int* l)
{
	if ( ! s ) return NULL;
	int	ls=strcspn(s, delims);
	
	char *pt=new char[ls+1];
	if ( ls) strncpy(pt, s, ls);
	pt[ls]=0;
	if ( l ) *l=ls;
	return pt;
}

//------------------------------------------------------------------------
// StrCat
//------------------------------------------------------------------------

char* StrCat(char* s1, const char* s2, int l)
{
	if ( s1 && s2 ) { 
		int	ls=strlen(s2);
		if ( l > -1 && l < ls ) ls = l;
		
		char *pt=new char[strlen(s1)+ls+1];
		strcpy(pt, s1);
		if ( ls ) strncat(pt, s2, ls);
		pt[strlen(s1)+ls]=0;
		delete[] s1;
		return pt;
	} else return NULL;
}

char* StrCat(char* s1, const char* s2, const char* delims)
{
	if ( s1 && s2 ) {
		int	ls=strcspn(s2, delims);
	
		return StrCat(s1, s2, ls);
	} else return NULL;
	
}

//------------------------------------------------------------------------
//  StrPad : renvoie une chaine de chars remplie de n chars.
//------------------------------------------------------------------------

const char* StrPad(int n, char padcar)
{
  static string buf;
  register int i;

  for(i=0, buf=""; i<n; buf += padcar, i++);
  return buf.c_str();

}

//------------------------------------------------------------------------
// StrSub : extrait une sous-chaine
//------------------------------------------------------------------------

const char* StrSub(const char* s, int l)
{
  static string strbuf;
  register int i;
  
  if ( ! s || l < 0 ) return "";

  for (i=0, strbuf.erase() ; i<l && s[i]; strbuf += s[i++]);
  return strbuf.c_str();
}

const char* StrSub(const char* s, const char* delims)
{
  if ( ! s ) return "";
  
  s += strspn(s, delims);
  int ls = strcspn(s, delims);
  return StrSub(s, ls);
}


const char* StrTok(const char* s, const char* delims, const char** n)
{
  if ( ! s ) return "";
  
  static string token;

  s += strspn(s, delims);
  int ls = strcspn(s, delims);
  if ( n != NULL ) *n = (s+ls);
  token = StrSub(s, ls);
  return token.c_str();
}

string StrTrim(const string& s)
{
  unsigned int deb,fin;
  for ( deb=0; deb < s.length() && isspace(s[deb]); ++deb);
  for ( fin=s.length()-1; fin > deb && isspace(s[fin]); --fin);
  if ( fin > deb ) 
    return s.substr(deb,(fin-deb+1));
  else return string("");
}


int StrSplit(const char* s, const char* delims, vector<string>& vec)
{
  vec.clear();
  if ( !s ) return 0;
  const char* psuiv;
  const char* tok;
  for ( tok = StrTok(s, delims, &psuiv);
	*tok;
	tok = StrTok(psuiv, delims, &psuiv) ) 
    vec.push_back(tok);
  return vec.size();
}

//------------------------------------------------------------------------
// StrToUpper : passe une chaine en majuscule
//------------------------------------------------------------------------

const char* StrToUpper(const char* s, int l)
{
  static string	strbuf;
  register int i;
  
  if ( !s ) return NULL;
  
  if ( l == -1 ) l = strlen(s);
  
  for (i=0, strbuf=""; s[i] && i<l; i++ )
    strbuf += ( s[i] >= 'a' && s[i] <= 'z'  ? toupper(s[i]) : s[i]);
  for (; s[i]; strbuf += s[i], i++);
  return strbuf.c_str();
}

const char* StrToUpper(const char* s, const char *delims)
{
  if ( !s ) return NULL;
  
  int	ls=strcspn(s, delims);
  return StrToUpper(s, ls);
}

//------------------------------------------------------------------------
// StrToLower : passe une chaine en minuscule
//------------------------------------------------------------------------

const char* StrToLower(const char* s, int l)
{
  static string	strbuf;
  register int i;

  if ( !s ) return NULL;
  
  if ( l == -1 ) l = strlen(s);
  
  for (i=0, strbuf=""; s[i] && i<l; i++ )
    strbuf += ( s[i] >= 'A' && s[i] <= 'Z'  ? tolower(s[i]) : s[i]);
  for (; s[i]; strbuf += s[i], i++);
  return strbuf.c_str();
}

const char* StrToLower(const char* s, const char *delims)
{
  if ( !s ) return NULL;
  
  int	ls=strcspn(s, delims);
  return StrToLower(s, ls);
}


//------------------------------------------------------------------------
//  MakeStr : alloue et formatte une chaine
//  MakeCStr :  formatte une chaine et retourne un pointeur sur un buffer 
//              interne qui sera modifie au prochain appel de la fonction
//------------------------------------------------------------------------

char* MakeStr(const char* fmt, ...)
{
  va_list ap;
  static char ibuf[512];
  int sz;
  char *retbuf;

  va_start(ap, fmt);
  // (void)vsprintf(buf, fmt, ap);
  if ( (sz = vsnprintf(ibuf, sizeof(ibuf), fmt, ap)) >= (int)sizeof(ibuf) ) {
    // La chaine a ete tronquee, reallouons un buffer suffisamment grand
    retbuf = new char[sz+2];
    vsnprintf(retbuf, sz+2, fmt, ap);
  } else retbuf = StrDup(ibuf);
  va_end(ap);
  return retbuf;
}

const char* MakeCStr(const char* fmt, ...)
{
  va_list ap;
  static int cbuf_sz=1024;
  static char* cbuf=NULL;
  int sz;
  
  if ( cbuf == NULL ) cbuf = new char[cbuf_sz];

  va_start(ap, fmt);

  if ( (sz = vsnprintf(cbuf, cbuf_sz ,fmt, ap)) >= cbuf_sz ) {
    cbuf_sz = sz + 2;
    delete[] cbuf;
    cbuf =  new char[cbuf_sz];
    vsnprintf(cbuf, cbuf_sz ,fmt, ap);
  }

  va_end(ap);
  return (const char*)cbuf;
}


const char* Adr2Str(uint64_t adr, int lg)
{
  static char bufadr[128];

  sprintf(bufadr, "%0*llX", lg*2, adr);
  return (const char*)bufadr;
}

bool Str2Adr(const char* adrstr, uint64_t& offset)
{
  int ok;

  ok=sscanf(adrstr, "%llx",  &offset);
  return (ok == 1);
}


const char* Itoa(int i)
{
  static char itoabuf[20];
  sprintf(itoabuf, "%d", i);
  return (const char*)itoabuf;
}

#define H2V(c) ( ( (c) <= '9') ? ( (c) - '0') : (10 + (c) - 'A') )
uint64_t strtouint64(const char* str, const char **endptr, int base) {
  size_t n, i;
  const char *p;
  uint64_t ret;
  char c;

  p = str;

  while(isspace(*p))
    p++;

  switch(base) {
  case 10:
    n = strspn(p, "0123456789");
    break;
  case 16:
    if(*p && *(p+1)
       && *p == '0' && *(p+1) == 'x')
      p += 2;

    n = strspn(p, "0123456789ABCDEFabcdef");
    break;
  default:
    n = 0;
    break;
  }

  if(n == 0) {
    if(endptr)
      *endptr = str;
    return 0;
  }

  ret = 0;
  for(i = 0; i < n; i++) {
    switch(base) {
    case 10:
      ret = ret * 10 + (p[i] - '0');
      break;
    case 16:
      c = toupper(p[i]);
      ret = (ret<<4) + H2V(c);
      break;
    }
  }

  if(endptr)
    *endptr = p + n;
  return ret;
}


//------------------------------------------------------------------------
//  IsNumeric : renvoie Vrai si une chaine n'est composee que de chiffres
//------------------------------------------------------------------------

int IsNumeric(const char *p)
{
  while (*p && (*p >= '0') && (*p <= '9') ) p++;
  return (!*p);
}

int IsNumeric(const char *p, int l)
{
  const char* f=p+l;
  while (*p && p < f && (*p >= '0') && (*p <= '9') ) p++;
  return (!*p || p==f);
}


//------------------------------------------------------------------------
//	Dirname, Basename : idem commandes systeme du meme nom
//	NameLessSuffix : retourne le path du fichier sans son eventuel suffixe
//
//	ATTENTION : renvoient un pointeur sur un buffer interne -> utiliser le
//			resultat avant un nouvel appel a ces fonctions
//------------------------------------------------------------------------

const char *Dirname(const char* path)
{
  static char dirname[1024];
  const char	*pt;
  
  if ( !path) return NULL;
  
  if ( ! *path ) return ".";
  
  for (pt=path+strlen(path)-2; pt >= path && *pt != '/' ; pt--);
  if ( pt < path ) return ".";
  else if ( pt == path ) return "/";
  else {
    strncpy(dirname, path, (pt-path)); 
    dirname[(pt-path)] = 0;
    return dirname;
  }	
}

const char *Basename(const char* path)
{
  static char basename[256];
  const char	*pt;
  
  if ( !path) return NULL;
  
  if ( ! *path ) return ".";
  
  for (pt=path+strlen(path)-2; pt >= path && *pt != '/' ; pt--);
  strcpy(basename, (pt+1)); 
	if ( basename[strlen(basename)-1] == '/' ) basename[strlen(basename)-1] = 0;
	return basename;
}

const char *BasenameP(const char* path)
{
  const char	*pt;
  
  if ( ! path ) return NULL;
	
  for (pt=path+strlen(path)-2; pt >= path && *pt != '/' ; pt--);
  return (pt+1);
}

const char*	NameLessSuffix(const char* filename)
{
  static char bufname[256];

  (void)strcpy(bufname, filename);
  
  char	*pt=strrchr(bufname, '.');
  char	*pt2=strrchr(bufname, '/');
  
  if ( pt && pt > pt2 ) *pt=0;
  
  return bufname;
} 

//------------------------------------------------------------------------
//	StrMakeFormat:	controle les formats de lecture/ecriture,
//		et traduit les chaines de caracteres "\n" en '\n' ,
//		"\t" en '\t', ...
//------------------------------------------------------------------------

void StrMakeFormat(char *fmt) 
{
  int	 i,j;
  int	esc;
  
  for ( i=0, j=0, esc=0; fmt[i]; i++ )
    switch (fmt[i]) {
    case '\\': if ( esc ) {
      fmt[j++] = '\\'; esc = 0;
    } else esc = 1;
    break;
    case 'n': fmt[j++] = ( esc ? '\n' : 'n'); esc = 0; break;
    case 't': fmt[j++] = ( esc ? '\t' : 't'); esc = 0; break;
    default: fmt[j++] = fmt[i]; break;
    }
  fmt[j] = '\0';
}

const char* StrParseFormatStr(const char *fmt) 
{
  int	 i;
  int	esc;
  static string parsed("");
  
  parsed = "";
  for ( i=0, esc=0; fmt[i]; i++ )
    switch (fmt[i]) {
    case '\\': if ( esc ) {
      parsed += "\\"; esc = 0;
    } else esc = 1;
    break;
    case 'n': parsed += ( esc ? "\n" : "n"); esc = 0; break;
    case 't': parsed += ( esc ? "\t" : "t"); esc = 0; break;
    default:
      if ( esc && isdigit(fmt[i])
	   && isdigit(fmt[i+1]) && isdigit(fmt[i+2]) ) {
	unsigned char c = ((fmt[i]-'0')*64) + ((fmt[i+1]-'0')*8) + (fmt[i+2]-'0');
	parsed += (char)c;
	i += 2;
      } else 
	parsed += fmt[i];
      esc = 0;
      break;
    }
  return parsed.c_str();
}


//------------------------------------------------------------------------
// JustifyToWord : retourne une chaine avec des retours chariot � la longueur
//		voulue, ou au mot pr�c�dent si cela evite une troncature de mot
// JustifyToLen : retourne une chaine avec des retours chariot � la longueur
//		voulue meme si cela cause une troncature de mot
//
//	ATTENTION : renvoient un pointeur sur un buffer interne -> utiliser le
//			resultat avant un nouvel appel a ces fonctions
//
//------------------------------------------------------------------------

const char* JustifyToWord(const char* buf, int llen)
{
  static string texte;
  char sub[256];
  int	j=0, l=0, lsub;
  if ( !buf ) return "";
  
  texte="";
  if ( llen >= (int)sizeof(sub) ) {
    fprintf(stderr, "JustifyToWord: len max ligne = %d\n", sizeof(sub)-1);
    llen = (sizeof(sub)-1);
  }
  
  while ( buf[l] ) {
    j=texte.size();
    strncpy(sub, buf+l, llen); sub[llen]=0; lsub=strlen(sub);
    while ( lsub > 0 && strchr(" \t\n",sub[lsub-1]) == NULL ) lsub--;
    if ( lsub == 0 ) lsub = llen;
    else sub[lsub]=0;
    texte += sub;
    l += (texte.size() - j);
    texte += "\n";
  }	
  
  return texte.c_str();
}

const char* JustifyToLen(const char* buf, int llen)
{
  static string texte;
  int	j=0, l=0;
  if ( !buf ) return "";
  
  texte="";
  
  while ( buf[l] ) {
    j=texte.size();
    texte += StrSub(buf+l, llen);
    l += (texte.size()-j);
    texte += "\n";
  }	
  return texte.c_str();
}

void vstr_printf(char *dest, const char *format, char *tab[], int nb_elem) {
  int i, n, len;

  i = n = 0;
  while(format[i]) {
    if(format[i] == '%') {
      if(format[i+1] == '%') {
	*(dest++) = '%';
	i += 2;
	continue;
      }

      if(format[i+1] == 's') {
	int j;

	len = strlen(tab[n]);
	for(j = 0; j < len; j++)
	  *(dest++) = tab[n][j];
	
	i += 2;
	n++;
	continue;
      }

      if(format[i+1] >= '0' && format[i+1] <= '9') {
	int j, val = 0;

	for(j = i + 1; format[j] && format[j] >= '0' && format[j] <= '9'; j++)
	  val = val * 10 + (format[j] - '0');

	if(format[j] && format[j] == '$' && format[j+1] == 's') {
	  int k;

	  assert(val > 0 && val-1 < nb_elem);
	  val--;

	  len = strlen(tab[val]);
	  for(k = 0; k < len; k++)
	    *(dest++) = tab[val][k];

	  i = j + 2;
	  continue;
	}
      }
    }
    *(dest++) = format[i++];
  }
  *dest = '\0';
}

//------------------------------------------------------------------------
// test module
//------------------------------------------------------------------------

#ifdef TEST

#include <iostream>

main()
{
  cout << "StrToUpper(\"aga/OGO\") = " << StrToUpper("aga/OGO") << endl;
  cout << "StrToUpper(\"aga/OGO\",2) = " << StrToUpper("aga/OGO",2) << endl;
  cout << "StrToUpper(\"aga/OGO\",\"g\") = " << StrToUpper("aga/OGO","g") << endl;
  cout << "StrToLower(\"aga/OGO\") = " << StrToLower("aga/OGO") << endl;
  cout << "StrToLower(\"aga/OGO\",6) = " << StrToLower("aga/OGO",6) << endl;
  cout << "StrToLower(\"aga/OGO\",\"G\") = " << StrToLower("aga/OGO","G") << endl;
  cout << endl;
  cout << "StrDup(\"aga/OGO\") = " << StrDup("aga/OGO") << endl;
  cout << "StrDup(\"aga/OGO\",5) = " << StrDup("aga/OGO",5) << endl;
  cout << "StrDup(\"aga/OGO\",\"/\") = " << StrDup("aga/OGO","/") << endl;
  cout << "StrSub(\"aga/OGO\",5) = " << StrSub("aga/OGO",5) << endl;
  cout << "StrSub(\"aga/OGO\",\"/\") = " << StrSub("aga/OGO","/") << endl;
  
  return 0;
}

#endif // TEST

#ifdef TEST_TEXTE

#include <iostream.h>

char* buf= "c'est un serpent python, c'est un python serpent, qui se promene"
			" dans la foret en cherchant a devorer... Un beau petit lapin, ou"
			" bien un negre fin, car le serpent python a faim, il a une faim "
			"sans fin ! Les animaux sont partis hier";
main()
{
	cout << "JustifyToWord:" << endl;
  	cout << JustifyToWord(buf, 40) << endl << endl;
	cout << "JustifyTolen:" << endl;
	cout << JustifyToLen(buf, 40) << endl << endl;
	
	return 0;
}

#endif // TEST_TEXTE

#ifdef TEST_FMT
main()
{
  char* fmt = "il fait\\ttres beau\\nce matin!\\n";

  printf("%s -> %s -- \n", fmt , StrParseFormatStr(fmt));
  
}
#endif // TEST_FMT
