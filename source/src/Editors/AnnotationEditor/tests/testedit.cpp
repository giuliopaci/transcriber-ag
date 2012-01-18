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
 * MERCHANTABILITY or FITNESS FOR A PARTIULAR PURPOSE.  See the GNU
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
#include "DialogFileProperties.h"
#include "Common/FileInfo.h"
#include "Languages.h"
extern "C" {
#include "sig.h"
}

using namespace std;
using namespace tag;

#include <ag/AGException.h>
#include <ag/agfio.h>

#define THREADS_ENTER if ( m_threads ) gdk_threads_enter();
#define THREADS_LEAVE if ( m_threads ) { gdk_flush(); gdk_threads_leave(); }


#ifdef OLD
map<string, string> config_options;
#else
#include "Common/Parameters.h"

Parameters config_options;
#endif


class ExampleWindow : public Gtk::Window
{
public:
  ExampleWindow();
	virtual ~ExampleWindow() {}
	virtual void on_menu_file_open();
	virtual void on_menu_file_properties();
	virtual void on_menu_file_quit();
	virtual void on_close_file(AnnotationEditor* w);
	virtual void on_file_modified(bool b, AnnotationEditor* w);


	bool add_editor(string path, string lang="fre");
	virtual void add_edit_menu();
	virtual void add_signal_menu(int l, AnnotationEditor* w );
	void updateUIAnnot(AnnotationEditor* tw);

  AnnotationEditor* editor() { return tw; }

protected:
 //Child widgets:
  vector<AnnotationEditor*> v_tw;
  AnnotationEditor* tw;
  Gtk::VBox m_Box;
  Gtk::HBox m_HBox;
  Gtk::Statusbar m_statusBar;
   Glib::RefPtr<Gtk::UIManager> m_refUIManager;
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
	bool _play;
bool _signal_menu_added;
	guint m_uiAnnotId;
	bool m_threads;


};

ExampleWindow::ExampleWindow()
  : Gtk::Window(Gtk::WINDOW_TOPLEVEL),
	_play(false), _signal_menu_added(false), m_uiAnnotId(0)
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
  m_refActionGroup->add( Gtk::Action::create("file_properties", Gtk::Stock::PROPERTIES),
    sigc::mem_fun(*this, &ExampleWindow::on_menu_file_properties) );
  m_refActionGroup->add( Gtk::Action::create("file_quit", Gtk::Stock::QUIT),
    sigc::mem_fun(*this, &ExampleWindow::on_menu_file_quit) );


	m_refUIManager = Gtk::UIManager::create();
	m_refUIManager->insert_action_group(m_refActionGroup);



  //Layout the actions in a menubar and toolbar:

  Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='FileMenu'>"
        "        <menuitem action='file_open'/>"
        "        <menuitem action='file_properties'/>"
        "        <menuitem action='file_new'/>"
        "      <separator/>"
		"      <menuitem action='file_quit'/>"
        "      <separator/>"
        "    </menu>"
        "  </menubar>"
        "  <toolbar  name='ToolBar'>"
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

 // m_Box.pack_start(m_HBox, Gtk::PACK_EXPAND_WIDGET);
  m_Box.pack_start(m_HBox, true, true);
  m_Box.pack_end(m_statusBar,Gtk::PACK_SHRINK);



  show_all();
}

void ExampleWindow::add_edit_menu()
{
  Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='FileMenu'>"
        "      <separator/>"
        "      <menuitem action='file_save'/>"
        "      <menuitem action='file_saveas'/>"
        "      <menuitem action='file_close'/>"
        "      <separator/>"
        "      <menuitem action='file_refresh'/>"
        "    </menu>"
        "    <menu action='EditMenu'>"
        "      <menuitem action='edit_undo'/>"
        "      <menuitem action='edit_redo'/>"
        "      <separator/>"
		"      <menuitem action='edit_cut'/>"
         "      <menuitem action='edit_copy'/>"
         "      <menuitem action='edit_paste'/>"
       "    </menu>"
        "  </menubar>"
        "  <toolbar  name='ToolBar'>"
        "    <toolitem action='edit_undo'/>"
        "    <toolitem action='edit_redo'/>"
				"      <toolitem action='edit_cut'/>"
         "      <toolitem action='edit_copy'/>"
         "      <toolitem action='edit_paste'/>"
        "  </toolbar>"
        "</ui>";

cout << " ADDING EDIT MENU " << endl;
	m_refActionGroup->add( Gtk::Action::create("EditMenu", _("Edit")) );

	m_refUIManager->insert_action_group(tw->getActionGroup("edit"));
 	m_refUIManager->insert_action_group(tw->getActionGroup("file"));
	m_refUIManager->add_ui_from_string(ui_info) ;

cout << " ADDING ANNOTATE MENU " << endl;
	m_refActionGroup->add( Gtk::Action::create("AnnotateMenu", _("Annotate")) );
  	m_refUIManager->insert_action_group(tw->getActionGroup("annotate"));
	updateUIAnnot(tw);
}

