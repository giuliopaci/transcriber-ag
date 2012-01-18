/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/**
* @file 	AnnotationBuffer.h
* @brief 	Specific annotation buffer
*/

#ifndef _HAVE_ANNOTATION_BUFFER_H
#define _HAVE_ANNOTATION_BUFFER_H


#include <map>
#include <iostream>
#include <gtkmm.h>

/* SPELL */
//extern "C" {
//#include <gtkspell/gtkspell.h>
//}

#include "Editors/AnnotationEditor/Anchors.h"
using namespace std;

/**
 * Operator for Gtk::TextIter print
 * @param out
 * @param iter
 * @return
 */
std::ostream& operator << (std::ostream& out, const Gtk::TextIter& iter);


namespace tag {

/**
 * @def		IS_TEXT_EMBEDDED
 * Tag property: indicates whether the tagged element has to get the same
 * presentation (font size mainly) as normal text editor
 */
#define IS_TEXT_EMBEDDED 0x1

/**
 * @def		IS_ACTIVE_TAG
 * Tag property: indicates whether the tag can receive tag event
 */
#define IS_ACTIVE_TAG 0x2

/**
 * @def		NEWLINE_BEFORE_TAG
 * Tag property: indicates whether the tagged element has to
 * be placed at line start
 */
#define NEWLINE_BEFORE_TAG 0x4

/**
 * @def		NEWLINE_AFTER_TAG
 * Tag property: indicates whether the tagged element has to
 * be followed by new line
 */
#define NEWLINE_AFTER_TAG 0x8

/**
 * @def		CAN_BE_HIDDEN_TAG
 * Tag property: indicates whether the tagged element can
 * be hidden
 */
#define CAN_BE_HIDDEN_TAG 0x10

/**
 * @def		INHIBATE_BACKGROUND_TAG
 * Tag property: indicates whether background style shall be applied for tag or not
 */
#define INHIBATE_BACKGROUND_TAG 0x20

/**
 * @def		DO_UNDERLINE_TAG
 * Tag property: indicates whether tag has underline property
 */
#define DO_UNDERLINE_TAG 0x40

class AnnotationView;

/**
* @class 	AnnotationBuffer
* @ingroup	AnnotationEditor
*
* This class represents the annotion buffer.
*
*/
class AnnotationBuffer : public Gtk::TextBuffer
{

	public:

		/*! constructor */
		AnnotationBuffer() ;
		/*! destructor */
		virtual ~AnnotationBuffer();

		/**
		 * Delete all buffer content
		 */
		void clearBuffer();

		/**
		 * Static AnnotationBuffer creator
		 * @return		Pointer the newly AnnotationBuffer created
		 */
		static Glib::RefPtr<AnnotationBuffer> create();

		/**
		 * Initialize text tags for current annotation conventions
		 * @param view			Associated AnnotationView
		 */
		void configure(AnnotationView* view);

		/**
		 * Sets the editable text type
		 * @param type		Editable text type
		 */
		void setUnitType(const std::string& type) { m_unitType=type; if ( m_segmentType.empty() ) m_segmentType=type; }
		/**
		 * Sets the segment type
		 * @param type		speech segment  type
		 */
		void setSegmentType(const std::string& type) { m_segmentType = type; ; if ( m_unitType.empty() ) m_unitType=type; }
		/**
		 * Sets the prefix of all identifier tag
		 * @param prefix	Prefix of all identifier tags
		 */
		void setIdTagPrefix(const std::string& prefix) { m_idTagPrefix = prefix;}

		/**
		 * Accessor to the tag prefix map
		 * @return			Tag prefix map
		 */
		map<string,string> getTagPrefixMap() { return m_tagPrefix ; }

		/**
		 * Checks if given text position tagged with given tagname
		 * @param curpos 			Text position
		 * @param tagname 			Tag name
		 * @param is_prefix 		True if tag name is prefix for a family of tags
		 * @param check_end_tag		If true the method only search for end tag
		 * @param nohighlight       If set to true, will not stop for highlight tag (false by default)
		 * @return 					RefPtr on tag if found, else 0
		 */
		Glib::RefPtr<Gtk::TextTag> iterHasTag(const Gtk::TextIter& curpos, const std::string& tagname, bool is_prefix=false, bool check_end_tag=false, bool noHighlight=false);

		/**
		 * Applies or removes the timestamp tag on the given element label and update anchor
		 * property
		 * @param id			Annotation id
		 * @param on			True for setting the tag, false for removing it
		 * @return				True for success, false for failure
		 */
		bool switchTimestamp(const string& id, bool on) ;

		/**
		 * Gets the tag prefix corresponding to the given type
		 * @param type		Type of annotation tag
		 * @return			The corresponding prefix
		 */
		const std::string& getTagPrefix(const std::string& type) { if ( m_tagPrefix[type].empty() ) return type; else return m_tagPrefix[type]; }

		/**
		 * Highlights text segment of given type starting at or near given position
		 * @param type 		Segment type
		 * @param start		Segment approximate start position
		 * @param track		Number of the track
		 */
		void setHighlight(const std::string& type, Gtk::TextBuffer::iterator& start, int track);
		/**
		 * Highlights text segment for given id
		 * @param id 		Anchored element id
		 * @param track		Number of the track
		 */
		Gtk::TextIter setHighlight(const std::string& id, int track);

		/**
		 * Applies a tag in the given range
		 * @param tagname			Name of the tag to apply
		 * @param type				Annotation type
		 * @param pos				Position in text
		 * @param track				Track impacted
		 * @param applied_start		Range start
		 * @param applied_stop		Range end
		 */
		void setTag(const string& tagname, const string& type, Gtk::TextBuffer::iterator& pos, int track, Gtk::TextIter& applied_start , Gtk::TextIter& applied_stop) ;

		/**
		 * Removes highlight for any text segment of given type
		 * @param type 		Segment type
		 * @param track		Impacted track
		 */
		void clearHighlight(const std::string& type, int track);

		/**
		 * Removes all occurrences of the given tag
		 * @param tagname		Tag whose occurrences should be removed
		 */
		void clearTag(const string& tagname) ;

		/**
		 * Clears text buffer
		 */
		void clear();

