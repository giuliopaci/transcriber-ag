/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/**
 *  @file 	AnnotationView.h
 */

#ifndef _HAVE_ANNOTATION_VIEW_H
#define _HAVE_ANNOTATION_VIEW_H 1

#include <map>
#include <stack>
#include <gtkmm.h>
#include <time.h>

/* SPELL */
//extern "C" {
//	typedef struct _GtkSpell GtkSpell;
//}

#include "DataModel/UndoableDataModel.h"

#include "Editors/AnnotationEditor/AnnotationBuffer.h"
#include "Editors/AnnotationEditor/UndoableTextView.h"
#include "Editors/AnnotationEditor/AnnotationViewTooltip.h"
#include "Editors/AnnotationEditor/menus/AnnotationMenu.h"

namespace tag {

class AnnotationEditor;
class AnnotationRenderer;

/**
*  @class 		AnnotationView
*  @ingroup 	AnnotationEditor
*
*  Represents the view used by the AnnotationEditor.\n
*  Uses an AnnotationBuffer for text model.\n\n
*
*  This class uses UndoableTextView inherited members for all undo/redo mechanisms.
*/
class AnnotationView : public UndoableTextView
{

	public:

		/**
		 * Constructor
		 * @param editor	Editor embedding the view
		 */
		AnnotationView (AnnotationEditor& editor);

		/**
		 * Constructor
		 * @param editor	Editor embedding the view
		 * @param buffer	Buffer to be embedded in the view
		 */
		AnnotationView (AnnotationEditor& editor, Glib::RefPtr<AnnotationBuffer>& buffer);

		/**
		 * Destructor
		 * @return
		 */
		~AnnotationView();

	   /**
		* Initializes the view component.
		* @param editable  		States if the view is in editable or read-only mode
		*/
		void initView(bool editable=true);

		/**
		 * Full text component configuration (display and content)
		 * @param master_dic		Speller dictionary path (UNUSED in current version)
		 * @param lang_iso639_3		Locale language used
		 * @param editmode			"EditMode" for edition, "BrowseMode" for consultation
		 * @param is_stereo			True for dual display, false otherwise
		 */
		void configure(string master_dic="", string lang_iso639_3="eng", const string& editmode="EditMode", bool is_stereo=false);

		/**
		 * Fast text component configuration (display only)
		 * @param editmode			"EditMode" for edition, "BrowseMode" for consultation
		 * @param is_stereo			True for dual display, false otherwise
		 */
		void configureDisplay(const string& editmode="EditMode", bool is_stereo=false);

/* SPELL */
//		/**
//		 * Configures text component speller
//		 * @param master_dic		Speller dictionary path
//		 * @param lang_iso639_3		Locale language used
//		 */
//		void configureSpeller(string master_dic="", string lang_iso639_3="eng") ;

		/**
		* Accessor to the editor that embeds the view
		* @return 	Reference on the AnnotationEditor parent
		*/
		AnnotationEditor& getParent() { return m_parent; }

		/**
		* Accessor to the model associated to the view
		* @return 	Reference on the used DataModel
		*/
		UndoableDataModel& getDataModel() { return m_dataModel; }

		/**
		 * Accessor to the speaker dictionary used by editor
		 * @return	Reference on the SpeakerDictionary in use.
		 */
		SpeakerDictionary& getSpeakerDictionary() { return m_dataModel.getSpeakerDictionary(); }

		/**
		* Updates the view by reloading data from model
		* @param type				Annotation element type to update (if empty, updates all types)
		* @param id					Annotation element id to update (if empty, updates all elements)
		* @param upd				Update type
		* @param fromSignal			True if called from signal handler, false otherwise
		* @param ignoreUpdTrack		If true, no signal is send to signal track widget
		*/
		void updateView(const string& type="", string id="", DataModel::UpdateType upd=DataModel::UPDATED, bool fromSignal=false, bool ignoreUpdTrack=false);

		/**
		 * Sets the buffer that will be embedded in the view
		 * @param buffer
		 */
		void setBuffer(Glib::RefPtr<AnnotationBuffer>& buffer);

		/**
		* Accessor to the embedded buffer
		* @return 			Pointer on current annotation buffer
		*/
		Glib::RefPtr<AnnotationBuffer> getBuffer();

		/**
		 * Gets menu items for editor (can be added to host application UI manager)
		 * @param groupname		Group name (available: file / edit / annotate / signal / display)
		 * @return				Pointer on the corresponding ActionGroup
		 */
		const Glib::RefPtr<Gtk::ActionGroup>& getActionGroup(string groupname) ;

		/**
		 * @return 	True if buffer has been modified since the last save action
		 */
		gboolean get_modified() { return getBuffer()->get_modified(); }

