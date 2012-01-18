/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "TextEditorFrame.h"

namespace tag {

TextEditorFrame::TextEditorFrame(Configuration* _config, Gtk::Window* _parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values)
	: PreferencesFrame(_config, _parent, _("Text Editor"), _dynamic_values, _static_values)
{
	//> DISPLAY OPTIONS
	vbox.pack_start(displayOptions_frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		displayOptions_frame.set_label(_("Display options")) ;
		displayOptions_frame.add(displayOptions_vbox) ;
		displayOptions_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
			displayOptions_vbox.pack_start(displayOptions_entityBg_Hbox, false, false, 7) ;
				displayOptions_entityBg_Hbox.pack_start(displayOptions_entityBg_check, false, false, 5) ;
			displayOptions_vbox.pack_start(displayOptions_tooltip_Hbox, false, false, 7) ;
				displayOptions_tooltip_Hbox.pack_start(displayOptions_tooltip_check, false, false, 5) ;

	displayOptions_entityBg_check.set_label(_("Display colored background for named entity tags")) ;
	displayOptions_entityBg_check.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCheckBox_changed), "display-entity"));

	displayOptions_tooltip_check.set_label(_("Allow tooltips inside editor")) ;
	displayOptions_tooltip_check.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCheckBox_changed), "allow-tooltip"));


	//> INPUT OPTIONS
	vbox.pack_start(inputOptions_frame, false, false, 7) ;
		inputOptions_frame.add(inputOptions_VBox) ;
		inputOptions_frame.set_label(_("Input options")) ;
		inputOptions_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
			inputOptions_VBox.pack_start(inputOptions_allowKeyDelete_HBox, false, false, 5) ;
				inputOptions_allowKeyDelete_HBox.pack_start(inputOptions_allowKeyDelete_label, false, false, 5) ;
				inputOptions_allowKeyDelete_HBox.pack_start(inputOptions_allowKeyDelete_combo, false, false, 5) ;
//			inputOptions_VBox.pack_start(inputOptions_allowBrowseOnTags_HBox, false, false, 5) ;
//				inputOptions_allowBrowseOnTags_HBox.pack_start(inputOptions_allowBrowseOnTags_check, false, false, 5) ;
//				inputOptions_allowBrowseOnTags_HBox.pack_start(warning_allowBrowseOnTags, false, false, 5) ;
			inputOptions_VBox.pack_start(inputOptions_autosetLanguage_HBox, false, false, 5) ;
				inputOptions_autosetLanguage_HBox.pack_start(inputOptions_autosetLanguage_check, false, false, 5) ;
				inputOptions_autosetLanguage_HBox.pack_start(inputOptions_autosetLanguage_image, false, false, 5) ;

