/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef DATAMODELFRAME_H_
#define DATAMODELFRAME_H_

#include <gtkmm.h>
#include "PreferencesFrame.h"

namespace tag {
/**
 * @class 	DataModelFrame
 * @ingroup	GUI
 *
 * Frame for consulting and editing Colors preferences
 *
 */
class DataModelFrame : public PreferencesFrame
{
	public:
		/**
		 * Constructor
		 * @param config			Pointer on the Configuration object (instanciated in top level)
		 * @param parent			Parent window
		 * @param dynamic_values	Pointer on a map where all dynamic values that have been modified are kept(out)\n
		 * 							Dynamic values are formed by a code and a value\n
		 * 							int: code of the option (see <em>define macros</em> in Configuration file documentation
		 * 							Glib::ustring:  new value for the option
		 * @param static_values	Pointer on a vector the modified static value images are inserted to (out)\n
		 * 							The modified static value images are displayed in notebook header when a static preference
		 * 							is changed.
		 * @note					A static value is an option that needs the notebook page to be closed
		 * 							for the modification to be visible. in constrast the dynamic values are
		 * 							immediatly applied.
		 * @remarks					Some static values could be changed into dynamic values {evolution}
		 */
		DataModelFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* dynamic_values, std::vector<IcoPackImage*>* static_values) ;
		/*! destructor */
		virtual ~DataModelFrame();
		/*! reload data */
		void reload_data() ;

	private:
		bool lock_combo ;
		Gtk::Frame convention_frame ;
		Gtk::VBox convention_vbox ;

		Gtk::HBox conv_hbox ;
			Gtk::Label conv_label ;
			Gtk::ComboBoxText conv_combo ;
		Gtk::HBox lang_hbox ;
			Gtk::Label lang_label ;
			Gtk::ComboBoxText lang_combo ;

		void on_convention_combo_changed(Glib::ustring type) ;
		void prepare_combo() ;
};

} //namespace

#endif /*DATAMODELFRAME_H_*/
