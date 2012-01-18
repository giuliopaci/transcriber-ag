/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "GuiWidget.h"

#include "Explorer_fileHelper.h"
#include "Explorer_dialog.h"
#include "ShortcutDialog.h"
#include "AboutDialog.h"
#include "PreferencesDialog.h"
#include "FilePropertyDialog.h"
#include "Doc.h"
#include "CreateTranscriptionDialog.h"

#include "Common/VersionInfo.h"
#include "Common/widgets/ToolLauncher.h"
#include "Common/Languages.h"
#include "Common/InputLanguage.h"
#include "Common/FileInfo.h"
#include "Common/Formats.h"
#include "Common/Explorer_utils.h"
#include "Common/util/Utils.h"
#include "Common/util/FileHelper.h"
#include "Common/globdef.h"
#include "Common/icons/Icons.h"
#include "Common/widgets/GtUtil.h"

#include "Editors/AnnotationEditor/dialogs/DialogFileProperties.h"
#include "DataModel/speakers/SpeakerDictionary.h"

#include <algorithm>
#include <glib.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>


#define TAG_TOOLBAR_BUTTON_SIZE		20
#define TAG_TOOLBAR_COMBO_H_SIZE	7
#define TAG_TOOLBAR_COMBO_V_SIZE	0

//using namespace tag ;
namespace tag {

GuiWidget::GuiWidget(Parameters* param, TAGCommandLine* commandline)
{
	// Values initialization
	preferencesPanelOption = 0 ;

	user_dialog = NULL ;
	searchManager = NULL ;
	TsearchManager = NULL ;
	clipboard = NULL ;
	dicoManager = NULL ;
	config = NULL ;

	parameters = param ;
	commandLine = commandline ;

	InitialisedQuit = false ;
	combo_language_lock = false;
	visible_tree = true ;
	lock_tree_button = false ;

	#ifdef __APPLE__
	fontInitialized = false;
	#endif

	//> Set icon
	Glib::RefPtr<Gdk::Screen> screen = get_screen() ;
	set_default_size(screen->get_width(), screen->get_height());
	Icons::set_window_icon(this, ICO_TRANSCRIBER, 17) ;

	//> set title
	set_title(TRANSAG_DISPLAY_NAME);
	set_border_width(2) ;

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CONFIGURE DATAs
	configure() ;

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> SET WIDGETS
	filter = Explorer_filter::getInstance() ;
	treeManager = new TreeManager(this, config) ;
	dicoManager = new DictionaryManager(this, config) ;
	searchManager = new SearchManager(config, this) ;
	TsearchManager = new TSearchManager(config) ;
	clipboard = new Clipboard(config, this) ;

	set_menu() ;
	set_frame() ;
	set_filter(filter);
	set_scroll_window() ;
	set_expander() ;
	add_default_tree() ;
	set_noteBook();
	set_clipboard() ;
	set_research_in_file() ;
	set_combo_language();
	status.push(TRANSAG_VERSION_NO,0);

	//> -- Additional signals
	treeManager->signalOpenFileRequired().connect(sigc::bind<bool>(sigc::mem_fun(this, &GuiWidget::open_file), false)) ;
	dicoManager->signalRaiseCurrentDictionary().connect(sigc::mem_fun(*this, &GuiWidget::onRaiseCurrentDictionary)) ;

	Explorer_utils::print_trace("TranscriberAG --> <*> Graphic setting [OK]", 1) ;

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ADD WIDGET
	show_all_children() ;
	add_widget() ;
	loadPos() ;
	Explorer_utils::print_trace("TranscriberAG --> <*> Graphic displaying [OK]", 1) ;

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ASK FTP CONNECTION
	//signalStarted-->identification-->signalIdentified-->loadDatas-->signalLoaded
	m_signalStarted.connect(sigc::mem_fun(this, &GuiWidget::identifier)) ;
	m_signalIdentified.connect(sigc::mem_fun(this, &GuiWidget::beforeDisplayConfiguration)) ;
}

GuiWidget::~GuiWidget()
{
	Explorer_utils::print_trace("TranscriberAG --> <.> Killing widget...", 1) ;
	//>delete objects
	if (filter)
		filter->kill() ;
	if (clipboard)
		delete(clipboard) ;
	if (dicoManager)
		delete(dicoManager) ;
	if (treeManager)
		delete(treeManager) ;
	if (searchManager)
		delete(searchManager) ;
	if (TsearchManager)
		delete(TsearchManager) ;
	if (config)
		delete(config) ;
	Settings::kill() ;
	Languages::kill() ;
	ToolLauncher::kill() ;
	Formats::kill() ;
	Explorer_utils::print_trace("TranscriberAG --> <.> ... Widget killed", 1) ;
}

void GuiWidget::configure()
{
	//> LOADER
	Explorer_utils::print_trace("TranscriberAG --> <*> Initializing status report", 1) ;

	//> Window geometry
	Settings::configure(parameters) ;

	//> EXTERNAL TOOLS
	Explorer_utils::print_trace("TranscriberAG --> <*> Loading Tool Launcher", 1) ;
	ToolLauncher::configure(parameters, this) ;

	//> GLOBAL GUI CONFIGURATION
	config = new Configuration(parameters) ;

	//> LANGUAGES
	Explorer_utils::print_trace("TranscriberAG --> <*> Loading languages: ", config->get_languages_path(), 1) ;
	Languages::configure(config->get_languages_path(), "Usual") ;

	//> FORMAT
	Explorer_utils::print_trace("TranscriberAG --> <*> Loading format", 1) ;
	Formats::configure(config->get_CONFIG_path()) ;

	//> INPUT LANGUAGE
	Glib::ustring input_language = config->get_InputLanguages_path() ;
	Explorer_utils::print_trace("TranscriberAG --> <*> Loading input languages: ", input_language, 1) ;
	InputLanguageHandler::load_language_map(input_language, config->get_externalIME_mode());
}

//***************************************************************************************
//***************************************************************** WIDGETS SET FUNCTIONS
//***************************************************************************************

void GuiWidget::add_widget()
{
	add(box_gen) ;

	//> add menubar
	Gtk::Widget* pmenu = menu.get_widget_mainMenu("/MenuBar") ;
	if (pmenu)
		box_gen.pack_start(*pmenu, false, false) ;
	else
		TRACE << "TranscriberAG --> (!) Adding menu failed" << std::endl ;

	//> TOOLBAR
	Gtk::Toolbar* ptool = (Gtk::Toolbar*)(menu.get_widget_mainMenu("/Toolbar")) ;
	if (ptool){
		box_gen.pack_start(tool_box, false, false) ;
		tool_box.pack_start(*ptool, true, true) ;
		Glib::ustring display = config->get_TOOLBAR_displayLabel() ;
		change_toolbar_style(ptool, display) ;
		ptool->append(toolB_tree) ;
		ptool->append(toolB_speakerDicoGlobal) ;
	}
	else{
		TRACE << "TranscriberAG --> (!) Adding toolbar failed" << std::endl ;
	}

	//> add Language combo set
	align.set_padding(TAG_TOOLBAR_COMBO_V_SIZE, TAG_TOOLBAR_COMBO_V_SIZE, TAG_TOOLBAR_COMBO_H_SIZE, TAG_TOOLBAR_COMBO_H_SIZE);
	align.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0) ;
	tool_box.pack_start(align, false, false, 0);
		align.add(language_box) ;
			language_box.pack_start(language_image, false, false, 4) ;
			language_box.pack_start(combo_language, false, false) ;

	//> add options button set
	tool_box.pack_start(tool_frame, false, false) ;
		tool_frame.add(tool_box_2) ;

	//> add hilight option
	set_tool_buttons() ;

	//> PANED
	box_gen.pack_start(paned, true, true, 2);
		//>> First side
		paned.pack1(left_box, false, false) ;
			left_box.pack_start(filter_combo_align, false, false) ;
				filter_combo_align.add(hbox_combo) ;
					hbox_combo.pack_start(label_combo, false, false) ;
					hbox_combo.pack_start(*filter->get_combo(), false, false) ;
			left_box.pack_start(treeEventBox, true, true, 2) ;
				treeEventBox.set_name("tree_box") ;
				treeEventBox.add(scrollW) ;
					scrollW.add(treeBox) ;
					scrollW.set_name("tree_box") ;

		//>> Second side
		paned.pack2( right_box, true, true ) ;
		right_box.pack_start( note, true, true ) ;
		right_box.pack_start( *(searchManager->get_mini()), false, false ) ;

	filter_combo_align.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_BOTTOM, 0.0, 0.0) ;

	//> add StatusBar
	box_gen.pack_start(status, false, false) ;
	show_all_children();
	searchManager->get_mini()->hide() ;

	//display
	if (ptool && config->get_GUI_toolbarShow()==0)
		tool_box.hide() ;
	if (config->get_GUI_statusbarShow()==0)
		status.hide() ;
}

void GuiWidget::set_scroll_window()
{
	//> auto-scrolling
	scrollW.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
}

void GuiWidget::set_paned()
{
	paned.set_position(300);
}


void GuiWidget::set_frame()
{
}

void GuiWidget::change_toolbar_style(Gtk::Toolbar* ptool, Glib::ustring mode)
{
	if (!ptool)
		ptool = (Gtk::Toolbar*)(menu.get_widget_mainMenu("/Toolbar")) ;

	if (mode.compare("true")==0)
		ptool->set_toolbar_style(Gtk::TOOLBAR_BOTH) ;
	else if (mode.compare("text")==0)
		ptool->set_toolbar_style(Gtk::TOOLBAR_TEXT) ;
	else
		ptool->set_toolbar_style(Gtk::TOOLBAR_ICONS) ;
}

void GuiWidget::onStatusBar(std::string msg)
{
	if (!msg.empty())
		status.push(msg, 0) ;
	else
		status.push(TRANSAG_VERSION_NO, 0) ;
}

//***************************************************************************************
//***************************************************************** SYSTEM INITIALIZATION
//***************************************************************************************

void GuiWidget::initialise()
{
}

void GuiWidget::identifier()
{
	Glib::ustring id = config->get_USER_acronym() ;
	if (id=="")
	{
		user_dialog = new UserDialog(config, true) ;
        user_dialog->signalIdentified().connect(sigc::mem_fun(*this, &GuiWidget::beforeDisplayConfiguration));
#ifdef __APPLE__
        user_dialog->set_transient_for(*this);
#endif
        user_dialog->show() ;
    }
	else
		m_signalIdentified.emit(true) ;
}

void GuiWidget::beforeDisplayConfiguration(bool isIdentified)
{
	if (user_dialog)
		delete(user_dialog) ;

	if (isIdentified)
	{
		//> open old loaded files only if option is ON and if no path in command line
		Glib::ustring openOption = parameters->getParameterValue("GUI", "data,openLastFiles");

		if ( Glib::file_test(config->get_NOTEBOOK_opened_path(), Glib::FILE_TEST_EXISTS)
					&& openOption=="true"
					&& commandLine->getFilename()=="")
			load_openedFiles(config->get_NOTEBOOK_opened_path()) ;

		//> if path in command line open it
		if (commandLine->getFilename()!="")
		{
			open_command_file(commandLine->getFilename(), true) ;
			commandLine->resetFilename() ;
		}

		//> Load recent files
		if ( Glib::file_test(config->get_MENU_recent_path(), Glib::FILE_TEST_EXISTS) )
			load_recentFiles(config->get_MENU_recent_path());

		//> Load clipboard
		if ( Glib::file_test(config->get_CLIPBOARD_path(), Glib::FILE_TEST_EXISTS) )
			clipboard->load(config->get_CLIPBOARD_path()) ;

		//> load_shortcuts
		if ( Glib::file_test(config->get_SHORTCUTS_path(), Glib::FILE_TEST_EXISTS) )
			load_shortcuts(config->get_SHORTCUTS_path()) ;
	}
	else
	{
		InitialisedQuit = true ;
		quit(true) ;
	}
}

void GuiWidget::load_openedFiles(Glib::ustring path)
{
	std::vector<Glib::ustring> vect ;
	int res = Explorer_utils::read_lines(path, &vect) ;
	if (res==1 && vect.size()!=0)
	{
		std::vector<Glib::ustring>::iterator it = vect.begin() ;
		while ( it!=vect.end() )
		{
			if ( Glib::file_test(*it, Glib::FILE_TEST_EXISTS) && filter->is_import_annotation_file(*it) )
				open_file(*it, "openall") ;
			it++ ;
		}
	}
}

void GuiWidget::load_recentFiles(Glib::ustring path)
{
	std::vector<Glib::ustring> vect ;
	int res = Explorer_utils::read_lines(path, &vect) ;
	if (res==1 && vect.size()!=0)
	{
		std::vector<Glib::ustring>::reverse_iterator it = vect.rbegin() ;
		while ( it!=vect.rend() )
		{
			if ( Glib::file_test(*it, Glib::FILE_TEST_EXISTS) && filter->is_import_annotation_file(*it) )
					actualize_recent_file(*it) ;
			it++ ;
		}
	}
}

void GuiWidget::load_shortcuts(Glib::ustring path)
{
	std::vector<Glib::ustring> vect ;
	int res = Explorer_utils::read_lines(path, &vect) ;
	if (res==1 && vect.size()!=0)
	{
		std::vector<Glib::ustring>::iterator it = vect.begin() ;
		std::vector<Glib::ustring> tmp ;
		while ( it!=vect.end() )
		{
			int res = mini_parser('|', *it, &tmp) ;
			if ( res>0 )
			{
			 	if (Glib::file_test(tmp[1], Glib::FILE_TEST_EXISTS))
					on_create_shortcut(tmp[1], tmp[0], false) ;
			}
			it++ ;
		}
	}
}

void GuiWidget::open_command_file(Glib::ustring path, bool threadProtection)
{
	FileInfo info(path) ;
	Glib::ustring real = info.realpath() ;
	if ( Glib::file_test(real, Glib::FILE_TEST_EXISTS) )
		open_file(real, "openall", true) ;
	else
	{
		gdk_threads_enter() ;
		Explorer_dialog::msg_dialog_error(_("File not found"), this, true) ;
		gdk_threads_leave() ;
	}
}


//***************************************************************************************
//********************************************************************** LANGUAGE METHODS
//***************************************************************************************


void GuiWidget::set_combo_language()
{
	 std::map<std::string, InputLanguage*>::iterator ite;
	 for(ite = InputLanguageHandler::get_first_language_iter();
	 		ite != InputLanguageHandler::get_last_language_iter();
	 		ite++)
	 {
		 if ((*ite).second->isActivated())
			 combo_language.append_text((*ite).second->getLanguageDisplay());
	 }
     combo_language.set_active(0);
	 combo_language.set_sensitive(false);
	 connection_combo_language = combo_language.signal_changed().connect(sigc::mem_fun(*this, &GuiWidget::onChangeLanguageByCombo));
	 combo_language.set_focus_on_click(false) ;

	 language_image.set_image(ICO_COMBOLANGUAGE_ICON, 21) ;
}


void GuiWidget::show_hide_language()
{
	if (align.is_visible())
		align.hide_all() ;
	else
		align.show_all() ;
}

void GuiWidget::onChangeLanguageByCombo()
{
	Glib::ustring name = combo_language.get_active_text() ;
	if (name != "")
	{
		InputLanguage *il = NULL;
		il = InputLanguageHandler::get_input_language_by_name(combo_language.get_active_text());
		AnnotationEditor *editor = (AnnotationEditor*)note.get_active_widget();
		if(editor)
			editor->set_input_language(il);
	}
	else
		TRACE_D << "mmTAG --> (!) language combo null" << std::endl ;
}

 void GuiWidget::onLanguageChanged(InputLanguage *il, Gtk::Widget* widget)
{
	if (il == NULL)
		il = InputLanguageHandler::get_input_language_by_name(DEFAULT_LANGUAGE) ;
	if (il==NULL)
		return ;

	AnnotationEditor* edit = (AnnotationEditor*) widget ;
	Glib::ustring current_name = il->getLanguageDisplay() ;

	//> Actualize combo
	connection_combo_language.disconnect() ;
	combo_language.set_active_text(current_name);
	connection_combo_language =  combo_language.signal_changed().connect(sigc::mem_fun(*this, &GuiWidget::onChangeLanguageByCombo));

	//> Actualize external IME
	if ( edit != NULL )
	{
		if (il && il->getLanguageType().compare(IME_LANGUAGE)==0)
			edit->externalIMEcontrol(true) ;
		else
			edit->externalIMEcontrol(false) ;
	}
}

