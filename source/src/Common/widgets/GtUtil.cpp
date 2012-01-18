/*
 * GtUtil.cpp
 *
 *  Created on: 3 févr. 2009
 *      Author: montilla
 */
#include <string.h>
#include "GtUtil.h"

#include "Common/util/FileHelper.h"
#include "Common/Dialogs.h"
#include "Common/FileInfo.h"
#include "Common/globals.h"
#include "Common/util/Utils.h"


namespace tag {

std::string GtUtil::baseColor = "" ;

bool GtUtil::isFunctionKey(guint32 keyval)
{
	const char* name = gdk_keyval_name (keyval);
	if ( name == NULL ) return true;

#ifdef __APPLE__
		static const char* funnames[] = {
				"Up", "Down", "Left", "Right", "End", "Home",
				"Return", "Enter", "Tab", "Escape", "Shift", "Control",
				"Back", "Del", "Lock", "Insert", "Alt", "Meta",
				NULL };
#else
		static const char* funnames[] = {
				"Up", "Down", "Left", "Right", "End", "Home",
				"Return", "Enter", "Tab", "Escape", "Shift", "Control",
				"Back", "Del", "Lock", "Insert","Alt",
				NULL };
#endif

	if (name[0] == 'F' && isdigit(name[1]))
		return true;
	for (int i =0; funnames[i] != NULL; ++i )
		if (strstr(name, funnames[i]) != NULL)
			return true;

	return false;
}

bool GtUtil::isAccelKeyEvent(GdkEventKey* event)
{
	bool is_accel= ( (event->state & GDK_CONTROL_MASK)
				&& strchr("zycvZYCV", (char)event->keyval) != NULL );
	return is_accel;
}

bool GtUtil::isUndoRedoEventKeys(GdkEventKey* event)
{
	if (!event)
		return false ;

	if ( ((event->state & GDK_CONTROL_MASK) && (event->keyval == GDK_z))
			|| ((event->state & GDK_CONTROL_MASK) && (event->keyval == GDK_y)) )
		return true ;

	return false ;
}

void GtUtil::flushGUI(bool mm, bool threadProtection)
{
	if (threadProtection)
		gdk_threads_enter() ;

	if (mm)
	{
		while( Gtk::Main::events_pending() )
			Gtk::Main::iteration();
	}
	else
	{
		while( gtk_events_pending() )
    		gtk_main_iteration();
	}

	if (threadProtection)
		gdk_threads_leave() ;
}

void GtUtil::copy(Glib::ustring src_path, Glib::ustring dest_directory, ProgressionWatcher* progressWatcher, Gtk::Window* top)
{
	Glib::ustring name = Glib::path_get_basename(src_path) ;
	Glib::ustring future_path = Glib::build_filename(dest_directory, name) ;

	if (src_path==future_path)
		return ;

	Glib::ustring label =  Glib::ustring(_("Copy")) + " " + name ;
	Glib::ustring legend = Glib::ustring(_("TO: ")) + dest_directory ;
	ProgressionWatcher::Entry* entry = progressWatcher->add_entry(label, src_path, legend, false) ;
	Gtk::ProgressBar* progress = Glib::wrap(entry->get_pbar());
	progressWatcher->show() ;

	int res = FileHelper::copy_in_filesystem(src_path, dest_directory, progress) ;
	if (res < 0)
	{
		Glib::ustring txt, msg ;
		msg = string(_("Copy failed for file")) + string(" ") + string(name) ;
		if (res==-20)
				txt = _("Permissions problem on file, target directory or source directory") ;
		else if (res==-41)
				txt = _("Can't find the source file or source directory") ;
		else if (res==-42)
				txt = _("Can't find destination directory") ;
		else
				txt = _("Copying file problem") ;
		dlg::error(msg, txt, top) ;
		Log::err() << "Copy error: code " << number_to_string(res) << std::endl ;
	}
	else
	{
		entry->complete_bar() ;
		entry->set_status(true, _("Copied")) ;
	}
}

std::string GtUtil::getBaseColor(Gtk::Widget* widget)
{
	if (baseColor.empty() && widget)
	{
		//> try to get current color
		Glib::RefPtr<Gtk::Style> style = widget->get_style() ;
		if (style)
		{
			Gdk::Color c = style->get_background(Gtk::STATE_NORMAL) ;
			std::vector<Gdk::Color> v ;
			v.push_back(c) ;
			baseColor = Gtk::ColorSelection::palette_to_string(v) ;
		}
	}
	return baseColor ;
}


} //namespace
