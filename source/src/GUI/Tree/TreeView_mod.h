/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef TREEVIEW_MOD_H_
#define TREEVIEW_MOD_H_

#include <gtkmm.h>
#include "Explorer_popup.h"
#include "Explorer_tooltip.h"

namespace tag {


class Explorer_tree ;

/**
 * @class 		TreeView_mod
 * @ingroup		GUI
 *
 * TreeView specialization for the Explorer_tree.\n
 * Uses TreeModel_Columns class as Gtk::TreeModel::ColumnRecord.\n
 *
 * Provides tooltip (Explorer_tooltip class) and popup menu (Explorer_popup class).
 *
 */
class TreeView_mod : public Gtk::TreeView
{
	public :
		/**
		 * Constructor
		 */
		TreeView_mod() ;
		~TreeView_mod()  ;

		/**
		 * Accessor to the Explorer_popup used by the view.
		 * @return		Pointer on the Explorer_popup
		 */
		Explorer_popup* get_popup() ;

		/**
		 * Accessor to the last iterator the tree popup was launching from.
		 * @return		Pointer on the tree iterator
		 */
		Gtk::TreeIter* get_popup_active_iter()  {return &popup_active_iter ; }

		/**
		 * Set the Explorer_tree that uses the view.
		 * @param model		Pointer on the Explorer_tree using the view
		 */
		void set_modelAG(Explorer_tree* model) { modelAG=model ;}

		/**
		 * Accessor to the Explorer_tree that uses the view.
		 * @return 			Pointer on the Explorer_tree using the view
		 */
		Explorer_tree* get_modelAG() { return modelAG ;}

		/**
		 * Disable tooltip display
		 */
		void disable_tooltip() ;

		/**
		 * Enable tooltip display (default)
		 */
		void enable_tooltip() ;

		/**
		 * Disbale popup menu use
		 */
		void disable_popup() ;

		/**
		 * Enable or disable the past action in popup menu
		 * @param value		True for enabling, False otherwise
		 */
		void enable_popup_paste(bool value) ;

		/**
		 * Set the parent window.
		 * @param w		Pointer on the parent window.
		 */
		void set_window(Gtk::Window* w) ;

		/**
		 * Hide tooltip
		 */
		void hide_tooltip() { tooltip->hide() ;}

		/** Signal emitted when the tree selection changes\n
		 * <b>Gtk::TreeIter* parameter:</b> 	Pointer on the new Gtk::TreeIter
		 * @return								The corresponding signal
		 */
		sigc::signal<void, Gtk::TreeIter*>& signalSelection() { return m_signalSelection; }


	private :
		Explorer_tree* modelAG ;
		Gtk::Window* window ;

		Explorer_popup popup ;
		Explorer_tooltip* tooltip ;

		//Glib::ustring popup_active_file ;
		Gtk::TreeIter popup_active_iter ;
		Gtk::TreeIter selection_active_iter ;
		Gtk::TreeIter tooltip_active_iter ;

		bool popup_enabled ;
		bool tooltip_enabled ;

		virtual void on_cursor_changed() ;
		virtual bool on_focus_out_event(GdkEventFocus* event) ;
		virtual bool on_button_press_event(GdkEventButton *event) ;
		virtual bool on_motion_notify_event(GdkEventMotion *event) ;
		virtual bool on_leave_notify_event(GdkEventCrossing* event) ;
		virtual bool on_key_press_event(GdkEventKey* event) ;

		virtual void on_hide();

//		void prepare_tooltip(Gtk::TreeModel::iterator iter, GdkEventMotion* event) ;

		//signal emitted each time a row is selected (simple click)
		//pass iter
		sigc::signal<void, Gtk::TreeIter*> m_signalSelection ;
} ;

} //namespace

#endif /*TREE_VIEW_H_*/

