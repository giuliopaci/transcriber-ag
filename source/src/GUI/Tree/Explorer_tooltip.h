/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef EXPLORER_TOOLTIP_H_
#define EXPLORER_TOOLTIP_H_

#include <gtkmm.h>
#include "Common/widgets/TooltipTT.h"

namespace tag {

class TreeView_mod ;

/**
 * @class 	Explorer_tooltip
 * @ingroup	GUI
 *
 *  TooltipTT specialization for the Explorer_tree class.\n
 *	Used by the TreeView_mod class.
 *
 */
class Explorer_tooltip : public TooltipTT
{
	public:
		/**
		 * Constructor
		 * @param parent	TreeView_mod parent
		 */
		Explorer_tooltip(TreeView_mod& parent) ;
		virtual ~Explorer_tooltip();

		bool display(GdkEventMotion event, Gtk::Widget* win) ;

	private:

		TreeView_mod& parent ;
		Glib::ustring path ;

		//> display the tooltip
		Gtk::Table* table ;

		Gtk::Label name_l ;
		Gtk::Label name_v ;
		Gtk::Alignment name_al ;
		Gtk::Alignment name_av ;

		Gtk::Label type_l ;
		Gtk::Label type_v ;
		Gtk::Alignment type_al ;
		Gtk::Alignment type_av ;

		Gtk::Label size_l ;
		Gtk::Label size_v ;
		Gtk::Alignment size_al ;
		Gtk::Alignment size_av ;

		Gtk::Label channel_l ;
		Gtk::Label channel_v ;
		Gtk::Alignment channel_al ;
		Gtk::Alignment channel_av ;

		Gtk::Label encoding_l ;
		Gtk::Label encoding_v ;
		Gtk::Alignment encoding_al ;
		Gtk::Alignment encoding_av ;

		Gtk::Label duration_l ;
		Gtk::Label duration_v ;
		Gtk::Alignment duration_al ;
		Gtk::Alignment duration_av ;

		Gtk::Label imported_l ;
		Gtk::Label imported_v ;
		Gtk::Alignment imported_al ;
		Gtk::Alignment imported_av ;

		Gtk::Label modtime_l ;
		Gtk::Label modtime_v ;
		Gtk::Alignment modtime_al ;
		Gtk::Alignment modtime_av ;

		void set_labels() ;
		void prepare_tooltip(Gtk::TreeModel::iterator iter, GdkEventMotion event) ;
		void prepare_tooltip_info(Glib::ustring info) ;
		void prepare_tooltip_table(std::vector<Glib::ustring> info, Gtk::Table* tab) ;
		void prepare_tooltip_shortcut(Glib::ustring name, Glib::ustring path) ;

		void reset_data() {} ;

};

} //namespace


#endif /*EXPLORER_TOOLTIP_H_*/
