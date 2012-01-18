/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @file AnnotationBuffer.cpp
 * @brief Annotation buffer widget implementation
 *
 *  handles annotations textual representation
 *
 */
#include "AnnotationBuffer.h"

#include <iostream>
#include <sstream>
#include <stdlib.h>

#include "AnnotationView.h"
#include "AnnotationEditor.h"
#include "Common/InputLanguageArabic.h"
#include "Common/ColorsCfg.h"

#define INVALID(a,b,c) if (a) { Log::err() << "** (" << __FILE__ << "," << __LINE__ << ") : " << b << " " << c << endl; return; }

using namespace std;

#define ANNOT_MAX_UNDO_LEVEL  100
#define DOTRACE_ANNOTBUFFER true
#define WORD_DELIMS " \n.,:!?'"

#define  NO_TAG (Glib::RefPtr<Gtk::TextTag>)0

namespace tag {

AnnotationBuffer::AnnotationBuffer() :
	m_hasSelection(false)
{
	cursorOffset = 0;
/* SPELL */
//	m_nospellTag = (Glib::RefPtr<Gtk::TextTag>)0;
	m_labelTag = (Glib::RefPtr<Gtk::TextTag>) 0;
	m_confidenceTag = (Glib::RefPtr<Gtk::TextTag>) 0;
	m_inhibSignal = false;
/* SPELL */
//	m_speller = NULL;
	m_view = NULL;
	//activate to TRUE if wanted to take the turn language by default
	m_language_change_activated = false;
	m_inhibSpaceHandling = false;
	m_unitType = "";
	m_segmentType = "";
	specialPasteInitialized = false;
}

AnnotationBuffer::~AnnotationBuffer()
{
}

void AnnotationBuffer::clearBuffer()
{
	m_anchors.clear();
	clear();
}

/*========================================================================
 *
 * initialize buffer : tags and current annotation conventions
 *
 =======================================================================*/

void AnnotationBuffer::configure(AnnotationView* view)
{
	m_leftGravityCandidate = NULL;
	m_leftGravityMark = create_mark("leftGravityMarkTmp", begin(), true);
	m_leftGravityTextMode = false;
	m_leftGravityUserBlok = false;
	m_interactiveInsert = false;

	m_view = view;

	configureConventions();
	m_anchors.setDebugMode(m_view->getParent().isDebugMode());

	string tagname;

	/* SPELL */
	//	m_nospellTag = get_tag_table()->lookup(GTKSPELL_NOCHECK_TAG);  // le tag NOSPELL
	//	if ( m_nospellTag  == 0 ) MSGOUT << " Pas de nospell tag !! " << endl;

	m_tagPrefix["background"] = "background";
	m_tagPrefix["background_end"] = "background";

	Glib::RefPtr<Gtk::TextTag> tag = get_tag_table()->lookup("label");
	if (tag != 0)
		return; // Marks & tags already created

	m_backup[0] = create_mark("backup_insert0", get_insert()->get_iter(), false);
	m_backup[1] = create_mark("backup_insert1", get_insert()->get_iter(), false);
	m_backup[2] = create_mark("backup_insert2", get_insert()->get_iter(), false);
	m_startMark = create_mark("upd_start", get_insert()->get_iter(), true);
	m_endMark = create_mark("upd_end", get_insert()->get_iter(), false);

	cursorOffset = 0;

	m_defaultSegmentTagColor = "Grey";

	// labels -> non editable items
	tag = create_tag("label");
	tag->property_editable().set_value(false);
	view->addUndoableTag(tag->property_name().get_value());
	m_labelTag = tag;
	m_propagatingTags.push_back(m_labelTag);

	// data type
	//  hidden data associated to turns/segments
	//	tag = create_tag("segdata");
	//	tag->property_editable().set_value(false);
	//	tag->property_invisible().set_value(true);
	//	view->addUndoableTag(tag->property_name().get_value());


	//>> ------------------------------------------------- FONT

	//> special label font
	string key = "Fonts-editor,label";
	string font = view->getColorsCfgOption(key);
	setFontStyle(font, "label");

	//> labels that have to get the same size than text in view
	key = "Fonts-editor,text";
	font = view->getColorsCfgOption(key);
	setFontStyle(font, "text");

	m_connectionInsertBefore = signal_insert().connect(sigc::mem_fun(*this, &AnnotationBuffer::onInsertBefore), false);
}

void AnnotationBuffer::configureConventions()
{
	const string& graphtype = m_view->getGraphType();

	m_anchors.setOrderedTypes(m_view->getDataModel().getMainstreamTypes(graphtype));
	m_idTagPrefix = m_view->getDataModel().getAGPrefix();
	m_view->setUndoableTagPrefix(m_idTagPrefix);
	m_unitType = m_view->getDataModel().mainstreamBaseType(graphtype);
	m_segmentType = m_view->getDataModel().segmentationBaseType(graphtype);
	m_spaceHandling = m_view->getDataModel().conventions().automaticSpaceHandling();
	m_spaceBordering = m_view->getDataModel().conventions().spaceBorderingForced();
	m_borderedChars = m_view->getDataModel().conventions().getSpaceBorderedChars();
	const vector<string>& v = m_view->getDataModel().conventions().getQualifierTypes("entity");
	vector<string>::const_iterator itv;
	m_entities.clear();
	for (itv = v.begin(); itv != v.end(); ++itv)
		m_entities.insert(*itv);

	m_leftGravityTags.push_back(m_unitType);
	m_leftGravityTags.push_back("qualifier_");
}

void AnnotationBuffer::createHighlightTags()
{
	// -- main track
	Glib::RefPtr<Gtk::TextTag> tag = create_tag("track0");
	tag->property_left_margin().set_value(0);
	m_view->addUndoableTag(tag->property_name().get_value());

	// -- second track in merged view l2r
	tag = create_tag("track1");
	tag->property_left_margin().set_value(30);
	tag->property_background_full_height().set_value(true);
	tag->property_paragraph_background().set_value("LightGrey");
	m_view->addUndoableTag(tag->property_name().get_value());

	// FOR r2l overlap
	tag = create_tag("track2");
	tag->property_right_margin().set_value(30);
	m_view->addUndoableTag(tag->property_name().get_value());

	// FOR l2r overlap
	tag = create_tag("track3");
	tag->property_left_margin().set_value(30);
	m_view->addUndoableTag(tag->property_name().get_value());

	// -- second track in merged view r2l
	tag = create_tag("track4");
	tag->property_right_margin().set_value(30);
	tag->property_background_full_height().set_value(true);
	tag->property_paragraph_background().set_value("LightGrey");
	m_view->addUndoableTag(tag->property_name().get_value());

	tag = create_tag("segment_highlight1");
	Glib::ustring color_bg = getLabelLook(TAG_COLORS_BUFFER_HIGHLIGHT1_SEGMENT, "bg", 40000);
	if (!color_bg.empty())
		tag->property_background().set_value(color_bg);
	tag->property_background_full_height().set_value(true);
	tag->property_paragraph_background().set_value(color_bg);
	tag->set_property("paragraph_background", color_bg);
	m_sameTextDisplayTags.push_back(tag);
	tag->set_priority(0);

	tag = create_tag("segment_highlight2");
	color_bg = getLabelLook(TAG_COLORS_BUFFER_HIGHLIGHT2_SEGMENT, "bg", 15000);
	if (!color_bg.empty())
		tag->property_background().set_value(color_bg);
	tag->property_background_full_height().set_value(true);
	tag->property_paragraph_background().set_value(color_bg);
	tag->set_property("paragraph_background", color_bg);
	m_sameTextDisplayTags.push_back(tag);
	tag->set_priority(1);

	tag = create_tag("unit_highlight1");
	color_bg = getLabelLook(TAG_COLORS_BUFFER_HIGHLIGHT1_SEGMENT, "bg");
	if (!color_bg.empty())
		tag->property_background().set_value(color_bg);
	tag->set_property("background-full-height", true);
	tag->property_paragraph_background().set_value(color_bg);
	m_sameTextDisplayTags.push_back(tag);
	tag->set_priority(2);

	tag = create_tag("unit_highlight2");
	color_bg = getLabelLook(TAG_COLORS_BUFFER_HIGHLIGHT2_SEGMENT, "bg");
	if (!color_bg.empty())
		tag->property_background().set_value(color_bg);
	tag->set_property("background-full-height", true);
	m_sameTextDisplayTags.push_back(tag);
	create_mark("unit_highlight1", get_insert()->get_iter(), true);
	create_mark("unit_highlight2", get_insert()->get_iter(), true);
	tag->set_priority(3);
}

void AnnotationBuffer::configureMainTags()
{
	// -- Time coded tag
	string color_bg = getLabelLook(TAG_COLORS_BUFFER_TIMESTAMP_BACKGROUND, "");
	string color_fg = getLabelLook(TAG_COLORS_BUFFER_TIMESTAMP_FOREGROUND, "");

	if (!color_bg.empty() || !color_fg.empty())
	{
		m_timestampTag = create_tag("timestamp");

		if (!color_bg.empty())
			m_timestampTag->property_background().set_value(color_bg);
		if (!color_fg.empty())
		{
			m_timestampTag->property_foreground().set_value(color_fg);
			m_timestampTag->set_property("underline", Pango::UNDERLINE_DOUBLE);
			m_timestampTag->set_property("underline-set", true);
		}
		m_sameTextDisplayTags.push_back(m_timestampTag);
		m_propagatingTags.push_back(m_timestampTag);
		m_view->addUndoableTag(m_timestampTag->property_name().get_value());
	}

	// -- Editable tag
	m_editableTag = create_tag("force_editable");
	m_editableTag->property_editable().set_value(true);
	m_view->addUndoableTag(m_editableTag->property_name().get_value());

	// -- Confidence tag
	m_confidenceTag = create_tag("high_confidence");
	m_sameTextDisplayTags.push_back(m_confidenceTag);
	m_confidenceTag->property_weight().set_value(from_CfgWeight_to_Pango("bold"));
	m_view->addUndoableTag(m_confidenceTag->property_name().get_value());

	// -- Highlight tag
	createHighlightTags();
}

Glib::RefPtr<Gtk::TextTag> AnnotationBuffer::createAnnotationTag(const string& name, const string &tagclass,
        bool undoable, const string& color_fg, const string& color_bg, const string& style, const string& weight,
        unsigned long flags)
{
	Glib::RefPtr<Gtk::TextTag> tag;
	//gen
	tag = create_tag(name);
	m_tagPrefix[name] = tagclass;

	if (!color_fg.empty())
		tag->property_foreground().set_value(color_fg);
	if (!style.empty())
		tag->property_style().set_value(from_CfgStyle_to_Pango(style));
	if (!weight.empty())
		tag->property_weight().set_value(from_CfgWeight_to_Pango(weight));
	if (!color_bg.empty() && !(flags & INHIBATE_BACKGROUND_TAG))
		tag->property_background().set_value(color_bg);

	tag->property_editable().set_value(false);

	if (undoable)
		m_view->addUndoableTag(tag->property_name().get_value());

	if (flags & DO_UNDERLINE_TAG)
	{
		tag->set_property("underline", Pango::UNDERLINE_DOUBLE);
		tag->set_property("underline-set", true);
	}

	if (flags & IS_TEXT_EMBEDDED)
	{
		m_propagatingTags.push_back(tag);
		m_sameTextDisplayTags.push_back(tag);
	}
	else
		m_labelDisplayTags.push_back(tag);

	if (flags & IS_ACTIVE_TAG)
	{
		//TRACE_D << " SetActiveTag " << name <<" "<< tagclass << endl;
		setActiveTag(tag, tagclass);
	}

	m_newlineBefore[tagclass] = (flags & NEWLINE_BEFORE_TAG);
	m_newlineAfter[tagclass] = (flags & NEWLINE_AFTER_TAG);

	return tag;
}

void AnnotationBuffer::printTagName(const Glib::RefPtr<Gtk::TextTag>& tag)
{
	TRACE << " TAG = " << tag->property_name().get_value() << endl;
}

void AnnotationBuffer::setActiveTag(Glib::RefPtr<Gtk::TextTag>& tag, const string& tagclass)
{
	tag->signal_event().connect(sigc::bind<string>(sigc::mem_fun((*this), &AnnotationBuffer::emitTagEvent), tagclass));
	m_activeTags[tag->property_name().get_value()] = tagclass;
}

bool AnnotationBuffer::emitTagEvent(const Glib::RefPtr<Glib::Object>& obj, GdkEvent* ev, const Gtk::TextIter& iter,
        string tagclass)
{
	m_signalTagEvent.emit(tagclass, ev, iter);
	return true;
}

string AnnotationBuffer::getActiveTagClass(const Gtk::TextIter& iter)
{
	const list<Glib::RefPtr<const Gtk::TextTag> >& tags = iter.get_tags();
	list<Glib::RefPtr<const Gtk::TextTag> >::const_iterator it;

	for (it = tags.begin(); it != tags.end(); ++it)
	{
		const string& name = (*it)->property_name().get_value();
		if (m_activeTags.find(name) != m_activeTags.end())
			return m_activeTags[name];
	}
	return "";
}

//**LG3
void AnnotationBuffer::print_hidden_chars(const Glib::ustring& txt, const Glib::ustring& normal_chars, bool beep)
{
	int cpt = 0;
	Glib::ustring text;
	if (txt.compare("") == 0)
		text = get_text(false);
	else
		text = txt;
	Glib::ustring::iterator ite;
	TRACE_D << "<<********************print_hidden_chars () : START ********" << std::endl;
	for (ite = text.begin(); ite != text.end(); ite++)
	{
		if ((cpt % 85) == 0 && cpt != 0)
			TRACE_D << endl;
		cpt++;
		if (InputLanguageArabic::is_presentation_character(*ite))
		{
			TRACE_D << "[" << hex << (*ite) << dec << "]";
			if (beep)
				gdk_beep();
		}
		else if (!g_unichar_isprint(*ite))
		{
			TRACE_D << "?" << hex << (*ite) << dec << "?";
			if (beep)
				gdk_beep();
		}
			else
			TRACE_D << normal_chars;
	}
	TRACE_D << std::endl;
	TRACE_D << "********************print_hidden_chars () : END ********>>\n" << std::endl;

}

void AnnotationBuffer::print_buffer(bool hexa, const Glib::ustring& txt)
{
	TRACE << "\nSTART Buffer" << std::endl;
	Glib::ustring::const_iterator it;
	for (it = txt.begin(); it != txt.end(); it++)
	{
		if (hexa)
			TRACE << hex << *it << dec << " - " << std::endl;
			else
			TRACE << Glib::ustring(1, *it) << " - " << std::endl;
	}
	TRACE << "END Buffer\n" << std::endl;
}

void AnnotationBuffer::check_presentation_chars_in_buffer()
{
	gunichar space = 0x0020;
	gunichar carriage = 0x000D;
	gunichar lineFeed = 0x000A;

	const Glib::ustring& txt = get_text(true);
	Glib::ustring::const_iterator it;
	for (it = txt.begin(); it != txt.end(); it++)
	{
		if (InputLanguageArabic::is_presentation_character(*it))
			TRACE_D << hex << *it << dec << " - "; //<<  std::endl ;
		else if (g_unichar_isspace(*it))
		{
			if ((*it) == carriage || (*it) == lineFeed)
				TRACE_D << "\n" << endl;
			else if ((*it) == space)
				TRACE_D << "SPACE - "; //<<  std::endl ;

				else
				TRACE_D << "otherSPACE=" << *it << " - "; //<<  std::endl ;
		}
		else
		{
			TRACE_D << Glib::ustring(1, *it) << " - ";// << std::endl ;
		}
	}
	TRACE_D << endl;
}

/**
 * Set the speller that the parent AnnotationView is using
 * @param spell			Pointer on the speller used by the parent
 */
/* SPELL */
/*
 void AnnotationBuffer::setSpeller(GtkSpell* spell)
 {
 m_speller = spell;
 if ( m_view && m_view->getWithConfidence() )
 setSpellerConfidence(true);
 else
 setSpellerConfidence(false) ;
 }
 */

/* SPELL */
/*
 void AnnotationBuffer::setSpellerConfidence(bool value)
 {
 if ( !m_speller || m_confidenceTag == (Glib::RefPtr<Gtk::TextTag>)0 )
 return ;

 Log::out() << "~~~~ speller confidence mode [" << value << "]" << std::endl ;

 if (value)
 gtkspell_set_confidence_tag(m_speller, m_confidenceTag->property_name().get_value().c_str());
 else
 gtkspell_set_confidence_tag(m_speller, NULL) ;
 }
 */

/* SPELL */
/*
 void AnnotationBuffer::spellerRecheck(float off_start, float off_end)
 {
 if ( !m_speller || off_end<0 || off_start<0 || off_start==off_end)
 return ;

 Gtk::TextIter start = get_iter_at_offset(off_start) ;
 Gtk::TextIter end = get_iter_at_offset(off_end) ;

 gtkspell_recheck_range(m_speller, start.gobj(), end.gobj());
 }
 */

/*========================================================================
 *
 * Annotations managment
 *
 ========================================================================*/

bool AnnotationBuffer::blockLeftGravityConnection(bool block)
{
	bool connectionState = m_connectionInsertBefore.block(block);
	return connectionState;
}

/*
 * insert anchored label at cursor position
 */

const string& AnnotationBuffer::insertAnchoredLabel(const string& type, const string& id, const Glib::ustring& label,
        const string & tagname, int track, bool r2l, const string& alignId)
{
	begin_user_action();

	bool connectionState = blockLeftGravityConnection(true);

	bool prev_inhib = inhibateEditSignal(true);
	inhibateSpaceHandling(true);

	//	const vector<Anchor*>& aset = anchors().getAnchorsAtPos(get_insert()->get_iter(), "", false);

	addNewline(type, alignId);

	Gtk::TextIter start = get_insert()->get_iter();
	Gtk::TextIter pos;
	move_mark(m_startMark, get_insert()->get_iter());

	if (!label.empty())
	{
		/* SPELL */
		//		if ( m_speller != NULL ) {
		//			gtkspell_inhibate_check(m_speller, true);
		//		}
		Glib::ustring label2Add = "";

		if (r2l)
			label2Add += Glib::ustring(1, InputLanguageArabic::RLM);
		label2Add += label;
		if (r2l)
			label2Add += Glib::ustring(1, InputLanguageArabic::RLM);

		//		pos = insert_with_tag(start, label2Add, m_labelTag);
		pos = insert(start, label2Add);

		const Gtk::TextIter& prevstart = m_startMark->get_iter();
		if (!tagname.empty())
		{
			const Glib::RefPtr<Gtk::TextTag>& tag = get_tag_table()->lookup(tagname);
			if (tag != 0)
				apply_tag(tag, prevstart, pos);
		}
		/* SPELL */
		//		if ( m_nospellTag != 0 )
		//			apply_tag(m_nospellTag, prevstart, pos);

		if (m_newlineAfter[type])
			pos = insert_with_tag(pos, "\n", m_labelTag);

		/* SPELL */
		//		if ( m_speller != NULL ) {
		//			//  spell error tag may have propagate -> remove it
		//			gtkspell_recheck_range(m_speller, m_startMark->get_iter().gobj(), pos.gobj());
		//			gtkspell_inhibate_check(m_speller, false);
		//		}

		addTrackTag(m_startMark->get_iter(), pos, track);
	}

	anchors().createAnchor(id, type, m_startMark->get_iter(), /*label.empty()*/false, true, false, true);

	inhibateEditSignal(prev_inhib);

	inhibateSpaceHandling(false);

	blockLeftGravityConnection(connectionState);
	end_user_action();
	return id;
}

/*
 *  insert text at cursor position
 */
void AnnotationBuffer::insertText(const Glib::ustring& text)
{
	if (!text.empty())
	{
		const Gtk::TextIter& pos = get_insert()->get_iter();
		guint curoff = pos.get_offset();

		/* SPELL */
		//		if ( m_speller != NULL ) gtkspell_inhibate_check(m_speller, true);

		const Gtk::TextIter& endpos = AnnotationBuffer::insert(pos, text);

		const Gtk::TextIter& startpos = get_iter_at_offset(curoff);
		removePropagatingTags(startpos, endpos);
		/* SPELL */
		//		if ( m_speller != NULL ) {
		//			gtkspell_inhibate_check(m_speller, false);
		//			gtkspell_recheck_range(m_speller, startpos.gobj(), endpos.gobj());
		//		}
	}
}

/*
 *  insert text at cursor position
 * insert surrounding blanks if required
 */
void AnnotationBuffer::insertWord(const Glib::ustring& word)
{
	if (!word.empty())
	{
		string text("");
		Gtk::TextIter iter = get_insert()->get_iter();
		iter.backward_char();
		if (!g_unichar_isspace(iter.get_char()))
			text = " ";
		text += word;
		iter = get_insert()->get_iter();
		if (!g_unichar_isspace(iter.get_char()))
			text += " ";
		insertText(text);
	}
}

/*
 *  insert turn text at cursor position
 */
Gtk::TextIter AnnotationBuffer::replaceText(const Gtk::TextIter& start, const Gtk::TextIter& stop,
        const Glib::ustring& text, bool user_action)
{
	if (text.empty())
		return start;

	if (user_action)
		begin_user_action();

	bool prev_inhib = inhibateEditSignal(true);

	// -- Keep positions
	Glib::RefPtr<Gtk::TextMark> mstart = create_mark("replaceText_startTmp", start, true);
	Glib::RefPtr<Gtk::TextMark> mstop = create_mark("replaceText_stopTmp", stop, true);

	Anchor* markNeedLeftG = NULL;
	bool erasingSel = false;

	// -- Erasing an existing selection
	if (start.compare(stop) != 0)
	{
		setCursor(stop, false);
		erasingSel = true;
	}
	// -- Inserting at a position
	else
	{
		setCursor(start, false);
		//		markNeedLeftG = markNeedsLeftGravity(start) ;
	}

	bool connectionState = false;
	if (erasingSel)
		connectionState = blockLeftGravityConnection(true);

	// -- Insert text
	// Note: insert text before avoiding anchor superposition cases
	insertText(text);

	if (erasingSel)
		blockLeftGravityConnection(connectionState);

	// -- Erase selection if needed
	const Gtk::TextIter& iter_start = mstart->get_iter();
	const Gtk::TextIter& iter_stop = mstop->get_iter();
	if (erasingSel)
	{
		clearSelection();
		erase_interactive(iter_start, iter_stop);
	}

	const Gtk::TextIter& myend = getCursor();

	if (user_action)
		end_user_action();

	inhibateEditSignal(prev_inhib);

	//> compute end of text
	m_signalHasEdits.emit(myend, 10);
	delete_mark(mstart);
	delete_mark(mstop);

	return myend;
}

void AnnotationBuffer::insertAnchoredTaggedText(const string& type, const string& id, const Glib::ustring& text,
        bool do_enhance, bool isAnchored, bool isText, const Glib::ustring& label, const string& nextId)
{
	bool connectionState = blockLeftGravityConnection(true);

	//> 1 -- Keep offset position
	move_mark(m_startMark, get_insert()->get_iter());

	Anchor* markNeedLeftG = NULL;

	//> 2 -- Insert tag
	if (!label.empty())
	{
		list<Glib::RefPtr<Gtk::TextTag> > tags;
		prepareElementTags(id, type, true, tags);

		Gtk::TextIter pos = get_insert()->get_iter();
		/* SPELL */
		//		if ( m_speller != NULL ) {
		//			gtkspell_inhibate_check(m_speller, true);
		//		}

		//2.1 -- Check if a mark needs to be moved after insertion
		/* If we're inserting a tag that will represent an element
		 * (will be followed by a mark-anchor or is followed if we're processing an update)
		 * don't deplace the following anchor !
		 */
		markNeedLeftG = markNeedsLeftGravity(pos, nextId);
		if (pos != end())
		{
			// get rid of eventual unwanted spaces at pos
			Gtk::TextIter it = pos;
			while (markNeedLeftG == NULL && it.editable() && g_unichar_isspace(it.get_char()))
			{
				if (!it.forward_char())
					break;
				markNeedLeftG = markNeedsLeftGravity(it, nextId);
			}
			if (pos.compare(it) != 0)
				pos = erase(pos, it);
		}
		pos = insert_with_tags(pos, label, tags);

		if (isAnchored)
		{
			Gtk::TextIter it = m_startMark->get_iter();
			it++;
			setTimestamp(m_startMark->get_iter(), it, true);
		}

		/* SPELL */
		//		if ( m_speller != NULL ) {
		//			gtkspell_recheck_range(m_speller, m_startMark->get_iter().gobj(), pos.gobj());
		//			gtkspell_inhibate_check(m_speller, false);
		//		}

		//2.2 -- Move mark if needed
		if (markNeedLeftG)
		{
			anchors().moveAnchor(markNeedLeftG, m_startMark->get_iter(), false);
			markNeedLeftG = NULL;
		}
	}

	//> 3 -- Insert Text
	if (!text.empty())
	{
		/* Specific case: 
		 * Paste text unit => unit renderer => insertion text type unit with no label
		 * If the past is done in an empty segment, since we've block left gravity : anchor superposition !
		 * In this case, check out for left gravity candidate
		 */
		if (specialPasteInitialized && label.empty())
			markNeedLeftG = markNeedsLeftGravity(get_insert()->get_iter(), nextId);

		// -- Insert
		insertText(text);

		// -- Adjust mark if needed
		if (markNeedLeftG)
		{
			anchors().moveAnchor(markNeedLeftG, m_startMark->get_iter(), false);
			//reset after 1st use, only needed at first elemet pasted
			specialPasteInitialized = false;
			markNeedLeftG = NULL;
		}

		// -- Confidence tag
		if (do_enhance)
		{
			const Gtk::TextIter& start = m_startMark->get_iter();
			const Gtk::TextIter& end = get_insert()->get_iter();
			apply_tag(m_confidenceTag, start, end);
		}
	}

	//> 4 -- Create anchor
	// TODO -> voir ici si necessaire de passer with_attachment à true si isAnchored
	anchors().createAnchor(id, type, m_startMark->get_iter(), false, isAnchored, isText, !isText);

	blockLeftGravityConnection(connectionState);
}

/*
 * clear highlight for item type
 * @param type item type (turn/word/..)
 */
void AnnotationBuffer::clearHighlight(const string& type, int track)
{
	if (!type.empty())
	{
		//TRACE << "AnnotationBuffer::clearHighlight TYPE:" << type << " - track:" << track << endl;
		string tagname;
		if (track == 0 || track == 2)
			tagname = type + "_highlight1";
		else if (track == 1 || track == 2)
			tagname = type + "_highlight2";
		const Glib::RefPtr<Gtk::TextTag>& tagHighlight = get_tag_table()->lookup(tagname);
		if (!tagHighlight)
		{
			return; // not highlight-able
		}
		remove_tag(tagHighlight, begin(), end());
	}
	else
	{
		//TRACE << "AnnotationBuffer::clearHighlight NOTYPE: - track:" << track << endl;
		// ICI TODO BROWSE THROUGH TAG TABLE
		get_tag_table()->foreach(sigc::bind<int>(sigc::mem_fun(*this, &AnnotationBuffer::clearHighlightTag), track));
	}
}

void AnnotationBuffer::clearHighlightTag(const Glib::RefPtr<Gtk::TextTag>& tag, int track)
{
	if (track == 0 || track == 3 || track == 2)
	{
		if (strstr(tag->property_name().get_value().c_str(), "_highlight1") != NULL)
		{
			remove_tag(tag, begin(), end());
		}
	}
	if (track == 1 || track == 3 || track == 2)
	{
		if (strstr(tag->property_name().get_value().c_str(), "_highlight2") != NULL)
		{
			remove_tag(tag, begin(), end());
		}
	}
}

void AnnotationBuffer::clearTag(const string& tagname)
{
	const Glib::RefPtr<Gtk::TextTag>& tag = get_tag_table()->lookup(tagname);
	if (tag != 0)
		remove_tag(tag, begin(), end());
}

/*
 * highlight current item
 * @param type item type (turn/word/..)
 * @param start item approximate start pos in buffer
 */
void AnnotationBuffer::setHighlight(const string& type, Gtk::TextIter& pos, int track)
{
	INVALID( (type.empty()), "empty type ! ", type );

	string tagname;
	if (track == 0)
		tagname = type + "_highlight1";
	else if (track == 1)
		tagname = type + "_highlight2";

	const Glib::RefPtr<Gtk::TextTag>& tagHighlight = get_tag_table()->lookup(tagname);

	// not highlight-able
	if (tagHighlight == 0)
		return;

	// already highlighted
	if (pos.has_tag(tagHighlight))
		return;

	//> Search for the previous TIME ANCHORED element of given type
	const string& id = anchors().getPreviousAnchorId(pos, type, true);

	INVALID( (id.empty()), "No previous anchor for type", type );
	setHighlight(id, track);
}

/*
 * highlight current item
 * @param type item type (turn/word/..)
 * @param start item approximate start pos in buffer
 */
Gtk::TextIter AnnotationBuffer::setHighlight(const string& id, int track)
{
	//> Search for the previous TIME ANCHORED element of given type
	Anchor* a = anchors().getAnchor(id);
	if ((a == NULL))
	{
		Log::err() << "No anchor with id" << id << endl;
		return getCursor();
	}
	const string& type = a->getType();
	string tagname;
	if (track == 0)
		tagname = type + "_highlight1";
	else if (track == 1)
		tagname = type + "_highlight2";

	const Glib::RefPtr<Gtk::TextTag>& tagHighlight = get_tag_table()->lookup(tagname);

	// not highlight-able
	if (tagHighlight == 0)
		return getCursor();

	Gtk::TextIter start = m_anchors[id].getMark()->get_iter();
	if (type != m_unitType)
	{
		while (!start.starts_line())
			start.backward_char();
	}

	bool ok = true;
	bool need_split = true;
	//	while ( ok && !isEditablePosition(start, need_split) )
	//		ok = start.forward_char() ;
	// already highlighted
	if (!start.has_tag(tagHighlight))
	{
		// goto end anchor for given type
		//	id = anchors().getEndAnchor(start, type, true);
		const string& eid = anchors().getEndAnchorId(id, true);
		const Gtk::TextIter& stop = (eid == "" ? end() : m_anchors[eid].getMark()->get_iter());

		Gtk::TextIter prev_start = begin();
		/** if begin has highlight, start from begin - otherwise, find the highlight start **/
		if (!prev_start.has_tag(tagHighlight))
			prev_start.forward_to_tag_toggle(tagHighlight);
		if (prev_start != end())
		{
			Gtk::TextIter prev_end = prev_start;
			prev_end.forward_to_tag_toggle(tagHighlight);
			remove_tag(tagHighlight, prev_start, prev_end);
		}
		apply_tag(tagHighlight, start, stop);
	}
	return start;
}

void AnnotationBuffer::setTag(const string& tagname, const string& type, Gtk::TextIter& pos, int track,
        Gtk::TextIter& applied_start, Gtk::TextIter& applied_stop)
{
	INVALID( (type.empty()), "empty type ! ", type );

	const Glib::RefPtr<Gtk::TextTag>& tag = get_tag_table()->lookup(tagname);
	if (tag == 0)
		return; // not highlight-able

	if (pos.has_tag(tag))
		return; // already tagged

	string id = anchors().getPreviousAnchorId(pos, type, true);
	INVALID( (id.empty()), "No previous anchor for type", type );

	Gtk::TextIter start = m_anchors[id].getMark()->get_iter();
	bool ok = true;
	bool need_split = true;
	while (ok && !isEditablePosition(start, need_split))
		ok = start.forward_char();
	// goto end anchor for given type
	//	id = anchors().getEndAnchor(start, type, true);
	id = anchors().getEndAnchorId(id);
	const Gtk::TextIter& stop = (id == "" ? end() : m_anchors[id].getMark()->get_iter());

	//	TRACE << "AnnotationBuffer::setHighlight " << type << " id= " << id
	//		<< " from " << start << " to " << stop << endl;

	remove_tag(tag, begin(), end());
	apply_tag(tag, start, stop);
	applied_start = start;
	applied_stop = stop;
}

//
// add newline to buffer
// @param only_if_required if true, add newline only if non empty buffer and no
//      preceeding newline
//
bool AnnotationBuffer::addNewline(const string& type, const string& align_id)
{
	bool connectionState = blockLeftGravityConnection(true);
	bool doit = (m_newlineBefore[type] && get_char_count() > 0);

	if (doit)
	{
		Gtk::TextIter pos = get_insert()->get_iter();
		if (!pos.backward_char())
			doit = false; // no need at buffer start !!
		else
			doit = !(get_text(pos, get_insert()->get_iter()) == "\n");

		if (doit)
		{
			// no newline if just after label of higher precedence element
			// except if must have a newline after
			if (!pos.editable())
			{
				string activeTag = getActiveTagClass(pos);
				if (!activeTag.empty())
				{
					doit = (activeTag == type);
					if (!doit)
					{
						int indexTag = anchors().getPrecedenceIndex(activeTag);
						int indexType = anchors().getPrecedenceIndex(type);
						doit = (indexTag > indexType);
					}
				}
			}
		}
	}
	if (doit)
	{
		move_mark(m_startMark, get_insert()->get_iter());

		//1 -- Check if a mark need to be moved after insertion
		Anchor* markNeedLeftG = markNeedsLeftGravity(m_startMark->get_iter(), align_id);

		//2 -- Insert
		const Glib::RefPtr<Gtk::TextMark>& mark = create_mark("tmpAddNewLine", getCursor());
		Gtk::TextIter it = insert_with_tag(getCursor(), "\n", m_labelTag);
		Gtk::TextIter start = get_iter_at_mark(mark);
		removePropagatingTags(start, it);
		apply_tag(m_labelTag, start, it);
		delete_mark(mark);

		//3 -- Move mark if needed
		if (markNeedLeftG != NULL)
		{
			anchors().moveAnchor(markNeedLeftG, m_startMark->get_iter(), false);
		}
	}

	blockLeftGravityConnection(connectionState);
	return doit;
}

//
// check if given position is or should be editable
//
bool AnnotationBuffer::isEditablePosition(const Gtk::TextIter& iter, bool& need_split, bool interactive)
{
	need_split = false;

	// -- At end of buffer position is editable but we could find an event -
	//    don't allow edition right now in this case
	if (iter.editable() && !(interactive && iter == end()))
		return true;

	if (interactive && iter != end() && iter.can_insert())
		return true;

	Gtk::TextIter previous = iter;
	previous.backward_char();

	//> -- CASE 1: we have a unit anchor at current postition
	vector<Anchor*> va = anchors().getAnchorsAtPos(iter, m_unitType);
	if (va.size() > 0)
	{
		if (!interactive)
			return isLabelOnly(iter);

		//-- Search for text unit
		vector<Anchor*>::iterator ita;
		Anchor* a = NULL;
		for (ita = va.begin(); ita != va.end(); ++ita)
			if ((*ita)->isTextType())
				a = *ita;

		// -- Label only ? ok
		if (a != NULL)
			if (isLabelOnly(iter))
				return true;

		const Glib::RefPtr<Gtk::TextTag>& tag = iterHasTag(previous, "unit", true);

		//-- We have a unit tag just before current position ?
		// If the nearer anchor before the tag is a text unit, it means we have a timestamped text unit just before current position
		// => no need to split, this anchor & this tag can receive text, editable OK
		// Otherwise, it means we need to create a text unit for receiving the text, needSplit !
		if (tag)
		{
			Anchor* prev = anchors().getPreviousAnchor(previous, "");
			// should never happen, should always have a previous anchor
			if (prev == NULL)
				return false;
			need_split = !prev->isTextType();
		}
		//-- We don't have tag before
		// 1) We're at segment start and there's a text unit at current position
		// 2) We're at segment start and there's an event/qualifier at current position
		else
		{
			/** TEMPORARY BLOCK **/
			Anchor* current = anchors().getAnchorAtPos(iter, "");
			// current is not text
			if (current && !current->isTextType())
			{
				Anchor* prev = anchors().getPreviousAnchor(*current);
				// current is at segment start : ugly block
				if (prev && current->hasLowerPrecedence(*prev))
					return false;
			}
			/** END OF DIRTY TEMP **/
			need_split = !(iter == end());
		}

		//		cout << "@@@ at unit anchor at " << iter << " need_split=" << need_split << endl;
		return true;
	}

	//> - Between  qualifier_start / qualifier_end pair -> is editable
	const Glib::RefPtr<Gtk::TextTag>& tag_end_event = iterHasTag(iter, "qualifier_", true, true);
	if (tag_end_event != NO_TAG)
	{
		if (iter.begins_tag(tag_end_event))
		{
			// then check that previous does not have an "end" tag
			if (iterHasTag(previous, "qualifier_", true, true) != NO_TAG)
				return false;
			if (iterHasTag(previous, "qualifier_", true) != NO_TAG)
			{
				//					TRACE_D << "@@@ between qualif at " << iter << " need_split=" << need_split << endl;
				return true;
			}
		}
		return false;
	}

	if (isLabelOnly(iter) || iter == end())
	{
		Anchor* prev = anchors().getPreviousAnchor(iter, "");
		if (prev != NULL && prev->getType() == m_unitType && prev->getIter().get_line() == iter.get_line())
		{
			if (interactive)
			{
				bool prev_is_qual = (iterHasTag(previous, "qualifier_", true) != NO_TAG);
				need_split = !prev->isTextType() || prev_is_qual;
			}
			//				cout << "@@@ labelonly at " << iter << " need_split=" << need_split << endl;
			return true;
		}
		return false;
	}
	return (iter == end());
}

bool AnnotationBuffer::isEditableRange(const Gtk::TextIter& start, const Gtk::TextIter& end)
{
	bool all = true;
	Gtk::TextIter iter = start;
	//	bool split_NOT_USED ;
	while (all)
	{
		if (iter == end)
			break;
		all = iter.editable(); // isEditablePosition(const_cast<Gtk::TextIter&>(iter), split_NOT_USED, false) ;
		iter++;
	}
	return all;
}

/*========================================================================
 *
 *  Events display
 *
 ========================================================================*/

void AnnotationBuffer::insertTaggedElement(const string& id, const std::string& parent_id, const string& type,
        const Glib::ustring& label, bool start_tag, bool rtl)
{
	begin_user_action();
	bool connectionState = blockLeftGravityConnection(true);
	inhibateEditSignal(true);
	inhibateSpaceHandling(true);

	//	TRACE << "IN  insertTaggedElement  id= " << id << " parent_id=" << parent_id << " type=" << type << "  is_start=" << start_tag << " label=" << label  << endl;

	Gtk::TextIter pos;
	bool to_restore = true;
	bool need_check_order = false;
	bool need_timestamp_mark = false;
	bool at_end = false;

	//> this may happen when inserting "on the fly"
	Anchor* anchor = anchors().getAnchor(parent_id);
	if (anchor == NULL)
	{
		at_end = true;
		pos = end();
		to_restore = false;
		need_check_order = !start_tag;
	}
	else
	{
		pos = anchor->getMark()->get_iter();
		// insertion for start tag
		if (start_tag)
		{
			pos = nextEditablePosition(pos, anchors().getEndAnchor(*anchor));
			if (anchor->isTimeAnchored() && pos.compare(anchor->getIter()) == 0)
				need_timestamp_mark = true;
		}
		// insertion for end tag
		else
		{
			Gtk::TextIter pos2 = pos;
			pos = previousEditablePosition(pos);

			// Check special case: end anchor is after an upper level mainstream element
			// move cursor to previous editable position
			// (note: a lower precedence index indicates a higher mainstream level
			Anchor* current = anchors().getAnchorAtPos(pos, "", false);
			if (current && current->getType() == m_unitType
			        && anchors().getPreviousAnchor(*current, "")->getPrecedenceIndex() < current->getPrecedenceIndex())
			{
				if (pos.backward_char())
					pos = previousEditablePosition(pos);
			}
			else
			{
				//TRACE_D << "IN  inser tTaggedElement (end) ID = " << id << " for segment " << parent_id << " at " << pos << "  start=" << start_tag << " anchor = " << anchor->getMark()->get_iter()  << endl;
				if (pos2.compare(pos) != 0 && pos.editable())
					pos.forward_char();
				need_check_order = true;
			}
		}
	}

	if (need_check_order)
	{
		Gtk::TextIter pos2 = pos;
		if (at_end)
		{
			pos2.backward_char();
			const Glib::RefPtr<Gtk::TextTag>& tag = iterHasTag(pos2, "qualifier", true, true);
			if (tag != 0)
			{
				//				TRACE_D << "@@@@  before backward tag=" << tag ->property_name() << "  pos2=" << pos2 ;
				pos2.backward_to_tag_toggle(tag);
				//				TRACE_D << "   after pos2=" << pos2 << endl;
			}
		}

		need_check_order = false;

		// if other qualifiers ending at same pos -> adjust insert position
		const Glib::RefPtr<Gtk::TextTag>& tag = iterHasTag(pos2, "qualifier", true, true);
		if (tag != 0)
		{
			const Glib::RefPtr<Gtk::TextTag>& idtag = iterHasTag(pos2, m_idTagPrefix, true);
			const Glib::RefPtr<Gtk::TextTag>& instag = get_tag_table()->lookup(id);
			if (idtag != 0 && instag != 0)
			{
				Gtk::TextIter pos3 = pos2;
				Gtk::TextIter pos4 = pos2;
				if (pos3.backward_to_tag_toggle(idtag) && pos4.backward_to_tag_toggle(instag))
				{
					//					TRACE_D << "@@@@  XMP  idtag=" << idtag->property_name() << " pos3=" << pos3 << endl;
					//					TRACE_D << "@@@@  XMP instag=" << instag->property_name() << " pos4=" << pos4 << endl;
					if (pos4.compare(pos3) < 0)
					{
						if (!at_end)
						{
							// then should be inserted after inbuf tag
							pos.forward_to_tag_toggle(idtag);
						}
					}
					else
					{
						if (at_end)
						{
							pos = pos2;
							need_check_order = true;
						}
					}
				}
			}
		}
	}

	if (anchor != NULL)
		to_restore = (pos.compare(anchor->getMark()->get_iter()) == 0);

	//> -- Backup current anchor pos
	guint off = pos.get_offset();

	//> -- Prepare corresponding tag(s) and create if needed
	list<Glib::RefPtr<Gtk::TextTag> > tags;
	prepareElementTags(id, type, start_tag, tags);

	bool prev_inhib = inhibateEditSignal(true);

	bool done_r2l = false;
	//> -- CASE right to left input -> have to protect event tag
	if (rtl)
	{
		if (label[0] == ' ' && label[label.size() - 1] == ' ')
		{
			// Add special letter and replace spaces
			Glib::ustring labnew = " " + Glib::ustring(1, InputLanguageArabic::RLE) + label.substr(1, label.size() - 2)
			        + Glib::ustring(1, InputLanguageArabic::PDF) + " ";
			pos = insert_with_tags(pos, labnew, tags);
			done_r2l = true;
		}
	}

	if (!done_r2l)
		pos = insert_with_tags(pos, label, tags);

	//> remove "high confidence" tag
	if (m_view->getWithConfidence())
		remove_tag(m_confidenceTag, get_iter_at_offset(off), pos);

	if (need_timestamp_mark)
		switchTimestamp(id, true);

	//	TRACE_D << "IN  insertTaggedElement  ID = " << id << " for segment " << parent_id << " at " << pos << "  start=" << start_tag  << endl;

	//> -- Restore right-anchored label to its initial position
	if (to_restore)
	{
		if (anchor && start_tag)
			anchors().moveAnchor(anchor, get_iter_at_offset(off));
	}

	// finally delete unwanted eventual preceeding / following space
	if (start_tag)
	{
		if (anchor)
		{
			Gtk::TextIter it = anchor->getMark()->get_iter();
			--it;
			if (it != end() && it.editable() && it.get_char() == ' ')
			{
				erase(it, anchor->getMark()->get_iter());
			}
		}
	}
	else
	{
		anchor = anchors().getAnchorAtPos(pos, m_unitType);
		if (anchor)
		{
			Gtk::TextIter it = anchor->getMark()->get_iter();
			if (it != end() && it.editable() && it.get_char() == ' ')
			{
				++it;
				erase(anchor->getMark()->get_iter(), it);
			}
		}
	}

	if (at_end && need_check_order)
	{
		// reset insert pos at end of buffer
		setCursor(end());
	}
	//	TRACE << "OUT  insertTaggedElement for " << parent_id << " start=" << start_tag ;

	inhibateEditSignal(prev_inhib);
	inhibateSpaceHandling(false);
	blockLeftGravityConnection(connectionState);
	end_user_action();
}

void AnnotationBuffer::prepareElementTags(const string& id, const string& type, bool start_tag, list<Glib::RefPtr<
        Gtk::TextTag> >& tags)
{
	//-- Id tag
	if (type != m_unitType)
	{
		Glib::RefPtr<Gtk::TextTag> idtag = get_tag_table()->lookup(id);
		if (!idtag)
			idtag = create_tag(id);
		//		m_view->addUndoableTag(idtag);
		tags.push_back(idtag);
	}

	//-- type tag
	const string& tagname = m_view->getRendererTagname(id, type);

	if (!tagname.empty())
	{
		if (start_tag)
		{
			const Glib::RefPtr<Gtk::TextTag>& tag = get_tag_table()->lookup(tagname);
			if (tag != 0)
				tags.push_back(tag);
				else
				TRACE_D << "WARN : NO TAG FOR " << tagname << endl;
		}
		else
		{
			string tmp = tagname + "_end";
			const Glib::RefPtr<Gtk::TextTag>& tag = get_tag_table()->lookup(tmp);
			if (tag != 0)
				tags.push_back(tag);
				else
				TRACE_D << "WARN : NO TAG FOR " << tagname << endl;
		}
	}

	//-- label tag
	//	tags.push_back(m_labelTag);

	/* SPELL */
	//	if ( m_nospellTag != 0 )
	//		tags.push_back(m_nospellTag);
}

void AnnotationBuffer::insertPixElement(const string& id, const std::string& parent_id, const string& type,
        bool start_tag, const string& pixbuf, const string& tag)
{
	if (type != "background")
		return;

	begin_user_action();
	inhibateEditSignal(true);
	Anchor* anchor = anchors().getAnchor(parent_id);

	//> Compute position
	Gtk::TextIter pos;
	bool to_restore = true;
	if (anchor == NULL)
	{
		// this may happen when inserting "on the fly"
		// Log::err() << " Warning: anchor not found for id " << parent_id << endl;
		pos = end();
		to_restore = false;
	}
	else
	{
		pos = anchor->getMark()->get_iter();
		if (start_tag)
			pos = nextEditablePosition(pos, anchors().getEndAnchor(*anchor));
		else
		{
			//>UNCOMMENT TO PLACE PIX ELEMENT AT END OF LINE
			pos = nextEditablePosition(pos, anchors().getEndAnchor(*anchor));
			Gtk::TextIter pos2 = pos;
			pos = previousEditablePosition(pos);
			if (pos2.compare(pos) != 0 && pos.editable())
				pos.forward_char();
		}
		to_restore = (pos.compare(anchor->getMark()->get_iter()) == 0);
	}

	guint off = pos.get_offset(); // backup current anchor pos

	bool prev_inhib = inhibateEditSignal(true);

	int offset = pos.get_offset();

	//> apply all tags
	list<Glib::RefPtr<Gtk::TextTag> > tags;
	prepareElementTags(id, type, true, tags);

	//> apply with tags
	pos = insert_with_tags(pos, pixbuf, tags);

	//> Remove tag
	remove_tag(m_labelTag, get_iter_at_offset(offset), pos);

	//> eventually restore
	if (to_restore)
	{
		if (anchor && start_tag)
			anchors().moveAnchor(anchor, get_iter_at_offset(off));
	}

	inhibateEditSignal(prev_inhib);
	end_user_action();
}

Glib::ustring AnnotationBuffer::getTaggedElementText(const Gtk::TextIter& pos)
{
	const Glib::RefPtr<Gtk::TextTag>& tag = iterHasTag(pos, "qualifier", true);

	if (tag == 0)
		return "";
	Gtk::TextIter start = pos;
	if (!start.begins_tag(tag))
		start.backward_to_tag_toggle(tag);
	Gtk::TextIter stop = pos;
	if (!stop.ends_tag(tag))
		stop.forward_to_tag_toggle(tag);
	return get_text(start, stop);
}

//
// return tagged element id at given cursor pos / "" if no tagged element found
//  prefix is the tag id prefix (usually AGSet id)
//
string AnnotationBuffer::getTaggedElementId(const Gtk::TextIter& iter)
{
	string value = "";

	if (iterHasTag(iter, m_unitType))
	{
		// get preceeding anchor id
		value = getPreviousAnchorId(iter, m_unitType);
	}
	else
	{
		const Glib::RefPtr<Gtk::TextTag>& idtag = iterHasTag(iter, m_idTagPrefix, true);

		if (idtag != 0)
			value = idtag->property_name().get_value();
	}
	return value;
}

//
//
Gtk::TextIter AnnotationBuffer::getTaggedElementIter(const string& id)
{
	const Glib::RefPtr<Gtk::TextTag>& tag = get_tag_table()->lookup(id);
	if (tag != 0)
	{
		Gtk::TextIter start = begin();
		if (!start.begins_tag(tag))
			start.forward_to_tag_toggle(tag);
		return start;
	}
	else
	{
		Anchor* a = anchors().getAnchor(id);
		if (a != NULL)
			return a->getMark()->get_iter();
	}
	return end();
}

//
// return tagged element type at given cursor pos / "" if no tagged element found
//  prefix is the tag id prefix (usually AGSet id)
//
string AnnotationBuffer::getTaggedElementType(const Gtk::TextIter& iter)
{
	const list<Glib::RefPtr<const Gtk::TextTag> >& tags = iter.get_tags();
	list<Glib::RefPtr<const Gtk::TextTag> >::const_iterator it;
	for (it = tags.begin(); it != tags.end(); ++it)
	{
		const string& tagname = (*it)->property_name().get_value();
		if (m_tagPrefix.find(tagname) != m_tagPrefix.end())
			return tagname;
	}
	return "";
}

bool AnnotationBuffer::canInsertQualifier(const Gtk::TextIter& pos)
{
	if (get_has_selection())
		return true;

	bool unused;
	if (!isEditablePosition(pos, unused, true) || !isalnum(pos.get_char()))
		return false;

	Gtk::TextIter iter = pos;
	iter.backward_char();
	if (!isEditablePosition(iter, unused, true) || !isalnum(iter.get_char()))
		return false;

	return true;
}

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
int AnnotationBuffer::canDeleteTextSelection(const Gtk::TextIter& start, const Gtk::TextIter& stop,
        vector<string>& toBeMergedOrDeleted)
{
	toBeMergedOrDeleted.clear();

	Gtk::TextIter backward_start = start;
	backward_start.backward_char();

	std::vector<string> tagnames;
	tagnames.push_back(m_unitType);
	tagnames.push_back("qualifier_");
	tagnames.push_back("background");

	bool tag_at_end = iterHasTags(stop, tagnames, true);
	bool tag_at_start = iterHasTags(backward_start, tagnames, true);
	const Anchor* current_anchor = anchors().getAnchorAtPos(start, m_unitType, false);

	if (tag_at_end && (tag_at_start || current_anchor != NULL))
	{
		string id_before, id_after;

		if (current_anchor != NULL)
			id_before = current_anchor->getId();
		else
		{
			id_before = getAnchoredElement(backward_start, false);
			if (id_before.empty())
				return -1;
		}

		id_after = getAnchoredElement(stop, false);
		if (id_after.empty())
			return -1;

		toBeMergedOrDeleted.push_back(id_before);
		toBeMergedOrDeleted.push_back(id_after);
		return 2;
	}
	// special case for last text unit of a segment : if empty & not anchored, delete it !
	else if (current_anchor != NULL && current_anchor->isTextType() && !current_anchor->isTimeAnchored())
	{
		Anchor* next = anchors().getEndAnchor(*current_anchor);
		if (next == NULL || next->getPrecedenceIndex() < current_anchor->getPrecedenceIndex())
		{
			toBeMergedOrDeleted.push_back(current_anchor->getId());
			return 1;
		}
	}

	return 0;
}

/*
 * -1: deletion can't be done
 *  0: deletion is allowed, nothing to do
 * 	1: deletion is allowed but an annotation must be deleted
 * 	2: deletion is allowed but annotation must be merged
 */
int AnnotationBuffer::canDeletePos(const Gtk::TextIter& pos, bool backward, vector<string>& toBeMergedOrDeleted)
{
	//	std::cout << "------------------------- pos=" << pos << std::endl ;
	//
	//	Gtk::TextIter start = pos;
	//	Gtk::TextIter stop = pos;
	//	if ( backward )
	//	{
	//		if ( ! start.backward_char() )
	//			return 0;
	//	}
	//	else if ( !stop.forward_char() )
	//		return 0;
	//
	//	std::cout << "------------------------- start=" << start << " - stop=" << stop << std::endl ;
	//
	//	return canDeleteTextSelection(start, stop, toBeMergedOrDeleted);

	//#ifdef TODEL
	toBeMergedOrDeleted.clear();

	//- will be set to postion before the one we're deleting
	Gtk::TextIter iter_before = pos;
	//- will be set to position after the one we're deleting
	Gtk::TextIter iter_after = pos;
	//- will be set to current position after deletion
	Gtk::TextIter iter_anchor = pos;
	vector < string > empty;

	std::vector<string> tagnames;
	tagnames.push_back("unit");
	tagnames.push_back("qualifier_");
	tagnames.push_back("background");

	// -- Checking forward position
	if (!backward)
	{
		iter_before.backward_char();
		iter_after.forward_char();
	}
	// -- Checking backward position
	else
	{
		iter_before.backward_char();
		iter_before.backward_char();
		iter_anchor.backward_char();
	}

	bool after = iterHasTags(iter_after, tagnames, true);
	bool before = iterHasTags(iter_before, tagnames, true);
	const string& current_anchor = getAnchorIdByType(iter_anchor, m_unitType, false);

	if (after && (!current_anchor.empty() || before))
	{
		string id_before, id_after;

		if (!current_anchor.empty())
			id_before = current_anchor;
		else if (before)
		{
			id_before = getAnchoredElement(iter_before, false);
			if (id_before.empty())
				return -1;
		}

		id_after = getAnchoredElement(iter_after, false);
		if (id_after.empty())
			return -1;

		toBeMergedOrDeleted.push_back(id_before);
		toBeMergedOrDeleted.push_back(id_after);
		return 2;
	}
	// special case for last text unit of a segment : if empty & not anchored, delete it !
	else if (!current_anchor.empty() && (iter_after.get_char() == '\n' || iter_after == end())
	        && m_view->getDataModel().mainstreamBaseElementHasText(current_anchor, "transcription_graph")
	        && m_view->getDataModel().isLastChild(current_anchor, "") && !m_view->getDataModel().isAnchoredElement(
	        current_anchor, 0))
	{
		toBeMergedOrDeleted.push_back(current_anchor);
		return 1;
	}

	return 0;
	//#endif
}

std::vector<Anchor*> AnnotationBuffer::needBlankBeforeDeleteSelection(Gtk::TextIter start, Gtk::TextIter end)
{
	std::vector<Anchor*> v;

	Anchor* startA = m_anchors.getAnchorAtPos(start, m_unitType, false, false, "");
	Anchor* endA = NULL;
	if (startA)
		endA = m_anchors.getAnchorAtPos(end, m_unitType, false, false, "");

	if (startA && endA)
	{
		v.push_back(startA);
		v.push_back(endA);
	}
	return v;
}

//
//  delete tagged element with given id from buffer
//   -> element may consist of 2 labels (begin label & end label) : both labels will
//    be deleted.
void AnnotationBuffer::deleteTaggedElement(const string& id)
{
	begin_user_action();
	bool connectionState = blockLeftGravityConnection(true);
	const Glib::RefPtr<Gtk::TextTag>& tag = get_tag_table()->lookup(id);

	if (tag != 0)
	{
		//> change anchor for annotation in order to permit good
		//  undo/redo actions
		// (i.e marks have to get particular gravity before tag is deleted
		// for the tag te be re-created at good place)

		Gtk::TextIter start = begin();
		bool doit = true;

		while (doit)
		{
			if (!start.begins_tag(tag))
			{
				if (!start.forward_to_tag_toggle(tag))
					doit = false;
			}
			if (doit)
			{
				Gtk::TextIter stop = start;
				stop.forward_to_tag_toggle(tag);
				start = erase(start, stop);
				Anchor* markNeedLeftG = markNeedsLeftGravity(start);
				if (markNeedLeftG != NULL)
				{
					// was anchored at start -> add blank if required
					Gtk::TextIter it = start;
					--it;
					if (it.editable() && !g_unichar_isspace(it.get_char()))
					{
						guint off = start.get_offset();
						start = insert(start, " ");
						markNeedLeftG->move(start);
					}
				}
			}
		}
		//> change anchor for annotation in order to permit good
		//  undo/redo actions
		// (i.e marks have to get particular gravity before tag is deleted
		// for the tag te be re-created at good place)
		get_tag_table()->remove(tag);

	}
	else
	{
		Log::err() << "Warning : in AnnotationBuffer::deleteTaggedElement : tag " << id << " not found" << endl;
	}

	blockLeftGravityConnection(connectionState);
	end_user_action();
}

//
//  delete tagged element with given id from buffer
//   -> element may consist of 2 labels (begin label & end label) : both labels will
//    be deleted.
void AnnotationBuffer::deletePixElement(const string& id)
{
	begin_user_action();
	const Glib::RefPtr<Gtk::TextTag>& tag = get_tag_table()->lookup(id);

	Glib::RefPtr<Gtk::TextMark> mymark;

	if (tag != 0)
	{

		Gtk::TextIter start = begin();
		bool doit = true;
		bool check_anchor = true;
		Anchor* anchor = NULL;

		while (doit)
		{
			if (!start.begins_tag(tag))
			{
				if (!start.forward_to_tag_toggle(tag))
					doit = false;
			}
			if (doit)
			{
				// Place stop erase iterator at end of tag
				Gtk::TextIter stop = start;
				stop.forward_to_tag_toggle(tag);

				// If anchor at start and anchor at end, superposition risk !!
				// UGLY: insert tmp white space
				Anchor* anchorAtStart = anchors().getAnchorAtPos(start, "");
				if (anchorAtStart)
				{
					// anchor at stop ? white space needed !
					Anchor* anchorAtStop = anchors().getAnchorAtPos(stop, "");
					if (anchorAtStop)
					{
						mymark = create_mark(start, "mytmpmark");
						list<Glib::RefPtr<Gtk::TextTag> > tags;
						tags.push_back(m_labelTag);
						stop = insert_with_tags(stop, " ", tags);
						stop.backward_char();
						start = get_iter_at_mark(mymark);
					}
				}

				start = erase(start, stop);
			}
		}
	}
	else
	{
		MSGOUT << "<?> Warning : in AnnotationBuffer::deletePixElement : unfound tag for " << id
		        << "(can be ok for shadow elements)" << endl;
	}
	end_user_action();
}
/*
 * replace segment label by given label
 */

void AnnotationBuffer::setAnchoredLabel(const std::string& id, const std::string& label, const string& tagname,
        bool r2l)
{
	Anchor* anchor = anchors().getAnchor(id);

	if (anchor != NULL)
	{

		Gtk::TextIter start = anchor->getMark()->get_iter();

		Glib::RefPtr<Gtk::TextTag> tag = iterHasTag(start, m_tagPrefix[anchor->getType()], true);
		Glib::RefPtr<Gtk::TextTag> tracktag = iterHasTag(start, "track", true);

		INVALID ( (tag == 0 ), " no tag for type ", anchor->getType());

		// skip eventual "label-only" prefix.
		bool ok = true;
		while (ok && isLabelOnly(start))
		{
			ok = start.forward_char();
		}

		if (!ok || start.editable())
			return; // No Anchored label for item
		Gtk::TextIter stop = start;
		if (!start.begins_tag(tag))
			start.backward_to_tag_toggle(tag);
		if (!stop.ends_tag(tag))
			stop.forward_to_tag_toggle(tag);

		begin_user_action();

		/* SPELL */
		//		if ( m_speller != NULL ) {
		//			gtkspell_inhibate_check(m_speller, true);
		//			move_mark(m_startMark, get_insert()->get_iter());
		//		}

		guint start_offset = start.get_offset();
		guint stop_offset = stop.get_offset();

		list<Glib::RefPtr<Gtk::TextTag> > tags;
		tags.push_back(get_tag_table()->lookup(tagname));
		//		tags.push_back(m_labelTag);
		if (tracktag != 0)
			tags.push_back(tracktag);
		/* SPELL */
		//		if ( m_nospellTag != 0 )
		//			tags.push_back(m_nospellTag);

		InputLanguage *il = NULL;
		std::string label2Add = "";
		if (r2l)
			label2Add += Glib::ustring(1, InputLanguageArabic::RLM);
		label2Add += label;
		if (r2l)
			label2Add += Glib::ustring(1, InputLanguageArabic::RLM);

		//> -- Insert new label
		stop = insert_with_tags(stop, label2Add, tags);

		/* SPELL */
		//		if ( m_speller != NULL )
		//		{
		//			//  spell error tag may have propagate -> remove it
		//			gtkspell_inhibate_check(m_speller, false);
		//		}

		//> -- Prepare anchor
		anchors().moveAnchor(anchor, get_iter_at_offset(start_offset));

		//> -- Erase old label
		erase(get_iter_at_offset(start_offset), get_iter_at_offset(stop_offset));

		end_user_action();
	}
else	g_return_if_reached ();
}

/**
 * delete anchored label & associated anchor in text buffer
 * @param id 					Id of anchored label to delete
 * @param keep_anchor 			True to keep corresponding text mark in buffer
 * @param keepPrevSpace			True for keeping eventual previous space or endline character
 */
void AnnotationBuffer::deleteAnchoredLabel(const string& id, bool keep_anchor, bool keepPrevSpace)
{
	Anchor* astart = anchors().getAnchor(id);

	if ( astart != NULL )
	{
		begin_user_action();

		/* (don't use const string because we'll need type after anchor deletion) */
		string type = astart->getType();

		/* We never delete the first base element of a segment except if we delete the segment
		 * itself (and in this case the segment would have been deleted first)
		 * So if we pass the following test, it means that we are deleting a foreground event
		 * at start of segmentation
		 * ==> don't delete alignment (space characters)
		 */
		//		Anchor* aprev = anchors().getPreviousAnchor(*astart) ;
		//		if ( aprev && aprev->getType() == m_segmentType )
		//			keepPrevSpace = true ;
		keepPrevSpace = false;

		Anchor* aend = anchors().getNextAnchor(*astart);
		string typeAend;
		if (aend)
		{
			move_mark(m_endMark, aend->getIter());
			typeAend = aend->getType();

			//> If we're deleting a mainstreamBaseType
			if ( astart->getType()==m_unitType )
			{
				//> UGLY HACK for UNDO action fix
				/*  For mainstream base type NON-TEXT element, tag is followed by mark of next element.
				 * There's a special case if the next element is a TEXT element : |[element1] |element2
				 *
				 *	When deleting element1, element2 is merged to element1, i.e element2's mark is deleted.
				 * 	Then element1's [tag] is deleted.
				 * 	==> When undoing deletion, element2 is re-created first with its mark. Then element1's tag and
				 * 	mark are re-inserted. But with the left gravity anchor mechanism, when inserting [tag],
				 * 	element2's mark is moved to the insertion point, which means at the start of the tag.
				 *	==> At the start of element1's tag, the element1's mark and element2's mark are superposed !
				 *	==> For forcing element2's mark position move mark at its current position
				 */
				if ( !astart->isTextType()
						&& aend->getType() == m_unitType )
				anchors().moveAnchor(aend, aend->getIter(), false);
			}
			else
			{
				//> UGLY HACK for UNDO action fix
				/*
				 * In several cases (delete overlapped turn, delete segment with first unit is event,...)
				 * the first child unit is created first, and we explicity block the left gravity mechanism
				 * for correct rendering.
				 * But when we delete and re-insert, the undo re-insertion can't block this mechanism.
				 *
				 * Workaround:
				 * ==> For forcing correct replacement, let's move the mark a its current position.
				 * 	   So we create an undo action that will proceeded after re-inserting tag.
				 */
				if ( aend->getType() != m_unitType )
				{
					Anchor* nextbase = anchors().getNextAnchor(*aend, m_unitType); // next unit type is the first child
					//					if ( nextbase && m_view->getDataModel().isFirstChild(nextbase->getId(), astart->getId()) )
					if ( nextbase )
					anchors().moveAnchor(nextbase, nextbase->getIter(), false);
				}
				else
				anchors().moveAnchor(aend, aend->getIter(), false);
			}
		}

		Gtk::TextIter start_pos = astart->getIter();
		const Glib::RefPtr<Gtk::TextTag>& tag = iterHasTag(start_pos, m_tagPrefix[type], true);
		Gtk::TextIter end_pos = start_pos;

		if ( tag )
		{
			// When deleting an element, check for all tag
			// but take care because an element of same type
			// can be placed at pos just after, so check anchor too
			// remark: if for some reason (update partially done) anchor found is at start pos, ignore
			Gtk::TextIter iter_toggled = end_pos;
			iter_toggled.forward_to_tag_toggle(tag);

			Gtk::TextIter iter_anchor = iter_toggled;
			if (aend)
			iter_anchor = aend->getIter();

			if ( iter_anchor < iter_toggled )
			end_pos = iter_anchor;
			else
			end_pos = iter_toggled;
		}

		//> !CAUTION
		// delete mark BEFORE deleting text because of action order for undoing/redoing
		if ( ! keep_anchor )
		anchors().deleteAnchor(id);

		if ( !start_pos.editable() )
		{
			if ( end_pos > start_pos > 0 )
			{
				//> Erase label anchored
				start_pos = erase(start_pos, end_pos);
				end_pos = start_pos;
			}

			map<string,bool>::iterator it = m_newlineAfter.find(type);
			bool newLineAfterType = (it!=m_newlineAfter.end() && it->second);
			while (isLabelOnly(end_pos) )
			{
				// If the element we're deleting doesn't displays a \n after its tag, don't delete eventual
				// following /n
				if ( !newLineAfterType && end_pos.get_char()=='\n' )
				break;

				// otherwise go forward from 1 char
				if ( !end_pos.forward_char() )
				break;
			}

			if ( end_pos > start_pos )
			{
				//				TRACE << " ££££££££££  deleteAnchoredLabel " << __LINE__ << " -> erase from " << start_pos << " to " << end_pos << " text= '" << get_text(start_pos, end_pos) <<"'"<< endl;
				start_pos = erase(start_pos, end_pos);
			}
			end_pos = start_pos;

			//> Backward to erase eventually "\n" or space characters
			//  (for pasting anchored label text to previous segment)
			if (! keepPrevSpace)
			{
				end_pos.backward_char();
				//				TRACE << " ££££££££££  before " << __LINE__ << " -> at " << end_pos << " editable=" << end_pos.editable() << " text= '" << get_text(end_pos, start_pos) <<"'"<< endl;

				if ( isLabelOnly(end_pos) && g_unichar_isspace(end_pos.get_char()) )
				{
					//					TRACE << " ££££££££££  deleteAnchoredLabel " << __LINE__ << " -> erase from " << end_pos << " to " << start_pos << " text= '" << get_text(end_pos, start_pos) <<"'"<< endl;
					start_pos = erase(end_pos, start_pos);
				}
			}

			//> Restore newline/space if required
			//  (since we've erased label-only text eventually following
			//   the erased anchored label, we need to place a new \n
			//   for displaying correctly the next anchored label)
			//  TODO: don't delete first one, and we won't need to
			//  	  insert this new one
			bool added = false;
			if ( aend && aend->isTimeAnchored() )
			{
				backupInsertPosition(1, true);
				Gtk::TextIter iter = get_iter_at_mark(m_endMark);
				if ( start_pos.compare(iter) == 0 )
				{
					guint offset = start_pos.get_offset();
					setCursor(iter, false);
					added = addNewline(typeAend);
					start_pos = get_iter_at_offset(offset);
				}
				// Eventually insert blank characters
				// (!) SPECIFIC CASE
				// If we're deleting a segmentationBaseType, the mainstreamBaseType renderer
				// will add its own space.
				if ( ! added && ! g_unichar_isspace(start_pos.get_char()))
				{
					end_pos = start_pos;
					end_pos.backward_char();
					if ( ! g_unichar_isspace(end_pos.get_char()) )
					insert(start_pos, " ");
				}
				restoreInsertPosition(1, false);
			}
			else
			{
				//  if prev pos is non-space and editable char -> add a space
				if ( start_pos.editable() && start_pos != end() )
				{
					end_pos = start_pos;
					end_pos.backward_char();
					if ( end_pos.editable()
							&& ! g_unichar_isspace(end_pos.get_char())
							&& ! g_unichar_isspace(start_pos.get_char()) )
					insert(start_pos, " ");
				}
			}

		}
		end_user_action();
	}
}

/**
 * delete anchored text & associated anchor in text buffer
 * @param id id of anchored text to delete
 * @param keep_anchor true to keep corresponding text mark in buffer
 *
 */
void AnnotationBuffer::deleteAnchoredText(const string& id, bool keep_anchor)
{
	Anchor* astart = anchors().getAnchor(id);
	if ( astart != NULL )
	{
		begin_user_action();
		Gtk::TextIter start_pos = astart->getMark()->get_iter();
		Gtk::TextIter end_pos;
		Anchor* aend = anchors().getNextAnchor(*astart);
		if ( aend != NULL )
		end_pos = aend->getMark()->get_iter();
		else
		end_pos = end();
		if ( keep_anchor )
		{
			start_pos = nextEditablePosition(start_pos, aend, true);
			end_pos = previousEditablePosition(end_pos, astart, true);
			if ( start_pos.compare(end_pos) > 0 )
			start_pos = end_pos;
		}

		//> !CAUTION
		// Necessary deleting mark BEFORE deleting text
		// because of action order for undoing/redoing
		if ( ! keep_anchor )
		anchors().deleteAnchor(id);

		//> Erase all text contained between start_pos and end_pos
		if ( start_pos.compare(end_pos) < 0 )
		{
			//			TRACE_D << " IN AnnotationBuffer::deleteAnchoredText " << id << " at " << start_pos << " text= '" << get_text(start_pos, end_pos) <<"'" << endl;
			//> Erase label anchored
			start_pos = erase(start_pos, end_pos);
			end_pos = start_pos;

			//> Also erase label-only text eventually following
			//  the erased anchored text
			//  TODO: don't delete end of line label tagged so we
			//  	  won't need to insert another later
			while (isLabelOnly(end_pos) )
			if ( !end_pos.forward_char() )
			break;
			if ( end_pos.compare(start_pos) > 0 )
			start_pos = erase(start_pos, end_pos);
		}
		end_user_action();
	}
}

/*
 * getAnchoredIter for given id
 *  -> return buffer iterator for corresponding id
 *   or current insert pos if id not found
 */
Gtk::TextIter AnnotationBuffer::getAnchoredIter(const string& id)
{
	Anchor* anchor = anchors().getAnchor(id);
	if ( anchor != NULL ) return anchor->getMark()->get_iter();
	return get_insert()->get_iter();
}

/*
 * getAnchoredLabel for given id
 *  -> return label if any, else displayed text
 */

Glib::ustring AnnotationBuffer::getAnchoredLabel(const string& id)
{
	Anchor* anchor = anchors().getAnchor(id);
	if ( anchor != NULL )
	{
		if ( anchor->getType() != m_segmentType )
		{
			Gtk::TextIter start = anchor->getMark()->get_iter();
			bool done = false;
			while ( !done )
			{
				const Glib::RefPtr<Gtk::TextTag>& tag = iterHasTag(start,m_tagPrefix[anchor->getType()], true);
				if ( tag !=0 )
				{
					Gtk::TextIter stop = start;
					stop.forward_char();
					stop.forward_to_tag_toggle(tag);
					if ( start.compare(stop) < 0 )
					return get_text(start, stop);
					done=true;
				}
				if ( ! start.forward_char() ) done=true;
			}
		}
		else return getSegmentText(id, "", true);
	}
	return "";
}

/**
 * return anchored element id at iter if exists, else ""
 * @param pos text iter position
 * @timeAnchored if true, return only signal-anchored elements (those with a defined signal offset)
 * @return element id / "" if no element at cursor pos
 */
string AnnotationBuffer::getAnchoredElement(const Gtk::TextIter& pos, bool timeAnchored)
{
	Anchor* a = anchors().getAnchorAtPos(pos, "", timeAnchored);
	if ( a != NULL ) return a->getId();
	return "";
}

string AnnotationBuffer::getAnchorIdByType(const Gtk::TextIter& pos, const string& type , bool timeAnchored)
{
	Anchor* a = anchors().getAnchorAtPos(pos, type, timeAnchored);
	if ( a != NULL ) return a->getId();
	return "";
}

/*==============================================================================
 */

// retrieve segment text starting at segment anchor up to next anchor
Glib::ustring AnnotationBuffer::getSelectedText(bool with_labels)
{
	Gtk::TextIter start_pos, end_pos;
	bool ok;

	if ( get_selection_bounds(start_pos, end_pos) )
	{
		if ( with_labels )
		return get_text(start_pos, end_pos);
		else
		{
			Glib::ustring buf("");
			Gtk::TextIter iter = start_pos;
			while ( iter.compare(end_pos) < 0 )
			{
				ok =true;
				while ( !iter.editable() && ok ) ok = iter.forward_char();
				start_pos = iter;
				while ( iter.editable() && ok && iter.compare(end_pos) < 0)
				ok = iter.forward_char();
				if ( buf.length() > 0 && ! g_unichar_isspace(buf[buf.length()-1]) ) buf += ' ';
				buf += get_text(start_pos, iter);
			}
			removePresentationCharacters(buf);
			return (string)buf;
		}
	}
	return "";
}

/**
 * retrieve segment text starting at segment anchor up to next anchor
 * @param id start anchor id
 * @param end_id end anchor id / "" to be guessed from buffer contents
 * @param with_labels true to include tagged elements labels in returned text
 * @param trim true to trim leading and trailing blanks from buffer
 *
 *  this function is mainly used for 2 purposes :
 *  - retrieving text to be stored in data model (with_labels=false);
 *  - retrieving text to be displayed in signal segments (with_labels=true)
 */

Glib::ustring AnnotationBuffer::getSegmentText(const string& id, const string& end_id, bool with_labels, bool trim)
{
	Anchor* start = anchors().getAnchor(id);
	if ( start != NULL )
	{
		Gtk::TextIter start_pos = start->getMark()->get_iter();

		bool go_on = true;

		if ( ! with_labels )
		{
			start_pos = nextEditablePosition(start_pos, end_id);
		}
		else
		{
			// -- Go forth to start of eventual tagged elements
			bool tagDone = false;
			while ( go_on )
			{
				if ( isLabelOnly(start_pos) )
				go_on = start_pos.forward_char();
				else
				{
					const Glib::RefPtr<Gtk::TextTag>& tag = iterHasTag(start_pos, m_tagPrefix[start->getType()], true, false, true);
					if ( tag != 0 && go_on )
					{
						// -- If we have already found a matching tag, it means we have already seen the id we're searching.
						//    So the second time we find a tag of same type, we must be at next element, so we should stop there
						//	  because it means we've found nothing for the id we want.
						if (!tagDone)
						{
							go_on = start_pos.forward_to_tag_toggle(tag);
							tagDone = true;
						}
						else
						go_on = false;
					}
					else
					go_on=false;
				}
			}
		}
		//		TRACE << "IN AnnotationBuffer::getSegmentText  id=" << id <<  " end_id=" << end_id << " start_pos=" << start_pos << " with_labels=" << with_labels << "  trim=" << trim << endl;
		Gtk::TextIter end_pos = start_pos;

		std::vector<Anchor*>::iterator it;
		if ( ! with_labels && !end_id.empty() )
		{
			int cnt=0;
			end_pos = getAnchoredIter(end_id);

			Anchor* next = anchors().getNextAnchor(*start);
			if ( next != NULL && end_pos > next->getIter() )
			end_pos = next->getIter();

			//-- Take care if we are just after a tag (i.e end element)
			//   => position is editable but don't want to return the tag content
			//   ==> test previous position too
			Gtk::TextIter itmp = end_pos;
			itmp.backward_char();
			if ( end_pos.editable() && itmp > start_pos && !itmp.editable() )
			end_pos.backward_char();

			while ( end_pos > start_pos && ! end_pos.editable() )
			{
				end_pos.backward_char();
				++cnt;
			}
			if ( cnt > 0 )
			end_pos.forward_char();
		}
		else
		{
			go_on= ( with_labels || end_pos.editable() );
			if ( end_pos.compare(start->getMark()->get_iter()) == 0 )
			if (go_on)
			go_on = end_pos.forward_char(); // dont stay stucked at start anchor !

			while ( go_on )
			{
				if ( go_on = (with_labels || end_pos.editable()) )
				{
					Anchor* ae = anchors().getAnchorAtPos(end_pos, "", true);
					if ( ae != NULL && ! ae->hasLowerPrecedence(*start) )
					{
						if ( with_labels && !ae->isTimeAnchored()
								&& ae->getPrecedenceIndex() == start->getPrecedenceIndex() )
						go_on= end_pos.forward_char();
						else
						go_on = false;
					}
					else
					go_on= end_pos.forward_char();
				}
			}
		}

		//		TRACE << " GET TEXT 1.2 start=" << start_pos << " end=" << end_pos << " text=(" << get_text(start_pos, end_pos) << ")"  << endl;
		//		end_pos.backward_char();
		//		TRACE << " GET TEXT 2 start=" << start_pos << " end=" << end_pos << " text=(" << get_text(start_pos, end_pos) << ")" << endl;

		//> Trim white space at start
		if ( trim )
		{
			while ( start_pos.compare(end_pos) < 0 && g_unichar_isspace(start_pos.get_char()) )
			start_pos.forward_char();
		}

		//> Trim white space at end
		if ( trim )
		{
			bool back = false;
			while ( start_pos.compare(end_pos) < 0 && g_unichar_isspace(end_pos.get_char()) )
			back = end_pos.backward_char();

			// if we've moved, check if we're not 1 position too back
			if ( back && ! g_unichar_isspace(end_pos.get_char()) && end_pos.editable())
			end_pos.forward_char();
		}

		//> REMOVE PRESENTATION CHARACTERS
		if ( start_pos.compare(end_pos) < 0 )
		{
			//> only for dataModel write
			if (!with_labels)
			{
				//> get trimmed text
				Glib::ustring res = get_text(start_pos, end_pos);
				removePresentationCharacters(res);
				Gtk::TextIter iter;

				res = spaceHandler(iter, res, false, m_spaceHandling, m_spaceBordering);
				return (string)res;
			}
			else
			return get_text(start_pos, end_pos);
		}
	}
	return "";
}

void AnnotationBuffer::removePresentationCharacters(Glib::ustring& s)
{
	Glib::ustring::iterator its;
	for (its=s.begin(); its!=s.end();)
	{
		if ( InputLanguageArabic::is_presentation_character(*its) )
		its = s.erase(its);
		else ++its;
	}
}

/**
 *  returns end text iterator for text part starting at anchor id
 *  @param id start anchor id
 *  @param end_id end anchor id / "" if to be guessed from buffer contents
 *  @param timeAnchored true to stop only at a "main" anchor location, false to stop at next anchor in buffer (used only if end_id not specified)
 *
 *  @note return iter is set just before any eventual label-only text preceeding *	end anchor.
 */
Gtk::TextIter AnnotationBuffer::getAnchoredEndIter(Anchor* start, const string& end_id, bool timeAnchored)
{
	Gtk::TextIter end_pos = end();
	if ( start != NULL )
	{
		Anchor* stop;
		const Gtk::TextIter& start_pos = start->getMark()->get_iter();
		//		if ( end_id.empty() )
		stop = anchors().getEndAnchor(*start, timeAnchored);
		//		else
		//			stop = anchors().getAnchor(end_id);
		//		if ( ! end_id.empty() ) {
		//			Anchor* stop2 = anchors().getAnchor(end_id);
		//			if ( stop->getMark()->get_iter() > stop2 ->getMark()->get_iter() )
		//				stop = stop2;
		//		}

		if ( stop != NULL )
		{
			end_pos = stop->getMark()->get_iter();
			end_pos.backward_char();
			while ( start_pos.compare(end_pos) < 0 && isLabelOnly(end_pos) )
			end_pos.backward_char();
			if (end_pos.get_char()!='\n') //** LG3
			end_pos.forward_char();
			// eventually skip one more blank
			if ( end_pos.get_char() == ' ' && isLabelOnly(end_pos) )
			end_pos.forward_char();

		}
	}
	return end_pos;
}

/**
 * delete text segment & return initial segment position in buffer
 * @param id id of segment to be deleted
 * @param timeAnchored if true,  delete up to next "main" segment
 * @return position of deleted segment start, or, if segment not found, current insert position,
 * */
Gtk::TextIter AnnotationBuffer::deleteSegment(const string& id, bool timeAnchored)
{

	//	TRACE << " IN AnnotationBuffer::deleteSegment " << id << " ismain=" << timeAnchored << endl;
	Anchor* astart = anchors().getAnchor(id);
	if ( astart != NULL )
	{
		begin_user_action();
		Gtk::TextIter start_pos = astart->getMark()->get_iter();
		guint start_offset = start_pos.get_offset();

		Gtk::TextIter end_pos;

		// delete everything up to end anchor
		Anchor* aend = anchors().getEndAnchor(*astart);
		if ( aend != NULL ) end_pos = aend->getMark()->get_iter();
		else end_pos = end();

		/*			TRACE << "ERASE BUF for seg id=" << id << " from " << start_pos << " to " << end_pos
		 << " endid= " << ( aend != NULL ? aend->getId() : "")
		 << " text=(" << get_text(start_pos, end_pos)<< ")"<< endl;
		 */
		// delete all anchors between start & end pos

		anchors().deleteAnchors(start_pos, end_pos);
		erase(start_pos, end_pos);

		//		anchors().deleteAnchor(id);
		end_user_action();

		return get_iter_at_offset(start_offset);
	}
	return get_insert()->get_iter();
}

//
// get all ids (anchors & tagged elements) between start & stop iters
vector<string> AnnotationBuffer::getIds(const Gtk::TextIter& start, const Gtk::TextIter& stop, const string& prefix, bool with_anchored)
{
	vector<string> ids;

	// get first qualifiers
	Gtk::TextIter iter = start;

	bool ok = true;
	string classtag;
	while ( ok && iter.compare(stop) < 0 )
	{
		Glib::RefPtr<Gtk::TextTag> tag = iterHasTag(iter, prefix, true);
		classtag = getActiveTagClass(iter);
		if ( tag != 0 /*&& (!m_view->getDataModel().isMainstreamType(classtag) || with_anchored )*/)
		{
			ids.push_back(tag->property_name().get_value());
			iter.forward_to_tag_toggle(tag);
		}
		else
		ok = iter.forward_char();
	}

	// then add anchors
	const vector<string>& anchor_ids = anchors().getAnchorIds(start, stop, with_anchored);
	copy(anchor_ids.begin(), anchor_ids.end(), back_inserter(ids));

	return ids;
}

/**
 * Updates lines alignment for whole element
 * @param id 		element id
 * @param r2l 			True for right to left display, false otherwise
 */
// TODO -> add/remove RLM only if anchored at line start.
//    -> iee add/remove RLM to all lines between current id and next (same type) id
void AnnotationBuffer::updateAlignment(const string& id, bool r2l)
{
	Anchor *an = NULL;
	Gtk::TextIter it;
	gunichar c;

	//> get start anchor and update the RL mode at each line start
	//  starting at id's anchor and up to id's end anchor.
	an = anchors().getAnchor(id);
	if(an)
	{
		it = an->getMark()->get_iter();
		Anchor *an2 = anchors().getEndAnchor(*an);
		if ( an2 != NULL )
		move_mark(m_endMark, an2->getMark()->get_iter());
		else
		move_mark(m_endMark, end());
		if ( it.get_line_offset() > 0 )
		it.set_line_offset(0);
		while (it < m_endMark->get_iter() )
		{
			c = it.get_char();
			//> have to left align but we found a RLM sign -> erase the RLM
			move_mark(m_startMark, it);
			if(c == InputLanguageArabic::RLM && !r2l)
			{
				it++;
				erase(m_startMark->get_iter(), it);
			}
			//> have to right align but we didn't found a RLM sign -> insert a RLM

			else if(c != InputLanguageArabic::RLM && r2l)
			Gtk::TextBuffer::insert(it, Glib::ustring(1, InputLanguageArabic::RLM));
			it = m_startMark->get_iter();
			if ( ! it.forward_line() ) break;
		}
	}
}

//**LG3

/*========================================================================
 *
 *  Buffer utilities
 *
 ========================================================================*/

bool AnnotationBuffer::canSplitAtIter(const Gtk::TextIter& iter)
{
	Gtk::TextIter back = iter;
	back.backward_char();

	if ( g_unichar_isalnum(iter.get_char())
			&& iterHasTag(back, "qualifier_", true)
			&& (iterHasTag(back, "", true, true) == NO_TAG) )
	{
		return false;
	}
	else if ( g_unichar_isalnum(back.get_char())
			&& (iterHasTag(iter, "qualifier_", true) != NO_TAG)
			&& (iterHasTag(iter, "", true, true) != NO_TAG) )
	{
		return false;
	}

	return true;
}

void AnnotationBuffer::setTrackTag(const string& id, int notrack)
{
	Anchor* astart = anchors().getAnchor(id);
	if ( astart != NULL )
	{
		const Gtk::TextIter& it1= getAnchoredIter(id);
		const Glib::RefPtr<Gtk::TextTag>& tag = iterHasTag(it1, "track", true);
		if ( tag != 0 )
		{
			Gtk::TextIter it2 = it1;
			it2.forward_to_tag_toggle(tag);
			Anchor* aend = anchors().getEndAnchor(*astart);
			if ( aend != NULL && it2.compare(aend->getMark()->get_iter()) > 0 ) it2 = aend->getMark()->get_iter();
			remove_tag(tag, it1, it2);
			addTrackTag(it1, it2, notrack);
		}
	}
}

void AnnotationBuffer::addTrackTag(const Gtk::TextIter& it1, const Gtk::TextIter& it2, int notrack)
{
	ostringstream tagname;

	tagname << "track";
	if(notrack <= 0)
	tagname << 0;
	else
	{
		if ( notrack == 3 || notrack == 1)
		{
			InputLanguage *il = m_view->getParent().get_input_language();
			if (il && il->ModeLeft2Right() == false)
			{
				if (notrack==3)
				tagname << 2;
				else
				tagname << 4;
			}
			else
			tagname << notrack;
		}
		else
		tagname << notrack;
	}
	apply_tag_by_name(tagname.str(), it1, it2);
}

void AnnotationBuffer::setTimestamp(const Gtk::TextIter& startIter, const Gtk::TextIter& endIter, bool on)
{
	if (!m_timestampTag)
	return;

	if (on)
	apply_tag(m_timestampTag, startIter, endIter);
	else
	remove_tag(m_timestampTag, startIter, endIter);
}

bool AnnotationBuffer::switchTimestamp(const string& id, bool on)
{
	// no tag ? exit
	if (! m_timestampTag )
	return false;

	// no element found ? exit
	Gtk::TextIter iter = getTaggedElementIter(id);
	if (iter==end())
	return false;

	// search for end tagged element position
	Gtk::TextIter start = iter;
	iter.forward_char();

	// apply or remove tag between the 2 positions
	if (on && !iterHasTag(start, m_timestampTag->property_name().get_value(), false))
	{
		setTimestamp(start, iter, true);
		Anchor* an = anchors().getAnchor(id);
		if (an)
		an->setTimeAnchored(true);
	}
	else if (!on && iterHasTag(start, m_timestampTag->property_name().get_value(), false))
	{
		setTimestamp(start, iter, false);
		Anchor* an = anchors().getAnchor(id);
		if (an)
		an->setTimeAnchored(false);
	}
	else
	return false;

	return true;
}

bool AnnotationBuffer::iterHasTags(const Gtk::TextIter& iter, const vector<string>& tagnames , bool is_prefix)
{

	const list< Glib::RefPtr<Gtk::TextTag > >& tags = const_cast<Gtk::TextIter&>(iter).get_tags();
	list< Glib::RefPtr<Gtk::TextTag > >::const_iterator it;

	if ( is_prefix )
	{
		vector<string>::const_iterator itv;
		for ( it = tags.begin(); it != tags.end(); ++it )
		{
			for ( itv=tagnames.begin(); itv!=tagnames.end(); itv++ )
			{
				if ( strncmp((*it)->property_name().get_value().c_str(), (*itv).c_str(), (*itv).length()) == 0 )
				return true;
			}
		}
	}
	else
	{
		vector<string>::const_iterator itv;
		for ( it = tags.begin(); it != tags.end(); ++it )
		{
			itv = find( tagnames.begin(), tagnames.end(), (*it)->property_name().get_value() );
			if (itv != tagnames.end())
			return true;
		}
	}
	return false;
}

/*
 *  check if current text pos has tag matching tagname
 *   if is_prefix=true, check if current pos has any tag   matching tagname
 */
Glib::RefPtr<Gtk::TextTag> AnnotationBuffer::iterHasTag(const Gtk::TextIter& iter, const string& tagname, bool is_prefix, bool check_end_tag, bool noHighlight)
{

	if ( is_prefix )
	{
		const list< Glib::RefPtr<Gtk::TextTag > >& tags = const_cast<Gtk::TextIter&>(iter).get_tags();
		list< Glib::RefPtr<Gtk::TextTag > >::const_iterator it;
		for ( it = tags.begin(); it != tags.end(); ++it )
		{
			const string& name = (*it)->property_name().get_value();
			bool ok_prefix = ( tagname.empty() || strncmp(name.c_str(), tagname.c_str(), tagname.length()) == 0 );
			bool ok_highlight = ( !noHighlight || (noHighlight && name.find("highlight") == string::npos) );
			if ( ok_prefix && ok_highlight && (! check_end_tag || name.find("_end") != string::npos) )
			return *it;
		}
	}
	else
	{
		Glib::RefPtr<Gtk::TextTag > tag = get_tag_table()->lookup(tagname);
		if ( tag && iter.has_tag(tag) )
		return tag;
	}

	return NO_TAG;
}

/*
 *  set insert mark to next editable position after given pos
 */
void AnnotationBuffer::setCursor(const Gtk::TextIter& pos, bool to_edit_pos)
{
	Glib::Timer e;
	//	TRACE << "IN AnnotationBuffer::setCursor " << pos << endl;
	if ( to_edit_pos )
	{
		const Gtk::TextIter& pos1 =nextEditablePosition(pos);
		place_cursor(pos1);
	}
	else
	place_cursor(pos);
	//std::cout << "  ###########  AnnotationBuffer::setCursor pos=" << pos << "  toedit=" << to_edit_pos << " in " << e.elapsed() << endl ;
}

/*
 *  set insert mark to editable position after given id
 *  @param id anchor id
 *  @param at_end if true place cursor at element end, else at element start
 *  @param force if true always moves cursor, else if cursor between element start and end leave it at its current place
 */
void AnnotationBuffer::setCursorAtAnchor(const string& id, bool at_end, bool force)
{
	Anchor* anchor = anchors().getAnchor(id);
	if ( anchor == NULL )
	{
		TRACE_D << "!! mark " << id << " not found " << endl;
		return;
	}

	if ( at_end )
	{
		Gtk::TextIter pos = getAnchoredEndIter(anchor, "", true);
		if ( ! force )
		{
			const Gtk::TextIter& cur = getCursor();
			if ( cur < pos )
			{
				Gtk::TextIter pos2 = nextEditablePosition(anchor->getIter());
				if ( cur >= pos2 ) return; // don't move
			}
		}
		setCursor(pos, false);
	}
	else
	{
		const Gtk::TextIter& pos = nextEditablePosition(anchor->getIter());
		if ( ! force )
		{
			const Gtk::TextIter& cur = getCursor();
			if ( cur >= pos )
			{
				Gtk::TextIter pos2 = getAnchoredEndIter(anchor, "", true);
				if ( cur < pos2 ) return;
			}
		}
		setCursor(pos);
	}
}

void AnnotationBuffer::saveCursor()
{
	Gtk::TextIter current = get_iter_at_mark(get_insert());
	cursorOffset = current.get_offset();
}

float AnnotationBuffer::restoreCursor()
{
	if (cursorOffset < 0)
	return -1;

	Gtk::TextIter iter = get_iter_at_offset(cursorOffset);
	if (cursorOffset==0)
	iter = nextEditablePosition(iter, "", false);
	place_cursor(iter);
	return cursorOffset;
}

/*
 *  move insert mark by n chars
 */
void AnnotationBuffer::moveCursor(int nbchar)
{
	Gtk::TextIter pos = getCursor();
	if ( nbchar > 0 ) pos.forward_chars(nbchar);
	else pos.backward_chars(abs(nbchar));
	place_cursor(pos);
}
/*
 * find next editable position -> after label & data, but before next mark
 */
Gtk::TextIter AnnotationBuffer::nextEditablePosition(const Gtk::TextIter& pos, const string& endmark, bool interactive)
{
	Anchor* endAnchor = anchors().getAnchor(endmark);
	return nextEditablePosition(pos, endAnchor, interactive);
}

/*
 * find next editable position -> after label & data, but before next mark
 */
Gtk::TextIter AnnotationBuffer::nextEditablePosition(const Gtk::TextIter& pos, Anchor* endAnchor, bool interactive)
{
	Gtk::TextIter pos1 = pos;
	bool need_split;
	bool stop = false;
	//	cout << "@@@@ nextEditablePosition at " << pos << " interactive=" << interactive << endl;
	while ( !stop && ! isEditablePosition(pos1, need_split, interactive) )
	{
		stop = ( pos1 == end() || ! pos1.forward_char() );
	}

	if ( endAnchor != NULL && pos1.compare(endAnchor->getIter()) > 0 )
	pos1 = endAnchor->getIter();

	//	cout << " @]@@  -> set to " << pos1 << endl;
	return pos1;
}

/*
 * find previous editable position ->  before label & data, but after previous mark
 */
Gtk::TextIter AnnotationBuffer::previousEditablePosition(const Gtk::TextIter& pos, const string& afterId, bool interactive)
{
	Anchor* afterAnchor = anchors().getAnchor(afterId);
	return previousEditablePosition(pos, afterAnchor, interactive);
}

/*
 * find previous editable position ->  before label & data, but after previous mark
 */
Gtk::TextIter AnnotationBuffer::previousEditablePosition(const Gtk::TextIter& pos, Anchor* afterAnchor, bool interactive)
{
	//	const Glib::RefPtr<Gtk::TextTag>& tagData = get_tag_table()->lookup("segdata");

	Gtk::TextIter pos1 = pos;
	bool need_split;
	while ( ! isEditablePosition(pos1, need_split, interactive) )
	{
		if ( ! pos1.backward_char() )
		break;
	}

	if ( afterAnchor != NULL && pos1.compare(afterAnchor->getMark()->get_iter()) < 0 )
	pos1 = afterAnchor->getMark()->get_iter();

	return pos1;
}

/**
 * return next non-space text position after given text position
 * @param pos current text position
 * @param editable_only if true, stop at first non-space or non-editable char
 * @return text iterator of next non-space character
 */
Gtk::TextIter AnnotationBuffer::nextNonSpacePosition(const Gtk::TextIter& pos, bool editable_only)
{
	Gtk::TextIter it = pos;
	while ( g_unichar_isspace(*it) && (it.editable() || !editable_only) ) if ( ! it.forward_char() ) break;
}

/**
 *  move cursor to editable position
 * @param forward if true move in forward direction, else move backward
 * @param downward if true move in downward direction, else move upward
 * @param remain_on_line if true forbids going to prev/next line
 * @param interactive true if called upon user action, else false
 *
 * @note : distinction is made between going forward and going downward to cope with some bidirectionnal behaviours
 */
void AnnotationBuffer::moveCursorToEditablePosition(bool forward, bool downward, bool remain_on_line, bool interactive)
{
	Gtk::TextIter pos = get_insert()->get_iter();
	int nl = pos.get_line();

	if ( forward )
	{
		if ( downward )
		{
			const Gtk::TextIter& pos1 =nextEditablePosition(pos, NULL, interactive);
			//			TRACE_D << "moveCursorToEditablePosition  from " << pos << " to " << pos1 << endl;
			if ( pos1.get_line() == nl || !remain_on_line ) pos = pos1;
		}
		else
		{
			const Gtk::TextIter& pos1 =nextEditablePosition(pos, NULL, interactive);
			if ( pos1.get_line() == nl ) pos = pos1;
			else if ( !remain_on_line )
			{
				pos = previousEditablePosition(pos, NULL, interactive);
				if ( pos.is_start() ) pos=pos1;
			}
		}
		setCursor(pos, false);
	}
	else
	{
		bool need_split;
		bool ok =true;

		while ( ok && ! ( pos.is_start() || isEditablePosition(pos, need_split, interactive))
				&& ( !remain_on_line || pos.get_line() == nl) )
		ok = pos.backward_char();
		if ( remain_on_line && pos.get_line() != nl ) pos.forward_char();
		setCursor(pos, false);
	}
}

/**
 * move cursor to anchor with given id
 * @param id anchor id
 * @param to_edit_pos if true, places cursor on next editable position
 * @return true if cursor moved, else false
 */
bool AnnotationBuffer::moveCursorToAnchor(const string& id, bool to_edit_pos)
{
	Anchor* anchor = anchors().getAnchor(id);
	if ( anchor != NULL )
	{
		const Gtk::TextIter& pos = anchor->getMark()->get_iter();
		//		TRACE <<  id << " is at " << pos << endl;
		if ( to_edit_pos )
		place_cursor(nextEditablePosition(pos));
		else
		place_cursor(pos);
		return true;
	}
	return false;
}

/**
 * Moves cursor to anchor with given id
 * @param id anchor 		Anchor id
 * @param to_edit_pos 	If true, places cursor on previous editable position (ie before any tagged element attached to end anchored element)
 * @return 				True if cursor moved, else false
 */
bool AnnotationBuffer::moveCursorToAnchorEnd(const string& id, bool to_edit_pos)
{
	Anchor* anchor = anchors().getAnchor(id);
	if ( anchor != NULL )
	{
		Anchor* end_anchor = anchors().getEndAnchor(*anchor);
		Gtk::TextIter pos = end();
		if ( end_anchor != NULL )
		{
			pos = end_anchor->getMark()->get_iter();
			// go back to editable position
			pos = previousEditablePosition(pos, id, to_edit_pos);
		}
		place_cursor(pos);
		return true;
	}
	return false;
}

/**
 *	All anchors (i.e mark buffer) are right gravity. In most of cases they are
 *	followed by a non-editable tag, so edition is allowed at their left by the
 *	gravity, and at their right by the non-editable tag.
 *
 *	==> If there's no tag after the mark (first unit of segment or after label)
 *		problem for inserting at the right of the mark... So we need to move
 *		the mark after insertion.
 *
 *  This method returns the anchor that would need to be moved if a text
 *  insertion occurred at iter position / NULL if no anchor to be moved
 */
Anchor* AnnotationBuffer::markNeedsLeftGravity(const Gtk::TextIter& iter, const string& exclude, bool textType)
{
	//> -- See anchor at position
	Anchor* a = anchors().getAnchorAtPos(iter, m_unitType, false, textType, exclude);

	//> -- Nothing ? good
	if ( a == NULL )
	return NULL;

	//> -- Iter has no tag (should only occur for first mainstreamn base element of segment base element)
	// 	   ==> must stay on start of line, valid candidate to left gravity
	bool proceed = !iterHasTags(iter, m_leftGravityTags, true);

	//> -- Specific case : insertion between 2 tags
	/*
	 * Warning: when inserting between 2 tags, model makes a split and creates a text unit,
	 * therefore we check the "textType condition"
	 */
	if (!proceed && textType)
	{
		Gtk::TextIter tmp = iter;
		// -- Proceed only if previous pos is tagged
		if ( tmp.backward_char() && iterHasTags(tmp, m_leftGravityTags, true) )
		{
			Anchor* b = anchors().getAnchorAtPos(tmp, m_unitType, true, false, "");
			// -- Don't move mark if we have text unit at previous position (means we are inserting between 2 anchored text unit)
			//	  Otherwise, proceed
			if ( (b && !b->isTextType()) || !b)
			proceed = iterHasTags(iter, m_leftGravityTags, true);
		}
	}

	if ( proceed )
	{
		//		TRACE << "~> markNeedsLeftGravity id=" <<  a->getId()  << std::endl ;
		return a;
	}
	//> -- Otherwise, take care:
	/*     When creating a new segment, a unit is created first. If the creation happens in an empty segment,
	 * 	   the new unit mark is inserted at the same place of the mark of the unit of empty segment. Moreover the new
	 *     unit mark is inserted with tag (see following TODO for understand why).
	 *     ==> Therefore, the previous test - !iterHasTag - will fail EVEN FOR THE UNIT OF THE EMPTY SEGMENT
	 *     ==> Therefore the unit of the empty segment is moved at the next line instead of beeing replaced at its old position.
	 *
	 *     FIX ==> If iter is tagged and is a text unit, let's look at the previous anchor.
	 *     If it's a segment, and if the candidate anchor is the first child of that previous anchor,
	 *     let's move it .
	 *     In all other cases, since candidate anchor is tagged, no need to move it.
	 *
	 *	   TODO It would be sexier not to display the tag when creating the segment (and the unit) but
	 *	   when the unit is created and tell the GUI to be displayed, the segment is not created yet
	 *	   so the unit is not considered as 1st child => since it's our criteria to render or not a tag,
	 *	   the unit is first rendered with a tag (which will be removed at segment display).
	 *	   We should change model mechanism to fix that.
	 */
	else
	{
		Anchor* prevA = anchors().getPreviousAnchor(*a, "");
		if ( prevA == NULL)
		return a;

		// I'm a unit, i'm not an event and and im'just behind my father: i am candidate to left gravity
		if ( a->isTextType() && a->hasLowerPrecedence(*prevA) )
		{
			//			TRACE << "~> markNeedsLeftGravity id=" <<  a->getId()  << std::endl ;
			return a;
		}
	}
	return NULL;
}

void AnnotationBuffer::moveAnchor(const string& id, const Gtk::TextIter& it)
{
	if (id.empty())
	return;

	Anchor* a = anchors().getAnchor(id);
	if (!a)
	return;

	anchors().moveAnchor(a, it, false);
}

/**
 * return insert position given in "text" segment as an offset in chars from
 *  segment start
 * @param id 		Anchor id
 * @param iter 		Text buffer iterator
 * @param trim 		If true, returns offset in "trimmed" text
 * @return offset 	Offset in text starting at anchor up to given iterator.
 *
 * @note : a negative offset indicates that given iterator is placed before eventual
 *  tagged elements attached to given anchor
 */
int AnnotationBuffer::getOffsetInTextSegment(const string& id, const Gtk::TextIter& iter, bool do_trim)
{
	int offset = 0;
	Anchor* start = anchors().getAnchor(id);
	if ( start != NULL )
	{
		if ( start->isTextType() )
		{
			Gtk::TextIter pos2 = nextEditablePosition(start->getMark()->get_iter());
			if ( do_trim )
			while ( pos2.editable() && pos2.get_char() == ' ' && pos2.compare(iter) <= 0 && anchors().getAnchorAtPos(pos2, "") == NULL )
			if ( ! pos2.forward_char() )
			break;
			offset = iter.get_offset() - pos2.get_offset();
			TRACE << "(AnnotationBuffer::getOffsetInTextSegment) text_start=" << pos2 << " cur=" << iter << " offset=" << offset << endl;
		}
	}
	else
	{
		Log::err() << "getOffsetInTextSegment: No anchor with id = " << id << endl;
	}
	return offset;
}

// TRES PROVISOIRE POUR DEMO
void AnnotationBuffer::setTrack(const string& type, const string& id, int notrack, bool fromOverlap, bool r2l)
{
	string alt_origTag;
	if (fromOverlap)
	{
		if (!r2l)
		alt_origTag = "track3";
		else
		alt_origTag = "track2";
	}
	else
	alt_origTag = (notrack==0 ? "track1" :"track0");

	const Glib::RefPtr<Gtk::TextTag>& origTag = get_tag_table()->lookup(alt_origTag);
	const Glib::RefPtr<Gtk::TextTag>& dstTag = get_tag_table()->lookup(notrack==0 ? "track0" :"track1");
	Anchor* start = anchors().getAnchor(id);

	INVALID( (start == NULL), "no anchor for id ", id );

	Gtk::TextIter pos = start->getMark()->get_iter();

	if ( ! pos.has_tag(origTag) )
	{ /*MSGOUT << " not changing track tag " << endl ;*/return;}

	Gtk::TextIter endpos = pos;
	endpos.forward_to_tag_toggle(origTag);

	remove_tag(origTag, pos, endpos);
	apply_tag(dstTag, pos, endpos);
}

void AnnotationBuffer::backupInsertPosition(int no, bool left_gravity)
{
	INVALID ( (no < 0 || no >= 3), "invalid backup insert no", no );
	if ( m_backup[no]->get_left_gravity() != left_gravity )
	{
		string name = m_backup[no]->get_name();
		delete_mark(m_backup[no]);
		m_backup[no] = create_mark(name, get_insert()->get_iter(), left_gravity);
	}
	else
	{
		move_mark(m_backup[no], get_insert()->get_iter());
	}
	//	if (no==0) TRACE << "BACKUP INSPOS " << no << " = " << m_backup[no]->get_iter() << " lg=" << m_backup[no]->get_left_gravity() << " (" << m_backup[no]->get_iter().get_char() <<")" << endl;
}

void AnnotationBuffer::restoreInsertPosition(int no, bool to_edit_pos)
{
	INVALID ( (no < 0 || no >= 3), "invalid backup insert no", no );
	Gtk::TextIter pos = m_backup[no]->get_iter();
	setCursor(pos, to_edit_pos);
	//	if ( no==0) TRACE << "RESTORE INSPOS " << no << " = " << m_backup[no]->get_iter() << " lg=" << m_backup[no]->get_left_gravity() << " (" << m_backup[no]->get_iter().get_char() <<")"  << endl;
	clearSelection();
}

//
// returns true if current text position tagged with "label" tag,
//    and eventually other presentation tags, but no annotation tag.
//    else returns false.
bool AnnotationBuffer::isLabelOnly(const Gtk::TextIter& iter)
{
	if ( !iter.editable() )
	return getActiveTagClass(iter).empty();
	return false;
	//	if ( !iter.editable() )
	//		return iterHasTag(iter, m_labelTag->property_name().get_value(), false, false) ;
	//	return false ;
}

void AnnotationBuffer::setEditable(const Gtk::TextIter& iter, bool editable)
{
	m_view->setUndoableActions(false);
	if ( editable && !iter.editable() )
	{
		Gtk::TextIter start, stop;
		Gtk::TextIter next = iter;
		// -- Clear any active selection ! to avoid non-editable text to be overwritten
		if ( get_selection_bounds(start, stop) ) clearSelection();
		next.forward_char();
		// -- On pose une marque repere car on va rendre la position à iter editable, et
		// 	  il faudra la remettre à non editable après traitement de l'event
		_tmpEditMark = create_mark(iter, false);
		// -- Force "label" tag from iter pos
		apply_tag(m_editableTag, iter, next);
		//		remove_tag(m_labelTag, iter, next);
	}
	else if ( _tmpEditMark != 0 )
	{
		Gtk::TextIter next = _tmpEditMark->get_iter();
		next.forward_char();
		remove_tag(m_editableTag, _tmpEditMark->get_iter(), next);
		//		apply_tag(m_labelTag, _tmpEditMark->get_iter(), next);
		delete_mark(_tmpEditMark);
		_tmpEditMark = (Glib::RefPtr<Gtk::TextMark>)0;
	}
	m_view->setUndoableActions(true);
}

/**
 * get selected text
 * @param iter (in) current iterator position
 * @param start (OUT) selection start iterator
 * @param stop  (OUT) selection stop iterator
 * @param adjustToWords	if true, adjust selected range to word boundaries
 * @return true if text selected, else false
 */
bool AnnotationBuffer::getSelectedRange(const Gtk::TextIter& iter, Gtk::TextIter& start, Gtk::TextIter& stop, bool adjustToWords)
{
	bool separated_words = true;
	bool has_sel = true;

	if ( ! (has_sel = get_selection_bounds(start, stop)) )
	{
		start = iter;
		stop = iter;
	}

	if ( ! adjustToWords ) return has_sel;

	/*
	 *  Do special processing for enabling quick word tagging without selecting
	 *  word: research start and end word and apply tag.
	 *
	 *  Don't process for languages without word separation and keep selection as it is
	 */
	InputLanguage* il = InputLanguageHandler::get_input_language_by_char(start.get_char());
	if ( il != NULL && il->modifyMapping())
	separated_words = il->isSpaceSeparated();
	if ( separated_words )
	{
		int cnt = 0;
		gunichar c;

		//> let's go backward until we find a space char or a delimiter char
		while ( (c=start.get_char()) > 0
				&& !g_unichar_isspace(start.get_char())
				&& start.editable()
				&& strchr(WORD_DELIMS, (char)c) == NULL )
		{
			if (!start.backward_char())
			break;
			else
			++cnt;
		}

		if (cnt > 0)
		start.forward_char();

		if ( stop.compare(start) != 0)
		{
			while ( g_unichar_isspace(start.get_char()) && start.editable() )
			start.forward_char();
			gunichar c;
			c=stop.get_char();
			if (c > 0 && ! g_unichar_isspace(c) )
			{
				// check if prev char is space
				Gtk::TextIter prev=stop;
				--prev;
				if ( prev.compare(start) > 0 && g_unichar_isspace(prev.get_char() ) )
				stop=prev;
			}
			c=stop.get_char();
			while ( stop.editable() && c > 0 && !g_unichar_isspace(c) )
			{
				if ( g_unichar_ispunct(c) )
				if (strchr(WORD_DELIMS, (char) c) != NULL)
				break;
				if (!stop.forward_char())
				break;
				c=stop.get_char();
			}
		}
	}
	//	if ( cnt > 0 ) stop.backward_char();

	TRACE << "after adjust " << " start = " << start << " stop=" << stop << " text='" << get_text(start, stop, true) << "'" << endl;

}

/*========================================================================
 *
 *  Events bindings
 *
 ========================================================================*/

/*
 void AnnotationBuffer::hideTags(bool doit)
 {
 Log::err() << " IN HIDE TAGS" << endl;

 g_object_set(G_OBJECT(_tags["speaker"]), "invisible",
 (doit ? TRUE : FALSE), NULL);
 }
 */

void AnnotationBuffer::on_apply_tag(const Glib::RefPtr<Gtk::TextBuffer::Tag>& tag,
		const Gtk::TextIter& range_begin, const Gtk::TextIter& range_end)
{
	//	TRACE << "IN on_apply_tag " << tag->property_name().get_value() << " from " << range_begin.get_offset() << " to " << range_end.get_offset()<< endl << flush;
	Gtk::TextBuffer::on_apply_tag(tag, range_begin, range_end);
}

void AnnotationBuffer::on_remove_tag(const Glib::RefPtr<Gtk::TextBuffer::Tag>& tag,
		const Gtk::TextIter& range_begin, const Gtk::TextIter& range_end)
{
	//	TRACE << "IN on_remove_tag " << tag->property_name().get_value() << " from " << range_begin.get_offset() << " to " << range_end.get_offset() << endl << flush;
	Gtk::TextBuffer::on_remove_tag(tag, range_begin, range_end);
}

void AnnotationBuffer::on_mark_set(const Gtk::TextIter& pos,
		const Glib::RefPtr<Gtk::TextBuffer::Mark>& mark)
{
	const string& name=mark->get_name();
	bool insertFlag = false;

	if ( !m_inhibSignal )
	{
		bool check_sel = false;
		if ( name.compare(0,6,"insert") == 0 )
		{
			//			TRACE_D << " AnnotationBuffer : emit set cursor at " << pos << endl;
			m_signalSetCursor.emit(pos);
			check_sel= true;
			insertFlag = true;
		}
		else if ( name.compare(0,15,"selection_bound") == 0 )
		{
			if (m_view != NULL && m_view->isDebugMode())
			m_signalSetCursor.emit(pos);
			check_sel= true;
		}
		if ( check_sel )
		{
			bool sel = (get_selection_bound()->get_iter().compare(get_insert()->get_iter()) != 0 );
			//		TRACE << "IN on_mark_set " << mark->get_name() << " at pos " << pos.get_offset() << " SEL =" << sel << endl << flush;
			if ( sel != m_hasSelection )
			m_signalSelectionSet.emit(sel);
			m_hasSelection = sel;
		}
	}// end_inhibate

	Gtk::TextBuffer::on_mark_set(pos, mark);
	static int i = 0;
	i++;
	if (m_view != NULL && m_language_change_activated == true /*&& insertFlag == true*/)
	{
		gunichar c;
		Gtk::TextIter it = mark->get_iter();
		Glib::UnicodeType type;
		bool ok = false;

		//check if cursor is at beginning of segment or if there is a char after
		// TODO ICI CHECK USAGE TAG SPEECH -> pour l'instant on commente
		//		const Glib::RefPtr<Gtk::TextTag>& tag = get_tag_table()->lookup("speech");
		bool cursorAtSegmentStart = false;
		while (it != begin() && ok == false && cursorAtSegmentStart == false)
		{
			it.backward_char();
			//			if(tag != 0 && it.has_tag(tag) == true)
			//				cursorAtSegmentStart = true;
			//			else
			if (it.editable() == false)
			continue;
			else
			{
				c = it.get_char();
				type = Glib::Unicode::type(c);
				if(type == Glib::UNICODE_LOWERCASE_LETTER ||
						type == Glib::UNICODE_UPPERCASE_LETTER ||
						type == Glib::UNICODE_OTHER_LETTER)
				{
					InputLanguage *il = InputLanguageHandler::get_input_language_by_char(c);
					if (il==NULL)
					il = InputLanguageHandler::get_input_language_by_shortcut(DEFAULT_LANGUAGE);
					if(this->m_view && il != m_view->getParent().get_input_language())
					m_view->getParent().set_input_language(il);
					ok = true;
					//				Log::trace() << " ok ";
				}
			}
		}
		if(cursorAtSegmentStart == true)//cursor at beginning of segment

		{
			//		Log::trace() << " cursor at segment start";
			bool changed = false;
			if(mark->get_iter() != end())
			{
				c = mark->get_iter().get_char();
				type = Glib::Unicode::type(c);
				if((type == Glib::UNICODE_LOWERCASE_LETTER ||
								type == Glib::UNICODE_UPPERCASE_LETTER ||
								type == Glib::UNICODE_OTHER_LETTER))
				{
					InputLanguage *il = InputLanguageHandler::get_input_language_by_char(c);
					if (il==NULL)
					il = InputLanguageHandler::get_input_language_by_shortcut(DEFAULT_LANGUAGE);
					if(this->m_view && il != m_view->getParent().get_input_language())
					{
						m_view->getParent().set_input_language(il);
						changed = true;
						//					Log::trace() << " changed = true";
					}
				}
			}
			else;
			//				Log::trace() << " iter = end";
			if(changed == false)
			{
				InputLanguage *il = InputLanguageHandler::get_input_language_by_shortcut(m_view->getParent().getTranscriptionLanguage());
				if (il==NULL)
				il = InputLanguageHandler::get_input_language_by_shortcut(DEFAULT_LANGUAGE);
				if(this->m_view)
				m_view->getParent().set_input_language(il);
			}
		}
	}
	//	Log::trace() << std::endl;
}

void
AnnotationBuffer::removePropagatingTags(const Gtk::TextIter& start, const Gtk::TextIter& stop)
{
	//	cout << "+++ Remove all propagating tags" << endl;

	const list< Glib::RefPtr<Gtk::TextTag > >& tags = const_cast<Gtk::TextIter&>(start).get_tags();
	list< Glib::RefPtr<Gtk::TextTag > >::const_iterator it;

	for ( it = tags.begin(); it != tags.end(); ++it )
	{
		vector< Glib::RefPtr<Gtk::TextTag> >::iterator itp;
		for (itp= m_propagatingTags.begin(); itp!=m_propagatingTags.end(); itp++)
		{
			if ( *it == *itp )
			{
				//			TRACE_D << "\n removing " << (*it)->property_name() << endl;
				remove_tag(*it, start, stop);
				break;
			}
		}
		if ( itp == m_propagatingTags.end() )
		{ // is it an "id" tag" ?
			if (strncmp((*it)->property_name().get_value().c_str(), m_idTagPrefix.c_str(), m_idTagPrefix.length()) == 0 )
			remove_tag(*it, start, stop);
		}
	}
}

//
// TODO -> en sursis, à virer dès que on aura vérifié que est inutile pour undo/redo.
//
void AnnotationBuffer::addPropagatingTag(const Glib::RefPtr<Gtk::TextTag>& tag)
{
	bool found = false;
	vector< Glib::RefPtr<Gtk::TextTag> >::iterator it;
	for (it= m_propagatingTags.begin(); it!=m_propagatingTags.end() && !found; it++)
	{
		if ( tag->property_name() == (*it)->property_name() )
		found = true;
	}
	if (!found)
	m_propagatingTags.push_back(tag);
}

/**
 * Inserts a text in the buffer regarding the possibility of insertion
 * @param range_begin			Text start position
 * @param range_end				Text end position
 * @param default_editable		Whether the buffer is editable by default. If set to False, will block the deletion
 * @return						New iterator position, and whether the deletion has been done.
 */
std::pair<Gtk::TextIter, bool>
AnnotationBuffer::erase_interactive (const Gtk::TextIter& range_begin, const Gtk::TextIter& range_end, bool default_editable)
{
	//	cout << "IN AnnotationBuffer::erase_interactive from" << range_begin << " to " << range_end << " , has_tag=" << range_begin.has_tag(m_confidenceTag) << endl;
	if ( default_editable && m_view->getWithConfidence() && ! range_begin.has_tag(m_confidenceTag))
	{
		begin_user_action();
		applyConfidenceTag(range_begin);
		const std::pair<Gtk::TextIter, bool>& res = Gtk::TextBuffer::erase_interactive(range_begin, range_end, default_editable);
		end_user_action();
		return res;
	}
	else
	Gtk::TextBuffer::erase_interactive(range_begin, range_end, default_editable);
}
/** Deletes the range between the "insert" and "selection_bound" marks,
 * that is, the currently-selected text. If @a interactive  is <tt>true</tt>,
 * the editability of the selection will be considered (users can't delete
 * uneditable text).
 * @param interactive Whether the deletion is caused by user interaction.
 * @param default_editable Whether the buffer is editable by default.
 * @return Whether there was a non-empty selection to delete.
 */
bool AnnotationBuffer::erase_selection(bool interactive, bool default_editable)
{
	const Gtk::TextIter& range_begin = get_insert()->get_iter();
	//	cout << "IN AnnotationBuffer::erase_selection at " << range_begin << " , has_tag=" << range_begin.has_tag(m_confidenceTag)<<  endl;
	if ( interactive && default_editable && m_view->getWithConfidence() && ! range_begin.has_tag(m_confidenceTag) )
	{
		begin_user_action();
		applyConfidenceTag(range_begin);
		bool res = Gtk::TextBuffer::erase_selection(interactive, default_editable);
		end_user_action();
		return res;
	}
	else
	Gtk::TextBuffer::erase_selection(interactive, default_editable);
}

std::pair<Gtk::TextIter,bool>
AnnotationBuffer::insert_interactive(const Gtk::TextIter& pos, const Glib::ustring& text, bool default_editable)
{
	//	cout << "IN AnnotationBuffer::insert_interactive at " << pos << " , has_tag=" << pos.has_tag(m_confidenceTag)<<  endl;
	bool need_split;
	if (default_editable && isEditablePosition(pos, need_split, true))
	{
		InputLanguage *current_input_language = this->m_view->getParent().get_input_language();
		bool ok = true;
		if (current_input_language)
		ok = current_input_language->check_insertion_rules(this->m_view, pos, text);
		if ( ok )
		{
			m_interactiveInsert = true;
			m_leftGravityTextMode = true;
			Gtk::TextIter it = Gtk::TextBuffer::insert(pos, text);
			return std::pair<iterator,bool>(it, true);
		}
	}
	m_leftGravityCandidate = NULL;
	return std::pair<iterator,bool>(pos, false);
}

bool AnnotationBuffer::applyConfidenceTag(const Gtk::TextIter& it)
{
	bool done = false;
	bool firstOfSeg = false;

	// if with_confidence and curpos isn't tagged with confidence -> set confidence
	if ( m_view->getWithConfidence() )
	{
		bool need_tag = false;
		Anchor* a = anchors().getAnchorAtPos(it, m_unitType, false);
		// no anchor, tag can be needed
		if ( it.editable() && a == NULL )
		need_tag = !it.has_tag(m_confidenceTag);
		// anchor ? two cases possible :
		// 1 - we're at an anchor but there are another before
		// 2 - we're at the first anchor of a segment

		else
		{
			Gtk::TextIter prev = it;
			if ( prev.backward_char() )
			{
				need_tag = (prev.editable() && !prev.has_tag(m_confidenceTag));
				// check for segment start position : prev will not be editable, pass through
				if ( !prev.editable() && a && !it.has_tag(m_confidenceTag))
				{
					Anchor* pa = anchors().getPreviousAnchor(*a, "");
					if (pa && pa->getType()!=a->getType() && pa->getPrecedenceIndex() < a->getPrecedenceIndex())
					{
						firstOfSeg = true;
						need_tag = true;
					}
				}
			}
		}

		// -- Let's proceed
		if ( need_tag )
		{
			// We're not a segment start, means we need to tag the the unit before
			// the one we're at : let's go backward
			if ( a != NULL && !firstOfSeg)
			a = anchors().getPreviousAnchor(*a, m_unitType);
			// We have no anchor, let's found the first backwared

			else if ( a == NULL )
			{
				const string& aid = anchors().getPreviousAnchorId(it, m_unitType, false);
				if ( !aid.empty() )
				a = anchors().getAnchor(aid);
			}
			if ( a != NULL )
			{
				Anchor* b = anchors().getEndAnchor(*a, false);
				if ( b != NULL )
				{
					Gtk::TextIter it1 = nextEditablePosition(a->getMark()->get_iter());
					Gtk::TextIter it2= previousEditablePosition(b->getMark()->get_iter());
					++it2;
					//					Log::out() << "~~~~~~~~~~ apply confidence : " << it1 << " - " << it2 << std::endl ;
					apply_tag(m_confidenceTag, it1, it2);
					done=true;
				}
			}
		}
	}
	return done;
}

void AnnotationBuffer::onInsertBefore(const Gtk::TextIter& pos, const Glib::ustring& t, int bytes)
{
	bool userBlockDone = false;

	m_leftGravityCandidate = markNeedsLeftGravity(pos, "", m_leftGravityTextMode);
	if (m_leftGravityCandidate)
	{
		if (m_leftGravityUserBlok)
		{
			begin_user_action();
			userBlockDone=true;
		}
		// keep position - real move will be proceded after insertion
		move_mark(m_leftGravityMark, pos);
	}

	if ( m_interactiveInsert && m_view->getWithConfidence() && !userBlockDone)
	begin_user_action();
}

bool AnnotationBuffer::onInsertAfter(const Gtk::TextIter& start, const Glib::ustring& text, int length)
{
	/**
	 * 	CAUTION:
	 * 	Don't add or remove text here, cause it would invalidate some iterators
	 * 	used by internal Gtk mechanisms
	 */

	if (!m_interactiveInsert && !m_leftGravityCandidate)
	return false;

	//> -- Left gravity problem
	if (m_leftGravityCandidate)
	{
		anchors().moveAnchor(m_leftGravityCandidate, m_leftGravityMark->get_iter(), false);
		m_leftGravityCandidate = NULL;
	}

	//> -- Tag business
	//  CAUTION : do it ATER gravity adjustment because we'll need some anchor data
	bool do_confidence = m_view->getWithConfidence();
	if (m_interactiveInsert)
	{
		Gtk::TextIter tmp = start;
		for ( int i = 0; i < length; i++ )
		tmp.backward_char();

		if ( do_confidence )
		applyConfidenceTag(tmp);

		removePropagatingTags(tmp, start);
	}

	if (m_leftGravityUserBlok || (m_interactiveInsert && do_confidence) )
	end_user_action();

	m_leftGravityUserBlok = false;
	m_leftGravityTextMode = false;
	m_interactiveInsert = false;

	return false;
}

//
// check insert events -> set pending updates when paste in buffer
//
void AnnotationBuffer::on_insert(const Gtk::TextIter& pos, const Glib::ustring& t, int bytes)
{
	if ( !m_inhibSignal )
	{
		guint offset = pos.get_offset();
		Gtk::TextBuffer::on_insert(pos, t, t.bytes());
		m_signalHasEdits.emit(get_iter_at_offset(offset), t.bytes());
	}
	else
	Gtk::TextBuffer::on_insert(pos, t, t.bytes());
}

bool AnnotationBuffer::needSpaceBorders(gunichar c)
{
	std::set<gunichar>::iterator it = m_borderedChars.find(c);
	return (it!=m_borderedChars.end());
}

bool AnnotationBuffer::spaceDeleter(const Gtk::TextIter& start, const Gtk::TextIter& stop, std::vector<Gtk::TextIter>& res,
		bool spaceHandling, bool spaceBordering)
{
	if (!spaceBordering && !spaceHandling)
	{
		if (start==stop)
		res.push_back(start);
		else
		{
			res.push_back(start);
			res.push_back(stop);
		}
		return true;
	}

	Gtk::TextIter next = stop;
	Gtk::TextIter previous = start;
	bool ret = true;

	// single char
	if (start==stop)
	{
		previous.backward_char();
		next.forward_char();
		gunichar current = start.get_char();
		gunichar nex = next.get_char();
		gunichar prev = previous.get_char();

		//> Deleting a char that needs space-borders
		if ( spaceBordering && needSpaceBorders(current) )
		{
			//> we have to delete the two space-border characters too
			//  BUT ONLY if they're not bording special character
			if (isEditableSpaceChar(nex, next) && isEditableSpaceChar(prev, previous))
			{
				Gtk::TextIter previous_of_previous = previous;
				previous_of_previous.backward_char();
				gunichar prev_of_prev = previous_of_previous.get_char();
				// previous space doesn't guard a special character, we can delete
				if (!needSpaceBorders(prev_of_prev))
				res.push_back(previous);
				else
				res.push_back(start);

				Gtk::TextIter next_of_next = next;
				next_of_next.forward_char();
				gunichar nex_of_nex = next_of_next.get_char();
				// next space doesn't guard a special character, we can delete
				if (!needSpaceBorders(nex_of_nex))
				res.push_back(next);
				else
				res.push_back(stop);

				ret = true;
			}
			// shouldn't be reach: special character but not surrounded by spaces

			else
			{
				ret = true;
				//				gdk_beep() ;
			}
		}
		// Deleting space

		else if ( spaceBordering && isSpaceChar(current) )
		{
			Gtk::TextIter previous_of_previous = previous;
			previous_of_previous.backward_char();
			gunichar prev_of_prev = previous_of_previous.get_char();
			Gtk::TextIter next_of_next = next;
			next_of_next.forward_char();
			gunichar nex_of_nex = next_of_next.get_char();

			//> We're deleting the left space border, delete special char and right space border too
			if (needSpaceBorders(nex) && isEditableSpaceChar(nex_of_nex,next_of_next))
			{
				next_of_next.forward_char();
				res.push_back(start);

				Gtk::TextIter next_of_next_of_next = next_of_next;
				next_of_next_of_next.forward_char();
				gunichar nex_of_nex_of_nex = next_of_next_of_next.get_char();

				// only delete space after if it doesn't guard another special character
				if (!needSpaceBorders(nex_of_nex_of_nex))
				res.push_back(next_of_next);
				else
				res.push_back(next);

				ret = true;
			}
			//> We're deleting the right space border, delete special char and left space border too

			else if (needSpaceBorders(prev) && isEditableSpaceChar(prev_of_prev, previous_of_previous))
			{
				Gtk::TextIter previous_of_previous_of_previous = previous_of_previous;
				previous_of_previous_of_previous.backward_char();
				gunichar prev_of_prev_of_prev = previous_of_previous_of_previous.get_char();

				// only delete space before if it doesn't guard another special character
				if (!needSpaceBorders(prev_of_prev_of_prev))
				res.push_back(previous_of_previous);
				else
				res.push_back(previous);
				res.push_back(next);

				ret = true;
			}
			//> We're deleting a space between 2 special chars, can't be allowed

			else if (needSpaceBorders(nex)
					||needSpaceBorders(prev) )
			{
				ret = false;
				gdk_beep();
				Log::out() << "SpaceEraser -> borders problem." << std::endl;
			}
		}
		// For characters different than special ones or space

		else
		{
			// check if we're not about to collapse 2 spaces
			if (spaceHandling && isEditableSpaceChar(nex, next) && isEditableSpaceChar(prev, previous))
			{
				res.push_back(previous);
				res.push_back(next);
				ret = true;
			}
			// otherwise ok

			else
			{
				ret = true;
				res.push_back(start);
			}
		}
	}
	// selection

	else
	{
		previous.backward_char();
		gunichar nex = next.get_char();
		gunichar prev = previous.get_char();

		// 2 spaces before and after selection, eat one
		if (spaceHandling && isEditableSpaceChar(nex, next) && isEditableSpaceChar(prev, previous))
		{
			res.push_back(previous);
			res.push_back(next);
			ret = true;
		}
		// special char before and/or after selection, don't allow deletion

		else if ( spaceBordering && (needSpaceBorders(prev)
						|| needSpaceBorders(nex)) )
		{
			ret = false;
			gdk_beep();
			Log::out() << "SpaceEraser -> selection borders problem." << std::endl;
		}
		// other cases, OK

		else
		{
			ret = true;
			res.push_back(start);
			res.push_back(stop);
		}
	}

	return ret;
}

Glib::ustring AnnotationBuffer::spaceHandler(const Gtk::TextIter& pos, const Glib::ustring& text, bool use_iter,
		bool spaceHandling, bool spaceBordering)
{
	if (!m_view )
	return "";

	//> Options are unset
	//  do nothing
	if (!spaceHandling && !spaceBordering)
	return text;

	Glib::ustring::const_iterator it, it_prev, it_next;
	Glib::ustring next, res, current, previous;

	Gtk::TextIter beforeFirst, afterLast;
	gunichar beforFirstC = 0;
	gunichar afterLastC = 0;

	//> If using iterator, we have to check character before given text and character after given text !
	if (use_iter)
	{
		beforeFirst = pos;
		afterLast = pos;
		bool okback = beforeFirst.backward_char();
		if (okback)
		{
			beforFirstC = beforeFirst.get_char();
			previous = Glib::ustring(1,beforFirstC);
		}
		// The character following the given text will be determined after all the text be parsed
		// Let's set it at begin
		if (afterLast!=end())
		afterLastC = afterLast.get_char();
	}

	//> -- Iterate inside all text
	for (it=text.begin(); it!=text.end(); it++)
	{
		bool reject = false;

		it_prev = it;
		it_next = it;
		//> If we're at text beginning, previous has been
		//  initialized with the character before the string we're checking
		//  so do nothing
		if (it!=text.begin())
		{
			it_prev--;
			previous = *it_prev;
		}
		//> If we're not at end text, compute next iterator
		if (it!=text.end())
		{
			it_next++;
			if (it_next!=text.end())
			next = *it_next;
			//  ==> if the next iterator is the end, 'next' is the
			//      following character after the text we're checking

			else if (use_iter && afterLastC!=0)
			next = Glib::ustring(1, afterLastC);
			// if no following character, set next empty

			else
			next = "";
		}
		//> same thing if no next iterator

		else
		{
			if (use_iter && afterLastC!=0)
			next = Glib::ustring(1, afterLastC);
			else
			next = "";
		}

		//> Space Handling
		if ( spaceHandling && isSpaceChar(*it) )
		{
			// If next is space, reject current
			// Special case for first char, if previous is space reject too
			if (next== " " || it==text.begin() && previous==" ")
			reject = true;
			else
			current = Glib::ustring(1, *it);
		}
		//> Space Bordering

		else if ( spaceBordering && needSpaceBorders(*it) )
		{
			// no spaces around, add
			if ( (previous!= " ") && (next!= " ") )
			current = " " + Glib::ustring(1, *it) + " ";
			// space after, only add before

			else if (previous!= " ")
			current = " " + Glib::ustring(1, *it);
			// space before, only add after

			else if (next!= " ")
			current = Glib::ustring(1, *it) + " ";
			// space before and after, that's ok

			else
			current = Glib::ustring(1, *it);
		}
		//> Nothing to do, that's ok

		else
		current = Glib::ustring(1, *it);

		if ( !reject )
		res.append(current);
	}

	return res;
}

void AnnotationBuffer::on_erase(const Gtk::TextIter& start, const Gtk::TextIter& stop )
{
	guint start_offset = start.get_offset();

	//TRACE << "on_erase at " << start << " up to " << stop << endl;
	// this will erase all selected editable text
	Gtk::TextBuffer::on_erase(start, stop);

	//	if ( start.editable() )
	m_signalHasEdits.emit(get_iter_at_offset(start_offset), 10); //(stop_offset - start_offset)
}

/*
 *  Clear buffer contents
 */
void AnnotationBuffer::clear()
{
	anchors().clear();
	erase(begin(), end());
}

/*========================================================================
 *
 *  create annotation buffer and return reference on new buffer.
 *
 *========================================================================*/

Glib::RefPtr<AnnotationBuffer> AnnotationBuffer::create()
{
	return Glib::RefPtr<AnnotationBuffer>( new AnnotationBuffer() );
}

/*========================================================================
 *
 *  sexy methods
 *
 *========================================================================*/

void AnnotationBuffer::setFontStyle(const string& font, const string& mode)
{
	Pango::FontDescription fd(font);
	int size = fd.get_size();
	string family = fd.get_family();
	std::vector< Glib::RefPtr<Gtk::TextTag> >::iterator it;

	if (mode.compare("label")==0)
	{
		for (it = m_labelDisplayTags.begin(); it!= m_labelDisplayTags.end(); it++)
		{
			if (*it)
			{
				(*it)->set_property("size", size);
				(*it)->set_property("family", family);
			}
		}
	}
	else if (mode.compare("text")==0)
	{
		//> get view font size, because view font value in configuratio
		// set letter weight that mustn't be changed for labels in that way
		for (it = m_sameTextDisplayTags.begin(); it!= m_sameTextDisplayTags.end(); it++)
		{
			if (*it)
			(*it)->set_property("size", size);
		}
	}
}

void AnnotationBuffer::resetColors()
{

}

string AnnotationBuffer::getLabelLook(const string& label, const string& mode, int coefficient)
{
	string section = "Colors-editor,";
	string key = section + label;
	if ( mode.compare("fg")==0)
	key = key + "_fg";
	else if ( mode.compare("bg")==0)
	key = key + "_bg";
	else if ( mode.compare("style")==0)
	key = key + "_style";
	else if ( mode.compare("weight")==0)
	key = key + "_weight";

	std::string value = "";
	if (m_view)
	{
		value = m_view->getColorsCfgOption(key);
		if (value.empty())
		{
			//			if (m_view->getDataModel().conventions().isQualifierClassType("entity",label))
			if ( m_entities.find(label) != m_entities.end() )
			{
				string key = section + "_" + mode;
				value = m_view->getColorsCfgOption(key);
			}
		}
	}
	if (coefficient!=0)
	ColorsCfg::color_from_str_change_light(value, coefficient);

	return value;
}

Pango::Style AnnotationBuffer::from_CfgStyle_to_Pango(const string& key)
{
	if ( key.compare("italic")==0 )
	return Pango::STYLE_ITALIC;
	else if ( key.compare("oblic")==0 )
	return Pango::STYLE_ITALIC;
	else
	return Pango::STYLE_NORMAL;
}

Pango::Weight AnnotationBuffer::from_CfgWeight_to_Pango(const string& key)
{
	if ( key.compare("bold")==0 )
	return Pango::WEIGHT_BOLD;
	else
	return Pango::WEIGHT_NORMAL;
}

bool AnnotationBuffer::isSpaceChar(char c)
{
	if (c==' ')
	return true;
	else
	return false;
}

bool AnnotationBuffer::isEditableSpaceChar(char c, const Gtk::TextIter& iter)
{
	bool unused;
	if (!isEditablePosition(iter, unused))
	return false;
	else
	{
		if (c==' ')
		return true;
		else
		return false;
	}
}

void AnnotationBuffer::setSpecialPasteInitialized(bool value)
{
	specialPasteInitialized = value;
	Log::out() << "~> special paste initialized " << value << std::endl;
}

string AnnotationBuffer::printCurrentLine()
{
	Gtk::TextIter iter = get_insert()->get_iter();
	iter.forward_to_line_end();
	string text = get_slice(get_insert()->get_iter(), iter, false);
	return text;
}

} /* namespace tag */

std::ostream& operator <<(std::ostream& out, const Gtk::TextIter& iter)
{
	out << iter.get_line() << "." << iter.get_line_offset();
	return out;
}
