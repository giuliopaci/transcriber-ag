/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @file AnnotationView.cpp
 * @brief Annotation text widget implementation
 *
 *   handle user interactions with text editor
 *   handle interactions with dataModel and text representation
 *
 * @note based on undoableTextView widget
 */
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include <gtkmm/accelmap.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/iconinfo.h>
#include <gtk/gtkimcontext.h>

#include <gtkmm/stock.h>

#include <gtkmm/icontheme.h>
#include <gtkmm/iconinfo.h>

#include <gtkmm/stock.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtkimmulticontext.h>
#include <gtk/gtkimmodule.h>

#include <ag/AGException.h>

#include "AnnotationView.h"

#include "Editors/AnnotationEditor/AnnotationEditor.h"

#include "AudioWidget/AudioSignalView.h"

#include "Common/ColorsCfg.h"
#include "Common/iso639.h"
#include "Common/globals.h"
#include "Common/Dialogs.h"
#include "Common/widgets/GtUtil.h"
#include "Common/FileInfo.h"
#include "Common/InputLanguage.h"
#include "Common/InputLanguageArabic.h"
#include "Common/util/StringOps.h"
#include "Common/util/Utils.h"

#include "DataModel/conventions/Topics.h"
#include "DataModel/speakers/Speaker.h"
#include "DataModel/DataModel_CPHelper.h"

#include "Editors/AnnotationEditor/dialogs/annotation_dialogs.h"
#include "Editors/AnnotationEditor/menus/annotation_menus.h"
#include "Editors/AnnotationEditor/renderers/annotation_renderers.h"

using namespace std;

using namespace Gtk::Menu_Helpers;

/* SPELL */
//extern "C" {
//#include <gtkspell/gtkspell.h>
//}