//***************************************************************************************
//********************************************************************** NOTEBOOK METHODS
//***************************************************************************************


void GuiWidget::set_noteBook()
{
	note.set_configuration(config) ;
	note.set_treeManager(treeManager) ;
	note.set_dicoManager(dicoManager) ;
	note.set_window(this) ;

	//> for first adding switching
	note.signalPersonalSwitch().connect( sigc::mem_fun(*this, &GuiWidget::on_switch_note_page_modified) ) ;
	//> for switch pages action
	note.signal_switch_page().connect( sigc::mem_fun(*this, &GuiWidget::on_switch_note_page) ) ;
	//> for last tab closing or first tab opening
	note.signalFirstLastTab().connect( sigc::mem_fun(*this, &GuiWidget::first_last_tab) ) ;
	//> for drag'n'drop file sexy use in notebook
	note.signal_drag_drop().connect( sigc::mem_fun(*this, &GuiWidget::onNoteSignalDragDropReceived));
	//> for actualize recent file menu
	note.signalAddRecentFile().connect( sigc::mem_fun(*this, &GuiWidget::actualize_recent_file)) ;
	//> for actualize recent file menu
	note.signalNewTreeFile().connect( sigc::mem_fun(*this, &GuiWidget::on_signalNewTreeFile)) ;
	//> for closing
	note.signalClosedLastTab().connect( sigc::mem_fun(*this, &GuiWidget::quit)) ;
	//> for large display
	note.signalHeaderDoubleClick().connect( sigc::mem_fun(*this, &GuiWidget::on_notebook_largeMode)) ;
	//> for reload status
	note.signalReloadPage().connect( sigc::mem_fun(*this, &GuiWidget::on_notebook_reloadPage) ) ;
}


void GuiWidget::on_switch_note_page(GtkNotebookPage* page, guint page_num)
{
	on_switch_note_page_modified(page_num) ;
}

void GuiWidget::on_switch_note_page_modified(guint page_num)
{
	// For the FIRST tab, add action groups is done in GuiWidget callback "first_last_tab"
	// For the LAST tab, remove action groups is done in "on_close_received" of NoteBook_mod

	//> when various files, switch action groups
	if ( (note.get_n_pages() != 0) && note.is_first_tab_passed() )
	{
		//> if page is loading, switching process is postponed
		//and will be launched by the notebook (signal loaded)
		if ( !note.is_loaded_page(page_num) )
			return ;

		actualizeEditorOptions(page_num) ;
		note.setActiveWidget(page_num) ;
	}//end between various page
}

void GuiWidget::actualizeEditorOptions(int new_page_num)
{
	//>> ACTUALIZE OLD ACTIVE PAGE
	int old_page = note.get_old_page() ;
	if (old_page!=-1)
	{
		Widget* old_w = note.get_page_widget(note.get_old_page()) ;
		if (old_w)
		{
			//> tell notebook we're switching from editor
			//menu.set_previous_reset_widget(old_w) ;
			remove_action_groups(old_w, true) ;

			// -- Stopping Old SignalView Playback --
			AnnotationEditor* old_editor	= (AnnotationEditor*)old_w;
			old_editor->setPlay(false, true);
			old_editor->onPageActivated(false) ;
		}
	}

	//>> ACTUALIZE NEW ACTIVE PAGE
	Widget* widget = note.get_page_widget(new_page_num) ;
	if (!widget)
		return ;

	AnnotationEditor* edit = (AnnotationEditor*)widget ;

	note.set_old_page(new_page_num) ;

	// Actualize current action group
	add_action_groups(widget,true, true) ;

	//> get synchronisation values
	if (edit->hasSignalView()) 
	{
		icop_synchro_signal_w_text.set_sensitive(true) ;
		icop_synchro_text_w_signal.set_sensitive(true) ;
		button_menu_highlight.set_sensitive(true) ;
		button_menu_tagDisplay.set_sensitive(true) ;
		Glib::ustring swt = edit->getOption("synchro_signal_to_text") ;
		Glib::ustring tws = edit->getOption("synchro_text_to_signal") ;
		actualize_synchronisation(swt, tws) ;

		//> get hilight value
		int highlight = edit->getHighlightMode() ;
		actualize_highlight(highlight) ;

		//> DIRTY SOLUCE:
		//  because notebook show page again, when several views are created
		//  they are all displayed. Let's force again the annotation display
		//  for only showing the needed one(s).
		bool isStereo = edit->isStereo() ;
		int display_mode = -1 ;
		if (isStereo)
			display_mode = edit->resetDisplayMode() ;
		actualize_display_options(isStereo,display_mode) ;
	}
	else
	{
		actualize_display_options(false,-1) ;
		button_menu_display.set_sensitive(false) ;
		icop_synchro_signal_w_text.set_sensitive(false) ;
		icop_synchro_text_w_signal.set_sensitive(false) ;
		button_menu_highlight.set_sensitive(false) ;
		button_menu_tagDisplay.set_sensitive(false) ;
	}

	//> get filemode value
	Glib::ustring mode = edit->getOption("mode") ;
	actualize_filemode(mode) ;

	//> get hiddenTag value
	int hidden = edit->getTagHiddenMode() ;
	actualize_tagDisplay(hidden) ;

	//> switch research reference
	if ( searchManager->is_active() ) {
		searchManager->switch_file(edit) ;
		searchManager->myShow() ;
	}
	if ( TsearchManager->is_active() ) {
		TsearchManager->switch_file(edit) ;
		TsearchManager->myShow() ;
	}

	//> switch clipboard reference
	if (clipboard->is_active()) {
		clipboard->switch_file(edit, true) ;
		if (icop_clipboard.get_active())
			clipboard->loadGeoAndDisplay() ;
	}

	//> actualize video view
	edit->onPageActivated(true) ;

	edit->setFirstFocus(true) ;
}


void GuiWidget::first_last_tab(Glib::ustring direction, Gtk::Widget* widget)
{
	//> After removing last tab
	if (direction=="last")
	{
		icop_filemode.set_relief(Gtk::RELIEF_NONE);
		icop_synchro_signal_w_text.set_relief(Gtk::RELIEF_NONE);
		icop_synchro_text_w_signal.set_relief(Gtk::RELIEF_NONE);
		icop_filemode.set_sensitive(false) ;
		icop_clipboard.set_sensitive(false) ;
		toolB_speakerDicoLocal.hide() ;
		icop_synchro_signal_w_text.set_sensitive(false) ;
		icop_synchro_text_w_signal.set_sensitive(false) ;
		button_menu_highlight.set_sensitive(false) ;
		button_menu_display.set_sensitive(false) ;
		button_menu_tagDisplay.set_sensitive(false) ;
		combo_language.set_sensitive(false);
		clipboard->close(true) ;
		if (searchManager->is_active())
			searchManager->myHide() ;
		if (TsearchManager->is_active())
			TsearchManager->myHide() ;
		connection_input_language.disconnect();
	}
	//> After opening first tab
	else if (direction=="first")
	{
		icop_filemode.set_sensitive(true) ;
		icop_clipboard.set_sensitive(true) ;
		toolB_speakerDicoLocal.show() ;
		icop_synchro_signal_w_text.set_sensitive(true) ;
		icop_synchro_text_w_signal.set_sensitive(true) ;
		button_menu_highlight.set_sensitive(true) ;
		button_menu_tagDisplay.set_sensitive(true) ;
		combo_language.set_sensitive(true);

		AnnotationEditor* edit = (AnnotationEditor*)widget ;

		//> get synchronisation values
		if (edit->hasSignalView())
		{
			Glib::ustring swt = edit->getOption("synchro_signal_to_text") ;
			Glib::ustring tws = edit->getOption("synchro_text_to_signal") ;
			actualize_synchronisation(swt, tws) ;

			//> highlight
			int high = edit->getHighlightMode() ;
			actualize_highlight(high) ;

			//> display
			int display_mode = edit->getActiveViewMode() ;
			bool isStereo = edit->isStereo() ;
			actualize_display_options(isStereo,display_mode) ;

		}
		else
		{
			icop_synchro_signal_w_text.set_sensitive(false) ;
			icop_synchro_text_w_signal.set_sensitive(false) ;
			button_menu_highlight.set_sensitive(false) ;
		}

		//> get filemode value
		Glib::ustring mode = edit->getOption("mode") ;
		actualize_filemode(mode) ;

		int hidden = edit->getTagHiddenMode() ;
		actualize_tagDisplay(hidden) ;

		//> actualize action group
		add_action_groups(edit, true, true);
		note.setActiveWidget(edit) ;
	}
}


void GuiWidget::on_notebook_action(Glib::ustring mode)
{
	if (mode.compare("close_current")==0)
		note.close_current_page() ;
	else if (mode.compare("large_mode")==0)
		note.toggle_large_mode() ;
	else
		note.switch_tab(mode) ;
}

void GuiWidget::notebook_toggle_large_mode()
{
	note.toggle_large_mode() ;
}

void GuiWidget::on_notebook_largeMode(bool mode_large)
{
	if (mode_large) {
		status.hide() ;
		tool_box.hide() ;
		show_hide_explorer(false) ;
		lock_tree_button = true ;
		toolB_tree.set_active(false) ;
		lock_tree_button = false ;
	}
	else {
		show_hide_explorer(true) ;
		lock_tree_button = true ;
		toolB_tree.set_active(true) ;
		lock_tree_button = false ;

		int display_bar = config->get_GUI_toolbarShow() ;
		if (display_bar==1)
			tool_box.show() ;
		int display_status = config->get_GUI_statusbarShow() ;
		if (display_status==1)
			status.show() ;
	}
}

//
//void GuiWidget::toggle_show_hide(std::string mode)
//{
//	if (mode == "tree")
//	{
//
//		if (!visible) {
//			paned.get_child1()->hide_all() ;
//			visible_tree = false ;
//		}
//		else {
//			paned.get_child1()->show_all() ;
//			visible_tree = true ;
//		}
//	}
//
//
//}

//***************************************************************************************
//********************************************************************** MENU BAR METHODS
//***************************************************************************************


/*
 * For all actions of menu bar
 */
void GuiWidget::on_action_menu_activated(Glib::ustring action)
{
	if (action=="ShowHide")
		show_hide_explorer_by("menu") ;
	else if (action=="show_global_dico")
	{
		if (dicoManager)
			dicoManager->showGlobalDictionary() ;
	}
	else if (action=="show_file_dico")
	{
		AGEditor* editor = (AGEditor*)note.get_active_widget() ;
		if(dicoManager && editor)
			dicoManager->showLocalDictionary(editor, "", false) ;
	}
	else if (action=="SearchInFile")
		show_search_panel("text") ;
	else if (action=="SearchInAnnotations")
		show_search_panel("tag") ;
	else if (action=="Quit")
		can_quit() ;
	else if (action=="Documentation")
		show_documentation() ;
	else if (action=="About")
		about() ;
	else if (action=="Preferences")
		show_preferences_panel() ;
	else if (action=="show_hide_toolbar")
		show_hide_toolbar() ;
	else if (action=="Open")
		open() ;
	else if (action=="show_hide_language")
		show_hide_language() ;
	else if (action=="show_hide_statusbar")
		show_hide_statusbar() ;
	else if (action=="show_hide_statusbar")
		show_hide_statusbar() ;
	else if (action=="Create")
	{
		std::vector<Glib::ustring> paths ;
		create_new_transcription_by_dialog(paths) ;
	}
//	else if (action=="ShowHideNotebook") {
//		note.toggle_hide_tabs() ;
//	}
	else
		Explorer_dialog::msg_dialog_info("...ASAP..." , this, true) ;
}


void GuiWidget::remove_action_groups(Gtk::Widget* w, bool secure_mode)
{
	AnnotationEditor* a = (AnnotationEditor*) w ;
	if (a!=NULL) {
		menu.remove_actionGroup(a->getActionGroup("file"), secure_mode) ;
		menu.remove_actionGroup(a->getActionGroup("annotate"), secure_mode) ;
		menu.remove_actionGroup(a->getActionGroup("display"), secure_mode) ;
		menu.remove_actionGroup(a->getActionGroup("edit"), secure_mode) ;
		if (a->hasSignalView())
			menu.remove_actionGroup(a->getActionGroup("signal"), secure_mode) ;
		if (a->hasVideo())
			menu.remove_actionGroup(a->getActionGroup("video_signal"), secure_mode) ;
	}
}


void GuiWidget::add_action_groups(Gtk::Widget* w, bool actualize_ui, bool secure_mode)
{
	AnnotationEditor* a = (AnnotationEditor*) w ;

	//> add search action in edit
	Glib::RefPtr<Gtk::Action> action = a->getActionGroup("edit")->get_action("SearchInFile") ;
	if (!action) {
			a->getActionGroup("edit")->add( Gtk::Action::create("SearchInFile", Gtk::Stock::FIND, _("_Find/Replace"), _("Find/Replace in file")),
										Gtk::AccelKey("<control>F"),
										sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "SearchInFile" ));
	}

	a->getActionGroup("edit")->get_action("SearchInAnnotations") ;
	if (!action) {
			a->getActionGroup("edit")->add( Gtk::Action::create("SearchInAnnotations", Gtk::Stock::FIND, _("S_pecial search"), _("Special search")),
										Gtk::AccelKey("<control><alt>F"),
										sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "SearchInAnnotations" ));
	}

	action = a->getActionGroup("edit")->get_action("show_file_dico") ;
	if (!action && dicoManager) {
		//> add file dictionary in edit
		a->getActionGroup("edit")->add( Gtk::Action::create("show_file_dico", _("_File speaker dictionary"), _("Display file speaker dictionary")),
									Gtk::AccelKey("<mod1><shift>G"),
									sigc::bind<AGEditor*, Glib::ustring, bool>( sigc::mem_fun(dicoManager, &DictionaryManager::showLocalDictionary), a, " ", false ));
	}

	menu.insert_actionGroup(a->getActionGroup("file"),NULL, secure_mode) ;
	menu.insert_actionGroup(a->getActionGroup("annotate"),NULL, secure_mode) ;
	menu.insert_actionGroup(a->getActionGroup("edit"),NULL, secure_mode) ;
	menu.insert_actionGroup(a->getActionGroup("display"),NULL, secure_mode) ;

	if (a->hasSignalView())
		menu.insert_actionGroup(a->getActionGroup("signal"),NULL, secure_mode) ;

	if (a->hasVideo())
		menu.insert_actionGroup(a->getActionGroup("video_signal"),NULL, secure_mode) ;

	//> only set ui if first page
	if ( actualize_ui )
	{
		Glib::ustring ui_signal = a->getUIInfo("signal") ;
		Glib::ustring ui_annotate = a->getUIInfo("annotate") ;
		Glib::ustring ui_video_signal = a->getUIInfo("video_signal") ;

	    menu.remove_ui("annotate") ;
	    menu.remove_ui("signal") ;
	    menu.remove_ui("video_signal") ;

	    menu.add_ui(ui_annotate,"annotate") ;
	    if (a->hasSignalView())
	    	menu.add_ui(ui_signal,"signal") ;
	    if (a->hasVideo())
	    	menu.add_ui(ui_video_signal,"video_signal") ;
	}

	//> actualise combo_language each time we actualise the action group
	//  opening file, switching file, closing file
	InputLanguage* ilang ;
	ilang = a->get_input_language() ;
	if (ilang==NULL)
		ilang = InputLanguageHandler::get_input_language_by_shortcut(a->getTranscriptionLanguage()) ;
	if (ilang==NULL)
		ilang = InputLanguageHandler::get_input_language_by_shortcut(DEFAULT_LANGUAGE) ;
	onLanguageChanged(ilang, a);
}