//	inputOptions_allowBrowseOnTags_check.set_label(_("Allow cursor deplacement on tags")) ;
//	inputOptions_allowBrowseOnTags_check.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCheckBox_changed), "allowBrowseOnTags"));
	inputOptions_allowKeyDelete_label.set_label(_("Delete time-anchored elements with suppression key (del/backspace)")) ;
	inputOptions_allowKeyDelete_combo.signal_changed().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCombos_changed), "allowKeyDelete")) ;
	inputOptions_autosetLanguage_check.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCheckBox_changed), "autosetLanguage"));
	inputOptions_autosetLanguage_check.set_label(_("Automatically set turn language as input language")) ;


	//> MONO OPTIONS
	vbox.pack_start(mono_frame, false, false, 7) ;
		mono_frame.set_label(_("Mono mode")) ;
		mono_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
		mono_frame.add(mono_vbox) ;
			mono_vbox.pack_start(mono_tagDisplay_Hbox, false, false, 7) ;
				mono_tagDisplay_Hbox.pack_start(mono_tagDisplay_check, false, false, 5) ;
				mono_tagDisplay_Hbox.pack_start(warning_mono_tagDisplay, false, false, 5) ;

	mono_tagDisplay_check.set_label(_("Display turn label and segment label on same line")) ;
	mono_tagDisplay_check.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCheckBox_changed), "display-turn-mono"));


	//> STEREO MODE
	vbox.pack_start(stereo_frame, false, false, 7) ;
		stereo_frame.set_label(_("Stereo mode")) ;
		stereo_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
		stereo_frame.add(stereo_vbox) ;
			stereo_vbox.pack_start(stereo_tagDisplay_Hbox, false, false, 7) ;
				stereo_tagDisplay_Hbox.pack_start(stereo_tagDisplay_check, false, false, 5) ;
				stereo_tagDisplay_Hbox.pack_start(warning_stereo_tagDisplay, false, false, 5) ;
			stereo_vbox.pack_start(stereo_viewMode_HBox, false, false, 7) ;
				stereo_viewMode_HBox.pack_start(stereo_viewMode_label, false, false, 5) ;
				stereo_viewMode_HBox.pack_start(stereo_viewMode_combo, false, false, 5) ;
				stereo_viewMode_HBox.pack_start(stereo_viewMode_image, false, false, 5) ;
				stereo_viewMode_HBox.pack_start(stereo_viewMode_imageDUAL, false, false, 5) ;

	stereo_tagDisplay_check.set_label(_("Display turn label and segment label on same line")) ;
	stereo_tagDisplay_check.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCheckBox_changed), "display-turn-stereo"));

	stereo_viewMode_label.set_label(_("Default view mode for stereo")) ;
	stereo_viewMode_combo.signal_changed().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCombos_changed), "stereo-viewmode")) ;


	//> SYNCHRONISTATION OPTIONS
	vbox.pack_start(synchroOptions_frame, false, false, 7) ;
		synchroOptions_frame.set_label(_("Default synchronization options")) ;
		synchroOptions_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
		synchroOptions_frame.add(synchroOptions_vbox) ;
			synchroOptions_vbox.pack_start(editMode_synchroStT_HBox, false, false, 7) ;
				editMode_synchroStT_HBox.pack_start(editMode_synchroStT_checkbox, false, false, 5) ;
				editMode_synchroStT_HBox.pack_start(editMode_synchroStT_image, false, false, 5) ;
				editMode_synchroStT_HBox.pack_start(editMode_synchroStT_imageNO, false, false, 5) ;
			synchroOptions_vbox.pack_start(editMode_synchroTtS_HBox, false, false, 7) ;
				editMode_synchroTtS_HBox.pack_start(editMode_synchroTtS_checkbox, false, false, 5) ;
				editMode_synchroTtS_HBox.pack_start(editMode_synchroTtS_image, false, false, 5) ;
				editMode_synchroTtS_HBox.pack_start(editMode_synchroTtS_imageNO, false, false, 5) ;
			synchroOptions_vbox.pack_start(editMode_highlight_HBox, false, false, 7) ;
				editMode_highlight_HBox.pack_start(editMode_highlight_combo, false, false, 5) ;
				editMode_highlight_HBox.pack_start(editMode_highlight_label, false, false, 5) ;
				editMode_highlight_HBox.pack_start(editMode_highlight_image, false, false, 5) ;
				editMode_highlight_HBox.pack_start(editMode_highlight_imageNO, false, false, 5) ;

			Gtk::Separator* sep = Gtk::manage(new Gtk::VSeparator())	;
			synchroOptions_vbox.pack_start(*sep , false, false, 5) ;

			synchroOptions_vbox.pack_start(browseMode_expander, false, false, 5) ;
				editMode_expander.set_expanded(true) ;
					browseMode_expander.set_label_widget(browseMode_expander_labelHBox) ;
						browseMode_expander_labelHBox.pack_start(browseMode_expander_labelLabel, false, false, 5) ;
							browseMode_expander_labelLabel.set_label(_("SET OPTIONS FOR READ ONLY MODE")) ;
						browseMode_expander_labelHBox.pack_start(browseMode_expander_labelImage, false, false, 5) ;
					browseMode_expander.add(browseMode_VBox) ;
						browseMode_VBox.pack_start(browseMode_synchroStT_HBox, false, false, 7) ;
							browseMode_synchroStT_HBox.pack_start(browseMode_synchroStT_checkbox, false, false, 5) ;
							browseMode_synchroStT_HBox.pack_start(browseMode_synchroStT_image, false, false, 5) ;
							browseMode_synchroStT_HBox.pack_start(browseMode_synchroStT_imageNO, false, false, 5) ;
						browseMode_VBox.pack_start(browseMode_synchroTtS_HBox, false, false, 7) ;
							browseMode_synchroTtS_HBox.pack_start(browseMode_synchroTtS_checkbox, false, false, 5) ;
							browseMode_synchroTtS_HBox.pack_start(browseMode_synchroTtS_image, false, false, 5) ;
							browseMode_synchroTtS_HBox.pack_start(browseMode_synchroTtS_imageNO, false, false, 5) ;
						browseMode_VBox.pack_start(browseMode_highlight_HBox, false, false, 7) ;
							browseMode_highlight_HBox.pack_start(browseMode_highlight_combo, false, false, 5) ;
							browseMode_highlight_HBox.pack_start(browseMode_highlight_label, false, false, 5) ;
							browseMode_highlight_HBox.pack_start(browseMode_highlight_image, false, false, 5) ;
							browseMode_highlight_HBox.pack_start(browseMode_highlight_imageNO, false, false, 5) ;
					browseMode_expander.set_expanded(false) ;


	editMode_synchroStT_checkbox.set_label(_("Synchronization signal to text")) ;
	editMode_synchroTtS_checkbox.set_label(_("Synchronization text to signal")) ;
	editMode_highlight_label.set_label(_("Highlight text at current position")) ;
	editMode_highlight_combo.signal_changed().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCombos_changed), "EDIT-highlight"));
	editMode_synchroTtS_checkbox.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCheckBox_changed), "EDIT-synchroTtS"));
	editMode_synchroStT_checkbox.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCheckBox_changed), "EDIT-synchroStT"));
	browseMode_synchroStT_checkbox.set_label(_("Synchronization signal to text")) ;
	browseMode_synchroTtS_checkbox.set_label(_("Synchronization text to signal")) ;
	browseMode_highlight_label.set_label(_("Highlight text at current position")) ;
	browseMode_highlight_combo.signal_changed().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCombos_changed), "READO-highlight"));
	browseMode_synchroTtS_checkbox.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCheckBox_changed), "READO-synchroTtS"));
	browseMode_synchroStT_checkbox.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &TextEditorFrame::on_editorOptionCheckBox_changed), "READO-synchroStT"));

	browseMode_highlight_image.set_image(ICO_HIGHLIGHT, 17) ;
	editMode_highlight_image.set_image(ICO_HIGHLIGHT, 17) ;
	browseMode_synchroStT_image.set_image(ICO_SYNC_SWT, 15) ;
	editMode_synchroStT_image.set_image(ICO_SYNC_SWT, 15) ;
	browseMode_synchroTtS_image.set_image(ICO_SYNC_TWS, 17) ;
	editMode_synchroTtS_image.set_image(ICO_SYNC_TWS, 17) ;
	stereo_viewMode_image.set_image(ICO_DISPLAY_UNIQUESCREEN, 17) ;
	browseMode_expander_labelImage.set_image(ICO_FILEMODE_BROWSE, 17) ;
	inputOptions_autosetLanguage_image.set_image(ICO_COMBOLANGUAGE_ICON, 17) ;

	browseMode_highlight_imageNO.set_image(ICO_HIGHLIGHT_DISABLED, 17) ;
	editMode_highlight_imageNO.set_image(ICO_HIGHLIGHT_DISABLED, 17) ;
	browseMode_synchroStT_imageNO.set_image(ICO_SYNC_SWT_DISABLED, 15) ;
	editMode_synchroStT_imageNO.set_image(ICO_SYNC_SWT_DISABLED, 15) ;
	browseMode_synchroTtS_imageNO.set_image(ICO_SYNC_TWS_DISABLED, 17) ;
	editMode_synchroTtS_imageNO.set_image(ICO_SYNC_TWS_DISABLED, 17) ;
	stereo_viewMode_imageDUAL.set_image(ICO_DISPLAY_TWOSCREEN, 17) ;

	warning_images.insert(warning_images.begin(), &warning_mono_tagDisplay) ;
	warning_images.insert(warning_images.begin(), &warning_stereo_tagDisplay) ;
	//warning_images.insert(warning_images.begin(), &warning_allowBrowseOnTags) ;


	//> get and display datas
	reload_data() ;
	modified(false) ;
	show_all_children() ;
}

