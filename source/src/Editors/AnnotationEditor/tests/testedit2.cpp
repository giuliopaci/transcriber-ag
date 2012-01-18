/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id: textwidget.cc,v 1.15 2003/05/21 05:17:18 murrayc Exp $ */

/* textwidget.cc
 *
 * Copyright (C) 2001-2002 The gtkmm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */



#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gtkmm/icontheme.h>
#include <gtkmm/iconinfo.h>
#include <libintl.h>
#include <gtkmm/accelmap.h>

#include "Common/globals.h"
#include "gtkmm.h"
#include "gtkmm/main.h"
#include "AnnotationEditor.h"
#include "Common/FileInfo.h"

using namespace std;
using namespace tag;

class ExampleWindow : public Gtk::Window
{
public:
  ExampleWindow();
  virtual ~ExampleWindow() {}
  virtual void on_menu_file_open();
  virtual void on_menu_file_quit();
	 virtual void on_close_file();

	
	void add_editor(string path);
	 virtual void add_signal_menu();

  AnnotationEditor* editor() { return tw; }

protected:
 //Child widgets:
  AnnotationEditor* tw;
  Gtk::VBox m_Box;
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
	bool _play;
 
};

ExampleWindow::ExampleWindow() 
  : Gtk::Window(Gtk::WINDOW_TOPLEVEL), 
	_play(false)
{
  set_title("main menu example");
  set_default_size(200, 200);


  add(m_Box); //We can put a MenuBar at the top of the box and other stuff below it.
	
  //Create actions for menus and toolbars:
  m_refActionGroup = Gtk::ActionGroup::create();

/*
  m_importGroup["file"] = Gtk::ActionGroup::create();
  m_importGroup["edit"] = Gtk::ActionGroup::create();
  m_importGroup["play"] = Gtk::ActionGroup::create();
*/
  
  //File|New sub menu:
   m_refActionGroup->add( Gtk::Action::create("FileMenu", _("File")) );
	// , "_Open file", "Open annotation file"
	m_refActionGroup->add( Gtk::Action::create("file_open", Gtk::Stock::OPEN),
    sigc::mem_fun(*this, &ExampleWindow::on_menu_file_open) );

// , "_New Annotation file", "Create a new annotation file "
  m_refActionGroup->add( Gtk::Action::create("file_new", Gtk::Stock::NEW),
    sigc::mem_fun(*this, &ExampleWindow::on_menu_file_open) );
  m_refActionGroup->add( Gtk::Action::create("file_quit", Gtk::Stock::QUIT),
    sigc::mem_fun(*this, &ExampleWindow::on_menu_file_quit) );

  //Edit menu:


	m_refActionGroup->add( Gtk::Action::create("EditMenu", _("Edit")) );



	m_refUIManager = Gtk::UIManager::create();
	m_refUIManager->insert_action_group(m_refActionGroup);
	
		tw = new AnnotationEditor();
	 
	  tw->signalCanClose().connect(sigc::mem_fun (*this, &ExampleWindow::on_close_file));

	tw->signalSignalViewAdded().connect(sigc::mem_fun (*this, &ExampleWindow::add_signal_menu));
	m_refUIManager->insert_action_group(tw->getActionGroup("edit"));
 	m_refUIManager->insert_action_group(tw->getActionGroup("file"));
	

  //Layout the actions in a menubar and toolbar:
  
  Glib::ustring ui_info = 
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='FileMenu'>"
        "        <menuitem action='file_open'/>"
        "        <menuitem action='file_new'/>"
        "      <separator/>"
        "      <menuitem action='file_save'/>"
        "      <menuitem action='file_saveas'/>"
        "      <menuitem action='file_close'/>"
        "      <separator/>"
		"      <menuitem action='file_quit'/>"
        "    </menu>"
        "    <menu action='EditMenu'>"
        "      <menuitem action='edit_undo'/>"
        "      <menuitem action='edit_redo'/>"
        "    </menu>"
        "  </menubar>"
        "  <toolbar  name='ToolBar'>"
        "    <toolitem action='edit_undo'/>"
        "    <toolitem action='edit_redo'/>"
        "  </toolbar>"
        "</ui>";
  
  cout << "BEFORE add_ui_from_string " << endl;

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {      
    m_refUIManager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    Log::err() << "building menus failed: " <<  ex.what();
  }
  #else
   Log::err() << "no  GLIBMM_EXCEPTIONS_ENABLED : " << endl;
 
   std::auto_ptr<Glib::Error> ex;
  m_refUIManager->add_ui_from_string(ui_info);
  if(ex.get())
  { 
    Log::err() << "building menus failed: " <<  ex->what();
  }
  #endif //GLIBMM_EXCEPTIONS_ENABLED
  cout << "AFTER add_ui_from_string " << endl;

	add_accel_group(m_refUIManager->get_accel_group());

   cout << "AFTER add_accel_group " << endl;

  //Get the menubar and toolbar widgets, and add them to a container widget:
  Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
  if(pMenubar)
    m_Box.pack_start(*pMenubar, Gtk::PACK_SHRINK);

  Gtk::Widget* pToolbar = m_refUIManager->get_widget("/ToolBar") ;
  if(pToolbar)
    m_Box.pack_start(*pToolbar, Gtk::PACK_SHRINK);
  
  m_Box.pack_start(*tw, Gtk::PACK_EXPAND_WIDGET);
 
  //show_all_children();
}