namespace tag {

#define ANNOT_MAX_UNDO_LEVEL  100
#define MAX_PENDING 5

//** SCIM
/*
 * signal handlers for preedit start / stop
 */
static void my_preedit_start_cb(GtkIMContext *context, AnnotationView* view)
{
	view->on_preedit_start();
}

static void my_preedit_end_cb(GtkIMContext *context, AnnotationView* view)
{
	view->on_preedit_end();
}

static void my_preedit_change_cb(GtkIMContext *context, AnnotationView* view)
{
	view->on_preedit_changed();
}

//** SCIM

static void trim(Glib::ustring& txt2)
{
	while (g_unichar_isspace(txt2[0]) ) txt2.erase(0,1);
	guint l = txt2.length() -1;
	while (l >= 0 && g_unichar_isspace(txt2[l]) ) --l;
	++l;
	txt2.erase(l);
}

/* constructors */

/**
 * constructs a new annotation view for parent annotation editor
 * @param editor parent annotation editor
 * @note calls initView to proceed to all initialisations
 */
AnnotationView::AnnotationView(AnnotationEditor& editor)
: UndoableTextView(AnnotationBuffer::create()),
m_parent(editor),
m_dataModel(editor.getDataModel()),
m_configuration(editor.getConfiguration()),
m_colorsCfg(editor.getColorsCfg()),
m_lang(editor.getTranscriptionLanguage()),
m_viewTrack(-1),
m_activeCursor(Gdk::HAND1),
m_textCursor(Gdk::XTERM),
m_waitCursor(Gdk::WATCH),
m_overlapState(false),
m_spaceHandlingModified(false)
{
	initView();
}

/**
 * constructs a new annotation view for parent annotation editor and given annotation buffer
 * @param editor parent annotation editor
 * @param buffer associated annotation buffer
 * @note calls initView to proceed to all initialisations
 */
AnnotationView::AnnotationView(AnnotationEditor& editor, Glib::RefPtr<AnnotationBuffer>& buffer)
: UndoableTextView(buffer),
m_parent(editor),
m_dataModel(editor.getDataModel()),
m_configuration(editor.getConfiguration()),
m_colorsCfg(editor.getColorsCfg()),
m_viewTrack(-1),
m_activeCursor(Gdk::TOP_LEFT_ARROW),
m_textCursor(Gdk::XTERM),
m_waitCursor(Gdk::WATCH),
m_overlapState(false)
{
	initView();
}

/**
 * release annotation view allocated resources
 */
AnnotationView::~AnnotationView()
{
	// -- clean speller
/* SPELL */
//	detachSpeller();

	// -- clean tooltip
	if (m_tooltip)
	{
		m_tooltip->stop() ;
		delete (m_tooltip) ;
	}

	// -- clean renderers
	std::map <string, AnnotationRenderer*>::iterator itr ;
	for (itr = m_renderer.begin(); itr != m_renderer.end(); ++itr )
	{
		if (itr->second != NULL)
			delete itr->second;
	}
}

/**
 * detach speller associated to current annotation view
 */
/* SPELL */
/*
void AnnotationView::detachSpeller()
{
	if ( m_speller != NULL ) {
		gtkspell_detach(m_speller);
		m_speller = NULL;
	}
}
*/


/**
 * @return pointer on top window for current view
 */
Gtk::Window& AnnotationView::getTopWindow()
{
	Gtk::Window* top = m_parent.getTopWindow();
	return *top;
}

/**
 * initialize annotation view
 * @param editable true if text view is editable, else false
 * @note configure underlying text view, create associated action groups
 */
void AnnotationView::initView(bool editable)
{

	m_withConfidence = true ;
	disable_thread = false ;
	m_signalTrack = ( m_viewTrack == -1 ? 0 : m_viewTrack);
/* SPELL */
//	m_speller = NULL;
	m_pendingTextEdits = 0;
	m_inhibPendingEdits = false;
	m_inhibateSynchro = false;
	m_autosetLanguage = false;
	m_currentId = "";
	m_squeezeNoSpeech = false;
	m_novalue = "";
	setUndoableActions(false) ;

	IME_on = false ;
	focus_in_locked = false;

	set_editable(editable); // widget editable
	setEditable(editable); // view editable

	set_wrap_mode (Gtk::WRAP_WORD);
	set_pixels_above_lines(4);
	set_pixels_below_lines(4);
	set_pixels_inside_wrap(2);

	m_isActiveCursor=false;

	add_events(Gdk::POINTER_MOTION_HINT_MASK);

	createAnnotationActionGroup();

	//> -- Undo Part signals to view or datamodel modifications done
	undoRedoActionSignal().connect(sigc::bind<bool>(sigc::mem_fun (*this, &AnnotationView::storePendingTextEdits), true));
	signalCustomUndoRedo().connect(sigc::mem_fun (m_dataModel, &UndoableDataModel::onUndoRedo)) ;
	signalUndoAllUndone().connect(sigc::mem_fun (*this, &AnnotationView::onUndoAllDone)) ;

	//> -- Connect set cursor event for signal-text synchronisation
	getBuffer()->signalSetCursor().connect(sigc::bind<bool>(sigc::mem_fun (*this, &AnnotationView::emitSetCursor), false));
	getBuffer()->signalSelectionSet().connect(sigc::mem_fun (*this, &AnnotationView::onSelectionSet));
	getBuffer()->signalHasEdits().connect(sigc::mem_fun (*this, &AnnotationView::setHasPendingEdits));

	//> -- Connect buffer tag events
	getBuffer()->signalTagEvent().connect(sigc::mem_fun (*this, &AnnotationView::onBufferTagEvent));

	//> -- Connect anchors signal
	getBuffer()->anchors().signalMoveAnchor().connect(sigc::mem_fun (*this, &AnnotationView::onBufferAnchorMoveEvent));

	//> -- Connect to data model update events
	m_dataModel.signalElementModified().connect(sigc::bind<bool,bool>(sigc::mem_fun (*this, &AnnotationView::updateView), true, false));
	m_dataModel.signalUndoableAction().connect(sigc::mem_fun (*this, &AnnotationView::onUndoableAction));
	m_dataModel.signalUndoRedoModification().connect(sigc::mem_fun (*this, &AnnotationView::updateAfterUndoRedo));
	m_dataModel.signalUndoRedoStackCorruption().connect(sigc::mem_fun (*this, &AnnotationView::displayUndoRedoError));

	//> -- Add copy / paste / cut actions to edit action groupname
	m_editActions["edit_cut"] = Gtk::Action::create("edit_cut", Gtk::Stock::CUT);
	m_editActionGroup->add(m_editActions["edit_cut"], Gtk::AccelKey("<control>x"),
				sigc::bind<string, bool>(sigc::mem_fun(*this, &AnnotationView::onEditAction), "cut", false));
	m_editActions["edit_copy"] = Gtk::Action::create("edit_copy", Gtk::Stock::COPY);
	m_editActionGroup->add(m_editActions["edit_copy"], Gtk::AccelKey("<control>c"),
				sigc::bind<string, bool>(sigc::mem_fun(*this, &AnnotationView::onEditAction), "copy", false));
	m_editActions["edit_paste"] = Gtk::Action::create("edit_paste", Gtk::Stock::PASTE);
	m_editActionGroup->add(m_editActions["edit_paste"], Gtk::AccelKey("<control>v"),
				sigc::bind<string, bool>(sigc::mem_fun(*this, &AnnotationView::onEditAction), "paste", false));
	m_editActions["edit_paste_special"] = Gtk::Action::create("edit_paste_special",  _("Special paste"));
	m_editActionGroup->add(m_editActions["edit_paste_special"], Gtk::AccelKey("<control><shift>v"),
				sigc::bind<string, bool>(sigc::mem_fun(*this, &AnnotationView::onEditAction), "paste_special", false));
	m_editActions["edit_cut"]->set_tooltip(_("Cut")) ;
	m_editActions["edit_copy"]->set_tooltip(_("Copy")) ;
	m_editActions["edit_paste"]->set_tooltip(_("Paste")) ;
	m_editActions["edit_paste_special"]->set_tooltip(_("Special paste")) ;

	//> -- Add input language related actions
	m_editActions["edit_language"] = Gtk::Action::create("edit_language",_("Input language"));
	m_editActionGroup->add(m_editActions["edit_language"]);

	m_editActions["edit_next_input_language"] = Gtk::Action::create("edit_next_input_language", _("Next language"));
	m_editActionGroup->add(m_editActions["edit_next_input_language"],
				Gtk::AccelKey("<control><shift>Page_Down"),
				sigc::mem_fun(*this, &AnnotationView::onNextInputLanguage));
	m_editActions["edit_previous_input_language"] = Gtk::Action::create("edit_previous_input_language", _("Previous language"));
	m_editActionGroup->add(m_editActions["edit_previous_input_language"],
				Gtk::AccelKey("<control><shift>Page_Up"),
				sigc::mem_fun(*this, &AnnotationView::onPreviousInputLanguage));

	//> -- Add view clipboard
	m_refClipBoard = Gtk::Clipboard::get();
	m_refClipBoardSpecial = Gtk::Clipboard::get(GDK_SELECTION_SECONDARY);

	m_editActions["edit_cut"]->set_sensitive(false);
	m_editActions["edit_copy"]->set_sensitive(false);

	bool can_paste = true;
	m_editActions["edit_paste"]->set_sensitive(can_paste);
	m_editActions["edit_paste_special"]->set_sensitive(can_paste);

	m_clipboardContents = "";

	//> -- Connect current input context preedit signals to deal with editable/non editable positions
	GtkIMContext* context = this->gobj()->im_context ;
	g_signal_connect (context, "preedit_start", G_CALLBACK (my_preedit_start_cb), this);
	g_signal_connect (context, "preedit_end", G_CALLBACK (my_preedit_end_cb), this);
	g_signal_connect (context, "preedit_changed", G_CALLBACK (my_preedit_change_cb), this);

	//> -- Prepare a tooltip widget
	m_tooltip = new class AnnotationViewTooltip(*this) ;

	//> -- Configuration undoable action
	setUndoableActions(true) ;
}

/**
 * create annotation renderers
 */
void AnnotationView::createAnnotationRenderers()
{
	std::map<std::string, AnnotationRenderer*>::iterator it;
	for ( it=m_renderer.begin(); it != m_renderer.end(); ++it)
		delete it->second;
	m_renderer.clear();


	// TODO -> generiquer ceci !!
	if ( m_dataModel.isMainstreamType("section") )
		m_renderer["section"] = new SectionRenderer(this) ;
	m_renderer["turn"] = new TurnRenderer(this) ;
	m_renderer["segment"] = new SegmentRenderer(this) ;
	m_renderer["unit"] = new UnitRenderer(this) ;
	m_renderer["qualifier_event"] = new EventRenderer(this) ;
	m_renderer["qualifier_entity"] = new EntityRenderer(this) ;
	m_renderer["qualifier_unknown"] = new UnknownQualifierRenderer(this) ;
	if ( m_dataModel.isMainstreamType("background", m_bgGraphType) )
			m_renderer["background"] = new BackgroundRenderer(this) ;
}

const string& AnnotationView::getRendererTagname(const string& id, const string& type)
{
	AnnotationRenderer* rend = getRenderer(type) ;
	if (!rend)
		return m_novalue ;
	else
		return rend->getTagName(id, type) ;
}

AnnotationRenderer* AnnotationView::getRenderer(const string& type)
{
	std::map<std::string, AnnotationRenderer*>::iterator it = m_renderer.end();

	if ( m_dataModel.isQualifierType(type) )
	{
		string renderer_type = "qualifier" ;
		string qclass = m_dataModel.getQualifierClass(type) ;
		if ( qclass.empty() )
			qclass = "unknown" ;
		renderer_type = renderer_type + "_" + qclass ;
		 it = m_renderer.find(renderer_type) ;
	}
	else if ( type.empty() )
		it = m_renderer.find("qualifier_unknown") ;
	else if  ( type.find( "unit" ) != string::npos )
		it = m_renderer.find("unit") ;
	else if ( type.compare(0, 9, "qualifier") == 0 )
	{
		guint pos = type.find("_end");
		if ( pos != string::npos )
			it = m_renderer.find(type.substr(0, pos));
		else
			it = m_renderer.find(type);
	}
	else
		 it = m_renderer.find(type) ;

	if (it==m_renderer.end())
		return NULL ;
	else
		return it->second ;
}

AnnotationMenu* AnnotationView::getRendererMenu(const string& type, const string& hint, bool edit_mode)
{
	AnnotationMenu* menu=NULL;
	AnnotationRenderer* rend = getRenderer(type) ;
	if (rend)
	{
		menu = rend->getContextualMenu(hint, edit_mode) ;
		if (menu)
			menu->setHint(hint);
	}
	return menu;
}

/* SPELL */
/*
bool AnnotationView::checkSpellerUsage(const std::string& lang_iso639_2)
{
	std::string skipped = m_configuration["Speller,ignored"] ;

	if ( skipped.empty() )
		return true ;

	std::vector<std::string> codes ;
	StringOps(skipped).split(codes, ";") ;

	std::vector<std::string>::iterator it = find(codes.begin(), codes.end(), lang_iso639_2) ;
	if (it!=codes.end())
	{
		Log::out() << "Speller: " << lang_iso639_2 << " ignored" << std::endl ;
		return false ;
	}
	else
		return true ;
}
*/

/* SPELL */
/*
const char* AnnotationView::determineAspellLanguageCode(const std::string& lang_iso639_2)
{
	std::vector<std::string> codes ;
	std::vector<std::string>::iterator it ;
	StringOps(lang_iso639_2).split(codes, "/") ;
	for (it=codes.begin(); it!=codes.end() ; it++)
	{
		const char* new_lang_iso639_1 = ISO639::get2LetterCode((*it).c_str()) ;
		if ( new_lang_iso639_1 && strcmp(new_lang_iso639_1,"") !=0 )
			return new_lang_iso639_1 ;
	}

	return "" ;
}
*/

/*
 * Configure speller
 * Method called by configure method or preferences reset only
 */
/* SPELL */
/*
void AnnotationView::configureSpeller(string master_dic, string lang_iso639_2)
{
	// -- Check availability
	if ( !checkSpellerUsage(lang_iso639_2) )
		return ;

	// -- Setting Dictionary Dico Option --
	gtkspell_set_config_option("dict", master_dic.c_str() );

	// -- Trace of antic existence ? remove it
	if ( m_speller != NULL )
	{
		gtkspell_detach(m_speller);
		m_speller = NULL;
		getBuffer()->setSpeller(NULL);
	}

	StringOps lan(lang_iso639_2) ;
	std::vector<string> cut ;
	lan.split(cut, "/", true) ;

	if (cut.size()==2 && m_viewTrack > -1 && m_viewTrack <= cut.size() )
		m_lang = cut[m_viewTrack] ;
	else
		m_lang = lang_iso639_2 ;

	GError *spell_error = NULL;
//	m_lang = lang_iso639_2;

	// -- Check for packed dictionary (except for disable)
	if ( ! master_dic.empty() && master_dic != "nospell" )
	{
		string master_dic_name = m_lang + ".multi" ;
		master_dic = Glib::build_filename(master_dic,master_dic_name) ;
		if ( ! FileInfo(master_dic).exists() )
		{
			Log::out() << "Warning : spell dictionary not found : " << master_dic << endl ;
			master_dic="";
		}
		else
			Log::out() << "Spellerer found -> " << master_dic << endl ;
	}

	// -- No packed dictionary: call default ones
	if ( master_dic != "nospell" )
	{
		// attach spell checker
		Glib::Timer timspell;

//		const char* lang_iso639_1 = determineAspellLanguageCode(lang_iso639_2) ;
		const char* lang_iso639_1 = determineAspellLanguageCode(m_lang) ;
		if ( strcmp(lang_iso639_1, "")!=0 )
		{
			Log::out() << "Attaching speller  for " << lang_iso639_1  << " dict = " << master_dic << endl;
			m_speller = gtkspell_new_attach((GtkTextView*)(Gtk::TextView::gobj()),
						(master_dic.empty() ? NULL : master_dic.c_str()),
						lang_iso639_1, &spell_error);
		}

		getBuffer()->setSpeller(m_speller);

		if ( m_speller != NULL )
		{
			// configure default behaviour of speller
			gtkspell_allow_user_dictionnary(m_speller, (m_configuration["Speller,allow_user_dic"] == "true"));
			gtkspell_allow_ignore_word(m_speller, (m_configuration["Speller,allow_ignore_word"] == "true"));
			const string& elision_chars = m_configuration["Speller,elision_chars,"+lang_iso639_2];

			//> set elision char
			if ( !elision_chars.empty() )
				gtkspell_set_elision_chars(m_speller, elision_chars.c_str());

			//> set preprocessing callback
			if (m_lang=="ara" )
			{
				InputLanguageArabic* il = (InputLanguageArabic*)InputLanguageHandler::get_input_language_by_shortcut("ara") ;
				gtkspell_set_preproc_callback(m_speller, &ptr_callback, (void*)il) ;
			}
			else
				gtkspell_set_preproc_callback(m_speller, NULL, NULL) ;

			TRACE << " ... done in " << timspell.elapsed() << " secs." << endl;
		}
		else
		{
			MSGOUT << "SPELLER ERROR = " << endl << spell_error << endl;
			//just display message in merged view
			//for avoiding to display 3 times message in stereo case
			if (m_viewTrack==-1) {
				string msg = _("Speller error : ") ;
				if (spell_error)
					msg += spell_error->message;

				#ifdef __APPLE__
				dlg::warning(msg, m_parent.getTopWindow());
				#else
				dlg::warning(msg);
				#endif
			}
		}
		// comment following line coz this sometimes causes double-free corruption (stack address used internally ?)
		//		if ( spell_error != NULL ) g_free(spell_error);
	}

}
*/

/*
 *  Configure buffer
 *  Called by Configure method only
 */
void AnnotationView::setBuffer()
{
	getBuffer()->configure(this);
}

/*
 *  Configure view display as fonts and police
 *  Called by Configure method only
 */

void AnnotationView::configureDisplay(const string& editmode, bool is_stereo)
{
	// configure police
	string key = "Fonts-editor,text" ;
	string font = getColorsCfgOption(key) ;
	setFontStyle(font, "text") ;

	tooltip_enabled = (m_configuration["AnnotationLayout,qualifiers,tooltip"] == "true");

	if ( is_stereo ) {
		string key = editmode;
		key += ",stereo,hide_nospeech";
		if (  m_configuration.find(key) == m_configuration.end() )
			TRACE << " ==== NO " << key << " KEY " << endl;
		setSqueezeNoSpeech(m_configuration[key] == "true");
	}
	else
		setSqueezeNoSpeech(false);

	const int lightCoeff = 5000;
	string fg, bg;

	setAutosetLanguage((m_configuration[editmode
	                                          + ",autoset_language"] == "true"));
	enableUndoRedoAction(editmode == "EditMode");

	// get colors to change background light in function of edition mode
	fg = getColorsCfgOption("Colors-editor,text_fg");
	bg = getColorsCfgOption("Colors-editor,text_bg");

	if (editmode != "EditMode")
	{
		// darker background
		ColorsCfg::color_from_str_change_light(bg, -lightCoeff);
		setTextColor(fg, bg);

		// put a circle cursor for text widget to warn user edition mode is off
		setTextCursor(Gdk::Cursor(Gdk::TOP_LEFT_ARROW), true);
		setEditable(false);

		// hide cursor
		set_cursor_visible(false);
	}
	else
	{
		// brighter background
		ColorsCfg::color_from_str_change_light(bg, +lightCoeff);
		setTextColor(fg, bg);
		setTextCursor(Gdk::Cursor(Gdk::XTERM), true);
		setEditable(true);
		set_cursor_visible(true);
	}

}

/**
 * configure annotation view for current main transcription language and annotation conventions
 * @param master_dic path of directory holding speller dictionnaries / "" to use system dictionnaries
 * @param lang_iso639_2 iso639-2 code (B) for main transcription language
 *
 * @note attaches word speller if available and configure available annotation menus and shortcuts for conventions
 */
void AnnotationView::configure(string master_dic, string lang_iso639_2, const string& editmode, bool is_stereo)
{
	setUndoableActions(false) ;

	// configure with actual conventions
	// TODO here properly configure graph type
	m_mainGraphType = "transcription_graph";
	m_altGraphType = "transcription_graph";
	m_bgGraphType = "background_graph";
	m_mainBaseType = m_dataModel.mainstreamBaseType(m_mainGraphType);

	//> Prepare buffer
	setBuffer() ;

	//> Prepare speller
/* SPELL */
//	configureSpeller(master_dic, lang_iso639_2) ;

	//> Prepare internal values
	m_lastEditTime = time(0);
	m_placeCursorAtEnd = ! (m_configuration["synchro_text_place_cursor"] == "begin") ; // defaults to end

	//> Prepare display value
	createAnnotationRenderers() ;

	configureDisplay(editmode, is_stereo) ;

	getBuffer()->configureMainTags() ;

	setUndoableActions(true) ;
}

/*
 *  set annotation buffer
 */
void AnnotationView::setBuffer(Glib::RefPtr<AnnotationBuffer>& buffer)
{
	Glib::RefPtr<Gtk::TextBuffer> sbuffer = Glib::RefPtr<Gtk::TextBuffer>::cast_dynamic(buffer);
	set_buffer(sbuffer);
}


/*
 *  get annotation buffer
 */
Glib::RefPtr<AnnotationBuffer> AnnotationView::getBuffer()
{
	return Glib::RefPtr<AnnotationBuffer>::cast_dynamic(get_buffer());
}


void AnnotationView::setWaitCursor(bool b)
{
	if ( get_window(Gtk::TEXT_WINDOW_TEXT) )
		get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(b ? m_waitCursor:m_textCursor);
}


/*
 * createAnnotationActionGroup : add edit actions to current action group
 */
void AnnotationView::createAnnotationActionGroup()
{

}


/*
 *  get exported action groups
 */
const Glib::RefPtr<Gtk::ActionGroup>& AnnotationView::getActionGroup(string groupname)
{
	if (groupname == "edit")
		return m_editActionGroup;
	return emptyGroup;
}

/*
 * highlight current item
 */
void AnnotationView::setHighlight(const string& type, const string& id, bool with_scroll, int track)
{
	if (track!=m_viewTrack && getParent().getActiveViewMode()!=-1 && getParent().isStereo())
		return ;

	const Gtk::TextIter& pos = getBuffer()->setHighlight(id, track);
	if ( with_scroll ) {
		scrollView(pos.get_offset());
	}
}

void AnnotationView::setTag(const string& tagname, const string& type, const string& id, bool with_scroll, int track, Gtk::TextIter& applied_start , Gtk::TextIter& applied_stop)
{
	if (track!=m_viewTrack && getParent().getActiveViewMode()!=-1 && getParent().isStereo())
		return ;

	Anchor* anchor = getBuffer()->anchors().getAnchor(id);

	if ( anchor == NULL )
	{
		getBuffer()->clearTag(tagname);
		if ( id != "" )
			MSGOUT << "MARK " << id << " NOT FOUND IN BUFFER " << endl;
	}
	else
	{
		Gtk::TextIter pos = anchor->getIter();
		if ( with_scroll ) {
			Gtk::TextIter scrollpos = pos;
			scrollpos.forward_to_line_end();
			Glib::signal_idle().connect(sigc::bind<guint32>(sigc::mem_fun(*this, &AnnotationView::scrollView), scrollpos.get_offset()));
		}
		getBuffer()->setTag(tagname, type, pos, track, applied_start, applied_stop);
	}
}

void AnnotationView::setCursor(const std::string& id, bool force, bool inhib_sync)
{
	if ( id != "" ) {
		if ( inhib_sync )
			inhibateSynchro();
		getBuffer()->setCursorAtAnchor(id, m_placeCursorAtEnd, force);
		if ( inhib_sync )
			inhibateSynchro(false);
	}
}


/*
  void hideTags(bool doit)
  {
  Log::err() << " IN HIDE TAGS" << endl;

  g_object_set(G_OBJECT(_tags["speaker"]), "invisible",
  (doit ? TRUE : FALSE), NULL);
  }
 */

/*
 * handle annotation acions
 */
void AnnotationView::onAnnotateAction(string action, int notrack, float cursor, float end_cursor, bool on_selection, bool ignoreTime, string hint)
{
	m_overlapState = false ;

	if ( ! m_editable )
	{
		get_display()->beep();
		return;
	}
	TRACE << "AnnotationView::onAnnotateAction " << action << " iter=" << getBuffer()->getCursor() << " timeStart=" << cursor << " - timeEnd=" << end_cursor << endl;

	if ( m_viewTrack != -1 && m_viewTrack != notrack )
	{
		// current view is not concerned by this event
		//	TRACE << " : NOT FOR THIS VIEW " << m_viewTrack << endl;
		return;
	}
	storePendingTextEdits();
	string type = m_dataModel.normalizeSubmain(action.substr(4), m_mainGraphType);
	if ( type.empty() )
		type = action.substr(4);

	//> -- Background are set with signal, so for that type of annotation
	//  we don't have to block when iter position isn't editable
	bool is_background = m_dataModel.isMainstreamType(type, m_bgGraphType);
	const Gtk::TextIter& curpos = getBuffer()->getCursor();
	bool need_split;

	//> -- If non editable, block action (except for background and timestamp)
	getBuffer()->isEditablePosition(curpos, need_split, true);
//	if (!getBuffer()->isEditablePosition(curpos, need_split, true)
//			&& !is_background
//			&& (action.find("_timestamp") == string::npos) )
//	{
//		get_display()->beep();
//		return;
//	}

	string diag("");

	int order = 0;
	int text_offset = 0;
	bool at_end = false;
	float start_curs = cursor ;

	string action_type;
	if ( is_background	)
		action_type = m_dataModel.mainstreamBaseType(m_bgGraphType) ;
	else
		action_type = m_mainBaseType ;

	//> -- Get base text type id closest to insert position ( may be signal-anchored or not)
	string prevId = getBuffer()->getPreviousAnchorId(curpos, action_type);

	//> -- Check that it corresponds to current track, else lookup backward in buffer to find previd for current track
	if ( m_dataModel.getNbTracks() > 1 )
	{
		while ( ! (prevId.empty() || m_dataModel.getElementSignalTrack(prevId) == notrack) )
			prevId = getBuffer()->getPreviousAnchorId(prevId, action_type);
	}

	string check_type = type;

	//> -- Adjust specific value for selection mode
	if (!on_selection)
	{
		bool is_mainstream = m_dataModel.isMainstreamType(type, m_mainGraphType);
		if ( is_mainstream
				&& type != m_mainBaseType
				&& type != m_dataModel.segmentationBaseType(m_mainGraphType) )
		{
			start_curs = -1.0;
			end_cursor = 0;
		}
	}
	else if (!is_background)
	{
		check_type = m_mainBaseType ;
	}

	// get text offset in current segment
	if ( ! prevId.empty() )
	{
		Gtk::TextIter tmp = getBuffer()->getCursor();
		//while ( tmp.editable() && tmp.get_char() == ' ') tmp.backward_char();
		text_offset = getBuffer()->getOffsetInTextSegment(prevId, tmp, true );
		if ( text_offset < 0
					&& check_type == m_mainBaseType
					&& (tmp.compare(getBuffer()->getAnchoredIter(prevId)) == 0) )
		{
			// insertion point is just before tagged element attached to prevId
			// we should then consider that insertion is at previous segment end
			// -> set prevId to previous anchor
			prevId = getBuffer()->getPreviousAnchorId(prevId, m_mainBaseType);
			text_offset = getBuffer()->getOffsetInTextSegment(prevId, tmp, true );
		}

		// TODO PLR ici -> verifier ce que cela fait
		Anchor* nextAnchor = getBuffer()->anchors().getEndAnchor(prevId);
		at_end = ( nextAnchor == NULL || nextAnchor->getType() != m_mainBaseType );
		if ( at_end )
		{
			const string& txt =  getBuffer()->getSegmentText(prevId, "", false, true);
			at_end =(text_offset >= g_utf8_strlen(txt.c_str(), -1));
		}
	}

	string alignCandidate = "" ;
	if ( type==m_dataModel.segmentationBaseType() )
	{
		Gtk::TextIter iter = getBuffer()->getCursor() ;
		Anchor* an = getBuffer()->anchors().getAnchorAtPos(iter, m_mainBaseType, true) ;
		while ( !an && iter.editable() == false ) {
			if ( ! iter.backward_char() ) break;
			an = getBuffer()->anchors().getAnchorAtPos(iter, m_mainBaseType, true) ;
		}
		if (an)
			alignCandidate = an->getId() ;

	}

	//> -- Check if action is available
	// 	For background will be done later
	// 	If time ignoring, don't check it
	if ( action.find("_background") == string::npos
			&& action.find("_timestamp") == string::npos
			&& !ignoreTime )
	{
		if ( m_dataModel.checkInsertionRules(check_type, notrack, start_curs, end_cursor, prevId, order, alignCandidate, diag) == false )
		{
			#ifdef __APPLE__
			dlg::warning(diag, m_parent.getTopWindow());
			#else
			dlg::warning(diag);
			#endif
			return;
		}
		else if ( !diag.empty() )
		{
			// ask for user confirmation
			if (!dlg::confirm(diag, m_parent.getTopWindow()))
				return;
		}
	}

	string id = "";
	try
	{
		//TRACE << "################################ New " << type << " prevId=" << prevId << " order =" << m_dataModel.getOrder(prevId) << endl;
		//> -- Section creation
		if ( action == "new_section" )
		{
			// action will be performed after user action on popup
			popupAnnotationMenu(type, getBuffer()->getCursor(), NULL, -1, -1, hint);
		}
		//> -- Turn creation
		else if (action == "new_turn")
		{
			if (order>0)
				m_overlapState = true ;

			// special case for no speech: no need to use to select anything
			if (hint==Speaker::NO_SPEECH)
			{
				bool fromSignalSelection = (start_curs>=0.0 && end_cursor>=0.0 && start_curs!=end_cursor) ;
				setSignalSelectionAction(fromSignalSelection) ;
				createSpeakerElement(getBuffer()->getCursor(), hint, false, start_curs, end_cursor) ;
			}
			// action will be performed after user action on popup
			else
				popupAnnotationMenu(type, getBuffer()->getCursor(), NULL, start_curs, end_cursor, hint);
		}
		//> -- Segment creation
		else if (action == "new_segment")
		{
			getBuffer()->begin_user_action() ;

			Glib::ustring txt = getBuffer()->getSegmentText(prevId, "", false, true);
			if ( end_cursor > 0.0 )
				Log::trace() << "--) segment in selection: forbidden, abort." << std::endl ;
			else
			{
				// don't allow to split a qualifier without splitting text
				if ( ! getBuffer()->canSplitAtIter(getBuffer()->getCursor()) )
				{
					gdk_beep() ;
					return ;
				}

				// When creating segment at a unit start, we won't need to split in some case
				bool needSplit = true ;
				bool textSegmentStartCase = (text_offset == 0 && m_dataModel.mainstreamBaseElementHasText(prevId, m_mainGraphType)) ;

				if ( textSegmentStartCase /*&& !m_dataModel.hasQualifiers(prevId)*/ )
					needSplit = false ;

				// forward qualifier attachments unless we're at segment end
				bool forward_qual_attachments = false;
				if ( needSplit && (!curpos.editable() || curpos==getBuffer()->end() || curpos.get_char()=='\n' ) )
				{
					const string& aid = getBuffer()->getNextAnchorId(curpos, "");
					if ( ! aid.empty() )
						forward_qual_attachments = ( m_dataModel.getElementType(aid) == m_mainBaseType );
				}
				else
					forward_qual_attachments = true;

			//				Log::out() << "~ ~ Inserting new Segment - needsplit = " << needSplit << std::endl ;
				id = m_dataModel.insertMainstreamElement(type, prevId, cursor, needSplit, true, forward_qual_attachments );

				// -- SPlit text (if no split need, at least will erase value of prevId)
				m_dataModel.splitTextContent(prevId, text_offset, true);
			}
			getBuffer()->end_user_action() ;
		}
		//> -- Background creation
		else if (action == "new_background")
		{
			getBuffer()->begin_user_action() ;
			string error;
			string inactive_to_update ;
			// check if we can insert a new bg at current position
			bool ok = m_dataModel.checkBackgroundInsertionRules(notrack, cursor, end_cursor, inactive_to_update, error);
			if (!ok)
				dlg::error(error.c_str(), (Gtk::Window*) this);
			else
			{
				bool new_item = inactive_to_update.empty() ;
				#ifdef __APPLE__
				Gtk::Window* top = m_parent.getTopWindow();
				#else
				Gtk::Window* top = NULL; // m_parent.getTopWindow();
				#endif
				BackgroundDialog* dlg = new BackgroundDialog(top, m_dataModel, inactive_to_update, true, new_item, notrack, cursor, end_cursor);
				dlg->run() ;
				dlg->hide();
				delete dlg;
			}
			getBuffer()->end_user_action() ;
		}
		else if (action == "new_timestamp")
		{
			annotateTimestamp(prevId, notrack, text_offset, cursor, end_cursor, on_selection, ignoreTime) ;
		}
		//> -- Background Edition
		else if (action == "edit_background")
		{
			getBuffer()->begin_user_action() ;

			vector<string> res ;

			m_dataModel.getSegmentsInRange( m_dataModel.getAG(m_bgGraphType), m_dataModel.mainstreamBaseType(m_bgGraphType), notrack, cursor, end_cursor, res, 3);

			int size = res.size() ;
			if (size>1)
				#ifdef __APPLE__
				dlg::warning(_("You can't edit several backgrounds at same time"), m_parent.getTopWindow()) ;
				#else
				dlg::warning(_("You can't edit several backgrounds at same time")) ;
				#endif
			else if (size==0)
				#ifdef __APPLE__
				dlg::msg(_("No background for displaying properties"), m_parent.getTopWindow()) ;
				#else
				dlg::msg(_("No background for displaying properties")) ;
				#endif
			else {
				string qid = res[0] ;
				editBackground(qid);
			}
			getBuffer()->end_user_action() ;
		}
		//> -- Background suppression
		else if (action == "delete_background")
		{
			getBuffer()->begin_user_action() ;
			vector<string> res ;

			m_dataModel.getSegmentsInRange(m_dataModel.getAG(m_bgGraphType), m_dataModel.mainstreamBaseType(m_bgGraphType), notrack, cursor, end_cursor, res, 3);

			int size = res.size() ;
			if (size>1)
				#ifdef __APPLE__
				dlg::warning(_("You can't delete several backgrounds at same time"), m_parent.getTopWindow()) ;
				#else
				dlg::warning(_("You can't delete several backgrounds at same time")) ;
				#endif
			else if (size==0)
				#ifdef __APPLE__
				dlg::msg(_("No background to delete"), m_parent.getTopWindow()) ;
				#else
				dlg::msg(_("No background to delete")) ;
				#endif
			else
			{
				string qid = res[0] ;
				deleteBackground(qid, true) ;
			}
			getBuffer()->end_user_action() ;
		}
		//> -- Event creation
		else if (action == "new_event")
		{
			// action will be proceded after user action on popup
			popupAnnotationMenu(type, getBuffer()->getCursor(), NULL, -1, -1, hint);
		}
		//> -- Entity creation
		else if (action == "new_qualifier_entity")
		{
			if ( getBuffer()->canInsertQualifier(getBuffer()->getCursor()) )
				popupAnnotationMenu(type, getBuffer()->getCursor(), NULL, -1, -1, hint) ;
			else
			{
				string msg = _("No selected text") ;
				#ifdef __APPLE__
				dlg::msg(msg, m_parent.getTopWindow());
				#else
				dlg::msg(msg);
				#endif
			}
		}
	}
	catch (const char* msg)
	{
		#ifdef __APPLE__
		dlg::warning(msg, m_parent.getTopWindow());
		#else
		dlg::warning(msg);
		#endif
	}
}

void AnnotationView::annotateTimestamp(const string& prevId, int notrack, int text_offset, float cursor, float end_cursor, bool on_selection, bool ignoreTime)
{
	const string& currentId = getBuffer()->getTaggedElementId( getBuffer()->getCursor() ) ;

	//> -- Tagged element, update timestamp
	if ( ! currentId.empty() )
		changeTimestamp(currentId, cursor, false) ;
	else
	{
		Anchor* an = getBuffer()->anchors().getAnchorAtPos(getBuffer()->getCursor(), "", false) ;
		if (!an)
			createTimestamp(prevId, notrack, text_offset, cursor, end_cursor, on_selection, ignoreTime) ;
		else
		{
			string id = an->getId() ;
			changeTimestamp(id, cursor, true) ;
		}
	}
}

void AnnotationView::changeTimestamp(const string& currentId, float cursor, bool newTag)
{
	getBuffer()->begin_user_action() ;
	bool done = false ;

	// Unset time stamp
	if ( m_dataModel.isAnchoredElement(currentId, 0) )
		done = m_dataModel.unsetElementOffset(currentId, true, true) ;
	// Set timestamp
	else
	{
		string error_s ;
		if ( ! m_dataModel.checkTimestampRules(cursor, -1, currentId, error_s) )
		{
			#ifdef __APPLE__
			dlg::warning(error_s, m_parent.getTopWindow());
			#else
			dlg::warning(error_s);
			#endif
			return;
		}

		if ( cursor >= 0.0 )
			done = m_dataModel.setElementOffset(currentId, cursor, true, true) ;
	}

	// When adding offset to a non-tag element, we need to add element tag too
	if (newTag && done)
	{
		const string& type = m_dataModel.getElementType(currentId) ;
		m_renderer[type]->updateElement(currentId, getR2LMode(currentId)) ;
	}

	if (!done)
	{
		string msg = _("Impossible action") ;
		dlg::msg(msg, NULL) ;
	}

	getBuffer()->end_user_action() ;
}

void AnnotationView::createTimestamp(const string& prevId, int notrack, int text_offset, float cursor, float end_cursor, bool on_selection, bool ignoreTime)
{
	string diag ;
	const string& baseType = m_mainBaseType ;

 	const string& checkType = getDataModel().mainstreamBaseType() ;
	int order = 0 ;
 	if (  ! m_dataModel.checkInsertionRules(checkType, notrack, cursor, end_cursor, prevId, order, "", diag) )
	{
		#ifdef __APPLE__
		dlg::warning(diag, m_parent.getTopWindow());
		#else
		dlg::warning(diag);
		#endif
		return;
	}

	getBuffer()->begin_user_action() ;
//	m_dataModel.insertMainstreamElement(baseType, prevId, cursor, true, true);
//	m_dataModel.splitTextContent(prevId, text_offset, true);
	m_dataModel.splitTextMainstreamElement(prevId, text_offset, cursor, true);
	getBuffer()->end_user_action() ;
}

void AnnotationView::removeTimestamp(const Gtk::TextIter& iter, string type)
{
	const string& currentId = getBuffer()->getTaggedElementId( iter ) ;

	//> -- Tagged element, update timestamp
	if ( ! currentId.empty() )
		changeTimestamp(currentId, iter, false) ;
}

void AnnotationView::deleteBackground(const string& qid, bool confirm)
{
	if (!m_dataModel.hasAnnotationGraph(m_bgGraphType))
		return ;

	if (qid.empty())
		return ;

	if (!m_dataModel.isActiveBackground(qid))
		#ifdef __APPLE__
		dlg::msg(_("No background to delete"), m_parent.getTopWindow()) ;
		#else
		dlg::msg(_("No background to delete")) ;
		#endif
	else {
		bool rep ;
		if (confirm)
			#ifdef __APPLE__
			rep = dlg::confirm(_("Delete background segment ? "), m_parent.getTopWindow());
			#else
			rep = dlg::confirm(_("Delete background segment ? ")) ;
			#endif
		else
			rep = true ;
		if (rep)
		{
			string err ;
			bool success = m_dataModel.deleteElement(qid, true, err, false);
			if (!success)
				dlg::error(err,m_parent.getTopWindow() ) ;
		}
	}
}

void AnnotationView::editBackground(const string& qid)
{
	if (!m_dataModel.isActiveBackground(qid))
		#ifdef __APPLE__
		dlg::msg(_("No background for displaying properties"), m_parent.getTopWindow()) ;
		#else
		dlg::msg(_("No background for displaying properties")) ;
		#endif
	else if ( !qid.empty() ) {
		#ifdef __APPLE__
		Gtk::Window* top =  NULL; // m_parent.getTopWindow();
		#else
		Gtk::Window* top =	m_parent.getTopWindow();
		#endif

		BackgroundDialog *dlg = new BackgroundDialog(top, getDataModel(), qid, m_editable, false);
		if ( dlg->run() ==  Gtk::RESPONSE_OK )
			TRACE << " Background updated : " << qid << endl;
		dlg->hide();
		delete dlg;
	}
}


/*========================================================================
 *
 *  update view for selected annotation items type & id
 *   default : update all
 *
 ========================================================================*/

void AnnotationView::updateView(const string& type, string id, DataModel::UpdateType upd, bool fromSignal, bool ignoreUpdTrack)
{
	bool TRACE_ON = true ;

	//	if ( ! is_visible() ) return;
	Glib::Timer tim;
	tim.start() ;

	bool updtrack = true;
	bool text_only = false;
	string upd_id = id;
	string upd_type = type;
	string upd_other_id = "" ;

	// 0=delete, 1=update, 2=insert 8=freeze_update 9=commit_update
	int upd_action = 1;
	int upd_other_action = 1 ;
	int cur_offset = -1;
	int notrack=-1;
	bool do_restore_cursor = true;
	bool globalUpdate = (type == "" && id == "") ;

	if (getParent().isStereo())
	{
		if ((m_parent.getActiveViewMode()==VIEW_MODE_MERGED && m_viewTrack!=-1)
					|| (m_parent.getActiveViewMode()!=VIEW_MODE_MERGED && m_viewTrack ==-1) )
			return ;
	}

	if ( !globalUpdate  &&  upd != DataModel::DELETED )
	{
		notrack = m_dataModel.getElementSignalTrack(id);
		if ( getViewTrack() != -1 && notrack != getViewTrack() )
			return;  // current view not concerned by this update !
	}

	inhibateStoreEdits();
	inhibateSynchro(true);


	// GLOBAL UPDATE
	if ( globalUpdate )
	{
/* SPELL */
//		inhibateSpellChecking(true);
		setUndoableActions(false);

		// TODO -> future versions : generalize decoration modes, not only for confidence.
		if ( m_withConfidence )
		{
			bool confidence = m_dataModel.hasElementsWithProperty(m_dataModel.getAGTrans(), "score", m_mainBaseType) ;
			if ( ! confidence )
				confidence = m_dataModel.hasElementsWithProperty(m_dataModel.getAGTrans(), "confidence", m_mainBaseType);

			setWithConfidence(confidence) ;
		}

		bool was_modified = getBuffer()->get_modified();

		// global refresh
		TRACE << " -------------------------------------((o))  START OF WHOLE DISPLAY  ++ " << endl << endl;

		upd_action = 2;

		cur_offset = (getBuffer()->get_char_count() > 0 ? getBuffer()->getCursor().get_offset() : -1);

		//	TRACE_D << " current cursor pos is " << getBuffer()->getCursor() << " offset= " << cur_offset << endl;
		getBuffer()->clearBuffer();

 		tim.reset() ;

 		bool old_disable_thread  ;
 		if (fromSignal)
 		{
 			old_disable_thread = disable_thread ;
 			disable_thread = true ;
 		}
 		
 		//> -- TRANSCRIPTION
		if ( m_dataModel.hasAnnotationGraph(m_mainGraphType) )
		{
			vector<string> mainstream_types = m_dataModel.getMainstreamTypes(m_mainGraphType);
			vector<string>::iterator it_types;
			if ( !mainstream_types.empty() )
				renderAll(m_mainGraphType , mainstream_types, 0, "", getViewTrack(), getR2LMode(""));
		}

 		//> -- BACKGROUND
		if ( m_dataModel.hasAnnotationGraph(m_bgGraphType) )
		{
			vector<string> mainstream_types = m_dataModel.getMainstreamTypes(m_bgGraphType);
			vector<string>::iterator it_types;
			if ( !mainstream_types.empty() )
				renderAll(m_bgGraphType , mainstream_types, 0, "", getViewTrack(), getR2LMode(""));
		}

 		if (fromSignal)
 			disable_thread = old_disable_thread ;
 			
 		m_pendingTextEdits = 0;
 		getBuffer()->set_modified(was_modified);

/* SPELL */
//		inhibateSpellChecking(false, true);
		// <!> Massive updateView reset display by checking DataModel
		//	   and re-inserting all elements. Therefore offsets and marks
		//     can be modified what could provoke undo/redo stack corruption.
		//     => clear for avoiding problems
		clearUndoHistory() ;
		setUndoableActions(true);
	}
	// LOCAL UPDATE
	else if ( id != "" )
	{
/* SPELL */
//		inhibateSpellChecking(true, false) ;

		SignalSegment s;
		if (TRACE_ON)
		{
			TRACE << "\n~~~ In updateView track="<< getViewTrack() <<" type= " << type << "  id = " << id 	<< " upd =" ;
			switch ( upd )
			{
				case DataModel::SPLITTED : TRACE << "SPLITTED" << endl; break;
				case DataModel::INSERTED : TRACE << "INSERTED" << endl; break;
				case DataModel::UPDATED : TRACE << "UPDATED" << endl; break;
				case DataModel::RESIZED : TRACE << "RESIZED" << endl; break;
				case DataModel::MOVED : TRACE << "MOVED" << endl; break;
				case DataModel::DELETED : TRACE << "DELETED" << endl; break;
				case DataModel::ANCHORED : TRACE << "ANCHORED" << endl; break;
				case DataModel::UNANCHORED : TRACE << "UNANCHORED" << endl; break;
			}
		}
		getBuffer()->begin_user_action();
		getBuffer()->backupInsertPosition(0, (upd==DataModel::DELETED));

		switch ( upd )
		{
			case DataModel::DELETED :
			{
				// TODO -> GERER LES RENDERER DE SEGMENTS !!!!
				if ( m_dataModel.isMainstreamType(type, "") )
				{
					m_renderer[type]->deleteElement(id);
					upd_action=0;
				}
				else if (m_dataModel.isQualifierType(type, m_mainGraphType))
				{
					// find previous mark to update corresp. seg label
					const Gtk::TextIter& iter = getBuffer()->getTaggedElementIter(id);
					updtrack = (iter.compare(getBuffer()->end()) != 0);
					if ( updtrack )
					{
//						upd_type = m_dataModel.segmentationBaseType();
						upd_type = m_mainBaseType ;
						upd_id = getBuffer()->getPreviousAnchorId(iter, upd_type, true);
						notrack = m_dataModel.getElementSignalTrack(upd_id);
					}
					string desc = m_dataModel.getElementProperty(id, "desc") ;
					const string& renderer = getQualifierRenderer(type, desc, "transcription_grapinhibateh");
					m_renderer[renderer]->deleteElement(id);
				}
				break;
			}

			case DataModel::UPDATED:  // update display
			{
				//> LOWEST ANNOTATION TYPE
				// TODO CHECK CORRECT BEHAVIOUR FOR MAINSTREAM BASE UPDATE
				if ( type == m_mainBaseType )
				{
//					if ( !m_dataModel.mainstreamBaseElementHasText(id, m_mainGraphType) )
					m_renderer[type]->updateElement(id, getR2LMode(id)) ;
					upd_other_id = m_dataModel.getParentElement(upd_id) ;
					upd_other_action = upd_action ;
				}
				else if ( type == m_dataModel.segmentationBaseType(m_mainGraphType) )
				{
					// the only case in which this case occurs is when segment order
					// is changed. (overlapping speech) or segment text set to ""
					int order = m_dataModel.getOrder(id);
					if ( order > 0 )
						getBuffer()->setTrack(type, id, order, true, getR2LMode(id));
					else
					{
						// Otherwise set back the "classic" track, i.e trackO
						// (track1 for 2nd track in stereo merged view)
						int notrack =  m_dataModel.getElementSignalTrack(id) ;
						if (isStereo() && m_viewTrack != -1 )
							notrack = 0 ;
						getBuffer()->setTrack(type, id, notrack, true, getR2LMode(id));
					}

					//> update background
					insertBackgroundSegment(id, 2, true) ;
				}
				//> OTHERS TRANSCRIPTION MAINSTREAM TYPES
				else if ( m_dataModel.isMainstreamType(type, m_mainGraphType) )
				{
					m_renderer[type]->updateElement(id, getR2LMode(id));
				}
				//> BACKGROUNDS
				else if (type == "background")
				{
					updateBackgroundDisplay(id, 2, true) ;
				}
				//> QUALIFIERS (Entities and Event)
				else
				{
					string desc = m_dataModel.getElementProperty(id, "desc") ;
					const string& renderer = getQualifierRenderer(type, desc, m_mainGraphType);
					m_renderer[renderer]->updateElement(id, getR2LMode(id));
				}
				break ;
			}
			case DataModel::ANCHORED:
			case DataModel::UNANCHORED:
			{
				bool isAnchor = ( m_dataModel.existsAnchor(id) && type=="anchor" ) ;
				updtrack = false ;
				if (isAnchor)
					updateAfterAnchorAction(id, notrack, upd) ;
				else
					getBuffer()->switchTimestamp(id, upd==DataModel::ANCHORED) ;
				break ;
			}
			case DataModel::MOVED:
			{
				if (type == "background")
				{
					m_renderer["background"]->deleteElement(id);
				}
				else if ( ! m_dataModel.isMainstreamType(type, m_mainGraphType) )
				{
					string desc = m_dataModel.getElementProperty(id, "desc") ;
					const string& renderer = getQualifierRenderer(type, desc, m_mainGraphType);
					m_renderer[renderer]->deleteElement(id);
				}
				else
				{
					// delete existing segment to re-insert it
					const Gtk::TextIter& pos = getBuffer()->deleteSegment(id, true);
					// move cursor to corresponding insertion position in text
					getBuffer()->setCursor(pos, false);
					upd_action=0;
				}
				break ;
			}
			case DataModel::SPLITTED :
			{
				upd_other_id = m_dataModel.getPreviousElementId(id) ;
				upd_other_action = 1 ;
			}
			case DataModel::INSERTED :
			{
				string align_id = "" ;

				s.setId("");

				//> -- Mainstream case
				if ( m_dataModel.isMainstreamType(type, m_mainGraphType) )
				{
					if ( m_dataModel.getSegment(id, s, getViewTrack(), false, false, -1) )
					{
						getBuffer()->clearHighlight("",3);

						// 	Get aligment id for current inserted element
						//   - section -> just before current turn
						//   - turn -> just before current segment / just before next turn if order > 1
						//       & insert pos = start of turn
						//   - first mainstream of a segment -> just after the segment
						//   - other types -> leave cursor where it is
						align_id = m_dataModel.getAlignmentId(id);
//						TRACE << " ALIGN FOR " << m_dataModel.print_annotation(id, true) << " = " << m_dataModel.print_annotation(align_id, true) << endl;
						if ( !align_id.empty() )
						{
							if ( ! getBuffer()->moveCursorToAnchor(align_id, false) )
								Log::err() << "updateView {INSERTED} no anchor found for id =" << id << endl;
						}
						else
						{
							//-- No alignement -> mainstream base type : check specific case for mainstream base type
							// 	 First mainstream base type of a segmentation base type must be inserted just after its parent type
							const string& parent = m_dataModel.getParentElement(id) ;
							if ( m_dataModel.isFirstChild(id, parent) ) {
								// then set cursor just after parent tag
								getBuffer()->moveCursorToAnchor(parent, false);
								const Gtk::TextIter& pos = getBuffer()->getCursor();
								getBuffer()->nextEditablePosition(pos, NULL, true);

								// ELSE -> LET CURSOR AT CURRENT POS
								// TODO -> gerer proprement le split pour réactiver ce cas général !!!

//							} else if ( upd != DataModel::SPLITTED ) {
//								// other case -> set cursor at previous element end position
//								const string& prevId = m_dataModel.getPreviousElementId(id);
//								getBuffer()->moveCursorToAnchorEnd(prevId, false);
							}

						}
						upd_action = 2;
					}
				}
				//> -- Background
				else if (type == "background")
				{
					upd_id = id;
					upd_type = type;
					upd_action = 2;
				}
				//> -- Qualifier
				else
				{
					upd_type = m_dataModel.segmentationBaseType() ;
					upd_id = m_dataModel.getParentElement(id, upd_type) ;
				}

				if ( s.getId() != "" )
				{
					//> -- Base unit element
					if ( type == m_mainBaseType )
					{
						string parent =  m_dataModel.getParentElement(id) ;
						TRACE << " renderBaseElement cursor pos = " << getBuffer()->getCursor()  << endl;
						renderBaseElement(m_mainGraphType, type, id, false, (upd == DataModel::SPLITTED), parent, getR2LMode(s.getId()));

						getBuffer()->backupInsertPosition(); // HACK HONTEUX TANT QUE SPLIT MAL GERE
						insertBackgroundSegment(id, 2, true) ;
					}
					//> -- Other mainstreams
					else if ( m_dataModel.isMainstreamType(type, m_mainGraphType) )
					{
						m_renderer[type]->render(s.getId(), getR2LMode(s.getId()));


						// Inserting a segment base type ? actualize the aligned base type renderer
						// (its tag should not be display, let's remove it)
						if ( !align_id.empty()
								&& type == m_dataModel.segmentationBaseType(m_mainGraphType)
								&& m_dataModel.isFirstChild(align_id, id)
								&& m_dataModel.mainstreamBaseElementHasText(align_id, m_mainGraphType) )
						{
							getBuffer()->deleteAnchoredLabel(align_id, true, true) ;
						}
					}
				}
				 // BACKGROUND PROCESS
				else if (type == "background")
				{
					//> display tags
					m_renderer["background"]->render(id, false);
					m_renderer["background"]->render_end(id, false);
				}
				// QUALIFIER PROCESS
				else
				{
					const string& subtype = m_dataModel.getElementProperty(id, "desc") ;
					const string& renderer = getQualifierRenderer(type, subtype, m_mainGraphType);
					bool r2l = getR2LMode(id) ;
					m_renderer[renderer]->render(id, r2l) ;
					m_renderer[renderer]->render_end(id, r2l) ;
					// if subtype is other, auto-popup for allowing modification
					if ( subtype == QualifiersMenu::OTHER_CHOICE )
					{
						Glib::signal_idle().connect(sigc::bind<Gtk::TextIter, string>(
									sigc::mem_fun(*this, &AnnotationView::editAnnotationPropertiesWhenIdle),
									getBuffer()->getTaggedElementIter(id), type));
					}
				}
				// ICI
				// else  -> (topic ??)
				break;
			} // end case inserted

			case DataModel::RESIZED :
			{
				if (type == m_mainBaseType) {
					// background
					insertBackgroundSegment(id, 2, true) ;
				}
				// background
				else if (type == "background") {
					updateBackgroundDisplay(id, 2, true) ;
				}
				break;
			}
		}
//		TRACE << "========================================  ANCHORS " << endl;
//		getBuffer()->anchors().dump();
//		TRACE << "========================================  END ANCHORS " << endl;

		getBuffer()->restoreInsertPosition(0, false);
		if ( upd = DataModel::INSERTED ) {
			const Gtk::TextIter& iter = getBuffer()->getCursor();
//			TRACE << " AFter restoreInsertPosition cursor pos = " << iter << " is editable=" << iter.editable() << endl;
		}

		cur_offset = getBuffer()->getCursor().get_offset();

		getBuffer()->set_modified(true);
		getBuffer()->end_user_action();

	} // End Specific Element update

	if ( type == "" && id == "" )
		TRACE << "-------------------------------------((o))  END OF WHOLE DISPLAY  ++ " << tim.elapsed() << endl << endl;

	if ( updtrack && !ignoreUpdTrack)
	{
		m_signalElementModified.emit(upd_type, upd_id, upd_action, text_only, m_viewTrack, true) ;
		// actualize parent if needed
		// TODO move it in editor
		if (!upd_other_id.empty())
			m_signalElementModified.emit(m_dataModel.getElementType(upd_other_id), upd_other_id, upd_other_action, text_only, m_viewTrack, true) ;
	}

	getBuffer()->clearSelection();

	inhibateStoreEdits(false);
	//	inhibateSynchro(false);
	Gtk::TextIter iter;
	if ( cur_offset >= 0 )
		iter = getBuffer()->get_iter_at_offset(cur_offset);
	else
		iter = getBuffer()->begin();

	iter = getBuffer()->nextEditablePosition(iter, "", (!type.empty()));

	getBuffer()->setCursor(iter, false);
	iter = getBuffer()->getCursor();

//	if ( upd = DataModel::INSERTED ) {
//		TRACE << " OUT updateView cursor pos = " << iter << " is editable=" << iter.editable() << endl;
//	}
	Glib::signal_idle().connect(sigc::bind<guint>(sigc::mem_fun(*this, &AnnotationView::scrollViewWhenIdle), iter.get_offset()));

	inhibateSynchro(false) ;

	return ;
}

string AnnotationView::getQualifierRenderer(const string& type, const string& subtype, const string& graphtype)
{
	string renderer = "qualifier_";

	//> check subtype validity
	if (!subtype.empty() && !m_dataModel.conventions().isValidSubtype(subtype, type, graphtype))
		renderer += "unknown";
	else
	{
		const string& qclass = m_dataModel.conventions().getQualifierClass(type, graphtype);
		if ( !qclass.empty() )
			renderer += qclass;
		else
			renderer += "unknown";
	}
	return renderer;
}

/**
 * render all annotations at a given mainstream type level for a given signal part
 * @param mainstream_types vector of mainstream types to render
 * @param itype index of current mainstream type level
 * @param parent parent signal segment
 */
void AnnotationView::renderAll(const string& graphtype, const vector<string>& mainstream_types, int itype, const string& parent, int notrack, bool do_r2l)
{
	const string& curtype = mainstream_types[itype];
	bool render_next_level = (itype+1) < mainstream_types.size() ;
	vector<string> childs;
	bool r2l = do_r2l;

	if ( itype == 0 )
	{
		string msg = std::string(_("Loading")) + " " + curtype + "s ..." ;
		getParent().signalStatusBar().emit(msg) ;
	}

	// eventually skip missing top levels
	if ( render_next_level && ! m_dataModel.hasElementsWithType(mainstream_types[itype]) ) {
		renderAll(graphtype, mainstream_types, itype+1, "", notrack, r2l);
	}

	//> -- Rendering all mainstreams except mainstream base type
	if ( curtype != m_dataModel.mainstreamBaseType(graphtype) )
	{
		m_dataModel.getChilds(childs, curtype, parent, notrack);
		vector<string>::iterator itc;

		bool do_flush = (curtype == m_dataModel.segmentationBaseType(graphtype) );
		bool check_r2l = (curtype == m_dataModel.conventions().getTypeFromFeature("speaker", graphtype));
		for ( itc = childs.begin(); itc != childs.end(); ++itc )
		{
			if ( check_r2l)
				r2l = getR2LMode(*itc) ;

			m_renderer[curtype]->render(*itc, r2l);
			if ( render_next_level )
			{
				if ( notrack == -1 && !parent.empty() )
					notrack = m_dataModel.getElementSignalTrack(parent);
				renderAll(graphtype, mainstream_types, itype+1, *itc, notrack, r2l);
			}
			m_renderer[curtype]->render_end(*itc, r2l);
			// flush
			if ( do_flush )
				GtUtil::flushGUI(false, !disable_thread) ;
		}
	}
	//> -- Rendering mainstream base type
	else
	{
		m_dataModel.getChilds(childs, curtype, parent, notrack);
		vector<string>::iterator itc;

		for ( itc = childs.begin(); itc != childs.end(); ++itc )
		{
			renderBaseElement(graphtype, curtype, *itc, true, false, parent, r2l);
		}
	}
}

/*
 * render "text" segments
 */
void AnnotationView::renderBaseElement(const string& graphtype, const string& type, const string& id,
											bool with_qualifiers, bool split_only,
											const string& parent, bool r2l)
{
	//> -- Transcription
	if (graphtype==m_mainGraphType)
	{
		//> -- Automatic rendering ? use given value
		m_renderer[type]->setFirstChild(m_dataModel.isFirstChild(id, parent));
		m_renderer[type]->render(id, r2l, !split_only);

		if ( m_dataModel.mainstreamBaseElementHasText(id, graphtype) && with_qualifiers )
		{
			renderQualifiers(id, false, true, r2l);
			renderQualifiers(id, true, false, r2l);
		}
		m_renderer[type]->render_end(id, r2l);
	}
	//> -- Background graph
	else if (graphtype==m_bgGraphType)
	{
		m_renderer[type]->render(id, r2l);
		m_renderer[type]->render_end(id, r2l);
	}
}

/*
 * insert segment qualifiers (text-attached noise /pron / ....)
 */
void AnnotationView::renderQualifiers(const string& id, bool starting_at, bool dont_set_cursor, bool r2l)
{
	//	getBuffer()->begin_user_action() ;

	vector<string> ids;
	vector<string>::iterator it;
	//	const string& to_show = m_configuration["AnnotationLayout,qualifiers,show"];
	const string to_show = "";  // provisoire
	const string& to_hide = m_configuration["AnnotationLayout,qualifiers,hide"];

	m_dataModel.getQualifiers(id, ids, "", starting_at, false);
	for ( it=ids.begin(); it != ids.end(); ++it )
	{
		const string& qtype = m_dataModel.getElementType(*it);

		//if ( to_show.empty() || to_show.find(qtype) != string::npos )
		if ( to_hide.empty() || to_hide.find(qtype) == string::npos )
		{
			const string& subtype = m_dataModel.getElementProperty(*it, "desc") ;
			const string& renderer = getQualifierRenderer(qtype, subtype, m_mainGraphType) ;

			if (!renderer.empty())
			{
				if (starting_at)
					m_renderer[renderer]->render(*it, r2l) ;
				else
					m_renderer[renderer]->render_end(*it, r2l) ;
			}
		}
	}
	//	getBuffer()->end_user_action() ;
}


/**
 * TODO A COMMENTER
 */
void AnnotationView::onUndoableAction(const string& data, int notrack)
{
	//> if model is locked, don't add custom action to undo/redo - stack
//	if (m_lockModelUndoRedo) {
//		TRACE << "******************   AV::onUndoableAction: model locked, don't propagate into stack" << std::endl ;
//		return ;
//	}

	if (notrack<=-2) {
		if (notrack==-2)
			#ifdef __APPLE__
			dlg::msg(_("Undo-Redo stack error, stack reset"), m_parent.getTopWindow()) ;
			#else
			dlg::msg(_("Undo-Redo stack error, stack reset"), NULL) ;
			#endif
		clearUndoHistory() ;
		TRACE << "AV::onUndoableAction:<!> stack error OR normal blocked behaviour  - stop. (iam=" << m_viewTrack << " - for=" << notrack << ")" << std::endl ;
		return ;
	}

	if (!isStereo() || m_viewTrack==notrack) {
//		TRACE << "AV::onUndoableAction:> custom event launched. (iam=" << m_viewTrack << " - for=" << notrack << ")" << std::endl ;
		doCustomEvent(data) ;
	}
}

void AnnotationView::updateAfterUndoRedo(const string& type, const string& id, DataModel::UpdateType upd)
{
//	TRACE_D << "\n\n********************************* UpdateAfterUndoRedo::> id=" << id << " type=" << type << std::endl ;

	int notrack=-1;

	//> Stereo: check if we're in merged view mode or not,
	//  and if current view matches the mode
	if (getParent().isStereo())
	{
		if ((m_parent.getActiveViewMode()==VIEW_MODE_MERGED && m_viewTrack!=-1)
					|| (m_parent.getActiveViewMode()!=VIEW_MODE_MERGED && m_viewTrack ==-1) )
			return ;
	}

	bool isAnchor = m_dataModel.existsAnchor(id) && !m_dataModel.isMainstreamType(type, "") ;

	//> Type and id not empty
	if ( ! (type == "" && id == "") )
	{
		if (!isAnchor)
			notrack = m_dataModel.getElementSignalTrack(id) ;
		else
			notrack = m_dataModel.getAnchorSignalTrack(id) ;
		// current view not concerned by this update !
		if ( getViewTrack() != -1 && notrack != getViewTrack() )
			return;
	}
	//> Otherwise, Out !
	else
		return ;

	// 0=delete, 1=update, 2=insert
	int upd_action = 1 ;
	switch ( upd )
	{
		case DataModel::INSERTED : upd_action = 2 ; break ;
		case DataModel::UPDATED : upd_action = 1 ; break ;
		case DataModel::DELETED : upd_action = 0 ; break ;
		case DataModel::ANCHORED : upd_action = 1 ; break ;
		case DataModel::UNANCHORED : upd_action = 1 ; break ;
		default : upd_action = 1 ;
	}

	//> ANCHOR case
	if (isAnchor)
		updateAfterAnchorAction(id, notrack, upd) ;
	//> MAINSTREAM case
	else if ( m_dataModel.isMainstreamType(type, "") )
	{
		m_signalElementModified.emit(type, id, upd_action, false, notrack, true) ;
		// update parent too for unit
		// TODO move this in editor
		if (m_mainBaseType ==type )
		{
			string parent = m_dataModel.getParentElement(id) ;

			if ( !parent.empty() && (m_dataModel.getOrder(parent)==m_dataModel.getOrder(id)) && upd_action!=0 )
				m_signalElementModified.emit(m_dataModel.getElementType(parent), parent, upd_action, false, notrack, true) ;
		}
	}

//	TRACE_D << "UpdateAfterUndoRedo::> END *********************************\n\n" << std::endl ;
}


void AnnotationView::updateAfterAnchorAction(const string& anchorId, int notrack, DataModel::UpdateType upd)
{
	//-- Default action : update
	int upd_action = 1 ;

	std::set<AnnotationId> incoming ;
	std::set<AnnotationId> outgoing ;
	m_dataModel.getInAndOutAnnotationAtAnchor(anchorId, incoming, outgoing) ;
	std::set<AnnotationId>::iterator it ;
	string type ;
	for (it=incoming.begin(); it!=incoming.end(); it++)
	{
		m_signalElementModified.emit(m_dataModel.getElementType(*it), *it, upd_action, false, notrack, true) ;
	}
	for (it=outgoing.begin(); it!=outgoing.end(); it++)
	{
		type = m_dataModel.getElementType(*it) ;
		if ( type == m_mainBaseType || m_dataModel.isQualifierType(type, m_mainGraphType))
		{
			// try to update buffer for tagged element
			if ( ! m_dataModel.mainstreamBaseElementHasText(*it))
				getBuffer()->switchTimestamp(*it, upd==DataModel::ANCHORED) ;
			// update renderer for text element
			else
			{
				AnnotationRenderer* rend = getRenderer(type) ;
				if (rend)
					rend->updateElement(*it, getR2LMode(*it)) ;
			}
		}
		//-- If an anchor is beeing time coded, even if the related elements exist in model,
		//   a track segment must be created
		if ( upd == DataModel::ANCHORED )
			upd_action = 2 ;
		//-- In contrary, when removing timestamp we need to delete segment track
		else if ( upd == DataModel::UNANCHORED )
			upd_action = 0 ;
		m_signalElementModified.emit(m_dataModel.getElementType(*it), *it, upd_action, false, notrack, true) ;
	}
}

/*
 * scroll view to given text offset when idle
 */
bool AnnotationView::scrollViewWhenIdle(guint32 text_offset)
{
	//MSGOUT << " IN THREAD " << endl;
	gdk_threads_enter();
	//MSGOUT << " IN THREAD 2 " << endl;
	scrollView(text_offset);
	gdk_flush();
	gdk_threads_leave();
	//MSGOUT << " OUT THREAD " << endl;
	return false;
}

/**
 * Scrolls view to the given text anchor
 * @param id		Text anchor id
 * @return					False (for event mechanism)
 */
bool AnnotationView::scrollViewToId(const string& id)
{
	return scrollViewToIter(getBuffer()->getAnchoredIter(id));
}

/**
 * scroll view to given text buffer offset
 * @param text_offset target offset in text buffer
 * @return true if scroll done
 */
bool AnnotationView::scrollView(guint32 text_offset)
{
	if (text_offset == (guint32) - 1)
		return scrollViewToIter(getBuffer()->getCursor());
	else {
		return scrollViewToIter(getBuffer()->get_iter_at_offset(text_offset));
	}
}
/**
 * Scrolls view to the given text iter
 * @param it		Text iter
 * @return					False (for event mechanism)
 */
bool AnnotationView::scrollViewToIter(const Gtk::TextIter& it)
{
	Gtk::TextIter iter = it;
	float f1,  f2, f3;
	Gdk::Rectangle   	 visible_rect;
	get_visible_rect(visible_rect);
	int y, height;

	get_line_yrange(iter, y, height);

	bool need_scroll=false;

	if ( y < (visible_rect.get_y()+10)  )
	{
		need_scroll = iter.backward_visible_line();
	}
	else if (  (y+height) > (visible_rect.get_y()+visible_rect.get_height()-10) )
	{
		// TODO -> scroll a full page
		need_scroll = iter.forward_visible_line(1);
	}

	if ( need_scroll ) {
		scroll_to(iter);
	}

	return false;
}


/*========================================================================
 *
 *  Display / update annotations
 *
 ========================================================================*/




/*========================================================================
 *
 *  Display / update annotations
 *
 ========================================================================*/

void AnnotationView::insertBackgroundSegment(const string& id, int mode, bool del)
{
	getBuffer()->begin_user_action() ;

	vector<string> ids;
	vector<string>::iterator it;
	//	const string& to_show = m_configuration["AnnotationLayout,qualifiers,show"];
	const string to_show = ""; // provisoire
	const string& to_hide = m_configuration["AnnotationLayout,background,hide"];
	//	bool is_speech = m_dataModel.isSpeechSegment(id);

	//TODO: get all background
	m_dataModel.getBackgroundsInSegment(id, ids, mode);
	for (it = ids.begin(); it != ids.end(); ++it) {
		const string& qtype = m_dataModel.getElementType(*it);
		if (to_hide.empty() || to_hide.find(qtype) == string::npos)
			updateBackgroundDisplay(*it, mode, del) ;
	}
	getBuffer()->end_user_action() ;
}

void AnnotationView::insertBackgroundSegment(const string& id, int mode, const std::vector<string>& bgs)
{
	const string& to_hide = m_configuration["AnnotationLayout,background,hide"];
	std::vector<string>::const_iterator it ;
	for (it = bgs.begin(); it != bgs.end(); ++it) {
		const string& qtype = m_dataModel.getElementType(*it);
		if (to_hide.empty() || to_hide.find(qtype) == string::npos)
			updateBackgroundDisplay(*it, mode, false) ;
	}
}

void AnnotationView::deleteBackgroundSegment(const string& id, std::vector<string>& bgs)
{
	m_dataModel.getBackgroundsInSegment(id, bgs, 2);
	vector<string>::iterator it;
	for (it=bgs.begin(); it!=bgs.end(); it++) {
		// mode -1: only delete
		updateBackgroundDisplay(*it, -1, true) ;
	}
}


void AnnotationView::updateBackgroundDisplay(const string& id, int mode, bool del)
{
	//> LOCK
	if (!m_dataModel.existsElement(id))
		return ;
	if ( ! m_dataModel.isMainstreamType(m_dataModel.getElementType(id), m_bgGraphType))
		return ;

	if (del)
		m_renderer["background"]->deleteElement(id);

	//> update
	if (mode==2 || mode==0)
		m_renderer["background"]->render(id, false);
	if (mode==2 || mode==1)
		m_renderer["background"]->render_end(id, false);
}



/*========================================================================
 *
 *  Speakers management
 *
 ========================================================================*/

/**
 * replace speaker label by selected speaker
 * @param iter text iter corresponding to speaker label position
 * @param label new speaker label
 */
void AnnotationView::setSpeaker(const Gtk::TextIter& iter, string spkid, string label, float selectionStart, float selectionEnd)
{
	const string& currentId = getCurrentTaggedElement(iter, "turn") ;

	bool overlap = (spkid=="overlap") || m_overlapState ;

	//reset
	m_overlapState=false ;

	//> Modifying an EXISTING element
	if (!currentId.empty() && !overlap)
		modifySpeakerElement(currentId, spkid, label) ;
	//> Creating a new ELEMENT
	else
		createSpeakerElement(iter, spkid, overlap, selectionStart, selectionEnd) ;
}

void AnnotationView::modifySpeakerElement(const string& currentId, const string& spkId, const string& label)
{
	getBuffer()->begin_user_action() ;

	string prev_spkId = m_dataModel.getElementProperty(currentId, "speaker", Speaker::NO_SPEECH);
	//        if ( prev_id != id || id == Speaker::NO_SPEECH ) {
	if ( prev_spkId != spkId )
	{
		Speaker::Gender gender = Speaker::NO_SPEECH_GENDER;
		if ( spkId != Speaker::NO_SPEECH )
		{
			try {
				Speaker& spk =
						m_dataModel.getSpeakerDictionary().getSpeaker(spkId);
				gender = spk.getGender();
			}
			catch (...) {}
			m_dataModel.deleteElementProperty(currentId,
					"nospeech_type", false);
			m_dataModel.setElementProperty(currentId, "speaker", spkId, true);
		}
		else
		{
			if ( label != Speaker::noSpeaker().getFullName() )
				m_dataModel.setElementProperty(spkId, "nospeech_type", label, false);
			m_dataModel.deleteElementProperty(currentId, "speaker", true);
		}
	}

    setCurrentInputLanguage(currentId, true);

    //> Associate track to the last chosen speaker (except for no speaker )
    int no_tr = m_dataModel.getElementSignalTrack(currentId) ;

    if (spkId != Speaker::NO_SPEECH)
        getDataModel().setSpeakerHint(spkId, no_tr) ;

    getBuffer()->end_user_action() ;
}

void AnnotationView::createSpeakerElement(const Gtk::TextIter& iter, const string& spkId, bool overlap, float selectionStart, float selectionEnd)
{
	Log::setTraceLevel(Log::FINE);
	//> -- Search model attachment
	string type = "turn";
	string turn_id = getBuffer()->anchors().getPreviousAnchorId(iter, type);
	const Gtk::TextIter& tmpIter = getBuffer()->nextEditablePosition(iter);
	string prevId = getBuffer()->getPreviousAnchorId(tmpIter, m_mainBaseType, true);
	int notrack = (m_viewTrack == -1 ? 0 : m_viewTrack);

	int order=0;
	string diag;
	float cursor = -1.0;
	if ( m_dataModel.checkInsertionRules(type, notrack, cursor, 0.0, prevId, order, "", diag) == false )
	{
		#ifdef __APPLE__
		dlg::warning(diag, m_parent.getTopWindow());
		#else
		dlg::warning(diag);
		#endif
		return;
	}

	//> -- New overlapping turn
	if ( overlap )
	{
		getBuffer()->begin_user_action() ;

		//> -- Search buffer insertion
		Gtk::TextIter iter2 ;
		string nextId = m_dataModel.getNextElementId(turn_id) ;
		// no next turn
		if (nextId.empty())
			iter2 = getBuffer()->end() ;
		else
		{
			iter2 = getBuffer()->getAnchoredIter(nextId) ;
			iter2 = getBuffer()->previousEditablePosition(iter2) ;
		}
		getBuffer()->setCursor(iter2, false);

    	SpeakerMenu* spk_menu = (SpeakerMenu*) getRendererMenu(type, spkId) ;

		//> -- We're not at a turn: let's create 1st turn first
        if  ( order == 0 )
        {
            // TODEL
        	if (spk_menu)
        		m_dataModel.setSpeakerHint(spk_menu->getLastSelectedSpeaker(true), notrack);
            turn_id = m_dataModel.insertMainstreamElement(type, prevId, -1, true, true);
            order = 1;
        }

		//> -- Define alternate speaker name for overlapping turn
        string spk2 = "" ;
        if (spkId.empty() || spkId=="overlap")
        {
			//> -- Define alternate speaker name for overlapping turn
			string spk = m_dataModel.getElementProperty(turn_id, "speaker");
			SpeakerDictionary& dic = m_dataModel.getSpeakerDictionary();
			string spk1 = "" ;

			if (spk_menu)
			{
				spk1 = spk_menu->getLastSelectedSpeaker(true) ;
				spk2 = spk_menu->getLastSelectedSpeaker(false) ;
			}
			if (spk2 == spk)
				spk2 = spk1 ;
			if ( spk2 == spk )
			{
				const Speaker& newspk = dic.defaultSpeaker(m_lang);
				spk2 = newspk.getId() ;
				dic.addSpeaker(newspk) ;
				if (spk_menu)
					spk_menu->setLastSelectedSpeaker(spk2);
			}
        }
        else
        	spk2 = spkId ;

        m_dataModel.setSpeakerHint(spk2, notrack);
        string turn_id2 = m_dataModel.insertMainstreamElement(type, prevId, -1, true, true);

        getBuffer()->end_user_action() ;
	}

	//> -- New turn
	else
	{
		// -- from text cursor
		if (!isSignalSelectionAction())
		{
			getBuffer()->begin_user_action() ;
			m_dataModel.setSpeakerHint(spkId, notrack) ;
			m_dataModel.insertMainstreamElement(type, prevId, -1, true, true );
			getBuffer()->end_user_action() ;
		}
		// -- from signal selection
		else
		{
			Log::out() << "CREATE NEW TURN between start=" << selectionStart << " - end=" << selectionEnd << " in offtext=" << iter << std::endl ;

			//TODO
			getBuffer()->begin_user_action() ;

			// signal data
			float start, stop ;
			getParent().getSignalSelection(start, stop) ;

			// model data
			string bck_hint =  m_dataModel.getSpeakerHint(notrack);
			if ( spkId != "" )
				m_dataModel.setSpeakerHint(spkId, notrack);

			int text_offset = 0 ;

			// ensure text cursor is coherent with signal cursor
			m_parent.synchroTextToSignal(selectionStart, -1, true, false);

			// insert new turn start
			Gtk::TextIter tmp = getBuffer()->getCursor();
			if ( tmp != iter ) {
				TRACE_D << "££££££££££££££££££££    TMP=" << tmp << " ITER=" << iter << endl;
			}
			prevId = getBuffer()->getPreviousAnchorId(tmp, m_mainBaseType);
			if ( ! prevId.empty()
				&& ( fabs(m_dataModel.getElementOffset(prevId, true) - selectionStart) > m_dataModel.conventions().minSegmentSize(m_mainBaseType) ) ) {
				text_offset = getBuffer()->getOffsetInTextSegment(prevId, tmp, true );
				TRACE_D << "  INSERT FROM SELECTION start_iter=" << tmp << " start_id=" << prevId << " start_time=" << selectionStart << endl;
				string prev_spkid = m_dataModel.getParentProperty(prevId, "speaker");
				m_dataModel.insertMainstreamElement(type, prevId, selectionStart, true, true );
				m_dataModel.splitTextContent(prevId, text_offset, true);
				if ( prev_spkid != "" )
				m_dataModel.setSpeakerHint(prev_spkid, notrack);
			}

			m_parent.synchroTextToSignal(selectionEnd, -1, true, false);
			tmp = getBuffer()->getCursor();
			prevId = getBuffer()->getPreviousAnchorId(tmp, m_mainBaseType);
			if ( ! prevId.empty()
				&& ( fabs(m_dataModel.getElementOffset(prevId, true) - selectionEnd) > m_dataModel.conventions().minSegmentSize(m_mainBaseType) ) ) {
				text_offset = getBuffer()->getOffsetInTextSegment(prevId, tmp, true );

				TRACE_D << "  INSERT FROM SELECTION stop_iter=" << tmp << " stop_id=" << prevId << " stop_time=" << selectionEnd << endl;
				m_dataModel.insertMainstreamElement(type, prevId, selectionEnd, true, true );
				m_dataModel.splitTextContent(prevId, text_offset, true);
			}
			m_dataModel.setSpeakerHint(bck_hint, notrack);

			getBuffer()->end_user_action() ;
		}
	}
	Log::resetTraceLevel();
}

/**
 * relay edit Speaker properties when idle
 * @param iter annotation location in text buffer
 * @return bool always returns false to be called only once.
 */
bool AnnotationView::editSpeakerPropertiesWhenIdle(Gtk::TextIter iter)
{
// now it's possible to consult speaker properties in browse mode
//	if ( m_editable == false )
//		return false;

	gdk_threads_enter();
	editSpeakerProperties(iter);
	gdk_flush();
	gdk_threads_leave();
	return false;
}

/**
 * edit speaker properties for speaker used at given text position
 * @param iter text position of label for speaker to edit
 * @note emits signalEditSpeaker to request speaker properties edition
 */
void AnnotationView::editSpeakerProperties(const Gtk::TextIter& iter)
{
	string turn_id = getBuffer()->anchors().getPreviousAnchorId(iter, "turn");
	string spk = m_dataModel.getElementProperty(turn_id, "speaker");

	// disable modal
	m_signalEditSpeaker.emit(spk, false) ;
}

/**
 * CALLED WITHIN DIALOG TURN PROPERTIES
 * edit speaker properties for given speaker id
 * @param spkid id of speaker to edit
 * @note emits signalEditSpeaker to request speaker properties edition
 */
void AnnotationView::editSpeakerProperties2(string spkid)
{
	// disable modal
	m_signalEditSpeaker.emit(spkid, true) ;
}

/**
 * Create new section / replace section label by given section type + details
 * @param iter text position of section label to replace
 * @param seg_type section type
 * @param label section details
 */
void AnnotationView::setSection (const Gtk::TextIter& iter, string seg_type, string label)
{
	string currentId = getCurrentTaggedElement(iter, "section") ;
	string prev_type  = (currentId.empty() ? "" : m_dataModel.getElementProperty(currentId, "type"));

	if ( prev_type.empty() || prev_type != seg_type ) {

		getBuffer()->begin_user_action() ;

		if ( currentId.empty()) {
			//> -- Creating a new element
			string prevId = getBuffer()->getPreviousAnchorId(iter, m_mainBaseType);
			// -- Check if the track matches
			if ( m_dataModel.getNbTracks() > 1 )
			{
				while ( ! (prevId.empty() || m_dataModel.getElementSignalTrack(prevId) == m_viewTrack) )
					prevId = getBuffer()->getPreviousAnchorId(prevId, m_mainBaseType) ;
			}

			currentId = m_dataModel.insertMainstreamElement("section", prevId, -1, true );
		}
		m_dataModel.setElementProperty(currentId, "type", seg_type, true);
//		if ( seg_type == "nontrans" ) {
//				// also set section turn to "no_speech"
//				const string& turn_id = m_dataModel.getAlignmentId(currentId);
//				if ( !turn_id.empty() )
//					m_dataModel.deleteElementProperty(turn_id, "speaker");
//				m_cancelledPopupId = turn_id; // no need to pop up speaker selection menu
//		}
		getBuffer()->end_user_action() ;
	}
}



// TODO -> verifier a quoi ca sert !

std::string AnnotationView::getCurrentTaggedElement(const Gtk::TextIter& iter, const string& type)
{
	bool tag = getBuffer()->iterHasTag(iter, type, true) ;

	if (!tag)
		return "" ;

	return getBuffer()->anchors().getPreviousAnchorId(iter, type) ;
}

//void AnnotationView::actualizeTrackCursor(int notrack, bool move)
//{
//	getBuffer()->changeTrack(notrack, move) ;
//}

/**
 * relay edit Annotation properties when idle
 * @param iter annotation location in text buffer
 * @param type annotation type
 * @return bool always returns false to be called only once.
 */
bool AnnotationView::editAnnotationPropertiesWhenIdle(Gtk::TextIter iter, string type)
{
	gdk_threads_enter();
	editAnnotationProperties(iter, type);
	gdk_flush();
	gdk_threads_leave();
	return false;
}

/**
 * edit Annotation properties of given type closest to current cursor position
 */
void AnnotationView::editAnnotationPropertiesAtCursor(string type)
{
	editAnnotationProperties(getBuffer()->getCursor(), type);
}

/**
 * edit current annotation properties
 * @param iter annotation location in text buffer
 * @param type annotation type
 * @note according to annotation type, opens appropriate properties dialog box
 */
void AnnotationView::editAnnotationProperties (const Gtk::TextIter& iter, string type)
{
	guint i;
	Gtk::Window* top = m_parent.getTopWindow();
	if ( top == NULL ) {
		MSGOUT << "got NULL top window pointer" << endl;
		return;
	}

	try
	{
		string id("");
		/*** BACKGROUND ***/
		if (type == "background")
		{
			// edit background properties
			id = getBuffer()->getTaggedElementId(iter);
			editBackground(id);
		} else {
			AnnotationRenderer* renderer =NULL;
			if ( m_dataModel.isMainstreamType(type, m_mainGraphType) ) {
				id = getBuffer()->getPreviousAnchorId(iter, type, false);
			} else if ( type.compare(0, 9, "qualifier") == 0 || m_dataModel.isQualifierType(type) ) {
				id = getBuffer()->getTaggedElementId(iter);
			}
			renderer = getRenderer(type);
			if ( !id.empty() && renderer != NULL ) {
				getBuffer()->begin_user_action() ;
				AnnotationPropertiesDialog* dlg = renderer->newPropertiesDialog(id);
				if ( dlg->run() ==  Gtk::RESPONSE_OK )
					TRACE << "  updated : " << m_dataModel.getElementType(id) << " id="<< id << endl ;
				getBuffer()->end_user_action() ;
			}

		}

	} //end try
	catch (const char* msg) {
		#ifdef __APPLE_
		dlg::warning(msg, m_parent.getTopWindow());
		#else
		dlg::warning(msg);
		#endif
	}
}


/**
 * delete Annotation at or near iter position
 * @param iter text buffer iterator
 * @param type annotation type
 *
 * @note browse back in buffer from iter position to find  annotation to delete;
 *      check that data model deletion rules are fulfilled, and do deletion.
 */
void AnnotationView::deleteAnnotation(const Gtk::TextIter& iter, string type)
{
	// Mainstream
	const string& qid = getBuffer()->getTaggedElementId(iter) ;
	bool done = false ;

	if (type == m_mainBaseType && ! qid.empty() )
	{
		getBuffer()->begin_user_action() ;

		// -- Delete foreground non-text element : special delete
		if ( ! m_dataModel.mainstreamBaseElementHasText(qid, m_mainGraphType)  )
		{

//			string prevId = getBuffer()->getPreviousAnchorId(iter, type, false) ;
			try {
				m_dataModel.deleteEventMainstreamElement(qid, true) ;
			} catch (const char* diag) {
				#ifdef __APPLE__
				dlg::warning(diag, m_parent.getTopWindow());
				#else
				dlg::warning(diag);
				#endif
			}
			done = true ;
		}
		// -- 	Deleting text element before qualifier/unit tag : specific treatment
		// 		(otherwise will be proceded like other mainstreams)
		else
		{
			Gtk::TextIter tmp = iter ;
			tmp.backward_char() ;
			vector<string> tagnames ;
			tagnames.push_back("unit") ;
			tagnames.push_back("qualifier_") ;
			if ( getBuffer()->iterHasTags(tmp, tagnames, true) )
			{
				changeTimestamp(qid, -1, false) ;
				done = true ;
			}
		}
		getBuffer()->end_user_action() ;
	}

	if (done)
		return ;

	if ( m_dataModel.isMainstreamType(type, m_mainGraphType) )
	{
		string prevId = getBuffer()->getPreviousAnchorId(iter, type, true);
		if ( ! prevId.empty() )
		{
			getBuffer()->begin_user_action() ;
			int notrack = m_dataModel.getElementSignalTrack(prevId);

			//			TRACE << " ## Delete annotation type=" << type << " id=" << prevId << " with_children=" << with_children << " status=" << diag << endl;
			try {
				m_signalElementModified.emit(type, prevId, 8, false, notrack, true) ;

				// set cursor on previous editable position
				getBuffer()->setCursor(getBuffer()->previousEditablePosition(iter, "", true), false);
				//TRACE_D << " DELETE AT " << iter << " CURSOR SET AT " << getBuffer()->getCursor() << endl;
				// freeze segment track update until all update actions done.
//				m_dataModel.deleteMainstreamElement(prevId);
				m_dataModel.deleteElement(prevId) ;
			}
			catch (const char* msg)
			{
				string error = string(msg) ;
				#ifdef __APPLE__
				dlg::warning(error, m_parent.getTopWindow());
				#else
				dlg::warning(error);
				#endif
			}
			m_signalElementModified.emit(type, prevId, 9, false, notrack, true) ;

			getBuffer()->end_user_action() ;
		}
	}
	// Event and entities
	else if (type.compare(0, 9, "qualifier") == 0 || m_dataModel.isQualifierType(type))
	{
		getBuffer()->begin_user_action() ;
		const string& qid = getBuffer()->getTaggedElementId(iter);

		if ( ! qid.empty() )
		{
			TRACE << " ## Delete annotation type=" << type << " id=" << qid  << endl;
			m_dataModel.deleteElement(qid, true);
		}
		getBuffer()->end_user_action() ;
	}
	// Background
	else if (type == "background")
	{
		getBuffer()->begin_user_action() ;
		const string& qid = getBuffer()->getTaggedElementId(iter);
		if (!qid.empty())
			deleteBackground(qid, true) ;
		getBuffer()->end_user_action() ;
	}
}

/**
 * delete Annotation of given type closest to current cursor position
 */
void AnnotationView::deleteAnnotationAtCursor(string type)
{
	deleteAnnotation(getBuffer()->getCursor(), type);
}

/*========================================================================
 *
 * Qualifiers management
 *
 ========================================================================*/
/**
 * set qualifier type at curpos position
 * @param iter position of event-tagged text to insert or replace
 * @param type selected event type
 * @param desc selected event desc
 * @note if iter corresponds to event-tagged text -> corresponding qualifier will be updated,
 *     else new qualifier will be inserted
 */
void AnnotationView::setQualifier(const Gtk::TextIter& iter, std::string type, std::string desc, string qual_class)
{
	Gtk::TextIter start, stop;
	string endId("");
	int start_offset = 0;
	int end_offset = 0;
	bool has_sel = false;
	bool can_upd = false;

	Glib::RefPtr<Gtk::TextTag> tag =  getBuffer()->iterHasTag(iter, m_dataModel.getAGTrans(), true);
	can_upd = ( tag != 0 && !iter.begins_tag(tag) ) ;

	start = iter;
	has_sel = getBuffer()->getSelectedRange(iter, start, stop, true);
	getBuffer()->clearSelection();
	TRACE << "setQualifier " << type << " " << desc << " at iter = " << iter << " start = " << start << " stop=" << stop << endl;


	//TRACE << "CHAR AT STOP: -" <<  stop.get_char() << "-" << endl;
	//printf("char at stop -%x-:", stop.get_char()) ;
	//TRACE << " START=" << start << "  stop=" << stop << endl;
	if ( start.compare(stop) != 0 )
		getIdAndOffsetFromIterator(stop, endId, end_offset);

	string qid = getBuffer()->getTaggedElementId(start);

	try
	{
		if ( can_upd && !qid.empty() )
		{
			getBuffer()->begin_user_action() ;
			m_dataModel.setQualifier(qid, type, desc, true);
			getBuffer()->end_user_action() ;
		}
		else
		{
			getBuffer()->begin_user_action() ;
			string prevId;
			bool reattach = false;

			getIdAndOffsetFromIterator(start, prevId, start_offset);
			if ( prevId.empty() )
			{
				MSGOUT << "No text segment found at " << start << endl;
				return;
			}
			if ( start_offset > 0 && end_offset == 0 )
			{
				// check if after event_end -> possible need for extra split
				bool ok = true;
				int off = 0;
				Gtk::TextIter prev=start;
				ok = prev.backward_char();
				while (ok &&  g_unichar_isspace(prev.get_char()) )
				{
					ok = prev.backward_char();
					++off;
				}
				if ( off > 0 ) start.backward_chars(off) ;

				if ( getBuffer()->iterHasTag(prev, "qualifier_event_end") && off > 0 )
					reattach = true;
			}
			// inserting new qualifier just before existing qualifier at segment start
			// -> ajust prevId to previous segment id
			else if ( start_offset < 0
						&& (stop.compare(start) == 0)
						&& (start.compare(getBuffer()->getAnchoredIter(prevId)) == 0) )
			{
				prevId = getBuffer()->getPreviousAnchorId(prevId, m_mainBaseType, false);
				start_offset = getBuffer()->getOffsetInTextSegment(prevId, start, true);
			}

			TRACE << "ANNOTATE EVENT start=" << start << " prevId=" << prevId << " start_offset=" << start_offset << " endId=" << endId << " end_offset=" << end_offset << endl;
			bool need_split = !(m_dataModel.conventions().isInstantaneous(type)) ;

			//> -- Split at the end of the selection
			if ( end_offset > 0 )
			{
				guint sav = start.get_offset() ;
				endId = splitTextAnnotation(stop, true, true) ;
//				endId = splitTextAnnotation(stop, true, false) ;
				need_split = false ;
				start = getBuffer()->get_iter_at_offset(sav) ;
			}
			//> -- No selection ? if we're inside a word we'll have to split anyway
			else
				need_split = (need_split && endId.empty());

			getBuffer()->setCursor(start, false);

			//> -- Split at the start cursor
			if ( start_offset > 0 )
			{
				prevId = splitTextAnnotation(start, true, false);
				start = getBuffer()->getCursor();
				TRACE << "  -> start set at " << start << endl;
			}

			// non-instantaneous event inserted -> split if it has not been done just before
			if ( need_split )
				endId = splitTextAnnotation(start, true, false);

			//> -- Add qualifier in model
			m_dataModel.addQualifier(type, prevId, endId, desc, true);

			getBuffer()->end_user_action() ;
		}
	}
	catch (const char* msg)
	{
		#ifdef __APPLE__
		dlg::warning(msg, m_parent.getTopWindow());
		#else
		dlg::warning(msg);
		#endif
	}
}

void AnnotationView::setForegroundEvent(const Gtk::TextIter& iter, std::string type, std::string subtype, float start, float end)
{
	int text_offset = 0 ;

	//TODO usefull ?
	bool selection = getBuffer()->get_has_selection() ;
	if (selection)
		return ;

	//> -- If existing element and update allowed, let's change element properties
	string id = getBuffer()->getTaggedElementId(iter);
	Glib::RefPtr<Gtk::TextTag> tag = getBuffer()->iterHasTag(iter, "unit", true);
	if ( !id.empty() && tag && !iter.begins_tag(tag) )
	{
		if ( !m_dataModel.mainstreamBaseElementHasText(id, m_mainGraphType) )
		{
			getBuffer()->begin_user_action() ;
			m_dataModel.setEventMainstreamElement(id, type, subtype, true) ;
			getBuffer()->end_user_action() ;
		}
	}
	//> -- Otherwise, means we need to create a brand new one, let's go
	else
		createForegroundEvent(iter, type, subtype, start, end) ;
}


//
//

void AnnotationView::createForegroundEvent(const Gtk::TextIter& iter, std::string type, std::string subtype, float start, float end)
{
	Gtk::TextIter it = iter ;

	getBuffer()->begin_user_action() ;

	//-- If an anchor exists at position, we need to insert a space for avoiding to superpose the existing mark
	//   and the one that will be created
	// -- Remark : don't do that for first child
	const string& anchorAtPos = getBuffer()->getAnchorIdByType(iter, m_mainBaseType, true) ;
	if (!anchorAtPos.empty() && !m_dataModel.isFirstChild(anchorAtPos, ""))
	{
		const std::pair<Gtk::TextIter,bool>& res =  getBuffer()->insert_interactive(iter, " ", getParent().isEditMode()) ;
		if ( res.second )
			it = res.first ;
	}

	string inserted, next ;

	//-- Insert foreground event
	string currentId = getBuffer()->getPreviousAnchorId(it, getDataModel().mainstreamBaseType(m_mainGraphType)) ;

	bool ko = false ;
	Glib::ustring currentTxt = "" ;
	string currentType = "" ;
	string currentOrder = m_dataModel.getElementProperty(currentId, "order", "") ;
	string currentScore = ""  ;

	// -- Check whether we are splitting a text or an event unit, and keep text data
	int splitTextOffset = -1 ;
	int helpOffset = -1 ;
	bool currentIsEvent = ! m_dataModel.mainstreamBaseElementHasText(currentId, m_mainGraphType) ;
	if ( !currentIsEvent )
	{
		splitTextOffset = getBuffer()->getOffsetInTextSegment(currentId, it, false) ;
		currentTxt = m_dataModel.getElementProperty(currentId, "value", "") ;
		currentScore = m_dataModel.getElementProperty(currentId, "score", "") ;
		helpOffset = checkStringOffset(currentTxt, splitTextOffset) ;
	}

	bool beforeText = ( helpOffset==0 ) ;
	bool afterText =  ( helpOffset==1 ) ;

	bool currentFirstChild = m_dataModel.isFirstChild(currentId, "") ;
	bool currentLastChild = m_dataModel.isLastChild(currentId, "") ;
	string previousId, nextId ;
	if (!currentFirstChild)
		previousId = m_dataModel.getPreviousElementId(currentId) ;
	if (!currentLastChild)
		nextId = m_dataModel.getNextElementId(currentId) ;

	/*
	 * 	Inserting into a text without splitting text
	 */
	if ( !currentIsEvent && (beforeText || currentTxt.empty()) )
	{
		//-- Empty text and current is anchored ? just replace
		if ( currentTxt.empty() && m_dataModel.isAnchoredElement(currentId, 0))
			inserted = m_dataModel.setEventMainstreamElement(currentId, type, subtype, true) ;
		//-- Before some text ? use currentId as new event and eventually propagate text value in next element
		else
		{
			/*
			 *  	No next element
			 *  or	Next element is an event
			 *  or  Next element is anchored
			 *  ==> we can't propagate text to next element, we have to split
			 */
			if ( currentLastChild || !m_dataModel.mainstreamBaseElementHasText(nextId) || m_dataModel.isAnchoredElement(nextId, 0) )
			{
				// insert
				inserted = m_dataModel.insertMainstreamBaseElement(currentId, start, true, true) ;

				// -- set new values
				m_dataModel.setEventMainstreamElement(currentId, type, subtype, true) ;

				// newly created receive txt data
				m_dataModel.setElementProperty(inserted, "value", currentTxt, true) ;
				m_dataModel.setElementProperty(inserted, "subtype", "unit_text", true) ;
				if (!currentScore.empty())
					m_dataModel.setElementProperty(inserted, "score", currentScore, true) ;

				next = inserted ;
			}
			/*
			 *  Otherwise, use currentId as new event and propagate text to next element
			 */
			else
			{
				inserted = m_dataModel.setEventMainstreamElement(currentId, type, subtype, true) ;
				string txt = m_dataModel.getElementProperty(nextId, "value", "") ;
				m_dataModel.setElementProperty(nextId, "value", currentTxt + txt, true) ;
			}
		}
	}
	/*
	 *  Insertion at segment end
	 */
	else if ( currentLastChild && (afterText || currentTxt.empty()) )
	{
		inserted = m_dataModel.insertEventMainstreamElement(currentId, start, type, subtype, false, true) ;
	}
	/*
	 *  Inserting event just before another fg event
	 *  (do nothing here for segment start & segment end case - will be done in last case)
	 */
	else if ( currentIsEvent && !previousId.empty() /*&& !m_dataModel.isAnchoredElement(currentId, 0)*/)
	{
		inserted = m_dataModel.insertEventMainstreamElement(previousId, start, type, subtype, false, true) ;
	}
	/*
	 *  All other cases
	 */
	else
	{
		inserted = m_dataModel.insertEventMainstreamElement(currentId, start, end, type, subtype, true, true) ;
		next = m_dataModel.getNextElementId(inserted) ;
	}

	if (!inserted.empty())
		m_dataModel.setElementProperty(inserted, "order", currentOrder, false) ;

	// If we have splitted (at start or/and at end) don't forget to update model
	// for splitted text
	if (!next.empty())
		updateDataModel(next, false) ;
	if (!currentId.empty())
		updateDataModel(currentId, false) ;

	getBuffer()->end_user_action() ;
}

/*========================================================================
 *
 *  getSegmentText / store pending edits
 *
 ========================================================================*/

/**
 * getSegmentText : return Anchored label // text associated to segmentation
 * @param type segment type
 * @param id segment id
 * @param end_id segment end id
 *
 *  return value is typically a section label, a turn speaker, or the transcription text
 */
Glib::ustring AnnotationView::getSegmentText(const std::string& type, const std::string& id, const std::string& end_id)
{
	if ( type == m_dataModel.segmentationBaseType() /*|| type == m_mainBaseType*/ )
	{
		if ( end_id == "end" )
			return getBuffer()->getSegmentText(id, m_dataModel.getMainstreamNextElement(id), true);
		else
			return getBuffer()->getSegmentText(id, end_id, true);
	}
	/* no text on unit */
	//TODO use a hint in config for indicate which type should display text data
	else if (type == m_mainBaseType)
		return "";
	else if (type == "background")
		return m_renderer["background"]->getSegmentText(id) ;

	else if (type== "alignmentREF" || type== "alignmentHYP")
		return m_dataModel.getElementProperty(id, "value", "") ;

	return getBuffer()->getAnchoredLabel(id);
}

/*
 * inhibate spell checking
 */
/* SPELL */
/*
void AnnotationView::inhibateSpellChecking(bool b, bool recheck)
{
	if ( m_speller != NULL ) {
		gtkspell_inhibate_check(m_speller, b);
		getBuffer()->setSpeller((b ? NULL: m_speller));
		if (recheck)
			gtkspell_recheck_all(m_speller);
	}
}
*/

/*
 * set pending edits indicator
 */
void AnnotationView::inhibateStoreEdits(bool b)
{
	m_inhibPendingEdits = b;
	getBuffer()->inhibateEditSignal(b);
	//	m_pendingTextEdits = 0;
}

void AnnotationView::setHasPendingEdits(const Gtk::TextIter& pos, int nbbytes)
{
	if ( ! m_inhibPendingEdits ) {
		if ( nbbytes > 0 ) {
			m_pendingTextEdits += nbbytes;
			m_lastIterOffset.push(pos.get_offset());
		}
		else
			m_pendingTextEdits = MAX_PENDING;
		if ( m_pendingTextEdits >= MAX_PENDING )
			Glib::signal_idle().connect(sigc::mem_fun(*this, &AnnotationView::storePendingTextEditsWhenIdle));
	}
}

bool AnnotationView::storePendingTextEditsWhenIdle()
{
	gdk_threads_enter();
	storePendingTextEdits(false,false);
	gdk_threads_leave();
	return false;
}

/*
 * if any updates performed on text buffer, store them in DataModel
 */
bool AnnotationView::storePendingTextEdits(bool isUndo, bool force)
{
	// consultation mode: exit
	if (!m_editable)
		return false;

	// pending text edit mechanism disabled: exit
	if ( m_inhibPendingEdits )
		return false;

	if ( force )
		m_lastIterOffset.push(getBuffer()->get_insert()->get_iter().get_offset());

	//TRACE_D << " IN storePendingTextEdits force=" << force << " m_pendingTextEdits=" << m_pendingTextEdits << endl;
	//	if ( ! m_lastIterOffset.empty() ) TRACE_D << " last_iter=" << getBuffer()->get_iter_at_offset(m_lastIterOffset.top()) << endl;

	if ( !force && getBuffer()->insertInProgress() )
	{
		//		TRACE_D << "WARNING : AnnotationView::storePendingTextEdits while insertInProgress at " << getBuffer()->get_insert()->get_iter().get_offset() << "  !!!!" << endl;
		return true;
	}

	if ( force || m_pendingTextEdits > 0 )
	{
		// get data at last edit position
		m_pendingTextEdits = 0;

		string previd = "";
		string id;
		while ( ! m_lastIterOffset.empty() )
		{
			const Gtk::TextIter iter = getBuffer()->get_iter_at_offset(m_lastIterOffset.top());
			m_lastIterOffset.pop();
			const string& id = getBuffer()->anchors().getPreviousAnchorId(iter, m_mainBaseType,false);

			if (id == previd)
				continue;

			if ( iter.editable() == false )
			{
				// check if iter is just on a mark -> if yes, then also update previous text segment
				Anchor* aid = getBuffer()->anchors().getAnchor(id);
				if ( aid!=NULL)
				{
					if( aid->getIter().compare(iter) == 0 )
					{
						previd = getBuffer()->anchors().getPreviousAnchorId(id, m_mainBaseType,false);
						updateDataModel(previd, false);
					}
				}
				else
					TRACE << "AnnotationView:> storePendingEdit:> anchor not found : " << id << endl ;
			}
			previd = id;

			updateDataModel(id, false);
		}
	}
	return false;
}

/**
 * update text annotation in data model
 * @param id id of text annotation to update
 * @param emit_signal if true, signalElementModified will be emitted by data model
 *  current text annotation value is retrieved from associated text buffer.
 */
void AnnotationView::updateDataModel(const string& id, bool emit_signal)
{
	// <!> Desactivate the dataModel UNDO/REDO mecanism :
	//     Each time the buffer is modified this method is called,
	//     therefore the update of the dataModel concerning all what is done
	//     by this current method will be done by the current method itself
	//     (called by the undo/redo of text)
	if ( id != "" )
	{
//		TRACE << "!!!!! updateDataModel  ID=" << id << std::endl ;

		if ( !m_dataModel.mainstreamBaseElementHasText(id))
			return ;

		m_dataModel.inhibateUndoRedo(true) ;
		const string& nextId = m_dataModel.getNextElementId(id);
		const string& text = getBuffer()->getSegmentText(id, nextId, false, true);

		m_dataModel.setElementProperty(id, "value", text, emit_signal);
		if ( m_withConfidence )
			m_dataModel.setConfidence(id, DataModel::USER_INPUT, emit_signal);
		if (m_dataModel.hasElementProperty(id, "desc") )
			m_dataModel.deleteElementProperty(id, "desc", emit_signal) ;

//		TRACE << "!!!!! updateDataModel  ID=" << id << " TEXT=(" << text <<")  emit=" << emit_signal << endl;

		//> If datamodel doesn't have to send signal to view,
		//  still force update of signal segments : mainstreamBaseType & segmentationBaseType
		if ( emit_signal == false )
		{
			m_signalElementModified.emit(m_mainBaseType, id, 1, true, m_viewTrack, true);
			string parent = m_dataModel.getParentElement(id, m_dataModel.segmentationBaseType()) ;
			m_signalElementModified.emit(m_dataModel.segmentationBaseType(), parent, 1, true, m_viewTrack, true);
		}
		m_dataModel.inhibateUndoRedo(false) ;
	}
}


/*========================================================================
 *
 *  Annotation popup menu
 *
 ========================================================================*/


/**
 * popup annotation-type related menu
 */
bool AnnotationView::popupAnnotationMenu(const string& type, const Gtk::TextIter& textIter, GdkEvent* event, float start, float end, const string& hint, bool force)
{
	// -- Keep selection information, will be use in popup callback
	bool fromSignalSelection = (start>=0.0 && end>=0.0 && start!=end) ;
	setSignalSelectionAction(fromSignalSelection) ;


	AnnotationMenu* annot_menu = NULL ;
	Gtk::Menu* merged_events_menu = NULL ;

	// -- For event, merge foreground and qualifier
	if (type == "event")
	{
		merged_events_menu = Gtk::manage( new Gtk::Menu() ) ;
		fillAnnotationMenu(merged_events_menu, textIter, getBuffer()->hasSelection(), true) ;
	}
	// -- Otherwise search for specific menu
	else
		annot_menu = getRendererMenu(type, hint, m_editable) ;

	if ( !annot_menu && !merged_events_menu)
		return false;

	guint32 event_time = 0 ;
	GdkEventType event_type ;

	bool can_delete = ( m_editable && (!hint.empty() || force) ) ;
	bool can_properties = ( m_editable && (!hint.empty() || force) ) ;
	bool can_unanchor = false ;

	if ( !hint.empty() && m_dataModel.existsElement(hint))
	{
		bool is_qual = ( m_dataModel.isQualifierType(type) || type=="qualifier_event" || type=="qualifier_entity" ) ;
		bool is_fgevent = ( m_mainBaseType==type && !m_dataModel.mainstreamBaseElementHasText( hint )) ;

		if ( is_qual || is_fgevent )
		{
			if ( m_dataModel.isAnchoredElement(hint, 0) )
				can_unanchor = true ;
		}
	}

	// -- Prepare popup position & values
	if ( event == NULL )
		event_type = GDK_KEY_PRESS;
	else
	{
		event_type= *(GdkEventType*)event;
		event_time = gdk_event_get_time (event);
	}

	GdkWindow* window = NULL;

	switch ( event_type )
	{
		// - Launched from mouse click
		case GDK_BUTTON_PRESS:
			m_popup_x = (int)((GdkEventButton*)event)->x;
			m_popup_y = (int)((GdkEventButton*)event)->y;
			window = ((GdkEventButton*)event)->window;
			break;
		// - Launched from menu or shortcut
		case GDK_KEY_PRESS:
		{
			Gdk::Rectangle location;
			get_iter_location(textIter, location);
			buffer_to_window_coords(Gtk::TEXT_WINDOW_TEXT, location.get_x(), location.get_y(), m_popup_x, m_popup_y);

			Glib::RefPtr<Gdk::Window> win = get_window(Gtk::TEXT_WINDOW_TEXT);
			if ( win != 0 )
				window = win->gobj();
			else if ( event !=  NULL )
				window =  ((GdkEventKey*)event)->window;
			m_popup_x += 10;
			m_popup_y += 15;
			break;
		}
		default:
			break;
	}

	if ( window != NULL )
	{
		gint winx, winy;
		gdk_window_get_origin(window, &winx, &winy);
		m_popup_x += winx;
		m_popup_y += winy;
	}

	//> -- Launch popup
	if (annot_menu)
		annot_menu->popup(textIter, m_popup_x, m_popup_y, event_time, true, can_properties , can_delete, can_unanchor, start, end);
	else if (merged_events_menu)
		merged_events_menu->popup(sigc::mem_fun(*this, &AnnotationView::onPopupMenuPosition), 1, event_time);
	else
		return false ;

	return true;
}

void AnnotationView::onPopupMenuPosition(int& x, int& y, bool& push_in)
{
	x = m_popup_x ;
	y = m_popup_y ;
	push_in = TRUE ;
}

/*========================================================================
 *
 *  Events bindings
 *
 ========================================================================*/

/**
 * handler for set text cursor event
 * @param iter new cursor location
 * @param force if true force synchronisation
 *
 * @note adjusts current input language setting on turn changes, and reemits set cursor signal
 * to client apps like AnnotationEditor.
 */
void AnnotationView::emitSetCursor(const Gtk::TextIter& iter, bool force)
{
	if ( m_autosetLanguage && !m_inhibateSynchro ) // AJOUT PLR
		setCurrentInputLanguage(iter, force);
	if ( !m_inhibateSynchro )
		signalSetCursor().emit(iter, force);
}

/**
 *  paste current selection clipboard contents in buffer if valid insertion position
 * @param text selection clipboard contents
 * @param text_only if true, will only paste textual content of the selection buffer,
 * 			if false will insert objects hold in selection buffer ("special paste")
 */
void AnnotationView::onSelectionPasteEvent(const Glib::ustring& text, bool text_only)
{
	getBuffer()->begin_user_action() ;

	try
	{
		DataModel_CPHelper helper(&m_dataModel) ;
		string xml_prefix="<?xml";
		if ( text_only)
		{
			// paste selected text at cursor position
			// -> get selected text from xml paste string
			if ( text.compare(0, xml_prefix.size(), xml_prefix) == 0 )
				pasteSelectedText(helper.getTextFromTAGBuffer(text, m_mainBaseType, "value"));
			else
				pasteSelectedText(text);
		}
		else
		{
			bool unused ;
			bool is_edit = getBuffer()->isEditablePosition(getBuffer()->get_insert()->get_iter(), unused, true);

			if (is_edit)
			{
				if ( text.compare(0, xml_prefix.size(), xml_prefix) == 0 )
				{
					// paste graph part at cursor position
					string where_id="" ;
					int where_offset = 0 ;
					Gtk::TextIter pos = getBuffer()->getCursor() ;
					getIdAndOffsetFromIterator(pos, where_id, where_offset, true) ;

//					Log::out() << "~~~~~~~ onPaste where_id=" << where_id << " of type=" << m_dataModel.getElementType(where_id) << std::endl ;

					if ( !where_id.empty() )
					{
						float signaloff = getParent().getSignalView()->getCursor() ;
						float signalWhereId = m_dataModel.getElementOffset(where_id, true) ;

						//- Where id is an unanchored element: let find the previous anchored element
						//  for help signal compareason
						if ( signalWhereId < 0 )
						{
							string prev = m_dataModel.getAnchoredBaseTypeStartId(where_id) ;
							if (!prev.empty())
								signalWhereId = m_dataModel.getElementOffset(prev, true) ;
						}

						int rep = Gtk::RESPONSE_NO ;

						//- Now check whether the signal in audio component is different than the last anchored element
						//  > if same, use graph to re-attach
						if ( (signaloff - signalWhereId <= DataModel::EPSILON) )
							signaloff = -1 ;

						if (rep != Gtk::RESPONSE_CANCEL )
						{
							getBuffer()->setSpecialPasteInitialized(true) ;
							const string& graphId = helper.getGraphFromTAGBuffer(text);
							string error ;
							bool done = helper.insertSubgraph(where_id, where_offset, signaloff, graphId, true, true, error);
							getBuffer()->setSpecialPasteInitialized(false) ;

							if ( done )
								emitSetCursor(getBuffer()->getCursor(), false) ;
							else
								dlg::error(error, m_parent.getTopWindow()) ;
						}
					}
					else
						gdk_beep() ;
				}
				else
					pasteSelectedText(text);
			}
			else
				gdk_beep() ;
		}
	}
	catch (AGException& e)
	{
		get_display()->beep();
		dlg::error(_("An error occurred when inserting selection."), e.error(), m_parent.getTopWindow(), false);
	}
	catch (const char* msg)
	{
		get_display()->beep();
		string det = string(msg) ;
		dlg::error(_("An error occurred when inserting selection."), det, m_parent.getTopWindow(), false);
	}
	getBuffer()->end_user_action() ;
}

/**
 *  paste string in buffer if valid insertion position
 * @param text text string
 */
void AnnotationView::pasteSelectedText(const Glib::ustring& text)
{
	if ( ! m_editable ) {
		gdk_beep() ;
		return ;
	}

	bool spaceHandling = m_dataModel.conventions().automaticSpaceHandling() ;
	bool spaceBordering = m_dataModel.conventions().spaceBorderingForced() ;

	// check if insert position is valid
	bool need_split;
//	Gtk::TextIter iter = getBuffer()->getCursor();
//	Gtk::TextIter iter2 = iter;
//	bool ok = iter2.backward_char();
	Gtk::TextIter start, stop;
	bool del_sel = ( getBuffer()->get_selection_bounds(start, stop) ) ;

	//> DEAL IF SOME SELECTION EXISTS
	//===> adjust iter if some caracters need to be deleted, for case where
	//	   spaceHandling is done and we need to delete one of 2 consecutive spaces
	//     Or if spaceBordering is done and we need to delete border-spaces of a special
	//     caracter
	if ( del_sel ) {
		std::vector<Gtk::TextIter> to_del_mod ;
		bool ok = getBuffer()->spaceDeleter(start, stop, to_del_mod, spaceHandling, spaceBordering) ;
		if ( !ok || to_del_mod.size()!=2 ) {
			getBuffer()->end_user_action() ;
			return ;
		}
		else {
			start = to_del_mod[0] ;
			stop = to_del_mod[1] ;

			while (!start.editable())
				if (!start.forward_char())
					break;
		}
	}
	//> NO SELECTION, JUST LOOK IF WE CAN INSERT AT CURRENT POSITION
	//===> for case where spaceBordering is set, and we're between border-space and
	//     special caracter
	else {
		start = getBuffer()->getCursor();
		stop = start;
		bool ok = insertionKeySpaceFilter(0, stop, false, spaceBordering) ;
		if (!ok) {
			getBuffer()->end_user_action() ;
			return ;
		}
	}

	// WhiteSpace filtering and bordering
	string clean_text = getBuffer()->spaceHandler(start, text, true, spaceHandling, spaceBordering) ;

	// filter newlines from input text
	for ( guint i =0; i < clean_text.length(); ++i )
		if (clean_text[i] == 0x0D || clean_text[i] == 0x0A)
			clean_text[i] = ' ';

	bool is_edit = getBuffer()->isEditablePosition(start, need_split, true);
	//	TRACE_D << "  at " << iter << " is_edit=" << is_edit  << " needsplit=" << need_split  << endl;

	if ( is_edit ) {

		if ( del_sel ) { // not allowed if selected range contains tagged elems
			Gtk::TextIter iter2 ;
			bool ok;
			for ( iter2 = start; ok && iter2.compare(stop) < 0; ok=iter2.forward_char() ) {
				if ( getBuffer()->iterHasTag(iter2, m_dataModel.getAGTrans(), true) ) {
					get_display()->beep();
					getBuffer()->end_user_action() ;
					return;
				}
			}
		}

		if ( need_split && ! del_sel) { // need to split annotation ?
			int dist = stop.get_offset() - start.get_offset();
			splitTextAnnotation(start, false);
			start=getBuffer()->getCursor();
			stop = start;
			if ( dist > 0 ) stop.forward_chars(dist);
		}
		getBuffer()->replaceText(start, stop, clean_text);
	}
	else
		get_display()->beep();
}

void AnnotationView::onClipboardDataEvent(const Gtk::SelectionData& data)
{
	//TRACE_D << "  IN ON CLIPBOARD DATA EVENT !!! " << endl;
	m_clipboardContents = data.get_text();
}

/* adjust "cut" and "copy" menu state vs current buffer selection */
void AnnotationView::onSelectionSet(bool has_sel)
{
	m_editActions["edit_copy"]->set_sensitive(has_sel);
	m_editActions["edit_cut"]->set_sensitive(has_sel);
}

/*
 *  edit actions
 */
void AnnotationView::onEditAction(string action, bool from_popup)
{
		TRACE << " ON EDIT ACTION = " << action << " from popup=" << from_popup << " focus= " <<has_focus()  << endl;
	if (!from_popup && !has_focus())
		return;
	// CuT
	if ( action == "cut" )	{
		if ( m_editable )
		{
			const string& plain_text = getBuffer()->getSelectedText(false);
			const string& data = getSelectedData();

			Gtk::TextIter start, stop;
			if ( getBuffer()->get_selection_bounds(start,stop) )
			{
				Log::out() << " DELETE SEL " << start << " to " << stop << endl;
				std::vector<Gtk::TextIter> toDel ;
				bool spaceHandling = m_dataModel.conventions().automaticSpaceHandling() ;
				bool spaceBordering = m_dataModel.conventions().spaceBorderingForced() ;
				bool res = getBuffer()->spaceDeleter(start, stop, toDel, spaceHandling, spaceBordering) ;
				if ( res && toDel.size()==2 )
					deleteSelection(toDel[0], toDel[1], false, true) ;
				else
					gdk_beep() ;
			}
			m_refClipBoard->set_text(plain_text) ;
			m_refClipBoardSpecial->set_text(data) ;
		}
	}
	// CoPY
	else if ( action == "copy" ) {
		const string& plain_text = getBuffer()->getSelectedText(false);
		const string& data = getSelectedData();
		m_refClipBoard->set_text(plain_text);
		m_refClipBoardSpecial->set_text(data);
	}
	// PaSTE
	else if (action == "paste" ) {
		if ( m_editable )
			m_refClipBoard->request_text(sigc::bind<bool>(sigc::mem_fun(*this, &AnnotationView::onSelectionPasteEvent), true));
	}
	// PaSTE special
	else if (action == "paste_special" ) {
		if ( m_editable )
			m_refClipBoardSpecial->request_text(sigc::bind<bool>(sigc::mem_fun(*this, &AnnotationView::onSelectionPasteEvent), false));
	}
}

/**
* get selected graph part from datamodel as XML string
*/
string AnnotationView::getSelectedData()
{
	Gtk::TextIter start, stop;
	if ( ! getBuffer()->get_selection_bounds(start,stop) ) return "";

//	string start_id = getBuffer()->getPreviousAnchorId(start, m_mainBaseType);
	string start_id ;
	string stop_id;
	int start_offset, stop_offset;

	getIdAndOffsetFromIterator(start, start_id, start_offset, false);
	Log::out() << " ~> getSelectedData :: start=" << start << " - id=" << start_id << " - offset=" << start_offset << std::endl ;

	getIdAndOffsetFromIterator(stop, stop_id, stop_offset, false);
	Log::out() << " ~> getSelectedData :: end=" << stop << " - id=" << stop_id << " - offset=" << stop_offset << std::endl ;

	if ( start_offset <= 0 )
	{
		// if selection includes annotation label tag -> keep "anchored" anchor, else not
		if ( start.compare(getBuffer()->getAnchoredIter(start_id)) == 0 )
			start_offset = -1;
		else
			start_offset = 0 ;
	}

	try
	{
		DataModel_CPHelper helper(&m_dataModel);
		const string& text = helper.getSubgraphTAGBuffer(start_id, start_offset, stop_id, stop_offset);
		return text;
	}
	catch (AGException& e)
	{
		Log::err() << "copy graph error: " << e.error() << std::endl ;
		return "" ;
	}
}

/*
 * handler for text buffer tag event
 *
 * - set cursor to "active" shape when entering tagged label
 * - popup selection menu when b3 pressed or ctrl-right pressed
 * - open properties dialog when b1 double-clicked on tag
 */
void
AnnotationView::onBufferTagEvent(string tagclass, GdkEvent* event, const Gtk::TextIter& iter)
{
	bool handled = bufferTagEventHandler(tagclass, event, iter);

	switch (event->type )
	{
		case GDK_2BUTTON_PRESS:
			break;
		case GDK_BUTTON_PRESS :
			if ( ! handled ) {
				getBuffer()->clearSelection();
				//Gtk::TextView::on_button_press_event((GdkEventButton*)event);
//				inhibateSynchro(true) ;
				getBuffer()->setCursor(iter, false);
//				inhibateSynchro(false) ;
			}
			break;
		case GDK_BUTTON_RELEASE :
			Gtk::TextView::on_button_release_event((GdkEventButton*)event);
			break;
		case GDK_KEY_PRESS:
			if ( ! handled )
				Gtk::TextView::on_key_press_event((GdkEventKey*)event);
			break;
		default:
			break;
	}
}

// bufferTagEventHandler : return true if event handled, else false.
bool AnnotationView::bufferTagEventHandler(const string& tagclass, GdkEvent* event, const Gtk::TextIter& iter)
{
	bool handled = false;
	m_lastEditTime = time(0);

	switch (event->type )
	{
		case GDK_2BUTTON_PRESS:
		{
			// edit Annotation properties
			if ( ((GdkEventButton*)event)->button == 1 )
			{
				//> Launch speaker dictionary if double click on turn
				if ( tagclass.compare("turn")==0 )
					Glib::signal_idle().connect(sigc::bind<Gtk::TextIter>(sigc::mem_fun(*this, &AnnotationView::editSpeakerPropertiesWhenIdle), iter));
				//> otherwise launch appropriated dialog
				else
					Glib::signal_idle().connect(sigc::bind<Gtk::TextIter, string>(sigc::mem_fun(*this, &AnnotationView::editAnnotationPropertiesWhenIdle), iter, tagclass));
				handled = true ;
			}
			break ;
		}
		case GDK_BUTTON_PRESS :
		{
			if ( ((GdkEventButton*)event)->button == 3 )
			{
				if ( (gdk_event_get_time(event) - m_lastEventTime) >= 1 )
				{
					//			inhibateSynchro(true) ;
					getBuffer()->setCursor(iter, false);
					//			inhibateSynchro(false);
					if ( popupAnnotationMenu(tagclass, iter, (GdkEvent*)event, -1., -1., getBuffer()->getTaggedElementId(iter), true) )
						handled = true;
				}
			}
			break ;
		}
		case GDK_KEY_PRESS:
		{
			switch (((GdkEventKey*)event)->keyval)
			{
				case GDK_Down:
					if ( m_editable && ((GdkEventKey*)event)->state & GDK_CONTROL_MASK)
					{
						if ( popupAnnotationMenu(tagclass, iter, (GdkEvent*)event, -1, -1, getBuffer()->getTaggedElementId(iter)) )
							handled = true;
					}
					break;
				case GDK_Right:
					if ( ((GdkEventKey*)event)->state & GDK_CONTROL_MASK)
					{
						const string& id = getBuffer()->getPreviousAnchorId(iter,"");
						TRACE_D << "at " << iter << " TagClass = " << tagclass << " id = " << id << endl;
					}
				default:
					break;
			}
			break;
		}
		case GDK_MOTION_NOTIFY:
		{
			// change pointer above tag
			gint gx, gy;
			GdkModifierType mask;
			if ( ! m_isActiveCursor )
			{
				get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(m_activeCursor);
				m_isActiveCursor = true;
			}
			gdk_window_get_pointer( get_window(Gtk::TEXT_WINDOW_TEXT )->gobj(), &gx, &gy, &mask);
			handled = true;

			// display tag if needed
			if (tooltip_enabled)
			{
				m_tooltip->launch(*((GdkEventMotion*)event), this) ;
				handled = true;
			}

			break ;
		}
		default:
			break;
	}

	return handled;
}

void AnnotationView::onBufferAnchorMoveEvent(float old_pos,
		const Gtk::TextBuffer::iterator &pos,
		const Glib::RefPtr<Gtk::TextMark>& mark,
		bool forceDisplayUpdate)
{
	string id = mark->get_name() ;
	string type = getDataModel().getElementType(id) ;

	//> Not  a mainstreamBaseType or not time anchored ? exit
	if ( getDataModel().mainstreamBaseType() != type
			|| getDataModel().getElementOffset(id, true)==-1 )
		return ;

	//> For time anchored mainstream base type element,
	//  we display a tag except for first element of segmentBaseType segment.
	//  Therefore, when deleting the first mainstream base type element of a
	//  segmentBaseType element, we need to display its tag.
	if (forceDisplayUpdate && m_dataModel.mainstreamBaseElementHasText(id) == 0)
	{
		Glib::ustring buf ;
		m_renderer[type]->formatLabelStart(buf, true) ;
		// <!> caution: renderer's update method should only update text, so don't use it for tag
		// 				simply insert new element
		getBuffer()->insertTaggedElement(id, id, type, buf, true, getR2LMode(id)) ;
	}
}


//******************************************************************************
//********************************* KEY PROCESS ********************************
//******************************************************************************

/*
 * handler for key-press events
 *  - forbid any char insertion in front of speaker- or segment- tagged text
 *  - popup speaker selection menu when accel key pressed
 */
bool AnnotationView::on_key_press_event(GdkEventKey* event)
{
	if (isDebugMode())
	{
		if (event->keyval==GDK_F2 && event->state & GDK_CONTROL_MASK)
		{
			print_stack(true) ;
			return true;
		}
		else if (event->keyval==GDK_F3 && event->state & GDK_CONTROL_MASK)
		{
			getBuffer()->anchors().dump() ;
			return true;
		}
		else if (event->keyval==GDK_F4 && event->state & GDK_CONTROL_MASK)
		{
			getBuffer()->print_buffer(true, getBuffer()->get_text()) ;
			return true;
		}
	}

	bool handled = false;
	bool ret = false;

	m_lastEditTime = time(0);
	m_tooltip->stop();

	//> -- Try to treat delete actions
	handled = processKeyDeletion(event, ret) ;

	//> -- Try to treat EDITION LOCKED actions
	Gtk::TextIter iter = getBuffer()->getCursor();
	if ( !handled && !iter.editable() )
	{
		const string & tagclass = getBuffer()->getActiveTagClass(iter);
		if ( !tagclass.empty() )
		{
			storePendingTextEdits();
			handled = bufferTagEventHandler(tagclass, (GdkEvent*)event, iter);
		}
	}

	//> -- Try to treat function actions
	if ( !handled )
		handled = processKeyFunc(event, ret) ;

	//> -- Try to treat authorized actions
	if ( !handled )
	{
		// can't insert if non editable view
		if ( ! m_editable )
			get_display()->beep();
		else
			ret = processKeyClassic(event);
	}

	//> Check for pending edits
	if ( m_pendingTextEdits >= MAX_PENDING
				&& !GtUtil::isFunctionKey(gdk_keyval_to_unicode(event->keyval))
				&& !GtUtil::isUndoRedoEventKeys(event))
		storePendingTextEdits();

	return ret ;
}

/*
 * process  keyboard delete events
 */
bool AnnotationView::processKeyDeletion(GdkEventKey* event, bool& ret)
{
	Gtk::TextIter iter = getBuffer()->getCursor();
	bool handled = false ;
	bool todel = false;
	bool spaceHandling = m_dataModel.conventions().automaticSpaceHandling() ;
	bool spaceBordering = m_dataModel.conventions().spaceBorderingForced() ;

	// special case for "delete char" events
	// -> if occurs on (non-editable) annotation tag -> delete annotation
	if ( ! (event->state & GDK_SHIFT_MASK ) )
	{
		Gtk::TextIter iter2 = iter;
		switch ( event->keyval )
		{
			case GDK_BackSpace :
				todel = true;
				if (!iter2.backward_char())
					return false;

				if ( !iter2.editable() )
				{
					while ( getBuffer()->isLabelOnly(iter2) )
						if (!iter2.backward_char())
							break;
				}
				break;
			case GDK_Delete:
			case GDK_KP_Delete:
				todel = true;
				if ( ! iter2.editable() )
				{
					while ( getBuffer()->isLabelOnly(iter2) )
					{
						if (!iter2.forward_char())
							break;
					}
				}
				break;
			default:
				break;
		}

		// handles DELETE EVENTS for one single char
		// selection-range deletes are handled in AnnotationBuffer on_erase method
		if ( m_editable && todel )
		{
			Gtk::TextIter start, stop;
			// no selection
			if ( ! getBuffer()->get_selection_bounds(start, stop) )
			{
				// not allowed to delete
				if ( ! iter2.editable() )
				{
					const string& tagclass = getBuffer()->getActiveTagClass(iter2);
					if ( !tagclass.empty() )
					{
						storePendingTextEdits();
						if ( canDeleteAtIter(iter2, event, tagclass) )
							deleteAnnotation(iter2, tagclass);
					}
				}
				// allowed to delete ? let's check a little more
				else
				{
					// -- Check if we need to merge or delete some annotations with char deletion
					std::vector<string> toBeMergedOrDeleted ;
					Glib::RefPtr<Gtk::TextMark> m1 = getBuffer()->create_mark("tmpDelMark", iter) ;
					Glib::RefPtr<Gtk::TextMark> m2 = getBuffer()->create_mark("tmpDelMark2", iter2) ;
					int res = getBuffer()->canDeletePos(iter2, false, toBeMergedOrDeleted) ;
					bool canDel = (res!=-1) ;
					iter2 = m2->get_iter();
					iter = m1->get_iter() ;
					getBuffer()->delete_mark(m1) ;
					getBuffer()->delete_mark(m2) ;

					//> -- Normal behaviour, just delete
					if (!canDel)
					{
						std::pair<Gtk::TextIter, bool> res;
						if ( iter2 < iter )
							res = getBuffer()->erase_interactive(iter2, iter) ;
						else
						{
							++iter2;
							res = getBuffer()->erase_interactive(iter, iter2) ;
						}
						ret =res.second;
					}

					//> -- We need to delete some annotations
					else
					{
						// -- HACK: keep first anchor position for correct re-insertion
						if ( !toBeMergedOrDeleted.empty() )
						{
							getBuffer()->begin_user_action() ;
							Anchor* a = getBuffer()->anchors().getAnchor(toBeMergedOrDeleted[0]) ;
							if (a)
								getBuffer()->anchors().moveAnchor(a, a->getIter()) ;
						}

						std::vector<Gtk::TextIter> toDel ;
						bool res = getBuffer()->spaceDeleter(iter2, iter2, toDel, spaceHandling, spaceBordering) ;
						if (res)
						{
							if (toDel.size()==2) {
								bool with_timeAnchored = (m_configuration["allow_key_delete"] == "true"
										|| (m_configuration["allow_key_delete"] == "control" && event->state & GDK_CONTROL_MASK) ) ;

								deleteSelection(toDel[0], toDel[1], false, with_timeAnchored) ;
							} else
							{
								std::pair<Gtk::TextIter, bool> res;
								if ( iter2 < iter )
									res = getBuffer()->erase_interactive(iter2, iter) ;
								else
								{
									++iter2;
									res = getBuffer()->erase_interactive(iter, iter2) ;
								}
								ret =res.second;
							}
						}

						// -- Delete or merge
						if ( !toBeMergedOrDeleted.empty())
						{
							if ( toBeMergedOrDeleted.size() == 2 )
								m_dataModel.mergeAnnotations(toBeMergedOrDeleted[0], toBeMergedOrDeleted[1], false, false, true, true) ;
							else if ( toBeMergedOrDeleted.size() == 1 )
							{
								updateDataModel(toBeMergedOrDeleted.back(), false) ;
								m_dataModel.deleteElement(toBeMergedOrDeleted.back() , true) ;
							}
							getBuffer()->end_user_action() ;
						}
					}
				}
			}
			//selection
			else
			{
				std::vector<Gtk::TextIter> toDel ;
				bool res = getBuffer()->spaceDeleter(start, stop, toDel, spaceHandling, spaceBordering) ;
				if (res && toDel.size()==2)
				{
					bool with_timeAnchored = (m_configuration["allow_key_delete"] == "true"
							|| (m_configuration["allow_key_delete"] == "control" && event->state & GDK_CONTROL_MASK) ) ;

					deleteSelection(toDel[0], toDel[1], false, with_timeAnchored) ;
				}
				else
					gdk_beep() ;
			}
			handled = true;
		}
	} //end not shift and control mask
	return handled ;
}

bool AnnotationView::processKeyFunc(GdkEventKey* event, bool& ret)
{
	//> -- No function key ? don't treat
	guint32 keyval = gdk_keyval_to_unicode (event->keyval) ;
	bool is_fun_key = GtUtil::isFunctionKey(event->keyval) ;
	if ( ! is_fun_key )
		return false ;

	Gtk::TextIter iter = getBuffer()->getCursor();
	m_pendingTextEdits = MAX_PENDING;

	//> -- Protect against unwanted shift+return
	bool enter = (event->keyval == GDK_KP_Enter) ;
	bool state = (event->state & GDK_SHIFT_MASK || event->state & GDK_CONTROL_MASK || event->state & GDK_MOD1_MASK) ;
	bool rreturn = event->keyval == GDK_Return ;
	if ( enter || (	state && (rreturn||enter)) )
		return true ;

	//> --  Handle "basic" cursor displacement
	bool forward = true;
	bool do_adjust = false;
	bool downward = true;
	if ( ! (event->state & GDK_SHIFT_MASK || event->state & GDK_CONTROL_MASK) )
	{
		switch ( event->keyval )
		{
			case GDK_End:
				forward = false;
			case GDK_Home:
			case GDK_Page_Up:
			case GDK_KP_Page_Up:
			case GDK_Up:
			case GDK_KP_Up:
				downward = false;
			case GDK_Page_Down:
			case GDK_KP_Page_Down:
			case GDK_Down:
			// always put cursor on "editable" position
			case GDK_KP_Down:
				do_adjust = true;
				break;
			case GDK_Left:
			case GDK_KP_Left:
				downward = false;
				if ( !getR2LMode(m_currentId) )
					forward = false;
				if ( m_configuration["allow_browse_on_tags"] == "false" )
					do_adjust = true;
				break;
			case GDK_Right:
			case GDK_KP_Right:
				if ( getR2LMode(m_currentId) )
					forward = false;
				if ( m_configuration["allow_browse_on_tags"] == "false" )
					do_adjust = true;
				break;
			default:
				m_lastIterOffset.push(iter.get_offset());
		}

		if ( do_adjust )
		{
			ret = Gtk::TextView::on_key_press_event(event);
			Glib::signal_idle().connect(sigc::bind<bool, bool>(sigc::mem_fun(*this, &AnnotationView::adjustCursorWhenIdle), forward, downward));
		}
	}
	// process any other function key -> standard textview behaviour
	else
		ret=Gtk::TextView::on_key_press_event(event);

	return true ;
}

/**
 * keypress events handler
 * @param event keypress event
 */
bool AnnotationView::processKeyClassic(GdkEventKey *event)
{
	bool need_useraction_end = false ;

	Gtk::TextIter it = getBuffer()->getCursor();
	m_lastIterOffset.push(it.get_offset());
	m_pendingTextEdits ++ ;

	guint32 keyval = gdk_keyval_to_unicode (event->keyval) ;
	if ( g_unichar_isspace(keyval) )
		m_pendingTextEdits = MAX_PENDING;

	//Do not process Shift and Ctrl Keys for their uses with arrows
	//in order to enable words selection or navigation
	bool shiftORctrl = ((event->state & GDK_SHIFT_MASK) || (event->state & GDK_CONTROL_MASK)) ;
	bool shiftANDctrl = ((event->state & GDK_SHIFT_MASK) && (event->state & GDK_CONTROL_MASK)) ;

	// -- Do not process remaining function keys
	// UGLY PROTECTION (we shouldn't get there)
	// TODO ensure if we really get there and see why
	bool is_fun_key = false;
	if ( keyval == 0 )
		is_fun_key = GtUtil::isFunctionKey(event->keyval) ;

	if ( event->keyval == GDK_Shift_L
				|| event->keyval == GDK_Shift_R
				|| event->keyval == GDK_Control_L
				|| event->keyval == GDK_Control_R
				|| ( shiftORctrl && event->keyval == GDK_Left)
				|| ( shiftORctrl && event->keyval == GDK_Up)
				|| ( shiftORctrl && event->keyval == GDK_Right)
				|| ( shiftORctrl && event->keyval == GDK_Down)
				|| ( shiftORctrl && event->keyval == GDK_End)
				|| ( shiftORctrl && event->keyval == GDK_Home)
				|| ( (shiftORctrl||shiftANDctrl) && event->keyval == GDK_space)
				|| is_fun_key )
	{
		TRACE_D <<" PROCESS KEY EVENT -> FUN KEY" << endl;
		return Gtk::TextView::on_key_press_event(event);
	}

	//> Do not process accel events
	if (GtUtil::isAccelKeyEvent(event))
		return true;

	// Prepare
	Gtk::TextIter iter, stop ;
	bool spaceHandling = m_dataModel.conventions().automaticSpaceHandling() ;
	bool spaceBordering = m_dataModel.conventions().spaceBorderingForced() ;

	// Eventually delete selected text
	bool has_sel= getBuffer()->get_selection_bounds(iter, stop);
	if ( has_sel )
	{
		if ( ! getBuffer()->isEditableRange(iter, stop) )
		{
			get_display()->beep();
			Log::out() << "processKeyClassic ~~> selection case : editability pb." << std::endl ;
			return false;
		}
		else
		{
			Gtk::TextIter start, stop ;
			getBuffer()->get_selection_bounds(start, stop) ;
			if ( start.editable() )
			{
				std::vector<Gtk::TextIter> toDel ;
				bool res = getBuffer()->spaceDeleter(start, stop, toDel, spaceHandling, spaceBordering) ;
				if (res)
				{
					if (toDel.size()==2)
					{
						bool with_anchored = (m_configuration["allow_key_delete"] == "true" || (m_configuration["allow_key_delete"] == "control" && event->state & GDK_CONTROL_MASK) ) ;

						getBuffer()->begin_user_action();
						need_useraction_end = true ;
						deleteSelection(toDel[0], toDel[1], true, with_anchored) ;
					}
				}
			}
		}
	}
	iter = getBuffer()->getCursor();

	// should it be an "editable" place ?
	bool need_split ;
	if ( iter.is_start() || !getBuffer()->isEditablePosition(iter, need_split, true) )
	{
		get_display()->beep();
		/* protection - sould not happen */
		if (need_useraction_end)
			getBuffer()->end_user_action() ;
		Log::out() << "processKeyClassic ~~~~> non editable." << std::endl ;
		return false;
	}

	bool iteredit = iter.editable();

	// we set current pos to be editable; we will restore it to non editable
	//  state after
	int offset = iter.get_offset();

	if ( !iteredit )
		getBuffer()->setEditable(iter, true);

	if ( need_split )
	{
		// -- Need to split annotation ?
		// In this case, link split action with character insertion
		if (!need_useraction_end)
		{
			getBuffer()->begin_user_action() ;
			need_useraction_end = true ;
		}
		splitTextAnnotation(iter, false);
	}

	bool ret = true;

	//> Check for consecutive whitespace
	//  Block insertion if some detected
	bool filtered = insertionKeySpaceFilter(keyval, iter, spaceHandling, spaceBordering) ;
	if (filtered)
	{
		//> -- Check language mapping
		Glib::ustring text("") ;
		InputLanguage *il = getParent().get_input_language() ;
		if ( il != NULL && il->modifyMapping() )
			il->hasKeyMap(event, text);

		//> -- Check space character needs
		// for classic mapping
		if ( text.empty() )
			text = insertionKeySpaceBorder(event, iter) ;
		// for mapped language
		else
			text = getBuffer()->spaceHandler(iter, text, true, false, spaceBordering) ;

		//> -- No treatment on text, do nothing but gtk behaviour
		if (text.empty())
		{
			int offset = -1 ;
			getBuffer()->setInteractiveInsert(true) ;
			getBuffer()->setLeftGravityTextMode(true) ;
			ret = Gtk::TextView::on_key_press_event(event);
		}
		//> -- Specific treatment, apply
		else
		{
			const std::pair<Gtk::TextIter,bool>& res = getBuffer()->insert_interactive(getBuffer()->getCursor(), text, m_editable) ;
			ret = res.second ;
		}
	}

	if ( !ret )
	{
		get_display()->beep();
		Log::out() << "processKeyClassic ~~~~> filter failure" << std::endl ;
	}

	if ( !iteredit )
		getBuffer()->setEditable(getBuffer()->get_iter_at_offset(offset), false);

	if ( need_useraction_end )
		getBuffer()->end_user_action() ;

	return ret;
}


bool AnnotationView::canDeleteAtIter(const Gtk::TextIter& iter, GdkEventKey* event, const string& tagclass)
{
	bool allowed = m_configuration["allow_key_delete"] == "true" ;
	if (allowed)
		return true ;

	bool specialDel = ( m_configuration["allow_key_delete"] == "control" && event->state & GDK_CONTROL_MASK ) ;
	if (specialDel)
		return true ;

	//TODO we have to differenciate anchored events to not-anchored events
	bool anchored = ( m_dataModel.conventions().isAnchoredType(tagclass) || m_dataModel.isMainstreamType(tagclass) ) ;
	if (!anchored)
		return true ;

	return false ;
}


//******************************************************************************
//******************************* INPUT LANGUAGES ******************************
//******************************************************************************

bool AnnotationView::getR2LMode(const string& id)
{
	const string& segLan = getDataModel().getSegmentLanguage(id);
	InputLanguage* il = InputLanguageHandler::get_input_language_by_shortcut(segLan);
	return (il && !il->ModeLeft2Right()) ;
}

/**
 * set current input language for turn corresponding to given text iterator
 * @param iter current iter position
 * @param force if true force set language, else is set only if current turn different
 *     of previous turn took into account
 */
void AnnotationView::setCurrentInputLanguage(const Gtk::TextIter& iter, bool force)
{
	const string& id = getBuffer()->getPreviousAnchorId(iter, "turn", true);
	setCurrentInputLanguage(id, force);
}

/**
 * set current input language for given turn id
 * @param id turn id
 * @param force if true force set language, else is set only if current turn different
 *     of previous turn took into account
 */
void AnnotationView::setCurrentInputLanguage(const string& id, bool force)
{
	if ( force || id != m_currentId ) {
		InputLanguage* il = NULL;
		if ( ! id.empty() ) {
			const string& language = m_dataModel.getSegmentLanguage(id) ;
			il = InputLanguageHandler::get_input_language_by_shortcut(language) ;
		}
		//		TRACE_D << " set input language for " << id << " il = " << (il==NULL? "" : " not null") << endl;
		if (il==NULL)
			il = InputLanguageHandler::get_input_language_by_shortcut(DEFAULT_LANGUAGE) ;
		getParent().set_input_language(il) ;
		m_currentId = id;
	}
}

//******************************************************************************
//******************************* INPUT FORMATTING *****************************
//******************************************************************************

bool AnnotationView::insertionKeySpaceFilter(gunichar keyval, const Gtk::TextIter& iter, bool spaceHandling, bool spaceBordering)
{
	if (!spaceHandling && !spaceBordering)
		return true ;

	Gtk::TextIter previous = iter ;
	previous.backward_char() ;

	//> FOR SPACE BORDERS
	if (spaceBordering)
	{
		// Don't allow insertion between a non-label space and a border-needing char
		if ( ( (previous.get_char()== ' ' && !getBuffer()->iterHasTag(previous, "label", false))
				&& m_dataModel.conventions().needSpaceBorders(iter.get_char()) )
			|| ( (iter.get_char()== ' ' && !getBuffer()->iterHasTag(iter, "label", false))
						&& m_dataModel.conventions().needSpaceBorders(previous.get_char()) )
			)
		{
			gdk_beep() ;
			Log::out() << "insertionKeySpaceFilter ~~> space bordering pb." << std::endl ;
			return false ;
		}
	}
	//> FoR SPACE INSERTION CASE
	if (spaceHandling && getBuffer()->isSpaceChar(keyval))
	{
		//> If previous is a non-label space, or iter is a non-label space,
		//  don't allow to insert space
		if ( (previous.get_char()== ' ' &&  !getBuffer()->iterHasTag(previous, "label", false))
				|| (iter.get_char()== ' '&&  !getBuffer()->iterHasTag(previous, "label", false)) )
		{
			gdk_beep() ;
			Log::out() << "insertionKeySpaceFilter ~~> space bordering pb." << std::endl ;
			return false ;
		}
	}
	return true ;
}

Glib::ustring AnnotationView::insertionKeySpaceBorder(GdkEventKey* event, const Gtk::TextIter& iter)
{
	bool spaceBordering = m_dataModel.conventions().spaceBorderingForced() ;
	if (!spaceBordering)
		return "" ;

	Gtk::TextIter previous = iter ;
	previous.backward_char() ;
	Glib::ustring res = "" ;
	bool modified  = false ;

	guint32 keyval = gdk_keyval_to_unicode (event->keyval) ;

	//> For spaceBordering allowed, let's place spaces before and after
	//  convention defined character
	if (m_dataModel.conventions().needSpaceBorders(keyval))
	{
		// If previous is not a space, or is a labeled space, insert space
		if ( previous.get_char()!= ' '
			|| (previous.get_char()== ' ' && getBuffer()->iterHasTag(previous, "label", false) )
		   )
		{
			res.append(" ") ;
			modified = true ;
		}
		res.append(Glib::ustring(1,keyval)) ;
		// Same here with iter, insert space if needed
		if ( iter.get_char()!= ' '
			|| (iter.get_char()== ' ' && getBuffer()->iterHasTag(previous, "label", false) )
		   )
		{
			res.append(" ") ;
			modified = true ;
		}
	}
	if (modified)
		return res ;
	else
		return "" ;
}

//******************************************************************************
//**************************** EXTERNAL INPUT MODULE ***************************
//******************************************************************************

//** SCIM
/**
 * check if text insertion allowed at current cursor position; if yes, and is not editable,
 * make it editable so that SCIM/IME commit-text will work
 */
void AnnotationView::on_preedit_start()
{
	bool need_split;
	Gtk::TextIter iter = getBuffer()->getCursor();
	bool edit = getBuffer()->isEditablePosition(iter, need_split, true);

	// TRACE << "on_preedit_start : is editable POS :" << iter << " | edit = " << edit << endl ;

	m_need_restore_edit_state = false;
	if ( edit ) {
		if ( need_split ) {
			// need to split annotation ?
			TRACE_D << "need to split annotation ? at " << iter <<  endl;
			splitTextAnnotation(iter, false);
			getBuffer()->insertText(" ");
		} else {
			bool iteredit=iter.editable();
			iter.backward_char();
			if ( !(iter.editable() || iteredit) ) {
				getBuffer()->setEditable(getBuffer()->getCursor(), true);
				iter = getBuffer()->getCursor();
				TRACE_D << "SET EDITABLE AT " << iter << " char="<< iter.get_char() << "--" << endl ;
				m_need_restore_edit_state = true;
			}
		}
	}
	//> in non editable position, can't tell SCIM not to display
	// the current preedit, even if it won't be added to buffer
	// Just tell the user it's not a correct action
	else
	{
		gdk_beep();
		Log::out() << "pre_edit_start ~~> non editable" << std::endl ;
	}
}


void AnnotationView::on_preedit_changed()
{
	bool need_split;
	const Gtk::TextIter& iter = getBuffer()->getCursor();
	bool edit = getBuffer()->isEditablePosition(iter, need_split, true);

	// TRACE << "on_preedit_changed : is editable POS :" << iter << " | edit = " << edit << endl ;

	//> in non editable position, can't tell SCIM not to display
	// the current preedit, even if it won't be added to buffer
	// Just tell the user it's not a correct action

	if ( !edit && getParent().getLoaded())
	{
		gdk_beep();
		Log::out() << "pre_edit_changed ~~> non editable" << std::endl ;
	}
}

/**
 * after SCIM/IME commit-text has occured or was canceled by user, restore editability state
 *  at current cursor position
 */
void AnnotationView::on_preedit_end()
{
	if ( m_need_restore_edit_state ) {
//		Gtk::TextIter iter = getBuffer()->getCursor();
//		TRACE_D << "RESET NOT EDITABLE AT " << iter << " char="<< iter.get_char() << "--" << endl ;
		getBuffer()->setEditable(getBuffer()->getCursor(), false);
	}
}

/**
 * switch to next input language mode
 */
void AnnotationView::onNextInputLanguage()
{
	m_signalLanguageChange.emit(1);
}
/**
 * switch to previous input language mode
 */
void AnnotationView::onPreviousInputLanguage()
{
	m_signalLanguageChange.emit(-1);
}
//**LG3

/**
 * Split current text annotation
 * @param iter current text iterator
 * @param reattach if false, segment qualifiers will remain attached to previous segment end, new text insertion will occur after those qualifiers.
 * @param  dont_split_at_end if true and split would occur at text end, then don't split and return next element id instead
 * @return new text annotation id
 * @note
 *  when inserting chars at segment end just after a qualifier end tag,
 *   if next text anchor doesn't immediately follows the qualifier
 *   then there is need to split current segment to support newly inserted text
 */

string AnnotationView::splitTextAnnotation(const Gtk::TextIter& iter, bool reattach, bool dont_split_at_end)
{
	//setUndoableActions(false);
	TRACE << "(AnnotationView::splitTextAnnotation)  iter=" << iter << " dont_split_at_end=" << dont_split_at_end << endl;
	const string& type = m_mainBaseType;
	string prevId = getBuffer()->getPreviousAnchorId(iter, type);
	const Gtk::TextIter& prevIter = getBuffer()->getAnchoredIter(prevId);
	bool first_child = false;

	if ( iter == prevIter )
	{
		// if not first child -> go to previous element
		first_child = m_dataModel.isFirstChild(prevId);
		if ( ! first_child )
			prevId = m_dataModel.getPreviousElementId(prevId);
	}

	long text_offset = getBuffer()->getOffsetInTextSegment(prevId, iter, true );
	if ( dont_split_at_end )
	{
		//> -- Get the iter positition relative to the nearest base segment
		Glib::ustring txt = getBuffer()->getSegmentText(prevId, "", false, true);
		//> -- 	Compare position with the length of the complete segment text:
		//		if we're located after the complete text, let's use the next segment base
		//		without splitting
		if ( text_offset >= txt.length() )
		{
			return 	m_dataModel.getNextElementId(prevId);
		}
	}

	getBuffer()->setCursor(iter, false);
	getBuffer()->begin_user_action() ;

	//> -- Split
	string id;
	getBuffer()->backupInsertPosition();
	id = m_dataModel.splitTextMainstreamElement(prevId, text_offset, -1, reattach, true);
	getBuffer()->restoreInsertPosition(0, false);
	getBuffer()->end_user_action() ;
	//TRACE_D << "splitTextAnnotation at " << iter << " new id = " << id << endl;
	return id;
}



/**
 * delete selected data between start and stop items
 * @param start selection start iterator
 * @param stop selection end iterator
 */
void AnnotationView::deleteSelection(const Gtk::TextIter& start, const Gtk::TextIter& stop, bool keep_end_anchor, bool with_timeAnchored)
{
	getBuffer()->begin_user_action() ;

	storePendingTextEdits(false, true);

	//> Keep text position
	Glib::RefPtr<Gtk::TextMark> mark_start = getBuffer()->create_mark("deletesel_start", start, false) ;
	Glib::RefPtr<Gtk::TextMark> mark_end = getBuffer()->create_mark("deletesel_stop", stop, false) ;

	//> Warning: always delete in reverse order for correct delete graph behaviour

	//> 1 : qualifiers
	Log::out() << "\n1###4 DeleteSelection: deleting qualifiers" << std::endl ;
	const vector<string>& todel = getBuffer()->getIds(start, stop, m_dataModel.getAGTrans(), with_timeAnchored);
	vector<string>::const_reverse_iterator itd;
	bool deleted ;
	for (itd = todel.rbegin(); itd != todel.rend(); ++itd)
	{
		if (m_dataModel.isQualifierType(m_dataModel.getElementType(*itd)))
		{
			Log::out() << "\n~~deleteSelection: qualifier step: deleting " << *itd << std::endl ;
			deleted = m_dataModel.deleteElement(*itd, true);
			Log::out() << "done=" << deleted << std::endl ;
		}
	}

	//> 2 : Annotations
	Log::out() << "\n2###4 DeleteSelection: deleting annotations" << std::endl ;
	const vector<string>& todel3 = getBuffer()->getIds(mark_start->get_iter(), mark_end->get_iter(), m_dataModel.getAGTrans(), with_timeAnchored);
	vector<string>::const_reverse_iterator itd2;
	deleted = false ;
	for (itd2 = todel3.rbegin(); itd2 != todel3.rend(); ++itd2)
	{
		Log::out() << "\n~~deleteSelection: deleting " << *itd2 << std::endl ;
		deleted = m_dataModel.deleteElement(*itd2, true);
		Log::out() << "done=" << deleted << std::endl ;
	}

	/*
	 * In default behaviour, Datamodel won't delete unit with text data.
	 * ==> <UGLY> in this case, if all text is in selection, force deletion
	 */
	Log::out() << "3###4 DeleteSelection: deleting unit & text" << std::endl ;
	const vector<string>& todel2 = getBuffer()->getIds(mark_start->get_iter(), mark_end->get_iter(), m_dataModel.getAGTrans(), with_timeAnchored);
	if (todel2.size() == 1)
	{
		const string& id = todel2.back() ;
		Log::out() << "-- delete candidate " << id << std::endl ;
		const string& next = getBuffer()->getNextAnchorId(id, "") ;
		Gtk::TextIter it = getBuffer()->getAnchoredIter(next) ;
		it = getBuffer()->previousEditablePosition(it) ;
		if (it <= mark_end->get_iter())
		{
			Log::out() << "----> ok for delete" << std::endl ;
			m_dataModel.deleteElement(id, true, true);
		}
	}

	//> Now we can delete text content

	/*
	 * Specific case :
	 * When selecting all text between the first unit of segment and the followong unit,
	 * when erasing selection the 2 anchors get superposed
	 * => if the deletion is made by replacing selection by another text, problem !!
	 *
	 * <UGLY HACK>: add a white space before deleting text for avoiding superposition
*/
	std::vector<Anchor*> borders = getBuffer()->needBlankBeforeDeleteSelection(mark_start->get_iter(), mark_end->get_iter()) ;
	if (borders.size()==2)
	{
		if ( m_dataModel.isFirstChild(borders[0]->getId(), "") )
			getBuffer()->insert(borders[0]->getIter(), " ") ;
			//TODO keep mark and remove char after treatment
			//caution: treatment can be done by on_key_press_event so no control until afterInsert callback
			//			it.backward_char() ;
			//			Glib::RefPtr<Gtk::TextMark> blankmark = getBuffer()->create_mark("blankmark", it, false) ;
	}

	const string& id = getBuffer()->getAnchorIdByType(mark_start->get_iter(), m_mainBaseType, false);
	getBuffer()->select_range(mark_start->get_iter(), mark_end->get_iter()) ;
	getBuffer()->erase_selection(true, true) ;
	updateDataModel(id, false);

	//> Clean
	getBuffer()->delete_mark(mark_start) ;
	getBuffer()->delete_mark(mark_end) ;

	getBuffer()->end_user_action() ;
	Log::out() << "4###4 DeleteSelection: done.\n" << std::endl ;
}


/*
 *  adjust cursor to editable position
 */
bool AnnotationView::adjustCursorWhenIdle(bool forward, bool downward)
{
	//MSGOUT << " IN THREAD " << endl;
	gdk_threads_enter();
	//MSGOUT << " IN THREAD 2 " << endl;
	getBuffer()->moveCursorToEditablePosition(forward, downward,
				( m_configuration["allow_browse_on_tags"] == "true"), true);

	gdk_flush();
	gdk_threads_leave();
	//MSGOUT << " OUT THREAD " << endl;
	return false;
}


/*
 * handler for button press event :
 *  set active time, emit "setCursor" event
 */
bool AnnotationView::on_button_press_event(GdkEventButton* event)
{
	m_tooltip->stop() ;

	storePendingTextEdits();
	if ( event->button == 1 ) {
		int x, y;
		Gtk::TextIter iter;
		window_to_buffer_coords (Gtk::TEXT_WINDOW_WIDGET, (gint)event->x, (gint)event->y, x, y);
		get_iter_at_location (iter, x, y);
		// if iter not editable, but isEditablePosition, then add blank editable char at iter
		// (in order to allow SCIM inter
		getBuffer()->clearSelection();
		emitSetCursor(iter, false);
	}
	//block wheel click
	if ( event->button == 2 )
		return true ;
	else
		return Gtk::TextView::on_button_press_event(event);
}

/*
 *  In a stereo file, when clicking in a view, focus must be set to the
 *  clicked view.
 *  BUT
 *  when parent had no focus and receives it, the focus must be set to the
 *  last active view.
 *
 *  GTK PROBLEM: in stereo mode, parent is container with 2 views (2 children).
 *  when parent receives focus, focus is automatically given to focus child,
 *  and we don't control which one receives it.
 *
 *  ==> we test validity for allowing or not the focus in child callback
 *
 *  If parent have first focus stamp, it means that we need to restore
 *  the last position of the last active view, so test the last active view
 *  value before giving focus.
 *  Otherwise, always take focus.
 */
bool AnnotationView::on_focus_in_event(GdkEventFocus* event)
{
	if (!focus_in_locked &&
			(!getParent().getFirstFocus() || getParent().getActiveViewTrack()==m_viewTrack) )
	{
		// TODO Futur -> gerer le switch de langue entre 2 vues
		//		setCurrentInputLanguage(getBuffer()->getCursor(), true);
		m_signalHasFocus.emit();
		externalIMEcontrol(IME_on) ;
	}
	getParent().setFirstFocus(false) ;

	return Gtk::TextView::on_focus_in_event(event);
}

/*
 * focus_out event -> store pending edits
 */
bool AnnotationView::on_focus_out_event(GdkEventFocus* event)
{
	//TRACE << " IN on_focus_out_event   viewtrack = " << getViewTrack() << endl;
	storePendingTextEdits();
	m_tooltip->stop();
	return Gtk::TextView::on_focus_out_event(event);
}

void AnnotationView::setFocus(bool protectSignal)
{
	if (protectSignal)
		focus_in_locked = true ;
	grab_focus() ;
	if (protectSignal)
		focus_in_locked = false ;
}

/*
 * on_motion_notify_ event -> set cursor on active tags
 */
bool AnnotationView::on_motion_notify_event(GdkEventMotion* event)
{
	// Such protection was done while trying to resolve
	// set_pixels_above_lines Gdk bug when tags are hidden
	// Doesn't seem necessary with solution found.
/*	if (getParent().isTagHiddenMode())
		return true ;	*/

	if (!getParent().getLoaded())
		return true ;

	// Hide tooltip
	m_tooltip->stop() ;

	// Reset cursor state
	if ( m_isActiveCursor )
	{
		get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(m_textCursor);
		m_isActiveCursor = false;
	}

	// Exit if not debug
	if ( !getParent().isDebugMode() )
		return Gtk::TextView::on_motion_notify_event(event) ;
	// Show marks info for debug
	else if (tooltip_enabled && (event->state & GDK_CONTROL_MASK) )
	{
		m_tooltip->launch(*event, this) ;
		return true ;
	}
	return true ;
}

//  add annotation menu to std text popup menu
//  ( and remove unwanted items )
void  AnnotationView::on_populate_popup(Gtk::Menu* menu)
{
	Gtk::TextIter iter;
	int winx, winy, x, y;

	//> -- Non editable ? Prepare simple menu
	if (!m_editable)
	{
		menu->items().clear();

		string action = "copy";
		string act = "edit_" + action;
		Gtk::Image* img = Gtk::manage(new Gtk::Image(Gtk::StockID(action), Gtk::ICON_SIZE_MENU));

		menu->items().push_back(ImageMenuElem(m_editActions[act]->property_label().get_value(), *img,
					sigc::bind<string, bool>(sigc::mem_fun(*this, &AnnotationView::onEditAction), action, true)));
		menu->items().back().set_sensitive(m_editActions[act]->get_sensitive());
		return;
	}

	menu->items().clear();

/* SPELL */
//	if ( m_speller != NULL )
//		gtkspell_populate_popup(m_speller, this->gobj(), menu->gobj(), _("Spelling suggestions")) ;

	get_pointer(winx, winy);
	window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, winx, winy, x, y);
	get_iter_at_location(iter, x, y);

