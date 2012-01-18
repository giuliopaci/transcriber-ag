/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "SGMLobjects.h"
#include <iostream>
#include "Common/util/StringOps.h"
#include "Common/util/Utils.h"


//------------------------------------------------------------------------------
//--------------------------------- SGML ENTRY ---------------------------------
//------------------------------------------------------------------------------

SGMLobjects::SGMLentry::SGMLentry(Glib::ustring entry_s)
{
	//> init
	type = "" ;
	ref_word = "" ;
	hyp_word = "" ;
	start_time = -1 ;
	end_time = -1 ;
	percent = -1 ;
	checked = false ;

	if (!entry_s.empty())
	{
		std::vector<string> vect ;
		mini_parser(',', entry_s, &vect) ;

		// for deletion percent doesn't seem obligatory
		if (vect.size()==5 || vect.size()==4)
		{
			type = vect[0] ;
			ref_word = formatText(vect[1]) ;
			hyp_word = formatText(vect[2]) ;
			string times = vect[3] ;
			string tmp = vect[4] ;
			if ( !tmp.empty() )
				percent = atof(tmp.c_str()) ;

			std::vector<string> vect2 ;
			StringOps timop(times) ;
			timop.split(vect2, "+", true) ;
			if ( vect2.size()==2 )
			{
				string tmp = vect2[0] ;
				if ( !tmp.empty() )
					start_time = my_atof(tmp.c_str()) ;
				tmp = vect2[1] ;
				if ( !tmp.empty() )
					end_time = my_atof(tmp.c_str()) ;
				checked = true ;
			}
		}
	}
}

Glib::ustring SGMLobjects::SGMLentry::toString()
{
	Glib::ustring res ;
	res.append( "\t\t\t" + type + " , ") ;
	res.append( ref_word + " , ") ;
	res.append( hyp_word + " , ") ;
	res.append( number_to_string(start_time) + " , ") ;
	res.append( number_to_string(end_time) + " , ") ;
	res.append( number_to_string(percent) ) ;
	return res ;
}


std::string SGMLobjects::SGMLentry::formatText(string text)
{
	if (text.empty())
		return text ;

	std::string tmp = text ;
	std::string::iterator it = tmp.begin() ;
	if (*it == '\"')
		tmp.erase(it) ;

	it = tmp.end() ;
	it -- ;
	if (*it == '\"')
		tmp.erase(it) ;

	return tmp ;
}


//------------------------------------------------------------------------------
//--------------------------------- SGML PATH  ---------------------------------
//------------------------------------------------------------------------------

SGMLobjects::SGMLpath::SGMLpath()
{

}

void SGMLobjects::SGMLpath::set_entries(Glib::ustring data)
{
	if (data.empty())
		return ;

	string tmp = data ;
	std::vector<string> vect ;
	mini_parser(':', tmp, &vect) ;
	std::vector<string>::iterator it ;
	for (it=vect.begin(); it!=vect.end(); it++) {
		if ( !(*it).empty() ) {
			SGMLobjects::SGMLentry* entry = new SGMLobjects::SGMLentry(*it) ;
			entries.push_back(entry) ;
		}
	}
}

SGMLobjects::SGMLpath::~SGMLpath()
{
	std::vector<SGMLobjects::SGMLentry*>::iterator it ;
	for (it=entries.begin(); it!=entries.end(); it++) {
		if (*it)
			delete(*it) ;
	}
}

Glib::ustring SGMLobjects::SGMLpath::toString()
{
	Glib::ustring res ;
	res.append("\t\t(( PATH ") ;
	res.append(id + " - ") ;
	res.append(word_cnt + " - ") ;
	res.append(labels + " - ") ;
	res.append(chanel + " - ") ;
	res.append(sequence + " - ") ;
	res.append(word_aux + " - ") ;
	res.append(number_to_string(R_T1) + " - ") ;
	res.append(number_to_string(R_T2) + " - \n") ;

	std::vector<SGMLobjects::SGMLentry*>::iterator it ;
	for (it=entries.begin(); it!=entries.end(); it++) {
		res.append("\n") ;
		res.append( "\t\t" + (*it)->toString()) ;
	}

	res.append( "\t\t\nPATH ))") ;
	return res ;
}

float SGMLobjects::SGMLpath::getLastTime()
{
	SGMLentry* last = NULL ;
	std::vector<SGMLentry*>::reverse_iterator it ;
	float time = -1 ;
	for (it=entries.rbegin(); it!=entries.rend() && time==-1 ; it++)
	{
		last = *it ;
		if (last->type=="D")
			time = R_T2 ;
		else if (last->end_time!=0 && last->end_time!=-1)
			time = last->end_time ;
	}
	return time ;
}


//------------------------------------------------------------------------------
//--------------------------------- SGML SPEAKER--------------------------------
//------------------------------------------------------------------------------


SGMLobjects::SGMLspeaker::SGMLspeaker(Glib::ustring p_id)
{
	id = p_id ;
}

SGMLobjects::SGMLspeaker::~SGMLspeaker()
{
	std::map<int, SGMLobjects::SGMLpath*>::iterator it ;
	for (it=paths.begin(); it!=paths.end(); it++) {
		if (it->second)
			delete(it->second) ;
	}
}

