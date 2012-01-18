/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/*
 * CHATfile.cpp: CHILDES CHAT native format : class definition
 */

#include "CHATfile.h"

string CHATfile::nomatch("");


void
CHATfile::init()
{
	try
	{
		// -- Headers --
		optionRE				= new RE("@(.*)");
		optionValueRE			= new RE("@([^:]+):(.*)");

		// -- Turns --
		turnRE					= new RE("\\*([^:]+):[\t ]*(.*)");
		turnOptionsRE			= new RE("%([^:]+):[\t ]*(.*)");
	}
	catch ( const RE::CompError& e)
	{
		cerr << e.what() << endl;
		throw e;
	}
	
	check_headers	= true;
	turn_matched	= false;
	record_line		= "";
	record_type		= "";
	current_option	= "";
	turnSpeaker		= "";
	turnText		= "";
	record_re		= turnRE;
	nmax			= 8;
	record_no		= 0;	
}


CHATfile::CHATfile()
: Record(1)
{
	init();
}


CHATfile::CHATfile(const string& fileName)
: Record(1)
{
	open(fileName);
	init();
}


CHATfile::~CHATfile()
{
	map<string, RE*>::iterator it;

	delete optionRE;
	delete optionValueRE;
	delete turnRE;
	delete turnOptionsRE;
}


bool
CHATfile::read_record()
{
  read_entry();
  return (get_type() == "" ? false : true);
}

bool
CHATfile::read_headers()
{
	map<string, RE*>::iterator it;

	record_line = "";
	record_type = "";

	while (readline(record_line) && check_headers)
	{
		record_no++;

		// -- Optional Headers --
		if ( optionValueRE->match(record_line) )
		{
			options[ optionValueRE->get_matched(1) ] = optionValueRE->get_matched(2);
			current_option = optionValueRE->get_matched(1);
			continue;
		}
		else
		if ( optionRE->match(record_line) )
		{
			options[ optionRE->get_matched(1) ] = "true";
			continue;
		}


		// -- Turn --> End of headers --
		if ( turnRE->match(record_line) )
		{
			check_headers	= false;
			current_option	= "";
			turnSpeaker		= turnRE->get_matched(1);
			turnText		= turnRE->get_matched(2);
			return true;
		}

		// -- Unmatched Text -> will be added to lastly parsed option/turn --
		if (!current_option.empty())
			options[current_option] += record_line;
	}

	return true;
}


void
CHATfile::read_entry()
{
	map<string, RE*>::iterator it;
	
	record_line = "";

	// -- Turn Restored --
	if (turn_matched)
	{
		turnOptions.clear();
		turnSpeaker	= turnRE->get_matched(1);
		turnText	= turnRE->get_matched(2);
	}

	// -- Reading until next turn --
	while (readline(record_line))
	{
		record_no++;

		// -- Turn Matcher --		
		if ( turnRE->match(record_line) )
		{
			turn_matched	= true;
			record_type		= "turn";
			return;
		}
		else
		{
			turn_matched = false;

			// -- EOF --
			if (optionRE->match(record_line))
			{
				record_type = "eof";
				return;
			}

			// -- Turn Option --
			if (turnOptionsRE->match(record_line))
			{
				turnOptions.insert( pair<string, string>(turnOptionsRE->get_matched(1), turnOptionsRE->get_matched(2)) );
				continue;
			}

			
			// -- Extra lines (turn text) --
			turnText += record_line;
			continue;
		}
		
		return;
	}
} 
    

string
CHATfile::get_matched(int i)
{ 
	return (record_re == NULL ?
		record_line :
		(i < nmax ?
			record_re->get_matched(i) :
			CHATfile::nomatch)); 
}

