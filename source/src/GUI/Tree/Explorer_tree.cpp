/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Explorer_tree.h"
#include "Explorer_utils.h"
#include "Explorer_fileHelper.h"
#include "Explorer_dialog.h"
#include "TreeManager.h"

#include "Common/FileInfo.h"
#include "Common/util/Utils.h"
#include "Common/util/FileHelper.h"
#include "Common/icons/Icons.h"
#include "Common/widgets/GtUtil.h"

#include <stdlib.h>

namespace tag {


//******************************************************************************
//									CONSTRUCT
//******************************************************************************

Explorer_tree::Explorer_tree(Gtk::Window* win, TreeManager* _manager)
{
	pColumn = NULL ;
	cellRend_ico = NULL ;
	cellRend_name = NULL ;
	cellRend_nbFiltered = NULL ;

	filter = Explorer_filter::getInstance() ;
	window = win ;
	manager = _manager ;

	refModelTree = Gtk::TreeStore::create(m_Columns) ;
	refFilteredModelTree = Gtk::TreeModelFilter::create(refModelTree) ;
	refSortedModelTree = Gtk::TreeModelSort::create(refFilteredModelTree) ;

	view.set_model(refSortedModelTree) ;
	view.set_modelAG(this) ;
	view.set_window(window) ;
	refSortedModelTree->set_default_sort_func(sigc::mem_fun(*this, &Explorer_tree::sort_tree)) ;

	loaded = false ;
	rootNumber = -1 ;
	rootPath = "" ;
	rootType = -1 ;

	//> prepare view
	view.set_enable_search(true) ;
	view.set_search_column(m_Columns.m_file_name) ;
	view.set_headers_visible(false) ;
	view.set_rules_hint(false) ;

	//> Choose the displayed columns
	int number ;
	pColumn = Gtk::manage(new Gtk::TreeViewColumn(M_COL_FILE_ICO)) ;
	cellRend_ico = Gtk::manage(new Gtk::CellRendererPixbuf()) ;
	cellRend_name = Gtk::manage(new Gtk::CellRendererText()) ;
	number = view.append_column(*pColumn) ;

	if (pColumn) {
		pColumn->pack_start(*cellRend_ico, false);
		pColumn->pack_start(*cellRend_name, false);

		pColumn->add_attribute(cellRend_ico->property_pixbuf(), m_Columns.m_file_ico) ;
		pColumn->add_attribute(cellRend_ico->property_pixbuf_expander_open(), m_Columns.m_file_ico2) ;
		pColumn->add_attribute(cellRend_ico->property_pixbuf_expander_closed(), m_Columns.m_file_ico) ;
		pColumn->add_attribute(cellRend_ico->property_is_expander(), m_Columns.m_file_isExpander) ;

		pColumn->add_attribute(cellRend_name->property_text(), m_Columns.m_file_display_wNumber) ;
		pColumn->add_attribute(cellRend_name->property_weight(), m_Columns.m_file_displayWeight) ;

	}
	view.signalSelection().connect(sigc::mem_fun(*this, &Explorer_tree::selection_has_changed)) ;
};


Explorer_tree::~Explorer_tree()
{
	//refSortedModelTree->unset_default_sort_func() ;
	//> disconnect view
	view.unset_model() ;
	//> clear model
	refModelTree.clear() ;

	if (pColumn)
		delete(pColumn) ;
	if (cellRend_ico)
		delete(cellRend_ico) ;
	if(cellRend_name)
		delete(cellRend_name) ;
	if(cellRend_nbFiltered)
		delete(cellRend_nbFiltered) ;
}


//**************************************************************************
//									METHODS
//**************************************************************************


/**
 *  Remove file pointed by the iterator of the filtered tree
 *  Remove file in filesystem, in treeModel and actualise treeView
 *  Display a message dialog to confirm the delete
 *	1 ok, -1 remove error, -2 file opened
 */
int Explorer_tree::remove_file(Gtk::TreeIter* sorted_iterator, Gtk::Window* window)
{
	int ret ;

	//> get the iterator in the base model
	Gtk::TreeIter iterator = convert_iter(*sorted_iterator) ;

	//get corresponding row
	Gtk::TreeRow row = *iterator ;
	TreeModel_Columns m ;

	//path of file
	Glib::ustring path = compute_path_from_node(*sorted_iterator, true) ;
	Glib::ustring text ;
	Glib::ustring name = Glib::path_get_basename(path) ;
	Glib::ustring space = " " ;
	Glib::ustring path_tmp = path ;
	Glib::ustring folder = Glib::path_get_dirname(path_tmp) ;

	if (row[m.m_file_sysType])
		text = _("Are you sure to delete the directory") + space + name + " ?"  ;
	else
		text = _("Are you sure to delete the file ")+ space + name + " ?" ;

	int res = Explorer_dialog::msg_dialog_question(text, window, true, "") ;

	if (res==Gtk::RESPONSE_YES)
	{
		//PREPARE PROGRESS BAR
		Glib::ustring label =  Glib::ustring(_("Removing")) + " " + name ;
		Glib::ustring legend = Glib::ustring("FROM: ") + folder ;
		ProgressionWatcher::Entry* entry = manager->get_progressWatcher()->add_entry(label, path, legend, false) ;
		Gtk::ProgressBar* progress = Glib::wrap(entry->get_pbar());
		manager->get_progressWatcher()->show() ;

		int ok = FileHelper::remove_from_filesystem(path, progress) ;
		//> remove from model
		if ( ok>=0 ) {
			// keep father path
			Gtk::TreePath father_path = refModelTree->get_path(iterator) ;
			father_path.up() ;
			// erase
			refModelTree->erase(iterator) ;
			refilter() ;
			ret = 1 ;
			entry->complete_bar() ;
			entry->set_status(true, _("Removed")) ;
			entry->get_bar()->set_text(name) ;
			// update father
			actualize_filtered_files(father_path) ;
		}
		else {
			Glib::ustring txt ;
			if (ok==-20) {
					txt = _("Permissions problem on file, target directory or source directory") ;
			}
			else if (ok==-41)
					txt = _("Can't find the file or directory to be removed") ;
			else
					txt = _("Removing problem") ;
			entry->set_status(true, txt) ;
			entry->get_bar()->set_text(name) ;
			ret = -1 ;
		}
	}
	else {
		TRACE << "Explorer_tree::remove_file:> canceled" <<  std::endl ;
		ret = 0 ;
	}
	return ret ;
}


/**
 *  Rename file pointed by the iterator of the filtered tree
 *  Rename file in filesystem, in treeModel and actualize treeView
 *  Display a message dialog to get the new name, and check it
 */
int Explorer_tree::rename_file(Gtk::TreeIter* sorted_iterator, Gtk::Window* window)
{
	int res ;

	//> get corresponding iter to model
	Gtk::TreeIter iterator = convert_iter(*sorted_iterator) ;

	Gtk::TreeRow row = *iterator ;

	TreeModel_Columns m ;

	//path of file
	Glib::ustring path = compute_path_from_node(*sorted_iterator, true) ;

	//path of directory
	Glib::ustring dir = Glib::path_get_dirname(path) ;

	Glib::ustring new_name = Explorer_dialog::msg_rename(window, path) ;

	//> if user has canceled do nothing
	if (new_name=="") {
		TRACE << "Explorer_tree:> rename_file:> canceled" <<  std::endl ;
		res = 0 ;
	}
	else {
		//> RENAME A FILE
		//construct new path
		Glib::ustring new_path = FileHelper::build_path(dir, new_name) ;
		//> rename in file system
		int ok = FileHelper::rename_in_filesystem(path, new_path) ;
		//> rename in model
		if ( ok == 0 ) {
			// keep father path
			Gtk::TreePath father_path = refModelTree->get_path(iterator) ;
			father_path.up() ;
			//rename in model
			row[m.m_file_name] = new_name ;
			row[m.m_file_display] = new_name ;
			row[m.m_file_display_wNumber] = new_name + " " + row[m.m_file_nbFilteredFiles] ;
			res = 1 ;
			refilter() ;
			// update father
			actualize_filtered_files(father_path) ;
		}
		else {
			Glib::ustring txt ;
			if (ok==-20) {
					txt = _("Permissions problem on file, target directory or source directory") ;
			}
			else if (ok==-41)
					txt = _("Can't find the source file or source directory") ;
			else
					txt = _("Renaming problem") ;
			Explorer_dialog::msg_dialog_error(txt, window, true) ;
			res = -1 ;
		}
	}
	return res ;
}

//******************************************************************************
//									DIRECTORIES
//******************************************************************************


/*
 * Return 1 if created, -1 if not a directory, -2 if problem
 */
int Explorer_tree::create_directory(Gtk::TreeIter* sorted_iterator, Gtk::Window* window)
{
	int ok ;

	//> get corresponding iter to model
	Gtk::TreeIter iterator = convert_iter(*sorted_iterator) ;
	Gtk::TreeModel::Row row = *iterator ;
	Gtk::TreeIter new_iter ;
	Gtk::TreeRow new_row ;

	TreeModel_Columns m ;
	//path of father directory
	Glib::ustring path = compute_path_from_node(*sorted_iterator, true) ;

	//> cannot create a directory on a file !
	if(!row[m.m_file_sysType])
		return -1 ;

	Glib::ustring new_folder_path = FileHelper::build_path(path,"New_folder") ;
	Glib::ustring new_name = Explorer_dialog::msg_rename(window, new_folder_path) ;

	//> if user has cancelled, do nothing
	if (new_name=="") {
		TRACE << "Explorer_tree:> create_directory:> canceled" <<  std::endl ;
	}
	else {
		//> get new path
		new_folder_path = FileHelper::build_path(path,new_name) ;
		//TODO complete filesystem creation
		ok = FileHelper::create_directory(new_folder_path) ;
		//> fill model
		if (ok>=0) {
			new_row = * ( refModelTree->append(row.children()) ) ;
			TreeModel_Columns::fill_row(&new_row, new_name, new_name, "nothing", ICO_TREE_DIR, ICO_TREE_DIR2, true, (*iterator)[m.m_file_root], (*iterator)[m.m_file_rootType]) ;
			refresh_directory(*sorted_iterator, true, true);
			refilter() ;
		}
		else {
			Glib::ustring txt ;
			if (ok==-20) {
					txt = _("This directory is not writable") ;
			}
			else
					txt = _("Problem while creating directory, check the permissions") ;
			Explorer_dialog::msg_dialog_error(txt, window, true) ;
		}
	}
	return ok ;
}

void Explorer_tree::refresh_directory(Glib::ustring& dir_wt_root)
{
	//if directory is root
	if (dir_wt_root==" " || dir_wt_root=="") {
		Glib::ustring path = "0" ;
		expand_node(path, true) ;
	}
	else {
		Glib::ustring treeView_path = find_treeView_path(dir_wt_root, refSortedModelTree) ;
		if (treeView_path!="")
			expand_node(treeView_path, true) ;
		else
			Explorer_utils::print_trace("Explorer_tree::refresh_directory:> PATH NOT FOUND", 1) ;
	}
}

void Explorer_tree::refresh_directory(Gtk::TreeIter iterator, bool isFiltered, bool expand)
{
	expand = true ;

	TreeModel_Columns m ;
	Gtk::TreeIter iter ;
	Gtk::TreeIter base_iter ;
	//> only do it on filtered iterator, so convert it if it's not
	if (isFiltered) {
		iter = iterator ;
		base_iter = convert_iter(iterator) ;
	}
	 else {
		iter = sort_filter_iter(iterator) ;
		base_iter = iterator ;
	}

	bool was_empty = false ;

	//> if no children, fill node
	if ( (iter->children()).size()==0 )
	{
		int nbfiltered ;
		nbfiltered = fill_node(iter, true) ;
		(*base_iter)[m_Columns.m_file_nbFilteredFiles] = "("+ number_to_string(nbfiltered)+")" ;
		(*base_iter)[m_Columns.m_file_display_wNumber] = (*base_iter)[m_Columns.m_file_display] + "  "+ (*base_iter)[m_Columns.m_file_nbFilteredFiles] ;
		refilter() ;
		iter = sort_filter_iter(base_iter) ;
		was_empty = true ;
	}//end of empty node

	if (expand) {
		Gtk::TreePath path = refSortedModelTree->get_path(iter) ;
		Glib::ustring pathstr = path.to_string() ;
		set_loaded(was_empty) ;
		expand_node(pathstr, true) ;
		set_loaded(false) ;
	}
}

//******************************************************************************
//									FILL TREE
//******************************************************************************


void Explorer_tree::fill_root_node(Glib::ustring path, Glib::ustring display,
								   int rootType, int rootNumber)
{
	TreeModel_Columns m ;
	//Explorer_utils::print_trace("Explorer_tree::fill_root_node START ", rootType, 1) ;

	Gtk::TreeIter iter = refModelTree->append() ;
	Gtk::TreeRow row = *iter ;
	Glib::ustring ico_path ;

	if (rootType!=4) {
		ico_path = filter->switch_ico(path, rootType) ;
		TreeModel_Columns::fill_row(&row, path, display, "nothing", ico_path, "",  rootType, rootNumber, rootType) ;
		//Explorer_utils::print_trace("Explorer_tree::fill_root_node PREPARE NODE", 1) ;
		fill_node(iter, false) ;
	}
}



int Explorer_tree::fill_node(Gtk::TreeIter it, bool isFiltered)
{
	TreeModel_Columns m ;
	Gtk::TreeIter iterator ;
	Gtk::TreeIter filtered ;

	//> Compute path of node
	if (isFiltered)
		iterator = convert_iter(it) ;
	else
	{
		iterator = it ;
		filtered = sort_filter_iter(it) ;
	}
	Glib::ustring path = compute_path_from_node(iterator, false) ;

	//> create row for treeStore
	Gtk::TreeModel::Row row ;
	Gtk::TreeIter iter ;
	Gtk::TreeIter child_iter ;
	Glib::ustring leaf_name ;

	int nb_filtered_files = 0  ;

	//> Fill only directory with conditions: readable & executable
	if ( Glib::file_test(path, Glib::FILE_TEST_IS_DIR)
			&& FileHelper::is_executable(path)
			&& FileHelper::is_readable(path) )
	{
		//> Don't fill with directories to which users can't have access
		try
		{
			Glib::Dir dir(path) ;
			//> for each files contained in it do reccurence
			while ( (leaf_name=dir.read_name()) != "" )
			{
				Glib::ustring new_path = FileHelper::build_path(path, leaf_name) ;

				//> append to tree
				child_iter = refModelTree->append((*iterator).children()) ;
				row = *child_iter ;

				//> system type
				int sysType = 0 ;
				if (Glib::file_test(new_path, Glib::FILE_TEST_IS_DIR))
					sysType = 1 ;

				//> icon
				Glib::ustring ico_path = filter->switch_ico(new_path, -1) ;
				Glib::ustring ico_path2 = "" ;
				if (sysType==1)
					ico_path2 = ICO_TREE_DIR2 ;

				// fill the row
				TreeModel_Columns::fill_row(&row, leaf_name, leaf_name, "", ico_path, ico_path2, sysType, (*iterator)[m.m_file_root], (*iterator)[m.m_file_rootType]) ;
				if (sysType==1)
					fill_node_first_row(child_iter, false) ;

				// number of file
				Explorer_filter* filter = Explorer_filter::getInstance() ;
				if ( filter->is_import_annotation_file(new_path) )
					nb_filtered_files++ ;

				// Refresh GUI
				if (view.is_visible())
					GtUtil::flushGUI(true, false) ;
			}
			dir.close() ;
		}
		catch (Glib::FileError e)
		{
		}
	}

	return nb_filtered_files ;
}

void Explorer_tree::fill_node_first_row(Gtk::TreeIter it, bool isFiltered)
{
	TreeModel_Columns m ;
	Gtk::TreeIter iterator ;

	//> Compute path of node
	if (isFiltered)
		iterator = convert_iter(it) ;
	else
		iterator = it ;
	Glib::ustring path = compute_path_from_node(iterator, false) ;

	//> create row for treeStore
	Gtk::TreeModel::Row row ;
	Gtk::TreeIter iter ;
	Gtk::TreeIter child_iter ;
	Glib::ustring leaf_name ;

	//> UGLY HACK: fill only with 1 virtual row to force the expander
	row = *(refModelTree->append((*iterator).children())) ;
	Glib::ustring ico_path = ICO_TREE_DEFAULT ;
	TreeModel_Columns::fill_row(&row, "empty", "empty", "", ico_path, "", -1, (*iterator)[m.m_file_root], (*iterator)[m.m_file_rootType]) ;
}

void Explorer_tree::refilter()
{
	refSortedModelTree->reset_default_sort_func() ;
	refSortedModelTree->set_default_sort_func(sigc::mem_fun(*this, &Explorer_tree::sort_tree)) ;
	refFilteredModelTree->refilter() ;
	//> Gtk bug ?? seems sometimes visibility is set to true, but view isn't updated
	//> do another filtering to force display
	refFilteredModelTree->refilter() ;
}

//******************************************************************************
//										VIEW
//******************************************************************************

void Explorer_tree::disconnect_view()
{
	view.unset_model() ;
}

void Explorer_tree::connect_view()
{
	if (refSortedModelTree)
		view.set_model(refSortedModelTree) ;
}


void Explorer_tree::expand_node(Glib::ustring& treeView_path_str, bool set_cursor)
{
	Glib::ustring path_str = treeView_path_str ;
	Gtk::TreePath* path_tmp = new Gtk::TreePath(treeView_path_str) ;
	get_view()->collapse_row(*path_tmp) ;
	get_view()->expand_to_path(*path_tmp) ;
	if (set_cursor)
		get_view()->set_cursor(*path_tmp) ;
 	if (path_tmp)
 		delete(path_tmp) ;
}

int Explorer_tree::sort_tree(const Gtk::TreeModel::iterator& it1, const Gtk::TreeModel::iterator& it2)
{
	int res_return, res_compare ;
	Glib::ustring tmp1, tmp2 ;
	int t1, t2 ;

	//> get type of iterator 1
	(*(it1)).get_value(M_COL_FILE_SYSTYPE_INDICE, t1) ;
	//> get type of iterator 2
	(*(it2)).get_value(M_COL_FILE_SYSTYPE_INDICE, t2) ;

	//> display folder before file
	if (t1==1 && t2==0)
		return -1 ;
	else if (t1==0 && t2==1)
		return 1 ;
	//> other cases, alphabetic order
	else {

		//get first value
		(*(it1)).get_value(M_COL_FILE_DISPLAY_INDICE , tmp1) ;
		//get second
		(*(it2)).get_value(M_COL_FILE_DISPLAY_INDICE, tmp2) ;

		//compare
		res_compare = tmp1.compare(tmp2) ;

		if (res_compare < 0)
			res_return = -1 ;
		else if (res_compare==0)
			res_return =  0 ;
		else
			res_return = 1 ;
		return res_return ;
	}
}

void Explorer_tree::actualize_filtered_files(Gtk::TreePath& father_path)
{
	Gtk::TreeIter it_model = refModelTree->get_iter(father_path) ;
	Gtk::TreeRow row = *it_model ;
	if (row.get_value(m_Columns.m_file_sysType) == 1)
	{
		Gtk::TreeModel::Children children = it_model->children() ;
		Gtk::TreeIter i = children.begin();
		int cpt = 0 ;
		// check all children
		while ( i!=children.end()) {
			Gtk::TreeIter tmp_iter = i ;
			Gtk::TreeRow row = *i ;
			Glib::ustring name = row.get_value(m_Columns.m_file_name) ;
			if (filter && filter->is_import_annotation_file(name))
				cpt++ ;
			i++ ;
		}
		// actualize father info
		row.set_value(m_Columns.m_file_nbFilteredFiles, Glib::ustring(number_to_string(cpt)) ) ;
		row[m_Columns.m_file_display_wNumber] = row.get_value(m_Columns.m_file_display) + "  (" + row.get_value(m_Columns.m_file_nbFilteredFiles) + ")" ;
	}
}

void Explorer_tree::reset_tree(Glib::ustring path, Glib::ustring display, int rootType)
{
	//> set inforamtion to tree
	set(path, rootType, get_rootNumber(), display) ;

	//> create filtered model and set callback function
	get_refModelTree()->clear() ;
	refilter() ;

	//> fill tree
	fill_root_node(path , display, rootType, get_rootNumber()) ;
}

void Explorer_tree::change_target_tree(Glib::ustring& path, Glib::ustring& display, Configuration* config)
{
	reset_tree(path, display, get_rootType());
	if (get_rootType()==5)
		config->set_FTP_received_path(path, true) ;
	else if (get_rootType()==3)
		config->set_WORKDIR_path(path, true) ;
}


//******************************************************************************
//								COMPUTE ACCESSOR
//******************************************************************************

Glib::ustring Explorer_tree::find_treeView_path(Glib::ustring& dir_wt_root, Glib::RefPtr<Gtk::TreeModel> model)
{
	Explorer_utils::print_trace("Explorer_tree::find_path:> SEarching", dir_wt_root, 1)  ;

	TreeModel_Columns m ;
	//final treeView path
	Glib::ustring good_path_str ;

	//place each element of path in a vector
	std::vector<Glib::ustring> paths ;
	FileHelper::cut_path(dir_wt_root, &paths) ;

	//get all children of tree
	Gtk::TreeModel::Children children2 = model->children() ;
	Gtk::TreeIter it = children2.begin() ;
	Gtk::TreeModel::Children children = it->children() ;

	Gtk::TreeIter it_tmp ;
	guint j = 1 ;
	bool found = true ;
	Gtk::TreePath path ;

	//for each name in path iterates over nodes and depths
	while( j<paths.size() && found )
	{
		found = false;

		//Explorer_utils::print_trace("Explorer_tree::find_path:> FATHER", (*it)[m.m_file_name], 1)  ;
		//Explorer_utils::print_trace("Explorer_tree::find_path:> SEARCHING", paths[j], 1)  ;
		it = children.begin();
		//in each node search for corresponding name
		while ( it!=children.end() && !found )
		{
			if ( (*it)[m.m_file_name]==paths[j] ) {
				//Explorer_utils::print_trace("Explorer_tree::find_path:> found !", 1)  ;
				found = true ;
				path = model->get_path(it) ;
				children = it->children() ;
				//if no children (node is not loaded) fill the node
				if (children.size()==0 || children.size()==1) {
					path.clear() ;
					fill_node(it,true) ;
					children = it->children() ;
					refilter() ;
					//Explorer_utils::print_trace("Explorer_tree::find_path:> nb children: ", children.size(), 1)  ;
				}
				//Explorer_utils::print_trace("Explorer_tree::find_path:> found: ", (*it)[m.m_file_name], 1)  ;
			}
			else {
				it++ ;
				//Explorer_utils::print_trace("Explorer_tree::find_path:> next ", (*it)[m.m_file_name], 1)  ;
			}
		}//end while all children
		j++ ;
	}//end while all words
	if (found) {
		path = model->get_path(it) ;
		good_path_str = path.to_string() ;
	}
	if (!found) {
		Explorer_utils::print_trace("Explorer_tree::find_path:> NOT FOUND DIRECTORY", 1) ;
		return "" ;
	}
	else {
		Explorer_utils::print_trace("Explorer_tree::find_path:> RESULT", good_path_str, 1) ;
		return good_path_str ;
	}
}


Glib::ustring Explorer_tree::compute_path_from_node(Gtk::TreeIter sorted_iterator, bool isSortedFiltered)
{
	Gtk::TreeIter iter ;

	if (isSortedFiltered)
		//> get corresponding iter to model
		iter = convert_iter(sorted_iterator) ;
	else
		iter = sorted_iterator ;

	TreeModel_Columns m ;

	Gtk::TreeRow tmp_row ;
	Gtk::TreeIter tmp_iter ;

	Gtk::TreePath tree_path = refModelTree->get_path(iter) ;
	tmp_row = *iter ;
	Glib::ustring path = tmp_row[m.m_file_name] ;

	while( tree_path.up() ) {
		if (tree_path.get_depth() > 0) {
			tmp_iter = refModelTree->get_iter(tree_path) ;
			tmp_row = *tmp_iter ;
			path = FileHelper::build_path(tmp_row[m.m_file_name],path) ;
		}
	}
	return path ;
}


//******************************************************************************
//									CONVERSION
//******************************************************************************

Gtk::TreeIter Explorer_tree::convert_iter(Gtk::TreeIter sortedIter)
{
	Gtk::TreeIter tmp = sortedIter ;
	Gtk::TreeIter it_filtered = get_refSortedModelTree()->convert_iter_to_child_iter(tmp) ;
	Gtk::TreeIter model_iter = get_refFilteredModelTree()->convert_iter_to_child_iter(it_filtered) ;
	return model_iter ;
}

Gtk::TreeIter Explorer_tree::sort_filter_iter(Gtk::TreeIter iter)
{
	Gtk::TreeIter it_filtered = get_refFilteredModelTree()->convert_child_iter_to_iter(iter) ;
	Gtk::TreeIter sorted_iter = get_refSortedModelTree()->convert_child_iter_to_iter(it_filtered) ;
	return sorted_iter ;
}


//******************************************************************************
//									SELECTION
//******************************************************************************

void Explorer_tree::clear_selection()
{
	Glib::RefPtr<Gtk::TreeSelection> selection = view.get_selection() ;
	if (selection)
		selection->unselect_all() ;
}

void Explorer_tree::selection_has_changed(Gtk::TreeIter* sorted_iter)
{
	if (sorted_iter && refSortedModelTree->iter_is_valid(*sorted_iter))
		child_selected = refSortedModelTree->get_path(view.get_selection()->get_selected()) ;
}


//******************************************************************************
//									DRAG'N'DROP
//******************************************************************************

void Explorer_tree::setDragAndDropTarget(std::vector<Gtk::TargetEntry> targetList)
{
	target_list = targetList ;
	view.drag_dest_set(target_list,Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
	view.drag_source_set(target_list,Gdk::MODIFIER_MASK,Gdk::ACTION_MOVE) ;
//	view.drag_source_set(target_list,Gdk::CONTROL_MASK,Gdk::ACTION_COPY) ;
	view.drag_source_set_icon(ICO_TREE_TXT) ;
}

} //namespace


