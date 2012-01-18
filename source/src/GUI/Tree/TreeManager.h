/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef TREEMANAGER_H_
#define TREEMANAGER_H_
//#include "FtpData.h"
#include "Explorer_tree.h"
#include "Configuration.h"
#include "TreeView_mod.h"
#include <iostream>
#include <gtkmm.h>
#include "Common/widgets/ProgressionWatcher.h"

namespace tag {

/**
 * @class 		TreeManager
 * @ingroup		GUI
 *
 * Trees controller used by top level widget
 *
 * Enables communication between all created trees such as drag 'n drop,
 * selection, ...
 */

class TreeManager
{
	public:
		/**
		 * Constructor
		 * @param window	Pointer on parent window
		 * @param config	Pointer on Configuration object
		 */
		TreeManager(Gtk::Window* window, Configuration* config/*, FtpData* FTPdata*/);
		virtual ~TreeManager();

		/**
		* Add a new tree
		* @param path 			Path of the folder root
		* @param display 		Name displayed in the root node
		* @param rootType 		Type of tree\n
		* 							1: (deprecated)
		* 							2: system tree
		* 							3: project tree
		* 							6: shortcut tree root
		* @param enable_dnd 	Enable drag and drop
		* @return				Pointer on the newly created Explorer_tree\n
		* 						Returns NULL if path couldn't be found.
		*
		*/
		Explorer_tree* add_tree(Glib::ustring path, Glib::ustring display,
								int rootType, bool enable_dnd) ;

		/**
		 * Add a shortcut tree.
		 * @param path		Target directory path
		 * @param display	Shortcut name
		 * @return			Pointer on the newly created Explorer_tree.\n
		 * 					Returns NULL if path couldn't be found.
		 * @note			Doesn't check if similar shortcut exists yet, must be done in top level layer)
		 */
		Explorer_tree* add_shortcutTree(Glib::ustring path, Glib::ustring display) ;

		/**
		 * Accessor to the tree corresponding to given stamp
		 * @param number	Stamp of the tree
		 * @return			A pointer on the corresponding Explorer_tree, or NULL if unexistant.
		 */
		Explorer_tree* get_tree(int number) ;

		/**
		* Remove a tree.
		* @param number	Stamp of the tree
		*/
		int remove_tree(int number) ;

		/**
		* Apply file filter on all trees for refreshing display.
		* @see	Explorer_tree::refilter()
		*/
		void refilter_all() ;

		/**
		 * Indicates the tree node iterator on which the copy action has been proceeded.\n
		 * Initiate copy mechanism flags.
		 * @param src		Last copy target (Gtk::TreeIter)
		 */
		void set_copy(Gtk::TreeIter src);

		/**
		 * Indicates the tree node iterator on which the cut action has been proceeded.\n
		 * Initiate cut mechanism flags.
		 * @param src		Last copy target (Gtk::TreeIter)
		 */
		void set_cut(Gtk::TreeIter src);

		/**
		 * Indicates the tree node iterator on which the cut action has been proceeded.\n
		 * Proceeds paste action if copy or cut action has been initiate
		 * @param dest		Paste target (Gtk::TreeIter)
		 */
		void paste(Gtk::TreeIter dest) ;

		/**
		 * Accessor to the last drag'ndrop source.
		 * @return		The last iterator the drag n' drop action has been initiated from
		 */
		Gtk::TreeIter get_drag_src() {return drag_src ;}

		/**
		 * Indicates if a drag 'n drop action has been initiated.
		 * @return		True if a drag n' drop source has been referenced.
		 */
		bool is_dragdrop_initiated() {return drag_drop_initiated ;}

		/**
		 * Reinitialize drag' n drop status.
		 */
		void dragdropdone() { drag_drop_initiated = false ;}

		/**
		 * Accessor to the Explorer_tree drag n' drop target.
		 * @return		Vector of all Gtk::TargetEntry allowed with Explorer_tree
		 */
		Gtk::TargetEntry get_dragDropTarget() { return dragdropTarget ;}

		/**
		 * Accessor to the Explorer_tree drag n' drop target list.
		 * @return		Vector of all Gtk::TargetEntry allowed with Explorer_tree
		 */
		const std::vector<Gtk::TargetEntry>& get_dragDropTargetList() { return dragdropTargetList ;}

		/**
		 * Accessor to the Explorer_tree that has the selection.
		 * @return		Pointer on Explorer_tree the last selected row belongs to
		 */
		Explorer_tree* get_last_selected() { return trees[last_selected_tree] ; }

		/**
		 * Set the last selected row
		 * @param iter			Gtk::TreeIter of the last selected node
		 * @param tree_number	Number of the tree the given iterator belongs to
		 */
		void set_last_selected(Gtk::TreeIter* iter, int tree_number) ;

