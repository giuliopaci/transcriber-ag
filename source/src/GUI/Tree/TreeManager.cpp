/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "TreeManager.h"
#include "TreeModel_Columns.h"
#include "Common/util/FileHelper.h"
#include "Explorer_dialog.h"
#include "Common/Explorer_filter.h"
#include "Explorer_utils.h"
#include "Common/util/Utils.h"

#include "Configuration.h"
#include "ShortcutDialog.h"
#include <gtk/gtk.h>


namespace tag {


#define MAX_COLUMN_ICO 100
#define MAX_COLUMN_DISPLAY 200
#define MAX_COLUMN_NBFILES 80

//**************************************************************************
//**************************************************************** CONSTRUCT
//**************************************************************************

TreeManager::TreeManager(Gtk::Window* win, Configuration* _config)
{
	filter = Explorer_filter::getInstance() ;

	config = _config ;
	window = win ;
	current_number = 0 ;
	drag_drop_initiated = false ;
	copy_initiated = false ;
	cut_initiated = false ;

	progressWatcher = new ProgressionWatcher(_("File Manager"), "directory", false, NULL) ;

	dragdropTarget.set_info(0) ;
	dragdropTarget.set_flags(Gtk::TARGET_SAME_APP) ;
	dragdropTarget.set_target("TRANSCRIBER_TREE_ROW") ;
	dragdropTargetList.push_back(dragdropTarget) ;
}

TreeManager::~TreeManager()
{
	std::map<int, Explorer_tree*>::iterator it ;
	it = trees.begin() ;
	while (it!=trees.end()) {
		delete(it->second) ;
		it++ ;
	}
	if (progressWatcher)
		delete(progressWatcher) ;
}


//**************************************************************************
//								CREATE
//**************************************************************************

Explorer_tree* TreeManager::add_tree(Glib::ustring path, Glib::ustring display,
									 int rootType, bool drag_enable)
{
	Explorer_tree* tree = NULL ;

	if ( !exist_tree(path, -1) )
	{
		//> -- Create
		trees[current_number] = new Explorer_tree(window, this) ;
		tree = trees[current_number] ;

		//> -- Set information to tree
		tree->set(path, rootType, current_number, display) ;

		//> -- Fill tree
		tree->fill_root_node(path , display, rootType, current_number) ;

		//> -- Set drag N drop
		if (drag_enable)
		{
			tree->setDragAndDropTarget(dragdropTargetList) ;
			tree->get_view()->signal_drag_motion().connect(sigc::bind<TreeView_mod*>(sigc::mem_fun(*this, &TreeManager::on_drag_motion), tree->get_view())) ;
			tree->get_view()->signal_drag_data_get().connect(sigc::bind<TreeView_mod*>(sigc::mem_fun(*this, &TreeManager::on_drag_data_get), tree->get_view())) ;
			tree->get_view()->signal_drag_drop().connect(sigc::bind<TreeView_mod*>(sigc::mem_fun(*this, &TreeManager::on_drag_drop), tree->get_view())) ;
		}

		//> -- Connect for getting selection
		tree->get_view()->signalSelection().connect( sigc::bind<int>(sigc::mem_fun(*this, &TreeManager::on_cursor_tree_changed), current_number)) ;

		//> -- Connect tree row actions
		tree->get_view()->signal_row_activated().connect(sigc::bind<Explorer_tree*>(sigc::mem_fun(*this, &TreeManager::on_tree_row_activated), tree)) ;
		tree->get_view()->signal_row_expanded().connect(sigc::bind<Explorer_tree*>(sigc::mem_fun(*this, &TreeManager::on_tree_row_expanded), tree)) ;
		tree->get_refFilteredModelTree()->set_visible_func( sigc::mem_fun(*this, &TreeManager::treeModelFilter_callback) ) ;
		current_number++ ;

		//> -- Save shortcuts
		if (rootType==6)
			save_shortcuts(config->get_SHORTCUTS_path()) ;
	}
	return tree ;
}


Explorer_tree* TreeManager::add_shortcutTree(Glib::ustring path, Glib::ustring display)
{
	return add_tree(path, display, 6, true) ;
}

Explorer_tree* TreeManager::get_tree(int number)
{
	if (number >= trees.size())
		return NULL ;
	else
		return trees[number] ;
}

int TreeManager::remove_tree(int number)
{
	if (number >= trees.size())
		return -1 ;
	else {
		std::map<int, Explorer_tree*>::iterator it ;
		it = trees.begin() ;
		bool ok = false ;
		while (it!=trees.end() && !ok ) {
			if (it->first ==number) {
				ok = true ;
				Explorer_tree* tmp = it->second ;
				delete_tree(tmp) ;
			}
			it++ ;
		}
		return 1 ;
	}
}

bool TreeManager::exist_tree(Glib::ustring path, int excepted_root_number)
{
	std::map<int, Explorer_tree*>::iterator it ;
	bool exist = false ;
	it = trees.begin() ;
	while (it!=trees.end() && !exist) {
		if (it->second->get_rootPath()==path
				&& it->second->get_rootType()==6
				&& it->second->get_rootNumber()!=excepted_root_number)
			exist = true ;
		it++ ;
	}
	return exist ;
}

void TreeManager::refilter_all()
{
	std::map<int, Explorer_tree*>::iterator it ;
	for (it=trees.begin() ; it!=trees.end(); it++) {
		it->second->refilter() ;
		it->second->get_view()->enable_popup_paste(false) ;
	}
}

bool TreeManager::on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time, TreeView_mod* view)
{
	TreeModel_Columns m ;
	Gtk::TreePath path ;
	int x2, y2 ;
	Gtk::TreeViewColumn* col ;
	bool exist = view->get_path_at_pos(x, y, path, col, x2, y2) ;
	if (exist)
	{
		Gtk::TreeIter it = view->get_model()->get_iter(path) ;
		if (drag_drop_initiated)
		{
			drag_dest = it ;
			move_file(drag_src, drag_dest, true) ;
		}
	}
	return true ;
}