TextEditorFrame::~TextEditorFrame()
{

}


//******************************************************************************
//*********************************** CALLBACK *********************************

/*
 * mode=0 for browse, 1 for edit
 */
void TextEditorFrame::on_editorOptionCombos_changed(Glib::ustring mode)
{
	if (lock_data)
		return ;

	modified(true);
	if (mode.compare("stereo-viewmode")==0)
	{
		Glib::ustring value = stereo_viewMode_combo.get_active_text() ;
		if (value==PREFERENCES_VIEWMODE_DUAL) {
			config->set_EDITOR_EditStereoViewMode(PREFERENCES_VIEWMODE_DUAL_VALUE, false) ;
			config->set_EDITOR_BrowseStereoViewMode(PREFERENCES_VIEWMODE_DUAL_VALUE, false) ;
		}
		else if (value==PREFERENCES_VIEWMODE_MERGED) {
			config->set_EDITOR_EditStereoViewMode(PREFERENCES_VIEWMODE_MERGED_VALUE, false) ;
			config->set_EDITOR_BrowseStereoViewMode(PREFERENCES_VIEWMODE_MERGED_VALUE, false) ;
		}
		else if (value==PREFERENCES_VIEWMODE_TRACK1) {
			config->set_EDITOR_EditStereoViewMode(PREFERENCES_VIEWMODE_TRACK1_VALUE, false) ;
			config->set_EDITOR_BrowseStereoViewMode(PREFERENCES_VIEWMODE_TRACK1_VALUE, false) ;
		}
		else if (value==PREFERENCES_VIEWMODE_TRACK2) {
			config->set_EDITOR_EditStereoViewMode(PREFERENCES_VIEWMODE_TRACK2_VALUE, false) ;
			config->set_EDITOR_BrowseStereoViewMode(PREFERENCES_VIEWMODE_TRACK2_VALUE, false) ;
		}
	}
	else if (mode.compare("allowKeyDelete")==0)
	{
		Glib::ustring value = inputOptions_allowKeyDelete_combo.get_active_text() ;
		if (value.compare(PREFERENCES_KEYDELETE_CONTROL)==0)
			config->set_EDITOR_EditAllowKeyDelete("control", false) ;
		else if (value.compare(PREFERENCES_KEYDELETE_TRUE)==0)
			config->set_EDITOR_EditAllowKeyDelete("true", false) ;
		else if (value.compare(PREFERENCES_KEYDELETE_FALSE)==0)
			config->set_EDITOR_EditAllowKeyDelete("false", false) ;
		set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_EDITOR_SUPPRESSSEGMENT, value) ;
	}
	else if (mode.compare("EDIT-highlight")==0)
	{
		Glib::ustring value = editMode_highlight_combo.get_active_text() ;
		if (value.compare(PREFERENCES_HIGHLIGHT_BOTH)==0)
			config->set_EDITOR_EditHighlight(PREFERENCES_HIGHLIGHT_BOTH_VALUE, false) ;
		else if (value.compare(PREFERENCES_HIGHLIGHT_SELECTED)==0)
			config->set_EDITOR_EditHighlight(PREFERENCES_HIGHLIGHT_SELECTED_VALUE, false) ;
		else if (value.compare(PREFERENCES_HIGHLIGHT_TRACK1)==0)
			config->set_EDITOR_EditHighlight(PREFERENCES_HIGHLIGHT_TRACK1_VALUE, false) ;
		else if (value.compare(PREFERENCES_HIGHLIGHT_TRACK2)==0)
			config->set_EDITOR_EditHighlight(PREFERENCES_HIGHLIGHT_TRACK2_VALUE, false) ;
		else if (value.compare(PREFERENCES_HIGHLIGHT_NO)==0)
			config->set_EDITOR_EditHighlight(PREFERENCES_HIGHLIGHT_NO_VALUE, false) ;
		set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_EDITOR_HIGHLIGHT, value) ;
	}
	else if (mode.compare("READO-highlight")==0)
	{
		Glib::ustring value = browseMode_highlight_combo.get_active_text() ;
		if (value.compare(PREFERENCES_HIGHLIGHT_BOTH)==0)
			config->set_EDITOR_BrowseHighlight(PREFERENCES_HIGHLIGHT_BOTH_VALUE, false) ;
		else if (value.compare(PREFERENCES_HIGHLIGHT_SELECTED)==0)
			config->set_EDITOR_BrowseHighlight(PREFERENCES_HIGHLIGHT_SELECTED_VALUE, false) ;
		else if (value.compare(PREFERENCES_HIGHLIGHT_TRACK1)==0)
			config->set_EDITOR_BrowseHighlight(PREFERENCES_HIGHLIGHT_TRACK1_VALUE, false) ;
		else if (value.compare(PREFERENCES_HIGHLIGHT_TRACK2)==0)
			config->set_EDITOR_BrowseHighlight(PREFERENCES_HIGHLIGHT_TRACK2_VALUE, false) ;
		else if (value.compare(PREFERENCES_HIGHLIGHT_NO)==0)
			config->set_EDITOR_BrowseHighlight(PREFERENCES_HIGHLIGHT_NO_VALUE, false) ;
		set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_EDITOR_HIGHLIGHT, value) ;
	}
}