void GuiWidget::on_reset_editor_aGroups(Glib::ustring type, Glib::RefPtr<Gtk::ActionGroup> oldgroup, Gtk::Widget* widget)
{
	AnnotationEditor* a = (AnnotationEditor*) widget ;
	//AnnotationEditor* old = (AnnotationEditor*) menu.get_previous_reset_widget() ;

	//> Only update action group if switching between view, inside same editor
	// MEANS editor must have passed at least one time before for being proceed
	// FOR its first passage, action groups must have been set by notebook switching mechanism
	if (type.compare("edit")==0)
	{
		if (a!=NULL)
		{
			Glib::RefPtr<Gtk::Action> action = a->getActionGroup(type)->get_action("SearchInFile") ;
			if (!action)
			{
				//> add search action in edit
				a->getActionGroup(type)->add( Gtk::Action::create("SearchInFile", Gtk::Stock::FIND, _("_Search"), _("Search in file")),
					Gtk::AccelKey("<control>F"),
					sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "SearchInFile" ));
			}

			a->getActionGroup("edit")->get_action("SearchInAnnotations") ;
			if (!action)
			{
					a->getActionGroup("edit")->add( Gtk::Action::create("SearchInAnnotations", Gtk::Stock::FIND, _("S_pecial search"), _("S_pecial search")),
												Gtk::AccelKey("<control><alt>F"),
												sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "SearchInAnnotations" ));
			}

			action = a->getActionGroup(type)->get_action("show_file_dico") ;
			if (!action && dicoManager)
			{
				//> add file dictionary in edit
				a->getActionGroup(type)->add( Gtk::Action::create("show_file_dico", _("_File speaker dictionary"), _("Display file speaker dictionary")),
					Gtk::AccelKey("<mod1><shift>G"),
					sigc::bind<AGEditor*, Glib::ustring, bool>( sigc::mem_fun(dicoManager, &DictionaryManager::showLocalDictionary), a, " ", false ));
			}

			//> nothing to be removed for first file
			menu.remove_actionGroup(oldgroup, true) ;
			menu.insert_actionGroup(a->getActionGroup(type),NULL, true) ;
		}
	}
}

/*
 * Stock open file in recent file menu, and actualize menu
 */
void GuiWidget::actualize_recent_file(Glib::ustring path)
{
	//> add in list
	Gtk::UIManager::ui_merge_id id = menu.get_mainMenu()->new_merge_id() ;
	Glib::ustring ui_path = "/MenuBar/FileMenu/RecentFiles";

	//> format path for well displaying the recent file name in menu
	Glib::ustring formatPath = Explorer_utils::format_recent_file(path) ;

	menu.add_recent_file(path, id, ui_path, Gtk::Action::create(path,formatPath,path),
   			 sigc::bind<Glib::ustring, Glib::ustring, bool>(sigc::mem_fun(*this, &GuiWidget::open_file), path, "openall", false) ) ;
}

Glib::ustring GuiWidget::setExternalToolMenu(bool global)
{
	ToolLauncher* toolLauncher = ToolLauncher::getInstance() ;
	if (!toolLauncher)
		return "" ;

	Glib::ustring ui = "" ;
	if (global)
	{
		// No global tools, exit
		if (!toolLauncher->hasGlobalScopeTools())
			return "" ;

		ui = "    	<menu action='ExternalTools'>" ;

		// Create main menu
		menu.get_actionGroup()->add( Gtk::Action::create("ExternalTools", _("_Tools") ));

		// For each external tool, add action and a ui string
		std::vector<ToolLauncher::Tool*> tools = toolLauncher->getTools() ;
		std::vector<ToolLauncher::Tool*>::const_iterator it ;
		for (it=tools.begin(); it!=tools.end(); it++)
		{
			// treat only global tool
			ToolLauncher::Tool* ctool = *it ;
			if (!ctool->isFileScope())
			{
				string name = ctool->getIdentifiant();
				string display = ctool->getDisplay();
				// action
				menu.get_actionGroup()->add( Gtk::Action::create(name, display, display), sigc::bind<ToolLauncher::Tool*>( sigc::mem_fun(*toolLauncher, &ToolLauncher::launch), ctool ));
				// menu
				ui.append("     		<menuitem action='" + name + "'/>") ;
			}
		}
		ui.append("    	</menu>") ;
	}
	else
	{
		// No global tools, exit
		if (!toolLauncher->hasFileScopeTools())
			return "" ;

		ui = "    	<menu action='ExternalToolsFile'>" ;

		// Create main menu
		menu.get_actionGroup()->add( Gtk::Action::create("ExternalToolsFile", _("_External actions") )) ;

		std::vector<ToolLauncher::Tool*> tools = toolLauncher->getTools() ;
		std::vector<ToolLauncher::Tool*>::const_iterator it ;
		for (it=tools.begin(); it!=tools.end(); it++)
		{
			// treat only global tool
			ToolLauncher::Tool* ctool = *it ;
			if (ctool->isFileScope())
			{
				//only add ui, action will be created by editor
				string name = ctool->getIdentifiant() ;
				string display = ctool->getDisplay() ;
				ui.append("     		<menuitem action='" + name + "'/>") ;
			}
		}
		ui.append("    	</menu>") ;
	}

	return ui ;
}

void GuiWidget::set_menu()
{
	//> SET MENU
	Glib::RefPtr<Gtk::UIManager> mainMenu = menu.get_mainMenu() ;
  	Glib::RefPtr<Gtk::ActionGroup> actionGroup = menu.get_actionGroup() ;

	// Main bar title
	menu.get_actionGroup()->add( Gtk::Action::create("FileMenu", _("_File") ));
	menu.get_actionGroup()->add( Gtk::Action::create("Edit", _("_Edit") ));
	menu.get_actionGroup()->add( Gtk::Action::create("AnnotateMenu", _("_Annotate") ));
	menu.get_actionGroup()->add( Gtk::Action::create("DisplayMenu", _("_Display") ));
	menu.get_actionGroup()->add( Gtk::Action::create("Speakers", _("Spea_kers") ));
	menu.get_actionGroup()->add( Gtk::Action::create("Search", _("_Search") ));
	menu.get_actionGroup()->add( Gtk::Action::create("SignalMenu", _("Si_gnal") ));
	menu.get_actionGroup()->add( Gtk::Action::create("videoMenu", _("_Video") ));
	menu.get_actionGroup()->add( Gtk::Action::create("Window", _("_Window") ));
	menu.get_actionGroup()->add( Gtk::Action::create("HelpMenu", _("_Help") ));

	//>> File menu submenu
	menu.get_actionGroup()->add( Gtk::Action::create("Create", Gtk::Stock::NEW, _("Create a new transcription"), _("Create a new transcription file")),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "Create" ));

	menu.get_actionGroup()->add( Gtk::Action::create("Open", Gtk::Stock::OPEN, _("Open"), _("Open a file")),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "Open"));
	menu.get_actionGroup()->add( Gtk::Action::create("Quit", Gtk::Stock::QUIT, _("Quit"), _("Quit")),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "Quit"));

	//>> RECENT FILES SUBMENU
	menu.get_actionGroup()->add( Gtk::Action::create("RecentFiles", _("_Recent Files") ));

	//>> Help menu submenu
	menu.get_actionGroup()->add( Gtk::Action::create("Manual", Gtk::Stock::HELP, _("_User manual"), "User manual"),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "Documentation" ));
	menu.get_actionGroup()->add( Gtk::Action::create("About", Gtk::Stock::ABOUT, _("_About"), "About"),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "About" ));

	//>> TREE VISIBILITY
	menu.get_actionGroup()->add( Gtk::Action::create("ShowHide", _("Show / Hide _explorer"), _("Show or hide file explorer")),
		Gtk::AccelKey("<control>J"),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "ShowHide" ));

	//> FOR GLOBAL DICTIONARY
	menu.get_actionGroup()->add( Gtk::Action::create("show_global_dico", _("_Global speaker dictionary"), _("Display the global speaker dictionary")),
		Gtk::AccelKey("<control>G"),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "show_global_dico" ));

	//> FOR NOTEBOOK ACTION
	menu.get_actionGroup()->add( Gtk::Action::create("notebook_next", _("Ne_xt tab"), _("Go to the next tab")),
		Gtk::AccelKey("<control>Page_Down"),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_notebook_action), "next" ));

	menu.get_actionGroup()->add( Gtk::Action::create("notebook_previous", _("Pre_vious tab"), _("Go to the previous tab")),
		Gtk::AccelKey("<control>Page_Up"),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_notebook_action), "previous" ));

	menu.get_actionGroup()->add( Gtk::Action::create("notebook_close", _("Cl_ose current page"), _("Close current page")),
		Gtk::AccelKey("<control>W"),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_notebook_action), "close_current" ));

	menu.get_actionGroup()->add( Gtk::Action::create("notebook_large", _("_Switch to enlarged editor display"), _("Switch to enlarged editor display")),
		Gtk::AccelKey("<control>comma"),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_notebook_action), "large_mode" ));

	//>> NOTEBOOK VISIBILITY
//	menu.get_actionGroup()->add( Gtk::Action::create("ShowHideNotebook", _("Show / Hide _notebook tabs"), _("Show or hide notebook tabs")),
//		Gtk::AccelKey("<control>I"),
//		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "ShowHideNotebook" ));

	//> FOR FILE PROPERTIES
	menu.get_actionGroup()->add( Gtk::Action::create("file_properties", Gtk::Stock::PROPERTIES,  _("_File properties"), _("Display file properties")),
		Gtk::AccelKey("<mod1><shift>F"),
		( sigc::mem_fun(*this, &GuiWidget::show_file_properties) ) );

	//> FOR CLIPBOARD
	//sub menu
	menu.get_actionGroup()->add( Gtk::Action::create("Clipboard", _("_Clipboard") ));
	menu.get_actionGroup()->add( Gtk::Action::create("Revert", _("_Revert file") ));
	menu.get_actionGroup()->add( Gtk::Action::create("Shortcut_show_clipboard", _("Display _clipboard"), _("Display clipboard")),
		Gtk::AccelKey("<mod1><shift>C"),
		( sigc::mem_fun(*this, &GuiWidget::shortcut_show_clipboard) ) );

	//> FOR Preferences
	menu.get_actionGroup()->add( Gtk::Action::create("Preferences", _("_Preferences"), _("Display preferences panel")),
		Gtk::AccelKey("<control>M"),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "Preferences" ));

	//> FOR TOOLBAR VISIBILITY
	menu.get_actionGroup()->add( Gtk::Action::create("show_hide_toolbar", _("Show / Hide _toolbar"), _("Show / Hide toolbar")),
		Gtk::AccelKey("<control>k"),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "show_hide_toolbar" ));

	//> FOR status bar visibility
	menu.get_actionGroup()->add( Gtk::Action::create("show_hide_statusbar", _("Show / Hide _status bar"), _("Show / Hide status bar")),
		sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::on_action_menu_activated), "show_hide_statusbar" ));

	//actions
	menu.insert_actionGroup(clipboard->get_actionGroup(), clipboard, true) ;

	Glib::ustring toolUIglobal = setExternalToolMenu(true) ;
	Glib::ustring toolUIfile = setExternalToolMenu(false) ;
	setMenuUI(toolUIglobal, toolUIfile) ;
}

void GuiWidget::setMenuUI(Glib::ustring tool_ui_global, Glib::ustring tool_ui_file)
{
	//> Construct menu WITHOUT OPENED FILE
	string default_ui, default_ui1, default_ui2, default_ui3, default_ui4 ;
	default_ui1 =
       "<ui>"
        "  <menubar name='MenuBar'>"
        "    	<menu action='FileMenu'>"
        "     		<menuitem action='Open'/>"
        "     		<menuitem action='Create'/>"
		"			<separator/>"
        "      		<menu action='RecentFiles'/>"
        "			<separator/>"
        "     		<menuitem action='Quit' position= 'bottom'/>"
        "    	</menu>"
		"    	<menu action='Edit'/>"
		"    	<menu action='DisplayMenu'/>"
        "    	<menu action='Search'/>"
        "    	<menu action='AnnotateMenu'/>"
        "    	<menu action='SignalMenu'/>"
        "    	<menu action='videoMenu'/>"
        "    	<menu action='Speakers'>"
        "      		<menuitem action='show_global_dico'/>"
        "    	</menu>"
        "    	<menu action='Window'>"
        "     		<menuitem action='show_hide_toolbar'/>"
        "			<separator/>"
        "     		<menuitem action='show_hide_statusbar'/>"
        "			<separator/>"
        "     		<menuitem action='ShowHide'/>";
//		"			<separator/>"
//		"     		<menuitem action='ShowHideNotebook'/>";
	default_ui2 = "" ;

	default_ui3 =
		"	        <separator/>"
        "      		<menuitem action='Preferences'/>"
        "    	</menu>" ;
	default_ui4 =
		"    	<menu action='HelpMenu' position='bottom'>"
        "      		<menuitem action='About'/>"
        "      		<menuitem action='Manual'/>"
        "    	</menu>"
        "  </menubar>"
        "  <toolbar  name='Toolbar'>"
        "    <toolitem action='Quit'/>"
        "	 <separator/>"
        "    <toolitem action='Open'/>"
        "	 <separator/>"
		"  </toolbar>"
        "</ui>";

	default_ui = default_ui1 + default_ui2 + default_ui3 + tool_ui_global + default_ui4 ;



	//> construct menu WITH OPENED FILE
	string general_ui, general_ui1, general_ui2, general_ui3, general_ui6, general_ui7, general_ui8 ;
	general_ui1 =
		   "<ui>"
			"  <menubar name='MenuBar'>"
			"    	<menu action='FileMenu'>"
			"     		<menuitem action='Open'/>"
			"     		<menuitem action='Create'/>"
			"  			<separator/>"
	        "      		<menuitem action='file_save'/>"
	        "      		<menuitem action='file_saveas'/>"
	        "      		<menuitem action='file_close' />"
			"		    <separator/>"
			"      		<menuitem action='file_export' />"
			"		    <separator/>"
			"			<menuitem action='filemode'/>"
			"		    <separator/>"
		    "	    	<menu action='Revert'>"
			" 		    	<menuitem action='file_revert_from_file'/>"
			" 		    	<menuitem action='file_revert_from_autosave'/>"
			"			</menu>"
	        "		    <separator/>"
        	" 		    <menuitem action='file_refresh'/>"
			"			<separator/>"
	        "      		<menu action='RecentFiles'/>"
	        "			<separator/>";

Glib::ustring	general_ui5 =	        "     		<menuitem action='Quit'/>"
	        "    	</menu>"
	        "    	<menu action='Edit'>"
	        "      		<menuitem action='edit_undo'/>"
	        "      		<menuitem action='edit_redo'/>"
			"  			<separator/>"
	        "      		<menuitem action='edit_copy'/>"
			"      		<menuitem action='edit_paste'/>"
	        "      		<menuitem action='edit_paste_special'/>"
	        "      		<menuitem action='edit_cut'/>"
			"  			<separator/>"
			"			<separator/>"
		    "	    	<menu action='Clipboard'>"
			" 	     		<menuitem action='clipboard_display'/>"
		    " 	     		<menuitem action='clipboard_up'/>"
		    "   	   		<menuitem action='clipboard_down'/>"
		    "      			<menuitem action='clipboard_import'/>"
		    "      			<menuitem action='clipboard_export'/>"
		    "      			<menuitem action='clipboard_clear'/>"
		    "      			<menuitem action='clipboard_suppress'/>"
			"			</menu>"
			"  			<separator/>"
		    "	    	<menu action='edit_language'>"
		    "   	   		<menuitem action='edit_next_input_language'/>"
		    "      			<menuitem action='edit_previous_input_language'/>"
		    "			</menu>"
	        "    	</menu>"
			"    	<menu action='DisplayMenu'>"
			"      		<menuitem action='hideTags'/>"
			"      		<menuitem action='highlight'/>"
			"      		<menuitem action='display'/>"
			"    	</menu>"
	        "    	<menu action='Search'>"
	        "      		<menuitem action='SearchInFile'/>"
			"      		<menuitem action='SearchInAnnotations'/>"
	        "    	</menu>"
	        "    	<menu action='AnnotateMenu'/>"
	        "    	<menu action='SignalMenu'/>"
	        "    	<menu action='videoMenu'/>"
	        "    	<menu action='Speakers'>"
        	"      		<menuitem action='show_global_dico'/>"
	        "      		<menuitem action='show_file_dico'/>"
	        "    	</menu>"
	        "    	<menu action='Window'>"
	        "     		<menuitem action='show_hide_toolbar'/>"
	        "			<separator/>"
	        "     		<menuitem action='show_hide_statusbar'/>"
	        "			<separator/>"
	        "     		<menuitem action='ShowHide'/>"
	        "			<separator/>"
			"     		<menuitem action='notebook_large'/>"
			"			<separator/>"
	        "     		<menuitem action='notebook_previous'/>"
	        "     		<menuitem action='notebook_next'/>"
        "     		<menuitem action='notebook_close'/>" ;

	general_ui2 = "" ;

	general_ui3 =
	        "			<separator/>"
	        "     		<menuitem action='Shortcut_show_clipboard'/>"
	        "			<separator/>"
	        "      		<menuitem action='file_properties' />"
			"	        <separator/>"
	        "      		<menuitem action='Preferences'/>"
			"    	</menu>";
	general_ui6 =
	        "    	<menu action='HelpMenu' position='bottom'>"
	        "      		<menuitem action='About'/>"
        	"      		<menuitem action='Manual'/>"
	        "    	</menu>"
	        "  </menubar>"
		    "  <toolbar  name='Toolbar'>"
	        "	 <separator/>"
	        "    <toolitem action='Quit'/>"
			"  	 <separator/>"
	        "    <toolitem action='Open'/>"
			"  	 <separator/>"
	        "    <toolitem action='file_save'/>"
			"    <toolitem action='file_close'/>"
			"    <toolitem action='file_refresh'/>" ;

	general_ui8 =
			"  	 <separator/>"
	        "    <toolitem action='edit_undo'/>"
	        "    <toolitem action='edit_redo'/>"
			"  	 <separator/>"
	        "    <toolitem action='edit_copy'/>"
	        "    <toolitem action='edit_paste'/>"
	        "    <toolitem action='edit_cut'/>"
	        "	 <separator/>"
			"    <toolitem action='SearchInFile'/>"
			"    <toolitem action='file_properties'/>"
			"  	 <separator/>"
	       "  </toolbar>"
	        "</ui>" ;

	general_ui = general_ui1 + tool_ui_file + general_ui5 + general_ui2
			+ general_ui3 + tool_ui_global + general_ui6 + general_ui8 ;

	menu.set_ui(default_ui,"default") ;
	menu.set_ui(general_ui,"general") ;

    menu.add_ui(default_ui,"default") ;

    //> set action with menu
    menu.set_menu_with_action(this) ;

    //> connect menu to notebook signal for removing actiongroup when closing the last file
	note.signalRemoveActionGroup().connect( sigc::bind<bool>(sigc::mem_fun(&menu, &Explorer_menu::remove_actionGroup), true)) ;

    //> connect menu to notebook signal for removing ui when closing th last file
	note.signalActualiseUI().connect(sigc::mem_fun(&menu, &Explorer_menu::switch_ui)) ;
}


