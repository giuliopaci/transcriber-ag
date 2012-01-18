/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef TEXTEDITORFRAME_H_
#define TEXTEDITORFRAME_H_

#include <gtkmm.h>
#include "PreferencesFrame.h"

//> editor font
#define LARGEFONT 			"Large"
#define MEDIUMFONT 			"Medium"
#define NORMALFONT 			"Normal"
#define MEDIUMFONT_VALUE 	"medium_editor_font"
#define LARGEFONT_VALUE 	"large_editor_font"
#define NORMALFONT_VALUE 	"normal_editor_font"

#define PREFERENCES_VIEWMODE_DUAL		_("dual display")
#define PREFERENCES_VIEWMODE_TRACK1		_("Track 1 display")
#define PREFERENCES_VIEWMODE_TRACK2		_("Track 2 display")
#define PREFERENCES_VIEWMODE_MERGED		_("merged display")
#define PREFERENCES_VIEWMODE_DUAL_VALUE		"dual"
#define PREFERENCES_VIEWMODE_TRACK1_VALUE	"track1"
#define PREFERENCES_VIEWMODE_TRACK2_VALUE	"track2"
#define PREFERENCES_VIEWMODE_MERGED_VALUE	"merged"

#define PREFERENCES_KEYDELETE_CONTROL _("Control + suppression key")
#define PREFERENCES_KEYDELETE_TRUE _("Allowed")
#define PREFERENCES_KEYDELETE_FALSE _("Forbidden")

#define PREFERENCES_HIGHLIGHT_BOTH			_("both tracks")
#define PREFERENCES_HIGHLIGHT_TRACK1		_("Track 1")
#define PREFERENCES_HIGHLIGHT_TRACK2		_("Track 2")
#define PREFERENCES_HIGHLIGHT_SELECTED		_("selected track")
#define PREFERENCES_HIGHLIGHT_NO			_("none")
#define PREFERENCES_HIGHLIGHT_BOTH_VALUE		"both"
#define PREFERENCES_HIGHLIGHT_TRACK1_VALUE		"track1"
#define PREFERENCES_HIGHLIGHT_TRACK2_VALUE		"track2"
#define PREFERENCES_HIGHLIGHT_SELECTED_VALUE	"selected"
#define PREFERENCES_HIGHLIGHT_NO_VALUE			"none"

