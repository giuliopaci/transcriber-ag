/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include <gtkmm/icontheme.h>
#include <gtkmm.h>
#include "Explorer_utils.h"
#include "Explorer_dialog.h"
#include "Explorer_fileHelper.h"
#include "NoteBook_mod.h"
#include <iostream>
#include <string>
#include "Editors/AnnotationEditor/AnnotationEditor.h"
#include "Common/FileInfo.h"
#include "Common/VersionInfo.h"


namespace tag {


//------------------------------------------------------------------------------
//								INTERNAL CLASS : HEADER
//------------------------------------------------------------------------------

NoteBook_mod::NoteBook_header::NoteBook_header(int p_indice, const std::string& lab, Configuration* configuration, NoteBook_mod* p_parent)
{
	parent = p_parent ;
	indice = p_indice ;

	//> progress indicator
	setProgressImage(configuration) ;
	left_image.hide() ;

	//> reload indicator
	setReloadImage() ;
	reload_image.hide() ;

	//> label
	label.set_label(lab);

	//> close button
	button.set_name("notebook_button") ;
	button.set_image(1, ICO_NOTEBOOK_CLOSE, TAG_NOTEBOOK_TAB_SIZE) ;
	button.set_image(2, ICO_NOTEBOOK_CLOSE_2, TAG_NOTEBOOK_TAB_SIZE) ;
	button.set_image(3, ICO_NOTEBOOK_CLOSE_3, TAG_NOTEBOOK_TAB_SIZE) ;
	button.set_tooltip(_("Close")) ;

	//settooltip
	tooltip.set_tip(*this, lab) ;

	//put button and label in HBox
	pack_start(reload_image, false, false, 3) ;
	pack_start(left_image, false, false, 5) ;
	pack_start(label_box, false, false, 0) ;
		label_box.add(label) ;
		label_box.add_events(Gdk::BUTTON_PRESS_MASK) ;
	pack_start(label_star, false, false, 2) ;
	pack_start(expanding_blank, true, true) ;
	pack_start(button, false, false, 1) ;

	expanding_blank.set_label(" ") ;
	expanding_blank.show() ;
	label_star.set_label(" ") ;
	label_star.set_name("notebook_label_updated") ;
	label_star.show() ;
	button.show() ;
	label.show() ;
	label_box.show() ;
	label_box.signal_button_press_event().connect(sigc::mem_fun(this, &NoteBook_mod::NoteBook_header::my_on_button_press_event)) ;
	left_image.show() ;
	show() ;
	button.set_sensitive(false) ;

	updated = false ;
	loaded = false ;
	refreshing = false ;
	preferences_modified = false ;

	loading_timer.start() ;
}

NoteBook_mod::NoteBook_header::~NoteBook_header()
{
	remove(label_box);
	remove(button);
}

bool NoteBook_mod::NoteBook_header::my_on_button_press_event(GdkEventButton* event)
{
	if (event->type==GDK_2BUTTON_PRESS && event->button==1 && loaded) {
		m_signalHeaderDoubleClick.emit() ;
		return true ;
	}
#if !defined __APPLE__ && !defined WIN32
//BUG with event on mac & win ? Let's disable that additional feature (no indispensable)
	else if (event->type==GDK_BUTTON_PRESS && event->button==2 && loaded) {
		m_signalHeaderWheelClick.emit(indice) ;
		return true ;
	}
#endif
	else
		return Gtk::HBox::on_button_press_event(event) ;
}

void NoteBook_mod::NoteBook_header::setProgressImage(Configuration* configuration)
{
	bool ok = true ;

	if (configuration!=NULL) {
		std::string conf = configuration->get_CONFIG_path() ;
		std::string iconsdir = FileInfo(conf).join("icons");
		std::string icons_gui = FileInfo(iconsdir).join("GUI");
		std::string progress = FileInfo(icons_gui).join(ICO_NOTEBOOK_PROGRESS);
		ok = left_image.set_image_path(progress, TAG_NOTEBOOK_TAB_SIZE) ;
	}
	if (!ok || configuration==NULL)
		left_image.set_image(ICO_NOTEBOOK_CLOCK, TAG_NOTEBOOK_TAB_SIZE-2) ;
}

void NoteBook_mod::NoteBook_header::setReloadImage()
{
	reload_image.set_image(1, ICO_PREFERENCES_RELOAD1, TAG_NOTEBOOK_TAB_SIZE-6) ;
	reload_image.set_image(2, ICO_PREFERENCES_RELOAD1_OVER, TAG_NOTEBOOK_TAB_SIZE-6) ;
	reload_image.set_image(3, ICO_PREFERENCES_RELOAD1_PRESSED, TAG_NOTEBOOK_TAB_SIZE-6) ;

	std::string txt = _("File needs to be reloaded for applying preferences modifications") ;
	txt.append("\n") ;
	txt.append(_("Click for reloading")) ;

	reload_image.set_tooltip(txt) ;
}

void NoteBook_mod::NoteBook_header::setReloadState(bool on)
{
	if (on) {
		reload_connection = reload_image.signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(parent, &NoteBook_mod::on_reload_required), indice)) ;
		reload_image.show() ;
	}
	else {
		reload_connection.disconnect() ;
		reload_image.hide() ;
	}

	preferences_modified = on ;
}