	bool need_split;
	Gtk::MenuItem *mitem;

	//> Take insert cursor pos into account
	if ( getBuffer()->isEditablePosition(iter, need_split) )
		iter = getBuffer()->getCursor();

	//> For Editable Positions, add some menu items : event & entities
	if ( getBuffer()->isEditablePosition(iter, need_split) )
	{
		iter = getBuffer()->getCursor();

		// -- add event & entities options
		fillAnnotationMenu(menu, iter, getBuffer()->hasSelection(), false) ;

		// -- add submenu for wordlist
		const list<WordList>& wordlists = m_dataModel.conventions().getPredefinedWordlists();
		if ( ! wordlists.empty() )
		{
			// add predefined words lists submenu
			menu->items().push_back(SeparatorElem());
			list<WordList>::const_iterator itl;
			Gtk::Menu* theMenu = menu;
			set<string>::const_iterator itw;
			if ( wordlists.size() > 1 )
			{
				mitem = Gtk::manage(new Gtk::MenuItem(_("Predefined _words"), true));
				theMenu = Gtk::manage(new class Gtk::Menu());
				mitem->set_submenu(*theMenu);
				menu->append(*mitem);
				mitem->show();
			}
			for ( itl = wordlists.begin(); itl != wordlists.end(); ++itl )
			{
				mitem = Gtk::manage(new Gtk::MenuItem(itl->getLabel(), true));
				Gtk::Menu* submenu = Gtk::manage(new class Gtk::Menu());
				for ( itw = itl->getWords().begin(); itw != itl->getWords().end(); ++itw )
				{
					submenu->items().push_back(MenuElem(*itw,
								sigc::bind<Gtk::TextIter, string>(sigc::mem_fun(*this, &AnnotationView::insertPredefinedWord), iter, *itw)));
				}
				mitem->set_submenu(*submenu);
				theMenu->append(*mitem);
				mitem->show();
			}
		}
		menu->items().push_back(SeparatorElem());
	}

