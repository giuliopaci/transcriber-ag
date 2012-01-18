/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef FRAMEBROWSER_H_
#define FRAMEBROWSER_H_

#include <gtkmm.h>
#include "FBListView.h"
#include "FBModelColumns.h"
#include "Common/icons/IcoPackImage.h"
#include "Common/widgets/GeoWindow.h"

#include <iostream>
#include <string>
#include <sstream>

namespace tag {

class FrameFiller ;

/**
 * @class 		FrameBrowser
 * @ingroup		MediaComponent
 *
 * Graphic component for browsing into video frames.
 */
class FrameBrowser : public Gtk::Dialog, public GeoWindow
{
	public:

		/**
		 * Constructor
		 * @param p_toplevel			Window toplevel
		 * @param configpath			Path of configuration folder
		 * @param enableResolution		True for enabling resolution changes
		 */
		FrameBrowser(Gtk::Window* p_toplevel, const Glib::ustring& configpath, bool enableResolution) ;

		/**
		 * Destructor
		 */
		virtual ~FrameBrowser();

		/**
		 * Adds a frame entry in the browser.
		 * @param pixbuf	Reference on a pixbuf
		 * @param time		AGtime (in seconds)
		 */
		void addFrame(const Glib::RefPtr<Gdk::Pixbuf> pixbuf, float time) ;

		/**
		 * Actualizes brwoser state and connect all business signals
 		 * @param error		True if an error occurred, false for success
		 */
		void ready(bool error) ;

		/**
		 * Fill model.
		 * @param path		Video file path
		 * @param width		Width for the video preview frame
		 * @param height	Height for the video preview frame
		 * @param step		Resolution step (if step=N, will extract 1 frame each N seconds)
		 */
		void fill(const Glib::ustring& path, int width, int height, int step) ;

		/**
		 * Fill model. (wrapper for string values)
		 * @param path		Video file path
		 * @param width		Width for the video preview frame
		 * @param height	Height for the video preview frame
		 * @param step		Resolution step (if step=N, will extract 1 frame each N seconds)
		 * @remarks			Wrapper for fill(const Glib::ustring&,int,int,int)
		 */
		void fill(const Glib::ustring& path, const Glib::ustring& width, const Glib::ustring& height, const Glib::ustring& step) ;

		/**
		 * Place cursor at the given time
		 * @param AGtime	Time in seconds
		 */
		void setToTime(double AGtime) ;

		/**
		 * Modifies the frame browser resolution
		 * @param step		The frame browser will extract a frame each <b>step</b> seconds.
		 */
		void setResolutionStep(int step) ;

		/**
		 * <b>float parameter:</b> 	Time in seconds\n
		 * @return
		 */
		sigc::signal<void, float>& signalFrameChanged() { return m_signalFrameChanged ; }

		/**
		 * Signal Accessor - m_signalCloseButton
		 * @return m_signalCloseButton signal
		 */
		sigc::signal<void>					signalCloseButton()		{ return m_signalCloseButton; }

		/*** Geometry interface ***/
		virtual void saveGeoAndHide() ;
		virtual int loadGeoAndDisplay(bool rundlg=false) ;

	private:
		/** relayed information **/
		Gtk::Window* toplevel ;
		Glib::ustring configdir ;

		Glib::ustring name ;
		Glib::ustring path ;
		int width ;
		int height ;
		int step ;

		class FBLoadingHeader : public Gtk::Frame
		{
			public :
				FBLoadingHeader(bool with_resolution) ;
				~FBLoadingHeader() {} ;
				Gtk::ComboBoxText* getCombo() { return &combo ; } ;
				void setName(const Glib::ustring& n) { name = n ; } ;
				void ready() ;
				void showConfigImage(const Glib::ustring& dir) ;
			private :
				bool enable_resolution ;
				Gtk::HBox hbox ;
				IcoPackImage image ;
				Gtk::ComboBoxText combo ;
				Glib::ustring configdir ;
				Glib::ustring name ;
				Gtk::Label label ;
		} ;

		/** Structure **/
		FBLoadingHeader* header ;
		Gtk::ScrolledWindow scrolled ;
			FBListView listView ;
		Gtk::Tooltips tooltip ;

		/** Model **/
		Glib::RefPtr<Gtk::ListStore> listModel ;
		Glib::RefPtr<Gtk::TreeModelFilter> filteredListModel ;

		/** Selection **/
		Gtk::TreePath selected ;

		/** Tools ***/
		FrameFiller* frameFiller ;
		Glib::ustring videoPath ;

		/** Options **/
		bool enable_resolution ;

		/** loading status **/
		bool isReady ;

		/*** Internal ***/
		void prepareList() ;
		void prepareGUI() ;
		bool checkVisibility(float frame) ;
		bool listFilterCallback(const Gtk::TreeIter& iter) ;
		void selectionChanged(const Gtk::TreeIter& iter) ;
		void prepareCombo() ;
		void on_combo_changed() ;

		/*** Dialog event ***/
		bool on_focus_in_event(GdkEventFocus* e) ;
		bool on_delete_event(GdkEventAny* event) ;
		bool on_key_press_event(GdkEventKey* event) ;

		/*** Seek ***/
		void setCursorAtPath(std::string s_path) ;
		std::string getPathAtTime(double AGtime) ;

		float string_to_number(const Glib::ustring& s)
		{
			float res = 0 ;
			std::istringstream istr(s) ;
			istr >> res ;
			return res ;
		}

		/*** test ***/
		Glib::Timer tim ;

		sigc::signal<void, float> 	m_signalFrameChanged ;
		sigc::signal<void>			m_signalCloseButton;

		/*** Geometry interface ***/
		virtual void getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual void setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel) ;
		virtual void getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
		virtual Glib::ustring getWindowTagType()  ;
};


} //namespace

#endif /* FRAMEBROWSER_H_ */
