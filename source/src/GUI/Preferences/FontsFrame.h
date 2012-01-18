/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef FONTSFRAME_H_
#define FONTSFRAME_H_

#include "PreferencesFrame.h"

namespace tag {
/**
 * @class 	FontsFrame
 * @ingroup	GUI
 *
 * Frame for consulting and editing editor font preferences
 *
 */
class FontsFrame : public PreferencesFrame
{
	public:

		/**
		 * Constructor
		 * @param config			Pointer on the Configuration object (instanciated in top level)
		 * @param parent			Parent window
		 * @param _dynamic_values	Pointer on a map where all dynamic values that have been modified are kept(out)\n
		 * 							Dynamic values are formed by a code and a value\n
		 * 							int: code of the option (see <em>define macros</em> in Configuration file documentation
		 * 							Glib::ustring:  new value for the option
		 * @param _static_values	Pointer on a vector the modified static value images are inserted to (out)\n
		 * 							The modified static value images are displayed in notebook header when a static preference
		 * 							is changed.
		 * @note					A static value is an option that needs the notebook page to be closed
		 * 							for the modification to be visible. in constrast the dynamic values are
		 * 							immediatly applied.
		 * @remarks					Some static values could be changed into dynamic values {evolution}
		 */
		FontsFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values);
		virtual ~FontsFrame();

		void reload_data() ;

	private:
		Glib::ustring editorFontChanged ;

		Gtk::Frame displayOptions_frame ;
			Gtk::VBox displayOptions_vbox ;
				Gtk::HBox editorFont_Hbox ;
					Gtk::Label editorFont_label ;
					Gtk::FontButton textFont_button ;
				Gtk::HBox labelFont_Hbox ;
					Gtk::Label labelFont_label ;
					Gtk::FontButton labelFont_button ;

		void on_editorOptionsFontButtons_changed(Glib::ustring option) ;

};

} //namespace

#endif /*FONTSFRAME_H_*/