		/**
		 *  Remove selection tag on selected text
		 */
		void clearSelection() { move_mark(get_selection_bound(), getCursor()); }

		/**
		 * get selected text
		 * @param iter (in) current iterator position
		 * @param start (OUT) selection start iterator
		 * @param stop  (OUT) selection stop iterator
		 * @param adjustToWords	if true, adjust selected range to word boundaries
		 * @return true if text selected, else false
		 */
		bool getSelectedRange(const Gtk::TextIter& iter, Gtk::TextIter& start, Gtk::TextIter& stop, bool adjustToWords=false);

		/**
		 * Checks whether the selection borders could make anchors superposition if the selection is deleted.
		 * @param start			Selection start
		 * @param end			Selection end
		 * @return				Vector of 2 elements (Pointer anchors of the two borders) if superposition can happen, empty vector otherwise
		 */
		std::vector<Anchor*> needBlankBeforeDeleteSelection(Gtk::TextIter start, Gtk::TextIter end) ;

		/**
		 *  Gets text buffer anchor set
		 */
		AnchorSet& anchors() { return m_anchors; }

		/**
		 * Signal emitted at cursor change\n
		 * <b>const Gtk::TextIter& parameter:</b> new cursor position
		 */
		sigc::signal<void, const Gtk::TextIter&>& signalSetCursor() { return m_signalSetCursor ; }

		/**
		 * Signal emitted when text edition has been proceeded
		 * <b>const Gtk::TextIter& parameter:</b> edition cursor position
		 */
		sigc::signal<void, const Gtk::TextIter&, int>& signalHasEdits() { return m_signalHasEdits ; }

		/**
		 * Signal emitted at selection change
		 * <b>boolean parameter:</b>	True when selection has been applied, False when it has been cleared
		 */
		sigc::signal<void, bool>& signalSelectionSet() { return m_signalSelectionSet; }

		/**
		 * Signal emitted when <em>signal_event</em> Gtk::TextTag signal is received
		 * <b>GdkEvent* parameter:</b>					Event associated to the Gtk::TextTag signal
		 * <b>const Gtk::TextIter& parameter:</b>		Text position of the impacted tag
		 */
		sigc::signal<void, std::string, GdkEvent*, const Gtk::TextIter& >& signalTagEvent() { return m_signalTagEvent; }


		/**
		 * Sets the cursor to given position. Enables to choose whether the cursor will be moved
		 * to an editable position only.
		 * @param pos				Text position
		 * @param to_edit_pos		If set to true, will be placed at the nearest
		 * 							editable position to the given position.
		 */
		void setCursor(const Gtk::TextIter& pos, bool to_edit_pos=true);

	   /**
		*  Set insert mark to editable position after given id
		*  @param id 		anchor id
		*  @param at_end 	if true place cursor at element end, else at element start
		*  @param force 	if true always moves cursor, else if cursor between element start and end leave it at its current place
		*/
		void setCursorAtAnchor(const string& id, bool at_end=true, bool force=false);

		/**
		 * Sets the cursor to given position defined by an offset from start of buffer
		 * @param offset	Offset in textbuffer
		 */
		void setCursor(const guint offset) { setCursor(get_iter_at_offset(offset), false); }

		/**
		 * Moves the cursor of N characters in the buffer
		 * @param nbchar		Number of deplacement characters
		 */
		void moveCursor(int nbchar);

		/**
		 * @return		The Gtk::TextIter corresponding to the cursor
		 */
		Gtk::TextIter getCursor() { return get_insert()->get_iter(); };

		/**
		 * Accessor to the next editable position.
		 * @param pos				Position from which we want to get the next editable position.
		 * @param endAnchor			Research limit. If the next editable computed position is placed
		 * 							beyond this limit, the returned value is the Iterator position of this anchor.
		 * @param interactive		True if called upon user action, else False
		 * @return					Iterator on the next editable position, or endAnchor position if set and reached.
		 */
		Gtk::TextIter nextEditablePosition(const Gtk::TextIter& pos, Anchor* endAnchor=NULL, bool interactive=false);

		/**
		 * Accessor to the next editable position.
		 * @param pos				Position from which we want to get the next editable position.
		 * @param endid				Research limit. If the next editable computed position is placed
		 * 							beyond this limit, the returned value is the Iterator position of
		 * 							the corresponding anchor.
		 * @param interactive		True if called upon user action, else False
		 * @return					Iterator on the next editable position, or endAnchor position if set and reached.
		 */
		Gtk::TextIter nextEditablePosition(const Gtk::TextIter& pos, const std::string& endid, bool interactive=false);

		/**
		 * Accessor to the previous editable position.
		 * @param pos				Position from which we want to get the previous editable position.
		 * @param afterAnchor		Research limit. If the previous editable computed position is placed
		 * 							beyond this limit, the returned value is the Iterator position of this anchor.
		 * @param interactive		True if called upon user action, else False
		 * @return					Iterator on the previous editable position, or endAnchor position if set and reached.
		 */
		Gtk::TextIter previousEditablePosition(const Gtk::TextIter& pos, Anchor* afterAnchor=NULL, bool interactive=false);

		/**
		 * Accessor to the previous editable position.
		 * @param pos				Position from which we want to get the previous editable position.
		 * @param afterId			Research limit. If the previous editable computed position is placed
		 * 							beyond this limit, the returned value is the Iterator position of
		 * 							the corresponding anchor.
		 * @param interactive		True if called upon user action, else False
		 * @return					Iterator on the previous editable position, or endAnchor position if set and reached.
		 */
		Gtk::TextIter previousEditablePosition(const Gtk::TextIter& pos, const std::string& afterId, bool interactive=false);

		/**
		 * Accessor to the next position, <b>excepted</b> space positions
		 * @param pos				Position from which we want to get the previous editable position.
		 * @param editable_only		Whether or not only considering the editable positions
		 * @return					Iterator on the next non-space position
		 */
		Gtk::TextIter nextNonSpacePosition(const Gtk::TextIter& pos, bool editable_only=false);