SGMLobjects::SGMLpath* SGMLobjects::SGMLspeaker::addPath(Glib::ustring id, Glib::ustring word_cnt, Glib::ustring labels,
													Glib::ustring file, Glib::ustring chanel, Glib::ustring sequence,
													Glib::ustring t1, Glib::ustring t2, Glib::ustring word_aux)
{
	SGMLobjects::SGMLpath* path = new SGMLpath() ;

	path->id = id ;
	path->word_cnt = word_cnt ;
	path->labels = labels ;
	path->file = file ;
	path->chanel = chanel ;
	path->sequence = sequence ;
	path->R_T1 = my_atof(t1.c_str()) ;
	path->R_T2 = my_atof(t2.c_str()) ;
	path->word_aux = word_aux ;
	int seq = atoi(sequence.c_str()) ;
	paths[seq] = path ;

	return path ;
}

Glib::ustring SGMLobjects::SGMLspeaker::toString()
{
	Glib::ustring res ;
	res.append( "\n<< SPEAKER ") ;
	res.append( id + "\n") ;

	std::map<int, SGMLobjects::SGMLpath*>::iterator it ;
	for (it=paths.begin(); it!=paths.end(); it++) {
		res.append( "\n") ;
		res.append( (it->second)->toString() ) ;
	}

	res.append( "\nSPEAKER >>") ;
	return res ;
}


//------------------------------------------------------------------------------
//--------------------------------- SGML CATEGORY--------------------------------
//------------------------------------------------------------------------------

SGMLobjects::SGMLlabel::SGMLlabel(Glib::ustring p_id, Glib::ustring p_title, Glib::ustring p_desc)
{
	id = p_id ;
	title = p_title ;
	desc = p_desc ;
}


//------------------------------------------------------------------------------
//--------------------------------- SGML LABEl--------------------------------
//------------------------------------------------------------------------------

SGMLobjects::SGMLcategory::SGMLcategory(Glib::ustring p_id, Glib::ustring p_title, Glib::ustring p_desc)
{
	id = p_id ;
	title = p_title ;
	desc = p_desc ;
}

//------------------------------------------------------------------------------
//-------------------------------- SGML OBJECT---------------------------------
//------------------------------------------------------------------------------

SGMLobjects::SGMLobjects()
{
	paths.clear() ;
	last_sequence = -1 ;
}

SGMLobjects::~SGMLobjects()
{
	std::vector<SGMLobjects::SGMLspeaker*>::iterator its ;
	for (its=speakers.begin(); its!=speakers.end(); its++) {
		if (*its)
			delete(*its) ;
	}

	std::vector<SGMLobjects::SGMLlabel*>::iterator ite ;
	for (ite=labels.begin(); ite!=labels.end(); ite++) {
		if (*ite)
			delete(*ite) ;
	}

	std::vector<SGMLobjects::SGMLcategory*>::iterator itc ;
	for (itc=categories.begin(); itc!=categories.end(); itc++) {
		if (*itc)
			delete(*itc) ;
	}
}


SGMLobjects::SGMLspeaker* SGMLobjects::addSpeaker(Glib::ustring id)
{
	SGMLspeaker* speaker = new SGMLspeaker(id) ;
	speakers.push_back(speaker) ;
	return speaker ;
}

SGMLobjects::SGMLentry* SGMLobjects::addLabel(Glib::ustring id, Glib::ustring title, Glib::ustring desc)
{
	SGMLlabel* label = new SGMLlabel(id, title, desc) ;
}

SGMLobjects::SGMLcategory* SGMLobjects::addCategory(Glib::ustring id, Glib::ustring title, Glib::ustring desc)
{
	SGMLcategory* category = new SGMLcategory(id, title, desc) ;
}

void SGMLobjects::setData(Glib::ustring p_title, Glib::ustring p_ref_fname, Glib::ustring p_hyp_fname,
								Glib::ustring p_creation_date, Glib::ustring p_format,
								Glib::ustring p_frag_corr, Glib::ustring p_opt_del,
								Glib::ustring p_weight_ali, Glib::ustring p_weight_filename)
{
	title = p_title ;
	ref_fname = p_ref_fname ;
	hyp_fname = p_hyp_fname ;
	creation_date = p_creation_date  ;
	format = p_format ;
	frag_corr = p_frag_corr ;
	opt_del = p_opt_del ;
	weight_ali = p_weight_ali ;
	weight_filename = p_weight_filename ;
}

void SGMLobjects::addPath(SGMLobjects::SGMLpath* path)
{
	int seq = atoi(path->sequence.c_str()) ;
	if (seq > last_sequence)
		last_sequence = seq ;
	paths[seq] = path ;
}

SGMLobjects::SGMLpath* SGMLobjects::getPathN(int n)
{
	SGMLobjects::SGMLpath* res = NULL ;
	std::map<int,SGMLobjects::SGMLpath*>::iterator it = paths.find(n) ;
	if (it!=paths.end())
		res = it->second ;

	return res ;
}

void SGMLobjects::print()
{
	Log::err() << "\n----------------------------" << std::endl ;
	Glib::ustring res ;
	res.append( "title= " + title + "\n") ;
	res.append( "ref_fname= " + ref_fname + "\n") ;
	res.append( "hyp_fname= " + hyp_fname + "\n") ;
	res.append( "creation_date= " + creation_date + "\n") ;
	res.append( "format= " + format + "\n") ;
	res.append( "frag_corr= " + frag_corr + "\n") ;
	res.append( "opt_del= " + opt_del + "\n") ;
	res.append( "weight_ali= " + weight_ali + "\n") ;
	res.append( "weight_filename= " + weight_filename + "\n") ;
	res.append( "**last_seq= " + number_to_string(last_sequence) + "\n") ;

	Log::err() << res << std::endl ;

	std::vector<SGMLobjects::SGMLspeaker*>::iterator it ;
	for (it=speakers.begin(); it!=speakers.end(); it++)
		Log::err() << (*it)->toString() << std::endl ;
	Log::err() << "----------------------------\n" << std::endl ;
}


