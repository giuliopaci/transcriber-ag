/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "TSearchManager.h"

namespace tag {

TSearchManager::TSearchManager(Configuration* configuration)
{
	mini_mode = false ;
	config = configuration ;
	dialog = new TSearchDialog(config) ;
	dialog->signal_response().connect( sigc::mem_fun(*this, &TSearchManager::search_exit) ) ;
}

TSearchManager::~TSearchManager()
{
	if (dialog)
		delete(dialog) ;
}

void TSearchManager::switch_file(AnnotationEditor* editor)
{
//	if (!mini_mode) {
		dialog->switch_file(editor) ;
//	}
	//> switch search panel Pointer
//	else {
//		mini.switch_file(editor) ;
//	}
}

void TSearchManager::init(AnnotationEditor* editor)
{
//	if (!mini_mode) {
		dialog->init(editor) ;

//	}
	//> switch search panel Pointer
//	else  {
//		mini.init(editor) ;
//	}
}

void TSearchManager::myShow()
{
//	if (!mini_mode) {
		dialog->loadGeoAndDisplay() ;
//	}
//	else {
//		mini.show() ;
//	}
}

void TSearchManager::myHide()
{
//	if (!mini_mode) {
		dialog->close() ;
		dialog->saveGeoAndHide() ;
//	}
//	else {
//		mini.close() ;
//		mini.hide() ;
//	}
}

//TODO use a search manager to hide changement
void TSearchManager::change_search_mode(int mode)
{
//	if (mode==0) {
//		mini.init(dialog->get_editor()) ;
//		mini.set_findList(dialog->get_findList()) ;
//		mini_mode=true ;
//		mini.set_current_search(dialog->get_current_search()) ;
//		dialog->close() ;
//	}
//	else{
//		dialog->init(mini.get_editor()) ;
//		dialog->set_findList(mini.get_findList()) ;
//		mini_mode=false ;
//		dialog->set_current_search(mini.get_current_search()) ;
//		mini.close() ;
//	}
//	myShow() ;
}

void TSearchManager::search_exit(int rep)
{
	if (rep==-4)
		dialog->close() ;
}

bool TSearchManager::myHasFocus()
{
//	if (mini_mode)
//		return mini.myHasFocus() ;
//	else
		return false ;
}

void TSearchManager::check_selection()
{
//	if (mini_mode) {
//		mini.check_initial_selection() ;
//		mini.mySet_focus() ;
//	}
//	else {
		dialog->check_initial_selection() ;
		dialog->mySet_focus() ;
//	}
}

} //namespace