		/**
		 * Moves cursor to the next or previous position
		 * @param forward				If true move in forward direction, else move backward
		 * @param downward				If true move in downward direction, else move upward
		 * @param remain_on_same_line	If true forbids going to prev/next line
		 * @param interactive			True if called upon user action, else False
		 */
		void moveCursorToEditablePosition(bool forward=true, bool downward=true, bool remain_on_same_line=false, bool interactive=false);

		/**
		* Moves cursor to anchor with given id
		* @param id anchor 		Anchor id
		* @param to_edit_pos 	If true, places cursor on next editable position
		* @return 				True if cursor moved, else false
		*/
		bool moveCursorToAnchor(const string& id, bool to_edit_pos=false);

		/**
		* Moves cursor to anchor with given id
		* @param id anchor 		Anchor id
		* @param to_edit_pos 	If true, places cursor on previous editable position (ie before any tagged element attached to end anchored element)
		* @return 				True if cursor moved, else false
		*/
		bool moveCursorToAnchorEnd(const string& id, bool to_edit_pos=false);

		/**
		 * Inserts an anchored lable
		 * @param type				Type id
		 * @param id				Annotation id
		 * @param label				Tag label
		 * @param tagname			Tag name
		 * @param track				Track impacted
		 * @param r2l				True for right to left direction, false otherwise
		 * @param alignId			Specify the annotation element on which the new inserted label
		 * 							should be aligned (empty except for segmentation type label)
		 * @return
		 */
		const string& insertAnchoredLabel(const string& type, const string& id,
								const Glib::ustring& label, const string & tagname,
								int track, bool r2l, const string& alignId) ;

		/**
		 * Gets the text corresponding to given anchor
		 * @param id		Anchor id
		 * @return			The anchor text, or empty value if no anchor found
		 */
		Glib::ustring getAnchoredLabel(const string& id);

		/**
		* Deletes anchored label & associated anchor in text buffer
		* @param id 					Id of anchored label to delete
		* @param keep_anchor 			True to keep corresponding text mark in buffer
		* @param keepPrevSpace			True for keeping eventual previous space or endline character
		*/
		void deleteAnchoredLabel(const string& id, bool keep_anchor, bool keepPrevSpace=false) ;

		/**
		 * Gets the iterator pointing the anchor corresponding to the given id
		 * @param id		Anchor id
		 * @return			Corresponding text position
		 */
		Gtk::TextIter getAnchoredIter(const string& id);

		/**
		 * Checks which tags should be applied for the given element, and create
		 * them if they don't exist. Return all tags to be applied in a list.
		 * @param 		id				Element id
		 * @param 		type			Element type
		 * @param 		start_tag		True for starting element, false for ending element
		 * @param[out]  tags			List inside which all tag needed by element will be referenced
		 */
		void prepareElementTags(const string& id, const string& type, bool start_tag, list< Glib::RefPtr<Gtk::TextTag> >& tags) ;

		/**
		 * Inserts an anchored element represented by tagged label and text at cursor position.
		 * @param type				Element type
		 * @param id				Element id
		 * @param text				transcription text to be inserted
		 * @param do_enhance		True if text to be displayed with enhanced decoration
		 * @param isAnchored		True if the created anchor must be time coded
		 * @param isText			True for text only anchor, false for tagged text anchor
		 * @param label				Tagged label to be inserted before text
		 * @param nextId			nextId if used for inserting a stand-alone element
		 */
		void insertAnchoredTaggedText(const string& type, const string& id,
										const Glib::ustring& text, bool do_enhance,
										bool isAnchored, bool isText,
										const Glib::ustring& label, const string& nextId="") ;

		/**
		 * Delete transcription text identified by id
		 * @param id				Anchor id
		 * @param keep_anchor		True for keeping anchor, false for deleting it at same time
		 */
		void deleteAnchoredText(const string&  id, bool keep_anchor=false);

		/**
		 * Checks whether deleting the character at the given position won't make anchors
		 * to be superposed. In theses cases, indicates that a white space is needed
		 * @param 		pos						Iterator position
		 * @param 		backward				True for deleting in backward direction, false in forward
		 * @param[out] 	toBeMergedOrDeleted		Will be filled with 2 annotation id for merging them (returns 2)
		 * 										or 1 annotation id for deleting it (returns 1);
		 * @return								-1: deletion can't be done\n
		 * 										 0: deletion is allowed, nothing to do\n
		 * 								 		 1: deletion is allowed but an annotation must be deleted\n
		 * 								 		 2: deletion is allowed but annotation must be merged
		 */
		int canDeletePos(const Gtk::TextIter& pos, bool backward, vector<string>& toBeMergedOrDeleted) ;

		/**
		 * Checks whether deleting the selection at the given positions won't make anchors
		 * to be superposed. In theses cases, indicates that a white space is needed
		 * @param 		start					Selection start iterator
		 * @param 		stop					Selection end iterator
		 * @param[out] 	toBeMergedOrDeleted		Will be filled with 2 annotation id for merging them (returns 2)
		 * 										or 1 annotation id for deleting it (returns 1);
		 * @return								-1: deletion can't be done\n
		 * 										 0: deletion is allowed, nothing to do\n
		 * 								 		 1: deletion is allowed but an annotation must be deleted\n
		 * 								 		 2: deletion is allowed but annotation must be merged
		 */
		int canDeleteTextSelection(const Gtk::TextIter& start, const Gtk::TextIter& stop, vector<string>& toBeMergedOrDeleted) ;

		/**
		 * Checks whether a qualifier can be inserted at current position.
		 * Takes care of the editability of the position, and if a text selection is
		 * available to apply the qualifier (or if the cursor it set inside a word)
		 * @param pos			Text position
		 * @return				True if a quialifier can be inserted, false otherwise
		 */
		bool canInsertQualifier(const Gtk::TextIter& pos) ;

		/**
		 * Applies the track tag at the given anchor
		 * @param id		Id of the anchor where applying the tag
		 * @param notrack	Number of the impacted track
		 */
		void setTrackTag(const string& id, int notrack);

		/**
		 * Applies the track tag on the given range
		 * @param it1		Range start iterator
		 * @param it2		Range end iterator
		 * @param notrack	Number of the impacted track
		 */
		void addTrackTag(const Gtk::TextIter& it1, const Gtk::TextIter& it2, int notrack);