void ExampleWindow::updateUIAnnot(AnnotationEditor* tw)
{
	if ( m_uiAnnotId != 0 )
		m_refUIManager->remove_ui(m_uiAnnotId);
	Glib::ustring ui_info2 = tw->getUIInfo("annotate");
 	m_uiAnnotId = m_refUIManager->add_ui_from_string(ui_info2) ;
}


void ExampleWindow::add_signal_menu(int l, AnnotationEditor* w )
{

	if ( _signal_menu_added ) return;

	cout << "\n\n  IN ADD SIGNAL MENU  FOR " << l << "\n\n" ;


  Glib::ustring ui_info2 = tw->getUIInfo("signal");

	m_refActionGroup->add( Gtk::Action::create("SignalMenu", _("Signal")));
	m_refUIManager->insert_action_group(tw->getActionGroup("signal"));
 	m_refUIManager->add_ui_from_string(ui_info2) ;
	_signal_menu_added =true;
}

void ExampleWindow::on_menu_file_quit()
{
 	Gtk::AccelMap::save("./AccelMap");
	 hide(); //Closes the main window to stop the Gtk::Main::run().
}


void ExampleWindow::on_close_file(AnnotationEditor* w)
{
  w->hide();
	delete tw;
	tw = NULL;
}


void ExampleWindow::on_file_modified(bool b, AnnotationEditor* w )
{
	if ( b ) set_title("(modified)");
		else  set_title("(NOT modified)");
}

void ExampleWindow::on_menu_file_open()
{
	if ( tw != NULL ) {
		tw->closeFile();
		return;
	}

	THREADS_ENTER
				// no select file name
			Gtk::FileChooserDialog file_chooser(_("Open annotation file"), Gtk::FILE_CHOOSER_ACTION_OPEN);

        file_chooser.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
        file_chooser.add_button (Gtk::Stock::OK, Gtk::RESPONSE_OK) ;
        file_chooser.set_select_multiple (false) ;


string file = "";
			if ( file_chooser.run() ==  Gtk::RESPONSE_OK )
			{
				string msg = "Folder= "+ file_chooser.get_current_folder()
					+ " file =" + file_chooser.get_filename();

				add_editor(file_chooser.get_filename());

			}
	THREADS_LEAVE
}

void ExampleWindow::on_menu_file_properties()
{
	if ( tw != NULL ) {
		Parameters parameters;

		/*DialogFileProperties* d = new DialogFileProperties(*this, tw->getDataModel(), true, parameters);
		d->run() ;
		d->hide();
		delete(d) ;
	*/
	}
}

bool ExampleWindow::add_editor(string path, string lang)
{
	cout << "\n\n   ADDDDDDD    EEDDIIITTAOOOORRRRRR\n\n";
	try {
	tw = new AnnotationEditor();

	v_tw.push_back(tw);

	tw->setOptions(config_options);
	tw->setOption("lang", lang);
	tw->setStatusBar(&m_statusBar);


//	 m_HBox.pack_start(*tw, Gtk::PACK_SHRINK);
	 m_HBox.pack_start(*tw, Gtk::PACK_EXPAND_WIDGET);
	add_edit_menu();


	tw->signalFileModified().connect(sigc::bind<AnnotationEditor*>(sigc::mem_fun (*this, &ExampleWindow::on_file_modified), tw));
	  tw->signalCanClose().connect(sigc::bind<AnnotationEditor*>(sigc::mem_fun (*this, &ExampleWindow::on_close_file), tw));

	tw->signalSignalViewAdded().connect(sigc::bind<AnnotationEditor*>(sigc::mem_fun (*this, &ExampleWindow::add_signal_menu), tw));
	//tw->signalUpdateUI().connect(sigc::bind<A

	  //Layout the actions in a menubar andnnotationEditor*>(sigc::mem_fun (*this, &ExampleWindow::updateUIAnnot), tw));

	tw->show();

	tw->loadFile(path);


	/*
  tw->signalCanClose().connect(sigc::mem_fun (*this, &ExampleWindow::on_close_file));

	m_refUIManager->insert_action_group(tw->getActionGroup("edit"));
	m_refUIManager->insert_action_group(tw->getActionGroup("file"));
*/

//	show_all() ;
	tw->setDefaultViewMode(false);
	cout << "AFTER SET DEFAULT VIEW MODE" << endl;

	} catch (const char* msg) {
		Log::err() << "add_editor ERROR : caught exception with msg= " << msg << endl;
		exit(1);
	}	catch(AGException& ex)		{
			MSGOUT << "Caught AGException " << ex.error() << endl;
			throw ex.error().c_str();
	} catch (...) {
		Log::err() << "add_editor ERROR : caught unknown exception with msg= " << endl;
		exit(1);
	}
	return false;
}



