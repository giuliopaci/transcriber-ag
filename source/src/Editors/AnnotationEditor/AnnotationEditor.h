/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/**
* @defgroup	AnnotationEditor
*/

/**
 *  @file 	AnnotationEditor.h
 */

#ifndef _HAVE_ANNOTATION_EDITOR_H
#define _HAVE_ANNOTATION_EDITOR_H

#include <map>
#include <string>
#include <glibmm/objectbase.h>
#include <glibmm/timer.h>
#include <gtkmm/icontheme.h>
#include <gtk/gtkimcontext.h>

#include "CommonEditor/AGEditor.h"
#include "AnnotationEditor/menus/SpeakerMenu.h"
#include "AnnotationEditor/AnnotationView.h"

#include "DataModel/speakers/Speaker.h"
#include "DataModel/speakers/SpeakerDictionary.h"

#include "Common/InputLanguageHandler.h"
#include "Common/widgets/ToolLauncher.h"
#include "Common/widgets/ProgressionWatcher.h"
#include "Common/ResultSet.h"

class InputLanguage ;

using namespace std;


//#define TRACEDICT(e) (e).traceDict(__FILE__, __LINE__)
#define TRACEDICT(e)

/**
 * @def		VIEW_MODE_BOTH
 * Dual display for stereo mode
 * (two AnnotationView)
 */
#define VIEW_MODE_BOTH -2

/**
 * @def		VIEW_MODE_MERGED
 * Single merged display mode for stereo mode
 * (unique AnnotationView)
 */
#define VIEW_MODE_MERGED -1

/**
 * @def		VIEW_MODE_ONE
 * Single display for stereo mode (first canal)
 */
#define VIEW_MODE_ONE 0

/**
 * @def		VIEW_MODE_TWO
 * Single display for stereo mode (second canal)
 */
#define VIEW_MODE_TWO 1


namespace tag {

class AnnotationEditor;
class VideoWidget ;
class FrameBrowser ;
class VideoManager ;

/**
*  @class 		AnnotationEditor
*  @ingroup		AnnotationEditor
*
*  TranscriberAG text editor widget.\n
*  Embeds one or several AnnotationView instances.\n
*  Communicates with AudioSignaView.
*/
class AnnotationEditor : public AGEditor
{
	public:

		/**
		* Constructor
		* @param top		Pointer on parent window
		*/
		AnnotationEditor(Gtk::Window* top=NULL);

		~AnnotationEditor();

		/**
		 * Initializes configuration options
		 * @param options		Configuration options
		 * @param reload		False for first call, True otherwise
		 */
		void setOptions(Parameters& options, bool reload=false)  ;

		/**
		 * Set an option value
		 * @param option	Option key
		 * @param value		Option value
		 */
		void setOption(const string& option, const string& value) ;

		/**
		 * Accessor to an option value
		 * @param option	Option key
		 * @return			Option value, or empty string if option couldn't be found
		 */
		virtual const string& getOption(const string& option) ;

		/**
		 * Set the status bar to be used
		 * @param sb		Pointer on status bar
		 */
		void setStatusBar(Gtk::Statusbar* sb) { m_statusBar=sb; }

		/**
		 * Displays given text in status bar
		 * @param st		Text to be displayed
		 */
		void showStatus(const string& st);

		/**
		 * Loads an annotation file in the editor
		 * @param filepath		File path when opening an existing transcription file
		 * @param signalFiles	Signal file names when creating a new transcription file
		 * @param streaming		True for streaming mode, False otherwise
		 * @param lockForced	True for forced locked edition mode, False otherwise
		 * @param reportLevel	Level determining when will be displayed the loading report dialog
		 * 						0: never\n
		 * 						1: always\n
		 * 						2: only if errors
		 * @return				True for success, False for failure
		 */
		bool loadFile(string filepath, std::vector<string>& signalFiles, bool streaming, bool lockForced, int reportLevel) ;

		/**
		 * Prepares the AnnotationView instances.
		 * @param newTranscription		True for new transcription mode
		 * @param recovered				True if recovered for automatic save
		 */
		void initializeView(bool newTranscription, bool recovered) ;

		/**
		 * Prepares DataModel and Conventions values for the new transcription
		 * @param signalFiles		Signal files used for the transcription
		 */
		void prepareNewTranscription(const std::vector<string>& signalFiles) ;

		/**
		*  Requests to close edited file if buffer modified, user will be prompted to eventually save file before closing.
		*  @param force 		True to force closing without prompting user even if buffer modified
		*  @param emitSignal	Only used if <em>force</em> is true: set it to True for emitting close signal, false otherwise
		*  @note 				As user may cancel action, calling application must connect to signalCanClose()
		* 						to check whether file was actually closed
		*  @attention			If emitSignal is set to True, it will ask the container (by default Notebook_mod class)
		*  						to close the file. If you need the editor to stay alive, set it to false
		*/
		void closeFile(bool force, bool emitSignal);

