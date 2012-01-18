/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "TreeView_mod.h"
#include "TreeModel_Columns.h"
#include "Common/Explorer_filter.h"
#include "Explorer_fileHelper.h"
#include "Explorer_tree.h"
#include <gtk/gtk.h>

#include <iostream>

namespace tag {



TreeView_mod::TreeView_mod()
{
	window = NULL ;
	tooltip = new class Explorer_tooltip(*this) ;
	tooltip->hide() ;
	//>by default activate popup and tooltip
	popup_enabled = true ;
	tooltip_enabled = true ;
}

TreeView_mod::~TreeView_mod()
{
	if (tooltip) {
		tooltip->stop() ;
		delete(tooltip) ;
	}
}

void TreeView_mod::set_window(Gtk::Window* win)
{
	if (win) {
		window = win ;
		tooltip->set_transient_for(*window);
	}
}


void TreeView_mod::enable_popup_paste(bool value)
{
	popup.enable_popup_paste(value) ;
}

bool TreeView_mod::on_button_press_event(GdkEventButton* event)
{
	tooltip->stop() ;

	TreeModel_Columns m_Columns ;

	bool res = TreeView::on_button_press_event(event) ;
	int x, y;
	int x2, y2 ;
	Gtk::TreePath path ;
	Gtk::TreeViewColumn* col ;
	Gtk::TreeRow row ;

	// RIGHT CLICK ON BUTTON PRESS
	if ( (event->type == GDK_BUTTON_PRESS) && (event->button==3) && popup_enabled )
	{
		x = (int) event->x ;
		y = (int) event->y ;
		bool exist = get_path_at_pos(x, y, path, col, x2, y2) ;

		if (exist) {
			Explorer_filter* filter = Explorer_filter::getInstance() ;

			popup_active_iter = get_model()->get_iter(path) ;
			//TODO check if path not null
			row = *popup_active_iter ;

			//audio file
			if (filter->is_audio_file(row[m_Columns.m_file_name]) )
				popup.prepare_explorer_popup(101) ;
			//annotation file
			else if (filter->is_import_annotation_file(row[m_Columns.m_file_name]) )
				popup.prepare_explorer_popup(102) ;
			//if classic directory
			else if ( row[m_Columns.m_file_sysType]==1 )
				popup.prepare_explorer_popup(1) ;
			//system root
			else if ( row[m_Columns.m_file_sysType]==2)
				popup.prepare_explorer_popup(2) ;
			//project root
			else if (row[m_Columns.m_file_sysType]==3 )
				popup.prepare_explorer_popup(3) ;
			//shortcut root
			else if ( row[m_Columns.m_file_sysType]==6 )
				popup.prepare_explorer_popup(6) ;
			//audio files
			else
				popup.prepare_explorer_popup(-1) ;

			//popup.get_popup_menu()->accelerate(*this) ;
			popup.popup(event->button, event->time) ;
		}

	}
	return res ;
}

void TreeView_mod::on_cursor_changed()
{
	selection_active_iter = get_selection()->get_selected() ;
	m_signalSelection.emit(&selection_active_iter);
}


bool TreeView_mod::on_motion_notify_event(GdkEventMotion* event)
{
	bool res = TreeView::on_motion_notify_event(event) ;
	if (tooltip_enabled)
		tooltip->launch(*event, this) ;

	return res ;
}

bool TreeView_mod::on_leave_notify_event(GdkEventCrossing* event)
{
	if (tooltip_enabled)
		tooltip->stop() ;
	return false ;
}

bool TreeView_mod::on_key_press_event(GdkEventKey* event)
{
	tooltip->stop() ;
	return Gtk::TreeView::on_key_press_event(event) ;
}

void TreeView_mod::on_hide()
{
	TreeView::on_hide() ;
	tooltip->hide() ;
	tooltip->stop() ;
}

bool TreeView_mod::on_focus_out_event(GdkEventFocus* event)
{
	bool res = TreeView::on_focus_out_event(event) ;
	tooltip->stop() ;
	tooltip->hide() ;
	return res ;
}

Explorer_popup* TreeView_mod::get_popup()
{
	return &popup ;
}

void TreeView_mod::disable_tooltip()
{
	tooltip_enabled = false ;
}

void TreeView_mod::enable_tooltip()
{
	tooltip_enabled = true ;
}

void TreeView_mod::disable_popup()
{
	popup_enabled = false ;
}

} //namespace

