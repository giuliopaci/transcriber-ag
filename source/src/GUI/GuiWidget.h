/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

/**
 * @defgroup 	GUI  TranscriberAG GUI
 */

#include <iostream>
#include <sstream>
#include <string>
#include <gtkmm.h>
#include <gtkmm/menutoolbutton.h>
#include <gtk/gtk.h>

#include "Explorer_tree.h"
#include "NoteBook_mod.h"
#include "Explorer_menu.h"
#include "SpeakerDico_dialog.h"
#include "DictionaryManager.h"
//#include "ConnectionDialog.h"
#include "UserDialog.h"
#include "TreeManager.h"
//#include "FtpData.h"
#include "Configuration.h"
#include "Clipboard.h"
#include "PreferencesDialog.h"

#include "Tools/TAGCommandLine.h"

#include "Common/icons/IcoPackToggleButton.h"
#include "Common/icons/IcoPackButton.h"
#include "Common/icons/IcoPackImage.h"
#include "Common/icons/IcoPackMenuToolButton.h"
#include "Common/icons/IcoPackImage.h"

#include "Common/widgets/GeoWindow.h"
#include "Common/widgets/Settings.h"

#include "Common/Explorer_filter.h"
#include "Common/Parameters.h"
#include "Common/Languages.h"

#include "text/SearchReplaceDialog.h"
#include "text/MiniSearch.h"
#include "text/SearchManager.h"
#include "tags/TSearchDialog.h"
#include "tags/TSearchManager.h"

#include "AnnotationEditor/AnnotationEditor.h"

class InputLanguage;

namespace tag {
/**
 * @class GuiWidget
 * @ingroup		GUI
 * Main GUI class that will be composed by different GUI elements such as
 * file explorer, notebook, toolbar, etc...
 */
class GuiWidget : public Gtk::Window, public GeoWindow
{
	public:
		/**
		 * Instantiates the general GUI component.
		 * @param parameters		Configuration parameters
		 * @param commandline		Command line object
		 * (will be used only if commandFilePath is set)
		 */
		GuiWidget(Parameters* parameters, TAGCommandLine* commandline) ;

		/**
		 * Destructor
		 */
		virtual ~GuiWidget() ;

		/*** Geometry interface ***/
		virtual void saveGeoAndHide() ;
		virtual int loadGeoAndDisplay(bool rundlg=false) ;

	private:

		/**** WIDGET ARCHITECTURE ****/

		//> General Vbox
		Gtk::VBox box_gen ;

			//>> Menu Bar & Toolbar
			Explorer_menu menu ;

			/* Generic toolbox */
			Gtk::Tooltips toolB_tooltip ;
			Gtk::ToolButton toolB_speakerDicoGlobal ;
				IcoPackImage icom_speakerdicoGlobal ;
			Gtk::ToolButton toolB_speakerDicoLocal ;
				IcoPackImage icom_speakerdicoLocal ;
			Gtk::ToggleToolButton toolB_tree ;
				IcoPackImage icom_tree ;
			Gtk::ToolButton toolB_about ;
			IcoPackImage icom_about ;

			/* Special annotation toolbox */
			Gtk::HBox tool_box;
				Gtk::Alignment align;
					Gtk::HBox language_box ;
						IcoPackImage language_image ;
						Gtk::ComboBoxText combo_language;
				Gtk::Frame tool_frame ;
					Gtk::HBox tool_box_2 ;
						IcoPackButton icop_synchro_signal_w_text ;
						IcoPackButton icop_synchro_text_w_signal ;
						IcoPackMenuToolButton button_menu_tagDisplay ;
						IcoPackMenuToolButton button_menu_highlight ;
						IcoPackMenuToolButton button_menu_display ;
							Gtk::RadioMenuItem* display_radioItem_2 ;
							Gtk::RadioMenuItem* display_radioItem_1 ;
							Gtk::RadioMenuItem* display_radioItem_0 ;
						IcoPackButton icop_filemode ;
						IcoPackToggleButton icop_clipboard ;
					Gtk::HSeparator sep_menu_tool ;
					Gtk::VSeparator sep_button1 ;
					Gtk::VSeparator sep_button2 ;