		/**
		* Saves annotation file
		* @param filename 				File name (if empty use last loaded file name)
		* @param evenNotModified		True for forcing when no modifications has been proceeded
		* @param skipPath				Set to true if you want to skip path hint actualization
		* @return 						True for success, False for failure
		*/
		bool saveFile(string filename, bool evenNotModified, bool skipPath, bool checkSpk) ;

		/**
		* Saves annotation file
		* @param filename 				File name (if empty use last loaded file name)
		* @param evenNotModified		True for forcing when no modifications has been proceeded
		* @return 						True for success, False for failure
	    */
		bool saveFile(string filename, bool evenNotModified) { saveFile(filename, evenNotModified, false, true) ;}

		/**
		 * Proceeds to autosave file (using auto save file name).
		 * @param inThread		True if launching since a thread
		 * @return				True for success, False for failure
		 */
		bool autosaveFile(bool inThread=false);

		/**
		* Exports file to another format
		* @return 		True for success, False for failure
		*/
		bool exportFile() ;

		/**
		 * Gets the speaker dictionary of the file
		 * @return		Reference on the speaker dictionary
		 */
		SpeakerDictionary& getSpeakerDictionary() { return m_dataModel.getSpeakerDictionary(); }

		/**
		 * Creates a default file name regarding the signal file name
		 * @param tagfilepath			If not empty, its basename will be used as transcription file name
		 * @param signalfilepath		Signal file path
		 * @return						The computed file name
		 */
		string makeDefaultFilename(const std::string tagfilepath, const std::string signalfilepath);

		/**
		 * Gets the editor configuration map (corresponds to the Parameters %AnnotationEditor component map)
		 * @return			The configuration map used by the AnnotationEditor for editor options
		 * @see				Parameters class
		 */
		std::map<std::string, std::string>& getConfiguration() { return m_configuration; }

		/**
		 * Gets the model configuration map (corresponds to the Parameters %DataModel component map)
		 * @return			The configuration map used by the AnnotationEditor for model options
		 * @see				Parameters class
		 */
		std::map<std::string, std::string>& getModelCfg() { return m_modelCfg; }

		/**
		 * Gets the display configuration map (corresponds to the Parameters %Display component map)
		 * @return			The configuration map used by the AnnotationEditor for display options
		 * @see				Parameters class
		 */
		std::map<std::string, std::string>& getColorsCfg() { return m_colorsCfg ; }

	#ifdef TODEL
		/**
		 * add new turn marker in annotation buffer
		 * @param speaker new turn speaker definition
		 * @param startTime turn start time
		 * @param endTime turn end time
		*/
		const std::string& addTurn(const Speaker& speaker, std::string startTime, std::string endTime="", int track = 1);

		/**
		 * add new turn text in annotation buffer
		 * @param text turn text
		 * @param id turn id
		*/
		void addTurnText(const std::string& text, const std::string& id);
	#endif

		/**
		 * Activates or inhibits the undo/redo feature for next edit actions
		 * @param b			True for activation, False otherwise
		 */
		void setUndoableActions(bool b);

		/**
		 * Accessor to the active AnnotationView .
		 * @return				Reference the embedded AnnotationView currently active.
		 */
		AnnotationView& getView() { return *m_activeView; }

		/**
		 * Accessor to the AnnotationView corresponding to the given track
		 * @param notrack		Track number
		 * @return				Pointer on the corresponding AnnotationView
		 */
		AnnotationView* getView(int notrack) ;

		/**
		 * Retrieves segment text from AnnotationBuffer and load it into return buffer
		 * @param 			id 			Annotation id
		 * @param[out] 		res 		Returned buffer pointer
		 * @param 			notrack 	Current track
		 * @param 			type 		Annotation type
		 * @note  			Returned string is allocated and will be freed by caller
		 */
		void getSegmentText(const string& id, char*& res, int notrack, const string& type);

		/**
		* Accessor to the active text view
		* @return 		Reference on editor text active view
		*/
		Gtk::TextView& getTextview() { return *((Gtk::TextView*)m_activeView); }

		/**
		 * Launches or stops the playback
		 * @param	play				True for launching, False for stopping
		 * @param	inhibateSynchro		True inhibate synchronization signal emission, false otherwise
		 */
		void	setPlay(bool play, bool inhibateSynchro=false);

		/**
		 * Gets the number of tracks used for the transcription
		 * @return		Number of channel
		 */
		int getNbTracks() { return m_nbTracks; }

