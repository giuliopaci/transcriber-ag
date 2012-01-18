/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef NOTEBOOK_MOD_H_
#define NOTEBOOK_MOD_H_

#include "DictionaryManager.h"
#include "Common/icons/Icons.h"
#include "Explorer_menu.h"
#include "Common/icons/IcoPackButton.h"
#include "Common/icons/IcoPackImage.h"
#include "TreeManager.h"
#include "Configuration.h"
#include "Common/icons/IcoPackImageButton.h"

#define TAG_NOTEBOOK_TAB_SIZE 25
#define TAG_NOTEBOOK_TAB_LABEL_MAXSIZE 19
#define TAG_NOTEBOOK_TAB_LABEL_ELLIPSE "..."


namespace tag {

/**
 * @class 		NoteBook_mod
 * @ingroup		GUI
 *
 *	Wrapper for TranscriberAG specific notebook
 *	Notebook containing AnnotationEditors, giving closing and expanding features.
 *
 *	Each notebook page contains a wrap box, that is a simple container (Box).\n
 *	Wrap boxes were used to pack a wait panel when opening an annotation editor. After complete loading,
 *	on a signal ready reception, the wait panel was removed and the annotation editor was packed. This
 *	enabled to keep the same page even if we removed and re-packed widget inside.\n
 *	This mechanism is deprecated, only wrap box are still present but Annotation Editor are directly packed
 *	into the wrap box.
 *	TODO in next version: skip wrap boxes
 */
class NoteBook_mod : public Gtk::Notebook
{
	public:
		NoteBook_mod();
		virtual ~NoteBook_mod();

		/**
		 * Set TranscriberAG configuration
		 * @param config 	Pointer on configuration object
		 */
		void set_configuration(Configuration* config) { configuration = config ; }