void TreeManager::on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& context,
							   Gtk::SelectionData& selection_data,
							   guint info, guint time, TreeView_mod* view)
{
	TreeModel_Columns m ;
	Gtk::TreeIter it = (view->get_selection())->get_selected() ;
	if ( (*it)[m.m_file_sysType]==0 || (*it)[m.m_file_sysType]==1 )
	{
		drag_src = it ;
		drag_drop_initiated = true ;
	}
}

bool TreeManager::on_drag_motion(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time, TreeView_mod* view)
{
	// TODO OPEN IF ITER IS OVER DIRECTORY
	return true ;
}

void TreeManager::move_file(Gtk::TreeIter src, Gtk::TreeIter dest, bool filtered)
{
	TreeModel_Columns m ;

	//> get tree of each Pointer
	Explorer_tree* tree_src = trees[(*src)[m.m_file_root]] ;
	Explorer_tree* tree_dest = trees[(*dest)[m.m_file_root]] ;

	//> get correct iterator
	Gtk::TreeIter src_iter ;
	Gtk::TreeIter dest_iter ;
	if (filtered) {
		src_iter = tree_src->convert_iter(src) ;
		dest_iter = tree_dest->convert_iter(dest) ;
	}
	else {
		src_iter = src ;
		dest_iter = dest  ;
	}

	//> get path of directory src
	Gtk::TreePath p_src =  tree_src->get_refModelTree()->get_path(src_iter) ;
	p_src.up() ;
	Gtk::TreeIter directory_src = tree_src->get_refModelTree()->get_iter(p_src) ;

	Glib::ustring src_iter_string = tree_src->compute_path_from_node(src_iter, false) ;
	Glib::ustring dest_iter_string = tree_dest->compute_path_from_node(dest_iter, false) ;
	Glib::ustring directory_src_string = tree_src->compute_path_from_node(directory_src, false) ;

	//do only if src!=dest and src_dir!=dest_dir
	if (src_iter_string!=dest_iter_string && directory_src_string!=dest_iter_string)
	{
		//> if destination is a file, up to the father directory
		Gtk::TreeIter directory_dest ;
		if ( (*dest_iter)[m.m_file_sysType]==0 ) {
			Gtk::TreePath p = tree_dest->get_refModelTree()->get_path(dest_iter) ;
			p.up() ;
			directory_dest = tree_dest->get_refModelTree()->get_iter(p) ;
		}
		else
			directory_dest = dest_iter ;

		Gtk::TreePath p_src =  tree_src->get_refModelTree()->get_path(src_iter) ;
		Gtk::TreePath p_directory_dest = tree_dest->get_refModelTree()->get_path(directory_dest) ;
		bool ok = true ;

		Glib::ustring dest_path = tree_dest->compute_path_from_node(directory_dest, false) ;
		//> If src is a directory and is ancestor of dest
		if ( (*src_iter)[m.m_file_sysType]==1 && (p_src.is_ancestor(p_directory_dest)) ) {
				ok=false ;
				Explorer_dialog::msg_dialog_error(_("You can't move a directory to itself"), window, true) ;
				Explorer_utils::print_trace("TreeManager::move_file:> same tree, from a directory", 0)  ;
		}
		else {
			//> If destination file already exist, ask user to overwrite it
			Glib::ustring filename = Glib::path_get_basename(src_iter_string) ;
			Glib::ustring new_file_path = FileHelper::build_path(dest_path, filename) ;
			if ( Glib::file_test(new_file_path, Glib::FILE_TEST_EXISTS) ) {
				int res = Explorer_dialog::msg_dialog_question(_("Directory already contains file with same name\nDo you want to replace it ?"), window, true, "") ;
				if (res==Gtk::RESPONSE_NO)
					ok = false ;
			}
		}

		if (ok) {
			Glib::ustring src_path = src_iter_string ;
			Explorer_utils::print_trace("TreeManager::move_file:> from ", src_path,  1)  ;
			Explorer_utils::print_trace("TreeManager::move_file:> to ", dest_path,  1)  ;

			//PREPARE PROGRESS BAR
			Glib::ustring label =  Glib::ustring(_("Move")) + " " + Glib::path_get_basename(src_path) ;
			Glib::ustring legend = Glib::ustring("FROM: ") + directory_src_string + "\n" + "TO: " + dest_path ;
			ProgressionWatcher::Entry* entry = progressWatcher->add_entry(label, src_path, legend, false) ;
			Gtk::ProgressBar* progress = Glib::wrap(entry->get_pbar());
			progressWatcher->show() ;

			int res = FileHelper::move_in_filesystem(src_path, dest_path, progress) ;
			if (res>=0) {
				tree_src->get_refModelTree()->erase(src_iter) ;
				//tree_dest->refresh_directory(dest_iter, false, true, NULL);
				tree_dest->refresh_directory(directory_dest, false, true/*, NULL*/);
				tree_src->refilter() ;
				entry->complete_bar() ;
				entry->set_status(true, _("Moved")) ;
				entry->get_bar()->set_text(Glib::path_get_basename(src_path)) ;
			}
			else {
				Glib::ustring txt ;
				if (res==-20) {
						txt = _("Permissions problem on file, target directory or source directory") ;
				}
				else
						txt = _("Moving file problem") ;
				entry->set_status(true, txt) ;
				entry->get_bar()->set_text(Glib::path_get_basename(src_path)) ;
			}
		}//end ok to move
	}//end same iter
	else {
		Explorer_utils::print_trace("TreeManager::move_file:> same row or src already in dest", 0)  ;
	}

	//> reset manager state
	if(drag_drop_initiated)
		drag_drop_initiated = false ;
}