void TextEditorFrame::on_editorOptionCheckBox_changed(Glib::ustring option)
{
	if (lock_data)
		return ;

	modified(true);
	//> read only
	if (option.compare("READO-synchroStT")==0)
			config->set_EDITOR_BrowseSynchroStT(browseMode_synchroStT_checkbox.get_active(), false) ;
	else if (option.compare("READO-synchroTtS")==0)
			config->set_EDITOR_BrowseSynchroTtS(browseMode_synchroTtS_checkbox.get_active(), false) ;
	//> edit mode
	else if (option.compare("EDIT-synchroStT")==0)
			config->set_EDITOR_EditSynchroStT(editMode_synchroStT_checkbox.get_active(), false) ;
	else if (option.compare("EDIT-synchroTtS")==0)
			config->set_EDITOR_EditSynchroTtS(editMode_synchroTtS_checkbox.get_active(), false) ;
/*	else if (option.compare("allowBrowseOnTags")==0) {
			config->set_EDITOR_EditAllowBrowseOnTags(inputOptions_allowBrowseOnTags_check.get_active(), false) ;
			bset_dynamic_value(TAG_PREFERENCES_PARAM_EDITOR_ALLOWBROWSEONTAGS, inputOptions_allowBrowseOnTags_check.get_active()) ;
	}*/
	else if (option.compare("display-turn-mono")==0) {
			config->set_EDITOR_newLineAfterTurnLabelMono(!(mono_tagDisplay_check.get_active()), false) ;
			set_warnings(&warning_mono_tagDisplay,1) ;
	}
	else if (option.compare("display-turn-stereo")==0) {
			config->set_EDITOR_newLineAfterTurnLabelStereo(!(stereo_tagDisplay_check.get_active()), false) ;
			set_warnings(&warning_stereo_tagDisplay,1) ;
	}
	else if (option.compare("autosetLanguage")==0) {
			config->set_EDITOR_autosetLanguage(inputOptions_autosetLanguage_check.get_active(), false) ;
			set_formatted_boolean_dynamic_value(TAG_PREFERENCES_PARAM_AUTOSETLANGUAGE, inputOptions_autosetLanguage_check.get_active() ) ;
	}
	else if (option.compare("display-entity")==0) {
			config->set_EDITOR_entitiesBg(displayOptions_entityBg_check.get_active(), false) ;
			set_formatted_boolean_dynamic_value(TAG_PREFERENCES_PARAM_EDITOR_ENTITYBG, displayOptions_entityBg_check.get_active() ) ;
	}
	else if (option.compare("allow-tooltip")==0) {
			config->set_EDITOR_displayQualifierTooltips(displayOptions_tooltip_check.get_active(), false) ;
			set_formatted_boolean_dynamic_value(TAG_PREFERENCES_PARAM_EDITOR_TOOLTIP, displayOptions_tooltip_check.get_active() ) ;
}
}