		/**
		 * Gets the anchored element id corresponding to the given position
		 * @param 	pos	 			Text iter position
		 * @param	timeAnchored 	If true, returns only signal-anchored elements (those with a defined signal offset)
		 * @return 					Element id, or empty value if no element at cursor position
		 */
		string getAnchoredElement(const Gtk::TextIter& pos, bool timeAnchored=false);

		/**
		 * Gets the anchored element id corresponding to the given position, if it has the type type.
		 * If there is more than one element, return the first found.
		 * @param 	pos	 			Text iter position
		 * @param 	type			Type
		 * @param	timeAnchored 	If true, returns only signal-anchored elements (those with a defined signal offset)
		 * @return 					Element id, or empty value if no element at cursor position
		 */
		string getAnchorIdByType(const Gtk::TextIter& pos, const string& type="" , bool timeAnchored=false) ;

		/**
		 * Sets label for given segment type
		 * @param id 		Segment id
		 * @param label 	Label to display
		 * @param tagname 	name of text tag to apply to label
		 * @param r2l		True for right to left zone, false otherwise
		 */
		void setAnchoredLabel(const std::string& id, const std::string& label, const string& tagname, bool r2l);

		/**
		 * Adds a new line to the buffer at the insert position
		 * @param type		Anchor type (will determinate if a new line can be inserted)
		 * @param id		Indicates an id for which the left gravity mechanism will not be applied
		 * 					("" for classic behaviour)
		 * @return			True if a new line has been insterted
		 */
		bool addNewline(const string& type, const string& id="");

		/**
		 * Inserts the given text at the cursor position
		 * @param text		Text to be inserted
		 */
		void insertText(const Glib::ustring& text);

		/**
		 * Inserts a word at the current position with surrounding blanks if required
		 * @param word		Word to be inserted
		 */
		void insertWord(const Glib::ustring& word);

		/**
		 * Replaces	the text in the given range.
		 * @param start				Range start
		 * @param stop				Range end
		 * @param text				Text to be inserted
		 * @param user_action		True for creating a single undo/redo block
		 * @return					The new text iterator position
		 */
		Gtk::TextIter replaceText(const Gtk::TextIter& start, const Gtk::TextIter& stop, const Glib::ustring& text, bool user_action=true) ;

		/**
		 * Inserts a tagged element in the buffer
		 * @param id			Tagged element id (id of the corresponding annotation)
		 * @param parent_id		Previous anchor element id
		 * @param type			Tagged element type
		 * @param label			Label to be displayed
		 * @param start_tag		True for start tag, False for end tag
		 * @param rtl			True for right to left mode, False otherwise
		 */
		void insertTaggedElement(const string& id, const std::string& parent_id,
									const string& type, const Glib::ustring& label,
									bool start_tag, bool rtl) ;

		/**
		 * Inserts a pixbuffer element
		 * @param id			Tagged element id (id of the corresponding annotation)
		 * @param parent_id		Previous anchor element id
		 * @param type			Tagged element type
		 * @param start_tag		True for start tag, False for end tag
		 * @param pixbuf		Name of the defined icon to be displayed
		 * @param tag			Tag name to be applied
		 * @deprecated			Not used anymore
		 */
		void insertPixElement(const string& id, const std::string& parent_id
					, const string& type, bool start_tag,
					const string& pixbuf, const string& tag) ;

		/**
		 * Returns the text of the tag element located at the given position
		 * @param pos		Text position
		 * @return			The corresponding text, or empty string if no tag could be found
		 */
		Glib::ustring getTaggedElementText(const Gtk::TextIter& pos);

		/**
		 * Returns the identifier of the tag element located at the given position
		 * @param pos		Text position
		 * @return			The corresponding identifier, or empty string if no tag could be found
		 */
		string getTaggedElementId(const Gtk::TextIter& pos);

		/**
		 * Returns the type of the tag element located at the given position
		 * @param pos		Text position
		 * @return			The corresponding type, or empty string if no tag could be found
		 */
		string getTaggedElementType(const Gtk::TextIter& pos);

		/**
		 * Returns the text position of the tagged element corresponding to the given id.
		 * @param id		Tagged element id
		 * @return			The corresponding text position.
		 */
		Gtk::TextIter getTaggedElementIter(const string& id);

		/**
		 * Deletes the tagged element corresponding to the given identifier.
		 * @param id		Tagged element id
		 * @note 			A tagged element can be composed by 2 tags (start and end labels).
		 * 					This method deletes the two labels.
		 */
		void deleteTaggedElement(const string& id);

		/**
		 * Deletes the piexbuffer element corresponding to the given identifier.
		 * @param id		Pixbuffer element id
		 * @deprecated		Not used anymore
		 */
		void deletePixElement(const string& id) ;

		/**
		 * Checks whether the given position allowed a split of element.
		 * Will return false if the cursor try to cut a qualifier without cutting text
		 * (between qualifier start tag and element text or element text and qualifier end tag)
		 * @param iter		Buffer position
		 * @return			True if allowed, false if forbidden
		 */
		bool canSplitAtIter(const Gtk::TextIter& iter) ;

		/**
		 * Checks whether the current position has the given tags
		 * @param iter			Buffer positiotn
		 * @param tagnames		List of tagnames
		 * @param is_prefix		True if the provided tagnames can be prefixes of the applied tags.
		 * @return				True if at least one tag has matched, false otherwise
		 */
		bool iterHasTags(const Gtk::TextIter& iter, const vector<string>& tagnames , bool is_prefix) ;

		/**
		 *  Wrapper to Anchors::getPreviousAnchorId(const Gtk::TextIter&,string,bool).\n
		 *  Gets anchor of given type at or preceding given position in text buffer
		 *  @param pos 			Position in text buffer
		 *  @param type 		Type of anchor / "" for any type
		 *  @param timeAnchored 		True if end anchor must be signal-anchored, else false
		 *  @return 			Anchor id
		 */
		const string& getPreviousAnchorId(const Gtk::TextIter& pos, const string& type, bool timeAnchored=false) { return anchors().getPreviousAnchorId(pos, type, timeAnchored); }