void TreeManager::copy_file(Gtk::TreeIter* src, Gtk::TreeIter* dest, bool filtered)
{
	TreeModel_Columns m ;

	Explorer_tree* tree_src = trees[(**src)[m.m_file_root]] ;
	Explorer_tree* tree_dest = trees[(**dest)[m.m_file_root]] ;
	Gtk::TreeIter src_iter ;
	Gtk::TreeIter dest_iter ;

	if (filtered) {
		src_iter = tree_src->convert_iter(*src) ;
		dest_iter = tree_dest->convert_iter(*dest) ;
	}
	else {
		src_iter = *src ;
		dest_iter = *dest  ;
	}

	//> get path of directory src
	Gtk::TreePath p_src =  tree_src->get_refModelTree()->get_path(src_iter) ;
	p_src.up() ;
	Gtk::TreeIter directory_src = tree_src->get_refModelTree()->get_iter(p_src) ;

	Glib::ustring src_iter_string = tree_src->compute_path_from_node(src_iter, false) ;
	Glib::ustring dest_iter_string = tree_dest->compute_path_from_node(dest_iter, false) ;
	Glib::ustring directory_src_string = tree_src->compute_path_from_node(directory_src, false) ;

	//do only if src!=dest and src_dir!=dest_dir
	if (src_iter_string!=dest_iter_string && directory_src_string!=dest_iter_string)
	{
		//> if destination is a file, up to the father directory
		Gtk::TreeIter directory_dest ;
		if ( (*dest_iter)[m.m_file_sysType]==0 ) {
			Gtk::TreePath p = tree_dest->get_refModelTree()->get_path(dest_iter) ;
			p.up() ;
			directory_dest = tree_dest->get_refModelTree()->get_iter(p) ;
		}
		else
			directory_dest = dest_iter ;

		Gtk::TreePath p_src =  tree_src->get_refModelTree()->get_path(src_iter) ;
		Gtk::TreePath p_directory_dest = tree_dest->get_refModelTree()->get_path(directory_dest) ;
		bool ok = true ;

		Glib::ustring dest_path = tree_dest->compute_path_from_node(directory_dest, false) ;
		//> If src is a directory and is ancestor of dest
		if ( (*src_iter)[m.m_file_sysType]==1 && (p_src.is_ancestor(p_directory_dest)) ) {
				ok=false ;
				Explorer_dialog::msg_dialog_error(_("You can't move a directory to itself"), window, true) ;
				Explorer_utils::print_trace("TreeManager::move_file:> same tree, from a directory", 0)  ;
		}
		else {
			//> If destination file already exist, ask user to overwrite it
			Glib::ustring filename = Glib::path_get_basename(src_iter_string) ;
			Glib::ustring new_file_path = FileHelper::build_path(dest_path, filename) ;
			if ( Glib::file_test(new_file_path, Glib::FILE_TEST_EXISTS) ) {
				int res = Explorer_dialog::msg_dialog_question(_("Directory already contains file with same name\nDo you want to replace it ?"), window, true, "") ;
				if (res==Gtk::RESPONSE_NO)
					ok = false ;
			}
		}

		if (ok) {
			Glib::ustring src_path = src_iter_string ;
			Explorer_utils::print_trace("TreeManager::copy_file:> from", src_path, 1)  ;
			Explorer_utils::print_trace("TreeManager::copy_file:> to", dest_path, 1)  ;

			//PREPARE PROGRESS BAR
			Glib::ustring label =  Glib::ustring(_("Copy")) + " " + Glib::path_get_basename(src_path) ;
			Glib::ustring legend = Glib::ustring("FROM: ") + directory_src_string + "\n" + "TO: " + dest_path ;
			ProgressionWatcher::Entry* entry = progressWatcher->add_entry(label, src_path, legend, false) ;
			Gtk::ProgressBar* progress = Glib::wrap(entry->get_pbar());
			progressWatcher->show() ;

			int res = FileHelper::copy_in_filesystem(src_path, dest_path, progress) ;
			if (res>=0) {
				tree_dest->refilter() ;
				//tree_dest->refresh_directory(dest_iter, false, true, NULL);
				tree_dest->refresh_directory(directory_dest, false, true/*, NULL*/);
				entry->complete_bar() ;
				entry->set_status(true, _("Copied")) ;
				entry->get_bar()->set_text(Glib::path_get_basename(src_path)) ;
			}
			else {
				Glib::ustring txt ;
				if (res==-20) {
						txt = _("Permissions problem on file, target directory or source directory") ;
				}
				else if (res==-41)
						txt = _("Can't find the source file or source directory") ;
				else if (res==-42)
						txt = _("Can't find destination directory") ;
				else
						txt = _("Copying file problem") ;
				entry->set_status(true, txt) ;
				entry->get_bar()->set_text(Glib::path_get_basename(src_path)) ;
				Explorer_utils::print_trace("TreeManager::copy_file:> copy aborted [", res, 0)  ;
			}
		}
	}
	else {
		Explorer_utils::print_trace("TreeManager::copy_file:> same row or src already in dest", 0)  ;
	}
}

