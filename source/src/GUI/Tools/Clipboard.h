/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef CLIPBOARD_H_
#define CLIPBOARD_H_

#include <gtkmm.h>
#include "Common/globals.h"
#include "AnnotationEditor/AnnotationEditor.h"
#include "AnnotationEditor/AnnotationView.h"
#include "Common/icons/IcoPackButton.h"
#include "Common/widgets/Settings.h"
#include "Common/widgets/GeoWindow.h"
#include "Configuration.h"

namespace tag {

/**
 * @class 		Clipboard
 * @ingroup		GUI
 *
 * Clipboard editor, only available when a transcription file is opened in the
 * AnnotationEditor.
 * This widget enables the user get a list of words that he can import in the
 * AnnotationEditor (the list is saved in a file and reloaded each time the
 * application starts).
 * It is always on top and doesn't keep focus in order to enable the user to
 * import or save words without needing to give focus back to the AnnotationEditor.
 */

class Clipboard : public Gtk::Dialog, public GeoWindow
{
	public:

		/**
		 * Constructor
		 * @param config	Pointer on Configuration object
		 * @param win		Toplevel window
		 * @return
		 */
		Clipboard(Configuration* config, Gtk::Window* win);
		virtual ~Clipboard();

	  /**
	   * Set the activated status of clipboard
	   * Activate or not the clipboard actions and accelerators
	   * @param value activate or not the clipboard
	   */
		void activate(bool value);

	  /**
	   * Clipboard is active if an editor is active
	   * @return true if clipboard is active
	   */
		bool is_active() {return active;}

	  /**
	   * Link the clipboard with an annotation editor
	   * @param editor TextBuffer from the file
	   */
		void set_editor(AnnotationEditor* editor) ;

	  /**
	   * Disconnect clipboard from former AnnotationEditor and connect it to the
	   * one passed as parameter
	   * @param editor TextBuffer from the file to be connected
	   * @param switching mostly true except for first calling
	   */
		void switch_file(AnnotationEditor* editor, bool switching) ;

	  /**
	   * Return action group of the clipboard
	   * @return pointer on the clipboard action group
	   */
		Glib::RefPtr<Gtk::ActionGroup> get_actionGroup() { return actionGr ; }

	  /**
	   * Deactivate clipboard
	   * Deactive actions, accelerators and signal connections
	   * @param needhide true for hidding clipboard
	   */
		void close(bool needhide) ;

	  /**
	   * Write clipboard in file defined in configuration
	   * @return true if done, false otherwise
	   */
		bool save() ;

	  /**
	   * Load clipboard from file defined in configuration
	   * @param path path of clipboard file
	   */
		void load(Glib::ustring path) ;

	  /**
	   * Set alphabetic order of clipboard list
	   * @param reverse true for reversing order
	   */
		void set_list_order(bool reverse) ;

	  /**
	   * Signal used to warn the top level GUI the clipboard must be activated
	   */
		sigc::signal<void>& signalDisplayClipboard() { return m_signalDisplayClipboard; }

		/*** Geometry interface ***/
		virtual void saveGeoAndHide() ;
		virtual int loadGeoAndDisplay(bool rundlg=false) ;

	private:
		Gtk::Tooltips tooltip ;
		Gtk::Window* window ;

		Configuration* config ;
		bool reverse_sort ;
		bool can_focus ;

		/* Widgets */
		Gtk::ScrolledWindow scroll ;
		Gtk::Table* table_button ;
		Gtk::Alignment align_button ;
		Gtk::HSeparator sep ;

		Gtk::HBox hbox1 ;
		Gtk::HBox hbox2 ;
		Gtk::Button* button_up ;
		Gtk::Button* button_down ;
		IcoPackButton* button_export ;
		IcoPackButton* button_import ;
		Gtk::Button* button_clear ;
		Gtk::Button* button_suppress ;
		IcoPackButton* button_sort_Asc ;
		IcoPackButton* button_sort_Desc ;

		/* Internal view and and buffer */
		Gtk::TextView* tview ;
		Glib::RefPtr<Gtk::TextBuffer> buffer ;

		/* external buffer on which clipboard is connected */
		AnnotationEditor* external_editor ;
		Glib::RefPtr<AnnotationBuffer> get_external_buffer() {
			return external_editor->getActiveView()->getBuffer() ;
		}
		AnnotationView* get_external_view() {
			return external_editor->getActiveView() ;
		}
		/* stock signal connection to disconnect */
		sigc::connection external_on_change_signal ;
	    sigc::connection connection_autosave ;

		/* action group of clipboard */
		Glib::RefPtr<Gtk::ActionGroup> actionGr ;

		/* flag for first tour */
		bool first_tour ;

		/* indicates if clipboard is linked with a view */
		bool active ;

		void set_widgets_label() ;

		/**
	   * Set the file buffer for the clipboard to be linked to
	   * @param ext_buf: TextBuffer from the file
	   */
		void set_buffer(Glib::RefPtr<AnnotationBuffer> ext_buf) ;

		/** Clipboard actions **/
		void up_selection() ;
		void down_selection() ;
		void import_text() ;
		void export_text() ;

		bool is_in_buffer(Glib::ustring txt) ;

		/** Used to know if selection exist in file buffer or in clipboard buffer **/
		void on_change_cursor(const Gtk::TextBuffer::iterator& it, const Glib::RefPtr<Gtk::TextBuffer::Mark >& mark) ;

		bool on_focus_in_event(GdkEventFocus* event) ;
 		bool on_focus_out_event(GdkEventFocus* event) ;

		void clear() ;
		void suppress() ;

		//> Display
		void activate_up(bool value) ;
		void activate_down(bool value) ;
		void activate_import(bool value) ;
		void activate_export(bool value) ;
		void activate_suppress(bool value) ;
		void activate_clear(bool value)	;
		void activate_sortAsc(bool value) ;
		void activate_sortDesc(bool value) ;
		void actualise_state() ;

		void order_buffer(std::vector<Glib::ustring>& vector, bool reverse) ;
		void get_vector_from_buffer(std::vector<Glib::ustring>& res) ;
		void set_buffer_with_vector(std::vector<Glib::ustring> vect) ;
		static bool compare_asc(Glib::ustring s1, Glib::ustring s2) ;
		static bool compare_desc(Glib::ustring s1, Glib::ustring s2) ;

		bool autosave() ;
		void display_clipboard() ;

		sigc::signal<void>  m_signalDisplayClipboard ;

		/*** Geometry interface ***/
		virtual void getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel) ;
		virtual void getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual Glib::ustring getWindowTagType()  ;
};

} //namespace


#endif /*CLIPBOARD_H_*/
