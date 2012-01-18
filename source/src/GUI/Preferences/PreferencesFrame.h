/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/** @file */

#ifndef PREFERENCESFRAME_H_
#define PREFERENCESFRAME_H_

#include <gtkmm.h>
#include "Configuration.h"
#include "Common/globals.h"
#include "Explorer_dialog.h"
#include "Common/icons/IcoPackImage.h"
#include "Common/icons/Icons.h"

#define TAG_PREFERENCESFRAME_SPACE 9

namespace tag {
/**
 * @class 	PreferencesFrame
 * @ingroup	GUI
 *
 * Frame for consulting and editing application global preferences
 *
 */
class PreferencesFrame : public Gtk::Frame
{
	public :
		/**
		 * Constructor
		 * @param config			Pointer on the Configuration object (instanciated in top level)
		 * @param parent			Parent window
		 * @param title				Title of the frame
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
		PreferencesFrame(Configuration* config, Gtk::Window* parent, Glib::ustring title, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values);
		virtual ~PreferencesFrame();

		/**
		 * Abstract method\n
		 * Refresh interface with configuration values.
		 */
		virtual void reload_data() = 0 ;

		/**
		 * Display static values image
		 * @param visible		True for displaying the static value changement
		 */
		void set_warnings_visible(bool visible) ;

		/**
		 * Format string boolean value to boolean value
		 * @param value		string boolean value
		 * @return			value in boolean format
		 */
		static bool get_formatted_bool_dynamic_value(Glib::ustring value) ;

		/**
		 * Format string integer value to interger value
		 * @param value		string interger value
		 * @return			value in integer format
		 */
		static int get_formatted_int_dynamic_value(Glib::ustring value) ;

		/**
		 * Signal emitted when modification state has changed\n
		 * <b>bool parameter:</b> 		True if status became modified, False if status became up to date.
		 * @return						The corresponfing signal
		 */
		sigc::signal<void,bool>& signalIsModified() { return m_signalIsModified; }


	protected :

		Gtk::Tooltips tooltip ;

		//> GUI
		Gtk::VBox vbox_display ;
			Gtk::HBox hbox_display ;
				Gtk::Alignment align ;
				Gtk::Label label ;
				Gtk::ScrolledWindow scrolledw ;
					Gtk::VBox scrolled_vbox ;
					Gtk::HBox scrolled_hbox ;
						//>>>> VBOX FOR DISPLAYING ALL FRAMES NEED
						Gtk::VBox vbox ;

		//> ACTION BUTTONS
		Gtk::Button apply ;
		Gtk::Button cancel ;


		/**
		 * @var lock_data
		 * Variable used to lock data modification when an operation is being
		 * processed
		 */
		bool lock_data ;

		/**
		 * @var warning_images
		 * Vector used to keep Pointers on all Warning images created
		 * @note an image is created for each preference but isn't displayed unless
		 * a static value is changed
		 */
		std::vector<IcoPackImage*> warning_images ;

		/**
		 * @var dynamic_values
		 * Map used to stock all preferences values that were modified and
		 * whose modification is dynamic\n
		 * int member:	preferences code - see <em># define</em> in Configuration file documentation
		 * Glib::ustring:  newly applied value
		 */
		std::map<int, Glib::ustring>* dynamic_values ;

		/**
		 * @var static_values
		 * Vector used to stock all warning_images that have been activated
		 */
		std::vector<IcoPackImage*>* static_values ;

		/**
		 * @var config
		 * Pointer on the Configuration object (created in top level)
		 */
		Configuration* config ;

		/**
		 * @var parent
		 * Pointer on the parent window
		 */
		Gtk::Window* parent ;

		/**
		 * @var m_signalIsModified
		 * Signal emitted when modification state has changed\n
		 * <b>bool parameter:</b> 		True if status became modified, False if status became up to date.
		 */
		sigc::signal<void,bool> m_signalIsModified ;

		/**
		 * Wrapper for Gtk::CheckBox state
		 * @param check					Pointer on the Gtk::CheckButton to update
		 * @param configuration_mode	Status to set to the button\n
		 * 								1: 		set activated\n
		 * 								0: 		set deactivated\n
		 * 								other:	set insensitive
		 * @note In most of the cases, configuration_mode is set to other value if
		 * 		 the corresponding property couldn't be found in configuration file
		 * 		 (bug)
		 */
		void set_check_state(Gtk::CheckButton& check, int configuration_mode) ;

		/**
		 * Format boolean value to preferences string value
		 * and update it in the dynamic_values map
		 * for the given preferences code
		 * @param param		Preferences code - see <i># define</i> in Configuration file documentation
		 * @param value		The formatted preferences string
		 */
		void set_formatted_boolean_dynamic_value(int param, bool value) ;

		/**
		 * Format integer value to preferences string value
		 * and update it in the dynamic_values map
		 * for the given preferences code
		 * @param param		Preferences code - see <i># define</i> in Configuration file documentation
		 * @param value		The formatted preferences string
		 */
		void set_formatted_integer_dynamic_value(int param, int value) ;

		/**
		 * Format string value to preferences string value
		 * and update it in the dynamic_values map
		 * for the given preferences code
		 * @param param		Preferences code - see <i># define</i> in Configuration file documentation
		 * @param value		The formatted preferences string
		 */
		void set_formatted_string_dynamic_value(int param, Glib::ustring value) ;

		/**
		 * Set the modified status
		 * @param ismodified	True if some frame options have been changed
		 */
		void modified(bool ismodified) ;

		/**
		 * Activate warning display for given image
		 * @param image		Image on which to set the warning icon
		 * @param level		Level of the warning (deprecated - 1 by default)
		 */
		void set_warnings(IcoPackImage* image, int level) ;


	private:
		void on_apply_clicked() ;
		void on_cancel_clicked() ;
};

} //namespace

#endif /*PREFERENCESFRAME_H_*/
