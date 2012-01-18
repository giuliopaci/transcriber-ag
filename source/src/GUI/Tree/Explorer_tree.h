/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef EXPLORER_TREE_H_
#define EXPLORER_TREE_H_

#include <gtkmm.h>
#include "TreeView_mod.h"
#include "TreeModel_Columns.h"
#include "Common/Explorer_filter.h"
//#include "FtpData.h"
#include "Configuration.h"

namespace tag {

class TreeManager ;
/**
 * @class 		Explorer_tree
 * @ingroup		GUI
 *
 * Class representing a file explorer tree.\n
 * Uses a ModelTree (filtered and sorted) and a specific TreeView (TreeView_mod class).
 * Provides popup and tooltip on explorer rows.
 *
 * The model of the tree follows the following chain :
 * 		TreeStore ->  TreeModelFilter  -> TreeModelSort
 */
class Explorer_tree
{
	public :
		/**
		 * Constructor
		 * @param window	Pointer on parent window
		 * @param manager	Pointer on parent controller
		 */
		Explorer_tree(Gtk::Window* window, TreeManager* manager) ;
	 	virtual ~Explorer_tree() ;

		//> Getter
	 	/**
	 	 * Accessor to the tree view.
	 	 * @return		Pointer on the TreeView_mod used
	 	 */
	 	TreeView_mod* get_view() { return &view ; }

	 	/**
	 	 * Accessor to the TreeModel_Columns used.
	 	 * @return
	 	 */
		TreeModel_Columns* get_model_Columns() { return &m_Columns ; }

		/**
		 * Accessor to the basic model tree.
		 * @return		Pointer on the Gtk::TreeStore tree used
		 */
		Glib::RefPtr<Gtk::TreeStore> get_refModelTree() { return refModelTree ;}

		/**
		 * Accessor to the filtered model tree.
		 * @return		Pointer on the Gtk::TreeModelFilter used
		 */
		Glib::RefPtr<Gtk::TreeModelFilter> get_refFilteredModelTree() { return refFilteredModelTree; }

		/**
		 * Accessor to the sorted model tree.
		 * @return		Pointer on the Gtk::TreeModelSort used
		 */
		Glib::RefPtr<Gtk::TreeModelSort> get_refSortedModelTree() { return refSortedModelTree; };

		/**
		 * Accessor to the last iterator the tree popup was launching from.
		 * @return		Pointer on the tree iterator
		 */
		Gtk::TreeIter* get_popup_active_iter() { return view.get_popup_active_iter() ; }

		/**
		 * Accessor to the Explorer_popup instance used by the view
		 * @return		Pointer on the Explorer_popup used
		 */
		Explorer_popup* get_popup() { return view.get_popup() ;}

		/**
		 * Fill the given tree node
		 * @param iterator		Gtk::Iterator pointing on the node to be filled
		 * @param isFiltered 	True if iterator passed as first parameter belongs
		 *					 	to the tree model filtered
		 */
		int fill_node(Gtk::TreeIter iterator, bool isFiltered) ;

		/**
		 * Fill the top node of the tree
		 * @param path 			Path of the directory to be represented as the tree
		 * @param display 		Name of the tree to be displayed
		 * @param rootType 		Code of the tree type
		 * @param treeStamp 	Unique identifier of the tree
		 * @see					get_rootType()
		 */
		void fill_root_node(Glib::ustring path, Glib::ustring display,
								int rootType, int treeStamp) ;

		/**
		 * Remove a file from the file-system and actualize display
		 * @param sorted_iterator 		Gtk::TreeIter pointing on the node of the <b>sorted</b> model to remove
		 * @param window 				Pointer on the Explorer_tree parent window
		 * @return  					1 if removed, 0 if user canceled, -1 if file-system error
		 */
		int remove_file(Gtk::TreeIter* sorted_iterator, Gtk::Window* window) ;

		/**
		 * Rename a file in the file-system and actualize display.\n
		 * Display a dialog that get and check the new name to apply.\n
		 * @param sorted_iterator 		Gtk::TreeIter pointing on the node of the <b>sorted</b> model to rename
		 * @param window 				Pointer on the Explorer_tree parent window
		 * @return 						1 if renamed, 0 if user cancels, -1 if renaming from filesystem problem
		 */
		int rename_file(Gtk::TreeIter* sorted_iterator, Gtk::Window* window) ;