		/**
		 * Set the buffer modified status.
		 * @param b		True for modified state, False for saved sate
		 */
		void set_modified(bool b) { getBuffer()->set_modified(b); }

		/**
		 * Highlights the given element with the highlight tag.
		 * @param type				Element type
		 * @param id				Element id
		 * @param with_scroll		True for scrolling at element position
		 * @param track				Track the element corresponding to identifer <em>id</em> belongs to.
		 */
		void setHighlight(const std::string& type, const std::string& id, bool with_scroll, int track);

		/**
		 * Applies the given id tag to the given range.
		 * @param tagname			Tag name
		 * @param type				Element type
		 * @param id				Element id
		 * @param with_scroll		True for scrolling at element position
		 * @param track				Track the element corresponding to identifer <em>id</em> belongs to.
		 * @param applied_start		Range start
		 * @param applied_stop		Range end
		 */
		void setTag(const string& tagname, const string& type, const string& id, bool with_scroll, int track, Gtk::TextIter& applied_start , Gtk::TextIter& applied_stop) ;

		/**
		 * Clears highlight tag for the given type
		 * @param type		Element type
		 * @param track		Track impacted
		 */
		void clearHighlight(std::string type="", int track=3) { getBuffer()->clearHighlight(type,track); }

		/**
		 * Removes all occurrences of the given tag
		 * @param tagname		Tag whose occurrences should be removed
		 */
		void clearTag(std::string tagname) { getBuffer()->clearTag(tagname); }

		/**
		* Sets the cursor position to the given anchored element
		* @param id 			Element id
		* @param at_end  		True if at element end, else False
		* @param inhib_sync 	True to inhibate "emitSetCursor" signal, False otherwise
		*/
		void setCursor(const std::string& id, bool at_end=true, bool inhib_sync=true);

		/**
		 * set default text cursor positionning rule when moving to anchor position
		 * @param b	 if true cursor set at anchored element end, else set at anchored element start
		 */
		void setPlaceCursorAtEnd(bool b) { m_placeCursorAtEnd = b; }

		/**
		 * Moves cursor to the next or previous position
		 * @param forward		If true move in forward direction, else move backward
		 * @param downward		If true move in downward direction, else move upward
		 * @return				False (for event mechanism)
		 * @attention			Has to be called from signal_idle
		 */
		bool adjustCursorWhenIdle(bool forward = true, bool downward=true);

		/**
		 * Callback for all edition action launched
		 * @param action			Action name
		 * @param notrack 			Active signal track
		 * @param cursor 			Current signal cursor position
		 * @param end_cursor 		Selection end cursor if signal selection active, else 0.0
		 * @param on_selection		True if selection is set in text
		 * @param ignoreTime		If set to true, the time information will not be used (creation from text cursor only)
		 * @param hint				Speaker hint
		 */
		void onAnnotateAction(string action, int notrack, float cursor, float end_cursor, bool on_selection, bool ignoreTime, string hint);

		/**
		 * @return		The last user edit time
		 */
		time_t lastEditTime() { return m_lastEditTime; }

		/**
 		 * Sets selected speaker name at cursor position
		 * @param iter position of speaker-tagged text to replace
		 * @param id selected speaker id
		 * @param label selected speaker label
		 * @param selectionStart
		 * @param selectionEnd
		 */
		void setSpeaker(const Gtk::TextIter& iter, std::string id, std::string label, float selectionStart, float selectionEnd);

		/**
		 * Callback for speaker properties edition
		 * @param iter		Text position from which the event was launched
		 * @return			False (event mechanism)
		 * @attention		Has to be called from signal_idle
		 */
		bool editSpeakerPropertiesWhenIdle(Gtk::TextIter iter) ;

		/**
		 * Edits speaker properties for speaker used at given text position
		 * @param iter 		Text position of label for speaker to edit
		 * @note 			Emits <em>signalEditSpeaker</em> to request speaker properties edition
		 */
		void editSpeakerProperties(const Gtk::TextIter& iter);

		/**
		 * CALLED WITHIN DIALOG TURN PROPERTIES
		 * Edits speaker properties for given speaker id
		 * @param spkid 		Id of speaker to edit
		 * @note 				Emits signalEditSpeaker to request speaker properties edition
		 * @remarks				Called within dialog turn properties
		 */
		void editSpeakerProperties2(string spkid) ;

		/**
		 * Edits current annotation properties
		 * @param iter 		Annotation location in text buffer
		 * @param type 		Annotation type
		 * @note 			According to annotation type, opens appropriate properties dialog box
		 */
		void editAnnotationProperties(const Gtk::TextIter& iter, string type) ;

		/**
		 * Edits annotation properties of the closest given type to current cursor position
		 * @param type		Annotation type
		 */
		void editAnnotationPropertiesAtCursor(string type) ;

