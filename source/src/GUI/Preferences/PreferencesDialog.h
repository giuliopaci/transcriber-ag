/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/** @file */

#ifndef PREFERENCESDIALOG_H_
#define PREFERENCESDIALOG_H_


#include "Explorer_fileHelper.h"
#include "Common/icons/Icons.h"
#include "Configuration.h"
#include "Common/globals.h"
#include "TreeViewPreferences.h"
#include "TreeModelColumnsPreferences.h"
#include "GeneralFrame.h"
#include "FTPFrame.h"
#include "DataModelFrame.h"
#include "TextEditorFrame.h"
/* SPELL */
//#include "SpellerFrame.h"
#include "SpeakersFrame.h"
#include "ColorsFrame.h"
#include "FontsFrame.h"
#include "AudioFrame.h"
#include "VideoFrame.h"
#include "Common/icons/IcoPackButton.h"
#include "Common/widgets/GeoWindow.h"
#include <gtkmm.h>
#include <iostream>
#include <gtkmm/icontheme.h>

namespace tag {
/**
 * @class 	PreferencesDialog
 * @ingroup	GUI
 *
 * Preferences main dialog
 *
 */
class PreferencesDialog : public Gtk::Dialog, public GeoWindow
{
	public:
		/**
		 * Constructor
		 * @param config			Pointer on the Configuration object (instanciated in top level)
		 * @param p_lastOption		Code indicating the last dialog thematic frame used at previous usage. The dialog
		 * 							will select the corresponding frame.
		 * @see	getLastFrameCode() method
		 */
		PreferencesDialog(Configuration* config, int p_lastOption);
		virtual ~PreferencesDialog();

		/**
		 * @deprecated		Not used anymore
		 * @return			Corresponding signal
		 */
		sigc::signal<void>& signalFTPMachineChanged() { return m_signalFTPMachineChanged; }

		/**
		 * Signal emitted when modifications are applied.\n
		 * <b>std::map<int, Glib::ustring> parameter:</b>	Map of dynamic values\n
		 * <b>integer parameter :</b> 						Indicates if some static values have been changed (set to 1 when some have been)
		 * @return											The corresponding signal
		 */
		sigc::signal<void, std::map<int, Glib::ustring>, int>& signalReloadModifications() { return m_signalReloadModifications ; }

        /**
          * Stores parent window reference
          *
          */
        void set_parent(Gtk::Window* win) { parent = win; if (parent) set_transient_for(*parent); }

        /**
         * Gets the code corresponding to the last chose frame type.
         * @return		Internal code to use in PreferencesDialog constructor
         */
        int getLastFrameCode() { return lastFrameCode ; }

		/*** Geometry interface ***/
		virtual void saveGeoAndHide() ;
		virtual int loadGeoAndDisplay(bool rundlg=false) ;

	private:
		Configuration* config ;

		std::vector<PreferencesFrame*> frames ;
		std::map<int, Glib::ustring> dynamic_values ;
		std::vector<IcoPackImage*> static_values ;

		TreeViewPreferences tview ;
		TreeModelColumnsPreferences m_columns ;
		Glib::RefPtr<Gtk::TreeStore> refModel ;

		//> GENERAL WIDGET
		Gtk::HPaned paned ;
		Gtk::ScrolledWindow scroll ;
		IcoPackButton apply ;
		IcoPackButton cancel ;
		IcoPackButton close ;

		Gtk::VBox datas_vbox ;
        Gtk::Window* parent;

		//> ALL FRAME FOR EACH CATEGORY
		GeneralFrame* gen_frame ;
		DataModelFrame* dmodel_frame ;
		TextEditorFrame* teditor_frame ;
/* SPELL */
//		SpellerFrame* speller_frame ;
		SpeakersFrame* speakers_frame ;
		AudioFrame* audio_frame ;
		ColorsFrame* lookNfeelColors_frame ;
		FontsFrame* lookNfeelFonts_frame ;
		VideoFrame* video_frame ;

		sigc::signal<void> m_signalFTPMachineChanged ;
		sigc::signal<void, Glib::ustring> m_signalEditorFontChanged ;
		sigc::signal<void, std::map<int, Glib::ustring>, int> m_signalReloadModifications ;

		//> session: indicated last chosen frame for restauring it at next opening
		int lastFrameCode ;
		Gtk::TreePath lastFramePath ;
		Gtk::TreePath lastParentFramePath ;
		bool need_expand ;

		void change_frame(int mode) ;
		void prepare_gui() ;
		void prepare_view() ;
		void fill_tree() ;
		void on_selection_change(Gtk::TreePath path) ;
		void on_response(int response) ;
		void on_modified(bool modified) ;
		void on_cancel() ;
		void on_apply() ;
		void reload_data() ;
		bool on_delete_event(GdkEventAny* event) ;
		bool has_static_modifications() ;
		void set_active_warning_visible(bool value) ;

		/*** Geometry interface ***/
		virtual void getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel) ;
		virtual Glib::ustring getWindowTagType()  ;
};

} //namespace

#endif /*PreferenceSDIALOG_H_*/