	// -- Add edit actions entries
	const char* actions[] = { "cut", "copy", "paste", "paste_special", NULL};
	for ( int i=0; actions[i] != NULL; ++i )
	{
		string act = "edit_";
		act += actions[i];
		/*
		mitem = m_editActions[act]->create_menu_item();
		mitem->show();
		menu->append(*mitem);
		mitem->set_sensitive(m_editActions[act]->get_sensitive());
		 */
		Gtk::Image* img = Gtk::manage(new Gtk::Image(Gtk::StockID(actions[i]), Gtk::ICON_SIZE_MENU));
		menu->items().push_back(ImageMenuElem(m_editActions[act]->property_label().get_value(), *img, sigc::bind<string, bool>(sigc::mem_fun(*this, &AnnotationView::onEditAction), actions[i], true)));
		menu->items().back().set_sensitive(m_editActions[act]->get_sensitive());
	}

	/* DEPRECATED: display input method menu for switching input method environment
	GtkWidget* menuitem = gtk_menu_item_new_with_mnemonic ("Input _Methods");
	gtk_widget_show(menuitem);
	gtk_widget_set_sensitive (menuitem, true);

	GtkWidget* submenu = gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(menuitem), submenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(this->gobj()->popup_menu), menuitem);

	gtk_im_multicontext_append_menuitems (GTK_IM_MULTICONTEXT(this->gobj()->im_context),
						GTK_MENU_SHELL(submenu));
	 */

