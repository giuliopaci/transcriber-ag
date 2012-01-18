/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		UndoableTextView.h
 */

/* *************************************************************************
 * Copyright (c) 2005 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * *************************************************************************/


/*
 * UndoableTextView.hh --
 *
 *      Implement custom widget to support Undo/Redo for TextViews.
 */


#ifndef _HAVE_UNDOABLE_TEXT_VIEW_H
#define _HAVE_UNDOABLE_TEXT_VIEW_H


#include <stack>
#include <list>
#include <gtkmm.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>
#include "AnnotationBuffer.h"


namespace tag {


class EditAction;

/**
 * @class 		UndoableTextView
 * @ingroup		AnnotationEditor
 *
 *	Custom widget to support Undo/Redo for TextViews.\n
 *  Inspired from VMware undoableTextView, modified to allow grouping
 *  of edit actions and managing external undo/redo actions (i.e tag creation/deletion).\n
 *
 */
class UndoableTextView: public Gtk::TextView
{
	public:
		/**
		 * Constructor.
		 * @param buffer  associated text buffer (will be created by default)
		 */
		UndoableTextView(const Glib::RefPtr<tag::AnnotationBuffer> &buffer =
			AnnotationBuffer::create());

		/*! destructor */
		virtual ~UndoableTextView(void);

		/**
		 * @return	True if some undoable actions are stored on stack
		 */
		bool getCanUndo(void);

		/**
		 * @return 	True if some redoable actions are stored on stack
		 */

		bool getCanRedo(void);

		/*!  Undoes last undoable action or action group */
		void undo(void);

		/*!  Redoes last undone action or action group */
		void redo(void);

		/*! Clears undo stack */
		void clearUndoHistory(void);

		/**
		 * Declares next edit actions as undoable or not.
		 * @param b 	True if next exit actions are undoable, else false.
		 */
		void setUndoableActions(bool b);

		/**
		 * Declares a TextTag as being undo/redo-able; only declared tags can
		 * benefit or the undo/redo mechanism.
		 * @param tagname 	Tag name
		 */
		void addUndoableTag(std::string tagname);

		/**
		 * Declares a TextTag as being undo/redo-able; only declared tags can
		 * benefit or the undo/redo mechanism.
		 * @param tag 		Reference on TextTag.
		 */
		void addUndoableTag(const Glib::RefPtr<Gtk::TextTag>& tag);

		/**
		 * Accessor to the undo/redo action group.
		 * @return			Pointer on the undo/redo action group.
		 */
		Glib::RefPtr<Gtk::ActionGroup>&  getActionGroup() { return m_editActionGroup; }

		/**
		 * Set undo/redo action enabled or not.
		 * @param editable		True for enabling, False otherwise
		 */
		void enableUndoRedoAction(bool editable) ;

		/**
		 * Signal emitted when undo/redo states changes.
		 */
		sigc::signal<void>& undoChangsignalCustomEventedSignal() { return m_undoChangedSignal; }

		/**
		 * Signal emitted when an undo/redo action has been proceded.\n
		 * <b>boolean parameter:</b>	True for undo, False for redo\n
		 */
		sigc::signal<bool, bool>& undoRedoActionSignal() { return m_undoRedoActionSignal; }

		/**
		 * Signal emitted when a custom undo/redo (annotation data model one) action has been proceded.\n
		 * <b>const string& parameter:</b>	String representation of the action\n
		 * <b>bool parameter:</b>			True for undo, False for redo\n
		 * @remarks							The UndoableTextView class manages and leads all undo/redo actions.
		 *									Once an undo/action has been done, the view actualizes by itself
		 *									(view internal methods) the modified view resulting from the action,
		 *									but only for internal mechanisms. Therefore, we need to tell external
		 * 									components that one of theirs externals actions has been popped from
		 * 									undo/redo stack.
		 */
		sigc::signal<void, const string&, bool>& signalCustomUndoRedo() { return m_signalCustomUndoRedo; }