		/**
		 *  Wrapper to Anchors::getNextAnchorId(const Gtk::TextIter&,string,bool).\n
		 *  Gets anchor of given type following given position in text buffer
		 *  @param pos 			Position in text buffer
		 *  @param type 		Type of anchor / "" for any type
		 *  @param timeAnchored 		True if end anchor must be signal-anchored, else false
		 *  @return 			Anchor id
		 */
		const string& getNextAnchorId(const Gtk::TextIter& pos, const string& type, bool timeAnchored=false) { return anchors().getNextAnchorId(pos, type, timeAnchored); }

		/**
		 *  Wrapper to Anchors::getPreviousAnchorId(const string&,const string&,bool).\n
		 *  @param id			Anchor id
		 *  @param type 		Type of anchor / "" for any type
		 *  @param timeAnchored 		True if end anchor must be signal-anchored, else false
		 *  @return				Anchor id
		 */
		const string& getPreviousAnchorId(const string& id, const string& type, bool timeAnchored=false) { return anchors().getPreviousAnchorId(id, type, timeAnchored); }

		/**
		 *  Wrapper to Anchors::getNextAnchorId(const string&,const string&,bool).\n
		 *  @param id			Anchor id
		 *  @param type 		Type of anchor / "" for any type
		 *  @param timeAnchored 		True if end anchor must be signal-anchored, else false
		 *  @return				Anchor id
		 */
		const string& getNextAnchorId(const string& id, const string& type, bool timeAnchored=false) { return anchors().getNextAnchorId(id, type,timeAnchored); }

		/**
		* Returns the insert position in the given text segment as an offset in chars from
		* segment start
		* @param 		id 		Anchor id
		* @param 		iter 	Text buffer iterator
		* @param	 	trim 	If true, returns offset in "trimmed" text
		* @return 				Offset in text starting at anchor up to given iterator.
		*
		* @note  				A negative offset indicates that given iterator is placed before
		* 						eventual tagged elements attached to the given anchor
		*/
		int getOffsetInTextSegment(const string& id, const Gtk::TextIter& iter, bool trim);

		/**
		 * Gets the text in current selection
		 * @param with_labels		If true the label text will be returned too
		 * @return					Selection text (with or without label text)
		 */
		Glib::ustring getSelectedText(bool with_labels=false);

		/**
		* Retrieves the segment text starting at segment anchor up to next anchor
		* @param id start 		Anchor id
		* @param end_id 		End anchor id, or empty string for guessing the end from buffer contents
		* @param with_labels 	True to include tagged elements labels in returned text
		* @param trim 			True to trim leading and trailing blanks from buffer
		*
		* @note					This function is mainly used for 2 purposes :\n
		*  							- retrieving text to be stored in data model (<em>with_labels</em>=false);\n
		*  							- retrieving text to be displayed in signal segments (<em>with_labels</em>=true)
		*/
		Glib::ustring getSegmentText(const string& id, const string& end_id="", bool with_labels=false, bool trim=false);

		/**
		 * Updates lines alignment for whole element
		 * @param id 		element id
		 * @param r2l 			True for right to left display, false otherwise
		 */
		void updateAlignment(const string& id, bool r2l);

		/**
		* Deletes text segment & returns initial segment position in buffer
		* @param id 		Id of segment to be deleted
		* @param timeAnchored 		If true,  delete up to next "main" segment
		* @return 			Position of deleted segment start or current insert position if segment not found
		*/
		Gtk::TextIter deleteSegment(const string& id, bool timeAnchored=false);

		/**
		 * Checks if the given position has editable property
		 * @param 		iter			Text position
		 * @param[out]	need_split
		 * @param interactive			True if called upon user action, else False
		 * @return						True if the position is editable, false otherwise.
		 */
		bool isEditablePosition(const Gtk::TextIter& iter, bool& need_split, bool interactive=false);

		/**
		 * Checks if the given range has editable property
		 * @param start		Range start
		 * @param end		Range end
		 * @return			True if the range is editable, false otherwise
		 */
		bool isEditableRange(const Gtk::TextIter& start, const Gtk::TextIter& end) ;

		//TODO ICI FAIRE ERASE RANGE qui retourne erased ids
		/**
		 * Gets all element identifiers (anchors & tagged elements) in the given range
		 * @param start			Range start
		 * @param stop			Range end
		 * @param prefix		Tag prefix (empty for all)
		 * @param with_timeAnchored		Whether or not getting main anchors
		 * @return				A vector with all matching identifiers
		 */
		vector<string> getIds(const Gtk::TextIter& start, const Gtk::TextIter& stop, const string& prefix, bool with_timeAnchored=true);

		/**
		 * Defines the track number associated to the given element
		 * @param type			Element type
		 * @param id			Element id
		 * @param notrack		Track number
		 * @param fromOverlap	True if change is due to overlap changes
		 * @param r2l			True for right-to-left language, false otherwise
		 */
		void setTrack(const string& type, const string& id, int notrack, bool fromOverlap, bool r2l) ;

		/**
		 * Defines the given tag as active, i.e that can receive activation signal
		 * for launching action (displaying popop,launching dialog, ...)
		 * @param tag			Tag name
		 * @param tagclass		Tag class
		 */
		void setActiveTag(Glib::RefPtr<Gtk::TextTag>& tag, const string& tagclass);

		/**
		 * Gets the active tag class at the given position
		 * @param iter		Buffer position
		 * @return			Tag class if found, empty otherwise
		 */
		string getActiveTagClass(const Gtk::TextIter& iter);

		// to allow updates without loosing current insert pos

		/**
		 * Creates a Gtk::TextMark for keeping current insert position.\n
		 * Useful for keeping the position before a buffer update.
		 * @param no				Backup stamp to use for restoration
		 * @param left_gravity		True for creating a left_gravity mark, False otherwise
		 * @see						restoreInsertPosition(int,bool);
		 */
		void backupInsertPosition(int no=0, bool left_gravity=false);

		/**
		 * Restores insert position saved with backupInsertPosition(int,bool)
		 * @param no				Backup stamp used when saving position
		 * @param to_edit_pos		If set to true, will be placed at the nearest
		 * 							editable position to the given position.
		 */
		void restoreInsertPosition(int no=0, bool to_edit_pos=true);

