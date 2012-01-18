/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "FontsFrame.h"

namespace tag {

FontsFrame::FontsFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values)
: PreferencesFrame(config, parent, _("Fonts"), _dynamic_values, _static_values)
{
	editorFontChanged = "" ;
	vbox.pack_start(displayOptions_frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		displayOptions_frame.set_label(_("Display options")) ;
		displayOptions_frame.add(displayOptions_vbox) ;
		displayOptions_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
			displayOptions_vbox.pack_start(editorFont_Hbox, false, false, 7) ;
				editorFont_Hbox.pack_start(editorFont_label, false, false, 5) ;
				editorFont_Hbox.pack_start(textFont_button, false, false, 5) ;
			displayOptions_vbox.pack_start(labelFont_Hbox, false, false, 7) ;
				labelFont_Hbox.pack_start(labelFont_label, false, false, 5) ;
				labelFont_Hbox.pack_start(labelFont_button, false, false, 5) ;

	editorFont_label.set_label(_("Editor text font")) ;
	labelFont_label.set_label(_("Editor labels font")) ;
	textFont_button.signal_font_set().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &FontsFrame::on_editorOptionsFontButtons_changed), "text")) ;
	labelFont_button.signal_font_set().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &FontsFrame::on_editorOptionsFontButtons_changed), "label")) ;
	labelFont_button.set_use_font(false) ;
	labelFont_button.set_show_style(false) ;

	#ifdef WIN32
	// -- Focus --
	labelFont_button.set_focus_on_click(true);
	textFont_button.set_focus_on_click(true);
	#endif

	reload_data() ;
	modified(false) ;
}

FontsFrame::~FontsFrame()
{
}


void FontsFrame::on_editorOptionsFontButtons_changed(Glib::ustring mode)
{
	if (mode.compare("text")==0) {
		modified(true) ;
		config->set_EDITOR_textFontStyle(textFont_button.get_font_name(), false) ;
		set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_EDITOR_TEXTFONT, textFont_button.get_font_name()) ;
	}
	else if (mode.compare("label")==0) {
		modified(true) ;
		config->set_EDITOR_labelFontStyle(labelFont_button.get_font_name(), false) ;
		set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_EDITOR_LABELFONT, labelFont_button.get_font_name()) ;
	}
}


void FontsFrame::reload_data()
{
	// fonts
	Glib::ustring font = config->get_EDITOR_textFontStyle() ;
	textFont_button.set_font_name(font) ;
	font = config->get_EDITOR_labelFontStyle() ;
	labelFont_button.set_font_name(font) ;
}

} //namespace