void ExampleWindow::add_signal_menu()
{
  Glib::ustring ui_info2 = 
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='SignalMenu'>"
        "      <menuitem action='signal_play'/>"
        "      <menuitem action='signal_pause'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";
	m_refActionGroup->add( Gtk::Action::create("SignalMenu", _("Signal")));	
	m_refUIManager->insert_action_group(tw->getActionGroup("signal"));
 	m_refUIManager->add_ui_from_string(ui_info2) ;
}

void ExampleWindow::on_menu_file_quit()
{
 	Gtk::AccelMap::save("./AccelMap");
	 hide(); //Closes the main window to stop the Gtk::Main::run().
}


void ExampleWindow::on_close_file()
{
  tw->hide();
	delete tw;
	tw = NULL;
}


void ExampleWindow::on_menu_file_open()
{
	if ( tw != NULL ) {
		tw->closeFile();
		return;
	}
				// no select file name
			Gtk::FileChooserDialog file_chooser(_("Open annotation file"), Gtk::FILE_CHOOSER_ACTION_OPEN);
	
        file_chooser.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
        file_chooser.add_button (Gtk::Stock::OK, Gtk::RESPONSE_OK) ;
        file_chooser.set_select_multiple (false) ;

	
			if ( file_chooser.run() ==  Gtk::RESPONSE_OK )
			{ 
				string msg = "Folder= "+ file_chooser.get_current_folder() 
					+ " file =" + file_chooser.get_filename();

				add_editor(file_chooser.get_filename());

			}				
}

void ExampleWindow::add_editor(string path)
{
//	tw = new AnnotationEditor();
	tw->loadFile(path);
	 
	/* 
  tw->signalCanClose().connect(sigc::mem_fun (*this, &ExampleWindow::on_close_file));

	m_refUIManager->insert_action_group(tw->getActionGroup("edit"));
	m_refUIManager->insert_action_group(tw->getActionGroup("file"));
*/
}
//#ifdef TEST_DATA_MODEL
#include "DataModel/DataModel.h"
//#endif

int main (int argc, char *argv[])
{

	if ( argc < 2  ) {
	cout << "USAGE: testedit <trsfile>" << endl;
	return 0;
	}

	string exepath=FileInfo(argv[0]).realpath();
	string rootdir = FileInfo(exepath).dirname(4);
	
	bindtextdomain(GETTEXT_PACKAGE, FileInfo(rootdir).join("locales"));

	g_thread_init(NULL);
	gdk_threads_init();
	
	
	Gtk::Main kit(argc, argv);
	
	string iconsdir = FileInfo(rootdir).join("icons");
cout << "ICONSDIR = " << iconsdir << endl;
	
	/* add iconsdir to icons search path */
	Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
    theme->prepend_search_path(iconsdir); 
		Gtk::AccelMap::load("./AccelMap");
 
	
    cout << "1" << endl;
    ExampleWindow window;
	


    cout << "2" << endl;
    
    window.set_title("AnnotationEditor demo");
    window.set_default_size (600, 700);
    
    cout << "Loading " << argv[1] << endl;
//#ifdef TEST_DATA_MODEL
	DataModel* data = new DataModel("TransAG");
	data->loadFromFile(argv[1]);

	cout << " corpus = " << data->getCorpusName() << endl;
	
	cout << " offset = " << GetAnchorOffset("TransAG:1:6") << endl;
	cout << " offset = " << GetStartOffset("TransAG:1:E6") << endl;
	vector<SignalSegment> v;
	data->getSegments("turn", v);

	cout << "-------" << endl;
	cout << " seg3 = " << v[3].getStartOffset() << endl;
//	return 1;	
	delete data;
	
//#endif
	
	if ( argc == 2 ) 
	    window.add_editor(argv[1]);

    cout << "4" << endl;
    window.show_all();
    cout << "5" << endl;
			gdk_threads_enter();
	

    kit.run(window);
			gdk_threads_leave();

    return 0;
}