		/**
		 * Signal emitted when all undo stack has been unstacked.
		 */
		sigc::signal<void>& signalUndoAllUndone () { return m_signalUndoAllDone ; }

		/**
		 * set undoable tag prefix (in addition to all tags listed in m_undoableTags
		 */

		void setUndoableTagPrefix(const string& pref) { m_undoableTagPrefix = pref; }

	protected:
		Glib::RefPtr<tag::AnnotationBuffer> m_buffer;  			/**< Buffer used by the view */
		std::stack<guint32> m_lastIterOffset;  					/**< Last edit position */
		Glib::RefPtr<Gtk::ActionGroup> m_editActionGroup ;		/**< Edit action group section used by the view */

		/**
		 * Places an external action into undo/redo stack.
		 * @param eventData		String representation of the external action
		 */
		void doCustomEvent(const string& eventData);

		/**
		 * Populates the given view popup menu.
		 * @param menu		Reference on the view contextual menu.
		 */
		void onPopulatePopup(Gtk::Menu *menu);

		/** Debug purpose **/
		void print_stack(bool undo) ;

	private:
		typedef std::deque<EditAction *> ActionStack;
		ActionStack m_undoStack ;
		ActionStack m_redoStack ;
	
		unsigned int m_frozenCnt;
		unsigned int m_userActionCnt;
		bool m_tryMerge;

		unsigned int m_actionsInCurrentGroup;
		unsigned int  m_modifiedClearedAt;
		std::list< Glib::RefPtr<Gtk::TextTag> > m_undoableTags;
		string m_undoableTagPrefix;

		bool lockedGroupAction ;
		Glib::RefPtr<Gtk::Action> m_refUndo;
		Glib::RefPtr<Gtk::Action> m_refRedo;
		string m_markPrefix;

		std::vector<string> m_replacements ;

		/*0:undo - 1: redo*/
		int last_action ;

		sigc::signal<void, const string&, bool> m_signalCustomUndoRedo;

		/* emit for specifying changes in buffer (modified state) */
		sigc::signal<void> m_undoChangedSignal;

		/* emit for specifying that an action has been undone/redone (for update view components */
		sigc::signal<bool, bool> m_undoRedoActionSignal;

		/* emit for telling the editor that we've get back to the "before-action" situation */
		sigc::signal<void> m_signalUndoAllDone ;

		void onInsert(const Gtk::TextBuffer::iterator &start, const Glib::ustring &text, int length);
		void onErase(const Gtk::TextBuffer::iterator &start, const Gtk::TextBuffer::iterator &end);
		void onTagEvent(const Glib::RefPtr<Gtk::TextTag>& tag, const Gtk::TextBuffer::iterator &start,
								const Gtk::TextBuffer::iterator &end, bool added);
		void onMarkSetEvent(const Gtk::TextBuffer::iterator &pos, const Glib::RefPtr<Gtk::TextMark>& mark);
		void onMarkMoveEvent(float old_pos, const Gtk::TextBuffer::iterator &pos, const Glib::RefPtr<Gtk::TextMark>& mark,
								bool forceDisplayUpdate) ;
		void onMarkDeletedEvent(const Glib::RefPtr<Gtk::TextMark>& mark);

		bool isUndoablePix(const Glib::RefPtr<Gdk::Pixbuf>& Pix) ;
		void onPixEvent(const Gtk::TextBuffer::iterator& pos, const Glib::RefPtr<Gdk::Pixbuf>& pix, bool added) ;
		void onBeginEndUserAction(bool);
		void onModifiedChanged();
		bool isUndoableTag(const Glib::RefPtr<Gtk::TextTag>& tag) ;
		bool isUndoableMark(const Glib::RefPtr<Gtk::TextMark>& mark) ;
		void createActionGroup();
		void undoRedo(ActionStack &popFrom, ActionStack &pushTo, bool isUndo);
		void addUndoAction(EditAction *action);
		void resetStack(ActionStack &stack);
		void onSignalLastIterOffset(guint32 offset) ;
};


} // namespace tag


#endif // _HAVE_UNDOABLE_TEXT_VIEW_H