		/**
		 * Edits annotation properties of the closest given type to current cursor position
		 * @param iter		Text position
		 * @param type		Annotation type
		 * @return			False (event mechanism)
		 * @attention		Has to be called from signal_idle
		 */
		bool editAnnotationPropertiesWhenIdle(Gtk::TextIter iter, string type) ;

		/**
		 * Deletes the annotation located at or near the given position position
		 * @param iter 		Text buffer iterator
		 * @param type 		Annotation type
		 *
		 * @note Browses back in buffer from iterator position to find annotation to delete.\n
		 *       Checks that data model deletion rules are fulfilled, and does deletion.
		 */
		void deleteAnnotation(const Gtk::TextIter& iter, string type) ;

		/**
		 * Deletes the annotation located at or near the cursor position
		 * @param type 		Annotation type
		 *
		 * @note Browses back in buffer from iterator position to find annotation to delete.\n
		 *       Checks that data model deletion rules are fulfilled, and does deletion.
		 */
		void deleteAnnotationAtCursor(string type) ;

		/**
		* Sets section type at cursor position  position
		* @param iter 	Position of event-tagged text to replace
		* @param type 	Selected event type
		* @param desc 	Selected event description
		*/
		void setSection(const Gtk::TextIter& iter, std::string type, std::string desc);

		/**
		 * Sets qualifier type at cursor position position.
		 * @param iter 			Position of event-tagged text to replace
		 * @param type 			Selected event type
		 * @param desc 			Selected event desc
		 * @param qual_class	Qualifier class
		 */
		void setQualifier(const Gtk::TextIter& iter, std::string type, std::string desc, string qual_class);

		/**
		 * Creates a foreground event
		 * @param iter		Buffer position
		 * @param type		Event type
		 * @param subtype	Event subtype
		 * @param start		Start time (if existing, -1 otherwise)
		 * @param end		End time (if existing, -1 otherwise)
		 */
		void setForegroundEvent(const Gtk::TextIter& iter, std::string type, std::string subtype, float start, float end) ;

		/**
		 * Edit the qualifier at the given position
		 * @param iter			Text position
		 * @param qual_class	Qualifier class
		 */
		void editQualifier(const Gtk::TextIter& iter, string& qual_class);

		/**
		 * Defines the associaed signal track
		 * @param notrack		Track number
		 */
		void setViewTrack(int notrack) { m_viewTrack = notrack; }

		/**
		 * Accessor to the associated view track
		 * @return		Active view track number
		 */
		int getViewTrack() { return m_viewTrack; }

		/**
		 * Sets the active view track.
		 * @param notrack		Track number
		 */
		void setCurrentTrack(int notrack) { m_signalTrack = notrack; }

		/**
		 * Gets the text associated to segmentation.
		 * @param type 		Segment type
		 * @param id 		Segment id
		 * @param end_id 	Segment end id
		 * @returns			Section label, turn speaker or the transcription text (depending on <em>type</em>)
		 */
		Glib::ustring getSegmentText(const std::string& type, const std::string& id, const std::string& end_id);

		/**
		 * Store any pending edit in data model
		 * @return			False (event mechanism)
		 * @attention		May be called from signal_idle
		 */
		bool storePendingTextEditsWhenIdle();

		/**
		 * If any updates have been performed in the text buffer, stores them in the model
		 * @param isUndo	True if resulting from undo/redo action
		 * @param force		True for forcing the model update even if no updates were listed
		 * @return
		 */
		bool storePendingTextEdits(bool isUndo=false, bool force=false);

		/**
		 * Updates text annotation in data model
		 * @param id 				Id of text annotation to update
		 * @param emit_signal 		If true, signalElementModified will be emitted by data model
		 * @note 					Current text annotation value is retrieved from associated text buffer.
		 */
		void updateDataModel(const string& id, bool emit_signal=true);

		/**
		 * Gets configuration map: gives a read access to the view configuration
		 * @return			Reference on the view configuration map
		 */
		const std::map<std::string, std::string>& getConfiguration() { return m_configuration; }

		/**
		 * Gets the configuration value of the option corresponding to the given key.
		 * @param key		Option key
		 * @return			Option value
		 */
		const std::string& getConfigurationOption(const std::string& key) { return m_configuration[key]; }

		/**
		 * Gets display configuration map: gives a read access to the view display configuration
		 * @return			Reference on the view display configuration map
		 */
		const std::map<std::string, std::string>& getColorsCfg() { return m_colorsCfg ; }

		/**
		 * Gets the display configuration value of the display option corresponding to the given key.
		 * @param key		Display option key
		 * @return			Display option value
		 */
		const std::string& getColorsCfgOption(const std::string& key) { return m_colorsCfg[key]; }

/* SPELL */
//		/**
//		 * Detaches speller associated to current annotation view
//		 */
//		void detachSpeller();