//------------------------------------------------------------------------------
//										CLASS
//------------------------------------------------------------------------------


NoteBook_mod::NoteBook_mod()
{
	dicoManager = NULL ;
	treeManager = NULL ;
	configuration = NULL ;

	mode_large = false ;
	old_page = -1 ;
	first_tab_passed=false ;
	current_n_header = 0 ;
	set_scrollable(true) ;
	popup_disable() ;
	show_all() ;
	set_name("notebook_style") ;
 }

NoteBook_mod::~NoteBook_mod()
{
	std::map<int, Gtk::Widget*>::iterator it_w ;
	for (it_w=widgets_map.begin(); it_w!=widgets_map.end(); it_w++) {
		if (it_w->second)
			delete(it_w->second) ;
	}

	std::map<int, NoteBook_header*>::iterator it_h ;
	for (it_h=head_map.begin(); it_h!=head_map.end(); it_h++) {
		if (it_h->second)
			delete(it_h->second) ;
	}

	std::map<int, Gtk::VBox*>::iterator it_b ;
	for (it_b=wrapbox_map.begin(); it_b!=wrapbox_map.end(); it_b++) {
		if (it_b->second)
			delete(it_b->second) ;
	}
}

void NoteBook_mod::setDragAndDropTarget(std::vector<Gtk::TargetEntry> list)
{
	drag_dest_set(list,Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
}

//******************************************************************************
//									GETTER
//******************************************************************************

std::map<int, Gtk::Widget*>* NoteBook_mod::get_widgets()
{
	return &widgets_map ;
}


//******************************************************************************
//									OPENING
//******************************************************************************

Gtk::Widget* NoteBook_mod::tab_annotation_editor(Gtk::Window* window, const std::string& file, bool savedState)
{
	//-- Create and stock
	widgets_map[current_n_header+1] = new AnnotationEditor(window) ;
	AnnotationEditor* editor = (AnnotationEditor*)widgets_map[current_n_header+1] ;

	//-- Append in display
	int indice = append_mod(editor, file, savedState)  ;

	//-- Connect first useful signal for display preparation
	editor->signalDisplayReloading().connect(sigc::bind<int>(sigc::mem_fun(*this, &NoteBook_mod::setPageAsRefreshing), indice)) ;

	return editor ;
}


void NoteBook_mod::cancel_tab_annotation_editor()
{
	Gtk::VBox* wrap = (Gtk::VBox*)getWrapboxByIndice(current_n_header) ;
	if (wrap)
	{
		remove(*wrap) ;
		delete_widgets(current_n_header) ;
		delete_header(current_n_header) ;
		delete_wrapbox(current_n_header) ;
	}
}


int NoteBook_mod::append_mod(Gtk::Widget* myWidget, const std::string& path, bool saveState)
{
	//> get the name of the file
	std::string name = Glib::path_get_basename(path) ;

	//> actualise indice
	current_n_header++ ;

	//> create header and stocking it
	head_map[current_n_header] = new NoteBook_header(current_n_header, name, configuration, this) ;
	NoteBook_header* h = head_map[current_n_header] ;

	//> create vbox and stocking it
	wrapbox_map[current_n_header] = new Gtk::VBox() ;
	Gtk::VBox* wrapbox = wrapbox_map[current_n_header] ;
	wrapbox->pack_start(*myWidget, true, true) ;

	//> mark the header with indice
	int header_indice = current_n_header ;
	h->indice = header_indice ;

	//> add a page with the header to notebook
	//append_page(*myWidget, *( h->get_box() ), *(h->get_label_menu()) );
	//h->set_size_request(TAG_NOTEBOOK_TAB_LABEL_SIZE,-1) ;
	set_tab_label_packing(*wrapbox, true, true, Gtk::PACK_START) ;
	append_page(*wrapbox, *h);
	show_all() ;
	wrapbox->show_all() ;

	//> check name
	//reprocess_name(myWidget, path) ;
	Gtk::Widget* w = (Gtk::Widget*)wrapbox ;
	change_label_name(w, _("Loading file ..."), false) ;
	change_label_state(wrapbox,1) ;

	//> connect to close button signal
	h->button.signal_button_release_event().connect( sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &NoteBook_mod::on_close_button_wrap), myWidget));
	h->signalHeaderDoubleClick().connect(sigc::mem_fun(*this, &NoteBook_mod::on_signalHeaderDoubleClick)) ;
	h->signalHeaderWheelClick().connect(sigc::mem_fun(*this, &NoteBook_mod::on_signalHeaderWheelClick)) ;

	//> connect with closing signal of widget
	((AnnotationEditor*)myWidget )->signalCanClose().connect(sigc::bind<Gtk::Widget*, int>(sigc::mem_fun(*this, &NoteBook_mod::on_can_close_received), wrapbox, current_n_header)) ;
	((AnnotationEditor*)myWidget )->signalCloseCancelled().connect(sigc::mem_fun(*this, &NoteBook_mod::on_close_cancelled)) ;
	((AnnotationEditor*)myWidget )->signalFileSaved().connect(sigc::bind<Gtk::Widget*, int>(sigc::mem_fun(*this, &NoteBook_mod::on_file_saved), wrapbox, current_n_header)) ;
	((AnnotationEditor*)myWidget )->signalFileModified().connect(sigc::bind<Gtk::Widget*, int>(sigc::mem_fun(*this, &NoteBook_mod::on_file_updated), wrapbox, current_n_header)) ;
	((AnnotationEditor*)myWidget )->signalReady().connect(sigc::bind<Gtk::Widget*, Gtk::VBox*, std::string, bool, int>( sigc::mem_fun(*this, &NoteBook_mod::on_loading_done),
				myWidget, wrapbox, path, saveState, current_n_header));
	//>set current page
	guint n = get_n_pages() ;
	set_current_page(n-1) ;

	set_waiting_state(h) ;

	return current_n_header ;
}