//***************************************************************************************
//****************************************************************** ARBORESCENCE METHODS
//***************************************************************************************
void GuiWidget::set_expander()
{
	Glib::ustring expander_space = "   " ;
	Glib::ustring expander_space_shortcut = expander_space ;

	//> system
	system_expander.set_label(_("SYSTEM")) ;
	system_expander.set_name("expander_tree") ;
	system_expander.add(system_hbox) ;
		system_hbox.pack_start(system_blank, false, false) ;
	    system_hbox.pack_start(system_box, true, true) ;
		system_blank.set_label(expander_space) ;
	system_expander.get_label_widget()->set_name("expander_tree_label");
	system_expander.set_expanded(true) ;
	treeBox.pack_start(system_expander, false, false) ;
	treeBox.pack_start(sep1, false, false) ;

	//> Shortcut
	shortcut_expander.set_name("expander_tree") ;
	shortcut_expander.set_label(_("MY SHORTCUTS")) ;
	shortcut_expander.add(shortcut_hbox) ;
	    shortcut_hbox.pack_start(shortcut_blank, false, false) ;
	    shortcut_hbox.pack_start(shortcut_box, true, true) ;
		shortcut_blank.set_label(expander_space_shortcut) ;
	shortcut_expander.get_label_widget()->set_name("expander_tree_label");
	shortcut_expander.set_expanded(true) ;

	treeBox.pack_start(shortcut_expander, false, false) ;

	shortcut_box.pack_start(shortcut_button_align, false, false) ;
	shortcut_button_align.set(Gtk::ALIGN_LEFT, Gtk::ALIGN_LEFT, 0, 0) ;
	shortcut_button_align.add(shortcut_button_box) ;

	shortcut_button_box.pack_start(shortcut_button_BLANK, false, false) ;
	shortcut_button_box.pack_start(shortcut_button, false, false) ;
	shortcut_button_box.pack_start(shortcut_button_label, false, false) ;

	shortcut_button.set_icon(ICO_TREE_SHORTCUT, _("Add a new shortcut"), 19, _("Add a new shortcut")) ;
	shortcut_button.signal_clicked().connect( sigc::bind<Glib::ustring, Glib::ustring, bool>( sigc::mem_fun(*this, &GuiWidget::on_create_shortcut), "", "", false)) ;
	shortcut_button.set_relief(Gtk::RELIEF_NONE);

	shortcut_button_BLANK.set_label("");
}

void GuiWidget::add_default_tree()
{
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> SYSTEM
	Explorer_tree* t = treeManager->add_tree("/", _("My Computer"), 2, true) ;
	pack_tree(t, &system_box) ;

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> DEFAULT SHORTCUT
	t = treeManager->add_tree(config->get_WORKDIR_path(), _("Work AG"), 3, true) ;
	pack_tree(t, &shortcut_box) ;
}

void GuiWidget::connect_tree(Explorer_tree* tree)
{
	set_popup_menu( tree->get_popup(), tree) ;
}


void GuiWidget::set_filter(Explorer_filter* filter)
{
	label_combo.set_label(_(" Filter  ")) ;
	//filter->get_combo()->set_title(_("filter: ")) ;
	filter->get_combo()->signal_changed().connect( sigc::mem_fun(*this, &GuiWidget::on_filter_combo_changed) ) ;
}

/*
 * Actualise filter when the combo-box is used
 * */
void GuiWidget::on_filter_combo_changed()
{
	//> get combo option chosen and actualise filter
	Glib::ustring chosen = filter->get_combo()->get_active_text() ;
	filter->set_filter_chosen(chosen) ;

	//> Filter all trees activated
	treeManager->refilter_all() ;
}

/*
 * Define all possibilities given by Popup menu of the treeview
 */