void TreeManager::set_copy(Gtk::TreeIter src)
{
	TreeModel_Columns m ;

	//> get corresponding tree
	Explorer_tree* tree_src = trees[(**src)[m.m_file_root]] ;
	Gtk::TreeIter src_iter = tree_src->convert_iter(src) ;

	//> stock iter
	cc_src = src_iter ;

	copy_initiated = true ;
	cut_initiated=false;

	//>actualise popup
	std::map<int, Explorer_tree*>::iterator it ;
	it = trees.begin() ;
	while (it!=trees.end()) {
		(it->second)->get_view()->enable_popup_paste(true) ;
		it++ ;
	}
}

void TreeManager::set_cut(Gtk::TreeIter src)
{
	TreeModel_Columns m ;

	//> get corresponding tree
	Explorer_tree* tree_src = trees[(**src)[m.m_file_root]] ;
	Gtk::TreeIter src_iter = tree_src->convert_iter(src) ;
	//> stock iter
	cc_src = src_iter ;

	cut_initiated = true ;
	copy_initiated=false ;

	std::map<int, Explorer_tree*>::iterator it ;
	it = trees.begin() ;
	while (it!=trees.end()) {
		(it->second)->get_view()->enable_popup_paste(true) ;
		it++ ;
	}
}


void TreeManager::paste(Gtk::TreeIter dest)
{
	TreeModel_Columns m ;
	//> get corresponding tree
	Explorer_tree* tree_dest = trees[(**dest)[m.m_file_root]] ;
	Gtk::TreeIter dest_iter = tree_dest->convert_iter(dest) ;
	//> stock iter
    cc_dest = dest_iter ;

	if(copy_initiated) {
		if (cc_src) {
			copy_file(&cc_src, &cc_dest, false) ;
			//copy_initiated = false ;
		}
		else {
			Explorer_utils::print_trace("TreeManager::paste:> CC_SRC", 1)  ;
		}
	}
	else if (cut_initiated) {
		move_file(cc_src, cc_dest, false) ;
		//cut_initiated = false ;
	}
}