		/**
		 * Set the TreeManager used for explorer tree in toplevel GUI
		 * @param pTreeManager		Pointer on the TreeManager used in toplevel GUI
		 */
		void set_treeManager(TreeManager* pTreeManager)
		{
			treeManager =pTreeManager ;
			if (treeManager)
				drag_dest_set(treeManager->get_dragDropTargetList(),Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
		}

		/**
		 * Set the DictionaryManager used in toplevel GUI
		 * @param pDicoManager		Pointer on the DictionaryManager used in toplevel GUI
		 */
		void set_dicoManager(DictionaryManager* pDicoManager) { dicoManager = pDicoManager ;}

		/**
		 * Set the toplevel GUI
		 * @param pWin		Pointer on the toplevel window
		 */
		void set_window(Gtk::Window* pWin) { window = pWin ;}

		/**
		 * Get all widgets opened in the Notebook
		 * @return		a container mapping all widgets with theirs internals stamp
		 */
		std::map<int, Gtk::Widget*>* get_widgets() ;

		/**
		 * Add a new widget in the notebook
		 * (wrapper for append method of Gtk::Notebook)
		 * @param 	myWidget 		Widget to add
		 * @param 	label 			Label of file
		 * @param 	savedState 		True for adding the file in savedState, false for adding it in modified state
		 * @return	Internal stamp numbering the tabs
		 */
		int append_mod(Gtk::Widget* myWidget, const std::string& label, bool savedState) ;

		/**
		 * Indicates if an editor with specific path is already opened in the notebook
		 * @param path		path whose existence must be checked
		 * @return			notebook widget stamp if opened (integer > 0), -1 if widget doesn't exist
		 */
		int is_opened_file(const std::string& path) ;

		/**
		 * Return the widget contained in the nth page
		 * @param n		number of the page we want to get the widget
		 * @return		Pointer on the widget contained in the nth page, NULL if page couldn't be found
		 */
		Gtk::Widget* get_page_widget(int n) ;

		/**
		 * Get the widget contained in the notebook activated page
		 * @return		Pointer on the widget contained in the activated page, NULL if page couldn't be found
		 */
		Gtk::Widget* get_active_widget() ;

		/**
		 * Return the file path of the widget contained in the numth page
		 * @param num	number of the page we want to access
		 * @return		the file path corresponding to
		 */
		std::string get_path_from_page_num(int num) ;

		/**
		 * Returns the widget corresponding to the given path.
		 * @param path		File path
		 * @return			The corresponding widget, or NULL if none could be found
		 */
		Gtk::Widget* get_widget_by_path(const std::string& path) ;

		/**
		 * Add a new empty annotation editor in the notebook
		 * @param window		Window top level
		 * @param file			File name
		 * @param savedState	True for opening in saved state, false for updated state
		 * @return				pointer on AnnotationEditor newly created
		 */
		Gtk::Widget* tab_annotation_editor(Gtk::Window* window, const std::string& file, bool savedState) ;

		/**
		 * Cancel last new tab_annotation_editor created
		 */
		void cancel_tab_annotation_editor() ;

		/**
		 * Indicates if more than one tab has been opened yet
		 * @return		true if at least 2 pages has been already opened
		 * @note		usefull the number of pages isn't yet activated when we need the information
		 */
		bool is_first_tab_passed() {return first_tab_passed;}

		/**
		 * Close all opened files and save their opened status in configuration file
		 * @param path			path of configuration file where opened status is saved
		 * @param only_save		true for saving in file without closing pages, false for closing and saving
		 */
		void close_all(const std::string& path, bool only_save=false) ;

		/**
		 * Get the previous activated page
		 * @return		number of the previous activated page
		 */
		int get_old_page() { return old_page ; }

		/**
		 * Set the previous activated page
		 * @param num
		 */
		void set_old_page(int num) { old_page=num ; }

		/**
		 *  Switch to next or previous page
		 * @param mode	"next" or "previous"
		 */
		void switch_tab(const std::string& mode) ;

		/**
		 * Set active the page containing the given widget
		 * @param w		Pointer on widget packed in notebook
		 */
		void setActiveWidget(Gtk::Widget* w) ;

		/**
		 * Set active the page containing the given widget
		 * @param pageNum		Notebook page number
		 */
		void setActiveWidget(int pageNum) ;

		/**
		 * Close the active page of notebook
		 */
		void close_current_page() ;

		/**
		 * Indicate if the page of number num is fully loaded
		 * @param num	number of the page whose status has to be checked
		 * @return		true if the page is fully loaded, false otherwise
		 */
		bool is_loaded_page(int num) ;

		/**
		 * Display a warning icon in the page header for indicating the page
		 * must be closed for the preferences previously applied to be effective
		 * @param all	If set to true, action applied for all opened tab.
		 * 				Otherwise, applied only on the current tab.
		 */
		void need_reload(bool all) ;

		/**
		 * Get all the AnnotationEditors opened in the notebook
		 * @param res	vector of Pointers on all opened AnnotationEditors
		 * @return		number of opened pages
		 */
		int get_loaded_tabs(std::vector<AnnotationEditor*>& res) ;

		/**
		 * Set or unset the large mode for page notebook
		 * (eclipse like function)
		 */
		void toggle_large_mode() ;

		/**
		 * Actualizes the page for refreshing state
		 */
		void setPageAsRefreshing(int indice) ;

		/**
		 * Disables the reload indicator displayed when a modification on preferences
		 * is waiting the file to be reloaded for being applied.
		 * @param indice		Internal notebook stamp corresponding to the notebook tab
		 * 						on which the state has to be disabled.\n
		 * @attention			The indice must have been received with the reload signal.\n
		 * 						Don't use otherwise.
		 */
		void disableReloadState(int indice) ;

		/**
		 * Set the drag and drop target list
		 * @param list		List of drag and drop targets
		 */
		void setDragAndDropTarget(std::vector<Gtk::TargetEntry> list) ;

		/**
		 * Signal sent when needing to remove action group of a file\n
		 * Glib::RefPtr<Gtk::ActionGroup> parameter:	action group to be removed
		 * @return	corresponding signal
		 */
		sigc::signal< void,Glib::RefPtr<Gtk::ActionGroup> >& signalRemoveActionGroup() { return m_signalRemoveActionGroup; }

		/**
		 * Signal sent when adding a file in the notebook\n
		 * std::string parameter:		path of file being added
		 * @return	corresponding signal
		 */
		sigc::signal<void,std::string>& signalAddRecentFile() { return m_signalAddRecentFile; }

		/**
		 * Signal sent when removing the last tab or adding the first tab\n
		 * std::string parameter: 	"first" for first opening, "last" for last closing
		 * @return	corresponding signal
		 */
		sigc::signal<void,std::string, Gtk::Widget*>& signalFirstLastTab() { return m_signalFirstLastTab; }

		/**
		 * Signal sent when last tab is closed to remove file ui\n
		 * std::string parameter: 	"first" for first opening, "last" for last closing
		 * @return	corresponding signal
		 */
		sigc::signal<void, std::string>& signalActualiseUI() { return m_signalActualiseUI ; }

		/**
		 * Signal sent when a new file has been created\n
		 * std::string parameter: 	path of new file
		 * @return	corresponding signal
		 */
		sigc::signal<void,std::string>& signalNewTreeFile() { return m_signalNewTreeFile; }

		/**
		 * Signal sent when closing application: warn if the last page has been closed\n
		 * bool parameter: 	false is closing has been cancelled, true if successfull
		 * @return	corresponding signal
		 */
		sigc::signal<void,bool>& signalClosedLastTab() { return m_signalClosedLastTab; }

		/**
		 * Signal sent for forcing the change of the page with all mechanisms that it needs\n
		 * guint paramater: number of the page that must be set to active
		 * @return	corresponding signal
		 */
		sigc::signal<void, guint>& signalPersonalSwitch() { return m_signalPersonalSwitch; }

		/**
		 * Signal sent a double click is handled on the page header
		 * (eclipse like function : enlarge or not the page)\n
		 * bool paramater: 	new state of page (true for enlarged mode)
		 * @return	corresponding signal
		 */
		sigc::signal<void, bool>& signalHeaderDoubleClick() { return m_signalHeaderDoubleClick ; }

		/**
		*  Signal sending a status info\n
		*  <b>std::string parameter:</b> 	message to display\n
		*/
		sigc::signal<void, std::string>& signalStatusBar() { return m_signalStatusBar; }

		/**
		*  Signal sent when a page reload is required.\n
		*  <b>int parameter:</b> 			Internal stamp that will be necessary to disable the reload status\n
		*  <b>Gtk::Widget* parameter:</b> 	Pointer in the corresponding embedded widget\n
		*/
		sigc::signal<void, int, Gtk::Widget*>& signalReloadPage() { return m_signalReloadPage; }

	private:
		bool mode_large ;

		Configuration* configuration ;
		TreeManager* treeManager ;
		DictionaryManager* dicoManager ;
		Gtk::Window* window ;

		//*********************** INTERNAL CLASSES *****************************

		// header used to display a close button
		class NoteBook_header : public Gtk::HBox
		{
			public:
				virtual ~NoteBook_header();
				NoteBook_header(int p_indice, const std::string& lab, Configuration* configuration, NoteBook_mod* p_parent);

				NoteBook_mod* parent ;

				Gtk::Tooltips tooltip ;

				/* waiting indicator */
				IcoPackImage left_image ;

				/* reload indicator */
				IcoPackImageButton reload_image ;

				/* tab name */
				Gtk::EventBox label_box ;
				Gtk::Label label ;				/* name */
				Gtk::Label label_star ;			/* star for updated state */

				/* close button */
				IcoPackImageButton button ;

				Gtk::Label expanding_blank ;

				/** indicates if display is OK **/
				bool loaded ;

				/** indicates if we were refreshing child display **/
				bool refreshing ;

				/** stamp used to link widgets, wrap box, header tab **/
				int indice ;

				/** used to display specific icon and msg when file need to be reloaded **/
				bool preferences_modified  ;
				sigc::connection reload_connection ;

				/** status **/
				bool updated ;

				sigc::signal<void> m_signalHeaderDoubleClick ;
				sigc::signal<void>& signalHeaderDoubleClick() { return m_signalHeaderDoubleClick ; }
				sigc::signal<void,int> m_signalHeaderWheelClick ;
				sigc::signal<void,int>& signalHeaderWheelClick() { return m_signalHeaderWheelClick ; }

				void setProgressImage(Configuration* configuration) ;
				void setReloadImage() ;
				void setReloadState() ;
				void setReloadState(bool on) ;
				bool my_on_button_press_event(GdkEventButton* event) ;

				Glib::Timer loading_timer ;
		} ;

		/* used to keep the last file opened before switching */
		/* JUST SET TO OLD VALUE DURING SWITCHING, AFTER SET TO CURRENT */
		int old_page ;
		/* counter used to give a unique indice for each tab header */
		int current_n_header ;
		/* flag that indicates the first page book has been opened or not */
		bool first_tab_passed ;

		/** Delete header with indice corresponding to parameter **/
		void delete_header(int stamp) ;
		/** Delete widget with indice corresponding to parameter **/
		void delete_widgets(int indice) ;
		/** Delete widget with indice corresponding to parameter **/
		void delete_wrapbox(int indice) ;

		/* Indicate state of file with indice corresponding to "indice"
		 *  state: false: saved
		 * 		   true: modified
		 */
		void set_waiting_state(NoteBook_header* header) ;
		void set_file_state(int indice, bool state) ;

		/*
		 *  change label if file has been saved or not
		 *  mode: saved (0) or updated(1)
		 */
		int change_label_state(Gtk::Widget* wrapbox, bool mode) ;
		/*
		 * set "name" as new label of "widget"
		 */
		void change_label_name(Gtk::Widget* wrapbox, std::string name, bool actualise_icon) ;

		/*
		 * Return the object packed in the wrapbox
		 */
		Gtk::Widget* get_widget_from_wrapBox(Gtk::VBox* box) ;

		/* Safe access */
		Gtk::Widget* getWidgetByIndice(int indice) ;
		NoteBook_header* getHeaderByIndice(int indice) ;
		Gtk::Widget* getWrapboxByIndice(int indice) ;

		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CALLBACK
		/*
		 * on close button pressed signal
		 */
		void on_close_button(Gtk::Widget* widget) ;
		/*
		 * on can close signal received from annotation editor
		 */
		void on_can_close_received(Gtk::Widget* wrapbox, int indice) ;
		/*
		 * On file saved signal received from annotation editor
		 * when a file is saved (new file or not)
		 */
		void on_file_saved(const std::string& path, bool newfile, bool force_rename, bool force_expand, Gtk::Widget* wrapBox, int indice) ;
		/*
		 * On file updated signal received from annotation editor
		 * when a file changes of state (state modified or saved)
		 */
		void on_file_updated( bool update, Gtk::Widget* wrapbox, int indice) ;
		/*
		 * Action done when object in tab has finished to be loaded
		 */
		void on_loading_done(Gtk::Widget* widget, Gtk::VBox* box, const std::string& path, bool saveState, int indice) ;
		/*
		 * Action done when a closing action has been required, then cancelled
		 */
		void on_close_cancelled() ;

		bool on_close_button_wrap(GdkEventButton* event, Gtk::Widget* widget) ;

		bool on_reload_required(GdkEventButton* event, int indice) ;

		/*
		 * mapping <indice header, objects>
		 */
		std::map<int, Gtk::Widget*> widgets_map ;
		/*
		 * Mapping <indice header, header object>
		 * Used to destruct header stamped after closing corresponding tab
		 */
		std::map<int, NoteBook_header*> head_map ;

		/*
		 * Mapping <indice header, vbox>
		 * Used to destruct header stamped after closing corresponding tab
		 */
		std::map<int, Gtk::VBox*> wrapbox_map ;

		//> signal used to make the menu remove the current file action group
		sigc::signal< void,Glib::RefPtr<Gtk::ActionGroup> > m_signalRemoveActionGroup ;
		//> signal used to make the menu add file in recent file
		sigc::signal<void,std::string> m_signalAddRecentFile ;
		//> signal used to tell GUI of closing last tab or opening first tab
		sigc::signal<void,std::string, Gtk::Widget*> m_signalFirstLastTab ;
		//> signal used to tell GUI Menu to remove the current ui
		sigc::signal<void, std::string> m_signalActualiseUI ;
		//> signal used to indicates tree to expand where file is new
		sigc::signal<void,std::string> m_signalNewTreeFile ;
		//> signal sent to IHM eah time a tab is closed
		sigc::signal<void,bool> m_signalClosedLastTab ;
		//> signal sent to IHM eah time a tab is closed
		sigc::signal<void, guint> m_signalPersonalSwitch ;
		//> signal sent when double click is proceded on header
		sigc::signal<void, bool> m_signalHeaderDoubleClick ;
		//> signal sent for updating status bar info
		sigc::signal<void, std::string> m_signalStatusBar ;
		//> signal sent for asking a page reload
		sigc::signal<void, int, Gtk::Widget*> m_signalReloadPage ;

		std::string ellipsize_label(const std::string& name) ;

		void on_signalHeaderDoubleClick()  ;
		void on_signalHeaderWheelClick(int indice) ;
		bool my_on_button_press_event(GdkEventButton* event) ;
	};

} //namespace


#endif
