/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */


#ifndef SEARCHREPLACEDIALOG_H_
#define SEARCHREPLACEDIALOG_H_

#include <gtkmm.h>
#include "SearchReplaceGeneral.h"
#include "Configuration.h"
#include "Common/icons/IcoPackImage.h"
#include "Common/widgets/GeoWindow.h"

namespace tag {

/**
 * @class 	SearchReplaceDialog
 * @ingroup	GUI
 *
 * Search component for search 'n replace dialog mode
 * Use SearchReplaceGeneral mechanisms
 *
 */

class SearchReplaceDialog : public SearchReplaceGeneral, public Gtk::Dialog, public GeoWindow
{
	public:
		/**
		 * Constructor
		 */
		SearchReplaceDialog(Configuration* configuration, int searchMode = 0, bool cas = false, bool wholeWord = false);
		virtual ~SearchReplaceDialog();

		/* Abstract method implentation */
		void mySet_focus() ;

		/*** Geometry interface ***/
		virtual void saveGeoAndHide() ;
		virtual int loadGeoAndDisplay(bool rundlg=false) ;

	private:
		Configuration* config;

		//> widgets
		Gtk::Table* table ;
		Gtk::Table* table_button ;
		Gtk::Table* table_gen ;

		Gtk::Alignment align_header;
		Gtk::Alignment align_gen ;
		Gtk::Alignment align_combo ;
		Gtk::Alignment align_frame ;
		Gtk::Alignment align_button ;
		Gtk::Alignment align_end ;
		Gtk::HSeparator sep ;
		Gtk::HSeparator sep2 ;

		Gtk::HBox hbox_input_language ;
		Gtk::HBox hbox_header ;
		Gtk::HBox boc_radio ;
		Gtk::HBox box_search ;
		Gtk::HBox box_replace ;
		Gtk::HBox hbox_options ;
		Gtk::HBox hbox_ends ;


		Gtk::Frame frame_scope ;
		Gtk::VButtonBox bbox_scope ;

		Gtk::Frame frame_options ;
		Gtk::VButtonBox bbox_option ;

		// ACTION BUTTOPN
		Gtk::VButtonBox bbox_gen ;
		Gtk::HButtonBox bbox_find ;
		Gtk::HButtonBox bbox_find_replace ;
		Gtk::HButtonBox bbox_replace ;

		Gtk::Frame search_in_progress_frame ;
		Gtk::HBox search_in_progress_hbox ;
		IcoPackImage search_in_progress ;	/**< search_in_progress gif */
		IcoPackImage search_static ;	/**< search_in_progress gif */

		void set_widgets_label() ;
		void prepare_gui()  ;

		void display_info(Glib::ustring text) ;
		int display_question(Glib::ustring text) ;
		void myHide() ;
		void on_change_mode() ;

		bool on_key_press_event(GdkEventKey* event) ;

		/*** Geometry interface ***/
		virtual void getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel) ;
		virtual Glib::ustring getWindowTagType()  ;
};

} //namespace

#endif /*SEARCHREPLACEDIALOG_H_*/