			//>> General Paned
			Gtk::HPaned paned ;

			//>>> one side, file tree and a filter menu
			Gtk::VBox left_box ;
				Gtk::ScrolledWindow scrollW ;
					Gtk::EventBox treeEventBox ;
						Gtk::VBox treeBox ;
							Gtk::Expander system_expander ;
								Gtk::HBox system_hbox ;
									Gtk::Label system_blank ;
									Gtk::VBox system_box ;
									Gtk::EventBox systemEventBox ;
							Gtk::HSeparator sep1 ;
							Gtk::HSeparator sep2 ;
							Gtk::Expander shortcut_expander ;
								Gtk::HBox shortcut_hbox ;
									Gtk::Label shortcut_blank ;
									Gtk::VBox shortcut_box ;
									IcoPackButton shortcut_button ;
								Gtk::Alignment shortcut_button_align ;
								Gtk::HBox shortcut_button_box ;
								Gtk::Label shortcut_button_label ;
								Gtk::Label shortcut_button_BLANK ;

							Gtk::HSeparator sep ;

							Gtk::Alignment filter_combo_align ;
									Gtk::HBox hbox_combo ;
										Gtk::Label label_combo ;

					//>>> Right Side panel, a VBOX
					Gtk::VBox right_box ;
						NoteBook_mod note ;
						//MiniSearch miniSearch ;

			//>> Status bar
			Gtk::HSeparator sep_tree_status ;
			Gtk::Statusbar status ;

		TreeManager* treeManager ;
		bool lock_tree_button ;

		/* Command line argument */
		Glib::ustring command_path ;
		float command_offset ;

		/* Search modules */
		TSearchManager* TsearchManager ;
		SearchManager* searchManager ;
		Clipboard* clipboard ;

		/* Static objects */
		Explorer_filter* filter ;

		/* Speaker */
		DictionaryManager* dicoManager ;

		/* Configuration */
		Configuration* config ;
		Parameters* parameters ;
		TAGCommandLine* commandLine ;

		/* Preference panel session information */
		int preferencesPanelOption ;

		/* User identication dialog */
		UserDialog* user_dialog ;

		/* Languages use */
		bool combo_language_lock ;
		Languages* languages  ;

		bool InitialisedQuit ;

		//** WIDGETS SETTING **//
		void add_default_tree() ;
		void set_menu() ;
		void setMenuUI(Glib::ustring tool_ui_global, Glib::ustring tool_ui_file) ;
		Glib::ustring setExternalToolMenu(bool global) ;
		void connect_tree(Explorer_tree* tree) ;
		void set_filter(Explorer_filter* filter) ;
		void set_frame() ;
		void set_paned() ;
		void set_noteBook() ;
		void set_scroll_window() ;
		void set_popup_menu(Explorer_popup* popup, Explorer_tree* tree) ;
		void set_treatment_monitor() ;
		void set_expander() ;
		void set_clipboard() ;
		void onStatusBar(std::string msg) ;

		//** STARTTING **//
		void configure() ;
		bool postDisplayProcessAfterIdle() ;
		void beforeDisplayConfiguration(bool isIdentified) ;
		void open_command_file(Glib::ustring path, bool threadProtection=false) ;
		void load_openedFiles(Glib::ustring path) ;
		void load_recentFiles(Glib::ustring path) ;
		void initialise() ;
		bool DTD_configuration() ;
		void identifier() ;

		sigc::signal<void> m_signalStarted ;
		sigc::signal<void>& signalStarted() { return m_signalStarted; }

		sigc::signal<void,bool> m_signalIdentified ;
		sigc::signal<void,bool>& signalIdentified() { return m_signalIdentified; }

		sigc::signal<void> m_signalLoaded ;
		sigc::signal<void>& signalLoaded() { return m_signalLoaded; }


