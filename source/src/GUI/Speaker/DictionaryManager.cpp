/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "DictionaryManager.h"

#include "GUI/Dialogs/Explorer_dialog.h"

#include "Common/FileInfo.h"
#include "Common/Dialogs.h"

#include "Editors/AnnotationEditor/AnnotationEditor.h"

namespace tag {

DictionaryManager::DictionaryManager(Gtk::Window* win, Configuration* config)
{
	global_dictionary = NULL ;
	drag_initialised = false ;

	window = win ;
	configuration = config ;

	setDragAndDropTarget(NULL) ;
}

DictionaryManager::~DictionaryManager()
{
	for (guint i=0; i<dicos.size(); i++) {
		if (dicos[i])
			delete(dicos[i]) ;
	}
}

SpeakerDico_dialog* DictionaryManager::new_dialog(AnnotationEditor* editor, int forcedEditability, bool modal, Gtk::Window* win)
{
	//-- Options
	bool editable = true ;
	if (forcedEditability==1)
		editable = true ;
	else if (forcedEditability==0)
		editable = false ;
	else if (editor->getOption("mode") == "BrowseMode")
		editable = false ;

	//-- Creation
	SpeakerDico_dialog* dialog = NULL ;
	dicos.insert(dicos.end(), new SpeakerDico_dialog::SpeakerDico_dialog(editable, modal, win) ) ;
	dialog = dicos[dicos.size()-1] ;

	//> -- Post options
	dialog->setDragAndDropTarget(dragdropTargetList) ;

	//-- Opening
	dialog->open_dictionary(editor) ;

	//-- DragNdrop signals
	dialog->get_view()->signal_drag_data_get().connect(sigc::bind<SpeakerDico_dialog*>(sigc::mem_fun(*this, &DictionaryManager::on_drag_data_get), dialog)) ;
	dialog->get_view()->signal_drag_drop().connect(sigc::bind<SpeakerDico_dialog*>(sigc::mem_fun(*this, &DictionaryManager::on_drag_drop), dialog)) ;
	dialog->signalDropOnData().connect(sigc::bind<SpeakerDico_dialog*>(sigc::mem_fun(*this, &DictionaryManager::on_replace_by_data_drop), dialog)) ;

	//-- Other signals
	dialog->signalClose().connect(sigc::bind<SpeakerDico_dialog*>(sigc::mem_fun(*this, &DictionaryManager::on_close), dialog)) ;
	dialog->signalRaiseDictionary().connect(sigc::mem_fun(*this, &DictionaryManager::onRaiseDictionary)) ;

	return dialog ;
}

SpeakerDico_dialog* DictionaryManager::new_dialog(Glib::ustring url, int forcedEditability, bool modal, Gtk::Window* win)
{
	//> -- Options

	// check file editability
	FileInfo info(url);
	bool editable = info.exists();
	if ( editable )
		editable = info.canWrite();

	// check if we can force editability
	if (editable)
	{
		if (forcedEditability==1)
			editable = true ;
		else if (forcedEditability==0)
			editable = false ;
	}

	//> -- Creation
	SpeakerDico_dialog* dialog = NULL ;
	dicos.insert(dicos.end(), new SpeakerDico_dialog::SpeakerDico_dialog(editable, modal, win) ) ;
	dialog = dicos[dicos.size()-1] ;
	global_dictionary = dialog ;

	//> -- Post options
	dialog->setDragAndDropTarget(dragdropTargetList) ;

	//> -- Opening
	dialog->open_dictionary(url) ;

	//> -- DragNDrop signals
	dialog->get_view()->signal_drag_data_get().connect(sigc::bind<SpeakerDico_dialog*>(sigc::mem_fun(*this, &DictionaryManager::on_drag_data_get), dialog)) ;
	dialog->get_view()->signal_drag_drop().connect(sigc::bind<SpeakerDico_dialog*>(sigc::mem_fun(*this, &DictionaryManager::on_drag_drop), dialog)) ;
	dialog->signalDropOnData().connect(sigc::bind<SpeakerDico_dialog*>(sigc::mem_fun(*this, &DictionaryManager::on_replace_by_data_drop), dialog)) ;

	//> -- Other signals
	dialog->signalClose().connect(sigc::bind<SpeakerDico_dialog*>(sigc::mem_fun(*this, &DictionaryManager::on_close), dialog)) ;
	dialog->signalRaiseDictionary().connect(sigc::mem_fun(*this, &DictionaryManager::onRaiseDictionary)) ;

	return dialog ;
}

//**************************************************************************************
//									   DISPLAY
//**************************************************************************************

void DictionaryManager::showGlobalDictionary()
{
	try
	{
		if ( ! is_opened_global() )
		{
			Glib::ustring path = configuration->get_global_dictionary() ;
			if (!Glib::file_test(path, Glib::FILE_TEST_EXISTS))
			{
				string msg = path + " :" ;
				msg +=  _("File not found");
				Explorer_dialog::msg_dialog_info(msg, window, true) ;
			}
			else
			{
				SpeakerDico_dialog * d = new_dialog(configuration->get_global_dictionary(), -1, false, window) ;
				d->loadGeoAndDisplay() ;
			}
		}
		else
			raiseDictionary("", "") ;
	}
	catch (const char* e)
	{
		Log::err() << "TranscriberAG --> (!) Error while displaying speaker dictionary" << e << std::endl ;
		Explorer_dialog::msg_dialog_error(_("Loading dictionary error"), window, true) ;
	}
}

void DictionaryManager::showLocalDictionary(AGEditor* edit, Glib::ustring id, bool modal)
{
	try
	{
		//TODO change this
//		AnnotationEditor* editor = (AnnotationEditor*)edit() ;
		if (edit==NULL)
			Explorer_dialog::msg_dialog_info("No opened file", window, true) ;
		else
		{
			Glib::ustring file_path = edit->getFileName() ;
			if ( !is_opened(file_path) )
			{
				SpeakerDico_dialog * d = new_dialog((AnnotationEditor*)edit, -1, modal, window) ;
				d->loadGeoAndDisplay() ;
				if (id!="" && id!=" ")
					d->set_cursor_to_speaker(id) ;
			}
			else
				raiseDictionary(file_path, id) ;
		}
	}
	catch (const char* e)
	{
		Log::err() << "TranscriberAG --> (!) Error while displaying speaker dictionary" << e << std::endl ;
		Explorer_dialog::msg_dialog_error(_("Loading dictionary error"), window, true) ;
	}
}

void DictionaryManager::onRaiseDictionary(Glib::ustring scope)
{
	if (scope.compare("local")==0)
		m_signalRaiseCurrentDictionary.emit() ;
	else
		showGlobalDictionary() ;
}

void DictionaryManager::raiseDictionary(Glib::ustring path, Glib::ustring id_to_select)
{
	if (path.compare("")==0 && global_dictionary) {
		//global_dictionary->get_window()->raise() ;
		global_dictionary->present() ;
	}
	else {
		SpeakerDico_dialog* d = get_dictionary(path) ;
		if (d) {
			//d->raise() ;
			d->present() ;
			if ( id_to_select.compare("")!=0 && id_to_select.compare(" ")!=0 )
				d->set_cursor_to_speaker(id_to_select) ;
		}
		else
			TRACE << "> error while raising dictionary window" << std::endl ;
	}
}

//**************************************************************************************
//									 DRAG'N DROP
//**************************************************************************************

void DictionaryManager::on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& context,
							   Gtk::SelectionData& selection_data,
							   guint info, guint time, SpeakerDico_dialog* dialog)
{
	std::vector<Gtk::TreePath> plist = dialog->get_view()->get_selection()->get_selected_rows() ;
	paths.clear() ;
	for(guint i = 0; i<plist.size(); i++){
		Gtk::TreeIter iter = dialog->get_refSortedModel()->get_iter(plist[i]) ;
		paths.insert(paths.end(), iter) ;
	}
	dialog_src = dialog ;
	drag_initialised = true ;
}