void NoteBook_mod::on_loading_done(Gtk::Widget* widget, Gtk::VBox* box, const std::string& path, bool saveState, int indiceHeader)
{
	int indice = indiceHeader ;
	NoteBook_header* header = getHeaderByIndice(indice) ;
	if (!header)
		return ;

	if (header!=NULL)
	{
		//the signal will be received each time editor is refresh
		//we only want to process it for loading tab, otherwise block
		if (header->loaded && !header->refreshing)
			return ;

		bool loading = !header->loaded ;
		header->loaded = true ;

		bool refreshing = header->refreshing ;
		header->refreshing = false ;

		//header->left_image.hide() ; for the moment change icon instead
		AnnotationEditor* edit = (AnnotationEditor*) widget ;

		// if we're loading, force total display
		if (loading)
			edit->setSignalOffset(-1) ;

		//> normal state
		change_label_state(box,false) ;

		//> actualise tab name
		std::string name = edit->getFileName() ;
		std::string name2 =  Glib::path_get_basename(name) ;
		change_label_name(box, name2, true) ;

		// after loading, set default file status display
		if (loading)
		{
			//> Using existing file & no modifications were applied
			if ( saveState && !edit->fileModified())
			{
				// state is classic
				set_file_state(indiceHeader,false) ;
				// add in recent files
				m_signalAddRecentFile(name) ;
			}
			//> File is modified or newly created,
			else
			{
				// existing file, even if modified, add it to recent file
				if (saveState)
					m_signalAddRecentFile(name) ;

				// state is updated
				set_file_state(indiceHeader,true) ;
				change_label_state(box,true) ;
			}
			signalStatusBar().emit( std::string(_("Loading done...")) ) ;
		}
		// after refreshing, get the file status and actualize display
		else
			change_label_state(box, edit->fileModified()) ;

		//> if first page send corresponding signal
		if (!first_tab_passed)
		{
			first_tab_passed = true ;
			//old_page = get_current_page() ;
			old_page = page_num(*box) ;
			m_signalFirstLastTab.emit("first", widget);
			m_signalActualiseUI.emit("first");
		}
		//> other page
		else {
			//> iterating over pages of notebook to compare box indice
			Gtk::Notebook_Helpers::PageList _pages = pages() ;
			Gtk::Notebook_Helpers::PageList::iterator it = _pages.find(*box) ;
			guint num_page = it->get_page_num() ;
			signalPersonalSwitch().emit(num_page) ;
		}

		//> actualise opened_files file
		close_all(configuration->get_NOTEBOOK_opened_path(), true) ;

		header->button.show() ;
		header->button.set_sensitive(true) ;

		signalStatusBar().emit(TRANSAG_VERSION_NO) ;

		header->loading_timer.stop() ;
		TRACE << "~~~~~~~~~~~~~~~~~~~~~ ToTAL LoADiNG " << header->loading_timer.elapsed() << std::endl ;
	}
	else {
		Explorer_utils::print_trace("on_loading_done:> HEADER ERROR", 0) ;
	}
}

