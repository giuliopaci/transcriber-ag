/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "SearchManager.h"

namespace tag {

SearchManager::SearchManager(Configuration* configuration, Gtk::Window* p_parent)
{
	config = configuration ;
	int is_mini = -1 ;

	if (config)
		is_mini = config->get_GUI_searchToolbarMode() ;

	if (is_mini==1)
		mini_mode = true ;
	else
		mini_mode = false ;

	parent = p_parent ;
	mini = new MiniSearch();
	mini->signalSearchModeChange().connect( sigc::mem_fun(*this, &SearchManager::change_search_mode) ) ;
	mini->setParent(parent) ;
	dialog = new SearchReplaceDialog(config);
	dialog->signalSearchModeChange().connect( sigc::mem_fun(*this, &SearchManager::change_search_mode) ) ;
	dialog->signal_response().connect( sigc::mem_fun(*this, &SearchManager::search_exit) ) ;
}

SearchManager::~SearchManager()
{
	if (dialog)
		delete dialog ;
    if (mini)
    	delete mini ;	
}

void SearchManager::switch_file(AnnotationEditor* editor)
{
	if (!mini_mode) {
		dialog->switch_file(editor) ;
	}
	//> switch search panel Pointer
	else {
		mini->switch_file(editor) ;
	}
}

void SearchManager::init(AnnotationEditor* editor)
{
	if (!mini_mode) {
		dialog->init(editor) ;
	}
	//> switch search panel Pointer
	else  {
		mini->init(editor) ;
	}
}

void SearchManager::myShow()
{
	if (!mini_mode) {
		dialog->loadGeoAndDisplay() ;
	}
	else {
		mini->show() ;
	}

}

void SearchManager::myHide()
{
	if (!mini_mode) {
		dialog->close() ;
		dialog->saveGeoAndHide() ;
	}
	else {
		mini->close() ;
		mini->hide() ;
	}
}

/*
 * mode=0: from dialog to mini mode
 * mode=1: from mini mode to dialog
 */
void SearchManager::change_search_mode(int mode)
{
	if (mode==0) {
		mini->init(dialog->get_editor()) ;
		mini_mode=true ;
		restoreData(0) ;
		dialog->close() ;
		if (config)
			config->set_GUI_searchToolbarMode(true, true) ;
	}
	else {
		dialog->init(mini->get_editor()) ;
		mini_mode=false ;
		restoreData(1) ;
		mini->close() ;
		if (config)
			config->set_GUI_searchToolbarMode(false, true) ;
	}
	myShow() ;
}

/*
 * mode=0: from dialog to mini mode
 * mode=1: from mini mode to dialog
 */
void SearchManager::restoreData(int mode)
{
	if (mode==0) {
		mini->set_findList(dialog->get_findList()) ;
		mini->set_current_search(dialog->get_current_search()) ;
		int searchMode ;
		bool caseSensitive, wholeWord ;
		dialog->getSearchOptions(searchMode, caseSensitive, wholeWord) ;
		mini->setSearchOptions(searchMode, caseSensitive, wholeWord) ;
	}
	else {
		dialog->set_findList(mini->get_findList()) ;
		dialog->set_current_search(mini->get_current_search()) ;
		int searchMode ;
		bool caseSensitive, wholeWord ;
		mini->getSearchOptions(searchMode, caseSensitive, wholeWord) ;
		dialog->setSearchOptions(searchMode, caseSensitive, wholeWord) ;
	}
}

void SearchManager::search_exit(int rep)
{
	if (rep==-4)
		dialog->close() ;
}

bool SearchManager::myHasFocus()
{
	if (mini_mode)
		return mini->myHasFocus() ;
	else
		return false ;
}

void SearchManager::check_selection()
{
	if (mini_mode) {
		mini->check_initial_selection() ;
		mini->mySet_focus() ;
	}
	else {
		dialog->check_initial_selection() ;
		dialog->mySet_focus() ;
	}
}

} //namespace
