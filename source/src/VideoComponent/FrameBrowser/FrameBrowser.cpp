/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  TransAGVideo				*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "FrameBrowser.h"
#include "FrameFiller.h"
#include "Common/util/Log.h"
#include "Common/globals.h"
#include "Common/FileInfo.h"
#include "Common/icons/Icons.h"
#include "Common/Dialogs.h"

#define FRAMEBROWSER_PREVIEW_DEFAULTWIDTH			80
#define FRAMEBROWSER_PREVIEW_DEFAULTHEIGHT			40
#define FRAMEBROWSER_PREVIEW_DEFAULTSTEP			5

namespace tag {

//------------------------------------------------------------------------------
//								 INTERNAL HEADER CLASS
//------------------------------------------------------------------------------

FrameBrowser::FBLoadingHeader::FBLoadingHeader(bool with_resolution)
{
	enable_resolution = with_resolution ;

	add(hbox) ;

	hbox.pack_start(image, false, false, 3) ;
	hbox.pack_start(label, false, false, 3) ;
	if (with_resolution)
		hbox.pack_start(combo, false, false, 3) ;
	label.set_label(_("Loading...")) ;

	show_all_children() ;

	/* loading state */
	combo.hide() ;
	label.set_name("notebook_label_updated") ;

	set_shadow_type(Gtk::SHADOW_ETCHED_OUT) ;
}

void FrameBrowser::FBLoadingHeader::ready()
{
	image.hide() ;
	if (enable_resolution) {
		label.set_name("notebook_label_saved") ;
		label.set_label(_("Resolution")) ;
		combo.show() ;
	}
	else {
//		label.set_name("bold_label") ;
//		label.set_label(name) ;
		label.hide() ;
	}

}

void FrameBrowser::FBLoadingHeader::showConfigImage(const Glib::ustring& dir)
{
	configdir = dir ;
	if (dir.empty())
		return ;

	Glib::ustring iconsdir = FileInfo(configdir).join("icons");
	Glib::ustring icons_gui = FileInfo(iconsdir).join("GUI");
	Glib::ustring progress = FileInfo(icons_gui).join(ICO_NOTEBOOK_PROGRESS);
	bool ok = image.set_image_path(progress, 12) ;
	if (ok)
		image.show() ;
}


//------------------------------------------------------------------------------
//								 		CLASS
//------------------------------------------------------------------------------



FrameBrowser::FrameBrowser(Gtk::Window* p_toplevel, const Glib::ustring& configpath, bool enableResolution)
{
	Icons::set_window_icon(this, ICO_TRANSCRIBER, 12) ;
	set_title(_("Browser")) ;

	if (p_toplevel)
		set_transient_for(*p_toplevel);

	set_skip_pager_hint(true) ;
	set_skip_taskbar_hint(true) ;

	enable_resolution = enableResolution ;
	isReady = false ;
	toplevel = p_toplevel ;
	configdir = configpath ;
	frameFiller = NULL ;

	width = -1 ;
	height = -1 ;
	step = -1 ;

	header = new FBLoadingHeader(enableResolution) ;
	header->showConfigImage(configpath) ;

	prepareList() ;
	prepareCombo() ;
	prepareGUI() ;
}

FrameBrowser::~FrameBrowser()
{
	if (frameFiller)
		delete(frameFiller) ;
	if (header)
		delete(header) ;
}

//------------------------------------------------------------------------------
//								 INITIALIZATION
//------------------------------------------------------------------------------


void FrameBrowser::prepareGUI()
{
//	add(vbox) ;
	Gtk::VBox* vbox = get_vbox() ;
	vbox->pack_start(*header, false, false) ;
	vbox->pack_start(scrolled, true, true) ;
		scrolled.add(listView) ;
	vbox->show_all_children(true) ;

	scrolled.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC) ;
	show_all_children(true) ;
	header->getCombo()->hide() ;
}