		/**
		 * Checks if the current text position is a only a label.\n
		 * Checks if it is tagged with "label" tag and no other.
		 * @param iter		Text position
		 * @return			True if label only condition is filled, False otherwise
		 */
		bool isLabelOnly(const Gtk::TextIter& iter);

		/**
		 * Inhibates the edit signal emission
		 * @param b			True for enabling emission, False otherwise
		 * @return			The previous value before the new was set
		 */
		bool inhibateEditSignal(bool b=true) { bool p=m_inhibSignal; m_inhibSignal = b; return p;}

		/**
		 * Indicates wheter an insertion is being proceeded (if edit signal is currently disabled)
		 * @return			True if insertion is locked, False otherwise
		 * @see				inhibateEditSignal(bool)
		 */
		bool insertInProgress() { return m_inhibSignal; }

		/**
		 * Modify the editability property at the given position
		 * @param iter			Text position
		 * @param editable		Editability to set
		 */
		void setEditable(const Gtk::TextIter& iter, bool editable=true);

/* SPELL */
//		/**
//		 * Set the speller that the parent AnnotationView is using
//		 * @param spell			Pointer on the speller used by the parent
//		 */
//		void setSpeller(GtkSpell* spell) ;

		/**
		 * Sets speller confidence mode
		 * @param value			True for enabling speller to use confidence tag
		 */
		void setSpellerConfidence(bool value) ;

		/**
		 * Recheck the given range
		 * @param off_start		Offset start
		 * @param off_end		Offset end
		 */
		void spellerRecheck(float off_start, float off_end) ;

		/**
		 *	Restore buffer cursor at the last offset position
		 */
		float restoreCursor() ;

		/**
		 * Save current cursor offset
		 */
		void saveCursor() ;

		/**
		 *   Filters non-unique consecutive spaces (spaceHandling)\n
		 *   Adds space borders around special characters defined in conventions (spaceBordering)\n
		 * @param pos				Budder position where the given <em>text_</em> is
		 * @param text_				Text to be filtered
		 * @param use_iter			If set to true, means the text must be correctly inserted
		 * 							at the given position, so spaceHandling and spaceBordering
		 * 							will check the characters preceding and following the given text.
		 * @param spaceHandling		True for checking the forbidden consecutive space characters
		 * @param spaceBordering	True for adding compelled space around specific characters
		 * @return					The filtered text
		 */
		Glib::ustring spaceHandler(const Gtk::TextBuffer::iterator& pos, const Glib::ustring& text_, bool use_iter,
															bool spaceHandling, bool spaceBordering);

		/**
		 * 	Checks if character(s) can be deleted:
		 *  1) Don't allow space deletion if they are border-spaces
		 *  (spaces around special character definded in convention)\n
		 *  2) Force border-space deletion when deleting special character\n
		 *  3) Force space deletion when deleting will collapse 2 spaces together\n\n
		 *
		 *  Do nothing if space-bordering and space handling are not activated in conventions
 		 *
		 *  @param 		start 				Iterator start of deleted selection
		 *  @param 		stop   				Iterator stop of deleted selection (equal to start if no selection)
		 *  @param[in]  res		  			Modified start and stop iterator for deletion, if empty use start
		 *  @param spaceHandling			True for checking the forbidden consecutive space characters
		 *  @param spaceBordering			True for adding compelled space around specific characters
		 *  @return  						True if deletion is allowed, False otherwise
		 */
		bool spaceDeleter(const Gtk::TextIter& start, const Gtk::TextIter& stop, std::vector<Gtk::TextIter>& res,
								bool spaceHandling, bool spaceBordering) ;

		/**
		 * Print all hexadecimal value of hidden characters in the given buffer,
		 * i.e presentation characters or space characters.
		 * @param txt				Text to be printed
		 * @param normal_chars		Text to display when a "normal" character is found
		 * @param beep				Enables a sonor mark when a hidden char is found
		 * @remarks					Ugly debug method
		 */
		void print_hidden_chars(const Glib::ustring& txt, const Glib::ustring&  normal_chars, bool beep);

		/**
		 * 	Displays in standard output special spaces character such as space, carriage, lineFeed.
		 * @remarks					Ugly debug method
		 */
		void check_presentation_chars_in_buffer() ;

		/**
		 * Removes all presentation character from the given string
		 * @param s			String to be cleaned
		 */
		void removePresentationCharacters(Glib::ustring& s) ;

		/**
		 * Prints the given buffer.
		 * @param hexa		True for printing the hexadecimal value
		 * @param txt		Text to be printted
		 */
		void print_buffer(bool hexa, const Glib::ustring&  txt) ;

		/**
		 * Inserts a text in the buffer regarding the possibility of insertion
		 * @param pos					Text position
		 * @param text					Text to be inserted
		 * @param default_editable		If set to False, will block the insertion
		 * @return						New iterator position, and whether the insertion has been done.
		 */
		std::pair<Gtk::TextIter,bool> insert_interactive(const Gtk::TextIter& pos, const Glib::ustring& text, bool default_editable=true) ;
		/** Deletes the range between the "insert" and "selection_bound" marks,
		* that is, the currently-selected text. If @a interactive  is <tt>true</tt>,
		* the editability of the selection will be considered (users can't delete
		* uneditable text).
		* @param interactive Whether the deletion is caused by user interaction.
		* @param default_editable Whether the buffer is editable by default.
		* @return Whether there was a non-empty selection to delete.
		*/
		bool erase_selection(bool interactive = true, bool default_editable = true);

		/**
		 * Inserts a text in the buffer regarding the possibility of insertion
		 * @param range_begin			Text start position
		 * @param range_end				Text end position
		 * @param default_editable		Whether the buffer is editable by default. If set to False, will block the deletion
		 * @return						New iterator position, and whether the deletion has been done.
		 */
		std::pair<Gtk::TextIter, bool> erase_interactive (const Gtk::TextIter& range_begin, const Gtk::TextIter& range_end, bool default_editable=true);