	UndoableTextView::onPopulatePopup(menu);
}

void AnnotationView::fillAnnotationMenu(Gtk::Menu* menu, const Gtk::TextIter& iter, bool selectedText, bool only_events)
{
	if (!menu)
		return ;

	Gtk::MenuItem* mitem = NULL ;
	bool canInsertQual = getBuffer()->canInsertQualifier(iter) ;

	//-- add submenu for events
	AnnotationRenderer* renderer = getRenderer("unit") ;
	if ( renderer )
	{
		string label = _("New _foreground event") ;
		// ICI -> necessité de dupliquer le submenu !!
		// add new event submenu
		mitem = Gtk::manage(new Gtk::MenuItem(label.c_str(), true));
		UnitMenu* umenu = (UnitMenu*) renderer->duplicateMenu() ;
		if ( umenu )
		{
			umenu->signalSetUnit().connect( sigc::mem_fun (*this, &AnnotationView::setForegroundEvent) ) ;
			umenu->setIter(iter) ;
			mitem->set_submenu(*umenu) ;
			menu->append(*mitem) ;
			mitem->show() ;

			if ( selectedText )
				mitem->set_sensitive(false) ;
		}
		else
			Log::err() << "FAIL --> adding qualifier menu for foreground events into standard popup: failed. Aborted." << std::endl ;
	}

	// -- add submenu for events
	renderer = getRenderer("qualifier_event") ;
	if ( renderer )
	{
		string label = _("New annotation _event") ;
		// ICI -> necessité de dupliquer le submenu !!
		// add new event submenu
		mitem = Gtk::manage(new Gtk::MenuItem(label.c_str(), true)) ;
		QualifiersMenu* qmenu = (QualifiersMenu*) renderer->duplicateMenu() ;
		if ( qmenu )
		{
			qmenu->signalSetQualifier().connect(sigc::bind<string>(sigc::mem_fun (*this, &AnnotationView::setQualifier), "qualifier_event"));
			qmenu->setIter(iter);
			mitem->set_submenu(*qmenu);
			menu->append(*mitem);
			mitem->show();

			if ( !canInsertQual )
				mitem->set_sensitive(false) ;
		}
		else
			Log::err() << "FAIL --> adding qualifier menu for event into standard popup: failed. Aborted." << std::endl ;
	}

	if ( only_events )
	{
		menu->select_first(false) ;
		return ;
	}

	// -- add submenu for entities
	renderer = getRenderer("qualifier_entity") ;
	if ( renderer )
	{
		string label = _("New named _entity") ;
		// ICI -> necessité de dupliquer le submenu !!
		// add new event submenu
		mitem = Gtk::manage(new Gtk::MenuItem(label.c_str(), true));
		QualifiersMenu* qmenu = (QualifiersMenu*) renderer->duplicateMenu() ;
		if ( qmenu )
		{
			qmenu->signalSetQualifier().connect(sigc::bind<string>(sigc::mem_fun (*this, &AnnotationView::setQualifier), "qualifier_entity"));
			qmenu->setIter(iter);
			mitem->set_submenu(*qmenu);
			menu->append(*mitem);
			mitem->show();

			if ( !canInsertQual )
				mitem->set_sensitive(false) ;
		}
		else
			Log::err() << "FAIL --> adding qualifier menu for entities into standard popup: failed. Aborted." << std::endl ;
	}

	menu->select_first(false) ;
}