void GuiWidget::set_popup_menu(Explorer_popup* popup, Explorer_tree* tree)
{
	//>  CONSTRUCT POPUP MENU OPTION
	Glib::ustring name ;
	std::vector<Glib::ustring> options ;

	//> ACTION NAME DEFINED IN EXPLORER_POPUP
	options.insert(options.end(), POPUP_TREE_NEW_DIR) ;
	options.insert(options.end(), POPUP_TREE_PLAY) ;
	options.insert(options.end(), POPUP_TREE_OPEN) ;
	options.insert(options.end(), POPUP_TREE_CREATE_SINGLETRANSCRIPTION) ;
	options.insert(options.end(), POPUP_TREE_CREATE_MULTITRANSCRIPTION) ;
	options.insert(options.end(), POPUP_TREE_RENAME) ;
	options.insert(options.end(), POPUP_TREE_SUPPRESS) ;
	options.insert(options.end(), POPUP_TREE_IMPORT_IN) ;
	options.insert(options.end(), POPUP_TREE_PROPERTY) ;
	options.insert(options.end(), POPUP_TREE_IMPORT) ;
	options.insert(options.end(), POPUP_TREE_COPY) ;
	options.insert(options.end(), POPUP_TREE_PASTE) ;
	options.insert(options.end(), POPUP_TREE_CUT) ;
	options.insert(options.end(), POPUP_TREE_CHANGESHORTCUT) ;
	options.insert(options.end(), POPUP_TREE_DELETESHORTCUT) ;
	options.insert(options.end(), POPUP_TREE_REFRESHDIR) ;
	options.insert(options.end(), POPUP_TREE_DEFINESHORTCUT) ;

	int i=0 ;
	Gtk::MenuItem* item = NULL ;
	Gtk::Image* image = NULL ;

	//play
	name = options[i] ;
	image = popup->create_image(ICO_POPUP_DIR) ;
	item =  new Gtk::ImageMenuItem(*image, name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//new dir
	name = options[i] ;
	image = popup->create_image(ICO_POPUP_DIR) ;
	item =  new Gtk::ImageMenuItem(*image, name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//open
	name = options[i] ;
	item =  new Gtk::MenuItem(name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//create simple transcription
	name = options[i] ;
	item =  new Gtk::MenuItem(name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//create multi transcription
	name = options[i] ;
	item =  new Gtk::MenuItem(name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//rename
	name = options[i] ;
	item =  new Gtk::MenuItem(name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//suppress
	name = options[i] ;
	item =  new Gtk::ImageMenuItem(Gtk::Stock::DELETE) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;


	//import in
	name = options[i] ;
	item =  new Gtk::MenuItem(name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//property
	name = options[i] ;
	image = popup->create_image(ICO_POPUP_PROPERTIES) ;
	item =  new Gtk::ImageMenuItem(*image, name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//import
	name = options[i] ;
	item =  new Gtk::MenuItem(name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//copy
	name = options[i] ;
	item =  new Gtk::ImageMenuItem(Gtk::Stock::COPY) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//paste
	name = options[i] ;
	item =  new Gtk::ImageMenuItem(Gtk::Stock::PASTE) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;
	item->set_sensitive(false) ;

	//cut
	name = options[i] ;
	item =  new Gtk::ImageMenuItem(Gtk::Stock::CUT) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//change shortcut
	name = options[i] ;
	image = popup->create_image(ICO_POPUP_MODIFYSHORTCUT) ;
	item =  new Gtk::ImageMenuItem(*image, name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//delete shortcut
	name = options[i] ;
	image = popup->create_image(ICO_POPUP_DELETESHORTCUT) ;
	item =  new Gtk::ImageMenuItem(*image, name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//refresh dir
	name = options[i] ;
	image = popup->create_image(ICO_POPUP_REFRESH) ;
	item =  new Gtk::ImageMenuItem(*image, name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;

	//define shortcut
	name = options[i] ;
	image = popup->create_image(ICO_POPUP_DEFINESHORTCUT) ;
	item =  new Gtk::ImageMenuItem(*image, name, true) ;
	popup->insert_item(item, image, name) ;
	image = NULL ;
	item->signal_activate().connect(sigc::bind<Glib::ustring,Explorer_tree*>(sigc::mem_fun(*this, &GuiWidget::on_popup_menu), name, tree )) ;
	i++ ;
}

/*
 *  Define all actions when popup menu of treeview is used
 */
void GuiWidget::on_popup_menu(Glib::ustring action, Explorer_tree* tree)
{
	TreeModel_Columns m ;
	if(action==POPUP_TREE_OPEN)
	{
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		Glib::ustring path = tree->compute_path_from_node(*sorted_iter, true) ;
		//path = tree->get_popup_active_file() ;
		open_file(path, "openall") ;
	}
	else if(action==POPUP_TREE_RENAME)
	{
		// get iterator on selected row of filtered model
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		Glib::ustring path = tree->compute_path_from_node(*sorted_iter, true) ;
		if (note.is_opened_file(path)!=-1)
			Explorer_dialog::msg_dialog_warning(_("Please close file before changing its name"), this, true);
		// rename file
		else
			tree->rename_file(sorted_iter, this) ;
	}
	else if(action==POPUP_TREE_SUPPRESS)
	{
		// get iterator on selected row of filtered model
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		Glib::ustring path = tree->compute_path_from_node(*sorted_iter, true) ;
		if (note.is_opened_file(path)!=-1)
			Explorer_dialog::msg_dialog_warning(_("Please close file before removing it"), this, true);
		// remove
		else
			tree->remove_file(sorted_iter, this) ;
	}
	else if(action==POPUP_TREE_NEW_DIR)
	{
		// get iterator on selected row of filtered model
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		//> remove
		int ok = tree->create_directory(sorted_iter, this) ;
		if (ok==-1) {
			Explorer_dialog::msg_dialog_error(_("The directory can't be created, check your permission"),this, true);
		}
	}
	else if(action==POPUP_TREE_CUT)
	{
		// get iterator on selected row of filtered model
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		Glib::ustring path = tree->compute_path_from_node(*sorted_iter, true) ;
		if (note.is_opened_file(path)!=-1)
			Explorer_dialog::msg_dialog_warning(_("Please close file"), this, true);
		else
			treeManager->set_cut(*sorted_iter) ;
	}
	else if(action==POPUP_TREE_COPY)
	{
		// get iterator on selected row of filtered model
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		treeManager->set_copy(*sorted_iter) ;
	}
	else if(action==POPUP_TREE_PASTE)
	{
		// get iterator on selected row of filtered model
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		treeManager->paste(*sorted_iter) ;
	}
	else if(action==POPUP_TREE_CHANGESHORTCUT)
	{
		// get iterator on selected row of filtered model
		treeManager->change_target_tree(this, tree, config) ;

		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		tree->refresh_directory(*sorted_iter, true, true /*, NULL*/) ;
	}
	else if (action==POPUP_TREE_DELETESHORTCUT)
	{
		on_delete_shortcut(tree) ;
	}
	else if (action==POPUP_TREE_REFRESHDIR)
	{
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		tree->refresh_directory(*sorted_iter, true, true /*, NULL*/) ;
	}
	else if (action==POPUP_TREE_PROPERTY) {
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		show_filePropertyDialog(*sorted_iter, tree) ;
	}
	else if (action==POPUP_TREE_DEFINESHORTCUT)
	{
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		Glib::ustring path = tree->compute_path_from_node(*sorted_iter, true) ;
		on_define_shortcut(path) ;
	}
	else if (action==POPUP_TREE_CREATE_SINGLETRANSCRIPTION)
	{
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		Glib::ustring path = tree->compute_path_from_node(*sorted_iter, true) ;
		open_file(path, "newannot") ;
	}
	else if (action==POPUP_TREE_CREATE_MULTITRANSCRIPTION)
	{
		Gtk::TreeIter* sorted_iter = tree->get_popup_active_iter() ;
		Glib::ustring path = tree->compute_path_from_node(*sorted_iter, true) ;
		open_file(path, "newmultiannot") ;
	}
	else
		Explorer_dialog::msg_dialog_info(_("Not yet available"), this, true) ;
}

void GuiWidget::on_define_shortcut(Glib::ustring path)
{
	Glib::ustring display = Glib::path_get_basename(path) ;
	on_create_shortcut(path, display, true) ;
}

void GuiWidget::on_create_shortcut(Glib::ustring path, Glib::ustring display, bool defining)
{
	ShortcutDialog* dialog = new ShortcutDialog(-1, false) ;
    dialog->set_treeManager(treeManager);
#ifdef __APPLE__
    dialog->set_transient_for(*this);
#endif
    int rep ;

	if ( (path=="" && display=="") || defining) {
		if (defining)
			dialog->set_default(path, display) ;
		rep = dialog->run() ;
		path =  dialog->get_chosen_path() ;
		display = dialog->get_chosen_display() ;
	}
	else
		rep=Gtk::RESPONSE_OK ;

	if (rep==Gtk::RESPONSE_OK) {
		Explorer_tree* t = treeManager->add_shortcutTree(path, display) ;
		pack_tree(t, &shortcut_box) ;
	}
	if (dialog)
		delete(dialog) ;
}

void GuiWidget::pack_tree(Explorer_tree* tree, Gtk::VBox* box)
{
	if (tree!=NULL){
		connect_tree(tree) ;
		box->pack_start(*(tree->get_view()), false, false) ;
		box->pack_start(*(tree->get_separator()), false, false) ;
		box->show_all_children() ;
	}
	else
		TRACE << "TranscriberAG --> (!) Problem when loading tree" << std::endl ;
}


void GuiWidget::on_delete_shortcut(Explorer_tree* tree)
{
	Glib::ustring txt = _("Do you want to delete the shortcurt") ;
	Glib::ustring name = " " + tree->get_rootDisplay() ;
	int res = Explorer_dialog::msg_dialog_question(txt+name, this, true,"") ;

	if (res==Gtk::RESPONSE_YES)
	{
		Gtk::Window* w = (Gtk::Window*) tree->get_view() ;
		shortcut_box.remove(*w) ;
		shortcut_box.remove( *(tree->get_separator()) ) ;
		treeManager->delete_tree(tree) ;
	}
}


//**************************************************************************************
//************************************************************************** FILE ACTION
//**************************************************************************************

void GuiWidget::open()
{
	Gtk::FileChooserDialog file_chooser(_("Open annotation file"), Gtk::FILE_CHOOSER_ACTION_OPEN);
	Icons::set_window_icon(&file_chooser, ICO_TRANSCRIBER, 11) ;

	Explorer_filter* filter = Explorer_filter::getInstance() ;

	if (!filter)
		return ;

	Gtk::FileFilter filter_ANNOT;
    Gtk::FileFilter filter_AUDIO;
	Gtk::FileFilter filter_VIDEO;

	//> ANNOTATION TYPE FOR IMPORT
	std::map<Glib::ustring, Glib::ustring> tmp2 = filter->get_import_annotations() ;
    std::map<Glib::ustring, Glib::ustring>::iterator it2 ;
	for(it2=tmp2.begin(); it2!=tmp2.end(); it2++) {
		filter_ANNOT.add_pattern("*" + (it2->second).uppercase()) ;
		filter_ANNOT.add_pattern("*" + (it2->second).lowercase());
	}

	//> AUDIO FILE
	std::vector<Glib::ustring>::iterator it ;
	std::vector<Glib::ustring> tmp ;
	tmp.clear() ;
	tmp = filter->get_audio_extensions() ;
	for(it=tmp.begin(); it!=tmp.end(); it++){
		filter_AUDIO.add_pattern("*" + (*it).uppercase()) ;
		filter_AUDIO.add_pattern("*" + (*it).lowercase());
	}

	//> VIDEO FILE
	tmp.clear() ;
	tmp = filter->get_video_extensions() ;
	for(it=tmp.begin(); it!=tmp.end(); it++)
	{
        filter_VIDEO.add_pattern("*" + (*it).uppercase()) ;
        filter_VIDEO.add_pattern("*" + (*it).lowercase());
    }

	filter_ANNOT.set_name(_("Transcription files")) ;
    file_chooser.add_filter(filter_ANNOT);

	filter_AUDIO.set_name(_("Audio files")) ;
    file_chooser.add_filter(filter_AUDIO);

	filter_VIDEO.set_name(_("Video files")) ;
	file_chooser.add_filter(filter_VIDEO);

    file_chooser.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
    file_chooser.add_button (Gtk::Stock::OK, Gtk::RESPONSE_OK) ;
    file_chooser.set_select_multiple (false) ;

#ifdef __APPLE__
    file_chooser.set_transient_for(*this);
#endif
	if ( file_chooser.run() ==  Gtk::RESPONSE_OK )
		open_file(file_chooser.get_filename(), "openall");
}


/*
 * Lauch open file actions, depending of its type
 */
void GuiWidget::open_file(Glib::ustring path, Glib::ustring mode, bool threadProtection)
{
	TreeModel_Columns m ;
	Glib::ustring name = Glib::path_get_basename(path) ;
	Explorer_utils::print_trace("TranscriberAG --> <o> Opening file ", path, 1) ;

	//check if opened
	if( note.is_opened_file(path)!=-1 )
	{
		Gtk::MessageDialog dialog(*this, _("This file is being edited") ) ;
		if (threadProtection)
			gdk_threads_enter() ;
#ifdef __APPLE__
        dialog.set_transient_for(*this);
#endif
        dialog.run();
		if (threadProtection)
			gdk_threads_leave() ;
	}
	else
	{
		if (mode.compare("openall")==0)
			open_action(path, name, threadProtection) ;
		else if (mode.compare("newannot")==0)
		{
			std::vector<Glib::ustring> audio_paths ;
			audio_paths.push_back(path) ;
			create_new_transcription(audio_paths) ;
		}
		else if (mode.compare("newmultiannot")==0)
		{
			std::vector<Glib::ustring> audio_paths ;
			audio_paths.push_back(path) ;
			create_new_transcription_by_dialog(audio_paths) ;
		}
	}
	Explorer_utils::print_trace("TranscriberAG --> <o> Opening file launched", 1) ;
}

void GuiWidget::open_action(Glib::ustring path, Glib::ustring name, bool threadProtect)
{
	//> OPEN SINGLE ANNOTATION FILE
	if ( filter->is_import_annotation_file(path) )
	{
		std::vector<Glib::ustring> no_audios ;
		prepare_editor_afterIdle(path, no_audios, true) ;
	}

	//> OPEN AUDIO FILE
	//TODO let all audio formats
	else if	 ( filter->is_audio_file(path) )
	{
		//> Search associated TRS or TAG
		Glib::ustring linked_annot = FileHelper::exist_transcription_file(path) ;
		bool exist ;
		if (linked_annot=="")
			exist=false ;
		else
			exist = true ;
		//> Propose to use existing annotation file
		int result = Explorer_dialog::msg_dialog_open_audio(exist, linked_annot, this) ;

		//> if want to use existing annotation file
		if ( (result==Gtk::RESPONSE_NO) )
			open_file(linked_annot, "openall") ;

		//> else open audio file in order to get new annotation file
		else if ( result==Gtk::RESPONSE_YES || result==Gtk::RESPONSE_OK )
		{
			std::vector<Glib::ustring> audio_paths ;
			audio_paths.push_back(path) ;
			create_new_transcription(audio_paths) ;
		}
	}

	//> SPECIAL FAST VIDEO MODE: OPEN VIDEO FILE
    else if  ( filter->is_video_file(path) )
    {
        Explorer_utils::print_trace("TranscriberAG --> <o> Opening file in video mode", 1) ;
        std::vector<Glib::ustring> audio_paths ;
        audio_paths.push_back(path) ;
        create_new_transcription(audio_paths) ;
    }
	//> NOT SUPPORTED FILE
	else
	{
		//TRACE << "open_file:> file type unknown" <<  std::endl ;
		Gtk::MessageDialog dialog(*this, _("This file type is not supported by Transcriber") ) ;
		if (threadProtect)
			gdk_threads_enter() ;
		dialog.run();
		if (threadProtect)
			gdk_threads_leave() ;
	}
}

void GuiWidget::create_new_transcription(const std::vector<Glib::ustring>& audio_paths)
{
	if ( audio_paths.size() )
		prepare_editor_afterIdle("", audio_paths, false) ;
}

void GuiWidget::create_new_transcription_by_dialog(std::vector<Glib::ustring>& audio_paths)
{
	Explorer_filter* filter = Explorer_filter::getInstance() ;
	std::vector<Glib::ustring> extensions = filter->get_audio_extensions()  ;
	CreateTranscriptionDialog dialog(audio_paths, extensions) ;

	#ifdef __APPLE__
	dialog.set_transient_for(*this);
	#endif

	int res = dialog.run() ;
	if (res == Gtk::RESPONSE_APPLY && audio_paths.size()>0)
	{
		int ok = 2 ;
		if (audio_paths.size()>=2)
			ok = Explorer_fileHelper::check_multi_audio(audio_paths) ;

		Glib::ustring text ;
		if (ok==-2) {
			text = _("Multi-audio transcription accepts mono audio files only.") ;
			Explorer_dialog::msg_dialog_warning(text, this, true) ;
		}
		else if (ok==-1) {
			text = _("Multi-audio transcription accepts files of same type only.") ;
			Explorer_dialog::msg_dialog_warning(text, this, true) ;
		}
		else if (ok==-3) {
			text = _("Multi-audio transcription needs different files.") ;
			Explorer_dialog::msg_dialog_warning(text, this, true) ;
		}
		else if (ok==1) {
			text = _("Selected files have different frame rates, it may cause a distorted sound") ;
			Explorer_dialog::msg_dialog_warning(text, this, true) ;
			prepare_editor_afterIdle("", audio_paths, false) ;
		}
		else
			prepare_editor_afterIdle("", audio_paths, false) ;
	}
}


void GuiWidget::prepare_editor_afterIdle(Glib::ustring file, std::vector<Glib::ustring> audios, bool saveState)
{
	// -- Add a new annotation editor
	Gtk::Widget* w = note.tab_annotation_editor(this, file, saveState) ;
	AnnotationEditor* tmp = (AnnotationEditor*)w ;
	Explorer_utils::print_trace("TranscriberAG --> <o> Editor packed", 1) ;

	// -- Font Flag --
    #ifdef __APPLE__
	tmp->setFontEngineStatus(fontInitialized);
	fontInitialized = true;
    #endif

	// -- display signals
	tmp->signalStatusBar().connect(sigc::mem_fun(*this, &GuiWidget::onStatusBar)) ;
	note.signalStatusBar().connect(sigc::mem_fun(*this, &GuiWidget::onStatusBar)) ;

	// -- Ready to prepare it
	Glib::signal_idle().connect(sigc::bind<Glib::ustring,std::vector<Glib::ustring>,bool,AnnotationEditor*>(sigc::mem_fun(*this, &GuiWidget::prepare_editor), file, audios, saveState, tmp)) ;
}

bool GuiWidget::prepare_editor(Glib::ustring file, std::vector<Glib::ustring> audios, bool saveState, AnnotationEditor* editor)
{
	Explorer_utils::print_trace("TranscriberAG --> <o> Setting editor ...", 1) ;

	//> do classic behaviour with first file
	Glib::ustring audio = "" ;
	if (audios.size()>0)
		audio = audios[0] ;

	editor->setOptions(*parameters);
	Glib::ustring default_name = editor->makeDefaultFilename(file, audio) ;

	//> Test if an opened file but not saved, with same name, is already opened
	// if open just tell user
	if (note.is_opened_file(default_name)!=-1)
	{
		gdk_threads_enter() ;
		Explorer_dialog::msg_dialog_warning(_("This file is being edited"), this, true) ;
		gdk_threads_leave() ;
		note.cancel_tab_annotation_editor() ;
		Explorer_utils::print_trace("TranscriberAG --> <o> Already opened, aborted.", 1) ;
	}
	// else process opening
	else
	{
		bool lockForced = false ;
		bool streaming = false ;
		int reportLevel = -1 ;
		if ( commandLine )
			reportLevel = commandLine->getReportLevel() ;
		std::vector<string> saudios;
		std::vector<Glib::ustring>::iterator ita;
		for (ita=audios.begin(); ita!=audios.end(); ++ita) saudios.push_back((std::string)*ita);

		gdk_threads_enter() ;
		bool ok = editor->loadFile(file, saudios, streaming, lockForced, reportLevel);
		gdk_threads_leave() ;
		Explorer_utils::print_trace("TranscriberAG --> <o> Editor loaded", 1) ;

		if (ok)
		{
			// -- Force editor display preparation
			editor->setDefaultViewMode(false, true) ;

			// -- Connect for drag n drop
			editor->addDragAndDropTarget(treeManager->get_dragDropTarget()) ;
			editor->addDragAndDropTarget(dicoManager->getDragAndDropTarget()) ;
			editor->signalGtkDragTarget().connect(sigc::mem_fun(*this, &GuiWidget::onEditorSignalDragDropReceived));

			// -- Connect to ui changement
			editor->signalUpdateUI().connect( sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &GuiWidget::on_reset_editor_aGroups), (Gtk::Widget*)editor)) ;
			editor->signalEditSpeaker().connect(sigc::mem_fun(*this, &GuiWidget::on_editSpeakerReceived));

			// -- Update current input language
			connection_input_language.disconnect() ;
			editor->signalLanguageChange().connect(sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &GuiWidget::onLanguageChanged), editor));

			// -- Actualize command line
			if (commandLine->getOffset()!=-1 &&  commandLine->getOffset()!=0)
			{
				editor->setCommandLineSignalOffset(commandLine->getOffset());
				commandLine->resetOffset() ;
			}

			// -- Annotation options connection
			editor->signalEditModeChanged().connect(sigc::mem_fun(*this, &GuiWidget::actualize_filemode)) ;
			editor->signalTagHiddenChanged().connect(sigc::mem_fun(*this, &GuiWidget::actualize_tagDisplay)) ;
			editor->signalHighlightChanged().connect(sigc::mem_fun(*this, &GuiWidget::actualize_highlight)) ;
			editor->signalSynchroChanged().connect(sigc::mem_fun(*this, &GuiWidget::actualize_synchronization_callback)) ;
			editor->signalDisplayChanged().connect(sigc::mem_fun(*this, &GuiWidget::actualize_display)) ;
		}
		else
		{
			Explorer_utils::print_trace("TranscriberAG --> <o> Editor loading failed, aborted.", 1) ;
			note.cancel_tab_annotation_editor() ;
		}
		Explorer_utils::print_trace("TranscriberAG --> <o> Editor setting done", 1) ;
	}

	return false ;
}



//**************************************************************************************
//************************************* DYNAMIC RELOAD *********************************
//**************************************************************************************

/*
 * mode:
 * 1: save
 * 2: reload
 * 3: save & reload
 */
bool GuiWidget::totalFileSavingOrReloading(int mode)
{
	// no opened files: nothing to do
	if (note.get_n_pages()==0)
		return false ;

	Glib::ustring txt = _("Some file preferences have been changed.") ;
	txt.append("\n   ") ;
	Glib::ustring button_yes_txt ;

	if (mode==1) {
		txt.append(_("The modifications will be saved when the files are saved.")) ;
		button_yes_txt = _("Save all now") ;
	}
	else if (mode==2) {
		txt.append(_("All opened files need to be reloaded for enabling these modifications.")) ;
		button_yes_txt = _("Reload all now") ;
	}
	else {
		txt.append(_("All opened files need to be saved and reloaded for enabling these modifications.")) ;
		button_yes_txt = _("Save and reload all now") ;
	}

	int rep = dlg::confirmWithButton(button_yes_txt, _("I'll do it later"), txt, this) ;

	if (rep)
	{
		std::vector<AnnotationEditor*> opened ;
		note.get_loaded_tabs(opened) ;
		std::vector<AnnotationEditor*>::const_iterator it ;
		for (it=opened.begin(); it!=opened.end(); it++)
		{
			AnnotationEditor* edit = *it ;
			saveOrReloadFile(edit->getFileName(), mode) ;
		}
		return true ;
	}
	else
		return false ;
}

/*
 * mode:
 * 1: save
 * 2: reload
 * 3: save & reload
 */
bool GuiWidget::prepareFileSavingOrReloading(Glib::ustring path, int mode)
{
	AnnotationEditor* edit = NULL ;
	bool all = false ;

	if (!path.empty())
		edit = (AnnotationEditor*) note.get_widget_by_path(path) ;
	else
		all = true ;

	if (!edit)
		return false ;

	Glib::ustring txt = _("Some file preferences have been changed.") ;
	txt.append("\n   ") ;
	Glib::ustring button_yes_txt ;

	if (mode==1) {
		txt.append(_("The modifications will be saved when the file is saved.")) ;
		button_yes_txt = _("Save now") ;
	}
	else if (mode==2) {
		txt.append(_("The file needs to be reloaded for enabling these modifications.")) ;
		button_yes_txt = _("Reload now") ;
	}
	else {
		txt.append(_("The file needs to be saved and reloaded for enabling these modifications.")) ;
		button_yes_txt = _("Save and reload now") ;
	}

	int rep = dlg::confirmWithButton(button_yes_txt, _("I'll do it later"), txt, this) ;

	if (rep)
	{
		saveOrReloadFile(path, mode) ;
		return true ;
	}
	else
		return false ;
}

/*
 * mode:
 * 1: save
 * 2: reload
 * 3: save & reload
 */
bool GuiWidget::saveOrReloadFile(Glib::ustring path, int mode)
{
	AnnotationEditor* edit = (AnnotationEditor*) note.get_widget_by_path(path) ;

	if (!edit)
		return false ;

	if (mode==1|| mode==3)
		edit->saveFile(path, true) ;

	if (mode==2|| mode==3)
	{
		edit->closeFile(false, false) ;
		open_file(path, "openall") ;
	}

	return true ;
}


void GuiWidget::on_notebook_reloadPage(int indice, Gtk::Widget* widget)
{
	if (!widget)
		return ;

	Glib::ustring text = _("Do you want to reload the file ?") ;
	bool rep = dlg::confirm(text, this) ;
	if (rep)
	{
		AnnotationEditor* edit = (AnnotationEditor*) widget ;
		bool ok = saveOrReloadFile(edit->getFileName(), 2) ;
		if (ok)
			note.disableReloadState(indice) ;
	}
}


//******************************************************************************
//									FILE  PROPERTIES
//******************************************************************************


void GuiWidget::show_file_properties()
{
	AnnotationEditor* current = (AnnotationEditor*)note.get_active_widget() ;

	if (!current)
		return ;

	Glib::ustring path = current->getFileName() ;
	if (filter->is_import_annotation_file(path) )
	{
		DialogFileProperties* d = new DialogFileProperties(*this, current, true, *parameters, true);
		d->loadGeoAndDisplay(true) ;
		//actualize file state
		bool needReload=d->getNeedReload() ;
		bool needSave=d->getNeedSave() ;
		if (needReload || needSave)
		{
			int mode ;
			if (needReload && needSave) {
				note.need_reload(false) ;
				mode=3 ;
			}
			else if (needReload) {
				note.need_reload(false) ;
				mode=2 ;
			}
			else if (needSave)
				mode= 1 ;

			prepareFileSavingOrReloading(path, mode) ;
		}
		delete(d) ;
	}
}


//**************************************************************************************
//************************************************************************** RESEARCH
//**************************************************************************************

void GuiWidget::set_research_in_file()
{
	//> TEXT
	SearchReplaceDialog* searchReplace = searchManager->get_dialog() ;
	searchReplace->set_transient_for(*this) ;

	//> TAG
	TSearchDialog* Tsearch = TsearchManager->get_dialog() ;
	Tsearch->set_transient_for(*this) ;
}

void GuiWidget::show_search_panel(Glib::ustring mode)
{
	//get current file
	AnnotationEditor* tmp = (AnnotationEditor*)note.get_active_widget() ;
	if (!tmp)
		return ;

	if (mode.compare("text")==0)
	{
		// if not active launch it
		if (!searchManager->is_active())
		{
			searchManager->init(tmp) ;
			searchManager->myShow() ;
		}
		// if active check selection
		else
			searchManager->check_selection() ;
	}
	else if (mode.compare("tag")==0)
	{
		// if not active launch it
		if (!TsearchManager->is_active())
		{
			TsearchManager->init(tmp) ;
			TsearchManager->myShow() ;
		}
		// if active check selection
//		else
//			TsearchManager->check_selection() ;
	}
}


//**************************************************************************************
//**************************************************************************** CLIPBOARD
//**************************************************************************************

void GuiWidget::set_clipboard()
{
	//> button for GUI
	Glib::ustring tmp = _("Show / Hide clipboard") ;
	Glib::ustring tmp2 = _("Alt + Shift + C") ;
	Glib::ustring tp = tmp + "\n" + tmp2 ;
	icop_clipboard.set_icon(ICO_CLIPBOARD, "", TAG_TOOLBAR_BUTTON_SIZE+5, tp) ;
	tool_box.pack_end(icop_clipboard, false, false) ;
	icop_clipboard.signal_clicked().connect( sigc::mem_fun(*this, &GuiWidget::show_clipboard) ) ;
	icop_clipboard.set_relief(Gtk::RELIEF_NONE);
	icop_clipboard.set_sensitive(false) ;
	clipboard->signal_response().connect( sigc::mem_fun(*this, &GuiWidget::on_close_clipboard));
	clipboard->signalDisplayClipboard().connect(sigc::mem_fun(*this, &GuiWidget::shortcut_show_clipboard)) ;

	icop_clipboard.set_focus_on_click(false) ;
}

void GuiWidget::show_clipboard()
{
	int x,y, xpos, ypos ;

	if (icop_clipboard.get_active())
	{
		AnnotationEditor* tmp = (AnnotationEditor*)note.get_active_widget() ;
		if (tmp!=NULL)
		{
			clipboard->switch_file(tmp, clipboard->is_active()) ;
			clipboard->loadGeoAndDisplay() ;
		}//end annot not null
		else
			icop_clipboard.set_active(false) ;
	} //end is active
	else
		clipboard->close(true) ;
}

void GuiWidget::on_close_clipboard(int response)
{
	if (response==-4)
		icop_clipboard.set_active(false) ;
}

void GuiWidget::shortcut_show_clipboard()
{
	icop_clipboard.activate() ;
}

//**************************************************************************************
//************************************************************ ANNOTATION EDITOR CHANGES
//**************************************************************************************


void GuiWidget::set_tool_buttons()
{
	tool_frame.set_shadow_type(Gtk::SHADOW_ETCHED_OUT) ;

	//> DISPLAY TOOL BUTTON
	Glib::ustring tooltip = _("Merged display") ;
	Glib::ustring arrow_tooltip = _("Change single editor") ;
	button_menu_display.set_icon(ICO_DISPLAY_UNIQUESCREEN , "" , TAG_TOOLBAR_BUTTON_SIZE+2, tooltip, arrow_tooltip);

	Gtk::RadioMenuItem* ite = button_menu_display.appendItem("DisplayMerged", _("Merged editor")) ;
	if (ite)
		ite->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::display_mode), -1)) ;

	ite = button_menu_display.appendItem("DisplayTrack1", _("Track 1 editor")) ;
	if (ite)
		ite->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::display_mode), 0)) ;

	ite = button_menu_display.appendItem("DisplayTrack2", _("Track 2 editor")) ;
	if (ite)
		ite->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::display_mode), 1)) ;

	button_menu_display.activate_menu() ;
	button_menu_display.signal_clicked().connect( sigc::mem_fun(*this, &GuiWidget::change_display));

	tool_box_2.pack_end(button_menu_display, false, false) ;
	button_menu_display.set_sensitive(false) ;

	Gtk::VSeparator* tool_sep_5 = Gtk::manage(new Gtk::VSeparator()) ;
	tool_box_2.pack_end(*tool_sep_5, false, true) ;


	//> HIGHLIGHT TOOL BUTTON
	tooltip = _("Highlight active") ;
	arrow_tooltip = _("Change highlight scope") ;
	button_menu_highlight.set_icon(ICO_HIGHLIGHT , "" , TAG_TOOLBAR_BUTTON_SIZE+2, tooltip, arrow_tooltip);
	Gtk::RadioMenuItem* item = button_menu_highlight.appendItem("SelectedTrack", _("Selected track")) ;
	if (item)
		item->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::highlight_mode), 3)) ;

	item = button_menu_highlight.appendItem("BothTracks", _("Both track")) ;
	if (item)
		item->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::highlight_mode), 2)) ;

	item = button_menu_highlight.appendItem("Track1",_("Track 1")) ;
	if (item)
		item->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::highlight_mode), 0)) ;

	item = button_menu_highlight.appendItem("Track2", _("Track 2")) ;
	if (item)
		item->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::highlight_mode), 1)) ;

	button_menu_highlight.activate_menu() ;
	button_menu_highlight.signal_clicked().connect( sigc::mem_fun(*this, &GuiWidget::change_highlight));

	tool_box_2.pack_end(button_menu_highlight, false, false) ;
	button_menu_highlight.set_sensitive(false) ;

	Gtk::VSeparator* tool_sep_3 = Gtk::manage(new Gtk::VSeparator()) ;
	tool_box_2.pack_end(*tool_sep_3, false, true) ;


	//> SYNCHRO BUTTONS
	icop_synchro_signal_w_text.set_icon(ICO_SYNC_SWT, "", TAG_TOOLBAR_BUTTON_SIZE+1, "") ;
	tool_box_2.pack_end(icop_synchro_signal_w_text, false, false) ;
	icop_synchro_signal_w_text.set_relief(Gtk::RELIEF_NONE);
	icop_synchro_signal_w_text.set_sensitive(false) ;
	icop_synchro_signal_w_text.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &GuiWidget::change_synchro), "swt"));
	icop_synchro_signal_w_text.set_focus_on_click(false) ;

	Gtk::VSeparator* tool_sep_2 = Gtk::manage(new Gtk::VSeparator()) ;
	tool_box_2.pack_end(*tool_sep_2, false, true) ;

	icop_synchro_text_w_signal.set_icon(ICO_SYNC_TWS, "", TAG_TOOLBAR_BUTTON_SIZE+4, "") ;
	tool_box_2.pack_end(icop_synchro_text_w_signal, false, false) ;
	icop_synchro_text_w_signal.set_relief(Gtk::RELIEF_NONE);
	icop_synchro_text_w_signal.set_sensitive(false) ;
	icop_synchro_text_w_signal.signal_clicked().connect( sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &GuiWidget::change_synchro), "tws" ));
	icop_synchro_text_w_signal.set_focus_on_click(false) ;

	Gtk::VSeparator* tool_sep_1 = Gtk::manage(new Gtk::VSeparator()) ;
	tool_box_2.pack_end(*tool_sep_1, false, true) ;

	//> DISPLAY TAG MENU BUTTON
	tooltip = _("Editor Tags are displayed") ; ;
	arrow_tooltip = _("Change hidden tag options") ;
	button_menu_tagDisplay.set_icon(ICO_TAG_DISPLAY, "", TAG_TOOLBAR_BUTTON_SIZE+2, tooltip, arrow_tooltip) ;
	Gtk::RadioMenuItem* itemm = button_menu_tagDisplay.appendItem("TagHideAll", _("Hide all qualifiers")) ;
	if (itemm)
		itemm->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::tagDisplay_mode), 1)) ;

	itemm = button_menu_tagDisplay.appendItem("TagHideUnit", _("Hide foreground elements")) ;
	if (itemm)
		itemm->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::tagDisplay_mode), 5)) ;

	itemm = button_menu_tagDisplay.appendItem("TagHideEvents", _("Hide events")) ;
	if (itemm)
		itemm->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::tagDisplay_mode), 2)) ;

	itemm = button_menu_tagDisplay.appendItem("TagHideEntities", _("Hide named entities")) ;
	if (itemm)
		itemm->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::tagDisplay_mode), 3)) ;

	itemm = button_menu_tagDisplay.appendItem("TagHideUnk", _("Hide unknown qualifiers")) ;
	if (itemm)
		itemm->signal_button_release_event().connect(sigc::bind<int>(sigc::mem_fun(this, &GuiWidget::tagDisplay_mode), 4)) ;

	button_menu_tagDisplay.activate_menu() ;
	tool_box_2.pack_end(button_menu_tagDisplay, false, false) ;
	button_menu_tagDisplay.set_sensitive(false) ;
	button_menu_tagDisplay.signal_clicked().connect( sigc::mem_fun(*this, &GuiWidget::change_tagdisplay));

	Gtk::VSeparator* sep = Gtk::manage(new Gtk::VSeparator()) ;
	tool_box_2.pack_end(*sep, false, true) ;

	//> FILE MODE
	icop_filemode.set_icon(ICO_FILEMODE_BROWSE, "", TAG_TOOLBAR_BUTTON_SIZE+2, "") ;
	tool_box_2.pack_end(icop_filemode, false, false) ;
	icop_filemode.set_sensitive(false) ;
	icop_filemode.set_relief(Gtk::RELIEF_NONE);
	icop_filemode.signal_clicked().connect( sigc::mem_fun(*this, &GuiWidget::change_filemode));
	icop_filemode.set_focus_on_click(false) ;

	//> ADDITIONAL BUTTONS IN TOOLBAR
	icom_speakerdicoGlobal.set_image(ICO_SPEAKER_DICO_GLOBAL, TAG_TOOLBAR_BUTTON_SIZE) ;
	toolB_speakerDicoGlobal.set_icon_widget(icom_speakerdicoGlobal) ;
	toolB_speakerDicoGlobal.set_label(_("Global speakers")) ;
	toolB_tooltip.set_tip(toolB_speakerDicoGlobal, _("Global Speakers Dictionary")) ;
	if (dicoManager)
		toolB_speakerDicoGlobal.signal_clicked().connect(sigc::mem_fun(*dicoManager, &DictionaryManager::showGlobalDictionary));

	icom_tree.set_image(ICO_TREE_EXPLORER, TAG_TOOLBAR_BUTTON_SIZE) ;
	toolB_tree.set_icon_widget(icom_tree) ;
	toolB_tree.set_label(_("File explorer")) ;
	toolB_tooltip.set_tip(toolB_tree, _("Show / Hide explorer")) ;
	toolB_tree.set_active(true) ;
	toolB_tree.signal_clicked().connect(sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &GuiWidget::show_hide_explorer_by), "button"));
}

