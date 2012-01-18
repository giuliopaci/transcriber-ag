/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef SPEAKERDICO_DIALOG_H_
#define SPEAKERDICO_DIALOG_H_

#include <gtkmm.h>
#include "DataModel/speakers/SpeakerDictionary.h"
#include "ListModel_Columns.h"
#include "SpeakerData.h"
#include "ListView_mod.h"
#include "Common/Parameters.h"
#include "Common/icons/IcoPackButton.h"
#include "Common/icons/IcoPackImage.h"
#include "Common/icons/IcoPackToggleButton.h"
#include "Common/widgets/GeoWindow.h"
#include "Editors/AnnotationEditor/AnnotationEditor.h"

namespace tag {
/**
 * @class 		SpeakerDico_dialog
 * @ingroup		GUI
 *
 * Dialog displaying the speaker dictionary, for both local and global mode
 * Enable display, edition and creation.
 *
 * @note	The global speaker dictionary is loaded from a file, modification are
 * 			saved immediately. The local dictionary is loaded from the file opened
 * 			in the AnnotationEditor, it is saved when the file is saved.
 */
class SpeakerDico_dialog : public Gtk::Window, public GeoWindow
{
	public:

	  /**
	   * Create a speaker dictionary.
	   * @param edit	 Allow dictionary edition
	   * @param modal	 Allow modal modal mode
	   * @param win	 	 Pointer on parent window
	   * @note 			 The dictionary will be fully created whith SpeakerDico_dialog::open_dictionary method
	   */
		SpeakerDico_dialog(bool edit, bool modal, Gtk::Window* win);
		virtual ~SpeakerDico_dialog();

	  /**
	   * Open the global speaker dictionary (loaded from an xml specific file).
	   * @param url 		Speaker dictionary url
	   */
		void open_dictionary(Glib::ustring url) ;

	  /**
	   * Open a local speaker dictionary (from an AnnotationEditor)
	   * @param editor: AnnotationEditor
	   */
		void open_dictionary(AnnotationEditor* editor) ;

		/**
		 * Accessor to the ListView_mod used for displaying speaker list
		 * @return	The ListView_mod used for the speaker list
		 */
		ListView_mod* get_view() {return &l_listView ;}

		/**
		 * Accessor to the TreeModelSort used as model for the speaker list
		 * @return	The TreeModelSort used for the speaker list
		 */
		Glib::RefPtr<Gtk::TreeModelSort> get_refSortedModel() { return refSortedModel; }

		/**
		 * Get the current selected Speaker object
		 * @return	Pointer on the current Speaker object
		 */
		Speaker* get_current_speaker() { return current_speaker ;}

		/**
		 * Set the current selected Speaker object
		 * @param s		Pointer on the current Speaker object
		 */
		void set_current_speaker(Speaker* s) { current_speaker =  s ; }

		/**
		 * Accessor to the SpeakerDictionary used for getting all speaker data
		 * @return		Pointer on the SpeakerDictionary used
		 */
		SpeakerDictionary* get_dictionary() { return s_dico ;}

		/**
		 *	Add a new speaker in the speaker list and in the SpeakerDictionary model
		 * @param change_id			If true then a brand new speaker will be created (new Id got from the model).
		 * 							Otherwise, the current speaker previously modified or created by SpeakerData will
		 * 							be used\n.
		 * 							Has to be true when the adding isn't proceeded from SpeakerData but from external
		 * 							component such as DictionaryManager.
		 * @param global_src_id		Specify an id that will be kept in the "scope" speaker field.\n
		 * 							Useful for conserving source id, otherwise empty value by default.
		 * @see						get_current_speaker() for adding from SpeakerData
		 */
		void addSpeaker(bool change_id, Glib::ustring global_src_id="") ;

		/**
		 * Get the name of the file from which the speaker dictionary has been loaded from
		 * @return		File name of global speaker dictionary file if the speaker dictionary has been
		 * 				loaded in global mode, or the name of transcription if the speaker dictionary has been
		 * 				loaded in local mode.
		 * @see 		is_global()
		 */
		Glib::ustring get_fileName() {return current_file ;}

		/**
		 * Get the editability status of the speaker dictionary
		 * @return		Editability of the dictionary
		 */
		bool is_editable() {return edition_enabled ;}

		/**
		 * Indicated the type of speaker dictionary
		 * @return		True for global dictionary (loaded from a specific xml file),
		 * 				False for a local dictionary (loaded from a TAG transcription file).
		 */
		bool is_global() {return global ;}