		//** GUI OPTIONS **//
		void set_tool_buttons() ;
		void actualize_synchronisation(Glib::ustring swt, Glib::ustring tws) ;
		void change_synchro(Glib::ustring mode) ;
		void on_change_type_annotation(AnnotationEditor* edit) ;
		void change_display() ;
		void change_highlight() ;
		void change_filemode() ;
		void change_tagdisplay() ;
		void actualize_filemode(Glib::ustring mode) ;
		void actualize_tagDisplay(int tagDisplayMode) ;
		void actualize_synchronization_callback(string annot_mode, string value) ;
		void actualize_highlight(int light) ;
		void actualize_highlight_options(bool isStereo, int current_mode) ;
		void actualize_display(int display) ;
		void actualize_display_options(bool isStereo, int mode) ;
		void actualize_tagDisplay_options(int tagDisplayMode) ;
		int map_highlight_value(Glib::ustring value) ;
		bool highlight_mode( GdkEventButton* event, int mode) ;
		bool display_mode( GdkEventButton* event, int mode) ;
		bool tagDisplay_mode( GdkEventButton* event, int mode) ;

		//** SEARCH **//
		void change_search_mode(int mode) ;
		void set_research_in_file() ;
		void search_exit(int rep) ;

		//** NOTEBOOK **//
		void on_notebook_action(Glib::ustring mode) ;
		bool onNoteSignalDragDropReceived(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time) ;
		void onEditorSignalDragDropReceived(const Glib::RefPtr<Gdk::DragContext>& context, AGEditor* editor, const string& speakerId) ;
		void dragDropOpenFileAction() ;
		void dragDropSpeakerAction(AGEditor* editor, const string& speakerId) ;

		void saveAndclose(bool still_opened_tab) ;
		bool prepare_editor(Glib::ustring file, std::vector<Glib::ustring> audios, bool saveState, AnnotationEditor* tmp);
		void prepare_editor_afterIdle(Glib::ustring file, std::vector<Glib::ustring> audios, bool saveState) ;
		void first_last_tab(Glib::ustring direction, Gtk::Widget* widget);

		//switch
		void on_switch_note_page(GtkNotebookPage* page, guint page_num) ;
		void on_switch_note_page_modified(guint page_num) ;
		void actualizeEditorOptions(int new_page_num) ;

		void on_notebook_largeMode(bool mode_large) ;
		void notebook_toggle_large_mode() ;
		void on_notebook_reloadPage(int indice, Gtk::Widget* widget) ;

		//** CLIPBOARD **//
		Gtk::Button* button_clipboard ;
		Gtk::Image image_clipboard ;
		void show_clipboard() ;
		void on_close_clipboard(int response) ;
		void clipboard_action(Glib::ustring action) ;
		void shortcut_show_clipboard() ;

		//** SPEAKER DICO **//
		void on_editSpeakerReceived(Glib::ustring id, bool modal) ;
		void onRaiseCurrentDictionary() ;

		//** TREE EXPLORER **/
		bool visible_tree ;
		void show_hide_explorer(bool visible) ;
		void show_hide_explorer_by(Glib::ustring) ;
		//if defining set at true, force menu to appear
		void on_create_shortcut(Glib::ustring path, Glib::ustring display, bool defining) ;
		void on_shortcut_change_target(Explorer_tree* tree) ;
		void on_delete_shortcut(Explorer_tree* tree) ;
		void on_define_shortcut(Glib::ustring path) ;
		void load_shortcuts(Glib::ustring path) ;
		void on_signalNewTreeFile(Glib::ustring path) ;
		void pack_tree(Explorer_tree* tree, Gtk::VBox* box) ;
		/*popup*/
		void on_popup_menu(Glib::ustring s, Explorer_tree* tree) ;
		void on_action_menu_activated(Glib::ustring action);
		/*filter*/
		void on_filter_combo_changed() ;
		bool treeModelFilter_callback(const Gtk::TreeIter iter) ;
		bool on_tree_press_event(GdkEventButton *event, Explorer_tree* tree) ;