//******************************************************************************
//									SAVE & UPDATE
//******************************************************************************

void NoteBook_mod::on_file_saved(const std::string& path, bool newfile, bool force_rename, bool force_expand, Gtk::Widget* wrapBox, int indice)
{
	//> if file currently updated, mark it as saved
	//TODO NORMALLY THIS CASE SHOULD NOT EXIST, SAVE SHOULD ONLY BE RECEIVED
	//IF FILE HAS BEEN MODIFIED
	NoteBook_header* head = getHeaderByIndice(indice) ;

	if ( head && head->updated ) 
	{
		change_label_state(wrapBox, false) ;
		set_file_state(indice,false) ;
	}
	//> Force is done when renaming and overwriting, or converting TRS into TAG.
	//> For these cases "newfile" will be false, but we need to update name
	//  and add to recent file.
	if (newfile || force_rename)
	{
		//> save its new name
		std::string name = Glib::path_get_basename(path) ;
		change_label_name(wrapBox, name, true) ;
		//> add in recent files
		m_signalAddRecentFile.emit(path) ;
		// only if name is new update tree
		if ( !force_rename || (force_rename && force_expand) )
			m_signalNewTreeFile.emit(path) ;
	}
}

void NoteBook_mod::on_file_updated(bool update, Gtk::Widget* wrapbox, int indice)
{
	NoteBook_header* head = getHeaderByIndice(indice) ;

	//> if file is currently saved and signal updated, mark it as updated
	if (update && head && !head->updated) {
		change_label_state(wrapbox, true) ;
		set_file_state(indice, true) ;
	}
	//> if file is currently updated and signal saved, mark it as saved
	else if (!update && head && head->updated) {
		change_label_state(wrapbox, 0) ;
		set_file_state(indice, false) ;
	}
}

