/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef COLORSFRAME_H_
#define COLORSFRAME_H_

#include "PreferencesFrame.h"
#include "Common/widgets/DynamicTable.h"

namespace tag {

/**
 * @class 	ColorsFrame
 * @ingroup	GUI
 *
 * Frame for consulting and editing Colors preferences
 *
 */
class ColorsFrame : public PreferencesFrame
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
		ColorsFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values);

		virtual ~ColorsFrame();

		void reload_data() ;

	private:
		class UnitFgBg
		{
			public:
				UnitFgBg (Glib::ustring _name, bool with_fg, bool with_bg, Glib::ustring id) {
					set(_name, with_fg, with_bg, id) ;
				}

				void set(Glib::ustring _name, bool with_fg, bool with_bg, Glib::ustring _id) {
					name.set_label(_name) ; fg = with_fg ; bg = with_bg ;	id = _id ;
 				}

				Glib::ustring get_color(Glib::ustring type) {
					std::vector<Gdk::Color> colors ;
					if (type=="fg")
						colors.push_back(color_fg.get_color()) ;
					else
						colors.push_back(color_bg.get_color()) ;
					return Gtk::ColorSelection::palette_to_string(colors) ;
				}

				void set_color(Glib::ustring type, Glib::ustring s_color) {
					if (type=="fg")
						color_fg.set_color(Gdk::Color(s_color)) ;
					else
						color_bg.set_color(Gdk::Color(s_color)) ;
				}

				~UnitFgBg() {} ;
				Gtk::Label name ;
				Gtk::ColorButton color_fg ;
				Gtk::ColorButton color_bg ;
				Gtk::Label blank_fg ;
				Gtk::Label blank_bg ;
				bool fg ;
				bool bg ;
				Glib::ustring id ;
		};


		//**************************************************** TEXT
		Gtk::Expander editor_expander ;
			IcoPackImage editor_expander_label ;
			Gtk::VBox editor_vbox ;

		std::map<Glib::ustring, UnitFgBg*> units ;


/*		//LABELS
		Gtk::Frame editor_labels_frame ;
			Gtk::VBox editor_labels_vbox ;
				DynamicTable* editor_label_table ; */

		//TEXT
		Gtk::Frame editor_text_frame ;
			Gtk::VBox editor_text_vbox ;
				Gtk::HBox editor_text_hbox ;
					DynamicTable* editor_text_table ;

		//EVENTS
		Gtk::Frame editor_event_frame ;
			Gtk::VBox editor_event_vbox ;
				Gtk::HBox editor_event_hbox ;
					DynamicTable* editor_event_table ;
					IcoPackImage warning_event ;

		//ENTITIES
		Gtk::Frame editor_entity_frame ;
			Gtk::VBox editor_entity_vbox ;
				Gtk::HBox editor_entity_hbox ;
					DynamicTable* editor_entity_table ;
					IcoPackImage warning_entity ;

		//BACKGROUND
		Gtk::Frame editor_background_frame ;
			Gtk::VBox editor_background_vbox ;
				Gtk::HBox editor_background_hbox ;
					DynamicTable* editor_background_table ;
					IcoPackImage warning_background ;

		//HIGHLIGHT
		Gtk::Frame editor_highlight_frame ;
			Gtk::VBox editor_highlight_vbox ;
				Gtk::HBox editor_highlight_hbox ;
					DynamicTable* editor_highlight_table ;
					IcoPackImage warning_highlight ;

		//**************************************************** AUDIO
		Gtk::Expander audio_expander ;
			IcoPackImage audio_expander_label ;
			Gtk::VBox audio_vbox ;

		//SIGNAL
		Gtk::Frame audio_signal_frame ;
			Gtk::VBox audio_signal_vbox ;
				DynamicTable* audio_signal_table ;

		void on_colorButtons_change(ColorsFrame::UnitFgBg* unit, Glib::ustring type) ;

		void add_color_table(Gtk::Box& container, DynamicTable* dtable, Glib::ustring type) ;
		std::vector<ColorsFrame::UnitFgBg*> get_colors_units(Glib::ustring type) ;
		ColorsFrame::UnitFgBg* get_unit(Glib::ustring element_type) ;
};

} //namespace

#endif /*COLORSFRAME_H_*/