		/**
		 * Scrolls view to the given text offset when idle.
		 * @param text_offset		Text offset
		 * @return					False (for event mechanism)
		 * @attention				Should be called from signal idle
		 */
		bool scrollViewWhenIdle(guint32 text_offset);

		/**
		 * Scrolls view to the given text offset
		 * @param text_offset		Text offset
		 * @return					False (for event mechanism)
		 */
		bool scrollView(guint32 text_offset = (guint32)-1);

		/**
		 * Scrolls view to the given text anchor
		 * @param id		Text anchor id
		 * @return					False (for event mechanism)
		 */
		bool scrollViewToId(const string& id);

		/**
		 * Scrolls view to the given text iter
		 * @param it		Text iter
		 * @return					False (for event mechanism)
		 */
		bool scrollViewToIter(const Gtk::TextIter& it);

		/**
		 * Pastes current selection clipboard contents in buffer if valid insertion position
		 * @param text				Selection clipboard contents
		 * @param special_paste		True for special paste (graph paste)
		 */
		void onSelectionPasteEvent(const Glib::ustring& text, bool special_paste=false);

		/**
		 * Pastes string in buffer at current insert position
		 * @param text		Text to be insterd
		 */
		void pasteSelectedText(const Glib::ustring& text);

		/**
		 * Adds un-stored pending edtis in queue.
		 * @param pos			Text position
		 * @param nbbytes		Number of bytes modified
		 * @note				When the pending edits value is reached, emit signal
		 * 						for calling AnnotationView::storePendingTextEditsWhenIdle()
		 */
		void setHasPendingEdits(const Gtk::TextIter& pos, int nbbytes=0);

		/**
		 * Changes the editor cursor for displaying the wait icon
		 * @param b			True for switching to wait state, False for leaving the wait state
		 */
		void setWaitCursor(bool b) ;

		/**
		 * Modifies the autoset language mode
		 * @param b				New autoset language mode
		 */
		void setAutosetLanguage(bool b) { m_autosetLanguage=b; }

		/**
		 * Activates (raises and uses) or not the external IME (if some is used)
		 * @param activate		True for activating the external IME, False otherwise.
		 * @see					externalIMEcontrol_afterIdle(bool)
		 */
		void externalIMEcontrol(bool activate) ;

		/**
		 * Activates (raises and uses) or not the external IME (if some is used)
		 * @param activate		True for activating the external IME, False otherwise.
		 * @return				False (event mechanism)
		 * @attention			Should be called by signal idle
		 */
		bool externalIMEcontrol_afterIdle(bool activate) ;

		/**
		 * Sets the IME configuration state
		 * @param value			True for enabling IME, false otherwise
		 */
		void setIMEstatus(bool value) { IME_on=value ; }

		/**
		 * Set focus to view (grab_focus accessor)
		 * @param protectSignal		True for block on_focus_in signal callback, false for classic behaviour
		 */
		void setFocus(bool protectSignal) ;

/* SPELL */
//		/**
//		 * Forces speller to check all buffer.
//		 */
//		void speller_recheck_all() ;

		/**
		 * Defines whether qualifiers label corresponding to nammed entities are displayed with a
		 * background color.
		 * @param use_bg		True for displaying tag background, False otherwise
		 */
		void set_entityTag_bg(bool use_bg) ;

		/**
		 * Defines the AnnotationBuffer font style
		 * @param font		Font name to set
		 * @param mode		"text" for setting normal text font, "label" for label font
		 * @note			When used in "text" mode, this will impact qualifiers too
		 */
		void setFontStyle(const string& font, const string& mode) ;

		/**
		 * Resets text foreground and background color
		 */
		void resetColors() ;

		/**
		 * Sets text color.
		 * @param fg	Foreground color
		 * @param bg	Background color
		 */
		void setTextColor(string fg, string bg) ;

		/**
		 * Gets the current view base color.
		 * @return		String representation of the current color, or white value if recuperation failed.
		 */
		Glib::ustring getBaseViewColor() ;

		/**
		 * Sets the tooltip mode.
		 * @param value		True for allowing tooltip in the text zone, False otherwise
		 */
		void setUseTooltip(bool value) { tooltip_enabled = value ; }

	#ifdef _USE_RENDERER
		bool needsNewlineBefore(const string& type) { return m_renderer[type].newlineBefore(); }
		bool needsNewlineAfter(const string& type) { return m_renderer[type].newlineAfter(); }
	#endif

		/**
		 * Inhibates signal/text and text/signal synchronization
		 * @param b		True for locking, False for unlocking synchronization
		 */
		void inhibateSynchro(bool b=true) { m_inhibateSynchro = b; }

