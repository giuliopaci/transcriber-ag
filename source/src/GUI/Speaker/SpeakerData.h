/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#ifndef SPEAKERDATA_H_
#define SPEAKERDATA_H_

/* $Id */

/** @file */

#include "ListView_mod.h"
#include "ListModelColumns_language.h"
#include "Common/Languages.h"
#include "Common/icons/IcoPackButton.h"

#include <gtkmm.h>

namespace tag {
/**
 * @class 		SpeakerData
 * @ingroup		GUI
 *
 * Derived frame representing a speaker.
 * Displays its names, gender, spoken languages, ...
 *
 * Used in the SpeakerDico_dialog
 */
class SpeakerData : public Gtk::Frame
{
	public:
		/**
		 * @class 		ComboModelColumn
		 * @ingroup		GUI
		 *
		 * Small class implementing the model of the ComboBoxText pack in the
		 * speaker language list.
		 */
		class ComboModelColumn : public Gtk::TreeModel::ColumnRecord
		{
			public:
				/**
				 * Constructor
				 * @return
				 */
				ComboModelColumn() : Gtk::TreeModel::ColumnRecord() {
					add(m_value) ;
				}
				virtual ~ComboModelColumn(){};

				Gtk::TreeModelColumn<Glib::ustring> m_value; /**< text value */

				/**
				 * Static wrapper for filling a row using the current model column
				 * @param row				Pointer on the row we want to fill
				 * @param val				Value to add in combo
				 */
				static void fill_row(Gtk::TreeModel::Row* row, Glib::ustring val){
					ComboModelColumn m ;
					(*row)[m.m_value] = val ;
				}
		};

		/**
		 * Constructor
		 * @param parent		Parent window
		 * @param s				Speaker whose information are displayed
		 * @param lang			Languages object
		 * @param new_speaker	True if creating a new speaker, False otherwise
		 * @param edition		True if edition is allowed, False otherwise
		 * @param global		True if displaying in the global speaker dictionary, False otherwise
		 */
		SpeakerData(Gtk::Window* parent, Speaker* s, Languages* lang, bool new_speaker, bool edition, bool global);
		virtual ~SpeakerData();

		/**
		 * Change speaker
		 * @param speaker	New Speaker whose information are to be displayed
		 */
		void changeSpeaker(Speaker* speaker) ;

		/**
		 * Set window parent Pointer
		 * @param w		Window parent address
		 */
		void set_window_parent(Gtk::Window* w) {parent = w; }

		/**
		 * Switch the frame from classic mode to adding mode, or inverse
		 * @param value		True for adding mode, False for classic mode (display and update)
		 */
		void switch_to_add(bool value=true) ;

		/**
		 * Return wether the frame is displayed in adding mode
		 * @return		True if adding mode is on.
		 */
		bool is_adding_mode() { return adding_mode ;}

		/**
		 * Save speaker information
		 * @return 		True if data were saved successfully
		 * @note 		The information are saved in memory only, the file modification
		 * 				will be done at save file (see AnnotationEditor class & DataModel class)
		 */
		bool save() ;

		/**
		 * Modified the status of the frame : updated or not
		 * @param value		If true the frame will be marked as updated, otherwise will be marked as up to date.
		 */
		void set_has_changed(bool value) { hasChanged=value; if (!value) button_validate.set_sensitive(false);}

		/**
		 * Accessor to the modified status
		 * @return		True if some modification have been proceeded, false otherwise
		 */
		bool get_hasChanged() {return hasChanged ;}

		/**
		 * Accessor to the current speaker id
		 * @return		The DataModel speaker id currently displayed in the frame
		 */
		Glib::ustring get_speaker() ;

		/**
		 * Wrapper for the focus method
		 */
		void my_grab_focus() ;

		/**
		 * Signal emitted when speaker data have been updated\n
		 * @return		The corresponding signal
		 */
		sigc::signal<void>& signalSpeakerUpdated() { return m_signalSpeakerUpdated; }

		/**
		 * Signal emitted when a speaker has been created\n
		 * @return		The corresponding signal
		 */
		sigc::signal<void>& signalSpeakerAdded() {return m_signalSpeakerAdded ; }

		/**
		 * Signal emitted when a modification has been done
		 * @return		The corresponding signal
		 */
		sigc::signal<void>& signalModified() {return m_signalModified ; }

		/**
		 * Signal emitted when a drop event has been received
		 * @return		The corresponding signal
		 */
		sigc::signal<void>& signalOnDrop() {return m_signalOnDrop ; }

	private:
		//>widgets
			bool lock_signals ;