void FrameBrowser::prepareList()
{
	Gtk::CellRendererPixbuf* cellRend_ico = NULL ;
	Gtk::CellRendererText* cellRend_text = NULL ;
	Gtk::TreeViewColumn* pColumn = NULL ;

	FBModelColumns columns ;
	listModel = Gtk::ListStore::create(columns) ;
	filteredListModel = Gtk::TreeModelFilter::create(listModel) ;

	//> Choose the displayed columns
	pColumn = Gtk::manage(new Gtk::TreeViewColumn("image")) ;
	cellRend_ico = Gtk::manage(new Gtk::CellRendererPixbuf()) ;
	cellRend_text = Gtk::manage(new Gtk::CellRendererText()) ;
	if (!pColumn) {
		Log::err() << "FrameBrowser: Error at list view initialization... Aborted." << std::endl ;
		return ;
	}

	cellRend_text->set_property("size-points", 9) ;
	pColumn->pack_start(*cellRend_ico, false);
	pColumn->add_attribute(cellRend_ico->property_pixbuf(), columns.image) ;
	pColumn->pack_start(*cellRend_text, false);
	pColumn->add_attribute(cellRend_text->property_text(), columns.displayTime) ;

	listView.append_column(*pColumn) ;
	listView.set_headers_visible(false) ;
	listView.set_rules_hint(false) ;
	listView.set_model(filteredListModel) ;
	filteredListModel->set_visible_func( sigc::mem_fun(*this, &FrameBrowser::listFilterCallback) ) ;
	listView.signalSelection().connect(sigc::mem_fun(*this, &FrameBrowser::selectionChanged)) ;
};

void FrameBrowser::prepareCombo()
{
	//TODO do resolution mechanism
	Gtk::ComboBoxText* combo = header->getCombo() ;
	combo->signal_changed().connect(sigc::mem_fun(this, &FrameBrowser::on_combo_changed)) ;
}

//------------------------------------------------------------------------------
//								     RESOLUTION
//------------------------------------------------------------------------------

void FrameBrowser::on_combo_changed()
{
	filteredListModel->refilter() ;
}

//------------------------------------------------------------------------------
//								     BUSINESS
//------------------------------------------------------------------------------

bool FrameBrowser::checkVisibility(float time)
{
	if (time < 0)
		return false ;
	else
		//TODO
		return true ;
}

//------------------------------------------------------------------------------
//								     CALLBACK
//------------------------------------------------------------------------------

bool FrameBrowser::listFilterCallback(const Gtk::TreeIter& iter)
{
	FBModelColumns columns ;
	Gtk::TreeRow row = *iter ;
	float time = row[columns.frameTime] ;
	return checkVisibility(time) ;
	return true ;
}

void FrameBrowser::selectionChanged(const Gtk::TreeIter& iter)
{
	selected = filteredListModel->get_path(*iter) ;
	Gtk::TreeRow row = *iter ;
	FBModelColumns columns ;
	float time = row[columns.frameTime] ;
	m_signalFrameChanged.emit(time) ;
}


// --- OnFocusInEvent ---
bool FrameBrowser::on_focus_in_event(GdkEventFocus* e)
{
/*	if (toplevel)
		toplevel->present();
*/
	// -- Focus Out --
	Glib::RefPtr<Gdk::Window> win = get_window();

/*	if (win)
	{
		win->set_focus_on_map(false);
		win->set_accept_focus(false);
	}
*/
#ifndef __APPLE__
	return false ;
#else
	return true;
#endif
}

//------------------------------------------------------------------------------
//								     INTERFACE
//------------------------------------------------------------------------------

void FrameBrowser::fill(const Glib::ustring& p_path, const Glib::ustring& p_width, const Glib::ustring& p_height, const Glib::ustring& p_step)
{
	path = p_path ;
	name = Glib::path_get_basename(path) ;
	header->setName("FrameBrowser") ;
	tooltip.set_tip(*header, path) ;
	set_title(name) ;
	loadGeoAndDisplay() ;

	float w = -1 ;
	float h = -1 ;
	float s = -1 ;
	if (!p_width.empty())
		w = string_to_number(p_width) ;
	if (!p_height.empty())
		h = string_to_number(p_height) ;
	if (!p_step.empty())
		s = string_to_number(p_step) ;

	fill(path, w, h, s) ;
}

void FrameBrowser::fill(const Glib::ustring& path, int p_width, int p_height, int p_step)
{
	// check value
	if (p_width < 0)
		width =  FRAMEBROWSER_PREVIEW_DEFAULTWIDTH ;
	else
		width = p_width ;
	if (p_height < 0)
		height =  FRAMEBROWSER_PREVIEW_DEFAULTHEIGHT ;
	else
		height = p_height ;
	if (p_step < 0)
		step =  FRAMEBROWSER_PREVIEW_DEFAULTSTEP ;
	else
		step = p_step ;

	// proceed
	frameFiller = new FrameFiller(this, path, width, height, step) ;
	bool res = frameFiller->launch() ;
	if (!res)
		dlg::error(_("Error while computing video preview, aborted."), this) ;
	else
		tim.start() ;
}

