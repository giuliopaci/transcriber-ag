/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */


#ifndef AGEDITOR_H_
#define AGEDITOR_H_

#include <iostream>
#include <sstream>
#include <gtkmm.h>

#include "Common/globals.h"
#include "Common/Parameters.h"
#include "ActivityWatcher.h"
#include "AnnotationIndex.h"
#include "DataModel/UndoableDataModel.h"

#include "Common/util/Utils.h"
#include "Common/util/StringOps.h"
#include "Common/iso639.h"
#include "Common/FileInfo.h"
#include "Common/ColorsCfg.h"
#include "Common/Explorer_filter.h"


#define THREADS_ENTER if ( m_threads ) gdk_threads_enter();
#define THREADS_LEAVE if ( m_threads ) { gdk_flush(); gdk_threads_leave(); }

namespace tag {

class SegmentTrackWidget;
class AudioSignalView ;
class AGEditor ;


/**
 * @typedef 	SegThreadData
 * Data used by thread.
 */
typedef struct
{
	int notrack; 				/**< Track number */
	AGEditor* edit;		/**< Pointer on AnnotationEditor object */
} SegThreadData;

/**
*  @class 		AGEditor
*  @ingroup		AnnotationEditor
*
*  TranscriberAG basic editor widget.\n
*  All editors should heritate from this class.
*/
class AGEditor : public Gtk::Frame, public ActivityObject
{
	public:

		/**
		*  @class 		SegIndex
		*  @ingroup		AnnotationEditor
		*
		*  Class used for providing a segment management by cache.\n
		*  Represents a time coded segment, and provides a segment comparison operator
		*/
		class SegIndex
		{
			public:
				/**
				 * Constructor
				 */
				SegIndex() : id(""), startTime(0.0), order(0) {}

				/**
				 * Constructor
				 * @param p_id		Segment id
				 * @param p_start	Start time
				 * @param p_order	Segment order
				 */
				SegIndex(string p_id, float p_start, int p_order) : id(p_id), startTime(p_start), order(p_order) {}

				/**
				 * Construcor copy
				 * @param copy		Basis SegIndex
				 */
				SegIndex(const SegIndex& copy) : id(copy.id), startTime(copy.startTime), order(copy.order) {}

				string id;			/**< Segment id */
				float startTime;	/**< Start time */
				int order;			/**< Segment order (0 for basis order, 1 for overlap) */

				/**
				 * Inferiority comparison operator
				 * @param cmp	SegIndex to compare
				 * @return		True if startTime < <em>cmp</em>.startTime\n
				 * 				False if startTime > <em>cmp</em>.startTime\n
				 * 				True if startTime = <em>cmp</em>.startTime & order < <em>cmp</em>.order
				 */
				bool operator < (const SegIndex& cmp) const {
					if ( startTime < cmp.startTime ) return true;
					if ( startTime > cmp.startTime ) return false;
					return ( order < cmp.order );
				}
		};


	/*****************************************************************************/
	/********************************** COMMON ***********************************/
	/*****************************************************************************/
	public:

		/**
		 * Constructor
		 * @param top	Parent window pointer
		 */
		AGEditor(Gtk::Window* top);
		virtual ~AGEditor();

		/**
		* get current open AG filename
		* @return AG file name
		*/
		virtual const std::string& getFileName();

		/**
		* Accessor to the file DataModel instance
		* @return 		Reference on Data model
		*/
		virtual UndoableDataModel& getDataModel() { return m_dataModel; }

		/**
		* Accessor to the file DataModel instance
		* @return 		Pointer on Data model
		*/
		virtual UndoableDataModel* getDataModelPtr() { return &m_dataModel; }

		/**
		* Accessor to the file model
		* @return 		Reference on the DataModel used.
		*/
		virtual UndoableDataModel& dataModel() { return m_dataModel; }