//******************************* tag display **********************************

bool GuiWidget::tagDisplay_mode( GdkEventButton* event, int mode)
{
	AnnotationEditor* edit = (AnnotationEditor*)note.get_active_widget() ;

	if (edit)
		edit->setHiddenTagsMode(mode) ;

	return false ;
}

void GuiWidget::change_tagdisplay()
{
	Gtk::Widget* widget = note.get_active_widget() ;
	AnnotationEditor* edit = (AnnotationEditor*)widget ;

	if (!edit)
		return ;

	int res = edit->toggleHideTags(false) ;
	actualize_tagDisplay(res) ;
}

void GuiWidget::actualize_tagDisplay(int tagDisplayMode)
{
	Glib::ustring tooltip ;
	if(tagDisplayMode==-1)
	{
		tooltip = _("Editor Tags are displayed") ;
		button_menu_tagDisplay.change_icon(ICO_TAG_DISPLAY, "", 21, tooltip) ;
		// unlock editmode button
		icop_filemode.set_sensitive(true) ;
	}
	else
	{
		tooltip = _("Editor Tags are hidden") ;
		button_menu_tagDisplay.change_icon(ICO_TAG_DISPLAY_DISABLED, "", 21, tooltip) ;
		// lock editmode button
		icop_filemode.set_sensitive(false) ;
	}
	actualize_tagDisplay_options(tagDisplayMode) ;
}