			Gtk::HBox general_hbox ;
			Gtk::Label blank_left ;
			Gtk::Label blank_right ;
			Gtk::Label blank ;

			Gtk::Label label_id ;
			Gtk::Entry entry_id ;

			Gtk::Label label_firstName ;
			Gtk::Entry entry_firstName ;
			Gtk::Alignment align_firstName ;
			Gtk::Alignment align_lab_firstName ;

			Gtk::Label label_lastName ;
			Gtk::Entry entry_lastName ;
			Gtk::Alignment align_lastName ;
			Gtk::Alignment align_lab_lastName ;

			Gtk::Label label_gender ;
			Gtk::ComboBoxEntryText combo_gender ;
			Gtk::Alignment align_gender ;
			Gtk::Alignment align_lab_gender ;

			//Gtk::ScrolledWindow scrollW ;
			Gtk::Alignment language_button_align ;
			ListView_mod language_list ;
			Gtk::VBox language_box ;
			Gtk::HBox language_button_box ;
			Gtk::Label label_language ;
			Gtk::Alignment align_language ;
			Gtk::Alignment align_lab_language ;
			IcoPackButton language_button_add ;
			IcoPackButton language_button_remove ;

			Gtk::Label label_description ;
			Gtk::Entry entry_description ;
			Gtk::Alignment align_description ;
			Gtk::Alignment align_lab_description ;

			Gtk::ToggleButton button_edit ;
			Gtk::Button button_validate ;
			Gtk::Button button_cancel ;

			Gtk::VBox vbox ;
			Gtk::HBox hbox_button ;
			Gtk::Alignment align ;
			Gtk::Alignment align_general ;
			Gtk::HSeparator sep ;

			Gtk::Table* table ;
			Gtk::Window* parent ;

			/******************************** List View for language **/
			//> list model
			Glib::RefPtr<Gtk::ListStore> refModel ;
			//active iterator on tree
			Gtk::TreeIter* active_language ;
			//> columns model
			ListModelColumns_language columns ;
			ComboModelColumn c_columns ;

			Glib::RefPtr<Gtk::ListStore> modelComboLanguage ;
			Glib::RefPtr<Gtk::ListStore> modelComboDialect ;
			Glib::RefPtr<Gtk::ListStore> modelComboAccent ;

			void list_editable(bool value) ;
			void prepare_combo() ;
			void fill_dialect_accent(const Glib::ustring& lang, bool first_time=false);
			std::vector<Speaker::Language> languages_tmp ;
			std::vector<Gtk::TreeIter> new_languages ;
			/******************************** List View for language **/

			bool from_global_dico ;
			bool adding_mode ;
			bool hasChanged ;
			bool is_editable ;

			Speaker* speaker ;
			Languages* languages ;

			void set_labels() ;
			void actualise_datas(bool is_empty) ;
			//enable or not edition of data
			void set_editable(bool editable) ;
			void set_combo() ;

			void edition_mode(bool edition) ;
			int get_updated_datas() ;

			void on_validate() ;
			void on_cancel() ;

			/** used to tell the dictionary dialog that a specified action has been initiated **/
			sigc::signal<void> m_signalSpeakerUpdated ;
			sigc::signal<void> m_signalSpeakerAdded ;
			/** used to tell the dictionary dialog that the state has changed **/
			sigc::signal<void> m_signalModified ;
			/** used when a drag drop is received**/
			sigc::signal<void> m_signalOnDrop ;

			void prepare_list() ;
			void on_list_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) ;
			void on_languageList_selection(std::vector<Gtk::TreeIter> _paths) ;
			void actualise_language_combos(const Gtk::TreePath& path, const Gtk::TreeIter& iter) ;
			void fill_model(std::vector<Speaker::Language> languages) ;
			void addRemoveLanguage(int mode) ;
			void modifyLanguage(const Gtk::TreeModel::Path&, const Gtk::TreeModel::iterator&) ;

			void on_combo_edited(const Glib::ustring& s1, const Glib::ustring& s2, Glib::ustring combo) ;
			bool exists_language(std::vector<Speaker::Language> languages, Glib::ustring code, Glib::ustring dialect, Gtk::TreeIter iter);
			void on_toggle_edited(const Glib::ustring& s1, Glib::ustring toggle) ;
			int get_language_from_iter(Gtk::TreeIter iter);
			void print_languages(std::vector<Speaker::Language> languages) ;
			void addRemove_language(Glib::ustring mode) ;
			bool check_new_languages(int& indice) ;

			void on_entries_changed() ;

			void modified(bool mod) ;

			virtual bool on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time) ;
			int check_locutor_name(Glib::ustring name) ;
};

} //namspace

#endif /*SPEAKERDATA_H_*/