		/**
		 * Get the Identifier of the current selected speaker
		 * @return		Id (model id) of the selected speaker
		 * @see			get_current_speaker()
		 */
		Glib::ustring get_active_id() {return active_id ;}

		/**
		 * Get the modified status of the dictionary
		 * @return		True if modifications (not saved) have been done, False otherwise
		 */
		bool is_updated() { return updated; }

		/**
		 * Replace the selected speaker by the speaker corresponding to the given id
		 * @param id				Id (model id) of the new speaker
		 * @param global_src_id		Specify an id that will be kept in the "scope" speaker field.\n
		 *							Useful for conserving source id, otherwise empty value by default.
		 * @return					1 for success, negative integer for failure
		 * @see						get_current_speaker() and set_current_speaker() for selected speaker manipulation
		 */
		int replaceSpeaker(Glib::ustring id, Glib::ustring global_src_id) ;

		/**
		 * Activate the row of the speaker that corresponds to the given id
		 * @param id	Id (model id) of the speaker to be selected
		 */
		void set_cursor_to_speaker(Glib::ustring id) ;

		/**
		 * 	Save the modifications done on dictionary. (update GUI too)
		 */
		void save() { genApply->clicked() ; }

		/**
		 * Close the speaker dictionary
		 * @param keep_speaker_selection	In most case has to be set to false, except in light_mode for which
		 * 									we need to have a valid selected speaker Pointer even after closing dictionary
		 * @return							True if closing has been done, False if error occurred of if user canceled the operation.
		 * @note							Display a confirmation dialog when the dictionary had been updated but not saved yet.
		 */
		bool close_dialog(bool keep_speaker_selection=false) ;

		/**
		 * Activate the speaker dictionary light mode.
		 * The light mode only provide a speaker consultation and selection list.
		 * Once the dialog closed, the selected speaker can be accessed with the
		 * SpeakerDico_dialog::get_current_speaker() method
		 * @note	  	Differences with classic mode :\n
		 *  				- no buttons add /remove\n
		 *  				- no details\n
		 *  				- signal emitted when double click on speaker\n
		 * @attention  	When using the light mode, the SpeakerDico_dialog::close_dialog() must be used
		 * 				with argument <em>keep_speaker_selection</em> set to TRUE.
		 */
		void set_light_mode() ;

		/**
		 * Enables raise option : if the dialog is the global one, a button will
		 * enable to open the local one
		 * @param value		True or False
		 */
		void allowRaiseOption(bool value) ;

		/**
		 * Set the drag and drop target list
		 * @param targetList	Drag and drop target list
		 */
		void setDragAndDropTarget(std::vector<Gtk::TargetEntry> targetList) ;

		/**
		 *  Signal sent when a speaker in selected in dialog
		 *  ustring: Id speaker
		 */
		sigc::signal<void,Glib::ustring>& signalSelectedSpeaker() { return m_signalSelectedSpeaker ; }

		/**
		 *  Signal sent when closing the dialog
		 */
		sigc::signal<void>& signalClose() { return m_signalClose ; }

		/**
		 *  Signal sent when a speaker has been dropped on data in order to replace
		 */
		sigc::signal<void>& signalDropOnData() { return m_signalDropOnData ; }

		/**
		 *  Signal sent when the user wants to raise other dictionary
		 *  <b>ustring parameter:</b> "local" for raising current file dictionary, "global" for global dictionary
		 */
		sigc::signal<void,Glib::ustring>& signalRaiseDictionary() { return m_signalRaiseDictionary ; }

		/*** Geometry interface ***/
		virtual void saveGeoAndHide() ;
		virtual int loadGeoAndDisplay(bool rundlg=false) ;

	private:
		/*
		 *  Define the mode for only consultation list
		 *  Differences with read-only mode :
		 *  	- no buttons add /remove
		 *  	- no details
		 *  	- signal emitted when double click on speaker
		 */
		bool m_light_mode ;

		/*
		 *  Disable the actualization of speaker data panel
		 */
		bool lock_on_selection ;
		/*
		 *  Disable the possibility of displaying speaker data panel
		 */
		bool lock_show_details ;
		int details_size ;

		/* stock data drop signal connection to disconnect */
		sigc::connection drop_connection ;

		/* list of targetEntry to define drag and source identity */
		std::vector<Gtk::TargetEntry> target_list ;

		AnnotationEditor* editor ;
		Languages* languages ;