//******************************************************************************
//******************************** PREPARE DATAS *******************************

void TextEditorFrame::prepare_combos()
{
	//> EDIT
	stereo_viewMode_combo.clear() ;
	stereo_viewMode_combo.append_text(PREFERENCES_VIEWMODE_DUAL) ;
	stereo_viewMode_combo.append_text(PREFERENCES_VIEWMODE_MERGED) ;
	stereo_viewMode_combo.append_text(PREFERENCES_VIEWMODE_TRACK1) ;
	stereo_viewMode_combo.append_text(PREFERENCES_VIEWMODE_TRACK2) ;
	Glib::ustring editVM = config->get_EDITOR_EditStereoViewMode() ;
	if ( editVM.compare(PREFERENCES_VIEWMODE_DUAL_VALUE)==0 )
		stereo_viewMode_combo.set_active_text(PREFERENCES_VIEWMODE_DUAL) ;
	else if ( editVM.compare(PREFERENCES_VIEWMODE_MERGED_VALUE)==0 )
		stereo_viewMode_combo.set_active_text(PREFERENCES_VIEWMODE_MERGED) ;
	else if ( editVM.compare(PREFERENCES_VIEWMODE_TRACK1_VALUE)==0 )
		stereo_viewMode_combo.set_active_text(PREFERENCES_VIEWMODE_TRACK1) ;
	else if ( editVM.compare(PREFERENCES_VIEWMODE_TRACK2_VALUE)==0 )
		stereo_viewMode_combo.set_active_text(PREFERENCES_VIEWMODE_TRACK2) ;

	//> ALLOW KEY DELETE
	inputOptions_allowKeyDelete_combo.clear() ;
	inputOptions_allowKeyDelete_combo.append_text(PREFERENCES_KEYDELETE_TRUE) ;
	inputOptions_allowKeyDelete_combo.append_text(PREFERENCES_KEYDELETE_FALSE) ;
	inputOptions_allowKeyDelete_combo.append_text(PREFERENCES_KEYDELETE_CONTROL) ;
	Glib::ustring value = config->get_EDITOR_EditAllowKeyDelete() ;
	if (value.compare("true")==0)
		inputOptions_allowKeyDelete_combo.set_active_text(PREFERENCES_KEYDELETE_TRUE) ;
	else if (value.compare("false")==0)
		inputOptions_allowKeyDelete_combo.set_active_text(PREFERENCES_KEYDELETE_FALSE) ;
	else if (value.compare("control")==0)
		inputOptions_allowKeyDelete_combo.set_active_text(PREFERENCES_KEYDELETE_CONTROL) ;

	//> HIGHLIGHT
	editMode_highlight_combo.clear() ;
	editMode_highlight_combo.append_text(PREFERENCES_HIGHLIGHT_BOTH) ;
	editMode_highlight_combo.append_text(PREFERENCES_HIGHLIGHT_NO) ;
	editMode_highlight_combo.append_text(PREFERENCES_HIGHLIGHT_SELECTED) ;
	editMode_highlight_combo.append_text(PREFERENCES_HIGHLIGHT_TRACK1) ;
	editMode_highlight_combo.append_text(PREFERENCES_HIGHLIGHT_TRACK2) ;
	value = config->get_EDITOR_EditHighlight() ;
	if (value.compare(PREFERENCES_HIGHLIGHT_NO_VALUE)==0)
		editMode_highlight_combo.set_active_text(PREFERENCES_HIGHLIGHT_NO) ;
	else if (value.compare(PREFERENCES_HIGHLIGHT_BOTH_VALUE)==0)
		editMode_highlight_combo.set_active_text(PREFERENCES_HIGHLIGHT_BOTH) ;
	else if (value.compare(PREFERENCES_HIGHLIGHT_TRACK1_VALUE)==0)
		editMode_highlight_combo.set_active_text(PREFERENCES_HIGHLIGHT_TRACK1) ;
	else if (value.compare(PREFERENCES_HIGHLIGHT_TRACK2_VALUE)==0)
		editMode_highlight_combo.set_active_text(PREFERENCES_HIGHLIGHT_TRACK2) ;
	else if (value.compare(PREFERENCES_HIGHLIGHT_SELECTED_VALUE)==0)
		editMode_highlight_combo.set_active_text(PREFERENCES_HIGHLIGHT_SELECTED) ;

	browseMode_highlight_combo.clear() ;
	browseMode_highlight_combo.append_text(PREFERENCES_HIGHLIGHT_BOTH) ;
	browseMode_highlight_combo.append_text(PREFERENCES_HIGHLIGHT_NO) ;
	browseMode_highlight_combo.append_text(PREFERENCES_HIGHLIGHT_SELECTED) ;
	browseMode_highlight_combo.append_text(PREFERENCES_HIGHLIGHT_TRACK1) ;
	browseMode_highlight_combo.append_text(PREFERENCES_HIGHLIGHT_TRACK2) ;
	value = config->get_EDITOR_BrowseHighlight() ;
	if (value.compare(PREFERENCES_HIGHLIGHT_NO_VALUE)==0)
		browseMode_highlight_combo.set_active_text(PREFERENCES_HIGHLIGHT_NO) ;
	else if (value.compare(PREFERENCES_HIGHLIGHT_BOTH_VALUE)==0)
		browseMode_highlight_combo.set_active_text(PREFERENCES_HIGHLIGHT_BOTH) ;
	else if (value.compare(PREFERENCES_HIGHLIGHT_TRACK1_VALUE)==0)
		browseMode_highlight_combo.set_active_text(PREFERENCES_HIGHLIGHT_TRACK1) ;
	else if (value.compare(PREFERENCES_HIGHLIGHT_TRACK2_VALUE)==0)
		browseMode_highlight_combo.set_active_text(PREFERENCES_HIGHLIGHT_TRACK2) ;
	else if (value.compare(PREFERENCES_HIGHLIGHT_SELECTED_VALUE)==0)
		browseMode_highlight_combo.set_active_text(PREFERENCES_HIGHLIGHT_SELECTED) ;
}