		/**
		 * Indicates wether one of the Explorer_tree has an active selection.
		 * @return		True if one of the Explorer_tree views has a selected row
		 */
		bool has_selection() ;

		/**
		 * Indicates wether one of the Explorer_tree has focus.
		 * @return		True if one of the Explorer_tree views has focus
		 */
		bool has_focus() ;

		/**
		 * Refreshes display by adding new file row
		 * @param file		Path of the new file to be displayed
		 * @note 			Can be improved
		 */
		void display_new_file(Glib::ustring file) ;

		/**
		 * Modify root node target of a tree.\n
		 * Mainly used for Explorer_tree with <em>shortcut type</em>.
		 * @param window	Pointer on parent window
		 * @param tree		Pointer on the Explorer_tree whose target will be changed
		 * @param config	Pointer on application Configuration object
		 */
		void change_target_tree(Gtk::Window* window, Explorer_tree* tree, Configuration* config) ;

		/**
		 * Remove the given Explorer_tree from the TreeManager
		 * @param tree		Pointer on the Explorer_tree to be removed
		 */
		void delete_tree(Explorer_tree* tree) ;

		/**
		 * Check if one of the Explorer_tree instances referenced in the manager
		 * corresponds to the given path.
		 * @param path						Directory path to be checked
		 * @param excepted_root_number		Number of an Explorer_tree we want to exclude from the check.
		 * 									Useful when modifying
		 * @return
		 */
		bool exist_tree(Glib::ustring path, int excepted_root_number) ;

		/**
		 * Deprecated feature
		 * @param path	File path
		 * @deprecated	Not used anymore.
		 */
		void upload_file(Glib::ustring path) ;

		/**
		 * Force all Explorer_tree displayed tooltips to be hidden.
		 */
		void hide_tooltips() ;

		/**
		 * Accessor to the ProgressionWatcher instance of the TreeManager.
		 * @return		Pointer to the ProgressionWatcher of the manager
		 */
		ProgressionWatcher* get_progressWatcher() { return progressWatcher ;}

		/**
		 * Call the row activated signal callback for given tree
		 * @param path		Path of the row to activate
		 * @param column	Model column record used by the tree
		 * @param tree		Pointer to the concerned Explorer_tree
		 */
		void activate_tree_row(const Gtk::TreeModel::Path &path, const Gtk::TreeViewColumn* column, Explorer_tree* tree) ;

		/**
		 * Signal emitted when a row is activated.
		 * @return		The corresponding signal
		 */
		sigc::signal<void,Glib::ustring,Glib::ustring>& signalOpenFileRequired() { return m_signalOpenFileRequired; }


	private:
		Gtk::Window* window ;
		Explorer_filter* filter ;
		Configuration* config ;
//		FtpData* FTPdata ;
		ProgressionWatcher* progressWatcher ;

	    /* stock all tree : mapping <indice, tree> */
		std::map<int, Explorer_tree*> trees ;
	    /* list of targetEntry to define drag and source identity */
		std::vector<Gtk::TargetEntry> dragdropTargetList ;
	    /* type of drag and source object */
		Gtk::TargetEntry dragdropTarget ;
		/* unique identifier of file */
		int current_number ;
		/* last selection tree */
		int last_selected_tree ;

		virtual void on_drag_data_get (const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& selection_data, guint info, guint time, TreeView_mod* view) ;
		bool on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time, TreeView_mod* view) ;
		bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time, TreeView_mod* view) ;

		/* drag and drop flag */
		bool drag_drop_initiated ;
		Gtk::TreeIter drag_src ;
		Gtk::TreeIter drag_dest ;

		/* copy/cut/past flags*/
		bool copy_initiated ;
		bool cut_initiated ;

		Gtk::TreeIter cc_src ;
		Gtk::TreeIter cc_dest ;

		void move_file(Gtk::TreeIter drag_src, Gtk::TreeIter drag_dest, bool filtered) ;
		void copy_file(Gtk::TreeIter* src, Gtk::TreeIter* directory_dest, bool filtered) ;

		//Gtk::TreePath* get_node_by_name(Gtk::TreePath path, Glib::ustring name, Explorer_tree* tree) ;

		void save_shortcuts(Glib::ustring path) ;
		int get_rootNumber_from_file(Glib::ustring file) ;
		void on_cursor_tree_changed(Gtk::TreeIter* iter, int tree_number) ;
		void actualize_selection(int tree_number) ;

		sigc::signal<void,Glib::ustring,Glib::ustring> m_signalOpenFileRequired ;

		void on_tree_row_activated(const Gtk::TreeModel::Path &path, const Gtk::TreeViewColumn* column, Explorer_tree* tree) ;
		void on_tree_row_expanded(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path, Explorer_tree* tree) ;
		bool treeModelFilter_callback(const Gtk::TreeIter& iter) ;
};

} //namespace


#endif /*TREEMANAGER_H_*/