void TreeManager::set_last_selected(Gtk::TreeIter* iter, int tree_number)
{
	last_selected_tree = tree_number ;
}

void TreeManager::actualize_selection(int tree_number)
{
	if (last_selected_tree!=tree_number) {
		std::map<int, Explorer_tree*>::iterator it ;
		it = trees.begin() ;
		while (it!=trees.end()) {
			if ( (it->second)->get_rootNumber()!=tree_number )
				(it->second)->clear_selection() ;
			it++ ;
		}
	}
}

void TreeManager::on_cursor_tree_changed(Gtk::TreeIter* iter, int tree_number)
{
	//> actualize trees selection
	actualize_selection(tree_number) ;

	//> set selection
	set_last_selected(iter, tree_number) ;
}

bool TreeManager::has_selection()
{
	bool selection = false ;
	std::map<int, Explorer_tree*>::iterator it = trees.begin() ;
	while (it!=trees.end() && !selection ) {
		if ( it->second->get_view()->get_selection()->get_selected() )
			selection = true ;
		it++ ;
	}
	return selection ;
}


bool TreeManager::has_focus()
{
	bool has = false ;
	std::map<int, Explorer_tree*>::iterator it = trees.begin() ;
	while (it!=trees.end() && !has ) {
		if (it->second->get_view()->has_focus())
			has = true ;
		it++ ;
	}
	return has ;
}