		/**
		 * Sets focus on the active view
		 * @return			False
		 * @attention		Has to be called from signal_idle. Otherwise, use setFocus() method
		 */
		bool setFocusWhenIdle();
		
		/**
		 *	Sets focus on the actibe view
		*/
		void setFocus() ;
		
		/**
		 * Refreshes the AnnotationEditor.\n
		 * Forces synchronization between view and model.
		 * @return		False
		 * @attention	This action resets undo / redo stack
		 * @attention	Has to be called from signal_idle.
		 * @note		Calls AnnotationEditor::refresh() with idle signal.
		 */
		bool refreshWhenIdle() ;

		/**
		 * Refreshes the AnnotationEditor.\n
		 * Forces synchronization between view and model
		 * @param		disableThread	True for remove thread protection (should be mainly used to true value)
		 * @attention	This action resets undo / redo stack
		 */
		void refresh(bool disableThread=false) ;

		/**
		 * Reloads file from file system or from auto save.
		 * @param from_autosave		True for reloading from last auto save,
		 * 							False for reloading from last save (from file system)
		 */
		void reloadFile(bool from_autosave=false);

		/**
		 * Set file mode
		 * @param mode		"EditMode" for edition mode, "BrowseMode" for read-only
		 * @param force		Default behaviour is doing nothing when applying the actual mode... If set to
		 * 					true, this test is skipped
		 */
		void setMode(const string& mode, bool force=false);


		/**
		 * Checks if some update have been done since last save.
		 * @return		True if the file is in modified state.
		 */
		bool fileModified();

		/**
		 * Closes AnnotationEditor (cleaning speller, DataModel)
		 * @param 	emitCloseSignal		True for tellng parent to kill editor.
		 * @return						False
		 */
		bool terminateSession(bool emitCloseSignal=true);

		/**
		 * Gets the last activity time.
		 * @return		Time of the last user action (in seconds).
		 */
		virtual time_t lastActiveTime() const ;

		/**
		 * Checks the stereo mode
		 * @return		True if stereo, False otherwise
		 */
		bool isStereo() { return m_modeStereo ;}

		/**
		 * Sets the highlight configuration
		 * @param mode		Highlight mode\n
		 * 						0: highlight track 1 (indice 0)
		 * 						1: highlight track 2 (indice 1)
		 * 						2: highlight both track
		 * 						3: highlight selected track
		 */
		void setHighlightMode(int mode)  ;

		/**
		 * Sets the tag display configuration
		 * @param mode		Tag display hidden mode\n
		 * 						1: hide all qualifiers
		 * 						2: hide only events
		 * 						3: hide only named entities
		 * 						4: hide only unknown/invalid qualifiers
		 */
		void setHiddenTagsMode(int mode) ;

		/**
		 * Accessor to current highlight mode
		 * @return			The currently used highlight mode.
		 * @see				setHighlightMode(int)
		 */
		int getHighlightMode() { return m_highlightMode ; }

		/**
		 * handler for all edition actions
		 * @param action			Action name
		 * @param on_selection		True if selection is set in text
		 * @param ignoreTime		True if current action not linked to current signal offset
		 * @param hint				Speaker hint
		 */
		void onMenuAction(const std::string& action, bool on_selection, bool ignoreTime, string hint);

		/**
		 * Changes the view mode.
		 * @param view				View mode:\n
		 * 							-2: dual view (only for stereo)\n
		 * 							-1: merged view (default view for mono)\n
		 * 							 0: view 1 (only for stereo)\n
		 * 							 1: view 2 (only for stereo)\n
		 * @param only_visibility	True for only actualize the visibility of the
		 * 							views.
		 * @note				   	Sometimes Gtk refreshing all views,
		 * 							even those that should be hidden. In these case,
		 * 							use this method with <em>only_visibility</em> set
		 * 							to true.
		 */
		void changeActiveViewMode(int view, bool only_visibility=false) ;

		/**
		 * Force the display mode with current value (force applying again)
		 * @return					Current display mode
		 */
		int resetDisplayMode() ;

		/**
		 * Sets the view mode.
		 * @param switching			True for loading the view (slow, mainly false).
		 * @param inThread			True if launched from a thread.
		 */
		void setDefaultViewMode(bool switching, bool inThread) ;

		/**
		 * Accessor to the view mode
		 * @return			Code of the active view mode
		 */
		int getActiveViewMode() { return m_activeViewMode ;}

		/**
		 * Accessor to the active track number
		 * @return			Number of the active track
		 */
		int getActiveViewTrack() { return m_activeTrack ;}

		/**
		 * Accessor to the active view
		 * @return			Pointer on the active view
		 */
		AnnotationView* getActiveView() { return m_activeView; }

