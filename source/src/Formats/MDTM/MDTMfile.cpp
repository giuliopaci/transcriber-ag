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
* MDTMfile.h: MetaData Time-Mark Format Class Implementation
*/


#include "MDTMfile.h"

string MDTMfile::nomatch("");


void
MDTMfile::init()
{
	try
	{
		//turnRE = new RE("(([^\t ]+)[\t ]+){8}");
		turnRE = new RE("[\t ]*([^\t ]+)[\t ]+([^\t ]+)[\t ]+([^\t ]+)[\t ]+([^\t ]+)[\t ]+([^\t ]+)[\t ]+([^\t ]+)[\t ]+([^\t ]+)[\t ]+([^\t ]+)[\t ]*");
	}
	catch ( const RE::CompError& e)
	{
		cerr << e.what() << endl;
		throw e;
	}
	
	record_line		= "";
	record_type		= "";
	record_re		= turnRE;
	nmax			= 8;
	record_no		= 0;	
}


MDTMfile::MDTMfile()
: Record(1)
{
	init();
}


MDTMfile::MDTMfile(const string& fileName)
: Record(1)
{
	open(fileName);
	init();
}


MDTMfile::~MDTMfile()
{
	delete turnRE;
}


bool
MDTMfile::read_record()
{
  read_entry();
  return (get_type() == "" ? false : true);
}


void
MDTMfile::read_entry()
{
	map<string, RE*>::iterator it;
	
	record_line = "";
	record_type	= "";
	turn_items.clear();

	// -- Reading until next turn --
	while (readline(record_line))
	{
		record_no++;

		// -- Turn Matcher --		
		if ( turnRE->match(record_line) )
		{
			if (turnRE->get_matched(5) != "speaker")
				continue;

			record_type		= "turn";

			// -- Extracting Turn Data --
			for(int i=0; i<8; i++)
				turn_items.push_back( turnRE->get_matched(i+1) );

			return;
		}
	}
} 
    

string
MDTMfile::get_matched(int i)
{ 
	return (record_re == NULL ?
		record_line :
		(i < nmax ?
			record_re->get_matched(i) :
			MDTMfile::nomatch)); 
}