int TreeManager::get_rootNumber_from_file(Glib::ustring file)
{
	int number = -1 ;

	Glib::ustring file_tmp = file ;
	Glib::ustring new_file_dir = Glib::path_get_dirname(file_tmp) ;

	Glib::ustring current, tmp, old ;
	old = "" ;
	current = new_file_dir ;
	//testing on each stair of path
	while (current!=old)
	{
		//> for each tree test if root == current stair
		std::map<int, Explorer_tree*>::iterator it = trees.begin() ;
		while (it!=trees.end() && number==-1) {
			Glib::ustring rootPath = it->second->get_rootPath() ;
			if (rootPath==current)
				number = it->second->get_rootNumber() ;
			it++ ;
		}
		//> go up in path stair for next iteration if not found
		old = current ;
		tmp = current ;
		current = Glib::path_get_dirname(tmp) ;
	}
	return number ;
}


void TreeManager::display_new_file(Glib::ustring file)
{
	int number = get_rootNumber_from_file(file) ;

	if (number!=-1) {
		Glib::ustring file_tmp = file ;
		Glib::ustring new_file_dir = Glib::path_get_dirname(file_tmp) ;
		//compute directory path without the root
		guint sub_size = trees[number]->get_rootPath().size() ;
		guint total_size = new_file_dir.size() ;
		Glib::ustring dir_wt_root = new_file_dir.substr(sub_size, total_size-sub_size) ;
		//refresh directory
		trees[number]->refresh_directory(dir_wt_root) ;
		Explorer_utils::print_trace("TreeManager::display_new_file:> name: ", trees[number]->get_rootDisplay() ,1) ;
	}
	else {
		Explorer_utils::print_trace("TreeManager::display_new_file:> NOT FOUND TREE", 0) ;
	}
}


void TreeManager::change_target_tree(Gtk::Window* window, Explorer_tree* tree, Configuration* config)
{
	//> if project tree root don't allow to change name
	bool lock_name = false ;
	if (tree->get_rootType()==3 || tree->get_rootType()==5)
		lock_name = true ;

	ShortcutDialog* dialog = new ShortcutDialog(tree->get_rootNumber(), lock_name) ;
	dialog->set_treeManager(this) ;
	dialog->set_default( tree->get_rootPath(), tree->get_rootDisplay()) ;

	#ifdef __APPLE__
	dialog->set_transient_for(*window);
	#endif

	int rep ;
	rep = dialog->run() ;

	Glib::ustring path =  dialog->get_chosen_path() ;
	Glib::ustring display =  dialog->get_chosen_display() ;

	if ( rep==Gtk::RESPONSE_OK && !exist_tree(path, tree->get_rootNumber()) )
		tree->change_target_tree(path, display, config) ;

	save_shortcuts(config->get_SHORTCUTS_path()) ;

	delete(dialog) ;
}

void TreeManager::delete_tree(Explorer_tree* tree)
{
	tree->get_refModelTree()->clear() ;
	int number = tree->get_rootNumber() ;
	trees.erase(number);
	save_shortcuts(config->get_SHORTCUTS_path()) ;
}

void TreeManager::save_shortcuts(Glib::ustring path)
{
	std::vector<Glib::ustring> toWrite ;

	std::map<int, Explorer_tree*>::iterator it ;
	bool exist = false ;
	it = trees.begin() ;
	while (it!=trees.end() && !exist) {
		if (it->second->get_rootType()==6) {
			Glib::ustring tmp = it->second->get_rootDisplay() + "|" + it->second->get_rootPath() ;
			toWrite.insert(toWrite.end(), tmp) ;
		}
		it++ ;
	}
	int res =Explorer_utils::write_lines(path, toWrite, "w");
	if (res<0)
		Explorer_utils::print_trace("NoteBook_mod::close_all:> ", res, 0) ;
}

void TreeManager::hide_tooltips()
{
	std::map<int, Explorer_tree*>::iterator it ;
	it = trees.begin() ;
	while (it!=trees.end()) {
		(it->second)->get_view()->hide_tooltip() ;
		it++ ;
	}
}

//******************************************************************************
//								 BUSINESS CALLBACK
//******************************************************************************