		/**
		 * Inhibates pending edit mechanism
		 * @param b		True for locking, False for unlocking
		 */
		void inhibateStoreEdits(bool b=true) ;

/* SPELL */
//		/**
//		 * Inhibates speller mechanism
//		 * @param b				True for locking, False for unlocking
//		 * @param recheck		If set to true, the speller rechecks all buffer
//		 */
//		void inhibateSpellChecking(bool b=true, bool recheck=false) ;

		/**
		 * Checks the stereo mode
		 * @return		True if stereo, False otherwise
		 */
		bool isStereo() ;

		/**
		 * Callback for preedit start signal
		 */
		void on_preedit_start();

		/**
		 * Callback for preedit end signal
		 */
		void on_preedit_end();

		/**
		 * Callback for preedit changed signal
		 */
		void on_preedit_changed();

		/**
		 * Accessor to all renderers used in the buffer
		 * @return		Reference to the renderers map
		 */
		const std::map<std::string, AnnotationRenderer*>& getRendererMap() { return m_renderer ; }

		/**
		 * Checks if the given key value represents a function key
		 * @param keyval		Keyval code
		 * @return				True if the given keyval corresponds to a function key,
		 * 						False otherwise
		 */
		static bool isFunctionKey(guint32 keyval) ;

		/**
		 * Checks if the given event corresponds to an common accel key one
		 * @param event		Pointer on GdkEventKey
		 * @return			True or False (obvious)
		 */
		static bool isAccelKeyEvent(GdkEventKey* event) ;

		/**
		 * Presentation method.\n
		 * Displays or hide all qualifier tags that gets the AnnotationBuffer::CAN_BE_HIDDEN_TAG property.
		 * @param hide		Hide mode
		 * @see 			AnnotationEditor::setHiddenTagsMode(int) for hide mode
		 */
		void hideTags(int hide) ;

		/**
		 * Returns inter line value
		 * @param[in] above		Value above lines
		 * @param[in] below		Value below lines
		 */
		void get_pixels_interline(int& above, int& below) ;

		/**
		 * Sets the inter line value
		 * @param above		Value above lines
		 * @param below		Value below lines
		 */
		void set_pixels_interline(int above, int below) ;

		/**
		 * Enable or Disable thread protection mechanism
		 * @param 		disable		True for disable, False for allowing
		 * @attention				Mostly you don't need to use this method\n
		 * 							Use it with <em>disable</em> set to True when using a method that
		 * 							will call the AnnotationEditor::changeActiveViewMode method
		 * 							without a thread mechanism.
		 */
		void disableThread(bool disable)  { disable_thread = disable ; }

		/**
		 * Sets the current Gdk cursor and displays it if needed
		 * @param textCursor	Gdk Cursor
		 * @param apply			Change display if set to true
		 */
		void setTextCursor(Gdk::Cursor textCursor, bool apply=false)
		{
			m_textCursor = textCursor;
			if (apply)
				get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(m_textCursor);
		}

		/**
		 * Set view editability
		 * @param editable which is view editability
		 */
		void setEditable(bool editable) { m_editable = editable; }

		/**
		 * Accessor to view editability
		 * @return	True if editable, false otherwise
		 */
		bool isEditable() const { return m_editable; }

		/**
		 * Defines whether confidence highlight on scored word is enabled
		 * @param value					If true, scored words will be displayed in bold characters.
		 * @param actualizeSpeller		True for actualize confidence speller state (UNUSED)
		 */
		void setWithConfidence(bool value, bool actualizeSpeller=true) ;

		/**
		 * Accessor to confidence mode status
		 * @return		True if confidence is used, false otherwise
		 */
		bool getWithConfidence() const ;

/* SPELL */
//		/**
//		 * Accessor to speller
//		 * @return	GtkSpell pointer
//		 */
//		GtkSpell* getSpeller() { return m_speller; }

		/**
		 * Accessor to main graph type used by editor
		 * @return	Graph type used by editor
		 */
		const string& getGraphType() { return m_mainGraphType; }

		/**
		 * Sets the drag and drop target
		 * @param targetList	Target list
		 */
		void addDragAndDropTarget(Gtk::TargetEntry targetList) ;

		/**
		 * Accessor to transcription language of the current file
		 * @return	transcription language as cod ISO_8859_2
		 */
		string getTranscriptionLanguage() const { return m_lang ; }

		/**
		 * Return the name of the tag associated to the given element.
		 * @param id		Annotation id
		 * @param type		Annotation type
		 * @return			The name of the renderer tag
		 */
		const string& getRendererTagname(const string& id, const string& type) ;

		/**
		 * Remove timestamp information
		 * @param iter		Buffer iterator
		 * @param type		Annotation type
		 */
		void removeTimestamp(const Gtk::TextIter& iter, string type) ;

		/**
		 * Get annotation element at current cursor position
		 * @return			Annotation element
		 */
		const string& getTextCursorElement() ;