		//> enable options
		bool edition_enabled ;
		bool import_enabled ;
		bool global, fromUrl ;
		Glib::ustring global_dico_path ;
		Glib::ustring current_file ;
		bool data_initialised ;
		bool updated ;

		//> widgets
		//> BOX DIALOG
		Gtk::VBox* genVBox ;
			Gtk::HPaned paned ;
				//> first side
				Gtk::VBox l_vbox ;
					Gtk::HBox l_hbox_top ;
						IcoPackButton raise_button ;
							Gtk::HBox title_hbox ;
								IcoPackImage title_image ;
								Gtk::Label title_label ;
						IcoPackToggleButton details ;
					Gtk::VPaned hpaned ;
						//top part
						Gtk::ScrolledWindow scrollW ;
							ListView_mod l_listView ;
						//bottom part
						Gtk::VBox l_vbox_bottom ;
							Gtk::HBox l_hbox_bottom2 ;
								Gtk::Alignment align_edit ;
									IcoPackButton addSpeakerButton ;
									IcoPackButton removeSpeakerButton ;
			Gtk::HSeparator sep ;
			Gtk::Alignment button_align ;
				Gtk::HBox button_box ;
					Gtk::Button* close ;
					IcoPackButton choose_speaker ;
					Gtk::Button* genApply ;
					Gtk::Button* genCancel ;

		//> list model
		Glib::RefPtr<Gtk::ListStore> refModel ;
		Glib::RefPtr<Gtk::TreeModelSort> refSortedModel ;

		/** list of treeIter for enabling multiselection **/
		std::vector<Gtk::TreeIter> paths;
		/** list of speaker to keep temporary new speaker **/
		std::map<int, Speaker*> new_speakers ;
		/** state of new speakers 0:just created 1:saved */
		std::map<int, int> new_speakers_state ;
		int new_speakers_indice ;

		//> columns model
		ListModel_Columns columns ;

		//current dictionary
		SpeakerDictionary* s_dico ;
		//backup of dictionary
		SpeakerDictionary* s_dico_backUp ;
		//active data panel
		SpeakerData* data ;
		//active id
		Glib::ustring active_id ;
		//active iterator on tree
		Gtk::TreeIter active_iter ;
		Gtk::TreePath active_path ;
		//current speaker
		Speaker* current_speaker ;

		//> internal settings
		int fill_model() ;
		void set_buttons() ;
		void selection_buttons_state(bool activated) ;
		void prepare_list() ;
		void prepareGUI() ;

		/** Check if a speaker with same first name & last name already exists **/
		void check_speaker(Speaker speaker, std::vector<Gtk::TreeIter>* v ) ;

		//> set data panel visibilty
		void on_show_details() ;

		//> business method
		int deleteSpeaker(Glib::ustring id) ;
		int updateSpeaker(Glib::ustring id) ;

		void stock_current_iter_as_activePath(Gtk::TreeIter iter) ;
		void get_current_path_as_activeIter(Gtk::TreePath path) ;
		void prepare_datas_panel(Gtk::TreeIter iter) ;

		void pack_speaker_frame(Gtk::Frame* sd) ;
		Gtk::Frame* get_speaker_frame() ;

		void on_row_activated(const Gtk::TreeModel::Path&, Gtk::TreeViewColumn* c) ;
		bool on_response(GdkEventAny* event) ;
		void on_signal_modified() ;
		void on_data_drop() ;
		void on_select_speaker() ;
		void on_edit(Glib::ustring removing)	;
		void on_import(bool onlyActiveSpeaker) ;
		void on_updateDictionary(bool update, int response_id)	;
		void on_selection(std::vector<Gtk::TreeIter> paths) ;
		int on_list_sort(const Gtk::TreeModel::iterator& it1, const Gtk::TreeModel::iterator& it2) ;
		void on_header_column_clicked(int column_num) ;
		void close_dialog_wrap() ;
		void onRaiseButtonReleased() ;

		void initializeDataPanel(SpeakerData* data) ;

		//> signal to raise a dictionary
		sigc::signal<void,Glib::ustring> m_signalRaiseDictionary ;

		//> signal to chose a speaker
		sigc::signal<void,Glib::ustring> m_signalSelectedSpeaker ;

		//> signal to chose a speaker
		sigc::signal<void> m_signalClose ;

		//> signal to replace a speaker: drop
		sigc::signal<void> m_signalDropOnData ;

		/*** Geometry interface ***/
		virtual void getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel) ;
		virtual void getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual Glib::ustring getWindowTagType()  ;

};

}/* namespace*/

#endif /*SpeakerDico_DIALOG_H_*/