namespace tag {
/**
 * @class 	TextEditorFrame
 * @ingroup	GUI
 *
 * Frame for consulting and editing transcription editor preferences
 *
 */
class TextEditorFrame : public PreferencesFrame
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
		TextEditorFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* dynamic_values, std::vector<IcoPackImage*>* static_values);
		virtual ~TextEditorFrame();
		void reload_data() ;

	private:

		Glib::ustring editorFontChanged ;

		//> DISPLAY
		Gtk::Frame displayOptions_frame ;
			Gtk::VBox displayOptions_vbox ;
				Gtk::HBox displayOptions_entityBg_Hbox ;
					Gtk::CheckButton displayOptions_entityBg_check ;
				Gtk::HBox displayOptions_tooltip_Hbox ;
					Gtk::CheckButton displayOptions_tooltip_check ;

		//> SYNCHRONISATION
		Gtk::Frame synchroOptions_frame ;
			Gtk::Expander editMode_expander ;
				Gtk::Frame editMode_frame ;
					Gtk::VBox editMode_VBox ;
						Gtk::HBox editMode_synchroStT_HBox ;
							Gtk::CheckButton editMode_synchroStT_checkbox ;
							IcoPackImage editMode_synchroStT_image ;
							IcoPackImage editMode_synchroStT_imageNO ;
						Gtk::HBox editMode_synchroTtS_HBox ;
							Gtk::CheckButton editMode_synchroTtS_checkbox ;
							IcoPackImage editMode_synchroTtS_image ;
							IcoPackImage editMode_synchroTtS_imageNO ;
						Gtk::HBox editMode_highlight_HBox ;
							Gtk::Label editMode_highlight_label ;
							Gtk::ComboBoxText editMode_highlight_combo ;
							IcoPackImage editMode_highlight_image ;
							IcoPackImage editMode_highlight_imageNO ;
			Gtk::VBox synchroOptions_vbox ;
				Gtk::Expander browseMode_expander ;
					Gtk::HBox browseMode_expander_labelHBox ;
						Gtk::Label browseMode_expander_labelLabel ;
						IcoPackImage browseMode_expander_labelImage ;
					Gtk::Frame browseMode_frame ;
						Gtk::VBox browseMode_VBox ;
							Gtk::HBox browseMode_synchroStT_HBox ;
								Gtk::CheckButton browseMode_synchroStT_checkbox ;
								IcoPackImage browseMode_synchroStT_image ;
								IcoPackImage browseMode_synchroStT_imageNO ;
							Gtk::HBox browseMode_synchroTtS_HBox ;
								Gtk::CheckButton browseMode_synchroTtS_checkbox ;
								IcoPackImage browseMode_synchroTtS_image ;
								IcoPackImage browseMode_synchroTtS_imageNO ;
							Gtk::HBox browseMode_highlight_HBox ;
								Gtk::Label browseMode_highlight_label ;
								Gtk::ComboBoxText browseMode_highlight_combo ;
								IcoPackImage browseMode_highlight_image ;
								IcoPackImage browseMode_highlight_imageNO ;



		//> INPUT OPTIONS FRAME
			Gtk::Frame inputOptions_frame ;
				Gtk::VBox inputOptions_VBox ;
					Gtk::HBox inputOptions_allowBrowseOnTags_HBox ;
						Gtk::CheckButton inputOptions_allowBrowseOnTags_check ;
						IcoPackImage warning_allowBrowseOnTags ;
					Gtk::HBox inputOptions_allowKeyDelete_HBox ;
						Gtk::Label inputOptions_allowKeyDelete_label ;
						Gtk::ComboBoxText inputOptions_allowKeyDelete_combo ;
					Gtk::HBox inputOptions_autosetLanguage_HBox ;
						Gtk::CheckButton inputOptions_autosetLanguage_check ;
						IcoPackImage inputOptions_autosetLanguage_image ;


		//> MONO FRAME
			Gtk::Frame mono_frame ;
				Gtk::VBox mono_vbox ;
				Gtk::HBox mono_tagDisplay_Hbox ;
					Gtk::CheckButton mono_tagDisplay_check ;
					IcoPackImage warning_mono_tagDisplay ;

		//> STEREO FRAME
			Gtk::Frame stereo_frame ;
				Gtk::VBox stereo_vbox ;
				Gtk::HBox stereo_tagDisplay_Hbox ;
					Gtk::CheckButton stereo_tagDisplay_check ;
					IcoPackImage warning_stereo_tagDisplay ;
			Gtk::HBox stereo_viewMode_HBox ;
				Gtk::Label stereo_viewMode_label ;
				Gtk::ComboBoxText stereo_viewMode_combo ;
				IcoPackImage stereo_viewMode_image ;
				IcoPackImage stereo_viewMode_imageDUAL ;


					Gtk::HBox mono_viewMode_HBox ;
						Gtk::Label mono_allowBrowseOnTags_label ;
						Gtk::CheckButton mono_allowBrowseOnTags_check ;
					Gtk::HBox mono_allowKeyDelete_HBox ;
						Gtk::Label mono_allowKeyDelete_label ;
						Gtk::ComboBoxText mono_allowKeyDelete_combo ;
					Gtk::HBox mono_autosetLanguage_HBox ;
						Gtk::CheckButton mono_autosetLanguage_check ;
						IcoPackImage mono_autosetLanguage_image ;



		void prepare_combos() ;
		void on_editorOptionCombos_changed(Glib::ustring option) ;
		void on_editorOptionCheckBox_changed(Glib::ustring option) ;

};

} //namespace

#endif /*TEXTEDITORFRAME_H_*/