		//** OTHER WINDOWS **//
		void show_search_panel(Glib::ustring mode) ;
		void show_file_properties() ;
		void about() ;
		void show_preferences_panel() ;
		void onPreferencesPanelHide(Gtk::Window* window) ;
		void onPreferencesReloaded(std::map<int, Glib::ustring> dynamic_values, int static_values) ;
		void show_hide_toolbar(Glib::ustring value = "toggle") ;
		void show_hide_statusbar(Glib::ustring value = "toggle") ;
		void show_filePropertyDialog(Gtk::TreeIter, Explorer_tree* tree) ;
		void show_documentation() ;
		void change_toolbar_style(Gtk::Toolbar* ptool, Glib::ustring mode) ;

		//** LANGUAGE **/
		sigc::connection connection_input_language ;
		sigc::connection connection_combo_language ;
		void set_combo_language();
		void onChangeLanguageByCombo();
		void onLanguageChanged(InputLanguage *il, Gtk::Widget* widget);
		void show_hide_language() ;

		//** INTERNAL **//
		bool on_delete_event(GdkEventAny* event) ;
		void quit(bool accepted) ;
		void can_quit() ;
		bool on_key_press_event(GdkEventKey* event) ;
		bool keyPressEvent4Tree(const string& action) ;
		void on_map() ;
		void open() ;
		void add_widget() ;

		/**
		 * Open a given file
		 * @param mode
		 * 		"openall" 			open all file types
		 * 		"newannot"			new single audio transcription file
		 * 		"newmultiannot"		new multi audio transcription file
		 */
		void open_file(Glib::ustring path, Glib::ustring mode, bool threadProtection=false) ;
		void open_action(Glib::ustring path, Glib::ustring name, bool threadProtection=false) ;

		/**
		 * Enables the user to choose whether all files should be reloaded/saved or not
		 * @param mode	 	1: need to save\n
		 * 					2: need to reload\n
		 * 					3: need both
		 * @return			True if user has accepted, False otherwise
		 */
		bool totalFileSavingOrReloading(int mode) ;

		/**
		 * Enables the user to choose whether a file should be reloaded/saved or not
		 * @param path		File path
		 * @param mode	 	1: need to save\n
		 * 					2: need to reload\n
		 * 					3: need both
		 * @return			True if user has accepted, False otherwise
		 */
		bool prepareFileSavingOrReloading(Glib::ustring path, int mode) ;

		/**
		 * Close and reopen the given file
		 * @param path		File path
		 * @param mode	 	1: need to save\n
		 * 					2: need to reload\n
		 * 					3: need both
		 * @return			True if ok, False if error
		 */
		bool saveOrReloadFile(Glib::ustring path, int mode) ;

		/**
		 * Create a new transcription file for audio files associated
		 * @param audio_paths
		 * 		paths of audio files to be transcipted
		 */
		void create_new_transcription(const std::vector<Glib::ustring>& audio_paths) ;

		/**
		 * Display dialog for choosing audio file(s) and create a correspondant transcription file
		 */
		void create_new_transcription_by_dialog(std::vector<Glib::ustring>& audio_paths) ;

		void add_action_groups(Gtk::Widget* a, bool actualize_ui, bool secure_mode) ;
		void remove_action_groups(Gtk::Widget* w, bool secure_mode) ;
		void actualize_recent_file(Glib::ustring path) ;
		void on_reset_editor_aGroups(Glib::ustring type, Glib::RefPtr<Gtk::ActionGroup> gr, Gtk::Widget* widget) ;
		bool on_focus_out_event(GdkEventFocus* event) ;
		bool on_focus_in_event(GdkEventFocus* event) ;

		void test_afterAction(int id, SpeakerDico_dialog* d ) ;

		#ifdef __APPLE__
		bool fontInitialized;
		#endif

		/*** Geometry interface ***/
		virtual void getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel) ;
		virtual void getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual Glib::ustring getWindowTagType()  ;

};
}//namespace tag