		/**
		 * Accessor to pointer of all annotation views used by editor
		 * @return	Vector of AnnotationView*
		 */
		const std::vector<AnnotationView*>& getViews() { return m_textView ; }

		/**
		 * Changes the editor cursor for displaying the wait icon
		 * @param b			True for switching to wait state, False for leaving the wait state
		 */
		void setWaitCursor(bool b=true) ;

		/**
		 * Sets the active InputLanguage
		 * @param il		Pointer on the active InputLanguage
		 */
		void set_input_language(InputLanguage* il) ;

		/**
		 * Accessor to the active InputLanguage
		 * @return			Pointer on the active inputLanguage
		 */
		InputLanguage* get_input_language() { return m_ilang ; }

		/**
		 * Defines an rc style for all views of editor
		 * @param styleFile		Path of the corresponding Gtk rc file
		 */
		void setViewStyle(Glib::ustring styleFile) ;

		/**
		 * Defines the font style of all AnnotationView enbedded in the editor
		 * @param font		Font name to set
		 * @param mode		"text" for setting normal text font, "label" for label font
		 * @note			When used in "text" mode, this will impact qualifiers too
		 */
		void setFontStyle(const string& font, const string& mode) ;

		/**
		 * Activates (raises and uses) or not the external IME (if some is used)
		 * @param activate		True for activating the external IME, False otherwise.
		 */
		void externalIMEcontrol(bool activate) ;

		/**
		 * Gets menu items for editor (can be added to host application UI manager)
		 * @param groupname		Group name (available: file / edit / annotate / signal / display)
		 * @return				Pointer on the corresponding ActionGroup
		 */
		virtual const Glib::RefPtr<Gtk::ActionGroup>& getActionGroup (const string& groupname) ;

		/**
		 * Modifies the autosave period
		 * @param autosave_period		New autosave period
		 */
		void reset_autosave(int autosave_period) ;

		/**
		 * Modifies the activity period
		 * @param activity_period		New activity period
		 */
		void reset_activity(int activity_period) ;

		/**
		 * Modifies the autoset language mode
		 * @param autoset				New autoset language mode
		 */
		void reset_autosetLanguage(bool autoset) ;

		/**
		 * Modifies the stopOnClick mode
		 * @param stopOnClick		stopOnClick mode
		 */
		void reset_stopOnClick(bool stopOnClick) ;

		/**
		 * Reloads the defined speller
		 */
		void reset_speller() ;

		/**
		 * Modifies the time scale visibility
		 * @param visibility		True for visible, False otherwise
		 */
		void reset_timeScale(bool visibility) ;

		/**
		 * Modifies the entity tags background mode
		 * @param use_bg		True for using background, False otherwise
		 */
		void reset_entityTags_bg(bool use_bg) ;

		/**
		 * Loads preferences audio component colors
		 */
		void setAudioColors() ;

		/**
		 * Loads preferences editor colors
		 */
		void setEditorColors() ;

		/**
		 * Checks if one of the embedded views has focus
		 * @return
		 */
		bool viewHasFocus() ;

		/**
		 * Sets the tooltip mode.
		 * @param value		True for allowing tooltip in the text zone, False otherwise
		 */
		void setUseTooltip(bool value) ;

		/**
		 * Checks if the editor is in edition mode
		 * @return		True of False
		 */
		bool isEditMode() ;

		/**
		 * Accessor to edition mode
		 * @return		"BrowseMode" or "EditMode"
		 * @see			isEditMode()
		 */
		string getEditMode() ;

		/**
		 * Set sensitive or unsensitive view
		 * @param sensitive
		 */
		void setSensitiveViews(bool sensitive) ;

		/**
		 * Set sensitive or unsensitive signal view
		 * @param sensitive
		 */
		void setSensitiveSignalView(bool sensitive) ;

		/**
		 * Accessor to the tags visibility status.\n
		 * @return			hidden tag option status
		 * @see				setHiddenTagsMode(int)
		 */
		int getTagHiddenMode() { return m_hiddenTagsMode ; }

		/**
		 * Toggles the tags hidden mode.
		 * Displays or hide all qualifier tags that have AnnotationBuffer::CAN_BE_HIDDEN_TAG property.
		 * @param emit_signal		False for blocking the hidden tag mode signal
		 * @return					The newly applied value
		 */
		int toggleHideTags(bool emit_signal) ;

		/**
		 * Toggles the file edition mode.
		 * @param emit_signal		False for blocking the edition mode signal
		 * @return					The newly applied value
		 */
		string toggleFileMode(bool emit_signal) ;

		/**
		 * Toggles the highlight mode.
		 * @param emit_signal		False for blocking the highlight mode signal
		 * @return					The newly applied value
		 */
		int toggleHighlightMode(bool emit_signal) ;

