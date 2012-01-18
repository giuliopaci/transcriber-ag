/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Explorer_menu.h"
#include "Explorer_utils.h"
#include "Explorer_fileHelper.h"
#include <iostream>

namespace tag {


#define RECENT_FILE_MAX 10

//***************************************************************************************
//***************************************************************************** CONSTRUCT
//***************************************************************************************

Explorer_menu::Explorer_menu()
{
	active = false ;
	actionGroup = Gtk::ActionGroup::create() ;
	mainMenu = Gtk::UIManager::create();
}

Explorer_menu::~Explorer_menu()
{
}


//***************************************************************************************
//******************************************************************************** GETTER
//***************************************************************************************

Glib::RefPtr<Gtk::UIManager> Explorer_menu::get_mainMenu()
{
	return mainMenu ;
}


Glib::RefPtr<Gtk::ActionGroup> Explorer_menu::get_actionGroup()
{
	return actionGroup ;
}


Gtk::Widget* Explorer_menu::get_widget_mainMenu(Glib::ustring s_menu)
{
	return mainMenu->get_widget(s_menu) ;
}


//***************************************************************************************
//******************************************************************************** SETTER
//***************************************************************************************

void Explorer_menu::set_menu_with_action(Gtk::Window* window)
{
	mainMenu->insert_action_group(actionGroup);
	window->add_accel_group(mainMenu->get_accel_group());
}

void Explorer_menu::insert_actionGroup(Glib::RefPtr<Gtk::ActionGroup> actionG, Gtk::Window* window, bool check)
{
	if (!actionG)
		return ;

	bool ok = false ;

	//> insert directly
	if (!check)
		ok = true ;
	//> check if exist before inserting
	else {
		std::vector< Glib::RefPtr<Gtk::ActionGroup> > groups = mainMenu->get_action_groups() ;
		std::vector< Glib::RefPtr<Gtk::ActionGroup> >::iterator it = groups.begin() ;
		bool found = false ;
		while(!found && it!=groups.end()) {
			if ( *(it)==actionG )
				found = true ;
			it++ ;
		}
		if (!found)
			ok = true ;
	}
	//> proceed if allowed
	if (ok) {
		mainMenu->insert_action_group(actionG);
		if (window!=NULL)
			window->add_accel_group(mainMenu->get_accel_group()) ;
	}
}

void Explorer_menu::remove_actionGroup(Glib::RefPtr<Gtk::ActionGroup> actionG, bool check)
{
	if (!actionG)
		return ;

	//> remove directly
	if (!check)
		mainMenu->remove_action_group(actionG) ;
	//> remove after checking if it's in group
	else {
		std::vector< Glib::RefPtr<Gtk::ActionGroup> > groups = mainMenu->get_action_groups() ;
		std::vector< Glib::RefPtr<Gtk::ActionGroup> >::iterator it = groups.begin() ;
		bool found = false ;
		while(!found && it!=groups.end()) {
			if ( *(it)==actionG ) {
				found = true ;
				mainMenu->remove_action_group(actionG) ;
			}
			it++ ;
		}
	}
}

/** return the ui_id of the ui added **/
Gtk::UIManager::ui_merge_id Explorer_menu::add_ui(Glib::ustring ui_info, Glib::ustring agname)
{
	Gtk::UIManager::ui_merge_id res = -9 ;
	try {
		res = mainMenu->add_ui_from_string(ui_info) ;
	}
 	catch(const Glib::Error& ex) {
    	Log::err() << "Explorer_menu:> layout_action:> building menus failed: " << ex.what() << std::endl ;
  	}

  	if (agname=="general")
  		general_ui_id = res ;
  	else if (agname=="default")
  		default_ui_id = res ;
  	else if (agname=="signal")
  		signal_ui_id = res ;
  	else if (agname=="annotate")
  		annotate_ui_id = res ;
  	else if (agname=="display")
  		display_ui_id = res ;
  	else if (agname=="video_signal")
  		video_ui_id = res ;

  	return res ;
}

/**
 * Stock ui id of current ui_string
 * @param ui: ui merge id of ui string currently used
 * @param agname: ui merge id of signal / annotate
 */
void Explorer_menu::remove_ui(Glib::ustring agname)
{
	if (agname=="signal"){
		if (signal_ui_id>0) {
			mainMenu->remove_ui(signal_ui_id) ;
			signal_ui_id = -9 ;
		}
	}
	else if (agname=="general"){
		mainMenu->remove_ui(general_ui_id) ;
 	}
 	else if (agname=="default"){
		mainMenu->remove_ui(default_ui_id) ;
 	}
	else if (agname=="annotate"){
		mainMenu->remove_ui(annotate_ui_id) ;
	}
	else if (agname=="display"){
		mainMenu->remove_ui(display_ui_id) ;
	}
  	else if (agname=="video_signal") {
		if (video_ui_id>0) {
			mainMenu->remove_ui(video_ui_id) ;
			video_ui_id = -9 ;
		}
  	}
	else {
		TRACE << "Explorer_menu::remove_ui:> BAD option removing ui" << agname << std::endl ;
	}
}




//***************************************************************************************
//******************************************************************************** METHOD
//***************************************************************************************

/*
 * Take a file as parameter, and add it in recent files.
 * Return true if the file has been added to recent file list
 */
bool Explorer_menu::add_recent_file(Glib::ustring path, Gtk::UIManager::ui_merge_id id, Glib::ustring ui_path,  Glib::RefPtr<Gtk::Action> action, const Gtk::Action::SlotActivate& slot)
{
	double MAX = RECENT_FILE_MAX ;
	guint i=0 ;
	bool res = false ;
	bool exist = false ;

	//> catch if the files is already in
	while (i<recent_actions.size() && !exist) {
		//> if it exists, break
		if (recent_actions[i]->get_name()==path) {
			exist=true ;
		}
		i++ ;
	}
	//> if it doesn't exist, we can add it
	if (exist==false) {
		//add in list
		recent_files.insert(recent_files.begin(), path) ;
		recent_actions.insert(recent_actions.begin(), action) ;
		recent_uis.insert(recent_uis.begin(), id) ;

		//add in UI string and in action group
		mainMenu->add_ui(id, ui_path , path, path, Gtk::UI_MANAGER_AUTO, false) ;
		actionGroup->add(action, slot);

		// if max number files reached, suppress the last (first in)
		if (recent_actions.size()==MAX+1) {
			//> removes from action group
			actionGroup->remove( recent_actions[recent_actions.size()-1] ) ;
			//> removes from menu
			mainMenu->remove_ui( recent_uis[recent_uis.size()-1] ) ;

			//> actualize lists
			recent_actions.pop_back();
			recent_uis.pop_back() ;
			recent_files.pop_back() ;
		}
		res=true ;
	}
	return res ;
}

void Explorer_menu::save(Glib::ustring path)
{
	int res =Explorer_utils::write_lines(path, recent_files, "w");
	if (res<0)
		Log::err() << "Explorer_menu::close_all:> " << res << std::endl ;
}


void Explorer_menu::set_ui(Glib::ustring ui, Glib::ustring type)
{
	if (type=="default")
		default_ui_info = ui ;
	else if (type=="general")
		general_ui_info = ui ;
}

Glib::ustring Explorer_menu::get_ui(Glib::ustring type)
{
	if (type=="default")
		return default_ui_info ;
	else if (type=="general")
		return general_ui_info ;
	else
		return "" ;
}

void Explorer_menu::switch_ui(Glib::ustring mode)
{
	if (mode=="first") {
		remove_ui("default") ;
		add_ui(general_ui_info,"general") ;
	}
	else if (mode=="last") {
		remove_ui("general") ;
		remove_ui("signal") ;
		remove_ui("annotate") ;
		remove_ui("video_signal") ;
		add_ui(default_ui_info,"default") ;
	}
}


} //namespace