        /**
		 * Signal emitted at cursor change\n
		 * <b>const Gtk::TextIter& parameter:</b>  	New cursor position\n
		 * <b>bool parameter:</b>					Force synchronization\n
		 */
		sigc::signal<void, const Gtk::TextIter&, bool>& signalSetCursor() { return m_signalSetCursor ; }

		/**
		 * Signal emitted when receiving focus\n
		 */
		sigc::signal<void>& signalHasFocus() { return m_signalHasFocus ; }

		/**
		 * Signal indicating that drag target was received on a speaker tag.
		 * <b>Glib::RefPtr<Gdk::DragContext>& parameter:</b>	Drag'n'drop context\n
		 * <b>string parameter:</b>	Id of speaker to be replaced\n
		 */
		sigc::signal<void,const Glib::RefPtr<Gdk::DragContext>&, string>& signalGtkDragTarget() { return m_signalGtkDragTarget; }

		/**
		 * Signal emitted when an element has been modified.\n
		 * <b>string parameter:</b>			Element type\n
		 * <b>string parameter:</b>			Element id\n
		 * <b>int	  parameter:</b>		Update action type\n
		 * <b>bool	  parameter:</b>		True if text only was updated\n
		 * <b>int	  parameter:</b>		Active view track\n
		 * <b>bool	  parameter:</b>		Show element tracks or just display tracks\n
		 */
		sigc::signal<void, string, string, int, bool, int, bool>& signalElementModified() { return m_signalElementModified; }

		/**
		 * Signal emitted when a tag event on speaker label is received.\n
		 * <b>std::string parameter:</b> 	Speaker id\n
		 * <b>bool parameter:</b> 			Whether the speaker dialog has to be launched in modal mode\n
		 */
		sigc::signal<void, string, bool>& signalEditSpeaker() { return  m_signalEditSpeaker; }

		/**
		 * Signal emitted when a tag event on speaker label is receive.\n
		 * <b>int parameter:</b> 			Direction change (-1 for previous, 1 for next)\n
		 */
		sigc::signal<void, int>& signalLanguageChange() { return m_signalLanguageChange;}

		/**
		 * Debug method
		 */
		bool isDebugMode() ;

		/**
		 * Update alignement display for given annotation
		 * @param id	Annotation id
		 */
		void updateAlignment(std::string id) ;

		/**
		 * @return pointer on top window for current view
		 */
		Gtk::Window& getTopWindow();

//		void lockInput(bool value) { m_lockInput = value ; }
	private:

		std::string getCurrentTaggedElement(const Gtk::TextIter& iter, const string& type) ;
		void createSpeakerElement(const Gtk::TextIter& iter, const string& spkId, bool overlap, float selectionStart, float selectionEnd) ;
		void modifySpeakerElement(const string& id, const string& spkId, const string& label) ;

		void createForegroundEvent(const Gtk::TextIter& iter, std::string type, std::string subtype, float start, float end) ;

		bool m_editable;
		void createAnnotationRenderers();
		AnnotationRenderer* getRenderer(const string& type) ;
		AnnotationMenu* getRendererMenu(const string& type, const string& hint="", bool edit_mode=true) ;

		/** easy access **/
		// popup annotation-specific menu; returns true if menu popped for given
		//   annotation type
		// onElement: true if popup from existing element, false if popup for creating element
		bool popupAnnotationMenu(const string& type, const Gtk::TextIter& textIter, GdkEvent* event, float start, float end, const string& hint="", bool force=false) ;

		void emitSetCursor(const Gtk::TextIter& iter, bool force);
		void setCurrentInputLanguage(const Gtk::TextIter& iter, bool force=false);
		void setCurrentInputLanguage(const string& iter, bool force=false);

		void displayUndoRedoError(bool undo) ;

		/**
		* override textview default behaviour for key_press_event
		* @param event GdkEventKey event
		* @return boolean true if action handled by current handler, else false.
		*/
		virtual bool on_key_press_event(GdkEventKey* event);

		/* specific key press treatments - called by on_key_press_event */
		bool processKeyClassic(GdkEventKey *event);
		bool processKeyDeletion(GdkEventKey* event, bool& ret) ;
		bool processKeyFunc(GdkEventKey* event, bool& ret) ;

		/**
		 * Checks whether the non-editable element at current iter can be erased or not after key event
		 * @param iter			Iterator position
		 * @param event			Pointer on key press event
		 * @param tagclass		Class of element
		 * @return				True if deletion is allowed, false otherwise
		 */
		bool canDeleteAtIter(const Gtk::TextIter& iter, GdkEventKey* event, const string& tagclass) ;