//***
void AnnotationView::updateAlignment(string id)
{
	getBuffer()->updateAlignment(id, getR2LMode(id)) ;
	setCurrentInputLanguage(id, true) ;
}

char* AnnotationView::ptr_callback(const char* s, void* data)
{
	return ((InputLanguageArabic*)data)->remove_vowel_and_nocheck(s) ;
}
//***LG3

void AnnotationView::on_hide()
{
//	m_tooltip.hide();
//	m_tooltip.reset_timer();
}

bool AnnotationView::isStereo()
{
	return m_parent.isStereo() ;
}


//******************************************************************************
//**************************** UNDO REDO BEHAVIOUR *****************************
//******************************************************************************

void AnnotationView::displayUndoRedoError(bool undo)
{
	Glib::ustring txt ;
	if (!undo)
		txt = _("Error while redoing action.") ;
	else
		txt = _("Error while undoing action.") ;

	#ifdef __APPLE__
	dlg::error(txt, m_parent.getTopWindow()) ;
	#else
	dlg::error(txt, NULL) ;
	#endif
}

void AnnotationView::onUndoAllDone()
{
//	m_dataModel.setUpdated(false) ;
}


//******************************************************************************
//******************************* Input behaviours *****************************
//******************************************************************************

void AnnotationView::externalIMEcontrol(bool activate)
{
	Glib::signal_idle().connect(sigc::bind<bool>(sigc::mem_fun(*this, &AnnotationView::externalIMEcontrol_afterIdle), activate)) ;
}