		/**
		 * Removes all tags marked as propagating one in the given range.\n
		 * As Gtk::TextTag can propagate, this method is used after insertion
		 * to be sure the new inserted text hasn't been marked with a surrounding tag.
		 * @param start		Range start
		 * @param stop		Range end
		 * @see				addPropagatingTag(Glib::RefPtr<Gtk::TextTag>)
		 */
		void removePropagatingTags(const Gtk::TextIter& start, const Gtk::TextIter& stop) ;

		/**
		 * Marks a tag as a propagating one.
		 * @param tag		Pointer on given tag
		 * @note			Mostly used in removePropagatingTags(const Gtk::TextIter&,const Gtk::TextIter&)
		 */
		void addPropagatingTag(const Glib::RefPtr<Gtk::TextTag>& tag) ;

		/**
		 * Create a new buffer tag
		 * @param name			Tag name
		 * @param tagclass		Tag class
		 * @param undoable		True if the tag can be impacted with undo/redo actions
		 * @param color_fg		Foreground color
		 * @param color_bg		Background color
		 * @param style_cfg		"italic" or "oblic" for Italic presentation
		 * @param weight_cfg	"bold" for bold presentation
		 * @param flags			Tag property flags
		 * @return				Reference on the newly created tag
		 */
		Glib::RefPtr<Gtk::TextTag> createAnnotationTag(const string& name,
					const string &tagclass, bool undoable,
					const string& color_fg, const string& color_bg,
					const string& style_cfg, const string& weight_cfg,
					unsigned long flags = 0);

		/**
		 * Checks whether the given character is a space character (only space, not tabs or other similar types)
		 * @param c			Character
		 * @return			True of False
		 */
		bool isSpaceChar(char c) ;

		/**
		 * Checks whether the given character is a space one (isSpaceChar(char) meaning) and
		 * placed at an editable position.
		 * @param c			Character
		 * @param iter		Text position
		 * @return			True of False
		 */
		bool isEditableSpaceChar(char c, const Gtk::TextIter& iter) ;

		/**
		 * Defines the AnnotationBuffer font style
		 * @param font		Font name to set
		 * @param mode		"text" for setting normal text font, "label" for label font
		 * @note			When used in "text" mode, this will impact qualifiers too
		 */
		void setFontStyle(const string& font, const string& mode) ;

		/**
		 * Checks whether there is mark at current position that needs to have
		 * a left gravity.
		 * @param iter			Text position
		 * @param exclude		Annotation id to be exclude for anchor treatment
		 * @param textType		if true targets text-type anchors
		 * @return				the anchor that needs to be left, NULL if none was found
		 */
		Anchor* markNeedsLeftGravity(const Gtk::TextIter& iter, const string& exclude="", bool textType=false) ;

		/**
		 * Moves the anchor identified by id at the position given by text iterator
		 * by the given offset
		 * @param id		Anchor id
		 * @param iter	Text offset
		 */
		void moveAnchor(const string& id, const Gtk::TextIter& iter) ;

		/**
		 * Moves the given anchor identified  by id at the position given by text iterator
		 * by the given offset
		 * @param a		Anchor pointer
		 * @param iter	Text offset
		 */
		void moveAnchor(Anchor* a, const Gtk::TextIter& iter) ;

		/**
		 * Accessor to the AnnotationView parent
		 * @return		Pointer on the parent
		 */
		AnnotationView* getAnnotationView() ;

		/**
		 * Debug purpose
		 * @return
		 */
		std::string printCurrentLine() ;

		/**
		 * Create all tags (except annotation ones)
		 */
		void configureMainTags() ;

		/**
		 * Create highlightTags
		 */
		void createHighlightTags() ;

		/**
		 * Accessor to selection indicator
		 * @return		True if a selection has been done, false otherwise
		 */
		bool hasSelection() { return m_hasSelection ; }

		/**
		 * Returns the color of the given label. Uses configuration values
		 * @param label				Label
		 * @param mode				fg, bg, style, weight (foreground, background, style, weight)
		 * @param coefficient		Integer for attenuation
		 * @return					The color string value
		 * @todo					render classes should be friend of AnnotationBuffer
		 */
		string getLabelLook(const string& label, const string& mode, int coefficient=0) ;

		/**
		 * Proceeds specific actions relative to text insertion.
		 * These specific action should be applied after text insertion.
		 * @param start		Insertion position
		 * @param text		Inserted text
		 * @param length	Inserted text length
		 * @warning			Internal and specific use only.
		 */
		bool onInsertAfter(const Gtk::TextIter& start, const Glib::ustring& text, int length) ;

		/**
		 * Block connection for left gravity mechanism
		 * @param block		True for blocking, false for enabling
		 * @return			The previous state of the connection
		 * @warning			This method doesn't forbid to execute left gravity
		 * 					mechanism, manual mechanism (call of markNeedsLeftGravity method)
		 * 					will still work. This method only deactivate the
		 * 					automatic mechanism
		 */
		bool blockLeftGravityConnection(bool block) ;

		/**
		 * Sets the interactive insert mode
		 * @param value		True for interactive insert, false otherwise
		 * @note			Used by callback of insert action for applying specific
		 * 					interactive insert treatments
		 */
		void setInteractiveInsert(bool value) 	{ m_interactiveInsert = value  ; }

		/**
		 * Accessor to interactive access
		 * @return			True if interactive insert, false otherwise
		 */
		bool getInteractiveInsert() 			{ return m_interactiveInsert ; }

		/**
		 * Sets the text mode for left gravity mechanism.
		 * @param value		True if we are inserting text
		 * @note			Used by callback of insert action, for correct left
		 * 					gravity anchor mechanism
		 */
		void setLeftGravityTextMode(bool value) { m_leftGravityTextMode = value ; }

		/**
		 * Accessor to the left gravity text mode
		 * @return			True or false
		 */
		bool getLeftGravityTextMode() 			{ return m_leftGravityTextMode ; }

		/**
		 *
		 * @param value
		 */
		void setSpecialPasteInitialized(bool value) ;

	private:

		void configureConventions();

		/**
		 * override textview default behaviour for key_press_event
		 * @param event GdkEventKey event
		 */