		/**
		*	Check insertion keys
		*	@param keyval 			Keyval beeing inserted by current KeyEvent
		*	@param iter  			Current text iterator
		*	@param spaceHandling 	True if space insertion control is activated
		*	@param spaceBordering	True if space insertion is activated
		*	@return 				True if insertion is allowed, false otherwise
		*/
		bool insertionKeySpaceFilter(gunichar keyval, const Gtk::TextIter& iter, bool spaceHandling, bool spaceBordering) ;

		/**
		*	Check in eventKey for special char in convention to add space borders
		*	if it is needed
		*	@param event GdkEventKey
		*	@param iter  current text iterator
		*	@return modified string if spaces have been inserted, empty string if
		*			no modifications needed
		*/
		Glib::ustring insertionKeySpaceBorder(GdkEventKey* event, const Gtk::TextIter& iter) ;

		void deleteSelection(const Gtk::TextIter& start, const Gtk::TextIter& stop, bool keep_end_anchor=false, bool with_anchored=false);
		string getSelectedData();

		/**
		* override textview default behaviour for button_press_event
		* @param event GdkEventButton event
		* @return boolean true if action handled by current handler, else false.
		*/
		virtual bool on_button_press_event(GdkEventButton* event);

		void onBufferTagEvent(std::string tagclass, GdkEvent* event, const Gtk::TextIter& iter);
		bool bufferTagEventHandler(const std::string& tagclass, GdkEvent* event, const Gtk::TextIter& iter);
		void onBufferAnchorMoveEvent(float old_pos,
										const Gtk::TextBuffer::iterator &pos,
										const Glib::RefPtr<Gtk::TextMark>& mark,
										bool forceDisplayUpdate) ;

		virtual bool on_focus_in_event(GdkEventFocus* event);
		virtual bool on_focus_out_event(GdkEventFocus* event);
		/**
		* override textview default behaviour for focus_in_event, focus_out_event,
		*   on_motion_notify_event, on_drag_drop
		* @param event GdkEvent* event
		* @return boolean true if action handled by current handler, else false.
		*/
		virtual bool on_motion_notify_event(GdkEventMotion* event);

		virtual bool on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& context,
				int x, int y, guint time);

		/* list of targetEntry to define drag and source identity */
		std::vector<Gtk::TargetEntry> dragDropTargets ;

		/* menu management */
		virtual void on_populate_popup(Gtk::Menu* menu);
		void fillAnnotationMenu(Gtk::Menu* menu, const Gtk::TextIter& iter, bool selectedText, bool only_events) ;
		void onPopupMenuPosition(int& x, int& y, bool& push_in) ;


		void onUndoableAction(const string& data, int notrack) ;
		void onUndoAllDone() ;
		void onNextInputLanguage();
		void onPreviousInputLanguage();

		bool isSignalSelectionAction() { return signalSelectionAction ; }
		void setSignalSelectionAction(bool value) { signalSelectionAction = value ;}

		void updateAfterUndoRedo(const string& type, const string& id, DataModel::UpdateType upd) ;
		void updateAfterAnchorAction(const string& id, int notrack, DataModel::UpdateType upd) ;
//TODEL		void prepareCursorForAnnotationInsertion(const string& id, const string& type, SignalSegment& s) ;
		void on_hide() ;
		void emitPopupSpeakerMenu(Gtk::TextIter& textIter, GdkEvent* event);
		void createAnnotationActionGroup();
		void onEditAction(string action, bool from_popup=false);

		void insertBackgroundSegment(const string& id, int mode, bool del);
		void insertBackgroundSegment(const string& id, int mode, const std::vector<string>& bgs) ;
		void deleteBackgroundSegment(const string& id, std::vector<string>& bgs) ;
		void insertBackground(const string& type, const string& id, bool is_starting=true) ;
		string splitTextAnnotation(const Gtk::TextIter& iter, bool reattach=true, bool dont_split_at_end=false);

		string getLabelForSegmentType(const string& type, const string& id, int* code=NULL, bool timecoded=true);

		void deleteBackground(const string& id, bool confirm) ;
		void editBackground(const string& qid) ;

		void onClipboardDataEvent(const Gtk::SelectionData& data);
		void onSelectionSet(bool has_sel);

		void updateBackgroundDisplay(const string& id, int mode, bool del) ;

		//***LG3
		static char* ptr_callback(const char* s, void* data) ;
		//***LG3
		void insertPredefinedWord(Gtk::TextIter iter, string word);