		/**
		 * Insert a directory in the file-system and the tree widget.\n
		 * Display a dialog that get and check the new directory name.
		 * @param sorted_iterator 		Gtk::TreeIter pointing on the node of the <b>sorted</b> model into which the
		 * 								directory will be created
		 * @param window 				Pointer on the Explorer_tree parent window
		 * @return  1 if created, -1 if not a directory, -2 if problem
		 */
		int create_directory(Gtk::TreeIter* sorted_iterator, Gtk::Window* window) ;

		/**
		 * Compute the path of the file corresponding to the given iterator
		 * @param iterator				Gtk::TreeIter pointing on the node of which the name is wanted
		 * @param isSortedFiltered		True if the given iterator belongs to the <em>sorted</em> model, False
		 * 								if it belongs to the parent model.
		 * @return						Path of the pointed file
		 */
		Glib::ustring compute_path_from_node(Gtk::TreeIter iterator, bool isSortedFiltered) ;

		/**
		 * Force the Refiltering of the tree using the Explorer_filter class.
		 */
		void refilter() ;

		/**
		 * Getter / Setter for loaded
		 * loaded flag is usefull for expand a row
		 */

		/**
		 * Accessor to the loaded status\n
		 * @return		True if the node is already being loaded, False otherwise
		 * @note		As expand action loads the expanded node, this loaded status is used
		 * 				to lock the loading action and enables a simple expand.
		 */
		bool get_loaded() {return loaded ;}

		/**
		 * Set the loaded status.
		 * @param value		Value of the loaded status
		 */
		void set_loaded(bool value) {loaded = value ;}

		/**
		 * Accessor to the file path of the first node.
		 * @return		The file path of the top level node
		 */
		Glib::ustring get_rootPath() { return rootPath ;}

		/**
		 * Accessor to the Explorer_tree type.
		 * @note	Tree type codes:\n
		 * 				2: system tree root\n
		 * 				3: project tree root\n
		 * 				6: shortcut tree root\n
		 * @return	Code of the Explorer_tree type.
		 */
		int get_rootType() { return rootType ;}

		/**
		 * Accessor to the tree stamp (unique identifier)
		 * @return		The identifier of the tree.
		 * @note 		Mostly used by the TreeManager
		 */
		int get_rootNumber() { return rootNumber ;}

		/**
		 * Accessor to the tree display name
		 * @return		Name used for displaying in the TreeView_mod
		 */
		Glib::ustring get_rootDisplay() { return rootDisplay ;}

		/**
		 * Stupid accessor for stupid separator packed with each tree
		 * @return		Horizontal separator
		 * @note		Shouldn't be
		 */
		Gtk::HSeparator* get_separator() { return &separator; }

		/**
		 * Disconnect the sorted model and the view
		 * @deprecated Don't use this method
		 */
		void disconnect_view() ;

		/**
		 * Disconnect the sorted model and the view
		 * @deprecated Don't use this method
		 */
		void connect_view() ;

		/**
		 * Initialize the Explorer_tree characteristic
		 * @param root_path		File path of the tree root
		 * @param root_type		Type of the tree
		 * @param root_number	Unique identifier of the tree
		 * @param display		Name to be display in the tree view
		 */
		void set(Glib::ustring root_path, int root_type, int root_number, Glib::ustring display)
		{
			rootPath = root_path ;
			rootType = root_type ;
			rootNumber = root_number ;
			rootDisplay = display ;
		}

		/**
		 * Accessor to the last selected node
		 * @return		Last selected Gtk::TreePath of the <b>basic</b> model.
		 */
		Gtk::TreePath get_child_selected() { return child_selected ;}

		/**
		 * Change target directory of the root node.
		 * @param path		New directory node
		 * @param display	New display name
		 * @param config	Application Configuration instance
		 */
		void change_target_tree(Glib::ustring& path, Glib::ustring& display, Configuration* config) ;

		/**
		 * Refresh the content of the node of the given file path
		 * @param dir_wt_root		File path of the directory to be refreshed
		 */
		void refresh_directory(Glib::ustring& dir_wt_root) ;

		/**
		 * Refresh the content of the node pointed by the given iterator
		 * @param iter 					Gtk::TreeIter pointing to the node
		 * @param isSortedFiltered		True if the given iterator belongs to the <em>sorted</em> model, False
		 * 								if it belongs to the parent model.
		 * @param expand 				True for expanding the node
		 */
		void refresh_directory(Gtk::TreeIter iter, bool isSortedFiltered, bool expand) ;