/*
 * Received when a row is dropped into dialog LIST VIEW
 */
bool DictionaryManager::on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time, SpeakerDico_dialog* dialog)
{
	ListModel_Columns c ;

	//> CHECK FOR ADDING MODE !!
	if (drag_initialised && dialog!=dialog_src && dialog->is_editable())
	{
		std::vector<Gtk::TreeIter>::iterator it = paths.begin() ;
		while (it!=paths.end())
		{
			//> Get the id of the row beeing imported
			Glib::ustring id = (**it)[c.a_id] ;

			//> Copy speaker and set its reference in the destination dictionary
			Speaker s = dialog_src->get_dictionary()->getSpeaker(id) ;
			Speaker* newSpeaker = new Speaker(s) ;
			dialog->set_current_speaker( newSpeaker ) ;

			//> Keep speaker_id information if importing from GLOBAL to LOCAL
			Glib::ustring global_src_id = "" ;
			if (dialog_src->is_global())
				global_src_id = id ;

			//> Add speaker
			dialog->addSpeaker(true, global_src_id) ;
			it++ ;
		}
		drag_initialised = false ;
	}
	return true ;
}

/*
 * Received when a row is dropped into dialog SPEAKER DATA FRAME
 */
void DictionaryManager::on_replace_by_data_drop(SpeakerDico_dialog* dialog)
{
	ListModel_Columns c ;

	if (drag_initialised && dialog!=dialog_src && paths.size()==1)
	{
		//> to be replaced
		Glib::ustring current_id = dialog->get_active_id() ;
		std::vector<Gtk::TreeIter>::iterator it = paths.begin() ;

		//> to replace
		Glib::ustring id = (**it)[c.a_id] ;
		Speaker s = dialog_src->get_dictionary()->getSpeaker(id) ;

		//> prepare
		Speaker* newSpeaker = new Speaker(s) ;
		dialog->set_current_speaker( newSpeaker ) ;

		//> When replacing from a GLOBAL dico, keep the global speaker id info
		Glib::ustring global_id_src = "" ;
		if (dialog_src->is_global())
			global_id_src = id ;

		dialog->replaceSpeaker(current_id, global_id_src) ;
		drag_initialised = false ;
	}
	else
		gdk_beep() ;
}