		/**
		 * Accessor to the transcription language.
		 * @return		The Iso639-3 code of the transcription language
		 */
		virtual const std::string& getTranscriptionLanguage() const { return m_lang; }

		/**
		* Checks signal view (audio component) existence
		* @return 			True of False
		*/
		virtual bool hasSignalView() { return (m_signalView != NULL); }

		/**
		 * Defines whether streaming mode is active
		 * @param flag		True for activating, False otherwise
		 */
		virtual void setStreamMode(bool flag)				{ m_rtsp = flag; }

		/**
		 * Sets the streaming URL
		 * @param path		Streaming URL
		 */
		virtual void setRtspPath(const std::string& path)	{ m_rtsp_path	= path; }

		/**
		 * Sets the stream channels
		 * @param chn		Number of channels
		 */
		virtual void setRtspChannels(int chn)				{ m_channels	= chn; }

		/**
		 * Gets UI info for editor (can be added to host application UI manager)
		 * @param groupname		group name (available: annotate / signal)
	     * @return 				The corresponding UI info string
		 */
		virtual const std::string& getUIInfo (string groupname) { return m_UIInfo[groupname]; }

		/**
		 * Accessor to the audio component
		 * @return			Pointer on the component, or NULL if none
		 */
		virtual AudioSignalView*	getSignalView()	{ return m_signalView; }

		/**
		 * Sets the command line offset (-o option )
		 * @param offset	Time in seconds
		 * @note 			Only actualize internal value; the real setting will be
		 * 					applied by calling setSignalOffset(float)
		 */
		virtual void setCommandLineSignalOffset(float offset) {	m_command_line_offset = offset ; }

		/**
		 * Sets the audio component cursor to the given value
		 * @param offset	Time in seconds
		 * @note			When offset set to -1, will checks if some command line
		 * 					has been used for setting the offset
		 * @see				setCommandLineSignalOffset(float)
		 */
		virtual void setSignalOffset(float offset) ;

		/**
		 * Accesor to top level window
		 * @return		Pointer on the top level window
		 */
		virtual Gtk::Window* getTopWindow();


	/*****************************************************************************/
	/********************************** INTERFACE ********************************/
	/*****************************************************************************/
	public:

		/**
		* Saves annotation file
		* @param filename 				File name (if empty use last loaded file name)
		* @param evenNotModified		True for forcing when no modifications has been proceeded
		* @return 						True for success, False for failure
		*/
		virtual bool saveFile(std::string filename, bool evenNotModified=false) = 0;

		/**
		*  Requests to close edited file if buffer modified, user will be prompted to eventually save file before closing.
		*  @param force 		True to force closing without prompting user even if buffer modified
		*  @param emitSignal	Only used if <em>force</em> is true: set it to True for emitting close signal, false otherwise
		*  @note 				As user may cancel action, calling application must connect to signalCanClose()
		* 						to check whether file was actually closed
		*  @attention			If emitSignal is set to True, it will ask the container (by default Notebook_mod class)
		*  						to close the file. If you need the editor to stay alive, set it to false
		*/
		virtual void closeFile(bool force, bool emitSignal) = 0 ;

		/**
		 * Reloads file from file system or from auto save.
		 * @param from_autosave		True for reloading from last auto save,
		 * 							False for reloading from last save (from file system)
		 */
		virtual void reloadFile(bool from_autosave=false) = 0 ;

		/**
		 * Checks if some update have been done since last save.
		 * @return		True if the file is in modified state.
		 */
		virtual bool fileModified() = 0 ;

		/**
		 * Proceeds to autosave file (using auto save file name).
		 * @param inThread		True if launching since a thread
		 * @return				True for success, False for failure
		 */
		virtual bool autosaveFile(bool inThread=false) = 0 ;

		/**
		 * Activates or inhibits the undo/redo feature for next edit actions
		 * @param b			True for activation, False otherwise
		 */
		virtual void setUndoableActions(bool b) = 0 ;

