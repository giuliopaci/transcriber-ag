/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
//
//  Character encoding management
//
//  

#include "Encoding.h"
//
// supported encodings
//

namespace utils {
	
static Encoding_utf8 utf8;
static Encoding_latin1 latin1;

typedef struct {
  const char* name;
  Encoding* encoding;
} EncodingDecl;

static EncodingDecl supported_encodings[] = {
  {"utf-8", &utf8},
  {"utf8", &utf8},
  {"latin-1", &latin1},
  {"latin1", &latin1},
  {"iso-8859-1", &latin1},
  {"iso8859-1", &latin1},
  {NULL, NULL},
};

//
//  return encoding corresponding to name
//    throw exception if encoding not supported
const Encoding* Encoding::getEncoding(std::string encoding) throw (const char*)
{
  int i;
  for (i = 0;
       supported_encodings[i].name != NULL
	 && strcasecmp(supported_encodings[i].name, encoding.c_str()) != 0;
       ++i);
  if ( supported_encodings[i].encoding == NULL ) {
    std::string msg = "Unsupported encoding : ";
    msg += encoding;
    throw msg.c_str();
  }
  return supported_encodings[i].encoding;
}

//
//  Encoding utf-8 implementation

//
//  nboct: return the nb of bytes corresponding to next char in string (char*)
//    returns  -1 if invalid char sequence
//
int Encoding_utf8::nboct(const char* inpt, bool with_check) const
{
  const unsigned char* pt = (const unsigned char*)inpt;
  if ( *pt == 0 ) return 0;
  short  nb, ii;
  for ( nb = 0, ii = (*pt << 1); (ii & 0x0100) ; ++nb, (ii = ii << 1) ) ;
  if ( with_check && nb > 0 ) {  // must have nb-1 more bytes like "10xxxxxx"
    int i;
    for ( i = 1; i < nb && (*(pt+i) & 0x80); ++i);
    if ( i < nb ) return -1;
  }
  return ( nb == 0 ? 1 : nb );
}

//
//  nboct: return the nb of bytes corresponding to next char in string (string)
//    returns  -1 if invalid char sequence
//
int Encoding_utf8::nboct(const std::string& s, int pos, bool with_check) const
{
  if ( pos > s.length() ) return -1;
  if ( pos == s.length() ) return 0;

  short  nb, ii;
  for ( nb = 0, ii = (s[pos] << 1); (ii & 0x0100) ; ++nb, (ii = ii << 1) ) ;

  if ( with_check && nb > 0 ) {  // must have nb-1 more bytes like "10xxxxxx"
    int i,j;
    for ( i = 1, j=pos+1; i < nb && j < s.length() && (s[j] & 0x80); ++i, ++j);
    if ( i < nb ) return -1;
  }
  return ( nb == 0 ? 1 : nb );
}

//
//  strlen: return the length of UTF-8 coded string (char*)
//    returns negative value if invalid char sequence
//
int Encoding_utf8::strlen(const char* pt, bool with_check) const
{
  int l, nboct=0;
  for ( l = 0; (nboct = Encoding_utf8::nboct(pt, with_check)) > 0; ++l, pt+=nboct );
  return (nboct < 0 ? (-1 * l) : l);
}

//
//  strlen: return the length of UTF-8 coded string (string)
//    returns negative value if invalid char sequence
//
int Encoding_utf8::strlen(const std::string& s, int start, bool with_check) const
{
  int l, nboct=0, i;
  for ( l = 0, i=start;
	(nboct = Encoding_utf8::nboct(s, i, with_check)) > 0;
	++l, i+=nboct );
  return (nboct < 0 ? (-1 * l) : l);
}

//
//  strnlen: return the length of UTF-8 coded string bounded by stop
//    returns negative value if invalid char sequence
//
int Encoding_utf8::strnlen(const char* pt, const char* stop, bool with_check) const
{
  int l, nboct=0;
  for ( l = 0; pt < stop && (nboct = Encoding_utf8::nboct(pt, with_check)) > 0; ++l, pt+=nboct );
  return (nboct < 0 ? (-1 * l) : l);
}
//
//  strnlen_bytes: return the length of n-chars UTF-8 coded string expressed in bytes
//    returns negative value if invalid char sequence
//
int Encoding_utf8::strnlen_bytes(const char* pt, int nbchar, bool with_check) const
{
  int l, nboct=0;
  const char* begin = pt;
  for ( l = nbchar; *pt && l > 0; --l, pt += nboct ) 
		if (  (nboct = Encoding_utf8::nboct(pt, with_check))  < 0 )  break;
	l  = ( pt - begin);
  return (nboct < 0 ? (-1 * l) : l);
}


//
//  valid_string: returns true if valid utf-8 string, else false.
//
bool Encoding_utf8::valid_string(const char* pt) const 
{
  return ( Encoding_utf8::strlen(pt, true) >= 0 );
}


/**
 *  convertPunct : convert some strange utf-8 punct signs (quotes) to
 *    their basic monobyte equivalent
 */
void Encoding_utf8::convertPunct(std::string& token) 
{
	struct
	{
		char* multibyte;
		char* singlebyte;
	}
	utf8_signs[] =
	{
		{ (char*)"\342\200\230", (char*)"'" } ,
		{ (char*)"\342\200\231", (char*)"'" } ,
		{ (char*)"\342\200\234", (char*)"\"" } ,
		{ (char*)"\342\200\235", (char*)"\"" } ,
		{ NULL, NULL }
	};

	char prefix ='\342';

  if ( token.length() < 3 ) return;

  for ( int i=0; i < token.length()-2; ++i ) 
    if ( token[i] == prefix ) {
      int j;
      for ( j=0; utf8_signs[j].multibyte != NULL
	      && ! (token[i+2] == utf8_signs[j].multibyte[2]
		    && token[i+1] == utf8_signs[j].multibyte[1]) ; ++j );
      if ( utf8_signs[j].multibyte != NULL ) 
	token.replace(i,3, utf8_signs[j].singlebyte);
    }
}

} /* namespace utils */