		virtual void on_apply_tag(const Glib::RefPtr<Gtk::TextBuffer::Tag>& tag, const Gtk::TextBuffer::iterator& range_begin, const Gtk::TextBuffer::iterator& range_end);
		virtual void on_remove_tag(const Glib::RefPtr<Gtk::TextBuffer::Tag>& tag, const Gtk::TextBuffer::iterator& range_begin, const Gtk::TextBuffer::iterator& range_end);
		virtual void on_mark_set(const Gtk::TextBuffer::iterator&  pos, const Glib::RefPtr<Gtk::TextBuffer::Mark>& mark);
		//virtual void on_insert(const Gtk::TextBuffer::iterator&  pos, const Glib::ustring& text, int  bytes);

		virtual void on_erase(const Gtk::TextBuffer::iterator& start, const Gtk::TextBuffer::iterator& stop );

		bool emitTagEvent(const Glib::RefPtr<Glib::Object>& o, GdkEvent* e, const Gtk::TextIter& i, std::string tagclass);

		Gtk::TextBuffer::iterator insertAtCursorWithTag(const Glib::ustring& text, const Glib::ustring& tagname);
		void printTagName(const Glib::RefPtr<Gtk::TextTag >& tag);

		void clearHighlightTag(const Glib::RefPtr<Gtk::TextTag>& tag, int track);

		void configureEventTags() ;
		void configureBackgroundTags() ;
		void configureEntityTags() ;
		void createEntityTag(const string& name, const string & name_end,
					const string& color_fg, const string& color_bg,
					const string& style, const string& weight) ;

		Pango::Weight from_CfgWeight_to_Pango(const string& key) ;
		Pango::Style from_CfgStyle_to_Pango(const string& key) ;

		void resetColors() ;
		void inhibateSpaceHandling(bool value)  { m_inhibSpaceHandling = value ; }
		bool needSpaceBorders(gunichar c);

		void setTimestamp(const Gtk::TextIter& startIter, const Gtk::TextIter& endIter, bool on) ;
		/**
		*  Returns end text iterator for text part starting at anchor id
		*  @param id start 		Anchor id
		*  @param end_id 		End anchor id, or empty value for guessing it from buffer contents
		*  @param timeAnchored 		True to stop only at a "main" anchor location, false to stop at next anchor in buffer (used only if end_id not specified)
		*  @note return 		Iter is set just before any eventual label-only text preceding end anchor.
		*/
		Gtk::TextIter getAnchoredEndIter(Anchor* start, const string& end_id="", bool timeAnchored=false);

		bool applyConfidenceTag(const Gtk::TextIter& it);
		virtual void on_insert(const Gtk::TextBuffer::iterator&  pos, const Glib::ustring& text, int  bytes);
		void onInsertBefore(const Gtk::TextIter&  pos, const Glib::ustring& t, int  bytes) ;

	private:

		AnchorSet m_anchors; // text anchors for annotations
		string m_defaultSegmentTagColor;
		string m_idTagPrefix;
		std::map<std::string, std::string> m_tagBgColor;
		std::map<std::string, std::string> m_tagPrefix;
		std::map<Glib::ustring, std::string > m_activeTags;
		std::map<std::string, bool> m_newlineBefore; // newline before element
		std::map<std::string, bool> m_newlineAfter; // newline after element label
		std::string m_unitType; // editable text type
		std::string m_segmentType; // editable text type
		std::set<string> m_entities;

		bool specialPasteInitialized ;

		bool m_hasSelection; // has selected text
		bool m_inhibSignal;
		bool m_inhibSpaceHandling;
		bool m_spaceHandling;
		bool m_spaceBordering;
		std::set<gunichar> m_borderedChars;
/* SPELL */
//		GtkSpell* m_speller; /**< parent spell checker */

		Glib::RefPtr<Gtk::TextMark> m_backup[3];

		float cursorOffset ;

		Glib::RefPtr<Gtk::TextMark> _tmpEditMark;
		Glib::RefPtr<Gtk::TextMark> m_startMark; // for buffer local updates; left anchored
		Glib::RefPtr<Gtk::TextMark> m_endMark;   // for buffer local updates; right anchored
	
		/** mark left gravity mechanism **/
		sigc::connection m_connectionInsertBefore ;
		Anchor* m_leftGravityCandidate ;
		Glib::RefPtr<Gtk::TextMark> m_leftGravityMark ;
		bool m_leftGravityTextMode ;
		bool m_leftGravityUserBlok ;
		bool m_interactiveInsert ;

		sigc::signal<void, const Gtk::TextIter& > m_signalSetCursor;
		sigc::signal<void, const Gtk::TextIter&, int > m_signalHasEdits;
		sigc::signal<void, const Gtk::TextIter&, const Gtk::TextIter&> m_signalErase;
		sigc::signal<void, bool> m_signalSelectionSet;
		sigc::signal<void, std::string, GdkEvent*, const Gtk::TextIter& > m_signalTagEvent;

		Glib::RefPtr<Gtk::TextTag> m_nospellTag ;
		Glib::RefPtr<Gtk::TextTag> m_labelTag ;
		Glib::RefPtr<Gtk::TextTag> m_timestampTag ;
		Glib::RefPtr<Gtk::TextTag> m_confidenceTag ;
		Glib::RefPtr<Gtk::TextTag> m_editableTag ;
		vector<string> m_leftGravityTags ;

		/*
		 * View of all tags that propagate when inserting a character, and
		 * we want to remove after insertion
		 */
		vector< Glib::RefPtr<Gtk::TextTag> > m_propagatingTags ;
		/*
		 * View of all tags that should have the same size than  editor text
		 * for sexy presentation.
		 * Used when changing text size
		 */
		vector< Glib::RefPtr<Gtk::TextTag> > m_sameTextDisplayTags ;
		/*
		 * View of all tags that should have the same size than labels
		 * Used when changing label text size
		 */

		vector< Glib::RefPtr<Gtk::TextTag> > m_labelDisplayTags ;


		//==>CF
		AnnotationView *m_view;
		bool m_language_change_activated;
		//<==CF

};

} /* namespace tag */

#endif /* _HAVE_ANNOTATION_BUFFER_H */