void DictionaryManager::setDragAndDropTarget(SpeakerDico_dialog* dialog)
{
	if (!dialog)
	{
		dragdropTarget.set_info(0) ;
		dragdropTarget.set_flags(Gtk::TARGET_SAME_APP) ;
		dragdropTarget.set_target("TRANSCRIBER_DICTIONARY_ROW") ;
		dragdropTargetList.push_back(dragdropTarget) ;
	}
	else
		dialog->setDragAndDropTarget(dragdropTargetList) ;
}

Gtk::TargetEntry DictionaryManager::getDragAndDropTarget()
{
	return dragdropTarget ;
}


//******************************************************************************
//									STATUS
//******************************************************************************

bool DictionaryManager::is_opened(Glib::ustring filepath)
{
	bool res = false ;
	std::vector<SpeakerDico_dialog*>::iterator it =  dicos.begin() ;
	while ( it!=dicos.end() && !res ) {
		if ((*it)->get_fileName() == filepath)
			res=true ;
		it++ ;
	}
	return res ;
}

bool DictionaryManager::is_opened_global()
{
	bool res = false ;
	std::vector<SpeakerDico_dialog*>::iterator it =  dicos.begin() ;
	while ( it!=dicos.end() && !res ) {
		if ((*it)->get_fileName() == "")
			res=true ;
		it++ ;
	}
	return res ;
}

SpeakerDico_dialog* DictionaryManager::get_dictionary(Glib::ustring path)
{
	if (path.compare("")==0 )
		return global_dictionary ;
	else
	{
		SpeakerDico_dialog* res = NULL ;
		std::vector<SpeakerDico_dialog*>::iterator it =  dicos.begin() ;
		while ( it!=dicos.end() && !res ) {
			if ((*it)->get_fileName().compare(path)==0)
				res=*it ;
			else
				it++ ;
		}
		return res ;
	}
}


//******************************************************************************
//									 CLOSING
//******************************************************************************

void DictionaryManager::on_close(SpeakerDico_dialog* dialog)
{
	bool found=false ;
	std::vector<SpeakerDico_dialog*>::iterator it =  dicos.begin() ;
	while ( it!=dicos.end() && !found ) {
		if ((*it) == dialog) {
			SpeakerDico_dialog* tmp = *it ;
			dicos.erase(it) ;
			delete(tmp) ;
			found=true ;
		}
		it++ ;
	}
}

bool DictionaryManager::close_all()
{
	bool res = true ;
	for (guint i=0; i<dicos.size() && res; i++) {
		if (dicos[i])
			res = dicos[i]->close_dialog() ;
	}
	return res ;
}

} //namespace

