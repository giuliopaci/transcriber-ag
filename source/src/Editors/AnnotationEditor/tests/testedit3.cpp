/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
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

#include <sys/stat.h>

using namespace std;
using namespace tag;

int main(int argc, char** argv) {

	g_thread_init(NULL);
	gdk_threads_init();
	Gtk::Main kit(argc, argv);

	Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
	theme->prepend_search_path("/bali/croatie/DACTILO/DEVEL/Transcriber/etc/TransAG/icons");

	Gtk::Window* w = new Gtk::Window();

	AnnotationEditor* annot = new AnnotationEditor();

	Parameters params;
	params.load("/bali/croatie/DACTILO/DEVEL/Transcriber/src/AnnotationEditor/editor.rc");
	annot->setOptions(params);

	annot->loadFile(argv[1]);

	w->add(*annot);
	w->resize(1000, 100);
	w->show_all();

	gdk_threads_enter();
	Gtk::Main::run(*w);
	gdk_threads_leave();

	return 1;

}
