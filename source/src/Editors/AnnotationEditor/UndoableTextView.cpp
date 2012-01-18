/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  UndoableTextView
 *		 custom widget to support Undo/Redo for TextViews.
 *        inspired from VMware undoableTextView, but improved to allow
 *       grouping of edit actions and tracking of tag add/remove actions
 *
 *       also fixed selection problem by using "place_cursor" instead or
 *         "move_mark" in undo/redo actions
 *
 *    Copyright (c) 2007 Bertin Technologies
 */

/* *************************************************************************
 * Copyright (c) 2005 VMware, Inc.
 *
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
 * undoableTextView.cc --
 *
 *      Implement custom widget to support Undo/Redo for TextViews.
 */

#include <iostream>
using namespace std;

#include <gtkmm/stock.h>

#include "Common/globals.h"
#include "UndoableTextView.h"
#include "Common/util/Utils.h"

namespace tag {
#define TRACE_UNDO TRACE_D << "UNDO " << __LINE__

#define LOG_FINE 	Log::setTraceLevel(Log::FINE);
#define LOG_RESET	Log::resetTraceLevel();

#define DOTRACE_UNDOV true
//#define DOTRACE_UNDOV false

/*=================================================================================*/
/*=================================================================================*/
/*						ACTION TYPES DECLARATION                                   */
/*=================================================================================*/
/*=================================================================================*/
/*
 *-----------------------------------------------------------------------------
 *
 * tag::EditAction --
 *
 *      Abstract base class for editing events.  Supports merging similar
 *      events and undoing/redoing them.
 *
 *-----------------------------------------------------------------------------
 */

class EditAction
{
	protected:
		int m_OrderInGroup;
		Glib::ustring m_type;

		sigc::signal<void, guint32> m_signalAddToLastIterOffset;

	public:
		EditAction(int order_in_group = 1, Glib::ustring type = "") :
			m_OrderInGroup(order_in_group), m_type(type)
		{
		}
		virtual ~EditAction()
		{
		}

		virtual void undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) = 0;
		virtual void redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) = 0;
		virtual void merge(EditAction *action) = 0;
		virtual bool getCanMerge(EditAction *action) = 0;

		int orderInGroup()
		{
			return m_OrderInGroup;
		}
		void setOrderInGroup(int order)
		{
			m_OrderInGroup = order;
		}
		Glib::ustring getType()
		{
			return m_type;
		}

		virtual unsigned int getIndex() = 0;
		virtual Glib::ustring getData() = 0;
		virtual void setData(Glib::ustring value) = 0;

		sigc::signal<void, guint32>& signalAddToLastIterOffset()
		{
			return m_signalAddToLastIterOffset;
		}
};

/*
 *-----------------------------------------------------------------------------
 *
 * tag::InsertAction --
 *
 *      An EditAction created on text insert events, via typing or paste.
 *
 *-----------------------------------------------------------------------------
 */

class InsertAction: public EditAction
{
	public:
		InsertAction(const Gtk::TextBuffer::iterator &start, const Glib::ustring &text, int length, int order_in_group);
		virtual ~InsertAction(void);

		virtual void undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void merge(EditAction *action);
		virtual bool getCanMerge(EditAction *action);
		virtual Glib::ustring getData()
		{
			Glib::ustring data = "";
			data.append("[" + m_type + "]:::");
			data.append("text(" + mText + ")");
			data.append(":");
			data.append("index(" + number_to_string(mIndex) + ")");
			return data;
		}
		virtual void setData(Glib::ustring value)
		{
			mText = value;
		}
		virtual unsigned int getIndex()
		{
			return mIndex;
		}

	private:
		Glib::ustring mText;
		unsigned int mIndex;
		bool mIsPaste;
};

/*
 *-----------------------------------------------------------------------------
 *
 * tag::EraseAction --
 *
 *      An EditAction created on text delete events, via either delete or
 *      backspace or a cut operation.
 *
 *-----------------------------------------------------------------------------
 */

class EraseAction: public EditAction
{
	public:
		EraseAction(const Gtk::TextBuffer::iterator &start, const Gtk::TextBuffer::iterator &end, int order_in_group);
		virtual ~EraseAction(void);

		virtual void undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void merge(EditAction *action);
		virtual bool getCanMerge(EditAction *action);
		virtual Glib::ustring getData()
		{
			Glib::ustring data = "";
			data.append("[" + m_type + "]:::");
			data.append("text(" + mText + ")");
			data.append(":");
			data.append("index_start(" + number_to_string(m_start) + ")");
			data.append(":");
			data.append("index_end(" + number_to_string(m_end) + ")");
			return data;
		}
		virtual unsigned int getIndex()
		{
			return 0;
		}
		virtual void setData(Glib::ustring value)
		{
			mText = value;
		}

	private:
		Glib::ustring mText;
		int m_start;
		int m_end;
		bool mIsForward;
		bool mIsCut;
};

/*
 *-----------------------------------------------------------------------------
 *
 * tag::TagAction --
 *
 *      An EditAction created on text tag events, for undo/redoable tags
 *
 *-----------------------------------------------------------------------------
 */

class PixAction: public EditAction
{
	public:
		PixAction(const Gtk::TextBuffer::iterator& iter, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, bool pix_add,
		        int order_in_group);
		virtual ~PixAction(void);

		virtual void undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void merge(EditAction *action);
		virtual bool getCanMerge(EditAction *action);
		virtual Glib::ustring getData()
		{
			Glib::ustring data;
			data.append("[" + m_type + "]:::");
			data.append("pos(" + number_to_string(m_pos) + ")");
			data.append(":");
			data.append("pix_add(" + number_to_string(m_pixAdd) + ")");
			return data;
		}
		virtual unsigned int getIndex()
		{
			return 0;
		}
		virtual void setData(Glib::ustring value)
		{
		}

	private:
		Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
		int m_pos;
		bool m_pixAdd;
};

/*
 *-----------------------------------------------------------------------------
 *
 * tag::TagAction --
 *
 *      An EditAction created on text tag events, for undo/redoable tags
 *
 *-----------------------------------------------------------------------------
 */

class TagAction: public EditAction
{
	public:
		TagAction(const Gtk::TextBuffer::iterator &start, const Gtk::TextBuffer::iterator &end, const Glib::RefPtr<
		        Gtk::TextTag>& tag, bool tag_add, int order_in_group);
		virtual ~TagAction(void);

		virtual void undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void merge(EditAction *action);
		virtual bool getCanMerge(EditAction *action);
		virtual Glib::ustring getData()
		{
			if (!m_tag)
				return "[" + m_type + "]:::nothing";
			Glib::ustring data;
			data.append("[" + m_type + "]:::");
			data.append("name(" + m_tag->property_name() + ")");
			data.append(":");
			data.append("start(" + number_to_string(m_start) + ")");
			data.append(":");
			data.append("end(" + number_to_string(m_end) + ")");
			data.append(":");
			if (m_tagAdd)
				data.append("action(ADD)");
			else
				data.append("action(DELETE)");
			return data;
		}
		Glib::RefPtr<Gtk::TextTag> getTag(Glib::RefPtr<tag::AnnotationBuffer>& buffer, bool p_undoMode, bool p_addMode);
		virtual unsigned int getIndex()
		{
			return 0;
		}
		virtual void setData(Glib::ustring value)
		{
		}

