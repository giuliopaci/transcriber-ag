/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Explorer_tooltip.h"
#include "TreeModel_Columns.h"
#include "Explorer_fileHelper.h"
#include "Explorer_utils.h"
#include "Common/util/Utils.h"
#include "Common/globals.h"
#include "Common/Explorer_filter.h"
#include "Explorer_tree.h"
#include <iostream>

namespace tag {

Explorer_tooltip::Explorer_tooltip(TreeView_mod& p_parent)
: TooltipTT(), parent(p_parent)
{
	// look'n'feel
	set_name("tooltip_win");

	table = NULL ;
	set_labels() ;
}

Explorer_tooltip::~Explorer_tooltip()
{
	if (table!=NULL)
		delete(table) ;
}

void Explorer_tooltip::prepare_tooltip(Gtk::TreeModel::iterator iter, GdkEventMotion event)
{
	TreeModel_Columns m_Columns ;

	//> get file information
	Explorer_tree* tree = parent.get_modelAG() ;
	Glib::ustring path = tree->compute_path_from_node(iter, true);
	Glib::ustring info ;

	//for shortcut, display name and path only
	if ( (*iter)[m_Columns.m_file_sysType]==6 ) {
		Glib::ustring name = (*iter)[m_Columns.m_file_display] ;
		prepare_tooltip_shortcut(name, path) ;
	}
	//for file
	else {
		info = Explorer_fileHelper::TAG_file_info(path) ;
		prepare_tooltip_info(info);
	}
}

void Explorer_tooltip::prepare_tooltip_info(Glib::ustring info)
{
	TreeModel_Columns m ;

	//> format info
	std::vector<Glib::ustring> info_v ;
	mini_parser(',', info, &info_v) ;

	if (table!=NULL) {
		frame_in.remove() ;
		delete(table);
	}
	table = new Gtk::Table() ;
	prepare_tooltip_table(info_v, table) ;

	frame_in.add(*table) ;
	frame_in.show_all_children() ;

	show_all_children(true) ;
}

void Explorer_tooltip::prepare_tooltip_shortcut(Glib::ustring name, Glib::ustring path)
{
	TreeModel_Columns m ;

	if (table!=NULL) {
		frame_in.remove() ;
		delete(table);
	}
	table = new Gtk::Table() ;

	//> name
	table->attach(name_al , 0, 1, 0, 1, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	table->attach(name_av , 1, 2, 0, 1, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	name_l.set_label(_("Name: ")) ;
	name_v.set_label(name) ;

	//> path
	table->attach(imported_al , 0, 1, 1, 2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	table->attach(imported_av , 1, 2, 1, 2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	imported_l.set_label(_("Path: ")) ;
	imported_v.set_label(path) ;

	frame_in.add(*table) ;
	frame_in.show_all_children() ;

	show_all_children(true) ;
}

void Explorer_tooltip::prepare_tooltip_table(std::vector<Glib::ustring> info, Gtk::Table* tab)
{
	Glib::ustring type, size, channel, resolution, samplingRate;
	Glib::ustring name, totalSample, status, encoding, modtime ;
	Glib::ustring duration ;

	name = info[EXPLORER_FI_NAME] ;
	type = info[EXPLORER_FI_TYPE] ;
	size = info[EXPLORER_FI_SIZE] ;
	channel = info[EXPLORER_FI_CHANNEL] ;
	resolution =  info[EXPLORER_FI_SAMPLING_RES] ;
	samplingRate = info[EXPLORER_FI_SAMPLING_RATE]  ;
	totalSample = info[EXPLORER_FI_TOTAL_SAMPLE]  ;
	status = info[EXPLORER_FI_IMPORT_STATE] ;
	encoding = info[EXPLORER_FI_ENCODING] ;
	modtime = info[EXPLORER_FI_MODTIME] ;
	duration = info[EXPLORER_FI_DURATION] ;

	//start to 4 because always displaying NAME, SIZE, EXTENSION and MODIFICATION DATE
	int cpt = 4 ;
	int x1 = 0 ;
	int x2 = 1 ;

	//see additional information for Audio files
	if (!channel.empty())
		cpt++ ;
	if (!encoding.empty())
		cpt++ ;
	if (!duration.empty())
		cpt++ ;
	//unused: if not local file
	if (status!="local")
		cpt++ ;

	//update size table
	(tab)->resize(cpt,2) ;
	(tab)->set_col_spacings(10) ;
	(tab)->set_homogeneous(false) ;

	//> NAME OF FILE
	name_v.set_label(name) ;
	(tab)->attach(name_al , 0, 1, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	(tab)->attach(name_av , 1, 2, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	x1++ ;
	x2++ ;

	//> TYPE OF FILE
	type_v.set_label(type) ;

	(tab)->attach(type_al , 0, 1, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	(tab)->attach(type_av , 1, 2, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	x1++ ;
	x2++ ;

	//> CANAL and Sampling resolution
	Glib::ustring tmp_channel ;
	if (!channel.empty())
	{
		if (channel == number_to_string(1))
			channel = _("Mono") ;
		else if (channel == number_to_string(2))
			channel = _("Stereo") ;
		tmp_channel.append(channel) ;
		if (!resolution.empty()) {
			tmp_channel.append(" ") ;
			tmp_channel.append(resolution);
			tmp_channel.append(_(" bits"));
		}

		channel_v.set_label(tmp_channel) ;

		(tab)->attach(channel_al , 0, 1, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
		(tab)->attach(channel_av , 1, 2, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
		x1++ ;
		x2++ ;
	}

	//> ENCODING (Codecs)
	if (encoding != "" ) {
		encoding_v.set_label(encoding) ;
		(tab)->attach(encoding_al , 0, 1, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
		(tab)->attach(encoding_av , 1, 2, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
		x1++ ;
		x2++ ;
	}

	//> SIZE of FILE
	Glib::ustring tmp_size = Explorer_utils::format_size(size) ;
	size_v.set_label(tmp_size) ;
	(tab)->attach(size_al , 0, 1, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	(tab)->attach(size_av , 1, 2, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	x1++ ;
	x2++ ;

	//> DURATION
	Glib::ustring tmp_duration ;
	if (!duration.empty()) {
		double duration_seconds = string_to_number<double>(duration) ;

		std::vector<int> time ;
		Explorer_utils::get_time(duration_seconds, &time) ;

		tmp_duration = number_to_string(time[0]) + "h "
				+ number_to_string(time[1]) + "min "
				+ number_to_string(time[2]) + "sec" ;

		duration_v.set_label(tmp_duration) ;

		(tab)->attach(duration_al , 0, 1, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
		(tab)->attach(duration_av , 1, 2, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
		x1++ ;
		x2++ ;
	}

	//> display imported status
	if (status!="local" && !status.empty())
	{
		//if (status=="yes") {
		imported_v.set_label(status) ;
			//imported_v.set_name("tooltip_import_status_yes") ;
		/*} else if (status=="no") {
			imported_v.set_label("Not updated") ;
			imported_v.set_name("tooltip_import_status_no") ;
		}*/
		(tab)->attach(imported_al , 0, 1, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
		(tab)->attach(imported_av , 1, 2, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
		x1++ ;
		x2++ ;
	}

	//> dipslay Modification date
	if (modtime!="") {
		Glib::ustring display_modtime = Explorer_utils::format_date(modtime) ;
		modtime_v.set_label(display_modtime) ;
		(tab)->attach(modtime_al , 0, 1, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
		(tab)->attach(modtime_av , 1, 2, x1, x2, Gtk::FILL, Gtk::EXPAND, 0, 0) ;
	}

	type_l.set_label(_("Type:")) ;
	name_l.set_label(_("Name:")) ;
	size_l.set_label(_("Size:")) ;
	channel_l.set_label("") ;
	encoding_l.set_label(_("Encoding:")) ;
	duration_l.set_label(_("Duration:")) ;
	imported_l.set_label(_("Status:")) ;

	(tab)->show_all_children() ;
}

void Explorer_tooltip::set_labels()
{
	name_al.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	name_av.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	name_l.set_name("tooltip_label_bold") ;
	name_v.set_name("tooltip_label") ;

	type_al.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	type_av.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	type_l.set_name("tooltip_label_bold") ;
	type_v.set_name("tooltip_label") ;

	size_al.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	size_av.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	size_l.set_name("tooltip_label_bold") ;
	size_v.set_name("tooltip_label") ;

	channel_al.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	channel_av.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	channel_l.set_name("tooltip_label_bold") ;
	channel_v.set_name("tooltip_label") ;

	encoding_al.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	encoding_av.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	encoding_l.set_name("tooltip_label_bold") ;
	encoding_v.set_name("tooltip_label") ;

	duration_al.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	duration_av.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	duration_l.set_name("tooltip_label_bold") ;
	duration_v.set_name("tooltip_label") ;

	imported_al.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	imported_av.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	imported_l.set_name("tooltip_label_bold") ;

	modtime_al.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	modtime_av.set(0, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
	modtime_l.set_label(_("Modified:")) ;
	modtime_l.set_name("tooltip_label_bold") ;

	name_al.add(name_l) ;
	name_av.add(name_v) ;

	type_al.add(type_l) ;
	type_av.add(type_v) ;

	size_al.add(size_l) ;
	size_av.add(size_v) ;

	channel_al.add(channel_l) ;
	channel_av.add(channel_v) ;

	encoding_al.add(encoding_l) ;
	encoding_av.add(encoding_v) ;

	duration_al.add(duration_l) ;
	duration_av.add(duration_v) ;

	imported_al.add(imported_l) ;
	imported_av.add(imported_v) ;

	modtime_al.add(modtime_l) ;
	modtime_av.add(modtime_v) ;
}



bool Explorer_tooltip::display(GdkEventMotion event, Gtk::Widget* win)
{
	if (/*!event ||*/ !win)
		return true ;

	// check path
	TreeModel_Columns m ;
	int x, y, x2, y2 ;
	Gtk::TreePath path ;
	Gtk::TreeViewColumn* col ;

	x = (int) event.x ;
	y = (int) event.y ;
	bool exist = parent.get_path_at_pos(x, y, path, col, x2, y2) ;
	if (exist)
	{
		// check condition
		Gtk::TreeIter iter = parent.get_model()->get_iter(path) ;
		Gtk::TreeRow row = *iter ;
		// for file

		if (row[m.m_file_sysType]==0 || row[m.m_file_sysType]==6)
		{
			// compute position
			int toolx, tooly ;
			compute_position(win, event, toolx, tooly) ;

			// prepare tooltip
			prepare_tooltip(iter, event) ;

			// adjust size
			resize(5,5) ;
			set_default_size(5,5) ;

			//adjust position
			adust_in_screen(toolx, tooly, gdk_get_default_root_window ()) ;
			move(toolx, tooly) ;

			//> don't use show method cause dispay redrawed
			//> resizing for forcing complete resize, otherwise keep all size
			reshow_with_initial_size() ;
		}
	}

	timeout.disconnect() ;
	return true ;
}

} //namespace