/*
 *  Change label if file has been updated and not saved
 *  mode 0: set label as saved
 *	mode 1: set label as updated and not saved
 */
int NoteBook_mod::change_label_state(Gtk::Widget* wrapbox, bool mode)
{
	//> get box header from given widget
	NoteBook_header* header = (NoteBook_header*) get_tab_label(*wrapbox) ;

	//> get label (first element of boxHeader)
	Gtk::Label* label = &(header->label) ;

	std::string tmp ;

	int ok = 1 ;

	if (mode) {
		tmp = label->get_label() ;
		//tmp.append("*");
		tmp = ellipsize_label(tmp) ;
		label->set_label(tmp) ;
		label->set_name("notebook_label_updated") ;
		header->updated = true ;
		header->label_star.set_label("*") ;
	}
	else  {
		tmp = label->get_label() ;
		//tmp = tmp.substr(0,tmp.size()-1) ;
		tmp = ellipsize_label(tmp) ;
		label->set_label(tmp) ;
		label->set_name("notebook_label_saved") ;
		header->updated = false ;
		header->label_star.set_label(" ") ;
	}

	return ok ;
}

void NoteBook_mod::change_label_name(Gtk::Widget* wrapbox, std::string name, bool actualise_icon)
{
	//> get box header from given widget
	Gtk::HBox* current_box = (Gtk::HBox*) get_tab_label(*wrapbox) ;
	NoteBook_header* head = (NoteBook_header*) current_box ;

	int indice = head->indice ;
	AnnotationEditor* edit = (AnnotationEditor*) getWidgetByIndice(indice) ;

	if (!edit)
		return ;

	name = ellipsize_label(name) ;

	if ( head->label.get_label() != name) {
		//> change label
		head->label.set_label(name) ;
		//> change tooltip
		head->tooltip.unset_tip(*current_box) ;
		if (edit)
			head->tooltip.set_tip(*current_box, edit->getFileName()) ;
		//> change icon
		if (actualise_icon && edit) {
			std::string path = edit->getFileName() ;
			Explorer_filter* filter = Explorer_filter::getInstance() ;
			std::string icoPath = filter->icons_by_extension(path) ;
			head->left_image.set_image(icoPath, TAG_NOTEBOOK_TAB_SIZE-9) ;
		}
	}
}



//******************************************************************************
//									 FOR CLOSING
//******************************************************************************

bool NoteBook_mod::on_close_button_wrap(GdkEventButton* event, Gtk::Widget* widget)
{
	on_close_button(widget) ;
	return false ;
}

void NoteBook_mod::on_close_button(Gtk::Widget* widget)
{
	if (!widget)
		return ;

	std::string path = ((AnnotationEditor*)widget)->getFileName() ;
	//don't close if the file dictionary is opened
	if (dicoManager->is_opened(path))
		Explorer_dialog::msg_dialog_warning(_("Please close the file dictionary before closing the editor"),window, true) ;
	else {
		Explorer_utils::print_trace(">closing asked ", path, 1) ;
		( (AnnotationEditor*)widget )->closeFile(false, false) ;
	}
}

