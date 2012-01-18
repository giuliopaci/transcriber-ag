/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* $Id$
*
* VSFTfile.h: VS Fast Transcription file format class implementation
*/

#include <string.h>
#include "VSFTfile.h"

string VSFTfile::nomatch("");

struct _lcode {
	const char* acro;
	const char* iso;
} LanguageCodes[] = {
		{ "GB", "eng" },
		{ "FR", "fre" },
		{ NULL, NULL },
};


void
VSFTfile::init()
{
	try {
  headerRE["identification"] = new RE("# *[iI]nformations *[ =:]? *([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +([^-]+)-([^ ]+)");
  headerRE["start"] = new RE("# *[sS]tart *[ =:]? *([^ \n]+)");
  headerRE["end"] = new RE("# *[eE]nd *[ =:]? *([^ \n]+)");
  headerRE["duration"] = new RE("# *[dD]uration *[ =:]? *([^ \n]+)");
  headerRE["signalFile"] = new RE("# *[sS]ignal( *[fF]ile)? *[ =:]? *([^ \n]+)");

  turnRE["turn"] = new RE("^[sS]([0-9]+)(\\(.*\\))?/ *(.*) *$");

  nsub["identification"] = 6;
  nsub["start"] = 1;
  nsub["end"] = 1;
  nsub["duration"] = 1;
  nsub["signalFile"] = 2;
  nsub["turn"] = 3;

	} catch ( const RE::CompError& e) {
		cerr << e.what() << endl;
    		throw e;
	}

	check_headers = true;
	record_line = "";
	record_type = "";
	record_re=NULL;
	nmax=0;
	record_no = 0;
}


VSFTfile::VSFTfile()
: Record(1)
{
  init();
}


VSFTfile::VSFTfile(const string& filename)
: Record(1)
{
  open(filename);
  init();
}


VSFTfile::~VSFTfile()
{
	map<string, RE*>::iterator it ;

	for ( it=headerRE.begin(); it != headerRE.end(); ++it )
		delete it->second;
	for ( it=turnRE.begin(); it != turnRE.end(); ++it )
		delete it->second;
}

void
VSFTfile::read_entry()
{
	map<string, RE*>::iterator it ;

	record_line = "";
	record_type = "";
	record_re=NULL;
	nmax=0;

  while (readline(record_line))
  {
	++record_no;
	record_type = "";

    // readline will return a line without leading and ending white spaces
    if (record_line == "")
      continue;

	if ( check_headers ) {
		for ( it=headerRE.begin(); it != headerRE.end(); ++it ) {
			if ( it->second->match(record_line) ) {
				if ( it->first == "identification" ) {
					int i;
					items["lang"] = it->second->get_matched(1);
					for (i=0; LanguageCodes[i].acro != NULL && strcasecmp(LanguageCodes[i].acro, items["lang"].c_str()) != 0; ++i);
					if ( LanguageCodes[i].acro != NULL ) items["lang"] = LanguageCodes[i].iso;
					items["author"] = it->second->get_matched(5);
					items["date"] = it->second->get_matched(6);
				} else
					items[it->first] = it->second->get_matched(1);
				break;
			}
		}
		if ( it == headerRE.end() ) {
			// end of headers -> will not be checked anymore
			check_headers = false;
		}  else record_type = "header";
	}

	if ( record_type.empty() ) {
	//for ( it=turnRE.begin(); it != turnRE.end(); ++it ) {
		// only one expr -> no loop
		it = turnRE.begin();
		if ( it->second->match(record_line) ) {
			record_type = it->first;
			record_re = it->second;
			nmax = nsub[record_type]+1 ;
		} else
			record_type = "text";
	//}
		return;  // turn or text -> always return
	}
  }
}


bool
VSFTfile::read_record()
{
  read_entry();
  return (get_type() == "" ? false : true);
}


string
VSFTfile::get_matched(int i)
{
	return (record_re == NULL ?
		record_line :
		(i < nmax ?
			record_re->get_matched(i) :
			VSFTfile::nomatch));
}
