/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "HighlightGeneral.h"
#include "Explorer_utils.h"
#include "Common/util/Utils.h"
#include "Explorer_dialog.h"
#include "Common/icons/Icons.h"
#include "Common/globals.h"
#include "Common/InputLanguageHandler.h"

#include <gtk/gtkimcontext.h>

#define RESULTSET_HIGHLIGHT_TAG_NAME 		"ResultsetHighlightTag"
#define RESULTSET_HIGHLIGHT_TAG_COLOR_BG 	"#98F6FE"
#define RESULTSET_HIGHLIGHT_TAG_COLOR_FG 	"#000000"

namespace tag {

HighlightGeneral::HighlightGeneral()
{
}

HighlightGeneral::~HighlightGeneral()
{
}

void HighlightGeneral::init(AnnotationEditor* editor)
{
	lock_cursor = true ;
	active = true ;
	count = 0 ;
	bak  = false ;
	found = false ;
	_search = "" ;
	hasSelection = false ;
	whole_word_mode = true ;
	external_editor = editor ;

	if (editor->getHighlightResultset() && editor->getHighlightResultset()->isLoaded())
		matches = editor->getHighlightResultset()->getMatchingTerms() ;
}

void HighlightGeneral::highlight()
{
	if (!external_editor)
		return ;

	commandLineOffsetSegid = external_editor->getCommandLineOffsetSegid() ;

	std::vector<AnnotationView*>::const_iterator it ;
	for (it=external_editor->getViews().begin(); it!=external_editor->getViews().end(); it++)
		highlightAllTerms((*it)->getBuffer()) ;
}

void HighlightGeneral::highlightAllTerms(Glib::RefPtr<AnnotationBuffer> buffer)
{
	prepareTag(buffer) ;

	std::vector<Glib::ustring>::iterator it ;
	for (it=matches.begin(); it!=matches.end(); it++)
	{
		if ( !(*it).empty() )
		{
			_search = *it ;
			highlightAll(buffer) ;
		}
	}
}

void HighlightGeneral::highlightAll(Glib::RefPtr<AnnotationBuffer> buffer)
{
	lock_cursor = true ;

	bak = false ;
	reset_iterators(buffer) ;

	on_search_fw(buffer) ;

	//> found will be updated at false when search_fw
	// will reach end cursor
	while(found)
	{
		on_search_fw(buffer) ;
		//> Actualize current at end of last replacement
		current=end_occur ;
		//> Go ahead
		on_search_fw(buffer) ;
	}
	lock_cursor = false ;
}

void HighlightGeneral::reset_iterators(Glib::RefPtr<AnnotationBuffer> buffer)
{
	start_occur = buffer->begin() ;
	end_occur = buffer->begin() ;
	start_search = buffer->begin() ;
	end_search = buffer->end() ;
	current = start_search ;
}


//******************************************************************************
//									SEARCH
//******************************************************************************

void HighlightGeneral::on_search_fw(Glib::RefPtr<AnnotationBuffer> buffer)
{
	bool whole_word_allowed = true ;
	lock_cursor = true ;

	found = forward_search_insensitive_case(_search, start_occur, end_occur) ;

	//> if found an occurence
	if (found && start_occur.compare(end_search) < 0)
	{
		 current = end_occur ;
		if (start_occur.editable())
		{
			 bool whole_word_allowed = true ;
			 //> if we search only whole word, check it
			 if (whole_word_mode)
				whole_word_allowed = is_whole_word() ;
			 //> if all tags are editable, we can select
			 if ( whole_word_allowed )
			 {
					select_occ(buffer) ;
					count++ ;
			 }
			 //> else go on for next search
			 else
				on_search_fw(buffer) ;
      	}
      	else
      		on_search_fw(buffer) ;
 	}
 	else
 		found = false ;

	lock_cursor = false ;
}

bool HighlightGeneral::forward_search_insensitive_case(Glib::ustring& searched, Gtk::TextBuffer::iterator& match_start, Gtk::TextBuffer::iterator& match_end)
{
	bool foundWord = false;
	bool exit;
	Gtk::TextBuffer::iterator tmp_itext ;
	Glib::ustring::iterator it;

	match_start = current;

	// End text case
	if ( match_start.is_end() )
		match_start.set_line(0) ;

	// While not at text end and not match found, parse forward from match_start iterator
	while ( (! foundWord ) && (! match_start.is_end()) )
	{
		tmp_itext = match_start ;
		it = searched.begin() ;
		exit = false;

		// parse the searched word from match_start iterator
		while (( ! exit ) && ( it != searched.end() ))
		{
			// Not same character, don't need to keep on
			if ( tolower(tmp_itext.get_char()) != tolower(*it) )
				exit = true;
			// Ok, let's continue
			else {
				++it;
				tmp_itext.forward_char();
			}
		}

		if ( ! exit )
		{
			foundWord = true;
			match_end = tmp_itext;
		}
		else
			match_start.forward_char();
	}

	current = match_start ;

	return foundWord;
}

bool HighlightGeneral::is_whole_word()
{
	bool res, found_first, found_last ;
	found_first = start_occur.starts_word() ;
	found_last = end_occur.ends_word() ;
	if (found_first && found_last)
		res = true ;
	else
		res=false;
	return res ;
}

//******************************************************************************
//								 SELECTION
//******************************************************************************

void HighlightGeneral::select_occ(Glib::RefPtr<AnnotationBuffer> buffer)
{
	buffer->apply_tag(dialogTag_occ, start_occur, end_occur) ;
//	buffer->setCursor(start_occur, true) ;
//	checkScroll(start_occur, buffer) ;
}

//void HighlightGeneral::checkScroll(Gtk::TextIter iter, Glib::RefPtr<AnnotationBuffer> buffer)
//{
//	// -- No command line offset element found: nothing to check
//	if (commandLineOffsetSegid.empty())
//		return ;
//
//	// -- Determine current element
//	string segment = buffer->getPreviousAnchorId(iter, external_editor->getDataModel().mainstreamBaseType("transcription_graph"), true) ;
//	if (segment.empty())
//		return ;
//
//	// -- Does it match with the command line offset ?
//	if (segment == commandLineOffsetSegid)
//	{
//		// do it only for the first
//		commandLineOffsetSegid = "" ;
//		// scroll to the matching term
//		external_editor->getActiveView()->scroll_to(iter, 0.4) ;
//	}
//}

void HighlightGeneral::prepareTag(Glib::RefPtr<AnnotationBuffer> buffer)
{
  	dialogTag_occ = buffer->get_tag_table()->lookup(RESULTSET_HIGHLIGHT_TAG_NAME) ;
	if (!dialogTag_occ)
	{
		dialogTag_occ = buffer->create_tag(RESULTSET_HIGHLIGHT_TAG_NAME);
		dialogTag_occ->property_foreground().set_value(RESULTSET_HIGHLIGHT_TAG_COLOR_FG);
		dialogTag_occ->property_background().set_value(RESULTSET_HIGHLIGHT_TAG_COLOR_BG);
  	}
}

} //namespace