		/**
		 * Clear the tree selection
		 */
		void clear_selection() ;

		/**
		 * Convert the given sorted iterator (sorted model) to a child iterator (basic model)
		 * @param sortedIter		Gtk::Iterator of the <b>sorted</b> model
		 * @return					Gtk::Iterator of the <b>basic</b> model
		 */
		Gtk::TreeIter convert_iter(Gtk::TreeIter sortedIter) ;

		/**
		 * Convert the given child iterator (basic model) to a sorted iterator (sorted model)
		 * @param iter				Gtk::Iterator of the <b>basic</b> model
		 * @return					Gtk::Iterator of the <b>sorted</b> model
		 */
		Gtk::TreeIter sort_filter_iter(Gtk::TreeIter iter) ;

		/**
		 * Reset the tree characteristic with new values
		 * @param path		New root path
		 * @param display	New display
		 * @param rootType	New tree type
		 */
		void reset_tree(Glib::ustring path, Glib::ustring display, int rootType) ;

		/**
		 * Defines the target list for the explorer tree
		 * @param targetList		List of drag and drop target
		 * @note	It will be used only if drag is enabled for current tree
		 */
		void setDragAndDropTarget(std::vector<Gtk::TargetEntry> targetList) ;

	private:
		Gtk::Window* window ;
		Gtk::HSeparator separator ;
		Explorer_filter* filter ;
		TreeManager* manager ;

		/** Models and view */
		TreeModel_Columns m_Columns ;
		TreeView_mod view ;
		Glib::RefPtr<Gtk::TreeStore> refModelTree ;
		Glib::RefPtr<Gtk::TreeModelFilter> refFilteredModelTree ;
		Glib::RefPtr<Gtk::TreeModelSort> refSortedModelTree ;

		Gtk::TreeViewColumn* pColumn ;
		Gtk::CellRendererText* cellRend_blank ;
		Gtk::CellRendererPixbuf* cellRend_ico ;
		Gtk::CellRendererText* cellRend_name ;
		Gtk::CellRendererText* cellRend_nbFiltered ;

		Gtk::TreePath child_selected ;

		/* Used for mutliple pass in expanded row callback */
		bool loaded ;

		/* Path of the file that the tree represents */
		Glib::ustring rootPath ;

		/* Root type:
		* 2-> filesystem folder
		* 3-> project tree folder
		* 6-> shortcut folder
		*/
		int rootType ;

		/* Unique identifier for the tree */
		int rootNumber ;

		/* Name displayed on GUI */
		Glib::ustring rootDisplay ;

		/**
		 * Fill the first row of a node
		 * @param it 			Gtk::Iterator pointing on the file to be filled with one row
		 * @param isFiltered 	True if iterator belongs to a filtered tree
		 * @note 				Useful to display the expander on a directory row
		 */
		void fill_node_first_row(Gtk::TreeIter it, bool isFiltered);

		void selection_has_changed(Gtk::TreeIter* sorted_iter) ;

		Glib::ustring find_treeView_path(Glib::ustring& dir_wt_root) ;

		bool FTP_download_info_audio_file(Glib::ustring& audio_file_path, std::vector<Glib::ustring>& results) ;

		int sort_tree(const Gtk::TreeModel::iterator& it1, const Gtk::TreeModel::iterator& it2) ;

		void actualize_filtered_files(Gtk::TreePath& father_path) ;

		/**
		 * Given a file path and a tree model, returns the string representation of the
		 * corresponding Gtk::TreePath
		 * @param dir_wt_root	File path searched
		 * @param model			Gtk::TreeModel into which searching the path
		 * @return				String representation of the Gtk::TreePath, or empty value
		 * 						if no path could be found.
		 */
		Glib::ustring find_treeView_path(Glib::ustring& dir_wt_root, Glib::RefPtr<Gtk::TreeModel> model) ;

		/**
		 * Expand the node corresponding to the given tree path
		 * @param treeView_path_str		String representation of the
		 * @param set_cursor
		 */
		void expand_node(Glib::ustring& treeView_path_str, bool set_cursor) ;

		/* list of targetEntry to define drag and source identity */
		std::vector<Gtk::TargetEntry> target_list ;
} ;

} //namespace


#endif