bool AnnotationView::externalIMEcontrol_afterIdle(bool activate)
{
	focus_in_locked = true ;
	grab_focus();
	focus_in_locked = false ;
	GtkIMContext* context = this->gobj()->im_context ;
	GdkWindow* window = gtk_text_view_get_window(this->gobj(), GTK_TEXT_WINDOW_WIDGET) ;
	InputLanguageHandler::activate_external_IME(context, window, activate) ;
	return false ;
}

void AnnotationView::insertPredefinedWord(Gtk::TextIter iter, string word)
{
	getBuffer()->setCursor(iter);
	getBuffer()->insertWord(word);
}
/* SPELL */
//void AnnotationView::speller_recheck_all()
//{
//	if (m_speller)
//		gtkspell_recheck_all(m_speller) ;
//}

void AnnotationView::set_entityTag_bg(bool use_bg)
{
	if ( m_renderer.find("qualifier_entity") != m_renderer.end()) {
		EntityRenderer* er = (EntityRenderer*)&(m_renderer["qualifier_entity"]);
		er->set_use_bg(use_bg) ;
	}
}


//******************************************************************************
//								Sexy Behaviours
//******************************************************************************

void AnnotationView::setWithConfidence(bool value, bool actualizeSpeller)
{
	m_withConfidence = value ;
/* SPELL */
//	if (actualizeSpeller)
//		m_buffer->setSpellerConfidence(value) ;
	Log::out() << "+++++++++++++++++ High confidence mode [" << value <<"]" << endl;
}

