/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
//
//   ExpRegul.cc : classe de gestion d'expressions regulieres
//

#include <iostream>
#include "ExpRegul.h"
#include "str.h"

//
//  Constructeur : renvoie une exception si erreur dans l'expression
//          ou dans la post-condition
//
ExpRegul::ExpRegul(const char* expregul, bool nocase ) throw (const char*)
  : _expregul(expregul), _noelmax(0)
{
  if ( ! compileExpreg(nocase)  )
    throw errmsg();
}

//
//
//  mise � jour expression
bool ExpRegul::setExpr(const char* expregul, bool nocase )
{
  _expregul = expregul;

  if ( !  compileExpreg(nocase) )
    throw errmsg();
	return true;
}

//
//
//  compilation de l'expression reguliere
bool ExpRegul::compileExpreg(bool nocase)
{
  char msg[128];
  // on compile l'expression reguliere
  //cout << "ExpRegul : --" << _expregul << "--"<<endl;
  int opts = REG_EXTENDED|REG_NEWLINE;
  if ( nocase ) opts += REG_ICASE;
  int resu = regcomp(&_pattern, _expregul.c_str(), opts);

  if (resu != 0) {  // erreur de compilation
    regerror(resu, &_pattern, msg, sizeof(msg));
    _msg="Erreur dans l'expression reguli�re :\n";
    _msg += msg;
    return false;
  }
  return true;
}


//
//  match : effectue la recherche, 
//     et renvoie le premier offset de match dans le buffer
//  match[0] = le pattern global
//    match[1..n] les sous patterns
bool ExpRegul::match(const string& buffer, unsigned long& start )
{
  
  const char* pbuf = buffer.c_str();
  int trouve;

  if ( start >= buffer.length() ) return false;
  
    trouve = regexec(&_pattern, pbuf+start, MAX_MATCH, _match, 0);
    
    // erreur de recherche ou plus d'occurrence ...
    if (trouve != 0) {
      if ( trouve != REG_NOMATCH ) {
		char msg[256];
		regerror(trouve, &_pattern, msg, sizeof(msg));
		_msg = "Erreur interne lors de la recherche : ";
		_msg += msg;
     }
      return false;
    } 
    
   	start += _match[0].rm_eo;
 
  return true;
}

//
//  getSubmatch : recupere la valeur d'une sous-expression de l'expression
//     reguliere (definie entre parentheses)
string ExpRegul::getSubmatch(const string& buffer, int noelem) 
{
  int j;

  for ( j=1; j < noelem && _match[noelem].rm_so != -1; ++j);
  if ( j == noelem && _match[noelem].rm_so != -1 ) {
    j = _match[noelem].rm_eo - _match[noelem].rm_so;
    return buffer.substr(_match[noelem].rm_so, j);
  }
  return "";
}