		/**
		 * Toggles the synchronization mode.
		 * @param type				Synchronization type
		 * @param emit_signal		False for blocking the synchronization mode signal
		 * @return					The newly applied value
		 */
		string toggleSynchro(string type, bool emit_signal) ;

		/**
		 * Toggles the display mode.
		 * @param emit_signal		False for blocking the display mode signal
		 * @return					The newly applied value
		 */
		int toggleDisplay(bool emit_signal) ;

		/**
		 * Remove the tag corresponding to the given name from all buffers.
		 * @param tagname		Tag name
		 * @note				Does not remove the tag but just "untag" all tagged words
		 */
		void clearViewTag(std::string tagname) ;

		/**
		 * Launches segment data display for the given track
		 * @param notrack		Track number
		 */
		void getTrackSegmentation(int notrack);

		/**
		 * Launches tracks display for the given track
		 * @param notrack	Track number
		 * @return			False (for event mechanism)
		 * @attention		Has to be called from signal_idle
		 */
		bool showLabelTracksWhenIdle(int notrack);

		/**
		 * Enable or Disable thread mechanism
		 * @param 		disable		True for disable, False for allowing
		 * @attention				Mostly you don't need to use this method\n
		 * 							Disable thread when using a method that will call
		 * 							the AnnotationEditor::changeActiveViewMode method
		 * 							without a thread mechanism.
		 */
		void disableViewThreads(bool disable) ;

		/*** parent method ***/
		void onPageActivated(bool on) ;

		/**
		* Checks signal view (audio component) existence
		* @return 			True of False
		*/
		bool hasVideo() ;

		/**
		 * Modifies the frame browser resolution
		 * @param step		The frame browser will extract a frame each <b>step</b> seconds.
		 */
		void setFrameBrowserResolution(int step) ;

		/**
		 * Set the result set path (plugin only)
		 */
		void setHighlightResultset(Glib::ustring resultset) ;

		/**
		 * Set the result set path (plugin only)
		 */
		ResultSet* getHighlightResultset() { return resultSet ;}

		/**
		 * Returns the model id of the element corresponding to the command line offset
		 * @return		Element id or empty if no offset was done in command line
		 */
		string getCommandLineOffsetSegid() { return m_command_line_offset_segid ; }

		/**
		 * Set the drag'n'drop target list
		 * @param targetList
		 */
		void addDragAndDropTarget(Gtk::TargetEntry targetList) ;

		/**
		 * Sets the audio component cursor to the given value
		 * @param offset	Time in seconds
		 * @note			When offset set to -1, will checks if some command line
		 * 					has been used for setting the offset
		 * @see				setCommandLineSignalOffset(float)
		 */
		void setSignalOffset(float offset) ;

		/**
		 * Accessor to the signal selection time stamps.
		 * @param[out] start	Start time stamp, or -1 if error / none
		 * @param[out] stop		End time stamp, or -1 if error / none
		 * @return				-1 : error\n
		 * 						 0 : no selection is in signal\n
		 * 						 1 : selection is in signal
		 */
		int getSignalSelection(float& start, float& stop) ;

		/**
		 * Returns true if the editor is in loading state
		 * @return		True or False
		 */
		bool getLoaded() { return m_loaded ; }

		// -- Apple Font Handler --
		#ifdef __APPLE__
		void	setFontEngineStatus(bool mode)	{ fontInitialized = mode; }
		bool	getFontEngineStatus()			{ return fontInitialized; }
		#endif

		//***
		//*** SIGNALS ***/
		//***

		/**
		 * Defines whether confidence highlight on scored word is enabled
		 * @param value		If true, scored words will be displayed in bold characters.
		 */
		void setWithConfidence(bool value) ;

		/**
		 * Sets whether the first focus has been received
		 * @param on	True or False whether the first focus has been received
		 */
		void setFirstFocus(bool on) {  firstFocus = on ; }

		/**
		 * Accessor to the first focus reception status
		 * @return		True or False whether the first focus has been received
		 */
		bool getFirstFocus() { return firstFocus ; }

		/**
		 * Signal indicating active view change
		 */
		sigc::signal<void>& signalStreamingError() { return m_signalStreamingError ; }

		/**
		 *  Signal indicating that an annotation action group has changed.\n
		 *  <b>std::string parameter:</b> 							Corresponding UI info\n
		 *  <b>Glib::RefPtr<Gtk::ActionGroup> parameter:</b> 		Corresponding action group
		 */
		sigc::signal<void, string, Glib::RefPtr<Gtk::ActionGroup>& >& signalUpdateUI() { return m_signalUpdateUI; }

		/**
		 * Signal indicating active view change
		 */
		sigc::signal<void>& signalChangeActiveView() { return m_signalChangeActiveView; }

