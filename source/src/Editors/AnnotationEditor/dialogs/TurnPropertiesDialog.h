/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 	TurnPropertiesDialog.h
 */

#ifndef __HAVE_TurnPropertiesDialog__
#define __HAVE_TurnPropertiesDialog__

#include "Editors/AnnotationEditor/dialogs/AnnotationPropertiesDialog.h"

#include "Common/widgets/FieldEntry.h"

using namespace std;


namespace tag {
/**
 * @class TurnPropertiesDialog
 *
 * Dialog used for displaying and editing turn properties.
 */
class TurnPropertiesDialog : public AnnotationPropertiesDialog
{
	public:
		/**
		 * Constructor
		 * @param p_parent		Reference on window parent
		 * @param model			Reference on parent editor model
		 * @param id			Turn id
		 * @param editable		True for editable mode, false otherwise
		 */
		TurnPropertiesDialog(Gtk::Window& p_parent, DataModel& model, const std::string& id, bool editable=true);

		/**
		 * Destructor
		 */
		virtual ~TurnPropertiesDialog();


		/**
		 * Signal emitted when turn language is modified
		 * <b>string parameter:</b>			Language Iso639-3 code newly selected
		 */
		sigc::signal<void, string>& signalTurnLanguageChanged() { return  m_signalTurnLanguageChanged; }

	private:
		// ARCHITECTURE
		Gtk::SpinButton* a_trackEntry;
		FieldEntry* a_startTimeEntry;
		Gtk::Entry* a_startSecondsEntry;
		FieldEntry* a_endTimeEntry;
		Gtk::Entry* a_endSecondsEntry;
		Gtk::CheckButton* a_overlappingNoise;
		Gtk::RadioButton* a_speech;
		Gtk::CheckButton* a_overlappingSpeech;
		Gtk::ComboBoxText* a_speakerCombo;
		Gtk::ComboBoxText* a_languageCombo;
		Gtk::ComboBoxText* a_dialectCombo;
		Gtk::RadioButton* a_noSpeech;
		Gtk::ComboBoxText* a_noSpeechCombo;
		Gtk::Button* a_dictionary;
		Gtk::Label* a_lspeaker;
		Gtk::Label* a_llanguage;
		Gtk::Label* a_ldialect;

		Gtk::HBox defaultSpeaker_hbox ;
		Gtk::CheckButton* defaultSpeaker_checkBox ;
		Gtk::Label defaultSpeaker_label ;

		Gtk::Label** a_labels;
		Gtk::Widget** a_entries;

		// DATA

		tag::SignalSegment a_signalSegment;
		tag::SpeakerDictionary* a_speakerDictionary;
		list<Property> a_properties;
		std::map<string, list<string> > a_choiceLists;
		string id ;

		bool mainstreamBaseType ;
		bool segmentBaseType ;

		float start ;
		float end ;
		int track ;
		bool isSpeech ;

		bool combo_motion_is_locked ;

		void on_combos_changed(string id) ;
		void prepare_languages_combo() ;
		void prepare_speakers_combo() ;
		void on_comboLanguage_changed();

		void prepare_gui(string type) ;
		void prepare_data(string type) ;
		void display_error() ;
		void onButtonClicked(int p_id);
		void updateGUI();

		sigc::signal<void, string> m_signalTurnLanguageChanged ;
};

}

#endif // __HAVE_TurnPropertiesDialog__