void NoteBook_mod::on_can_close_received(Gtk::Widget* box, int header_indice)
{
	Explorer_utils::print_trace(">closing initiated ", header_indice, 1) ;
	if (box!=NULL)
	{
		//> remove box from notebook
		AnnotationEditor* obj = (AnnotationEditor*) getWidgetByIndice(header_indice) ;

		if (!obj)
			return ;

		remove(*box) ;

		Explorer_utils::print_trace(">removed from book ", 1) ;

		//> when last file, remove action group and ui
		// in other case, it is done on the switching callback
		if (get_n_pages()==0) {
			if (obj->hasSignalView())
				m_signalRemoveActionGroup.emit(obj->getActionGroup("signal")) ;
			m_signalRemoveActionGroup.emit(obj->getActionGroup("file")) ;
			m_signalRemoveActionGroup.emit(obj->getActionGroup("annotate")) ;
			m_signalRemoveActionGroup.emit(obj->getActionGroup("edit")) ;
			m_signalActualiseUI.emit("last") ;
			m_signalFirstLastTab.emit("last", NULL) ;
			first_tab_passed = false ;
			old_page = -1 ;
		}

		//> remove widget from list
		delete_widgets(header_indice) ;

		//> delete_header(header_indice)
		delete_header(header_indice) ;

		//> delete_wrapbox(header_indice)
		delete_wrapbox(header_indice) ;

		Explorer_utils::print_trace(">removed from memory ", 1) ;
		if (get_n_pages()==0) {
			m_signalClosedLastTab.emit(true) ;
		}
		//> actualise opened_files file
		close_all(configuration->get_NOTEBOOK_opened_path(), true) ;
	}
	else {
		Explorer_utils::print_trace("on_can_close_received:> ERROR closed object", 0) ;
	}
}

// when closing dialog is displayed, and user choses cancel
void NoteBook_mod::on_close_cancelled()
{
	m_signalClosedLastTab.emit(false) ;
}

void NoteBook_mod::close_current_page()
{
	Gtk::Widget* w = get_active_widget() ;
	on_close_button(w) ;
}

void NoteBook_mod::close_all(const std::string& path, bool only_save)
{
	std::vector<Glib::ustring> vect ;
	std::map<int, Gtk::Widget*>::iterator it = widgets_map.begin() ;
	while (it!=widgets_map.end())
	{
		AnnotationEditor* tmp = (AnnotationEditor*)(it->second) ;
		it++ ;
		if (tmp!=NULL)
		{
			std::string name = std::string( tmp->getFileName() ) ;
			vect.insert(vect.end(), name) ;
			if (!only_save)
			{
				setActiveWidget(tmp) ;
				tmp->closeFile(false, false) ;
			}
		}
	}
	int res =Explorer_utils::write_lines(path, vect, "w");
	if (res<0)
		TRACE << "NoteBook_mod::close_all:> " << res << std::endl ;
}


void NoteBook_mod::delete_header(int indice)
{
	NoteBook_header * tmp = getHeaderByIndice(indice) ;
	head_map.erase(indice) ;
	if (tmp)
		delete(tmp) ;
}

void NoteBook_mod::delete_widgets(int indice)
{
	Gtk::VBox* box = (Gtk::VBox*)getWrapboxByIndice(indice) ;
	if (!box)
		return ;

	Gtk::Widget* widget = getWidgetByIndice(indice) ;
	if (!widget)
		return ;

	NoteBook_header* head = getHeaderByIndice(indice) ;
	if (head && head->loaded && box && widget)
		box->remove(*widget) ;

	widgets_map.erase(indice) ;
	wrapbox_map.erase(indice) ;

	if (widget!=NULL)
		delete(widget) ;

	if (box!=NULL)
		delete(box) ;
}

void NoteBook_mod::delete_wrapbox(int indice)
{
	Gtk::VBox* tmp = (Gtk::VBox*)getWrapboxByIndice(indice) ;
	wrapbox_map.erase(indice) ;
	if (tmp)
		delete(tmp) ;
}


//******************************************************************************
//									CALLBACK
//******************************************************************************

void NoteBook_mod::setActiveWidget(int page_num)
{
	if (page_num < 0 || page_num > get_n_pages ())
		return ;

	if (get_current_page() != page_num)
		set_current_page(page_num) ;
}

void NoteBook_mod::setActiveWidget(Gtk::Widget* w)
{
	if (!w)
		return ;

	Gtk::Widget* wrap = w->get_parent() ;
	if ( !wrap )
		return ;

	int num = page_num(*wrap) ;
	setActiveWidget(num) ;
}

void NoteBook_mod::switch_tab(const std::string& mode)
{
	if (mode=="next")
		next_page() ;
	else if (mode=="previous")
		prev_page() ;
}

