/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 	DialogFileProperties.h
 */
#ifndef __HAVE_DIALOGFILEPROPERTIES__
#define __HAVE_DIALOGFILEPROPERTIES__

#include <gtkmm.h>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Editors/AnnotationEditor/handlers/SAXAnnotationsHandler.h"
#include "Editors/AnnotationEditor/dialogs/DialogMore.h"
#include "Editors/AnnotationEditor/StatsWidget.h"
#include "Editors/AnnotationEditor/AnnotationEditor.h"

#include "Common/Dialogs.h"
#include "Common/Parameters.h"
#include "Common/Languages.h"
#include "Common/widgets/FieldEntry.h"
#include "Common/widgets/GeoWindow.h"

using namespace std;

namespace tag {

/**
 * @class DialogFileProperties
 *
 * Dialogs for displaying and editing the file annotation properties
 */
class DialogFileProperties : public Gtk::Dialog, public GeoWindow
{
	public:

		/**
		 * Constructor
		 * @param p_parent						Reference on parent window
		 * @param p_editor						Pointer on parent editor
		 * @param p_displayAnnotationTime		True for displaying annotation time on dialog "more"
		 * @param p_params						Pointer on the parameters map
		 * @param p_editable					True for editable mode, False otherwise
		 */
		DialogFileProperties(Gtk::Window& p_parent, AnnotationEditor* p_editor, bool p_displayAnnotationTime, Parameters& p_params, bool p_editable);
		virtual ~DialogFileProperties();

		/**
		 * Modifies dialog editable property
		 * @param value		True for edition mode, False otherwise
		 */
		void set_editability(bool value) ;

		/**
		 * Indicates whether the file needs to be reopen for the modification to
		 * be applied.
		 */
		bool getNeedReload() { return need_reload ; }

		/**
		 * Indicates whether modification have been applied
		 */
		bool getNeedSave() { return need_save ; }

		/*** Geometry interface ***/
		virtual void saveGeoAndHide() ;
		virtual int loadGeoAndDisplay(bool rundlg=false) ;

	private:

		list<string> SIGNAL_PROPERTIES;

		Gtk::Entry* corpusEntry;
		Gtk::Entry* corpusVersionEntry;

		Gtk::ComboBoxText* a_transcriptionCombo;
		Gtk::CheckButton* a_transcription;
		Gtk::Label* a_language;
		Gtk::ComboBoxText* a_languageCombo;
		Gtk::Label* a_dialect;
		Gtk::ComboBoxText* a_dialectCombo;
		Gtk::CheckButton* a_namedEntities;
		Gtk::CheckButton* a_themes;
		Gtk::Button* a_addSignal;
		Gtk::Button* a_removeSignal;
		Gtk::HBox* a_place;
	 	Gtk::Notebook* a_notebook;
		Gtk::Entry* a_versionEntry;
		FieldEntry* a_creationEntry;
		Gtk::Entry* a_byEntry;
		Glib::RefPtr<Gtk::TextBuffer> a_tbuffer;
		Gtk::TextView* a_commentEntryTV;
		FieldEntry* a_lastModEntry;
		Gtk::Entry* a_byEntry2;
		Glib::RefPtr<Gtk::TextBuffer> a_tbuffer2;
		Gtk::TextView* a_commentEntryTV2;
		Glib::RefPtr<Gtk::TextBuffer> a_tbuffer3;
		Gtk::TextView* a_commentEntryTV3;
		Gtk::Entry* a_typeEntrySignal1;
		Gtk::Entry* a_typeEntrySignal2;
		Gtk::Entry* a_sourceEntrySignal1;
		Gtk::Entry* a_sourceEntrySignal2;
		Gtk::Entry* a_dateEntrySignal1;
		Gtk::Entry* a_dateEntrySignal2;
		Glib::RefPtr<Gtk::TextBuffer> a_tbufferSignal1;
		Glib::RefPtr<Gtk::TextBuffer> a_tbufferSignal2;

		Gtk::CheckButton checkb_singleSignal ;

		vector<Gtk::Label*> a_labels;
		vector<Gtk::Widget*> a_entries;

		/*** Signals ***/
		string a_signal1ID;
		string a_signal2ID;
		std::map<string, string> a_signal1;
		std::map<string, string> a_signal2;
		int a_signalsCount;

		/*** Data ***/
		list<Property> a_properties;
		std::map<string, list<string> > a_choiceLists;
		std::map<string, string> a_values;
		std::map<string, std::map<string, string> > a_signals;

		/*** References ***/
		Gtk::Window* a_parent;
		DataModel& a_model;
		VersionList a_versionList;
		Parameters& a_params;
		Languages* a_languages;

		/*** Status ***/
		bool a_editable ;
		bool a_displayAnnotationTime;
		bool need_reload ;
		bool need_save ;

		void on_response(int p_id) ;
		void loadCustomProperties(const string& path);
		string getDate(FieldEntry* entry);
		void displayDate(const string& s, FieldEntry* entry);
		Gtk::HBox* showFileProperties();
		void showConventions(Gtk::HBox* hbox);
		void updateGUI();
		void onMore();
		void onAddSignal();
		void onRemoveSignal();
		Gtk::HBox* drawSignalPage();
		Gtk::VBox* drawSignalProperties(int p_signal);
		Gtk::VBox* drawChannelsProperties() ;
		void updateVersions();
		void onSingleSignalChanged() ;

		/*** Geometry interface ***/
		virtual void getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel) ;
		virtual Glib::ustring getWindowTagType()  ;
};

}

#endif // __HAVE_DIALOGFILEPROPERTIES__