void GuiWidget::actualize_tagDisplay_options(int tagDisplayMode)
{
	if (tagDisplayMode==-1)
		button_menu_tagDisplay.setSensitiveItem("", false) ;
	else
	{
		if (tagDisplayMode==1)
			button_menu_tagDisplay.activateItem("TagHideAll") ;
		else if (tagDisplayMode==5)
			button_menu_tagDisplay.activateItem("TagHideUnits") ;
		else if (tagDisplayMode==2)
			button_menu_tagDisplay.activateItem("TagHideEvents") ;
		else if (tagDisplayMode==3)
			button_menu_tagDisplay.activateItem("TagHideEntities") ;
		else if (tagDisplayMode==4)
			button_menu_tagDisplay.activateItem("TagHideUnk") ;
		button_menu_tagDisplay.setSensitiveItem("", true) ;
	}
}


//******************************* highlight ************************************

bool GuiWidget::highlight_mode( GdkEventButton* event, int mode )
{
	AnnotationEditor* edit = (AnnotationEditor*)note.get_active_widget() ;

	if (edit)
		edit->setHighlightMode(mode) ;

	return false ;
}

void GuiWidget::change_highlight()
{
	Gtk::Widget* widget = note.get_active_widget() ;
	AnnotationEditor* edit = (AnnotationEditor*)widget ;

	if (!edit)
		return ;

	int mode = edit->toggleHighlightMode(false) ;
	actualize_highlight(mode) ;
}

void GuiWidget::actualize_highlight(int light)
{
	Widget* widget = note.get_active_widget() ;
	AnnotationEditor* edit = (AnnotationEditor*)widget ;

	if (!edit)
		return ;

	bool isStereo = edit->isStereo() ;

	if (light==2 || light==3 || light==0  || light==1)
	{
		Glib::ustring tooltip = _("Highlight active") ;
		button_menu_highlight.change_icon(ICO_HIGHLIGHT, "", 21, tooltip) ;
	}
	else if (light==-1)
	{
		Glib::ustring tooltip = _("Highlight inactive") ;
		button_menu_highlight.change_icon(ICO_HIGHLIGHT_DISABLED, "", 21, tooltip) ;
	}
	actualize_highlight_options(isStereo, light) ;
}

void GuiWidget::actualize_highlight_options(bool isStereo, int mode)
{
	bool sensitive ;

	if (isStereo)
	{
		if (mode==3)
			button_menu_highlight.activateItem("SelectedTrack") ;
		else if (mode==2)
			button_menu_highlight.activateItem("BothTracks") ;
		else if (mode==1)
			button_menu_highlight.activateItem("Track2") ;
		else if (mode==0)
			button_menu_highlight.activateItem("Track1") ;
		sensitive = (mode!=-1) ;
		button_menu_highlight.setSensitiveItem("", sensitive) ;
	}
	else
	{
		sensitive = false ;
		button_menu_highlight.selectItem("SelectedTrack") ;
		button_menu_highlight.setSensitiveItem("", sensitive) ;
	}
}

int GuiWidget::map_highlight_value(Glib::ustring value)
{
	if (value=="none")
		return -1 ;
	else if (value=="selected")
		return 3 ;
	else if (value=="both")
		return 2 ;
	else if (value=="track2")
		return 1 ;
	else if (value=="track1")
		return 0 ;
}


//********************************* display ************************************


bool GuiWidget::display_mode( GdkEventButton* event, int mode)
{
	AnnotationEditor* edit = (AnnotationEditor*)note.get_active_widget() ;

	if (edit)
	{
		edit->disableViewThreads(true) ;
		edit->changeActiveViewMode(mode) ;
		edit->disableViewThreads(false) ;
	}

	return false ;
}

void GuiWidget::change_display()
{
	Gtk::Widget* widget = note.get_active_widget() ;
	AnnotationEditor* edit = (AnnotationEditor*)widget ;

	if (!edit)
		return ;

	int res = edit->toggleDisplay(false) ;
	actualize_display(res) ;
}

void GuiWidget::actualize_display(int display)
{
	if (display==-2) {
		Glib::ustring tooltip = _("Separated editor") ;
		button_menu_display.change_icon(ICO_DISPLAY_TWOSCREEN, "", 21, tooltip) ;
		button_menu_display.setSensitiveItem("", false) ;
	}
	else if (display==-1) {
		Glib::ustring tooltip = _("Unique editor") ;
		button_menu_display.change_icon(ICO_DISPLAY_UNIQUESCREEN, "", 21, tooltip) ;
		button_menu_display.setSensitiveItem("", true) ;
		button_menu_display.activateItem("DisplayMerged") ;
	}
	else if (display==1) {
		Glib::ustring tooltip = _("Unique editor") ;
		button_menu_display.change_icon(ICO_DISPLAY_UNIQUESCREEN, "", 21, tooltip) ;
		button_menu_display.setSensitiveItem("", true) ;
		button_menu_display.activateItem("DisplayTrack2") ;
	}
	else if (display==0) {
		Glib::ustring tooltip = _("Unique editor") ;
		button_menu_display.change_icon(ICO_DISPLAY_UNIQUESCREEN, "", 21, tooltip) ;
		button_menu_display.setSensitiveItem("", true) ;
		button_menu_display.activateItem("DisplayTrack1") ;
	}
}

void GuiWidget::actualize_display_options(bool isStereo, int mode)
{
	if (isStereo) {
		button_menu_display.set_sensitive(true) ;
		actualize_display(mode) ;
	}
	else {
		Glib::ustring tooltip = _("Unique editor") ;
		button_menu_display.change_icon(ICO_DISPLAY_UNIQUESCREEN, "", 21, tooltip) ;
		button_menu_display.setSensitiveItem("", false) ;
		button_menu_display.set_sensitive(false) ;
		button_menu_display.activateItem("DisplayMerged") ;
	}
}

//********************************* synchro ************************************

void GuiWidget::change_synchro(Glib::ustring mode)
{
	Gtk::Widget* widget = note.get_active_widget() ;
	AnnotationEditor* edit = (AnnotationEditor*)widget ;

	if (!edit)
		return ;

	string annot_mode ;
	if (mode =="swt")
		annot_mode = "synchro_signal_to_text" ;
	else if (mode =="tws")
		annot_mode = "synchro_text_to_signal" ;

	Glib::ustring res = edit->toggleSynchro(annot_mode, false) ;
	actualize_synchronization_callback(annot_mode, res) ;
}


void GuiWidget::actualize_synchronisation(Glib::ustring swt, Glib::ustring tws)
{
	Glib::ustring tooltip ;
	if (swt=="true") {
		tooltip = _("Signal is synchronised with text") ;
		icop_synchro_signal_w_text.change_icon(ICO_SYNC_SWT, "", 21, tooltip) ;
		icop_synchro_signal_w_text.set_relief(Gtk::RELIEF_NONE);
	}
	else if (swt=="false"){
		tooltip = _("Signal is not synchronised with text") ;
		icop_synchro_signal_w_text.change_icon(ICO_SYNC_SWT_DISABLED, "", 21, tooltip) ;
		icop_synchro_signal_w_text.set_relief(Gtk::RELIEF_NONE);
	}
	if (tws=="true") {
		tooltip = _("Text is synchronised with signal") ;
		icop_synchro_text_w_signal.change_icon(ICO_SYNC_TWS, "", 21, tooltip) ;
		icop_synchro_text_w_signal.set_relief(Gtk::RELIEF_NONE);
	}
	else if (tws=="false"){
		tooltip = _("Text is not synchronised with signal") ;
		icop_synchro_text_w_signal.change_icon(ICO_SYNC_TWS_DISABLED, "", 21, tooltip) ;
		icop_synchro_text_w_signal.set_relief(Gtk::RELIEF_NONE);
	}
}

void GuiWidget::actualize_synchronization_callback(string annot_mode, string value)
{
	if (annot_mode == "synchro_signal_to_text")
		actualize_synchronisation(value,"") ;
	else if (annot_mode == "synchro_text_to_signal")
		actualize_synchronisation("",value) ;
}

//******************************** filemode ************************************

void GuiWidget::change_filemode()
{
	Gtk::Widget* widget = note.get_active_widget() ;
	AnnotationEditor* edit = (AnnotationEditor*)widget ;

	if (!edit)
		return ;

	Glib::ustring res = edit->toggleFileMode(false) ;
	actualize_filemode(res) ;

	//> get synchronisation values
	if (edit->hasSignalView()) {
		Glib::ustring swt = edit->getOption("synchro_signal_to_text") ;
		Glib::ustring tws = edit->getOption("synchro_text_to_signal") ;
		actualize_synchronisation(swt, tws) ;
	}
	//> highlight
	int high = map_highlight_value(edit->getOption("highlight_current")) ;
	actualize_highlight(high) ;
}

void GuiWidget::actualize_filemode(Glib::ustring mode)
{
	Glib::ustring tooltip ;
	if(mode=="BrowseMode") {
		tooltip = _("File not editable") ;
		icop_filemode.change_icon(ICO_FILEMODE_BROWSE, "", 21, tooltip) ;
		icop_filemode.set_relief(Gtk::RELIEF_NONE);
	}
	else if (mode=="EditMode") {
		tooltip = _("File in edition mode") ;
		icop_filemode.change_icon(ICO_FILEMODE_EDIT, "", 21, tooltip) ;
		icop_filemode.set_relief(Gtk::RELIEF_NONE);
	}
}

//**************************************************************************************
//***************************************************************************** CALLBACK
//**************************************************************************************

bool GuiWidget::on_focus_out_event(GdkEventFocus* event)
{
	//hide tooltip
	if (treeManager)
		treeManager->hide_tooltips() ;

	//hide scim if activated
	AnnotationEditor* edit = (AnnotationEditor*) note.get_active_widget() ;
	Glib::ustring current_name = combo_language.get_active_text() ;
	if ( edit != NULL  && current_name.compare(IME_LANGUAGE)==0 )
		edit->externalIMEcontrol(false) ;
	return Gtk::Window::on_focus_out_event(event) ;
}

bool GuiWidget::on_focus_in_event(GdkEventFocus* event)
{
	//reset scim if current input needs it
	AnnotationEditor* edit = (AnnotationEditor*) note.get_active_widget() ;
	Glib::ustring current_name = combo_language.get_active_text() ;
	if ( edit != NULL  && current_name.compare(IME_LANGUAGE)==0 )
		edit->externalIMEcontrol(true) ;
	return Gtk::Window::on_focus_in_event(event) ;
}

bool GuiWidget::onNoteSignalDragDropReceived(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time)
{
	std::vector<Glib::ustring> target_list = context->get_targets() ;
	string tree_target = "" ;

	if (treeManager)
		tree_target = treeManager->get_dragDropTarget().get_target() ;

	std::vector<Glib::ustring>::iterator it ;
	for (it=target_list.begin() ; it!=target_list.end(); it++)
	{
		if ( (*it) == tree_target )
			dragDropOpenFileAction() ;
		else
			Log::err() << "dNd: undefined target." << std::endl ;
	}
}

void GuiWidget::onEditorSignalDragDropReceived(const Glib::RefPtr<Gdk::DragContext>& context, AGEditor* editor, const string& speakerId)
{
	std::vector<Glib::ustring> target_list = context->get_targets() ;

	string tree_target = "" ;
	string dictionary_target = "" ;

	if (treeManager)
		tree_target = treeManager->get_dragDropTarget().get_target() ;
	if (dicoManager)
		dictionary_target = dicoManager->getDragAndDropTarget().get_target() ;

	std::vector<Glib::ustring>::iterator it ;
	for (it=target_list.begin() ; it!=target_list.end(); it++)
	{
		if ( (*it) == tree_target )
			dragDropOpenFileAction() ;
		else if ( (*it) == dictionary_target )
			dragDropSpeakerAction(editor, speakerId) ;
		else
			Log::err() << "dNd: undefined target." << std::endl ;
	}
}

void GuiWidget::dragDropSpeakerAction(AGEditor* editor, const string& speakerId)
{
	if (!editor || speakerId.empty())
		return ;

	//TODO proceed
}

void GuiWidget::dragDropOpenFileAction()
{
	 TreeModel_Columns m ;
	 if (treeManager->is_dragdrop_initiated())
	 {
	 	Gtk::TreeIter src = treeManager->get_drag_src();
	 	int numberTree = (*src)[m.m_file_root] ;
	 	Explorer_tree* t = treeManager->get_tree(numberTree) ;
	 	Glib::ustring path = t->compute_path_from_node(src, true) ;
		treeManager->dragdropdone() ;
		open_file(path, "openall") ;
	 }
}

void GuiWidget::on_signalNewTreeFile(Glib::ustring path)
{
	treeManager->display_new_file(path) ;
}

bool GuiWidget::on_key_press_event(GdkEventKey* event)
{
	TreeModel_Columns m ;

	//Entry if a file is selected in tree
	if (event->keyval==GDK_Return && treeManager->has_selection() && treeManager->has_focus())
	{
		return keyPressEvent4Tree("entry") ;
	}
	//Delete if a file is selected in tree
	else if (event->keyval==GDK_Delete && treeManager->has_selection() && treeManager->has_focus())
	{
		return keyPressEvent4Tree("delete") ;
	}
	//F2 if a file is selected in tree
	else if (event->keyval==GDK_F2 && treeManager->has_selection() && treeManager->has_focus())
	{
		return keyPressEvent4Tree("f2") ;
	}
	//ctrl+f3 for toolbar search mode backward
	else if ( event->keyval==GDK_F3  && (event->state & GDK_SHIFT_MASK) && searchManager->is_active() )
	{
		searchManager->search_backward() ;
		return true ;
	}
	//F3 for toolbar search mode forward
	else if ( event->keyval==GDK_F3 && searchManager->is_active() )
	{
		searchManager->search_forward() ;
		return true ;
	}
	//ENTER for toolbar search mode
	else if (event->keyval==GDK_Return && searchManager->is_active() && searchManager->myHasFocus())
	{
		searchManager->search_forward() ;
		return true ;
	}
	else if ( event->keyval==GDK_F5  && (event->state & GDK_CONTROL_MASK) )
	{
		AnnotationEditor* edit = (AnnotationEditor*) note.get_active_widget() ;
		if ( edit && edit->isDebugMode() )
		{
			edit->reportCheckAnchors() ;
			return true ;
		}
	}
	//F5 = refresh
	else if (event->keyval==GDK_F5)
	{
		AnnotationEditor* edit = (AnnotationEditor*) note.get_active_widget() ;
		if (edit && edit->viewHasFocus())
			Glib::signal_timeout().connect(sigc::mem_fun(edit, &AnnotationEditor::refreshWhenIdle),20); // after idle to force cursor update		}
		return true ;
	}
	return Gtk::Window::on_key_press_event(event);
}

bool GuiWidget::keyPressEvent4Tree(const string& action)
{
	//> Open file
	if (action=="entry")
	{
		Explorer_tree* tree = treeManager->get_last_selected() ;
		Gtk::TreePath path = tree->get_child_selected() ;
		const Gtk::TreeViewColumn* column ;
		treeManager->activate_tree_row(path, column, tree) ;
		return true ;
	}
	//> Delete file
	else if (action=="delete")
	{
		TreeModel_Columns m ;
		Explorer_tree* tree = treeManager->get_last_selected() ;
		Gtk::TreePath path = tree->get_child_selected() ;
		Gtk::TreeIter iter = tree->get_refSortedModelTree()->get_iter(path) ;
		Glib::ustring path_s = tree->compute_path_from_node(iter, true) ;
		if (note.is_opened_file(path_s)!=-1)
			return true ;
		if ( (*iter)[m.m_file_sysType]==0 || (*iter)[m.m_file_sysType]==1 )
			tree->remove_file(&iter, this) ;
		return true ;
	}
	//> Rename file
	else if (action=="f2")
	{
		TreeModel_Columns m ;
		Explorer_tree* tree = treeManager->get_last_selected() ;
		Gtk::TreePath path = tree->get_child_selected() ;
		Gtk::TreeIter iter = tree->get_refSortedModelTree()->get_iter(path) ;
		Glib::ustring path_s = tree->compute_path_from_node(iter, true) ;
		if (note.is_opened_file(path_s)!=-1)
			return true ;
		if ( (*iter)[m.m_file_sysType]==0 || (*iter)[m.m_file_sysType]==1 )
			tree->rename_file(&iter, this) ;
		return true ;
	}
	return false ;
}