	private:
		Glib::RefPtr<Gtk::TextTag> m_tag;
		int m_start;
		int m_end;
		bool m_tagAdd;
};

/*
 *-----------------------------------------------------------------------------
 *
 * tag::MarkAction --
 *
 *      An EditAction created on text mark events (except insert / selection_bound)
 *
 *-----------------------------------------------------------------------------
 */

class MarkAction: public EditAction
{
	public:
		MarkAction(const float old_pos, const Gtk::TextBuffer::iterator &pos, const Glib::RefPtr<Gtk::TextMark>& mark,
		        int mark_action_type, int order_in_group);
		virtual ~MarkAction(void);

		virtual void undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void merge(EditAction *action);
		virtual bool getCanMerge(EditAction *action);
		virtual Glib::ustring getData()
		{
			Glib::ustring data;
			data.append("[" + m_type + "]:::");
			data.append("name(" + m_mark + ")");
			data.append(":");
			if (m_markActionType == 0)
				data.append("actionType(DELETE)");
			else if (m_markActionType == 1)
				data.append("actionType(CREATE)");
			else if (m_markActionType == 2)
			{
				data.append("actionType(MOVE)");
				data.append(":");
				data.append("old_off(" + number_to_string(m_oldpos) + ")");
			}
			data.append(":");
			data.append("off(" + number_to_string(m_pos) + ")");
			data.append(":");
			if (m_left_gravity == 0)
				data.append("gravity(RIGHT)");
			else
				data.append("gravity(LEFT)");
			data.append(":");
			data.append("mark_data(" + m_data + ")");
			return data;
		}
		virtual unsigned int getIndex()
		{
			return 0;
		}
		virtual void setData(Glib::ustring value)
		{
		}

	private:
		string m_mark;
		int m_oldpos;
		int m_pos;
		bool m_left_gravity;
		/* 0: delete, 1: add, 2: move */
		int m_markActionType;
		string m_data;
};

/*
 *-----------------------------------------------------------------------------
 *
 * tag::CustomAction --
 *
 *      An EditAction created on custom action events
 *
 *-----------------------------------------------------------------------------
 */

class CustomAction: public EditAction
{
	public:
		CustomAction(UndoableTextView* parent, const string& data, int order_in_group);
		virtual ~CustomAction(void);

		virtual void undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer);
		virtual void merge(EditAction *action);
		virtual bool getCanMerge(EditAction *action);
		virtual Glib::ustring getData()
		{
			Glib::ustring data = "";
			data.append("[" + m_type + "]:::");
			data.append(m_data);
			return data;
		}
		virtual unsigned int getIndex()
		{
			return 0;
		}
		virtual void setData(Glib::ustring value)
		{
			m_data = value;
		}

	private:
		UndoableTextView* m_parent;
		string m_data;
};

/*=================================================================================*/
/*=================================================================================*/
/*						ACTION TYPES IMPLEMENTATION                                */
/*=================================================================================*/
/*=================================================================================*/

