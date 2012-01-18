/*
 * SearchGeneral.cpp
 *
 *  Created on: 1 avr. 2009
 *      Author: montilla
 */

#include "SearchGeneral.h"

namespace tag {

SearchGeneral::SearchGeneral()
{
	// TODO Auto-generated constructor stub
	iLang = NULL ;
	external_editor = NULL ;
}

SearchGeneral::~SearchGeneral()
{
	// TODO Auto-generated destructor stub
}

void SearchGeneral::removeSelectionTag()
{
	if (hasSelection) {
		get_external_buffer()->remove_tag(dialogTag_sel, start_search, end_search) ;
		hasSelection = false ;
	}
}


//******************************************************************************
//****************************** FOCUS CHANGEMENT ******************************
//******************************************************************************


void SearchGeneral::save_state(int mode)
{
	if (mode==0) {
		get_external_buffer()->create_mark("start_search",start_search,true);
		get_external_buffer()->create_mark("end_search",end_search,true);
		get_external_buffer()->create_mark("current",current,true);
		get_external_buffer()->create_mark("start_occur",start_occur,true);
		get_external_buffer()->create_mark("end_occur",end_occur,true);
	}
	else if (mode==1) {
		get_external_buffer()->create_mark("start_search",start_search,true);
		get_external_buffer()->create_mark("end_search",end_search,true);
	}
	else if (mode==2) {
		get_external_buffer()->create_mark("start_occur",start_occur,true);
		get_external_buffer()->create_mark("end_occur",end_occur,true);
	}
	else if (mode==3) {
		get_external_buffer()->create_mark("current",current,true);
	}
	else if (mode==4) {
		get_external_buffer()->create_mark("start_search",start_search,true);
		get_external_buffer()->create_mark("end_search",end_search,true);
		get_external_buffer()->create_mark("current",current,true);
	}
}

void SearchGeneral::load_state()
{
	mark_start_search = get_external_buffer()->get_mark("start_search") ;
	mark_end_search = get_external_buffer()->get_mark("end_search") ;
	mark_start_occur = get_external_buffer()->get_mark("start_occur") ;
	mark_end_occur = get_external_buffer()->get_mark("end_occur") ;
	mark_current = get_external_buffer()->get_mark("current") ;

	if (!mark_start_search) {
		return ;
	}
	else {
		start_search = mark_start_search->get_iter();
		end_search = mark_end_search->get_iter();
		start_occur = mark_start_occur->get_iter();
		end_occur = mark_end_occur->get_iter();
		current = mark_current->get_iter();
	}
}

void SearchGeneral::on_response(int rep)
{
	close() ;
}

}