bool NoteBook_mod::is_loaded_page(int num)
{
	Gtk::Widget* widget = get_nth_page(num) ;
	NoteBook_header* head = NULL ;
	if (widget!= NULL)
		head = (NoteBook_header*) get_tab_label(*widget) ;
	if (head!=NULL)
		return head->loaded ;
	else
		return false ;
}

std::string NoteBook_mod::ellipsize_label(const std::string& name)
{
	std::string res ;
	int size = name.size() ;

	if (size > TAG_NOTEBOOK_TAB_LABEL_MAXSIZE) {
		if (name.c_str()[size-1] == '*') {
			res = name.substr(0, TAG_NOTEBOOK_TAB_LABEL_MAXSIZE-4) ;
			res.append(TAG_NOTEBOOK_TAB_LABEL_ELLIPSE) ;
			res.append("*") ;
		}
		else {
			res = name.substr(0, TAG_NOTEBOOK_TAB_LABEL_MAXSIZE-3) ;
			res.append(TAG_NOTEBOOK_TAB_LABEL_ELLIPSE) ;
		}
	}
	else
		res = name ;

	return res ;
}

int NoteBook_mod::get_loaded_tabs(std::vector<AnnotationEditor*>& res)
{
	res.clear() ;
	std::map<int,Gtk::Widget*>::iterator it = widgets_map.begin();
	while(it!=widgets_map.end()) {
		if (it->second)
			res.insert(res.begin(), (AnnotationEditor*)it->second) ;
		it ++ ;
	}
	return res.size() ;
}

void NoteBook_mod::on_signalHeaderDoubleClick()
{
	toggle_large_mode() ;
}

void NoteBook_mod::toggle_large_mode()
{
	mode_large = !mode_large ;
	m_signalHeaderDoubleClick.emit(mode_large) ;
}

void NoteBook_mod::on_signalHeaderWheelClick(int indice)
{
	Gtk::Widget* w = getWidgetByIndice(indice) ;
	if (!w)
		return ;

	setActiveWidget(w) ;
	on_close_button(w) ;
}

//******************************************************************************
//								 REFRESHING
//******************************************************************************


void NoteBook_mod::setPageAsRefreshing(int indice)
{
	Gtk::VBox* wrapbox = (Gtk::VBox*)getWrapboxByIndice(indice) ;
	if (!wrapbox)
		return ;

	NoteBook_header* head = getHeaderByIndice(indice) ;
	if (!head || !head->loaded)
		return;

	head->refreshing = true ;
	head->loading_timer.reset() ;
	head->loading_timer.start() ;
	head->setProgressImage(configuration) ;
	head->button.set_sensitive(false) ;

	change_label_name(wrapbox, _("Refreshing..."), false) ;
	change_label_state(wrapbox, true) ;
}

void NoteBook_mod::set_waiting_state(NoteBook_header* header)
{
	if (header!=NULL)
		header->left_image.show() ;
}


//******************************************************************************
//								 DYNAMIC RELOADING
//******************************************************************************


void NoteBook_mod::need_reload(bool all)
{
	if (all) {
		std::map<int, NoteBook_header*>::iterator it	 ;
		for ( it=head_map.begin(); it!=head_map.end(); it++ )
			if (it->second)
				it->second->setReloadState(true) ;
	}
	else
	{
		//> get active page
		int num = get_current_page() ;
		Gtk::Widget* widget = get_nth_page(num) ;
		NoteBook_header* head = NULL ;
		if (widget!= NULL)
			head = (NoteBook_header*) get_tab_label(*widget) ;
		if (head)
			head->setReloadState(true) ;
	}
}

bool NoteBook_mod::on_reload_required(GdkEventButton* event, int indice)
{
	Gtk::Widget* widget = getWidgetByIndice(indice) ;
	if (widget)
		signalReloadPage().emit(indice, widget) ;

	return false ;
}