		/**
		 * Signal emitted when a tag event on speaker label is receive.\n
		 * <b>std::string parameter:</b> 	Speaker id\n
		 * <b>bool parameter:</b> 			Whether the speaker dialog has to be launched in modal mode\n
		 */
		sigc::signal<void, string, bool>& signalEditSpeaker() { return  m_signalEditSpeaker; }

		/**
		 * Signal emitted when hidden tag mode changes.\n
		 * <b>int parameter :</b> 		New hidden tag mode
		 */
		sigc::signal<void, int>& signalTagHiddenChanged() { return  m_signalTagHiddenChanged; }

		/**
		 * Signal emitted when highlight mode changes.\n
		 * <b>int parameter :</b> 		New highlight mode\n
		 */
		sigc::signal<void, int>& signalHighlightChanged() { return  m_signalHighlightChanged; }

		/**
		 * Signal emitted when synchronization mode changes.\n
		 * <b>std::string parameter :</b> 	Synchronization mode (signal with text or text with signal)\n
		 * <b>std::string parameter :</b> 	New value\n
		 */
		sigc::signal<void, string, string>& signalSynchroChanged() { return  m_signalSynchroChanged; }

		/**
		 * Signal emitted when display mode changes
		 * <b>int parameter :</b>		New applied mode
		 */
		sigc::signal<void, int>& signalDisplayChanged() { return  m_signalDisplayChanged; }

		/**
		 * Signal emitted when edit mode changes.\n
		 * <b>std::string parameter:</b> 	Newly applied mode\n
		 */
		sigc::signal<void, string>& signalEditModeChanged() { return  m_signalEditModeChanged; }

		/**
		 * Signal emitted when at InputLanguage change
		 * <b>InputLanguage* parameter:</b> 	Pointer on the newly chosen InputLanguage
		 */
		sigc::signal<void, InputLanguage*>& signalLanguageChange() { return m_signalLanguageChange; }

		/**
		 * Signal indicating that drag target was received.
		 * <b>Glib::RefPtr<Gdk::DragContext>& parameter:</b>	Drag'n'drop context\n
 		 * <b>AGEditor* parameter:</b>	Editor pointer\n
		 * <b>string parameter:</b>	Speaker id to be replace (empty if signal is file opening related)\n
		 */
		sigc::signal<void,const Glib::RefPtr<Gdk::DragContext>&,AGEditor*,string >& signalGtkDragTarget() { return m_signalGtkDragTarget; }

		/**
		 * Debug method
		 * @return		True or False
		 */
		bool isDebugMode() ;

		/**
		 * Debug method
		 */
		void printIndex() ;

		/**
		 * Debug method
		 */
		void reportCheckAnchors() ;

		/**
		 * synchroTextToSignal callback
		 *   connected to signal view signalCursorChanged() and signalSelectionChanged() events
		 *   if text-signal synchronisation activated, scrolls text view to make corresponding
		 *   text position visible and highlights corresponding text element
		 *   - current turn
		 *   - current segment
		 *   - current word if corresponding timecodes available.
		 *
		 * @param startTime 	current cursor position / selection start
		 * @param endTime 		selection end / -1 if signalCursorChanged
		 * @param force			True for forcing synchronization even if all conditions are not fulfilled
		 * @param inthread		True if action is launched by signal/thread, false otherwise
		 * @return				True if signal has been handled, false otherwise
		 */
		bool synchroTextToSignal(float startTime, float endTime=-1.0, bool force=false, bool inthread=false);

		/**
		 * synchroSignalToText callback
		 *   connected to text view signalSetCursor() event and signalSelectionChanged event
		 *   if text-signal synchronisation activated, set signal cursor position to
		 *   start time of current text element (turn / word)
		 *  @param pos current text position in view
		 *  @param force			True for forcing synchronization even if all conditions are not fulfilled
		 *  @param view current view
		 */
		void synchroSignalToText(const Gtk::TextIter& pos, bool force, AnnotationView* view);


	private:  // private methods
		/**
		 * Locks edition or restore old edition mode
		 * @param lock			True for locking, False for unlocking
		 */
//		void lockEditionMode(bool lock, bool hide_cursor) ;
		void setLocked(bool lock);


		void cursorChanged(const Gtk::TextIter& iter, bool unused) ;

		#ifdef __APPLE__
		bool	fontInitialized;
		#endif

		void updateTrack(string type, string id, int action, bool text_only, int notrack, bool fill_tracks);
		void terminateDisplayLoading() ;

		void setFileName(const std::string& name);
		bool loadFile(string filepath, std::vector<string>& signalFiles,
							bool newTranscription, bool multiAudio, bool streaming) ;

		void showLabelTracks(int notrack);