void GuiWidget::on_map()
{
	Gtk::Widget::on_map() ;
	Glib::signal_idle().connect(sigc::mem_fun(this, &GuiWidget::postDisplayProcessAfterIdle)) ; // after idle to force cursor update
}

bool GuiWidget::postDisplayProcessAfterIdle()
{
	m_signalStarted.emit() ;
	return false ;
}

void GuiWidget::on_change_type_annotation(AnnotationEditor* edit)
{
	menu.remove_ui("annotate") ;
	Glib::ustring ui_signal = edit->getUIInfo("annotate") ;
   	menu.add_ui(ui_signal,"annotate") ;
}

bool GuiWidget::on_delete_event(GdkEventAny* event)
{
	can_quit() ;
	return true ;
}



//**************************************************************************************
//***************************************************************************** CLOSING
//**************************************************************************************


void GuiWidget::can_quit()
{
	int close = Explorer_dialog::msg_dialog_question(_("Are you sure to quit Transcriber AG ?"), this, true, "") ;

	if (close==Gtk::RESPONSE_YES)
	{
		Explorer_utils::print_trace("TranscriberAG --> <.> Closing requested...", 1) ;
		InitialisedQuit = true ;
		if (note.get_n_pages()!=0)
		{
			//> ask for closing all
			saveAndclose(true) ;
		}
		else
		{
			//save values
			saveAndclose(false) ;
			//quit
			quit(true) ;
		}
	}
}

void GuiWidget::saveAndclose(bool still_opened_tab)
{
	savePos() ;
	Explorer_utils::print_trace("TranscriberAG --> <.> settings [OK] ", 1) ;
	menu.save(config->get_MENU_recent_path()) ;
	Explorer_utils::print_trace("TranscriberAG --> <.> recent [OK] ", 1) ;
	InitialisedQuit = dicoManager->close_all() ;
	Explorer_utils::print_trace("TranscriberAG --> <.> dictionaries [OK] ", 1) ;
	if (InitialisedQuit)
	{
		if (still_opened_tab)
			note.close_all(config->get_NOTEBOOK_opened_path()) ;
		else
			FileHelper::remove_from_filesystem(config->get_NOTEBOOK_opened_path()) ;
		Explorer_utils::print_trace("TranscriberAG --> <.> opened [OK] ", still_opened_tab, 1) ;
		clipboard->save() ;
		Explorer_utils::print_trace("TranscriberAG --> <.> clipboard [OK] ", 1) ;
		clipboard->close(true) ;
	}
}

void GuiWidget::quit(bool accepted)
{
	if (accepted && InitialisedQuit)
	{
		Explorer_utils::print_trace("TranscriberAG --> <.> Ready to exit main loop...", 1) ;
		Gtk::Main::quit() ;
		Explorer_utils::print_trace("TranscriberAG --> <.> ... exited", 1) ;
	}
	else if (!accepted)
		InitialisedQuit = false ;
}

//**************************************************************************************
//************************************************************************** DIALOG PART
//**************************************************************************************

void GuiWidget::show_filePropertyDialog(Gtk::TreeIter sorted_iter, Explorer_tree* tree)
{
	TreeModel_Columns m ;
	Glib::ustring path = tree->compute_path_from_node(sorted_iter, true) ;
	Glib::ustring info = Explorer_fileHelper::TAG_file_info(path) ;

	FilePropertyDialog property(path, info) ;

	#ifdef __APPLE__
	property.set_transient_for(*this);
	#endif
	property.run() ;
}

void GuiWidget::show_hide_explorer_by(Glib::ustring mode)
{
	if (mode.compare("button")==0 && !lock_tree_button)
	{
		if (toolB_tree.get_active()) {
			visible_tree = true ;
		}
		else {
			visible_tree = false ;
		}
		show_hide_explorer(visible_tree) ;
	}
	else if (mode.compare("menu")==0)
	{
		show_hide_explorer(!visible_tree) ;
		lock_tree_button = true ;
		toolB_tree.set_active(visible_tree) ;
		lock_tree_button = false ;
	}
}

void GuiWidget::show_hide_explorer(bool visible)
{
	if (!visible) {
		paned.get_child1()->hide_all() ;
		visible_tree = false ;
	}
	else {
		paned.get_child1()->show_all() ;
		visible_tree = true ;
	}
}

void GuiWidget::onRaiseCurrentDictionary()
{
	on_editSpeakerReceived("", false) ;
}

void GuiWidget::on_editSpeakerReceived(Glib::ustring id, bool modal)
{
	if (!dicoManager)
		return ;

	AGEditor* edit = (AGEditor*)note.get_active_widget() ;
	if (!edit)
	{
		Glib::ustring msg = _("No local dictionary available.") ;
		dlg::msg(msg, this) ;
		return ;
	}

	dicoManager->showLocalDictionary(edit, id, modal) ;
}

void GuiWidget::show_preferences_panel()
{
	PreferencesDialog* preferencesPanel = new PreferencesDialog(config, preferencesPanelOption) ;
	preferencesPanel->signal_hide().connect( sigc::bind<Gtk::Window*>(sigc::mem_fun(*this, &GuiWidget::onPreferencesPanelHide), preferencesPanel));
	preferencesPanel->signalReloadModifications().connect( sigc::mem_fun(*this, &GuiWidget::onPreferencesReloaded));
	preferencesPanel->set_parent(this);
	preferencesPanel->loadGeoAndDisplay(true) ;
	delete(preferencesPanel) ;
}

void GuiWidget::onPreferencesPanelHide(Gtk::Window* window)
{
	preferencesPanelOption = ((PreferencesDialog*)window)->getLastFrameCode() ;
}

/*
 *  Callback for preferences modification
 *  Static values will display symbol on opened tabs
 *  Dynamic values will call corresponding methods on different components
 */
void GuiWidget::onPreferencesReloaded(std::map<int, Glib::ustring> dynamic_values, int static_values)
{
	std::vector<AnnotationEditor*>::iterator it ;
	std::map<int, Glib::ustring>::iterator im ;

	std::vector<AnnotationEditor*> opened_tabs ;
	note.get_loaded_tabs(opened_tabs) ;

	if (dynamic_values.size()>0)
	{
		bool setOptionsDone = false ;
		bool colorResetAudio = false ;
		bool colorResetEditor = false ;
		//> set dynamic values and lauchn actions
		for (im=dynamic_values.begin() ; im!=dynamic_values.end(); im++)
		{
			bool b;
			int i ;
			Glib::ustring s ;
			switch (im->first)
			{
				case TAG_PREFERENCES_PARAM_AUTOSAVE :
					i = PreferencesFrame::get_formatted_int_dynamic_value(im->second) ;
					for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
						if (!setOptionsDone)
							(*it)->setOptions(*parameters, true) ;
						(*it)->reset_autosave(i) ;
					}
					setOptionsDone = true ;
					break ;
				case TAG_PREFERENCES_PARAM_INACTIVITY :
					i = PreferencesFrame::get_formatted_int_dynamic_value(im->second) ;
					for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
						if (!setOptionsDone)
							(*it)->setOptions(*parameters, true) ;
						(*it)->reset_activity(i) ;
					}
					setOptionsDone = true ;
					break ;
				case TAG_PREFERENCES_PARAM_AUTOSETLANGUAGE :
					b = PreferencesFrame::get_formatted_bool_dynamic_value(im->second) ;
					for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
						if (!setOptionsDone)
							(*it)->setOptions(*parameters, true) ;
						(*it)->reset_autosetLanguage(b) ;
					}
					setOptionsDone = true ;
					break ;
/* SPELL */
//				case TAG_PREFERENCES_PARAM_SPELLER_ALLOW :
//				case TAG_PREFERENCES_PARAM_SPELLER_IGNORE :
//				case TAG_PREFERENCES_PARAM_SPELLER_PATH :
//					for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
//						if (!setOptionsDone)
//							(*it)->setOptions(*parameters, true) ;
//						(*it)->reset_speller() ;
//					}
//					setOptionsDone = true ;
//					break ;
				case TAG_PREFERENCES_PARAM_STOPONCLICK :
					b = PreferencesFrame::get_formatted_bool_dynamic_value(im->second) ;
					for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
						if (!setOptionsDone)
							(*it)->setOptions(*parameters, true) ;
						(*it)->reset_stopOnClick(b) ;
					}
					setOptionsDone = true ;
					break ;
				case TAG_PREFERENCES_PARAM_TIMESCALE :
					b = PreferencesFrame::get_formatted_bool_dynamic_value(im->second) ;
					for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
						if (!setOptionsDone)
							(*it)->setOptions(*parameters, true) ;
						(*it)->reset_timeScale(b) ;
					}
					setOptionsDone = true ;
					break ;
				case TAG_PREFERENCES_PARAM_EDITOR_ENTITYBG :
					b = PreferencesFrame::get_formatted_bool_dynamic_value(im->second) ;
					for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
						if (!setOptionsDone)
							(*it)->setOptions(*parameters, true) ;
						(*it)->reset_entityTags_bg(b) ;
					}
					setOptionsDone = true ;
					break ;
				case TAG_PREFERENCES_PARAM_TOOLBAR :
					s = im->second ;
					change_toolbar_style(NULL, s) ;
					break ;
				case TAG_PREFERENCES_PARAM_GENERAL_TOOLBARSHOW :
					s = im->second ;
					show_hide_toolbar(s) ;
					break ;
				case TAG_PREFERENCES_PARAM_GENERAL_STATUSBARSHOW :
					s = im->second ;
					show_hide_statusbar(s) ;
					break ;
				case TAG_PREFERENCES_PARAM_EDITOR_HIGHLIGHT:
				case TAG_PREFERENCES_PARAM_EDITOR_ALLOWBROWSEONTAGS:
				case TAG_PREFERENCES_PARAM_EDITOR_SUPPRESSSEGMENT :
					for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
						if (!setOptionsDone)
							(*it)->setOptions(*parameters, true) ;
					}
					setOptionsDone = true ;
					break ;
				case TAG_PREFERENCES_PARAM_EDITOR_LABELFONT :
					s = im->second ;
					for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
						if (!setOptionsDone)
							(*it)->setOptions(*parameters, true) ;
						(*it)->setFontStyle(s, "label") ;
					}
					setOptionsDone = true ;
					break ;
				case TAG_PREFERENCES_PARAM_EDITOR_TEXTFONT :
						s = im->second ;
						for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
							if (!setOptionsDone)
								(*it)->setOptions(*parameters, true) ;
							(*it)->setFontStyle(s, "text") ;
						}
						setOptionsDone = true ;
						break ;
				case TAG_PREFERENCES_PARAM_COLORS_EDITOR:
						for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
							if (!setOptionsDone)
								(*it)->setOptions(*parameters, true) ;
							if (!colorResetEditor)
								(*it)->setEditorColors() ;
						}
						colorResetEditor = true ;
						setOptionsDone = true ;
						break ;
				case TAG_PREFERENCES_PARAM_COLORS_AUDIO:
						for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
							if (!setOptionsDone)
								(*it)->setOptions(*parameters, true) ;
							if (!colorResetAudio)
								(*it)->setAudioColors() ;
						}
						setOptionsDone = true ;
						colorResetAudio = true ;
						break ;
				case TAG_PREFERENCES_PARAM_EDITOR_TOOLTIP:
						b = PreferencesFrame::get_formatted_bool_dynamic_value(im->second) ;
						for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
							(*it)->setUseTooltip(b) ;
				}
						break ;
				case TAG_PREFERENCES_PARAM_FBROWSER_RESOLUTION :
						i = PreferencesFrame::get_formatted_int_dynamic_value(im->second) ;
						for (it=opened_tabs.begin(); it!=opened_tabs.end(); it++) {
							(*it)->setFrameBrowserResolution(i) ;
				}
			} // end cases
		} // end all dynamic parameters
	}

	//> if static values warning, display it on opened tabs
	if ( (static_values==1 || static_values==2) && opened_tabs.size()>0 ) {
		note.need_reload(true) ;
		totalFileSavingOrReloading(2) ;
	}
}


void GuiWidget::show_hide_toolbar(Glib::ustring value)
{
	if (value.compare("toggle")==0)
	{
		if (tool_box.is_visible())
			tool_box.hide() ;
		else
			tool_box.show() ;
	}
	else if (value.compare("true")==0)
		tool_box.show() ;
	else if (value.compare("false")==0)
		tool_box.hide() ;
}

void GuiWidget::show_hide_statusbar(Glib::ustring value)
{
	if (value.compare("toggle")==0)
	{
		if (status.is_visible())
			status.hide() ;
		else
			status.show() ;
	}
	else if (value.compare("true")==0)
		status.show() ;
	else if (value.compare("false")==0)
		status.hide() ;
}


void GuiWidget::show_documentation()
{
	Glib::ustring doc = config->get_UserDoc_path() ;
	Glib::ustring default_browser = config->get_GUI_defaultBrowser() ;
	Doc::LaunchInstalledBrowser(doc, this, default_browser) ;
}

void GuiWidget::about()
{
	//> Window appearance
	Glib::ustring icon = ICO_TRANSCRIBER ;
	Glib::ustring image = ICO_TRANSCRIBER_GEN ;
	Glib::ustring title = _("About") ;

	//> Text
	Glib::ustring space = "   " ;
	Glib::ustring line = "\n" ;
	Glib::ustring text = line ;

	text = text + space + TRANSAG_DISPLAY_NAME ;
	text.append(line);
	Glib::ustring tmp = space + _("Annotation Graphs Transcription Tool") ;
	text.append(tmp) ;

	text.append(line);
	text.append(line);
	Glib::ustring version = space + _("Version: ") + space + TRANSAG_VERSION_NO ;
	Glib::ustring date = space + _("Date: ") + space + getVersionStamp() ;

	text.append(version) ;
	text.append(line);
	text.append(date) ;
	text.append(line);
	text.append(line);
	tmp = space + _("Developed by Bertin Technologies, TIC activity, 2011.") + space + space ;
	text.append(tmp) ;
	text.append(line);
	text.append(line);

	Glib::ustring under = space + "This software uses libraries from the FFmpeg project under the LGPLv2.1" + space ;

	AboutDialog dialog(image, title, icon, text, under) ;

	#ifdef __APPLE__
	dialog.set_transient_for(*this);
	#endif
	dialog.run() ;
}


//**************************************************************************************
//										GEOMETRY INTERFACE
//**************************************************************************************

void GuiWidget::getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	get_position(pos_x, pos_y) ;
	panel = paned.get_position() ;
	get_size(size_xx, size_yy) ;
}

void GuiWidget::setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel)
{
	if (pos_x>0 && pos_y>0)
		move(pos_x, pos_y) ;
	paned.set_position(panel) ;
	if (size_xx>0 && size_yy>0)
		resize(size_xx, size_yy) ;
}

Glib::ustring GuiWidget::getWindowTagType()
{
	return SETTINGS_GENERAL_NAME ;
}

int GuiWidget::loadGeoAndDisplay(bool rundlg) {}
void GuiWidget::saveGeoAndHide() {}
void GuiWidget::getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) {}

}//NAMESPACE
