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
 * LBLfile.h: LBL file format class implementation
 */


#include "LBLfile.h"

string LBLfile::nomatch("");


void LBLfile::init()
{
	try {
		headerRE["all"] = new RE("^;;");
		headerRE["encoding"] = new RE("encoding ([^ ]+)");
		headerRE["version"] = new RE("transcribed by ([^,]+), version [0-9]+ of ([0-9]+)");
		string time = "([0-9.]+)" ;
		string text = "(.*)" ;
		string spaces = "[ \t]*" ;
		string space = "[ \t]*" ;
		//			1.start name		2.text				 
		turnRE = new RE(time + spaces + text + space + "$") ;
	} 
	catch ( const RE::CompError& e) {
		cerr << e.what() << endl;
		throw e;
	}

	//> LBL FORMAT: no header
	check_headers = false ;
	record_line = "" ;
	record_type = "" ;
	record_re=turnRE ;
	nmax=8 ;
	record_no = 0 ;	
}


LBLfile::LBLfile() : Record(1)
{
	init();
}


LBLfile::LBLfile(const string& filename) : Record(1)
{
	open(filename);
	init();
}


LBLfile::~LBLfile()
{
	map<string, RE*>::iterator it ;
	for ( it=headerRE.begin(); it != headerRE.end(); ++it )
		delete it->second;
	delete turnRE;
}


bool LBLfile::read_record()
{
	read_entry();
	return (get_type() == "" ? false : true);
}


void LBLfile::read_entry()
{
	map<string, RE*>::iterator it ;

	record_line = "";
	record_type = "";

	while (readline(record_line))	
	{
		++record_no;

		// readline will return a line without leading and ending white spaces
		if (record_line != "") 
		{
			if ( check_headers ) {
				if ( headerRE["all"]->match(record_line) )
				{
					if ( items["encoding"] == "" 
						&& headerRE["encoding"]->match(record_line) ) {
						items["encoding"] = headerRE["encoding"]->get_matched(1);
						// TODO normaliser nom encodage
						//items["encoding"] = normalize(items["encoding"]);
					}
					if ( items["version"] == "" 
						&& headerRE["version"]->match(record_line) ) {
						items["author"] = headerRE["version"]->get_matched(1);
						items["date"] = headerRE["version"]->get_matched(2);
					}
					continue ;
				} 
				else 
					check_headers = false;
			}

			if ( turnRE->match(record_line) ) 
				record_type = "turn";
			else 
				record_type = "unknown";
			return ;  // turn or text -> always return
		}
	}
} 


string LBLfile::get_matched(int i)
{ 
	return (record_re == NULL ?
				record_line :
	(i < nmax ?
				record_re->get_matched(i) :
					LBLfile::nomatch)); 
}