		string getSegmentColor(const string& id, const string& type) ;
		void onModelUpdated(bool b) ;
		void setViewInterline(int above, int below) ;
		void getViewInterline(int& above, int& below) ;

		/**
		 * Changes or adds corpus information.\n
		 * * 1:  Uses Corpus information defined in the <b>configuration file</b> transcriberAG.rc\n
		 * * 2:  If no values found, uses Corpus information defined in the <b>conventions file</b>\n
		 */
		void actualizeCorpusInformation() ;
		bool selectSignalFiles() ;
		bool checkSignalFiles(string type) ;
		bool selectVideoFile() ;
		bool openDataModel(string filename) ;
		std::map<string,string> fromFilesToSignals(const std::vector<string>& files) ;
		bool checkAutosave() ;

		void actualizeActions() ;
		void createActionGroups();
		void configureAnnotationUI();

		bool addSignalView(std::vector<string>& signalFiles);
		void createMediaComponents(const string& filepath, bool videoMode) ;
		void deleteVideoComponents() ;
		bool openSignals(std::vector<string>& signalFiles) ;
		void configureMediaActions() ;
		void configureMediaSignals() ;

		std::vector<string> fromSignalsToFiles() ;
		AnnotationView* addTextView(int notrack);

		void setTranscriptionType(const string& type);

		void showSegmentTracks(const string& type="", int notrack=-1, bool loading=false);
		void onForceSynchroSTT();
		void logLoading(bool success, bool inThread) ;

		void synchroVBrowserWithSignal(float time, float unusedEndTime) ;
		void onFrameBrowserChanged(float time) ;

		std::string synchroTextToSignalWithTypeAndTrack(const string& type,const string& id, int track, bool do_highlight, bool inthread=false) ;
		std::string synchroTextToSignalWithType(const string& type, float startTime, float endTime=-1.0, int notrack=0, bool inthread=false);
		void synchroTextToSignalIdle(float startTime, float endTime, bool force) ;
		void inhibateSynchro(bool b=true) { m_inhibateSynchro = b; }

		void onAudioSeekReceived(double ts, bool stop_on_click) ;
		void onAudioPlayPauseReceived(bool val) ;
		void onVideoStopReceived() ;
		void onSyncSelection(float s_begin, float s_end) ;

		void setTextCursor(bool play);
		void selectSignalTrack(int notrack, string item);
		void selectSegmentTrack(int notrack, string type) ;
		void onChangeInputLanguage(int offset);
		void onUpdateConventions(); 		/**<  Reconfigure UI when model conventions have changed	*/
		void configureSynchrotypes() ;
		void onSelectSignalTrack(int notrack);
		void onModifySegment(const string& idseg, float start, float end, SegmentTrackWidget* ptrack);
		void onActivateSignalTrack(bool activated) ;
		void saveSelectionSignal();
		void exportSelectionTo(string program);
		void onExternalAction(ToolLauncher::Tool* tool) ;
		void goToPosition() ;
		void onSignalPopulatePopup(int notrack, int x, int y, Gtk::Menu* menu);

	  /**
	   *   callback for file actionGroup actions
	   * @param action file action to perform save / saveas / close
	   * @param buffer_modified true if text buffer modified
	   */
		void onFileAction(const std::string& action);

		/**
		 * if modified state of buffer changes to true -> emits fileModified signal
		 */
		void onFileModified();
		bool hasModifiedBuffer();

		bool on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time);
		void onViewDragAndDropReceived(const Glib::RefPtr<Gdk::DragContext>& context, string id) ;

		/*! get / set view which currently has focus */
		void setActiveView(AnnotationView* view);
		bool langAndConventionsDlg(bool is_new_file, string& lang, string& conventions, string& forced_convention) ;
		void editSpeaker(string id, bool modal) ;

		void changeInterline() ;
		int getConfigurationHighlightMode() ;

		std::string prepareSavingExistingFile(string filename, bool& overwritten, bool& converted) ;
		std::string prepareSavingAsFile(int& copyMediaFile, bool& overwritten, bool& converted) ;
		bool processFileSaving(string filename, bool clean_speaker, int copyMediaFile, bool overwritten, bool converted) ;
		void actualizeMediaPathHint(string newDirectory) ;
		void cleanPathHint() ;
		std::string copyMediaFiles(string newAudioDir) ;
		void changeTrackCursor(int notrack, bool move) ;
		void hideInvalidSegmentTracks() ;
	/* private members */
	private:

		/*========================== Architecture ============================*/

		Gtk::Box*	m_textbox;	/** textview pack box */
		vector<AnnotationView*> m_textView; /** editor text views */
		AnnotationView* m_activeView;  /** current active view */
		Gtk::Statusbar* m_statusBar;