//#ifdef TEST_DATA_MODEL
#include "DataModel/DataModel.h"
//#endif

int main (int argc, char *argv[])
{
	time_t deb = time(0);
	cout << "STARTED AT " <<  deb << endl;

	  if ( argc < 2 ) {
		 Log::err() << "testedit <fic>  <fic2> " << endl;
		exit(1);
	  }


//	 sig_init(main_exit);

	string exepath=FileInfo(argv[0]).realpath();


	string cfgdir = FileInfo(exepath).dirname();
	cout << "tail = " << FileInfo(cfgdir).tail() << endl;

	FileInfo info(cfgdir);
	int lev = 3;

	if ( strcmp(info.tail(), ".libs") == 0 ) {
		cfgdir = FileInfo(cfgdir).dirname();
		++lev;
	}

	string rootdir = FileInfo(exepath).dirname(lev);
	cfgdir = rootdir + "/etc/TransAG";

	string homeAG = getenv("HOME");
	homeAG += "/.TransAG/";
	Parameters useropt;


		string cfgfic = homeAG+"transcriberAG.rc";

	if ( FileInfo(cfgfic).exists() ) {
		try {
			Log::err() << "Load config from " << cfgfic << endl;
		config_options.load(cfgfic);
		cfgfic = homeAG+"userAG.rc";
		Log::err() << "Load user config from " << cfgfic << endl;
		useropt.load(cfgfic);
		Parameters::mergeUserParameters(&useropt, &config_options, false);
		} catch ( const char* msg) {
			Log::err() << " ERREUR LOAD " << cfgfic << endl << msg << endl;
			exit(1);
		}
	}

//	DataModel::initEnviron(exepath);

	config_options.setParameterValue("General", "start,config", cfgdir);

	cout << " BIND TEXT DOMAIN = " << GETTEXT_PACKAGE << " " << FileInfo(rootdir).join("locales") << endl;
	bindtextdomain(GETTEXT_PACKAGE, FileInfo(rootdir).join("locales"));
	Languages::configure(FileInfo(rootdir).join("etc/TransAG/languagesAG.xml"), "Usual");


	g_thread_init(NULL);
	gdk_threads_init();

#ifdef TEST_DATA_MODEL
	DataModel* data = new DataModel("TransAG");
	data->loadFromFile(argv[1]);

	cout << " corpus = " << data->getCorpusName() << endl;

	cout << " offset = " << GetAnchorOffset("TransAG:1:6") << endl;
	cout << " offset = " << GetStartOffset("TransAG:1:E6") << endl;
/*	vector<SignalSegment> v;
	data->getSegments("turn", v);
	cout << "-------" << endl;
	cout << " seg3 = " << v[3].getStartOffset() << endl;
*/
//	return 1;
	delete data;

#endif

	Gtk::Main kit(argc, argv);

#ifdef TEST_DATA_MODEL
cout << "\n ====================================================\n";
cout << "\n ====================================================\n";
	data = new DataModel("TransAG");
	data->loadFromFile(argv[1]);

	cout << " corpus = " << data->getCorpusName() << endl;

	cout << " offset = " << GetAnchorOffset("TransAG:1:6") << endl;
	cout << " offset = " << GetStartOffset("TransAG:1:E6") << endl;

//	return 1;
	delete data;

cout << "\n ====================================================\n";
cout << "\n ====================================================\n";
#endif
	string iconsdir = FileInfo(rootdir).join("etc/TransAG/icons");

	/* add iconsdir to icons search path */
	Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
    theme->prepend_search_path(iconsdir);

	// load accel map if exists
	string accelmap = g_get_home_dir();
	accelmap += "/.TransAG/AccelMap";
	if ( FileInfo(accelmap).exists() )
		Gtk::AccelMap::load(accelmap.c_str());

    cout << "1" << endl;
    ExampleWindow window;

    window.set_title("AnnotationEditor demo");
    window.set_default_size (600, 700);

    cout << "Loading " << argv[1] << endl;

     window.show();


	if ( argc == 2 )
	    window.add_editor(argv[1]);
	if ( argc == 3)
	    window.add_editor(argv[2]);

     window.show_all();

    cout << "5" << endl;
			gdk_threads_enter();
 //   kit.run(window);
	    Gtk::Main::run(window);
			gdk_threads_leave();

    return 0;
}