void FrameBrowser::addFrame(const Glib::RefPtr<Gdk::Pixbuf> pixbuf, float time)
{
	if (!listModel)
		return ;

	gdk_threads_enter();
	Gtk::TreeIter iter = listModel->append() ;
	Gtk::TreeRow row = *iter ;
	FBModelColumns::fill_row(&row, pixbuf, time) ;
	gdk_threads_leave();
}

void FrameBrowser::ready(bool error)
{
	if (error) 
	{
		gdk_threads_enter() ;
		dlg::error(_("Error while loading video preview, aborted.")) ;
		hide() ;
		gdk_threads_leave() ;
	}
	else 
	{
		tim.stop() ;
		// actualize state
		gdk_threads_enter() ;
		header->ready() ;
		gdk_threads_leave() ;
		isReady = true ;
	}
}


void FrameBrowser::setResolutionStep(int p_step)
{
	FBModelColumns columns ;

	listView.unset_model() ;
	listModel->clear() ;
	listModel = Gtk::ListStore::create(columns) ;
	filteredListModel = Gtk::TreeModelFilter::create(listModel) ;
	listView.set_model(filteredListModel) ;
	fill(path, width, height, p_step) ;
}

//------------------------------------------------------------------------------
//								     TIME SYNCHRONIZATION
//------------------------------------------------------------------------------

void FrameBrowser::setToTime(double AGtime)
{
	if (!isReady || !listModel)
		return ;

	std::string s_path = getPathAtTime(AGtime) ;
	setCursorAtPath(s_path) ;
}

void FrameBrowser::setCursorAtPath(string s_path)
{
	if (s_path.empty())
		return ;

	Gtk::TreePath path(s_path) ;

	// check path
	Gtk::TreeIter iter = listModel->get_iter(path) ;
	if (listModel->iter_is_valid(iter)) {
		listView.scroll_to_row(path) ;
		listView.set_cursor(path) ;
	}
	else
		Log::err() << "setCursorAtPath:> invalid path" << std::endl ;
}


std::string FrameBrowser::getPathAtTime(double AGtime)
{
	FBModelColumns columns ;
	Gtk::TreeModel::Children children = listModel->children() ;
	Gtk::TreeIter iter = children.begin() ;
	Gtk::TreeRow row ;
	Gtk::TreePath path ;
	std::string s_path = "" ;
	std::string last_s_path = "" ;

	// TODO do it for all type of hierarchy
	for (iter = children.begin(); iter!= children.end() && s_path.empty(); iter++ )
	{
		row = *iter ;
		float current_time =  row[columns.frameTime] ;

		if ( current_time > AGtime && !last_s_path.empty() )
			s_path = last_s_path ;
		else
		{
			path = listModel->get_path(iter) ;
			last_s_path = path.to_string() ;
			if (current_time == AGtime)
				s_path = last_s_path ;
		}
	}
	return s_path ;
}


bool FrameBrowser::on_delete_event(GdkEventAny* event)
{
	m_signalCloseButton.emit() ;
	return true ;
}

//-- Prevent frame browser to close on escape key
//   because escape is used for media play action
bool FrameBrowser::on_key_press_event(GdkEventKey* event)
{
	if (event && event->keyval==GDK_Escape)
		return false ;
	return Gtk::Window::on_key_press_event(event) ;
}

//**************************************************************************************
//										GEOMETRY INTERFACE
//**************************************************************************************

void FrameBrowser::getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	get_position(pos_x, pos_y) ;
	panel = -1 ;
	get_size(size_xx, size_yy) ;
}

void FrameBrowser::setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel)
{
	if (pos_x>0 && pos_y>0)
		move(pos_x, pos_y) ;
	if (size_xx>0 && size_yy>0)
		resize(size_xx, size_yy) ;
}

Glib::ustring FrameBrowser::getWindowTagType()
{
	return SETTINGS_VIDEO_BROWSER ;
}

int FrameBrowser::loadGeoAndDisplay(bool rundlg)
{
	loadPos() ;
	show() ;
	return 1 ;
}

void FrameBrowser::saveGeoAndHide()
{
	if (is_visible())
		savePos() ;
	hide() ;
}

void FrameBrowser::getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	size_xx = 90 ;
	size_yy = 768 ;
	pos_x = 960 ;
	pos_y = 140 ;
}

} // namespace