bool AnnotationView::getWithConfidence() const
{
	return m_withConfidence ;
}

void AnnotationView::setFontStyle(const string& font, const string& mode)
{
	if (mode.compare("text")==0)
	{
		//> modify simple text
        Log::trace() << "SetFontStyle --> Font Set To : " << font.c_str() << std::endl ;
        modify_font(Pango::FontDescription(font));
    }

	//> tell buffer to change labels
    getBuffer()->setFontStyle(font, mode) ;
}

void AnnotationView::resetColors()
{
	const string& fg = getColorsCfgOption("Colors-editor,text_fg") ;
	const string& bg = getColorsCfgOption("Colors-editor,text_bg") ;
	setTextColor(fg, bg) ;
}

void AnnotationView::setTextColor(string fg, string  bg)
{
	if (fg.empty())
		fg="#000000" ;
	if (bg.empty())
		bg="#FFFFFF" ;
	modify_base(Gtk::STATE_NORMAL, Gdk::Color(bg)) ;
	modify_text(Gtk::STATE_NORMAL, Gdk::Color(fg)) ;
}

Glib::ustring AnnotationView::getBaseViewColor()
{
	Glib::ustring color ;

	//> try to get current color
	Glib::RefPtr<Gtk::Style> style = get_style() ;
	if (style) {
		Gdk::Color c = style->get_base(Gtk::STATE_NORMAL) ;
		std::vector<Gdk::Color> v ;
		v.push_back(c) ;
		color = Gtk::ColorSelection::palette_to_string(v) ;
	}
	//> if color recuperation fails, let's white by default
	else {
		color = "#ffffff" ;
	}
	return color ;
}

/*
 * 1: all
 * 2: events
 * 3: entities
 * 4: unknown qual
 * 5: unit
 * other value : show
 */
void AnnotationView::hideTags(int hide_mode)
{
	bool hide_events = false ;
	bool hide_entity = false ;
	bool hide_unk = false ;
	bool hide_unit = false ;

	std::map<std::string, AnnotationRenderer*>::iterator it ;

	// deals with event
	if (hide_mode==1 || hide_mode==2)
		hide_events = true ;

	// deals with named entities
	if (hide_mode==1 || hide_mode==3)
		hide_entity = true ;

	// deals with unknown (invalid) qualifiers
	if (hide_mode==1 || hide_mode==4)
		hide_unk = true ;

	// deals with time anchored unit
	if (hide_mode==1 || hide_mode==5)
		hide_unit = true ;

	// apply for all

	it = m_renderer.find("qualifier_event") ;
	if (it!=m_renderer.end() && it->second)
		it->second->hideTag(hide_events) ;

	it = m_renderer.find("qualifier_entity") ;
	if (it!=m_renderer.end() && it->second)
		it->second->hideTag(hide_entity) ;

	it = m_renderer.find("qualifier_unknown") ;
	if (it!=m_renderer.end() && it->second)
		it->second->hideTag(hide_unk) ;

	it = m_renderer.find("unit") ;
	if (it!=m_renderer.end() && it->second)
		it->second->hideTag(hide_unit) ;
}

void AnnotationView::get_pixels_interline(int& above, int& below)
{
	above = get_pixels_above_lines() ;
	below = get_pixels_below_lines() ;
}

void AnnotationView::set_pixels_interline(int above, int below)
{
	set_pixels_above_lines(above) ;
	set_pixels_below_lines(below) ;
}

const string& AnnotationView::getTextCursorElement()
{
	return getBuffer()->anchors().getPreviousAnchorId(getBuffer()->getCursor(), "") ;
}

void AnnotationView::getIdAndOffsetFromIterator(Gtk::TextIter& pos, string& prevId, int& text_offset, bool basetype_only)
{
	prevId = ""; text_offset = 0;
	if (!basetype_only)
	{
		// check if Anchor exists at current cursor position
		// first skip eventual non-editable spaces
		while (getBuffer()->isLabelOnly(pos))
			if (!pos.forward_char())
				break;
		const std::vector<Anchor*> v = getBuffer()->anchors().getAnchorsAtPos(pos, "", false);
		if (v.size() > 0)
		{
			std::vector<Anchor*>::const_iterator ita;
			Anchor* highest = NULL;
			for (ita = v.begin(); ita != v.end(); ++ita)
			{
				if (highest == NULL)
					highest = *ita;
				else
				{
					if (m_dataModel.conventions().isHigherPrecedence((*ita)->getType(), highest->getType()))
						highest = *ita;
				}
			}
			prevId = highest->getId();
			return;
		}
	}

	// no anchor at current text position -> get previous anchor
	prevId = getBuffer()->getPreviousAnchorId(pos, m_mainBaseType);

	// get text offset in current segment
	if (!prevId.empty())
	{
		Anchor* prev_anchor = getBuffer()->anchors().getAnchor(prevId);
		const Gtk::TextIter& prev_iter = prev_anchor->getIter() ;
		// the buffer cursor is not a current postion: should we compute text offset ?
		if (!pos.compare(prev_iter) == 0)
		{
			// the buffer cursor is at same line than anchor we found, compute text offset
			if (pos.get_line() == prev_iter.get_line())
				text_offset = getBuffer()->getOffsetInTextSegment(prevId, pos, true);
			// not the same line ? go one step forward
			else
			{
				prevId = getBuffer()->getNextAnchorId(prev_iter, m_mainBaseType);
				if (!prevId.empty())
					text_offset = -1 ;
			}
		}
	}
	else
	{
		// then we must be at graph start -> get 1st segment ??
	}
}

//******************************************************************************
//									Drag'N'Drop
//******************************************************************************

void AnnotationView::addDragAndDropTarget(Gtk::TargetEntry target)
{
	/* list of targetEntry to define drag and source identity */
	dragDropTargets.push_back(target) ;
	drag_dest_set(dragDropTargets,Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
}

// relay drag_drop signal to parent
bool AnnotationView::on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time)
{
	m_signalGtkDragTarget.emit(context, getSpeakerAtPosition(x, y)) ;
	if (getParent().isEditMode())
		return Gtk::TextView::on_drag_drop(context, x, y, time);
	else
		return true ;
}

string AnnotationView::getSpeakerAtPosition(int win_x, int win_y)
{
	string id = "" ;

	// -- Get Buffer location
	int buf_x = -1 ;
	int buf_y = -1 ;
	window_to_buffer_coords(Gtk::TEXT_WINDOW_WIDGET, win_x, win_y, buf_x, buf_y) ;

	if (buf_x<0 || buf_y<0)
		return id ;

	// -- Get corresponding text position
	Gtk::TextIter iter ;
	get_iter_at_location(iter, buf_x, buf_y) ;

	// -- Check if some element is under
	string element = getBuffer()->getTaggedElementId(iter) ;

	// no element, therefore no speaker
	if (element.empty())
		return id ;

	// element, get the speaker
	if ( m_dataModel.existsElement(element) )
		id = m_dataModel.getElementProperty(element, "speaker", "") ;

	return id ;
}

bool AnnotationView::isDebugMode()
{
	return getParent().isDebugMode() ;
}

} /* namespace tag */
