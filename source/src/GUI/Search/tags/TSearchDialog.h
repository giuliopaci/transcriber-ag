/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef TSEARCHDIALOG_H_
#define TSEARCHDIALOG_H_

#include <gtkmm.h>
#include "TSearchGeneral.h"
#include "Configuration.h"
#include "Common/icons/IcoPackImage.h"
#include "Common/widgets/GeoWindow.h"

namespace tag {

/**
 * @class 	TSearchDialog
 * @ingroup	GUI
 *
 * Search component for tag search dialog mode
 *
 */
class TSearchDialog : public TSearchGeneral, public Gtk::Dialog, public GeoWindow
{
	public:
		/**
		 * Constructor
		 * @return
		 */
		TSearchDialog(Configuration* configuration);
		virtual ~TSearchDialog();

		/* Abstract method implentation */
		void mySet_focus() ;

		/*** Geometry interface ***/
		virtual void saveGeoAndHide() ;
		virtual int loadGeoAndDisplay(bool rundlg=false) ;

	private:
		//> widgets
		Gtk::HBox  hbox_gen ;

		Gtk::Alignment align_header ;
		Gtk::Alignment align_gen ;
		Gtk::Alignment align_type ;
		Gtk::Alignment align_value ;
		Gtk::Alignment align_button ;
		Gtk::Alignment align_end ;

		Gtk::Frame frame_type ;
		Gtk::Frame frame_value ;

		Gtk::HBox hbox_header ;
		Gtk::VBox vbox_type ;
		Gtk::HBox vbox_value ;
		Gtk::HBox hbox_value ;
		Gtk::HButtonBox bbox_find ;

		Configuration* config ;

		void set_widgets_label() ;
		void prepare_gui()  ;

		void display_info(Glib::ustring text) ;
		int display_question(Glib::ustring text) ;
		void myHide() ;
		void on_change_mode() ;

		virtual bool on_key_press_event(GdkEventKey* event) ;

		void set_ico_searching() ;

		/*** Geometry interface ***/
		virtual void getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel) ;
		virtual Glib::ustring getWindowTagType()  ;
};

} //namespace

#endif /*TSEARCHDIALOG_H_*/