void TextEditorFrame::reload_data()
{
	lock_data = true ;

	//> EDITOR MODE
	//BrowseMode check buttons
	int synchroTtS = config->get_EDITOR_BrowseSynchroTtS() ;
	int synchroStT = config->get_EDITOR_BrowseSynchroStT() ;
	set_check_state(browseMode_synchroTtS_checkbox, synchroTtS) ;
	set_check_state(browseMode_synchroStT_checkbox, synchroStT) ;

	//EditMode check buttons
	synchroTtS = config->get_EDITOR_EditSynchroTtS() ;
	synchroStT = config->get_EDITOR_EditSynchroStT() ;
	set_check_state(editMode_synchroTtS_checkbox, synchroTtS) ;
	set_check_state(editMode_synchroStT_checkbox, synchroStT) ;

	// input options
/*	bool allowBrowseOnTags =  config->get_EDITOR_EditAllowBrowseOnTags() ;
	inputOptions_allowBrowseOnTags_check.set_active(allowBrowseOnTags) ;
*/
	// display mono
	int newLineAfterTurnLabel = config->get_EDITOR_newLineAfterTurnLabelMono() ;
	set_check_state(mono_tagDisplay_check, !newLineAfterTurnLabel) ;

	// display stereo
	newLineAfterTurnLabel = config->get_EDITOR_newLineAfterTurnLabelStereo() ;
	set_check_state(stereo_tagDisplay_check, !newLineAfterTurnLabel) ;

	int entityBg = config->get_EDITOR_entitiesBg() ;
	set_check_state(displayOptions_entityBg_check, entityBg) ;

	int tooltip = config->get_EDITOR_displayQualifierTooltips() ;
	set_check_state(displayOptions_tooltip_check, tooltip) ;

	// context menu
	int autosetLanguage = config->get_EDITOR_autosetLanguage() ;
	set_check_state(inputOptions_autosetLanguage_check, autosetLanguage) ;

	// browse/Edit mode combo
	prepare_combos() ;
	lock_data = false ;
}

} //namespace