		/**
		 * Checks if the editor is in edition mode
		 * @return		True of False
		 */
		virtual bool isEditMode() = 0 ;

		/**
		 * Creates a default file name regarding the signal file name
		 * @param tagfilepath			If not empty, its basename will be used as transcription file name
		 * @param signalfilepath		Signal file path
		 * @return						The computed file name
		 */
		virtual string makeDefaultFilename(const std::string tagfilepath, const std::string signalfilepath) = 0 ;

		/**
		 * Initializes configuration options
		 * @param options		Configuration options
		 * @param reload		False for first call, True otherwise
		 */
		virtual void setOptions(Parameters& options, bool reload=false) = 0  ;

		/**
		 * Set an option value
		 * @param option	Option key
		 * @param value		Option value
		 */
		virtual void setOption(const string& option, const string& value) = 0 ;

		/**
		 * Accessor to an option value
		 * @param option	Option key
		 * @return			Option value, or empty string if option couldn't be found
		 */
		virtual const string& getOption(const string& option) = 0 ;

		/**
		 * Gets the last activity time.
		 * @return		Time of the last user action (in seconds).
		 */
		virtual time_t lastActiveTime() const = 0 ;

		/**
		 * Gets menu items for editor (can be added to host application UI manager)
		 * @param groupname		Group name (available: file / edit / annotate / signal / display)
		 * @return				Pointer on the corresponding ActionGroup
		 */
		virtual const Glib::RefPtr<Gtk::ActionGroup>& getActionGroup (const string& groupname) = 0 ;

		/**
		 * Actions to proceed when the editor page is activated (called within a notebook)
		 * @param on		True if the page is being activated, false otherwise
		 */
		virtual void onPageActivated(bool on) = 0 ;

		/**
		*   Signal indicating that file close action has succeeded and that editor
		*   window can now safely be closed.
		*/
		sigc::signal<void>& signalCanClose() { return m_signalCanClose; }

		/**
		*  Signal indicating that file close action was canceled by user
		*/
		sigc::signal<void>& signalCloseCancelled() { return m_signalCloseCancelled; }

		/**
		*  	  Signal indicating that file save or saveas action has succeeded.\n
		*     <b>std::string parameter:</b> 	file path\n
		*     <b>bool parameter:</b> 			is new file ?\n
		*	  <b>bool parameter:</b> 			force notebook to rename tab\n
		*	  <b>bool parameter:</b> 			force explorer to expand file row\n
		*/
		sigc::signal<void, std::string, bool, bool, bool>& signalFileSaved() { return m_signalFileSaved; }

		/**
		*  Signal indicating that file was autosaved\n
		*  <b>std::string parameter:</b> 	saved file path\n
		*/
		sigc::signal<void, std::string>& signalFileAutosaved() { return m_signalFileAutosaved; }

		/**
		*  signal indicating whether a file was modified\n
		*  <b>bool parameter:</b> 			True if modified state, false if saved state\n
		*/
		sigc::signal<void, bool>& signalFileModified() { return m_signalFileModified; }

		/**
		*  Signal indicating that signal view is shown.\n
		*  <b>int parameter:</b>	 Used for debug only
		*/
		sigc::signal<void, int>& signalSignalViewAdded() { return m_signalSignalViewAdded; }

		/**
		 * Signal indicating that drag target was received.
		 */
		sigc::signal<void>& signalGtkDragTarget() { return m_signalGtkDragTarget; }

		/**
		 * Signal emitted when complete loading is done.
		 */
		sigc::signal<void>& signalReady() { return m_signalReady; }

		/**
		*  Signal indicating that file was autosaved\n
		*  <b>std::string parameter:</b> 	message to display\n
		*/
		sigc::signal<void, std::string>& signalStatusBar() { return m_signalStatusBar; }

		/**
		 * Signal emitted each time the display is reloading (loading, refresh, change view, etc...)
		 */
		sigc::signal<void>& signalDisplayReloading() { return m_signalDisplayReloading ; }