		/*=============================== Data ===============================*/
		Parameters	globalOptions;
		InputLanguage* m_ilang;
		ResultSet* resultSet ;

		/*======================= Global modes & status =======================*/

		bool disableThreadProtection ;
		bool consultationMode ;
		bool pathHintActualized ;

		int reportLevel ;
		bool externalIME_on ;
		/* used for finalize display only when all tracks are OK */
		int m_isActivatedTrack0 ;
		int m_isActivatedTrack1 ;

		bool firstFocus ;

		/*======================= Editor options status =======================*/
		/* view mode for transcription :
		 * -2: both views side by side
		 * -1: merged view
		 *  0: track 1 (indice 0)
		 *  1: track 2 (indice 1)
		 */
		int m_activeViewMode ;
		int m_defaultViewMode ;
		int m_lastTrack ;
		int m_activeTrack ;
		bool m_modeStereo ;
		/*
		 * -1: no highlight
		 * 0 : highlight track 1 (indice 0)
		 * 1 : highlight track 2 (indice 1)
		 * 2 : highlight both track
		 * 3 : highlight selected track
		 */
		int m_highlightMode ;

		/*
		 * -1: tag displayed
		 * 1 : hide all qualifiers
		 * 2 : hide events
		 * 3 : hide entities
		 * 4 : hide unknown qualifiers
		 */
		int m_hiddenTagsMode ;
		int m_lastModeWhenHidden ;
		string m_lastEditMode ;
		int m_tagHidden_lastInterlineAbove ;
		int m_tagHidden_lastInterlineBelow ;
		bool m_selectTrack_wt_cursor ;

		/*======================= Editor loading status =======================*/

		bool logLoadingDone ;

		bool loadingPeaksReady ;
		bool loadingTracksReady ;
		bool loadingViewReady ;
		Glib::Mutex loadingTracksMutex ;

		int m_nbLoadingTracks ;
		int m_totalLoadingTracks ;
		int m_loadingView ;
		void onPeaksReady(bool success) ;
		void onTracksReady() ;
		void onViewReady(int notrack) ;

		/*========================= Audio - Signal ===========================*/
		std::map<std::string, int> m_colorCode; /** for segments alternance in signal view*/
		std::vector<std::string> m_synchroTypes; /** annotation types for which synchroTextToSignal is to be performed */
		bool m_inhibateSynchro;   /** to inhibate synchro during some updates */
		Glib::Timer	m_TTSTimer;	/** timer for text to signal synchro events */
		Glib::Timer	m_elapsed;	/** timer for some performance measurements */
		float m_synchroInterval;  /** max interval between 2 synchro events to force synchro, event if synchro mode is user-deactivated */
		AnnotationView* m_syncSource;		/** widget which caused the synchro event */

		std::vector<std::string> m_trackTypes ; /** annotations to be displayed in configuration **/
		std::vector<std::string> m_visibleTracks ; /** annotations to be displayed in configuration **/
		std::vector<std::string> m_modelTypes ;	  /** annotations available in models **/
		std::vector<SegmentTrackWidget*> m_toHideTmp ;

		/*============================= Video ================================*/
		VideoWidget* videoPlayer ;
		FrameBrowser* videoFrameBrowser ;
		VideoManager* videoManager ;
		bool synchroLock ;

		ProgressionWatcher* progressWatcher ;

		/*============================ Signals ===============================*/

		sigc::signal<void> m_signalStreamingError ;
		sigc::signal<void, InputLanguage*> m_signalLanguageChange;
		sigc::signal<void, string, Glib::RefPtr<Gtk::ActionGroup>& > m_signalUpdateUI ;
		sigc::signal<void, string, bool>  m_signalEditSpeaker ;
		sigc::signal<void> m_signalChangeActiveView ;
		sigc::signal<void, string> m_signalEditModeChanged ;
		sigc::signal<void, int> m_signalTagHiddenChanged ;
		sigc::signal<void, int> m_signalHighlightChanged ;
		sigc::signal<void, string, string> m_signalSynchroChanged ;
		sigc::signal<void, int> m_signalDisplayChanged ;
		sigc::signal<void,const Glib::RefPtr<Gdk::DragContext>&,AGEditor*,string >  m_signalGtkDragTarget;

		Glib::RefPtr<Gtk::ActionGroup> emptyGroup;

		// A METTRE DANS LABELTRACK PLUS TARD
		bool showLabelTracksAfterIdle(int notrack);

		/* list of targetEntry to define drag and source identity */
		std::vector<Gtk::TargetEntry> dragDropTargets ;

		/* signal delay */
		void onSignalDelayChanged(int notrack, float delay) ;
		std::map<int,float> mapTrackDelays ;
};

} /* namespace tag */

#endif  /* _HAVE_ANNOTATION_EDITOR_H */