/*
 *-----------------------------------------------------------------------------
 *
 * tag::InsertAction::InsertAction --
 *
 *      Constructor.  Stores the inserted text and buffer offset of this action.
 *      If the inserted text length is greater than 1, denoting a text paste,
 *      this action is unmergable.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

InsertAction::InsertAction(const Gtk::TextBuffer::iterator &start, // IN:
        const Glib::ustring &text, // IN:
        int length, // IN:
        int order_in_group) // IN:
:
	EditAction(order_in_group, "insert"), mText(text), mIndex(start.get_offset() - length), mIsPaste(length > 1) // GTKBUG: No way to tell a 1-char paste.
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::InsertAction::InsertAction --
 *
 *      Destructor.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

InsertAction::~InsertAction(void)
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::InsertAction::undo --
 *
 *      Undo the insert action by deleting the inserted text block beginning at
 *      text buffer offset mIndex and ending at the mIndex plus the inserted
 *      text length.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void InsertAction::undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	if (DOTRACE_UNDOV)
	{
		LOG_FINE
		TRACE_D << "@@@ InsertAction UNDO - " << getData() << " - textinBuffer='" << buffer->get_text(
		        buffer->get_iter_at_offset(mIndex), buffer->get_iter_at_offset(mIndex + mText.length())) << "'" << endl;
	}

	buffer->erase(buffer->get_iter_at_offset(mIndex), buffer->get_iter_at_offset(mIndex + mText.length()));
	m_signalAddToLastIterOffset.emit(mIndex);

	LOG_RESET
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::InsertAction::redo --
 *
 *      Redo the insert action by re-inserting the previously re-inserted text
 *      mText at buffer offset mIndex.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void InsertAction::redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	if (DOTRACE_UNDOV)
	{
		LOG_FINE
		TRACE_D << "@@@ InsertAction REDO - " << getData() << endl;
	}

	// -- Re-insert text
	buffer->place_cursor(buffer->get_iter_at_offset(mIndex));
	Gtk::TextIter it = buffer->insert(buffer->get_iter_at_offset(mIndex), mText);

	// -- Remove propagating tags
	buffer->removePropagatingTags(buffer->get_iter_at_offset(mIndex), it);

	// -- Recheck with speller
/* SPELL */
//	buffer->spellerRecheck(mIndex, it.get_offset()) ;

	m_signalAddToLastIterOffset.emit(mIndex);

	LOG_RESET
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::InsertAction::merge --
 *
 *      Add the text block of the passed action to mText.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void InsertAction::merge(EditAction *action) // IN:
{
	InsertAction *insert = static_cast<InsertAction*> (action);
	mText += insert->mText;

	if (DOTRACE_UNDOV)
	{
		LOG_FINE
		TRACE_D << "Merged InsertAction offset=" << mIndex << " text='" << mText << "'" << endl;
		LOG_RESET
	}
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::InsertAction::getCanMerge --
 *
 *      Decide whether a given EditAction can be merged into this instance.
 *      Checking it is an InsertAction, that neither are paste operations, that
 *      the new action begins at the offset where this action ends, that this
 *      action is not a newline, and that the new action does not begin a new
 *      word.
 *
 * Results:
 *      True if the argument action can be merged with this action.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

bool InsertAction::getCanMerge(EditAction *action) // IN:
{
	InsertAction *insert = dynamic_cast<InsertAction*> (action);
	if (insert)
	{
		// Don't group text pastes
		if (mIsPaste || insert->mIsPaste)
		{
			return false;
		}

		// Must meet eachother
		if (insert->mIndex != mIndex + (int) mText.length())
		{
			return false;
		}

		// Don't group more than one line (inclusive)
		if (mText[0] == '\n')
		{
			return false;
		}

		// Don't group more than one word (exclusive)
		if (insert->mText[0] == ' ' || insert->mText[0] == '\t')
		{
			return false;
		}
		return true;
	}
	return false;
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::EraseAction::EraseAction --
 *
 *      Constructor.  Stores the start and end buffer offsets, and the removed
 *      text of this erase action, and whether is it from a cut operation, and
 *      if the text was deleted using backspace or delete.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

EraseAction::EraseAction(const Gtk::TextBuffer::iterator &start, // IN:
        const Gtk::TextBuffer::iterator &end, // IN:
        int order_in_group) // IN:
:
	EditAction(order_in_group, "erase"), mText(start.get_text(end)), m_start(start.get_offset()), m_end(
	        end.get_offset()), mIsCut(m_end - m_start > 1) // GTKBUG: No way to tell a 1-char cut.
{
	const Gtk::TextIter& cursor = start.get_buffer()->get_insert()->get_iter();
	mIsForward = cursor.get_offset() < m_start;
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::EraseAction::EraseAction --
 *
 *      Destructor.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

EraseAction::~EraseAction(void)
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::EraseAction::undo --
 *
 *      Undo the erase action by inserting the the deleted text block mText
 *      beginning at text buffer offset m_start.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void EraseAction::undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	if (DOTRACE_UNDOV)
	{
		LOG_FINE
		TRACE_D << "@@@ Erase action UNDO text= '" << getData() << "'" << endl;
	}

	// -- See if some anchor needs to be moved after insertion
	Anchor* needLeftGravity = buffer->markNeedsLeftGravity(buffer->get_iter_at_offset(m_start));

	Gtk::TextIter it = buffer->insert(buffer->get_iter_at_offset(m_start), mText);
	buffer->removePropagatingTags(buffer->get_iter_at_offset(m_start), it);

	// -- Recheck with speller
/* SPELL */
//	buffer->spellerRecheck(m_start, m_end) ;

	int cursor_position;
	int update_position;
	if (mIsForward)
	{
		cursor_position = m_start;
		update_position = m_end;
	}
	else
	{
		cursor_position = m_end;
		update_position = m_start;
	}
	// -- Stock one position for update data
	m_signalAddToLastIterOffset.emit(update_position);

	// -- Signal emitted at each undoRedo will actualise the other position data
	buffer->place_cursor(buffer->get_iter_at_offset(cursor_position));

	// -- Restore anchor place if needed
	if ( needLeftGravity )
		buffer->anchors().moveAnchor(needLeftGravity, buffer->get_iter_at_offset(m_start),  false);

	LOG_RESET
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::EraseAction::redo --
 *
 *      Redo the erase action by re-erasing the previously re-inserted text
 *      mText at buffer offset m_start.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void EraseAction::redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	if (DOTRACE_UNDOV)
	{
		LOG_FINE
		TRACE_D << "@@@ Erase action REDO text= '" << getData() << "'" << endl;
	}

	buffer->erase(buffer->get_iter_at_offset(m_start), buffer->get_iter_at_offset(m_end));
	// stock end position for update data
	m_signalAddToLastIterOffset.emit(m_end);
	// signal emitted at each undoRedo will actualise the m_start data
	buffer->place_cursor(buffer->get_iter_at_offset(m_start));

	LOG_RESET
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::EraseAction::merge --
 *
 *      Add the text block of the passed action to mText, and adjust the end or
 *      start buffer offsets to include the added text, depending on whether the
 *      passed action offsets meet this one at the beginning or end.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void EraseAction::merge(EditAction *action) // IN:
{
	EraseAction *erase = static_cast<EraseAction*> (action);
	if (m_start == erase->m_start)
	{
		mText += erase->mText;
		m_end += erase->m_end - erase->m_start;
	}
	else
	{
		mText = erase->mText + mText;
		m_start = erase->m_start;
	}
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::EraseAction::getCanMerge --
 *
 *      Decide whether a given EditAction can be merged into this instance.
 *      Checking it is an EraseAction, that neither are cut operations, that the
 *      new action offsets meet this action's, that this action is not a
 *      newline, and that the new action does not begin a new word.
 *
 * Results:
 *      True if the argument action can be merged with this action.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

bool EraseAction::getCanMerge(EditAction *action) // IN:
{
	EraseAction *erase = dynamic_cast<EraseAction*> (action);
	if (erase)
	{
		// Don't group separate text cuts
		if (mIsCut || erase->mIsCut)
		{
			return false;
		}

		// Must meet eachother
		if (m_start != (mIsForward ? erase->m_start : erase->m_end))
		{
			return false;
		}

		// Don't group deletes with backspaces
		if (mIsForward != erase->mIsForward)
		{
			return false;
		}

		// Don't group more than one line (inclusive)
		if (mText[0] == '\n')
		{
			return false;
		}

		// Don't group more than one word (exclusive)
		if (erase->mText[0] == ' ' || erase->mText[0] == '\t')
		{
			return false;
		}

		return true;
	}
	return false;
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::TagAction::TagAction --
 *
 *      Constructor.  Stores the start and end buffer offsets, and the tag
 *      of this tag action.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

TagAction::TagAction(const Gtk::TextBuffer::iterator &start, // IN:
        const Gtk::TextBuffer::iterator &end, // IN:
        const Glib::RefPtr<Gtk::TextTag> &tag, // IN:
        bool tag_add, // IN:
        int order_in_group) // IN:
:
	EditAction(order_in_group, "tag"), m_tag(tag), m_start(start.get_offset()), m_end(end.get_offset()), m_tagAdd(
	        tag_add)
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::TagAction::~TagAction --
 *
 *      Destructor.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

TagAction::~TagAction(void)
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::TagAction::getTag --
 *
 *      Get the current tag, checking its existence in buffer tagTable.
 *      If it doesn't exist anymore, tag is added
 *
 * Results:
 *      reference on existing or created tag.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

Glib::RefPtr<Gtk::TextTag> TagAction::getTag(Glib::RefPtr<tag::AnnotationBuffer>& buffer, bool p_undoMode,
        bool p_addMode)
{
	const Glib::RefPtr<Gtk::TextTag>& exist = buffer->get_tag_table()->lookup(m_tag->property_name());

	string undoMode, addMode;
	if (p_addMode)
		addMode = "Add";
	else
		addMode = "Remove";

	if (p_undoMode)
		undoMode = "UNDO";
	else
		undoMode = "REDO";

	// tag is not in tagtable
	if (exist == 0)
	{
		if (DOTRACE_UNDOV)
		{
			LOG_FINE
			TRACE_D << "TagAction::getTag " << undoMode << " " << addMode << ":> tag in table [0] -> adding it" << endl;
		}

		buffer->get_tag_table()->add(m_tag);
		//		buffer->addPropagatingTag(m_tag) ;

		LOG_RESET
		return m_tag;
	}
	// already in table
	else
	{
		if (DOTRACE_UNDOV)
		{
			LOG_FINE
			TRACE_D << "TagAction::getTag " << undoMode << " " << addMode << ":> tag in table [1]" << endl;
			LOG_RESET
		}
		return exist;
	}
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::TagAction::undo --
 *
 *      Undo the tag action
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void TagAction::undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	const Glib::RefPtr<Gtk::TextTag>& tag = getTag(buffer, true, m_tagAdd);

	if (DOTRACE_UNDOV)
		LOG_FINE

	if (m_tagAdd)
	{
		if (DOTRACE_UNDOV)
			TRACE_D << "TagAction UNDO create: " << getData() << endl;

		buffer->remove_tag(tag, buffer->get_iter_at_offset(m_start), buffer->get_iter_at_offset(m_end));
	}
	else
	{
		if (DOTRACE_UNDOV)
			TRACE_D << "TagAction UNDO remove: " << getData() << endl;

		//> force segment update in datamodel
		if (m_start - 1 > 0)
			m_signalAddToLastIterOffset.emit(m_start - 1);

		buffer->apply_tag(tag, buffer->get_iter_at_offset(m_start), buffer->get_iter_at_offset(m_end));

		string name = tag->property_name().get_value();
		if ( name.find("qualifier") != string::npos && name.find("_end") != string::npos ) {
			// check if text mark at start -> should not happen -> should be moved after tag.
			Anchor* a = buffer->anchors().getAnchorAtPos(buffer->get_iter_at_offset(m_start), "unit");
			if ( a != NULL )
				a->move(buffer->get_iter_at_offset(m_end));
		}
	}

	LOG_RESET
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::TagAction::redo --
 *
 *      Redo the tag action by re-tagging the previously re-inserted text
 *      mText at buffer offset m_start.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void TagAction::redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	const Glib::RefPtr<Gtk::TextTag>& tag = getTag(buffer, false, m_tagAdd);

	if (DOTRACE_UNDOV)
		LOG_FINE

	if (m_tagAdd)
	{
		if (DOTRACE_UNDOV)
			TRACE_D << "TagAction REDO add: " << getData() << endl;

		//> force segment update in datamodel
		if (m_start - 1 > 0)
			m_signalAddToLastIterOffset.emit(m_start - 1);

		//> apply tag in buffer
		buffer->apply_tag(tag, buffer->get_iter_at_offset(m_start), buffer->get_iter_at_offset(m_end));
	}
	else
	{
		if (DOTRACE_UNDOV)
			TRACE_D << "TagAction REDO remove: " << getData() << endl;

		buffer->remove_tag(tag, buffer->get_iter_at_offset(m_start), buffer->get_iter_at_offset(m_end));
	}

	LOG_RESET
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::TagAction::merge --
 *
 *      Add the text block of the passed action to mText, and adjust the end or
 *      start buffer offsets to include the added text, depending on whether the
 *      passed action offsets meet this one at the beginning or end.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void TagAction::merge(EditAction *action) // IN:
{
	TagAction *tag = static_cast<TagAction*> (action);
	if (tag)
	{
		if (m_start > tag->m_start)
			m_start = tag->m_start;
		if (m_end < tag->m_end)
			m_end = tag->m_end;
	}
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::TagAction::getCanMerge --
 *
 *      Decide whether a given EditAction can be merged into this instance.
 *      Checking it is an TagAction, that both actions concern adjacent
 *      pieces of text.
 *
 * Results:
 *      True if the argument action can be merged with this action.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

bool TagAction::getCanMerge(EditAction *action) // IN:
{
	TagAction *tag = dynamic_cast<TagAction*> (action);
	if (tag)
	{
		// starts and ends near to each other
		bool cond1 = (abs(m_start - tag->m_end) <= 1 || abs(m_end - tag->m_start) <= 1);
		// same type of tag
		bool cond2 = (m_tag->property_name() == tag->m_tag->property_name());
		return (cond1 && cond2);
	}
	else
		return false;
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::PixAction::PixAction --
 *
 *      Constructor.  Stores the start and end buffer offsets, and the tag
 *      of this tag action.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

PixAction::PixAction(const Gtk::TextBuffer::iterator& pos, // IN:
        const Glib::RefPtr<Gdk::Pixbuf>& pix, // IN:
        bool pix_add, // IN:
        int order_in_group) // IN:
:
	EditAction(order_in_group, "pix"), m_pixbuf(pix), m_pos(pos.get_offset()), m_pixAdd(pix_add)
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::PixAction::~PixAction --
 *
 *      Destructor.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

PixAction::~PixAction(void)
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::PixAction::undo --
 *
 *      Undo the tag action
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void PixAction::undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	if (DOTRACE_UNDOV)
		LOG_FINE

	if (m_pixAdd)
	{
		if (DOTRACE_UNDOV)
			TRACE_D << "PixAction UNDO create: " << getData() << endl;

		buffer->erase(buffer->get_iter_at_offset(m_pos - 1), buffer->get_iter_at_offset(m_pos));
	}
	else
	{
		if (DOTRACE_UNDOV)
			TRACE_D << "PixAction UNDO remove: " << getData() << endl;

		buffer->insert_pixbuf(buffer->get_iter_at_offset(m_pos - 1), m_pixbuf);
	}

	LOG_RESET
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::PixAction::redo --
 *
 *      Redo the tag action by re-tagging the previously re-inserted text
 *      mText at buffer offset m_start.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void PixAction::redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	if (DOTRACE_UNDOV)
		LOG_FINE

	if (m_pixAdd)
	{
		buffer->insert_pixbuf(buffer->get_iter_at_offset(m_pos - 1), m_pixbuf);
		if (DOTRACE_UNDOV)
			TRACE_D << "PixAction REDO add: " << getData() << endl;
	}
	else
	{
		buffer->erase(buffer->get_iter_at_offset(m_pos - 1), buffer->get_iter_at_offset(m_pos));
		if (DOTRACE_UNDOV)
			TRACE_D << "PixAction REDO remove: " << getData() << endl;
	}

	LOG_RESET
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::PixAction::merge --
 *
 *      Add the text block of the passed action to mText, and adjust the end or
 *      start buffer offsets to include the added text, depending on whether the
 *      passed action offsets meet this one at the beginning or end.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void PixAction::merge(EditAction *action) // IN:
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::PixAction::getCanMerge --
 *
 *      Decide whether a given EditAction can be merged into this instance.
 *      Checking it is an PixAction, that both actions concern adjacent
 *      pieces of text.
 *
 * Results:
 *      True if the argument action can be merged with this action.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

bool PixAction::getCanMerge(EditAction *action) // IN:
{
	return false;
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::MarkAction::MarkAction --
 *
 *      Constructor.  Stores the start and end buffer offsets, and the tag
 *      of this tag action.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

MarkAction::MarkAction(const float old_pos, const Gtk::TextBuffer::iterator &pos, // IN:
        const Glib::RefPtr<Gtk::TextMark> &mark, // IN:
        int mark_action_type, // IN:
        int order_in_group) // IN:
:
	EditAction(order_in_group, "mark"), m_oldpos(old_pos), m_mark(mark->get_name()), m_pos(pos.get_offset()),
	        m_left_gravity(mark->get_left_gravity()), m_markActionType(mark_action_type)
{
	void *data = mark->get_data("anchor");
	if (data != NULL)
		m_data = (char*) data;
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::MarkAction::~MarkAction --
 *
 *      Destructor.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

MarkAction::~MarkAction(void)
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::MarkAction::undo --
 *
 *      Undo the tag action
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void MarkAction::undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	if (DOTRACE_UNDOV)
		LOG_FINE

	//> ADD mark
	if (m_markActionType == 1)
	{
		if (DOTRACE_UNDOV)
			TRACE_D << "\n ||| MarkAction UNDO create: " << getData() << endl;
		buffer->anchors().deleteAnchor(m_mark);
	}
	//> DELETE mark
	else if (m_markActionType == 0)
	{
		const Glib::RefPtr<Gtk::TextMark>& mark = buffer->get_mark(m_mark);
		if (mark == 0)
		{
			if (DOTRACE_UNDOV)
				TRACE_D << "\n ||| MarkAction UNDO delete: " << getData() << endl;
			buffer->anchors().createAnchor(m_mark, buffer->get_iter_at_offset(m_pos), m_left_gravity, m_data);
		}
		else
		{
			if (DOTRACE_UNDOV)
				TRACE_D << "\n ||| MarkAction UNDO delete: " << getData() << endl;
			buffer->move_mark(mark, buffer->get_iter_at_offset(m_pos));
		}
	}
	//> MOVE mark
	else if (m_markActionType == 2)
	{
		if (DOTRACE_UNDOV)
			TRACE_D << "\n ||| MarkAction UNDO move: " << getData() << endl;
		const Glib::RefPtr<Gtk::TextMark>& mark = buffer->get_mark(m_mark);
		if (m_oldpos != -1 && mark != 0)
			buffer->move_mark(mark, buffer->get_iter_at_offset(m_oldpos));
	}

	LOG_RESET
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::MarkAction::redo --
 *
 *      Redo the tag action by re-tagging the previously re-inserted text
 *      mText at buffer offset m_start.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void MarkAction::redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	if (DOTRACE_UNDOV)
		LOG_FINE

	//> DELETE
	if (m_markActionType == 0)
	{
		//		buffer->delete_mark_by_name(m_mark);
		buffer->anchors().deleteAnchor(m_mark);
		if (DOTRACE_UNDOV)
			TRACE_D << "\n ||| MarkAction REDO delete: " << getData() << endl;
	}
	//> CREATE
	else if (m_markActionType == 1)
	{
		buffer->anchors().createAnchor(m_mark, buffer->get_iter_at_offset(m_pos), m_left_gravity, m_data);
		if (DOTRACE_UNDOV)
			TRACE_D << "\n ||| MarkAction REDO create: " << getData() << endl;
	}
	//> MOVE
	else if (m_markActionType == 2)
	{
		if (DOTRACE_UNDOV)
			TRACE_D << "\n ||| MarkAction REDO delete: " << getData() << endl;
		const Glib::RefPtr<Gtk::TextMark>& mark = buffer->get_mark(m_mark);
		if (mark != 0)
			buffer->move_mark(mark, buffer->get_iter_at_offset(m_pos));
	}

	LOG_RESET
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::MarkAction::merge --
 *
 *      Add the text block of the passed action to mText, and adjust the end or
 *      start buffer offsets to include the added text, depending on whether the
 *      passed action offsets meet this one at the beginning or end.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void MarkAction::merge(EditAction *action) // IN:
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::MarkAction::getCanMerge --
 *
 *      Decide whether a given EditAction can be merged into this instance.
 *      Checking it is an MarkAction, that both actions concern adjacent
 *      pieces of text.
 *
 * Results:
 *      True if the argument action can be merged with this action.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

bool MarkAction::getCanMerge(EditAction *action) // IN:
{
	return false;
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::CustomAction::CustomAction --
 *
 *      Constructor.  Stores the start and end buffer offsets, and the tag
 *      of this tag action.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

CustomAction::CustomAction(UndoableTextView* parent, const string& eventData, // IN:
        int order_in_group) // IN:
:
	EditAction(order_in_group, "custom"), m_parent(parent), m_data(eventData)
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::CustomAction::~CustomAction --
 *
 *      Destructor.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

CustomAction::~CustomAction(void)
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::CustomAction::undo --
 *
 *      Undo the tag action
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void CustomAction::undo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	m_parent->signalCustomUndoRedo().emit(m_data, true);
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::CustomAction::redo --
 *
 *      Redo the tag action by re-tagging the previously re-inserted text
 *      mText at buffer offset m_start.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void CustomAction::redo(Glib::RefPtr<tag::AnnotationBuffer>& buffer) // IN:
{
	m_parent->signalCustomUndoRedo().emit(m_data, false);
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::CustomAction::merge --
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void CustomAction::merge(EditAction *action) // IN:
{
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::CustomAction::getCanMerge --
 *
 *      Decide whether a given EditAction can be merged into this instance.
 *      Checking it is an CustomAction, that both actions concern adjacent
 *      pieces of text.
 *
 * Results:
 *      True if the argument action can be merged with this action.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

bool CustomAction::getCanMerge(EditAction *action) // IN:
{
	return false;
}

/*=================================================================================*/
/*=================================================================================*/
/*						UNDOABLETEXTVIEW IMPLEMENTATION                          */
/*=================================================================================*/
/*=================================================================================*/

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::UndoableTextView --
 *
 *      Constructor.  Connects to insert (after handler) and erase (before
 *      handler) signals so we can track edits.  Connect to populate_popup and
 *      key_press_event (before handler) signals.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

UndoableTextView::UndoableTextView(const Glib::RefPtr<tag::AnnotationBuffer> &buffer) // IN:
:
	Gtk::TextView(buffer), m_buffer(buffer), m_frozenCnt(0), m_userActionCnt(0), m_tryMerge(false),
	        m_actionsInCurrentGroup(0), m_modifiedClearedAt(0), m_editActionGroup(Gtk::ActionGroup::create("edit"))
{
	last_action = -1;
	m_undoableTagPrefix = "";

	//>> TEXT SIGNALS
	get_buffer()->signal_insert().connect(sigc::mem_fun(this, &UndoableTextView::onInsert));
	get_buffer()->signal_erase().connect(sigc::mem_fun(this, &UndoableTextView::onErase), false);

	//>> UNDO/REDO (DES)ACTIVATION
	get_buffer()->signal_begin_user_action().connect(sigc::bind<bool>(sigc::mem_fun(this,
	        &UndoableTextView::onBeginEndUserAction), true), false);
	get_buffer()->signal_end_user_action().connect(sigc::bind<bool>(sigc::mem_fun(this,
	        &UndoableTextView::onBeginEndUserAction), false), false);

	//  ********* TODO a commenter START *******************************************
	// -->  Si on veut que la vue se base sur le data model pour restorer les
	// 		options (modifier les signaux du datamodel)

	//>> TAG SIGNALS
	get_buffer()->signal_apply_tag().connect(sigc::bind<bool>(sigc::mem_fun(this, &UndoableTextView::onTagEvent), true), false);
	get_buffer()->signal_remove_tag().connect(sigc::bind<bool>(sigc::mem_fun(this, &UndoableTextView::onTagEvent), false), false);

	//>> MARK ACTIONS
	//> Re-implement deleted and inserted mark cause with gtk signal either the mark is already deleted when
	//  we receive signal, either datas can't have been set before mark creation
	//	get_buffer()->signal_mark_set().connect_notify(sigc::mem_fun(this, &UndoableTextView::onMarkSetEvent), true);
	//	get_buffer()->signal_mark_deleted().connect_notify(sigc::mem_fun(this, &UndoableTextView::onMarkDeletedEvent), false);
	m_buffer->anchors().signalDeleteAnchor().connect(sigc::mem_fun(this, &UndoableTextView::onMarkDeletedEvent));
	m_buffer->anchors().signalCreateAnchor().connect(sigc::mem_fun(this, &UndoableTextView::onMarkSetEvent));
	m_buffer->anchors().signalMoveAnchor().connect(sigc::mem_fun(this, &UndoableTextView::onMarkMoveEvent));

	//>> PIX ELEMENTS
	m_buffer->signal_insert_pixbuf().connect(sigc::bind<bool>(sigc::mem_fun(this, &UndoableTextView::onPixEvent), true));

	// ********* TODO a commenter END **********************************************

	// DATAMODEL SPECIFIC ACTIONS
	//	m_buffer->signalUndoableAction().connect(sigc::mem_fun(this, &UndoableTextView::onCustomEvent));


	get_buffer()->signal_modified_changed().connect(sigc::mem_fun(this, &UndoableTextView::onModifiedChanged), false);

	createActionGroup();
	//  signal_populate_popup().connect(sigc::mem_fun(this, &UndoableTextView::onPopulatePopup));
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::~UndoableTextView --
 *
 *      Destructor.  Release the memory stored in the undo/redo stacks.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

UndoableTextView::~UndoableTextView(void)
{
	clearUndoHistory();
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::onInsert --
 *
 *      Handler for the insert signal.  Create an InsertAction object and push
 *      it onto the undo stack.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::onInsert(const Gtk::TextBuffer::iterator &start, const Glib::ustring &text, int length) // IN:
{
	if (m_frozenCnt == 0)
	{
		if (m_userActionCnt == 0)
			m_actionsInCurrentGroup = 0;
		addUndoAction(new InsertAction(start, text, text.length(), ++m_actionsInCurrentGroup));
	}

	//-- Caution: be sure to execute the following action AFTER block above
	//   		  because it will create undo actions that must be proceded
	//			  AFTER the insert action
	//TODO 		  connect onInsertAfter to buffer::signal_insert() instead and
	//		      define priority by connection order or priority signal ?
	//			  sexier but more dangerous
	m_buffer->onInsertAfter(start, text, length) ;
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::onErase --
 *
 *      Handler for the erase signal.  Create an EraseAction object and push
 *      it onto the undo stack.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */
void UndoableTextView::onErase(const Gtk::TextBuffer::iterator &start, // IN:
        const Gtk::TextBuffer::iterator &stop) // IN:
{
	if (m_frozenCnt == 0)
	{
		// if tagged data, also store MarkActions
		if (m_userActionCnt == 0)
			m_actionsInCurrentGroup = 0;

		set<string> tagsInRange ;
		Gtk::TextIter iter = start ;
		Gtk::TextIter tmp;
		string name ;
		// -- Check for all iters in range
		while ( iter!=stop  )
		{
			list<Glib::RefPtr<Gtk::TextTag> > tags = const_cast<Gtk::TextBuffer::iterator&> (iter).get_tags();
			list<Glib::RefPtr<Gtk::TextTag> >::iterator it;
			// -- For each one, check alltag
			for (it = tags.begin(); it != tags.end(); ++it)
			{
				name = (*it)->property_name().get_value() ;
				// -- can be undo && Tag has not been checked ? proceeed
				if ( tagsInRange.find(name)==tagsInRange.end() && isUndoableTag(*it) )
				{
					tmp = iter ;
					tmp.forward_to_tag_toggle(*it);
					if (tmp > stop)
						tmp = stop;
					addUndoAction(new TagAction(iter, tmp, *it, false, ++m_actionsInCurrentGroup));
					tagsInRange.insert(name) ;
				}
			}
			iter.forward_char() ;
		}

		addUndoAction(new EraseAction(start, stop, ++m_actionsInCurrentGroup));
	}
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::onTagEvent --
 *
 *      Handler for the tag event signal.  Create a TagAction object and push
 *      it onto the undo/redo stack.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::onTagEvent(const Glib::RefPtr<Gtk::TextTag>& tag, // IN:
        const Gtk::TextBuffer::iterator &start, // IN:
        const Gtk::TextBuffer::iterator &end, bool added)// IN:
{
	if (m_frozenCnt == 0)
	{
		if (m_userActionCnt == 0)
			m_actionsInCurrentGroup = 0;

		if (isUndoableTag(tag)) {
			addUndoAction(new TagAction(start, end, tag, added, ++m_actionsInCurrentGroup));
		}
		/*else if (DOTRACE_UNDOV)
		 TRACE << "{{tag try to enter: " << tag->property_name() << endl ;*/
	}
}

bool UndoableTextView::isUndoableTag(const Glib::RefPtr<Gtk::TextTag>& tag)
{
	const string& tagname = tag->property_name().get_value();
	if ( m_undoableTagPrefix.length() < tagname.length()
			&& tagname.compare(0, m_undoableTagPrefix.length(), m_undoableTagPrefix) == 0 )
		return true;
	std::list<Glib::RefPtr<Gtk::TextTag> >::iterator it;
	for (it = m_undoableTags.begin(); it != m_undoableTags.end() && *it != tag; ++it)
		;
	return (it != m_undoableTags.end());
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::onPixEvent --
 *
 *      Handler for the Pix event signal.  Create a PixAction object and push
 *      it onto the undo/redo stack.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */
void UndoableTextView::onPixEvent(const Gtk::TextBuffer::iterator& pos, const Glib::RefPtr<Gdk::Pixbuf>& pix,
        bool added)
{
	if (m_frozenCnt == 0)
	{
		if (m_userActionCnt == 0)
			m_actionsInCurrentGroup = 0;
		if (isUndoablePix(pix))
			addUndoAction(new PixAction(pos, pix, added, ++m_actionsInCurrentGroup));
		else if (DOTRACE_UNDOV)
			TRACE_D << "{{Pix try to enter: " << endl;
	}
}

bool UndoableTextView::isUndoablePix(const Glib::RefPtr<Gdk::Pixbuf>& Pix)
{
	//TODO if use different type of PIXElEMENT, define which are candidates
	// to undo-redo action
	/*	std::list< Glib::RefPtr<Gdk::Pixbuf> >::iterator it;
	 for ( it = m_undoablePixs.begin(); 	it != m_undoablePixs.end() && *it != Pix; ++it );
	 return ( it != m_undoablePixs.end() );*/
	return true;
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::onMarkEvent --
 *
 *      it onto the undo/redo stack.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::onMarkMoveEvent(float old_pos, const Gtk::TextBuffer::iterator &pos, const Glib::RefPtr<
        Gtk::TextMark>& mark, bool forceDisplayUpdate)
{
	if (DOTRACE_UNDOV)
		LOG_FINE

	if (m_frozenCnt == 0)
	{
		if (m_userActionCnt == 0)
			m_actionsInCurrentGroup = 0;

		if (isUndoableMark(mark))
		{
			addUndoAction(new MarkAction(old_pos, pos, mark, 2, ++m_actionsInCurrentGroup));
		}
		else if (DOTRACE_UNDOV)
			TRACE_D << "((mark try to enter: " << mark->get_name() << endl;
	}

	LOG_RESET
}

void UndoableTextView::onMarkSetEvent(const Gtk::TextBuffer::iterator &pos, const Glib::RefPtr<Gtk::TextMark>& mark)
{
	if (DOTRACE_UNDOV)
		LOG_FINE

	if (m_frozenCnt == 0)
	{
		if (m_userActionCnt == 0)
			m_actionsInCurrentGroup = 0;

		if (isUndoableMark(mark))
		{
			addUndoAction(new MarkAction(-1, pos, mark, true, ++m_actionsInCurrentGroup));
		}
		else if (DOTRACE_UNDOV)
			TRACE_D << "((mark try to enter: " << mark->get_name() << endl;
	}

	LOG_RESET
}

void UndoableTextView::onMarkDeletedEvent(const Glib::RefPtr<Gtk::TextMark>& mark)
{
	if (m_frozenCnt == 0)
	{
		if (m_userActionCnt == 0)
			m_actionsInCurrentGroup = 0;
		bool deleted = mark->get_deleted();
		//> GTk emits mark deleted signals AFTER deletion !
		//  If we'll need for any reason to connect that signal, we must check
		//  if mark still exists. If not, go out !
		if (isUndoableMark(mark) && !deleted)
		{
			const Gtk::TextIter& pos = mark->get_iter();
			addUndoAction(new MarkAction(-1, pos, mark, false, ++m_actionsInCurrentGroup));
		}
	}
}

bool UndoableTextView::isUndoableMark(const Glib::RefPtr<Gtk::TextMark>& mark)
{
	const Glib::ustring& name = mark->get_name();
	// only undo transcriberAG mark
	return (name.compare(0, m_markPrefix.length(), m_markPrefix) == 0);
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::onCustomEvent --
 *
 *      Handler for custom event signals.  Create a CustomAction object and push
 *      it onto the undo/redo stack.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */
void UndoableTextView::doCustomEvent(const string &eventData)
{
	if (m_frozenCnt == 0)
	{
		if (m_userActionCnt == 0)
			m_actionsInCurrentGroup = 0;
		addUndoAction(new CustomAction(this, eventData, ++m_actionsInCurrentGroup));
	}
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::onBeginEndUserAction --
 *
 *      Handler for the begin_user_action signal.
 *       reset nb of actions in current action group
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::onBeginEndUserAction(bool begin)
{
	if (m_frozenCnt == 0 && m_userActionCnt == 0)
		m_actionsInCurrentGroup = 0;
	if (begin)
	{
		m_userActionCnt++;
		//		TRACE_D << "((((*****) USER_ACTION BEGINS !! " << endl ;
	}
	else
	{
		m_userActionCnt--;
		//		TRACE_D << "USER_ACTION ENDS (*****))))" << endl ;
	}
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::onModifiedChanged --
 *
 *      Handler for the begin_user_action signal.
 *       reset nb of actions in current action group
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::onModifiedChanged()
{
	if (m_frozenCnt == 0)
	{
		if (get_buffer()->get_modified() == FALSE)
		{
			// buffer saved
			//  -> any undo action will result in modified buffer
			m_modifiedClearedAt = m_undoStack.size();
		}
		/*  Only used if just text action handled ?
		 *  If activated, make loosing groups each time
		 *  event is received */
		//		m_actionsInCurrentGroup = 0;
		m_tryMerge = false;
		if (DOTRACE_UNDOV)
			TRACE_D << "~~~~~~~~~ SIGNAL MODIFIED !! -> frozen=0 && actionIngroup=0" << m_frozenCnt << endl;
	}
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::addUndoAction --
 *
 *      Adds an EditAction object to the undo stack.  If the new action can
 *      be merged with the stack's top action and we are not currently
 *      merge-locked, we merge the new action into the old one and delete the
 *      new action.  Otherwise we push the new action to the top of the stack.
 *      This also clears the redo buffer.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      The action argument may be deleted.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::addUndoAction(EditAction *action) // IN:
{
	//if (DOTRACE_UNDOV)
	Glib::ustring type = action->getType();

	if (DOTRACE_UNDOV)
	{
		try
		{
			LOG_FINE
			TRACE_D << "------------- ADDUNDOACTION type= " << action->getType() << " - data= '" << action->getData()
					<< "' - order= " << action->orderInGroup() << endl;
		}
		catch (Glib::ConvertError e)
		{
			std::cout << "------------- ADDUNDOACTION type= " << e.what() << std::endl ;
			return ;
		}
	}

	if (m_tryMerge && !m_undoStack.empty())
	{
		EditAction *top = m_undoStack.front();
		if (top->getCanMerge(action))
		{
			m_actionsInCurrentGroup = top->orderInGroup();
			top->merge(action);
			delete action;
			return;
		}
	}
	action->signalAddToLastIterOffset().connect(sigc::mem_fun(*this, &UndoableTextView::onSignalLastIterOffset));
	m_undoStack.push_front(action);

	// Clear redo stack
	resetStack(m_redoStack);

	// Try to merge new incoming actions...
	m_tryMerge = true;

	if (m_undoStack.size() == 1)
	{
		m_undoChangedSignal.emit();
	}
	m_refUndo->set_sensitive(getCanUndo());

	if (DOTRACE_UNDOV)
		LOG_RESET
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::getCanUndo --
 *
 *      Check if there are EditActions in the undo stack.
 *
 * Results:
 *      True if there is at least one undoable action.  False otherwise.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

bool UndoableTextView::getCanUndo(void)
{
	return !m_undoStack.empty();
	//return false;
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::CanRedo --
 *
 *      Check if there are EditActions in the redo stack.
 *
 * Results:
 *      True if there is at least one redoable action.  False otherwise.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

bool UndoableTextView::getCanRedo(void)
{
	return !m_redoStack.empty();
	//return false;
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::undo --
 *
 *      Undo the topmost action in the undo stack, and move it to the redo
 *      stack.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::undo(void)
{
	undoRedo(m_undoStack, m_redoStack, true /*undo*/);
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::redo --
 *
 *      Redo the topmost action in the redo stack, and move it to the undo
 *      stack.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::redo(void)
{
	undoRedo(m_redoStack, m_undoStack, false /*redo*/);
}

#ifdef TEST_ROLLBACK
void
UndoableTextView::rollback(void)
{
	bool need_undo = ( m_actionsInCurrentGroup > 0 );

	while ( m_userActionCnt > 0 )
	{
		get_buffer()->end_user_action();
	}
	if ( need_undo )
	{
		// appel undoRedo avec mode rollback pour ne pas remettre dans le pushTO
		undoRedo(m_redoStack, m_undoStack, true, true /*redo*/);
	}
}
#endif

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::UndoRedo --
 *
 *      Perform an undo or redo operation by popping the topmost undoable action
 *      off the popFrom stack, calling the virtual Undo or Redo member, and
 *      adding the action to the pushTo stack.  If popFrom is now empty or
 *      pushTo is now non-empty, undoChangedSignal is emited.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::undoRedo(ActionStack &popFrom, // IN:
        ActionStack &pushTo, // IN:
        bool isUndo) // IN:
{
	if (!has_focus())
		return;

	if (DOTRACE_UNDOV)
		LOG_FINE

	m_buffer->inhibateEditSignal(true);

	//-- Left gravity mechanism should be done by action themselves, no
	//   undo-redo action. (Only one exception in redo of EraseAction)
	//   ==> block auto mechanism
	bool connectionState = m_buffer->blockLeftGravityConnection(true) ;

	Glib::ustring action_type;
	if (!popFrom.empty())
	{

		int nb_in_group = popFrom.front()->orderInGroup();
		int new_order = 0;

		do
		{
			EditAction *action = popFrom.front();
			action_type = action->getType();
			popFrom.pop_front();

			++m_frozenCnt;

			if (isUndo)
			{
				action->undo(m_buffer);
				if (m_modifiedClearedAt == popFrom.size())
				{
					get_buffer()->set_modified(false);
					signalUndoAllUndone().emit();
				}
			}
			else
				action->redo(m_buffer);

			--m_frozenCnt;

			action->setOrderInGroup(++new_order);
			pushTo.push_front(action);

			// Lock merges until a new undoable event comes in...
			m_tryMerge = false;
		}
		while (!popFrom.empty() && (new_order < nb_in_group));

		if (popFrom.empty() || pushTo.size() == 1)
		{
			m_undoChangedSignal.emit();
		}
	}

	m_undoRedoActionSignal.emit(isUndo);

	if (isUndo && getCanUndo() == false && m_modifiedClearedAt == popFrom.size())
	{
		get_buffer()->set_modified(false);
		signalUndoAllUndone().emit();
	}

	m_buffer->inhibateEditSignal(false);
	m_buffer->blockLeftGravityConnection(connectionState) ;

	if (DOTRACE_UNDOV)
		LOG_RESET

	m_refUndo->set_sensitive(getCanUndo());
	m_refRedo->set_sensitive(getCanRedo());
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::ResetStack --
 *
 *      Delete all the EditActions in the ActionStack argument.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::resetStack(ActionStack &stack) // IN:
{
	//
	//	TRACE_D << "*******************************************************"<< std::endl ;
	//	TRACE_D << "************************* RESET STACK *****************"<< std::endl ;
	//	TRACE_D << "*******************************************************"<< std::endl ;

	while (!stack.empty())
	{
		delete stack.front();
		stack.pop_front();
	}
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::clearUndoHistory --
 *
 *      Delete the actions in the redo and undo stack, and emit
 *      undoChangedSignal.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::clearUndoHistory(void)
{
	resetStack(m_undoStack);
	resetStack(m_redoStack);
	m_undoChangedSignal.emit();
	m_refUndo->set_sensitive(getCanUndo());
	m_refRedo->set_sensitive(getCanRedo());
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::setUndoableActions --
 *
 *      Delete the actions in the redo and undo stack, and emit
 *      undoChangedSignal.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::setUndoableActions(bool b)
{
	if (b)
	{
		--m_frozenCnt;
		if (m_frozenCnt < 0)
			m_frozenCnt = 0;
		//		if (DOTRACE_UNDOV)
		//			TRACE_D << "--------- {{{{ setUndoableActions TRUE frozen=" << m_frozenCnt << endl ;
	}
	else
	{
		++m_frozenCnt;
		//		if (DOTRACE_UNDOV)
		//			TRACE_D << "--------- }}}} setUndoableActions FALSE frozen=" << m_frozenCnt << endl ;
	}
}

void UndoableTextView::enableUndoRedoAction(bool editable)
{
	if (m_refUndo)
		m_refUndo->set_sensitive(editable);

	if (m_refRedo)
		m_refRedo->set_sensitive(editable);
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::addUndoableTag --
 *
 *      Declare a text tag as beeing undo/redo-able.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::addUndoableTag(std::string tagname)
{
	Glib::RefPtr<Gtk::TextTag> tag = get_buffer()->get_tag_table()->lookup(tagname);
	if (tag)
		addUndoableTag(tag);
}

void UndoableTextView::addUndoableTag(const Glib::RefPtr<Gtk::TextTag>& tag)
{
	m_undoableTags.push_back(tag);
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::onPopulatePopup --
 *
 *      populate_popup signal handler.  Prepend Undo and Redo menu items,
 *      followed by a separator, to the TextView's right click menu.  Display
 *      their accelerators as Ctrl-z for Undo, and Ctrl-Shift-z as Redo, and
 *      set their sensitivity.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void UndoableTextView::onPopulatePopup(Gtk::Menu *menu) // IN:
{
	Gtk::MenuItem *mitem;

	//  TRACE_UNDO << "UndoableTextView::onPopulatePopup " << endl;

	mitem = Gtk::manage(new Gtk::SeparatorMenuItem());
	mitem->show();
	menu->prepend(*mitem);

	mitem = m_refRedo->create_menu_item();
	mitem->show();
	menu->prepend(*mitem);
	mitem->set_sensitive(getCanRedo());

	mitem = m_refUndo->create_menu_item();
	mitem->show();
	menu->prepend(*mitem);
	mitem->set_sensitive(getCanUndo());
}

/*
 *   create undo/redo action group
 */
void UndoableTextView::createActionGroup()
{
	//	m_refUndo = Gtk::Action::create("edit_undo", Gtk::Stock::UNDO, _("_Undo"), _("Undo last edit action"));
	m_refUndo = Gtk::Action::create("edit_undo", Gtk::Stock::UNDO);
	m_editActionGroup->add(m_refUndo, Gtk::AccelKey("<control>z"), sigc::mem_fun(*this, &UndoableTextView::undo));
	m_refUndo->set_tooltip(_("Undo last action"));

	//	m_refRedo = Gtk::Action::create("edit_redo", Gtk::Stock::REDO, _("_Redo"), _("Undo last edit action"));
	m_refRedo = Gtk::Action::create("edit_redo", Gtk::Stock::REDO);
	m_editActionGroup->add(m_refRedo, Gtk::AccelKey("<control>y"), sigc::mem_fun(*this, &UndoableTextView::redo));
	m_refRedo->set_tooltip(_("Redo last action"));

	m_refUndo->set_sensitive(false);
	m_refRedo->set_sensitive(false);
}

void UndoableTextView::onSignalLastIterOffset(guint32 offset)
{
	m_lastIterOffset.push(offset);
}

/*
 *-----------------------------------------------------------------------------
 *
 * tag::UndoableTextView::printStack --
 *
 *      Debug method: print first stack action data and stack size
 *
 * Results:
 *     	Data of all actions in stack
 *     	(if undo=true, undo stack, otherwise redo stack).
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */
void UndoableTextView::print_stack(bool undo)
{
	std::deque<EditAction *>::iterator it;
	int size;

	if (undo)
	{
		size = m_undoStack.size();
		TRACE_D << "------------ UNDO stack size (" << size << ")" << endl;
		for (it = m_undoStack.begin(); it != m_undoStack.end(); it++)
			TRACE_D << "> " << (*it)->getData() << std::endl;
	}
	else
	{
		size = m_redoStack.size();
		TRACE_D << "------------ REDO stack size (" << size << ")" << endl;
		for (it = m_redoStack.begin(); it != m_redoStack.end(); it++)
			TRACE_D << "> " << (*it)->getData() << std::endl;
	}
}

} // namespace tag