	/*****************************************************************************/
	/********************************** INTERFACE ********************************/
	/*****************************************************************************/

	protected:

		/*** STATUS ***/
		bool m_loaded ;
		bool m_isAutosaved ;	/* is autosaved */
		bool m_isLocked ;		/* is isLocked */
		bool m_newFile ;
		bool m_loading_cancelled ;

		/*** ARCHITECTURE ***/
		Gtk::Window* m_top;	/** top window */
		Gtk::Box*	m_box;	/** main pack box */
		Gtk::Statusbar* m_statusBar;
		void setStatusBar(Gtk::Statusbar* sb) { m_statusBar=sb; }

		/*** FILE DATA ***/
		std::string m_agFilename;  /** current filename */
		std::string m_fileFormat;  /* loaded file format */
		std::string m_autosavePath; /* path autosave */
		std::string m_lastsavePath; /* last save file */
		std::string m_workdir;  /** current work directory */
		std::string m_defaultFilename; /** default file name for new file */
	    sigc::connection m_autosave;  /** timeout signal connection id for autosave */

	    /*** command line offset ***/
	    float m_command_line_offset ;
	    string m_command_line_offset_segid ;

		/*** ACTION GROUPS ***/
		std::map<std::string, Glib::RefPtr<Gtk::ActionGroup> >  m_actionGroups ;
		std::map<std::string, Glib::RefPtr<Gtk::Action> > m_actions ;
		std::map<std::string, Glib::RefPtr<Gtk::ToggleAction> > m_toggleActions ;
		std::map<std::string, std::string> m_UIInfo ;

		/*** MODEL DATA ***/
		UndoableDataModel m_dataModel;	/** data model (AG) */
		std::map<std::string, std::string> m_configuration; /**> configuration parameters */
		std::map<std::string, std::string> m_modelCfg; /**> configuration parameters */
		std::map<std::string, std::string> m_colorsCfg ; /**> configuration parameters */

		/*** INTERNAL ***/
		bool m_threads;   /** to enclose dialogs with gdk_threads_enter/leave pairs or not */
		ActivityWatcher* m_activityWatcher;   /** activity watcher for current editor */
		GThread* m_segThread;
		int m_nbThread;

		/*** TRACKS ***/
		std::vector< std::map<std::string, SegmentTrackWidget*> > m_segmentTrack;
		std::map<std::string, string> m_trackColor; // color for tracks
		std::vector< std::map<std::string, SignalSegment> > m_currentSegment;
		std::vector< std::map< std::string, AnnotationIndex > > m_indexes;

		/*** SIGNAL ***/
		AudioSignalView* m_signalView; /**<  signal view and management widget */
		sigc::signal<void, int> m_signalSignalViewAdded ;

		/*** CONVENTIONS ***/
		std::string m_conventions; /** current annotation conventions */
		std::string m_lang;	/** current transcription language */

		/*** SIGNALS ***/
		int	m_nbTracks;	/**< nb audio tracks */
		sigc::signal<void> m_signalCanClose ;
	   	sigc::signal<void, std::string, bool, bool, bool> m_signalFileSaved ;
	   	sigc::signal<void, std::string > m_signalFileAutosaved ;
		sigc::signal<void, bool> m_signalFileModified ;
		sigc::signal<void> m_signalCloseCancelled ;
		sigc::signal<void, string, Glib::RefPtr<Gtk::ActionGroup>& > m_signalUpdateUI ;
		sigc::signal<void> m_signalGtkDragTarget ;
		sigc::signal<void> m_signalReady ;
		sigc::signal<void, std::string> m_signalStatusBar ;
		sigc::signal<void> m_signalDisplayReloading ;

		/*** STREAMING ***/
		bool			m_rtsp;
		int				m_channels;
		std::string		m_rtsp_path;
};

} //namespace

#endif /* AGEDITOR_H_ */