void NoteBook_mod::disableReloadState(int indice)
{
	NoteBook_header* head = getHeaderByIndice(indice) ;

	if (!head)
		return ;

	head->setReloadState(false) ;
}


//******************************************************************************
//								ACCESSORS & UTILS
//******************************************************************************


int NoteBook_mod::is_opened_file(const std::string& path)
{
	AnnotationEditor* tmp ;
	int res = -1 ;

	std::map<int,Gtk::Widget*>::iterator it = widgets_map.begin();

	while( it!=widgets_map.end() && res==-1 )
	{
		tmp = (AnnotationEditor*)it->second ;
		if (tmp!=NULL)
		{
			if (tmp->getFileName()==path)
			{
				int indice = it->first ;
				NoteBook_header* head = (NoteBook_header*) getHeaderByIndice(indice) ;
				if (head && head->loaded)
					res = indice ;
			}
		}
		it++ ;
	}
	return res ;
}


Gtk::Widget* NoteBook_mod::get_widget_by_path(const std::string& path)
{
	AnnotationEditor* tmp=NULL ;

	std::map<int,Gtk::Widget*>::iterator it = widgets_map.begin();
	bool found=false ;

	while( it!=widgets_map.end() && found==false  )
	{
		tmp = (AnnotationEditor*)it->second ;
		if (tmp!=NULL)
		{
			if (tmp->getFileName()==path)
				found=true ;
		}
		it++ ;
	}
	if (found)
		return (Gtk::Widget*)tmp ;
	else
		return NULL ;
}

std::string NoteBook_mod::get_path_from_page_num(int num)
{
	//get widget for given page
	Gtk::VBox* box = (Gtk::VBox*) get_nth_page(num) ;
	AnnotationEditor* editor =  (AnnotationEditor*) get_widget_from_wrapBox(box) ;
	if (editor!=NULL)
		return editor->getFileName() ;
	else
		return NULL ;
}

Gtk::Widget* NoteBook_mod::get_page_widget(int n)
{
	Gtk::VBox* box = (Gtk::VBox*) get_nth_page(n) ;
	return get_widget_from_wrapBox(box) ;
}

Gtk::Widget* NoteBook_mod::get_active_widget()
{
	//> get active page
	int num = get_current_page() ;

	if (num==-1)
		return NULL ;
	//> get active widget
	else {
		Gtk::VBox* box = (Gtk::VBox*) get_nth_page(num) ;
		return get_widget_from_wrapBox(box) ;
	}
}

Gtk::Widget* NoteBook_mod::get_widget_from_wrapBox(Gtk::VBox* box)
{
	if (box!=NULL) {
		Glib::ListHandle<Gtk::Widget*> list = box->get_children() ;
		std::vector<Gtk::Widget*> vect = std::vector<Gtk::Widget*>(list);
		Gtk::Widget* w = vect[0] ;
		return w ;
	}
	else
		return NULL ;
}

void NoteBook_mod::set_file_state(int indice, bool state)
{
	NoteBook_header* head = getHeaderByIndice(indice) ;
	if (head)
		head->updated = state  ;
}

//******************************************************************************
//							INTERNAL SAFE ACCESS
//******************************************************************************

Gtk::Widget* NoteBook_mod::getWidgetByIndice(int indice)
{
	std::map<int, Gtk::Widget*>::iterator it = widgets_map.find(indice) ;
	if (it!=widgets_map.end())
		return it->second ;
	else
		return NULL ;
}

Gtk::Widget* NoteBook_mod::getWrapboxByIndice(int indice)
{
	std::map<int, Gtk::VBox*>::iterator it = wrapbox_map.find(indice) ;
	if (it!=wrapbox_map.end())
		return it->second ;
	else
		return NULL ;
}

NoteBook_mod::NoteBook_header* NoteBook_mod::getHeaderByIndice(int indice)
{
	std::map<int, NoteBook_header*>::iterator it = head_map.find(indice) ;
	if (it!=head_map.end())
		return it->second ;
	else
		return NULL ;
}


} //namespace