		void getIdAndOffsetFromIterator(Gtk::TextIter& pos, string& id, int& offset, bool basetype_only = true);

/* SPELL */
//		bool checkSpellerUsage(const std::string& lang_iso639_2) ;
/* SPELL */
//		const char* determineAspellLanguageCode(const std::string& lang_iso639_2) ;
		/**
		* render all annotations at a given mainstream type level for a given signal part
		* @param mainstream_types vector of mainstream types to render
		* @param itype index of current mainstream type level
		* @param parent parent signal segment
		*/
		void renderAll(const string& graphtype, const vector<string>& mainstream_types, int itype, const string& parent, int notrack=-1, bool r2l=false);
		void renderBaseElement(const string& graphtype, const string& type, const string& id,
													bool with_qualifiers, bool split_only,
													const string& parent, bool r2l) ;
		void renderQualifiers(const string& id, bool starting_at=true, bool dont_set_cursor=false, bool r2l=false);

		string getQualifierRenderer(const string& type, const string& subtype, const string& graphtype) ;

		bool getR2LMode(const string& id) ;

		string getSpeakerAtPosition(int win_x, int win_y) ;
		void configureItemVisibility(string graphetype) ;
		void setBuffer() ;

		void annotateTimestamp(const string& prevId, int text_offset, int notrack, float cursor, float end_cursor, bool on_selection, bool ignoreTime) ;
		void createTimestamp(const string& prevId, int notrack, int text_offset, float cursor, float end_cursor, bool on_selection, bool ignoreTime) ;
		void changeTimestamp(const string& currentId, float cursor, bool newTag) ;
		void setSqueezeNoSpeech(bool b) { m_squeezeNoSpeech = b; }
		void insertNewTextElement(const string& prevId);

	private:

		bool disable_thread ;

		AnnotationViewTooltip* m_tooltip ;
		bool tooltip_enabled ;

		AnnotationEditor& m_parent; /** parent AnnotationEditor */
		UndoableDataModel& m_dataModel; /** associated data model */
		std::map<std::string, std::string>& m_configuration; /**> configuration parameters */
		std::map<std::string, std::string>& m_colorsCfg; /**> configuration parameters */
/* SPELL */
//		GtkSpell* m_speller; /**< spell checker */
		string m_lang;		/** transcription language */

		bool m_spaceHandlingModified ;

		bool IME_on ;

		bool focus_in_locked ;

		int m_viewTrack; /**  signal track associated to view / -1 */
		int m_signalTrack; /**  current signal track (input mode) */
		string m_mainGraphType;	/** main graphtype displayed in view */
		string m_mainBaseType;	/** main base type for main graphtype displayed in view */
		string m_altGraphType;	/** alternate graphtype displayed in view (merged view) */
		string m_bgGraphType;	/** "background" graphtype displayed in view */

		bool m_autosetLanguage;  /**< set automatically input language when cursor placed on turn */

		sigc::signal<void, const Gtk::TextIter&, bool > m_signalSetCursor ;
		sigc::signal<void> m_signalHasFocus ;
		sigc::signal<void,const Glib::RefPtr<Gdk::DragContext>&, string >  m_signalGtkDragTarget;
		sigc::signal<void, string, string, int, bool, int, bool> m_signalElementModified;
		sigc::connection m_afterTimeout;
		//	sigc::connection m_afterIdle;
		string m_currentId;	/**< current turn id */


		time_t	m_lastEditTime;	/** last edit action time */
		guint32	m_lastEventTime;  /** for simple/double click filtering */

		int m_pendingTextEdits;   /** has un-stored pending edits */

		bool m_inhibateSynchro;   /** to inhibate synchro during text updates */
		bool m_withConfidence;		/** enhance high confidence text */

		Glib::RefPtr<Gtk::Clipboard>  m_refClipBoard;  /** connect to selection clipboard */
		Glib::RefPtr<Gtk::Clipboard>  m_refClipBoardSpecial;  /** connect to selection clipboard for special paste*/
		string m_clipboardContents;
		string m_cancelledPopupId;

		/* cursors for active labels */
		Gdk::Cursor m_activeCursor;
		Gdk::Cursor m_textCursor;
		Gdk::Cursor m_waitCursor;
		bool m_isActiveCursor;
		bool m_inhibPendingEdits;
		bool m_need_restore_edit_state;
		bool m_placeCursorAtEnd;

		bool signalSelectionAction ;
		bool m_overlapState ;
		bool m_squeezeNoSpeech;


		GThread* m_segThread;


		const Glib::RefPtr<Gtk::ActionGroup> emptyGroup;

		std::map<std::string, Glib::RefPtr<Gtk::Action> > m_editActions;
		std::map<std::string, AnnotationRenderer*>  m_renderer;

		// popup menus
//		std::map <string, AnnotationMenu*> m_contextMenus;  /* contextual annot menu */
		sigc::signal<void, string, bool>  m_signalEditSpeaker ;
		sigc::signal<void, int> m_signalLanguageChange ;
		int m_popup_x ; 		/**< x position for menu popup */
		int m_popup_y;  		/**< y position for menu popup */

		string m_novalue;


};


} /* namespace tag */

#endif  /* _HAVE_ANNOTATION_VIEW_H */