/*** on double-click on row, open or close the node ***/
void TreeManager::on_tree_row_activated(const Gtk::TreeModel::Path &path, const Gtk::TreeViewColumn* column, Explorer_tree* tree)
{
	TreeModel_Columns* m_columns = tree->get_model_Columns();

	Gtk::TreeIter iter = tree->get_refSortedModelTree()->get_iter(path) ;
	Gtk::TreeRow row = *(iter) ;

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> FOR DIRECTORY
	// just open or close the row
	if ( row[m_columns->m_file_sysType] != 0 ) {
		//> if row expanded, collapse
		if ( tree->get_view()->row_expanded(path) ) {
			tree->get_view()->collapse_row(path) ;
		}
		//> else expand and fill row
		else {
			tree->get_view()->expand_row(path, false) ;
		}
	}
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> FOR FILE
	// launch appropriate file action
	else {
		//for classic file, open it
		if ( row[m_columns->m_file_rootType] !=4 ) {
			Glib::ustring f_path = tree->compute_path_from_node(iter, true) ;
			m_signalOpenFileRequired.emit(f_path, "openall") ;
		}
	}
}

void TreeManager::on_tree_row_expanded(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path, Explorer_tree* tree)
{
	if (!tree->get_loaded())
	{
		TreeModel_Columns m ;

		//> Special check for FTP
/*		if( tree->get_rootType()==4 )
		{
			if ( !FTPdata->is_connected() )
				return ;
			if (!FTPdata->check_connection())
			{
				FTPdata->disconnect(true) ;
				return ;
			}
		}
*/
		//> Erase all children
		Gtk::TreeIter it_model = tree->convert_iter(iter) ;
		Gtk::TreeModel::Children children = it_model->children() ;
		Gtk::TreeIter i = children.begin();
		while ( i!=children.end())
		{
			Gtk::TreeIter tmp_iter = i ;
			i++ ;
			tree->get_refModelTree()->erase(tmp_iter) ;
		}
		int nb = 0 ;

		//> Fill tree
		if( tree->get_rootType()!=4 )
			nb = tree->fill_node(it_model, false) ;

		tree->refilter() ;

		//> Lock tree for the following expand not to fill the tree
		tree->set_loaded(true) ;

		//> If can't expand because nothing in, unlock to enable next count
		if (!tree->get_view()->expand_row(path, false))
			tree->get_view()->expand_to_path(path) ;

		//> Unlock the tree
		tree->set_loaded(false) ;

		//> For classic directories display number of files contained
		if ( (*it_model)[m.m_file_sysType] == 1 && tree->get_rootType()!=4  )
		{
			(*it_model)[m.m_file_nbFilteredFiles] = "(" + number_to_string(nb) + ")" ;
			(*it_model)[m.m_file_display_wNumber] = (*it_model)[m.m_file_display] + "  " + (*it_model)[m.m_file_nbFilteredFiles] ;
		}
	}
}

void TreeManager::activate_tree_row(const Gtk::TreeModel::Path &path, const Gtk::TreeViewColumn* column, Explorer_tree* tree)
{
	on_tree_row_activated(path, column, tree) ;
}


/**	Function called each time a tree is filtered
 * 	Define a row as visible (return true) or not, depending on filter
 *  chosen by user and file type
 *  Called for each row of model
 */
bool TreeManager::treeModelFilter_callback(const Gtk::TreeIter& iter)
{
	TreeModel_Columns m_Columns ;

	Gtk::TreeModel::Row row = *iter ;
	Glib::ustring name = row[m_Columns.m_file_name] ;

	//> Don't display hidden files
	if ( (name.c_str())[0]=='.' && ((row[m_Columns.m_file_sysType]==0) | (row[m_Columns.m_file_sysType]==1)) )
		return false ;

	//> Always display visible directories |or| virtual row for enabling expander
	else if ( row[m_Columns.m_file_sysType] > 0 || row[m_Columns.m_file_sysType] == -1 )
		return true ;

	//> Depends on visible file type
	else
		return filter->is_file_in_filter(name) ;
}

} //namespace

