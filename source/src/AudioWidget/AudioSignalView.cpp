/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class AudioSignalView
 *
 * AudioSignalView...
 */

#include <iostream>
#include <set>
#include <glibmm/timer.h>

#include "PangoMarkup.h"
#include "AudioSignalView.h"
#include "Common/util/StringOps.h"
#include "Common/util/FormatTime.h"
#include "Common/widgets/GtUtil.h"

static int CURSOR_UPD_PERIOD	= 25;	// 4O fps
static int TEMPO_UPD_PERIOD		= 500;
static int INFOS_UPD_PERIOD		= 100;

using namespace std;

namespace tag {

bool AudioSignalView::a_scaleToShow = true;

// --- AudioSignalView_play ---
gpointer AudioSignalView_play(gpointer data)
{
	TRACE << "STARTING PLAY THREAD" << endl;

	AudioSignalView* trans = (AudioSignalView*)data;
	trans->play();

	TRACE << "ENDING PLAY THREAD" << endl;
	g_thread_exit(0);

	return NULL;
}

// --- AudioSignalView ---
AudioSignalView::AudioSignalView(bool useVideo)
  : a_playThread(NULL)
{
	// ----------------------
	// --- Loading Threads --
	// ----------------------
	m_totalNbThread = 0 ;
	m_nbThread = 0 ;

	// ----------------------
	// --- MediaComponent ---
	// ----------------------
	pa			= NULL;
	device		= NULL;

	// --------------------------------
	// --- Special configuration ---
	// --------------------------------
	singleSignal = false ;
	useVideo = false ;

	// --------------------------------
	// --- Variables Initialization ---
	// --------------------------------
	a_timer.stop();

	a_play		= false;
	a_delay		= false;
	a_wait		= true;
	a_expanded	= false;
	a_isAlive	= true;

	set_no_show_all(true);

	a_updateLoop_cursorTracks_param1= 0.0;
	a_updateLoop_cursorTracks		= FALSE;
	a_updateLoop_dlgError_param1	= "";
	a_updateLoop_play_param1		= FALSE;

	a_updateLoop_play		= FALSE;
	a_updateLoop_dlgError	= FALSE;
	a_entryDelayText		= "";
	a_loopActive			= FALSE;
	a_updateLoop_timeout	= FALSE;
	a_cursorChanged			= FALSE;

	a_threadTerminated		= true;
	a_windowConnected		= FALSE;
	a_onlySpeechAllTracks	= FALSE;

	a_lastCursor			= 0.0;
	a_cursor				= 0.0;
	a_duration				= 0.0;
	a_lastElapsed			= 0;
	a_endOfSegment			= -1.0;
	a_lastEnd				= -1.0;
	a_lastDelay				= 0.0;
	a_endDelay				= 0.0;
	a_loopDelay				= false;
	a_maxOffset				= 0.0;
	buf_len					= 0.0;
	pa_initialized			= false;

	tc						= false;
	lastBlock				= false;
	a_interrupt				= false;
	a_sleep					= false;
	a_closure				= false;
	a_firstFrame			= true;

	upcur					= 0;
	track_clicked			= FALSE;

	a_toFocus				= NULL;
	a_updateCursor			= FALSE;

	a_beginSelection		= -1.0;
	a_endSelection			= -1.0;
	a_zoom					= 1;
	a_tempo					= 1.0;
	a_selectedAudioTrack	= -1;
	a_rewindAtEnd			= false;
	a_rewindAtSelectionEnd	= false;

	a_cursorSize			= 2;
	a_cursorColor			= "yellow";
	a_stopOnClick			= false;
	a_infos = "";

	a_selecting				= FALSE;
	a_leftMoving			= FALSE;
	a_peaksDirectory		= NULL;

	a_updateTempo			= -1;
	a_scaleAdded			= FALSE;
	a_upd_widget			= FALSE;
	a_updateInfos			= FALSE;
	isRewinding				= FALSE;
	streamingMode			= false;
	a_delay_on				= false;
	silentMode 				= false ;


	// -----------------------------------
	// --- Main Widgets Initialization ---
	// -----------------------------------
	a_mainBox = Gtk::manage(new Gtk::VBox());
	a_toolBar = Gtk::manage(new Gtk::HBox());

	// --------------------
	// --- Action Group ---
	// --------------------
	a_backward_start_delay	= 0.0;
	a_quickmove_delay		= 5.0;

	a_actions	= Gtk::ActionGroup::create("SignalAction");
	a_menuPlay	= Gtk::Action::create("signal_play",	Gtk::Stock::MEDIA_PLAY);
	a_menuPause = Gtk::Action::create("signal_pause",	Gtk::Stock::MEDIA_PAUSE);

	a_actions->add( a_menuPlay,
					Gtk::AccelKey("<mod2>Escape"),
					sigc::mem_fun(*this, &AudioSignalView::onPlayPauseClicked));

	a_actions->add( a_menuPause,
					Gtk::AccelKey("<mod2>Escape"),
					sigc::mem_fun(*this, &AudioSignalView::onPlayPauseClicked));

	a_menuPause->set_visible(false);


	// -------------------------------
	// --- Move Forward / Backward ---
	// -------------------------------
	a_actions->add( Gtk::Action::create("signal_forward",
					Gtk::Stock::MEDIA_FORWARD),
					Gtk::AccelKey("F4"),
					sigc::bind<float>(sigc::mem_fun(*this, &AudioSignalView::moveCursor),
					1.0) );

	a_actions->add( Gtk::Action::create("signal_rewind",
					Gtk::Stock::MEDIA_REWIND),
					Gtk::AccelKey("F1"),
					sigc::bind<float>(sigc::mem_fun(*this, &AudioSignalView::moveCursor),
					-1.0) );
/*
	a_actions->add( Gtk::Action::create("signal_incr_tempo",
					_("_Increase tempo"),
					_("Increase tempo by 10%")),
					Gtk::AccelKey("<control>KP_Add"),
					sigc::bind<float>(sigc::mem_fun(*this, &AudioSignalView::adjustTempo),
					0.1) );

	a_actions->add( Gtk::Action::create("signal_decr_tempo",
					_("_Decrease tempo"),
					_("Decrease tempo by 10%")),
					Gtk::AccelKey("<control>KP_Subtract"),
					sigc::bind<float>(sigc::mem_fun(*this, &AudioSignalView::adjustTempo),
					-0.1));
*/

	// ----------------------
	// --- Playback Frame ---
	// ----------------------
	a_playbackFrame			= Gtk::manage(new Gtk::Frame("Playback"));
	a_hBoxPlaybackFrame		= Gtk::manage(new Gtk::HBox());
	a_playPause					= Gtk::manage(new AudioPlayControlWidget());
	a_loop					= Gtk::manage(new Gtk::CheckButton("Loop "));
	a_labelDelay			= Gtk::manage(new Gtk::Label());
	a_entryDelay			= Gtk::manage(new Gtk::Entry());
	a_speechOnly				= Gtk::manage(new Gtk::CheckButton("Only speech"));

	Glib::ustring tool =  _("Define a delay in seconds for stopping playback before looping") ;

	a_playbackFrame->set_shadow_type(Gtk::SHADOW_IN);

	a_playPause->set_focus_on_click(false);
	a_playPause->signal_clicked().connect(sigc::mem_fun(*this, &AudioSignalView::onPlayPauseClicked));
	a_playPause->show();

	a_loop->set_focus_on_click(false);
	a_loop->set_active(false);
	a_loop->signal_toggled().connect(sigc::mem_fun(*this, &AudioSignalView::onLoopClicked));
	a_loop->show();

	a_labelDelay->set_markup(MarkupSmall((string(_("Delay"))+" :").c_str()).c_str());
	a_labelDelay->set_sensitive(false);
	a_labelDelay->hide();

	a_entryDelay->signal_key_press_event().connect(sigc::mem_fun(*this, &AudioSignalView::onKeyPressed));
	a_entryDelay->set_width_chars(2);
	a_entryDelay->set_sensitive(false);
	a_entryDelay->hide();

	a_speechOnly->set_focus_on_click(false);
	a_speechOnly->set_active(false);

	a_hBoxPlaybackFrame->pack_start(*a_playPause, false, false, 2);
	a_hBoxPlaybackFrame->pack_start(*a_loop, false, false, 2);
	a_hBoxPlaybackFrame->pack_start(*a_labelDelay, Gtk::SHRINK, 2);
	a_hBoxPlaybackFrame->pack_start(*a_entryDelay, Gtk::SHRINK, 2);
	a_hBoxPlaybackFrame->pack_start(*a_speechOnly, false, false, 2);

	// -- Labels --
	Gtk::Label* labelLoop			= (Gtk::Label*)a_loop->get_child();
	Gtk::Label* labelPlaybackFrame	= (Gtk::Label*)a_playbackFrame->get_label_widget();
	Gtk::Label* labelSkipable		= (Gtk::Label*)a_speechOnly->get_child();

	labelLoop->set_markup(MarkupSmall(_("Loop ")).c_str());
	labelPlaybackFrame->set_markup(MarkupSmall(_("Playback")).c_str());
	labelSkipable->set_markup(MarkupSmall(_("Only speech")).c_str());

	// -- Tooltips --
	a_tooltips.set_tip(*a_playPause,			_("Start or stop playing timeline at current cursor position."));
	a_tooltips.set_tip(*a_loop,			_("Enable or disable looping when reaching the end of the signal or the end of a selection."));
	a_tooltips.set_tip(*a_speechOnly,		_("Enable or disable playing only speech segments."));
	a_tooltips.set_tip(*a_entryDelay,	tool);

	a_speechOnly->hide();


	// ---------------------------
	// --- AutoSelection Frame ---
	// ---------------------------
	a_selectionAutoFrame	= Gtk::manage(new Gtk::Frame("Selection"));
	a_selectionAutoHBox		= Gtk::manage(new Gtk::HBox());
	a_autoSelectionPlay		= Gtk::manage(new Gtk::CheckButton("Auto selection play"));

	a_autoSelectionPlay->set_focus_on_click(false);
	a_autoSelectionPlay->set_active(false);

	a_selectionAutoHBox->pack_start(*a_autoSelectionPlay, false, false, 2);
	a_selectionAutoHBox->show_all_children();

	a_selectionAutoFrame->set_shadow_type(Gtk::SHADOW_IN);
	a_selectionAutoFrame->add(*a_selectionAutoHBox);

	// -- Labels --
	Gtk::Label* labelSelectionCheckFrame	= (Gtk::Label*)a_selectionAutoFrame->get_label_widget();
	Gtk::Label* labelAutoSelectionPlay		= (Gtk::Label*)a_autoSelectionPlay->get_child();

	labelSelectionCheckFrame->set_markup(MarkupSmall(_("Selection")).c_str());
	labelAutoSelectionPlay->set_markup(MarkupSmall(_("Auto play")).c_str());

	// -- Tooltips --
	a_tooltips.set_tip(*a_autoSelectionPlay, _("Enable or disable auto playing when making a selection."));


	// ---------------------
	// --- Expand Button ---
	// ---------------------
	Gtk::VBox* expandBox	= Gtk::manage(new Gtk::VBox());
	a_expandButton.set_image(1, ICO_EXPAND_RIGHT, SMALL_TOGGLE_BUTTON_SIZE) ;
	a_expandButton.set_image(2, ICO_EXPAND_RIGHT_OVER, SMALL_TOGGLE_BUTTON_SIZE) ;
	a_expandButton.set_image(3, ICO_EXPAND_RIGHT_PRESS, SMALL_TOGGLE_BUTTON_SIZE) ;
	a_expandButton.signal_button_release_event().connect(sigc::mem_fun(*this, &AudioSignalView::onExpandClicked));
	a_expandButton.show();

	a_playbackFrame->add(*a_hBoxPlaybackFrame);
	a_playbackFrame->show();

	a_hBoxPlaybackFrame->pack_start(*expandBox, false, false, 0);
	a_hBoxPlaybackFrame->show();

	expandBox->pack_start(a_expandButton, true, false, 1);
	expandBox->show();

	a_toolBar->pack_start(*a_playbackFrame, false, false, 3);
	a_toolBar->pack_start(*a_selectionAutoFrame, false, false, 3);

	a_selectionAutoFrame->show();
	// -- Tooltips --
	a_tooltips.set_tip(a_expandButton, _("Expand or collapse playback options."));


	// -------------------
	// --- Tempo Frame ---
	// -------------------
	a_atc = Gtk::manage(new AudioTempoControlWidget(false, false, true, false));

	// -- Tooltips --
	Glib::ustring tip = _("Adjust tempo factor from 0.25 (4x slower) to 4 (4x faster)") ;
	tip.append("\n") ;
	tip.append(_("CTRL + click for reset")) ;

	a_atc->signalValueChanged().connect(sigc::mem_fun(*this, &AudioSignalView::onTempoChanged));
	a_atc->signalFocusIn().connect(sigc::mem_fun(*this, &AudioSignalView::onFocusIn));
	a_atc->show();
	a_atc->setTooltip(tip.c_str(), "");
	a_atc->set_name("small_scale") ;

	a_toolBar->pack_start(*a_atc, false, false, 3);


	// ------------------
	// --- Zoom Frame ---
	// ------------------
	a_zoomFrame 	= Gtk::manage(new Gtk::Frame("Zoom"));
	a_zin			= Gtk::manage(new AudioZoomControlWidget(true));
	a_zout			= Gtk::manage(new AudioZoomControlWidget(false));
	a_hBoxZoomFrame = Gtk::manage(new Gtk::HBox());

	a_zin->set_focus_on_click(false);
	a_zin->signal_button_press_event().connect(sigc::mem_fun(*this, &AudioSignalView::onZoomInClicked), false);
	a_zin->show();

	a_zout->set_focus_on_click(false);
	a_zout->signal_button_press_event().connect(sigc::mem_fun(*this, &AudioSignalView::onZoomOutClicked), false);
	a_zout->show();

	a_hBoxZoomFrame->pack_start(*a_zin,		false, false, 2);
	a_hBoxZoomFrame->pack_start(*a_zout,	false, false, 2);
	a_hBoxZoomFrame->show();

	a_zoomFrame->set_shadow_type(Gtk::SHADOW_IN);
	a_zoomFrame->add(*a_hBoxZoomFrame);
	a_zoomFrame->show();

	a_toolBar->pack_start(*a_zoomFrame, false, false, 3);

	// -- Labels --
	Gtk::Label* labelZoomFrame = (Gtk::Label*)a_zoomFrame->get_label_widget();
	labelZoomFrame->set_markup( MarkupSmall(_("Zoom")).c_str() );

	// -- Tooltips --
	a_tooltips.set_tip(*a_zin, _("Increase waveform display resolution"));
	a_tooltips.set_tip(*a_zout, _("Decrease waveform display resolution"));


	// -----------------------
	// --- Selection Frame ---
	// -----------------------
	a_selectionFrame		= Gtk::manage(new Gtk::Frame("Selection"));
	a_hBoxSelectionFrame	= Gtk::manage(new Gtk::HBox());
	a_all					= Gtk::manage(new Gtk::Button("All"));

	a_selectionFrame->set_shadow_type(Gtk::SHADOW_IN);

	a_all->set_focus_on_click(false);
	a_all->signal_clicked().connect(sigc::mem_fun(*this, &AudioSignalView::onSelectAllClicked));

	// -- Labels --
	Gtk::Label* labelSelectionFrame	= (Gtk::Label*)a_selectionFrame->get_label_widget();
	Gtk::Label* labelAll			= (Gtk::Label*)a_all->get_child();

	labelSelectionFrame->set_markup(MarkupSmall(_("Selection")).c_str());
	labelAll->set_markup(MarkupSmall(_("All")).c_str());


	// -- Main Toolbar --
	a_mainBox->pack_start(*a_toolBar, false, false, 3);
	a_toolBar->show();

	// -- Tracks --
	a_durationMax = 0.0;

	// -----------------
	// --- Scrollbar ---
	// -----------------
	a_adjust	= Gtk::manage(new Gtk::Adjustment(0.0, 0.0, 100.0, 0.1, 1.0, 100.0));
	a_scrollBar	= Gtk::manage(new Gtk::HScrollbar(*a_adjust));

	a_scrollBar->signal_change_value().connect(sigc::mem_fun(*this, &AudioSignalView::onTrackScrolled));
	a_scrollBar->show();
	a_mainBox->pack_end(*a_scrollBar, false, false, 1);


	// -------------------
	// --- Infos Frame ---
	// -------------------
	a_infosFrame		= Gtk::manage(new Gtk::Frame("Informations"));
	a_hBoxInfosFrame3	= Gtk::manage(new Gtk::HBox());
	a_infoCursor		= Gtk::manage(new Gtk::Entry());
	a_infoTotal			= Gtk::manage(new Gtk::Entry());
	a_infoMinSize1		= Gtk::manage(new Gtk::Entry());
	a_infoMinSize2		= Gtk::manage(new Gtk::Entry());
	a_infoSelection		= Gtk::manage(new Gtk::Entry());
	a_hBoxInfosFrame	= Gtk::manage(new Gtk::HBox());
	infoTable			= Gtk::manage(new Gtk::Table(1,2,false));

	a_infosFrame->set_shadow_type(Gtk::SHADOW_IN);
	a_infosFrame->add(align_infoTable);

	a_hBoxInfosFrame3->pack_start(*a_infoMinSize1, Gtk::SHRINK, 0) ;
	a_hBoxInfosFrame3->pack_start(*a_infoMinSize2, Gtk::SHRINK, 0) ;
	a_hBoxInfosFrame3->set_spacing(2) ;

	a_infoCursor->set_has_frame(FALSE);
	a_infoCursor->set_editable(FALSE);
	a_infoCursor->set_sensitive(FALSE);
	a_infoCursor->modify_text(Gtk::STATE_INSENSITIVE, a_infoCursor->get_style()->get_text(Gtk::STATE_NORMAL));
	a_infoCursor->modify_bg(Gtk::STATE_INSENSITIVE, a_infosFrame->get_style()->get_bg(Gtk::STATE_NORMAL));
	a_infoCursor->set_size_request(112, -1);

	a_infoTotal->set_has_frame(FALSE);
	a_infoTotal->set_editable(FALSE);
	a_infoTotal->set_sensitive(FALSE);
	a_infoTotal->modify_text(Gtk::STATE_INSENSITIVE, a_infoTotal->get_style()->get_text(Gtk::STATE_NORMAL));
	a_infoTotal->set_size_request(127, -1);

	a_infoSelection->set_has_frame(FALSE);
	a_infoSelection->set_editable(FALSE);
	a_infoSelection->set_sensitive(FALSE);
	a_infoSelection->modify_text(Gtk::STATE_INSENSITIVE, a_infoSelection->get_style()->get_text(Gtk::STATE_NORMAL));
	a_infoSelection->set_size_request(100);

	a_infoMinSize1->set_has_frame(FALSE);
	a_infoMinSize1->set_editable(FALSE);
	a_infoMinSize1->set_sensitive(FALSE);
	a_infoMinSize1->set_size_request(35, -1);
	a_infoMinSize1->modify_text(Gtk::STATE_INSENSITIVE, a_infoMinSize1->get_style()->get_text(Gtk::STATE_NORMAL));

	a_infoMinSize2->set_has_frame(FALSE);
	a_infoMinSize2->set_editable(FALSE);
	a_infoMinSize2->set_sensitive(FALSE);
	a_infoMinSize2->set_size_request(35, -1);
	a_infoMinSize2->modify_text(Gtk::STATE_INSENSITIVE, a_infoMinSize2->get_style()->get_text(Gtk::STATE_NORMAL));

	a_infosFrame->show() ;
	a_infosFrame->show_all_children(true);
	infoTable->show_all_children(true) ;

	// -- Table -> Row 1 --
	infoTable->attach(*a_infoCursor,		0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;
	infoTable->attach(*a_infoSelection,		1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;

	// -- Table -> Row 2 --
	infoTable->attach(*a_infoTotal,			2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;
	infoTable->attach(*a_hBoxInfosFrame3,	3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK , 0, 0) ;

	// -- Labels --
	Gtk::Label* labelInfosFrame = (Gtk::Label*)a_infosFrame->get_label_widget();
	labelInfosFrame->set_markup(MarkupSmall(_("Informations")).c_str());

	// -- Main Packing --
	a_toolBar->pack_start(*a_infosFrame, Gtk::SHRINK, 1);

	align_infoTable.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
	align_infoTable.add(*infoTable) ;

	add(*a_mainBox);
	a_mainBox->show() ;
	a_mainBox->show_all_children(true);

	// ---------------
	// --- Mutexes ---
	// ---------------
	seek_mutex	= g_mutex_new();
	seekRequest = false;
	seekPosition = 0.0;	

	// -------------------------------
	// --- Default Widget Settings ---
	// -------------------------------
	a_labelDelay->hide();
	a_entryDelay->hide();
	a_speechOnly->hide();
	setPlay(false);

	a_expandButton.set_mode(1) ;

	GtUtil::getBaseColor(this) ;
}

// --- UpdateLoop ---
bool AudioSignalView::updateLoop()
{
	if (a_updateLoop_timeout)
	{
		Glib::signal_timeout().connect(sigc::mem_fun(this, &AudioSignalView::updateCursor),	CURSOR_UPD_PERIOD);
		Glib::signal_timeout().connect(sigc::mem_fun(this, &AudioSignalView::updateTempo),	TEMPO_UPD_PERIOD);
		Glib::signal_timeout().connect(sigc::mem_fun(this, &AudioSignalView::updateInfos),	INFOS_UPD_PERIOD);
		a_updateLoop_timeout = FALSE;
	}

	if (a_updateLoop_play)
	{
		setPlay(a_updateLoop_play_param1);
		a_updateLoop_play = FALSE;
	}

	if (a_updateLoop_cursorTracks)
	{
		a_updateLoop_cursorTracks = FALSE;
	}

	if (a_updateLoop_dlgError)
	{
		gdk_threads_enter();
		
		#ifdef __APPLE__
		dlg::error(a_updateLoop_dlgError_param1, toplevel);
		#else
		dlg::error(a_updateLoop_dlgError_param1);
		#endif

		a_updateLoop_dlgError = FALSE;
		gdk_flush();
		gdk_threads_leave();
	}

	a_entryDelayText = a_entryDelay->get_text();
	a_loopActive = a_loop->get_active();
	return a_isAlive;
}


// --- BackToStart ---
void AudioSignalView::backToStart()
{
	if (a_speechOnly->get_active() && nextNotSkipable(0) == -1)
	{
		a_updateLoop_play_param1 = FALSE;
		a_updateLoop_play = TRUE;
		a_play = FALSE;
		setCursorAudio(0.0);
		a_updateLoop_cursorTracks_param1 = 0.0;
		a_updateLoop_cursorTracks = TRUE;
	}
	else
	{
		if (a_loopActive)
		{
			// -- Loop Branch --
			string str = a_entryDelayText;

			for (unsigned int i = 0; i < str.length(); i++)
			{
				if (str[i] == ',')
					str[i] = '.';
			}

			float f = my_atof(str.c_str());
			a_delay = true;

			if (a_endSelection != -1.0)
			{
				setCursorAudio(a_endSelection);
				a_updateLoop_cursorTracks_param1 = a_cursor;
				a_updateLoop_cursorTracks = TRUE;
			}
			else
			{
				setCursorAudio(a_duration);
				a_updateLoop_cursorTracks_param1 = a_cursor;
				a_updateLoop_cursorTracks = TRUE;
			}

			// -- Applying Delay --
			a_loopDelay = true;
			iSleep(f, a_interrupt);
			a_loopDelay = false;

			if (a_endSelection != -1.0)
			{
				a_seekVideo.emit(a_beginSelection);
				a_cursor = a_beginSelection;
				setCursorAudio(a_beginSelection);
				a_updateLoop_cursorTracks_param1 = a_cursor;
				a_updateLoop_cursorTracks = TRUE;
			}
			else
			{
				a_cursor = 0.0;
				setCursorAudio(a_cursor);
				a_updateLoop_cursorTracks_param1 = a_cursor;
				a_updateLoop_cursorTracks = TRUE;
			}

			a_lastCursor = a_cursor;
			a_timer.stop();
			a_timer.start();

			a_delay = false;
		}
		else
		{
			// Normal Branch
			a_play = FALSE;
			a_updateLoop_play_param1 = FALSE;
			a_updateLoop_play = TRUE;


			// -- New Cursor Position --
			if (a_endSelection != -1.0)
			{
				if (a_rewindAtSelectionEnd || a_loopActive)
				{
					setCursorAudio(a_beginSelection);
					a_seekVideo.emit(a_beginSelection);
				}
				else
				{
					setCursorAudio(a_endSelection);
				}

				a_updateLoop_cursorTracks_param1 = a_cursor;
				a_updateLoop_cursorTracks = TRUE;
			}
			else
			{
				if (a_rewindAtEnd || a_loopActive)
				{
					setCursorAudio(0.0);
				}
				else
				{
					setCursorAudio(a_duration);
				}

				a_updateLoop_cursorTracks_param1 = a_cursor;
				a_updateLoop_cursorTracks = TRUE;
			}
		}
	}
	a_updateCursor = TRUE;
}


// --- UpdateCursor ---
bool AudioSignalView::updateCursor()
{
	if (a_updateCursor)
	{
		a_updateCursor = FALSE;
		setCursorTracks(a_cursor);
		a_signalCursorChanged.emit(a_cursor);
	}
	else
	{
		if (!a_delay && a_delay_on)
		{
			a_signalPlayedPaused.emit(true);
			a_delay_on = false;
		}

		if (a_play && !a_delay && !a_wait)
		{
			a_cursor = a_lastCursor + a_timer.elapsed() * a_tempo;

			// -- Eventual End Of Selection Check --
			if (hasSelection() && a_cursor > a_endSelection)
				a_cursor = a_endSelection;

			setCursorTracks2(a_cursor);
			a_signalCursorChanged.emit(a_cursor);

			// -- On Segment End --
			a_lastEnd	= a_endOfSegment;
			a_lastDelay	= a_endDelay;

			endSegment(a_cursor, buf_len, a_endDelay, a_endOfSegment);

			if (a_cursor >= a_lastEnd && a_lastEnd != -1.0)
			{
				a_cursor	= a_lastEnd;

				if (a_lastDelay > 0.0)
					a_delay		= true;

				// -- Shall we stop ? --
				if (a_lastDelay == -1.0)
				{
					setPlay(false);
					a_stopVideo.emit();
					a_endDelay = 0.0;
					a_lastDelay = 0.0;
					setCursorTracks2(a_lastEnd);
				}
			}
		}
		else
		if (a_delay)
		{
			if (!a_delay_on)
			{
				a_stopVideo.emit();
				a_delay_on = true;
			}

			setCursorTracks2(a_cursor);
		}
	}

	return a_isAlive;
}

// --- UpdateTempo ---
bool AudioSignalView::updateTempo()
{
	if (a_updateTempo != -1)
	{
		setTempo2(a_updateTempo, a_upd_widget);
		a_updateTempo = -1;
	}
	return a_isAlive;
}

// --- UpdateInfos ---
bool AudioSignalView::updateInfos()
{
	if (a_updateInfos)
	{
		a_updateInfos = FALSE;
		updateInfos2();
	}
	return a_isAlive;
}


// --- iSleep ---
void AudioSignalView::iSleep(float sleep_time, bool& watcher)
{
	int target_time = 0;
	int increment	= 1000;	// 1ms max latency

	a_sleep = true;

	a_delaytimer.start();

	while(!watcher && a_delaytimer.elapsed() < sleep_time)
	{
		USLEEP(increment);
	}

	a_delaytimer.stop();

	watcher	= false;
	a_sleep = false;
}


// --------------------------------
// --- Threaded PlayBack Method ---
// --------------------------------
void AudioSignalView::play()
{
	if (a_audioTracks.size() == 0)
	{
		a_threadTerminated = true;
		return;
	}

	MediumFrame*	frame	= NULL;
	bool local_alloc = false;
	a_threadTerminated		= false;
	bool locked = false;

	// Active Wait
	while (!locked && a_isAlive)
	{
		locked = a_mut1.trylock();
		USLEEP(10*1000);
	}

	// Reset
	if (locked)
	{
		setCursorAudio(0.0);
		a_mut1.unlock();
		a_updateLoop_timeout = TRUE;
	}

	a_updateLoop_timeout = TRUE;


	// --- Device Initialization ---
	if (!silentMode)
		device	= Guesser::open( (char*)a_audio_url.c_str() );
	else
		device	= Guesser::open(silentModeNbChannels, silentModeLength);

	if (!device)
		return;

	s_info	= device->m_info();
	buf_len	= (float)AUDIO_BUFFER_SIZE / (float)s_info->audio_sample_rate;


	// -- Filters Chain --
	for(int i=0; i<a_audioTracks.size(); i++)
	{
		vector<AbstractFilter*>	filters = a_audioTracks[i]->getFilters();
		for(int i=0; i<filters.size(); i++)
		{
			if (a_signalFiles.size() > 1)
			{
				filters.at(i)->setMediumPath( (char*)a_signalFiles[i].c_str() );
				filters.at(i)->setChannels( a_signalFiles.size() );
			}
			else
			{
				filters.at(i)->setMediumPath( (char*)a_audio_url.c_str() );
				filters.at(i)->setChannels( device->m_info()->audio_channels );
			}
		}
	}


	// --- Streaming (buffering) process ---
	while (a_isAlive)
	{
		double ts1, ts2;
		ts1 = a_timer.elapsed();

		if (!a_play)
		{
			freeAudioResources();
			a_wait = TRUE;
		}

		// -- Pause --
		if (!a_play && a_isAlive)
		{
			freeAudioResources();
			float temp = a_cursor;

			// -- Active wait --
			while (!a_play && a_isAlive)
			{
				USLEEP(10*1000);
			}

			if (!a_cursorChanged)
				a_cursor = temp;
			else
				a_cursorChanged = FALSE;

			int a = (int)(a_cursor*100);
			int b = (int)(a_endSelection*100);

			if (abs(a-b) <= 1 && !a_rewindAtSelectionEnd && !a_loopActive)
				a_cursor = a_beginSelection;

			// -- New Cursor Reference --
			a_lastCursor = a_cursor;
			a_timer.stop();
			a_timer.start();
			setCursorAudio(a_cursor);
		}

		if (!a_isAlive)
		{
			break;
		}


		// --- On Segment End ---
		if (a_delay)
		{
			setCursorAudio(a_lastEnd);
			a_updateLoop_cursorTracks_param1	= a_lastEnd;
			a_updateLoop_cursorTracks			= true;
			a_interrupt							= false;

			if (a_lastDelay > 0.0)
			{
				a_play = FALSE;
				iSleep(a_lastDelay, a_interrupt);
				a_play = TRUE;

				a_lastCursor = a_cursor;

				a_timer.stop();
				a_timer.start();
			}

			a_delay			= false;
			a_endDelay		= 0.0;
			a_endOfSegment	= -1.0;
		}

		// -- Speech-Only Mode --
		if (a_speechOnly->get_active())
		{
			float cursor2 = nextNotSkipable(a_cursor);
			int cur1 = (int)(a_cursor*1000);
			int cur2 = (int)(cursor2*1000);

			if (cur1 != cur2)
			{
				if (cursor2 == -1)
			 	{
					if (a_endSelection != -1.0)
					{
						a_mut1.lock();
						backToStart();
						a_mut1.unlock();
						a_cursor		= a_endSelection;
						a_lastCursor	= a_cursor;
						setCursorAudio(a_cursor);
					}
					else
					{
						a_updateLoop_play_param1 = FALSE;
						a_updateLoop_play = TRUE;
						a_play = FALSE;
						a_updateLoop_cursorTracks_param1 = a_endOfSegment;
						a_updateLoop_cursorTracks = TRUE;
					}
				}
				else
				{
					if ( (a_endSelection != -1.0) && (a_cursor >= a_endSelection) )
					{
						a_mut1.lock();
						backToStart();
						a_mut1.unlock();
						a_cursor		= a_endSelection;
						a_lastCursor	= a_cursor;
						setCursorAudio(a_cursor);
					}
					else
					{
						a_cursor		= cursor2;
						a_lastCursor	= a_cursor;
						setCursorAudio(a_cursor);
						a_timer.stop();
						a_timer.start();
						continue;
					}
				}
			}
		}


		// -- Signal End / Selection End --
		if (a_cursor >= signalLength() || ( (a_endSelection != -1.0) && (a_cursor == a_endSelection)))
		{
			a_mut1.lock();
			backToStart();
			a_mut1.unlock();
		}


		// -- Playback Section --
		if (a_play && !a_loopDelay)
		{
			// -- Frame Initialization --
			if (frame == NULL)
			{
				if (a_signalFiles.size() < 2)
				{
					// -- Last Buffer -> Skipping --
					if ( a_cursor >= (signalLength() - buf_len) )
					{
						TRACE <<"Last Buffer -> We should switch to previous block" << std::endl ;
						a_wait = false;

						// -- Sleepin' --
						USLEEP( (int)((buf_len / a_tempo) * 1000 * 1000) );

						a_cursor = signalLength();
						backToStart();
						a_play = false;
						continue;
					}

					device->m_play();

					while(frame == NULL && !pa_initialized)
						frame = device->m_read();

					for(int i=0; i<frame->len; i++)
						frame->samples[i] = '0';

					device->m_stop();
				}
				else
				{
					// -- Creating a frame, large enough for current file --
					// -- Assuming all files as mono, same format(therefore, same frame sizes) --
					frame			= new MediumFrame;
					frame->len		= s_info->audio_frame_size * a_signalFiles.size();
					frame->samples	= new int16_t[frame->len];
					local_alloc = true;
				}

				TRACE << "AudioSignalView --> Base Frame Initialized" << std::endl;
			}


			// ------------
			// --- SEEK ---
			// ------------
			locked = false;

			while (!locked)
			{
				locked = g_mutex_trylock(seek_mutex);
				USLEEP(1000);
			}

			if (seekRequest)
			{
//				printf("AudioSignalView --> Seek at pos : %f\n", seekPosition);

				// --- RTSP Case ---
				for(int i=0; i<a_audioTracks.size(); i++)
				{
					a_audioTracks.at(i)->stop();
					a_audioTracks.at(i)->seek(seekPosition);
					a_audioTracks.at(i)->play();
				}

				seekRequest = false;
			}

			g_mutex_unlock(seek_mutex);


			// -- Filters Chain --
			for(int i=0; i<a_audioTracks.size(); i++)
			{
				vector<AbstractFilter*>	filters = a_audioTracks[i]->getFilters();
				for(int i=0; i<filters.size(); i++)
					filters.at(i)->filter(frame);
			}

			// -- Termination Check (NULL frame) --
			if (frame == NULL)
				continue;	// until cursor reaches eof

			// -- Audio sync --
			a_cursor = frame->ts;

			// -- PortAudio Initialization --
			if (pa == NULL)
			{
				pa_initialized = false;

				pa = new PortAudioStream();

				if (a_signalFiles.size() < 2)
					pa_initialized = pa->init(s_info->audio_channels, s_info->audio_sample_rate, s_info->audio_frame_size);
				else
					pa_initialized = pa->init(a_signalFiles.size(), s_info->audio_sample_rate, s_info->audio_frame_size);

				if (!pa_initialized)
				{
					TRACE << "AudioSignalView --> PortAudio Initialization Failed" << std::endl ;
					a_play = false;
					freeAudioResources();
					continue;
				}

				// -- New Cursor Reference --
				if (a_wait == TRUE)
				{
					a_wait = FALSE;
					a_timer.stop();
					a_lastCursor = a_cursor;
					a_timer.start();
				}

				if (device == NULL)
				{
					TRACE << "AudioSignalView --> Medium Device not initialized" << std::endl ;
					return;
				}

				BUFFER_SIZE = s_info->audio_frame_size;
			}

			// -- Buffer length check --
			float	delta	= 0.0;
			float	cur_len = 0;
			int		n		= 0;
			int		REAL_SZ = BUFFER_SIZE;


			if (a_endSelection != -1.0)
			{
				cur_len	= a_lastCursor + a_timer.elapsed() * a_tempo;
				delta	= (cur_len + buf_len) - a_endSelection;
			}
			else
			{
				cur_len	= a_lastCursor + a_timer.elapsed() * a_tempo;
				delta	= (cur_len + buf_len) - signalLength();
			}

			if ( (delta > 0.0) && (delta < buf_len) )
			{
				REAL_SZ	= (int)(s_info->audio_sample_rate * (buf_len - delta));	// Buffer too large
			}

			// -- Cursor Adjustment --
			a_cursor = cur_len;

			// -- Video Sync Signal --
//			a_syncVideo.emit(a_cursor, a_stopOnClick);

			// --- PortAudio Output ---
			if (frame->len > 0)
			{
				// -- Zeroing Frame --
				{
					int firstSample;

					if (a_signalFiles.size() > 1)
						firstSample = a_signalFiles.size();
					else
						firstSample = s_info->audio_channels;

					for(int i= REAL_SZ * firstSample; i<frame->len; i++)
						frame->samples[i] = '0';
				}

				// -- Writing --
				PaError err = pa->write(frame->samples, frame->len);

				// -- PortAudio Error --
				if (err != paNoError)
				{
					Glib::ustring str1 = _( "Audio Device (soundcard) is unavailable, probably required by an external program.\n"
											"Please shutdown this program and restart it.\n\n" );
					Glib::ustring str2 = Pa_GetErrorText(err);
					Glib::ustring str3 = _("PortAudio Error: ") + str2;
					a_updateLoop_dlgError_param1 = str1 + str3;
					a_updateLoop_dlgError = TRUE;
					bool b = !a_play;
					a_updateLoop_play_param1 = FALSE;
					a_updateLoop_play = TRUE;
					a_play = FALSE;
					a_signalPlayedPaused.emit(b);
				}

				// -- Termination Check --
				if (REAL_SZ < BUFFER_SIZE)
				{
					TRACE << "AudioSignalView --> Last Audio Buffer" << std::endl ;

					float nap_time = (float)REAL_SZ / (float)s_info->audio_sample_rate / a_tempo;

					// -- Sleepin' --
					USLEEP( (int)(nap_time * 1000 * 1000) );

					if (a_endSelection != -1.0)
						a_cursor = a_endSelection;

					//control signal
					a_signalPlayedPaused.emit(false);
					backToStart();
				}
			}
		}
	}

	a_threadTerminated = true;
	if ( local_alloc ) {
		delete[] frame->samples;
		delete frame;
		frame = NULL;
	}
	freeAudioResources();
}

// --- OnKeyPressed ---
bool AudioSignalView::onKeyPressed(GdkEventKey* p_event)
{
	return true;
}

// --- OnFocusIn ---
void AudioSignalView::onFocusIn()
{
	if (a_toFocus != NULL)
		a_toFocus->grab_focus();
}

// --- ~AudioSignalView ---
AudioSignalView::~AudioSignalView()
{
	terminatePlayThread(false);

	for(unsigned int i=0; i < a_tracks.size(); ++i)
		delete a_tracks[i];

	a_stopVideo.emit() ;

	// -- MediaComponent --
	if (device)
		device->m_close();

	delete pa;

	if (device)
		delete device;
}


// --- RunPlayThread ---
void AudioSignalView::runPlayThread()
{
	if ( a_playThread != NULL )
		terminatePlayThread();

	a_isAlive =true;

	Glib::signal_timeout().connect(sigc::mem_fun(this, &AudioSignalView::updateLoop), 200);
	a_playThread	= g_thread_create(AudioSignalView_play, this, true, NULL);
}


// --- TerminatePlayThread ---
void AudioSignalView::terminatePlayThread(bool emit_signal)
{
	setPlay(false, emit_signal);
	a_isAlive = false;

	while (!a_threadTerminated)
	{
		USLEEP(10*1000);
	}

	if (a_playThread != NULL)
	{
		g_thread_join(a_playThread);
		TRACE << "JOIN  PLAY THREAD" << endl;
		a_playThread = NULL;
	}
}


// --- AddSegmentTrack ---
SegmentTrackWidget* AudioSignalView::addSegmentTrack(string p_label, int p_numTrack, bool flatMode)
{
	int startGap = 0;

	if (a_audioTracks.size() > 1)
	{
		if (startGap == 0)
			startGap = 30;
	}

	SegmentTrackWidget* p_track = new SegmentTrackWidget(signalLength(), p_label, p_numTrack, startGap, flatMode);
	AudioTrackWidget* audioTrack= NULL;

	a_tracks.push_back(p_track);
	a_segmentTracks.push_back(p_track);

	p_track->setCursorSize(a_cursorSize);
	p_track->setCursorColor(a_cursorColor);
	p_track->signalSelectionChanged().connect(sigc::mem_fun(*this, &AudioSignalView::onSelectionChanged));
	p_track->signalCursorChanged().connect(sigc::mem_fun(*this, &AudioSignalView::onCursorChanged));
	p_track->signalSegmentModified().connect(sigc::bind<SegmentTrackWidget*>(sigc::mem_fun(*this, &AudioSignalView::onSegmentModified), p_track));
	p_track->signalSegmentClicked().connect(sigc::mem_fun(*this, &AudioSignalView::onSegmentClicked));

	for (unsigned int i = 0; i < a_audioTracks.size(); i++)
	{
		if (i == a_selectedAudioTrack)
		{
			audioTrack = a_audioTracks[i];
			break;
		}
	}

	p_track->setSelectedAudioTrack(audioTrack);

	// -- Signal Try --
	p_track->signal_show().connect( sigc::mem_fun(this, &AudioSignalView::refreshTracks) );

	// GUI
	a_mainBox->pack_start(*p_track, false, false, 1);
	a_mainBox->reorder_child(*p_track, a_tracks.size());
	p_track->show();

	return p_track;
}


// --- AddMarkTrack ---
MarkTrackWidget* AudioSignalView::addMarkTrack()
{
	return NULL;
}


// --- AddScaleTrack ---
void AudioSignalView::addScaleTrack()
{
	if (a_scaleAdded)
		return;

	ScaleTrackWidget* p_track = new ScaleTrackWidget(signalLength(), a_audioTracks.size());

	a_tracks.push_back(p_track);
	scale_indice = a_tracks.size()-1 ;

	a_mainBox->pack_end(*p_track, false, false, 1);
	a_mainBox->reorder_child(*a_scrollBar, a_tracks.size());
	p_track->show();

	a_scaleAdded = TRUE;
}

// --- AddAudioTrack ---
void AudioSignalView::addAudioTrack(AudioTrackWidget* p_track)
{
	IODevice *iodev = p_track->getDevice();

	p_track->setCursorSize(a_cursorSize);
	p_track->setCursorColor(a_cursorColor);
	p_track->setChannelID( a_audioTracks.size() );

	a_tracks.push_back(p_track);
	a_offsets.push_back(0.0);

	p_track->signalSelectionChanged().connect(sigc::mem_fun(*this, &AudioSignalView::onSelectionChanged));
	p_track->signalCursorChanged().connect(sigc::mem_fun(*this, &AudioSignalView::onCursorChanged));
	((AudioTrackWidget*)p_track)->signalActivated().connect(sigc::mem_fun(*this, &AudioSignalView::onAudioTrackActivated));
	p_track->signalOffsetUpdated().connect(sigc::bind<AudioTrackWidget*>(sigc::mem_fun(*this, &AudioSignalView::onAudioTrackOffsetUpdated), p_track));
	p_track->signalAudioTrackSelected().connect(sigc::bind<AudioTrackWidget*>(sigc::mem_fun(*this, &AudioSignalView::onAudioTrackSelected), p_track));
	p_track->signalPopulatePopup().connect(sigc::bind<AudioTrackWidget*>(sigc::mem_fun(*this, &AudioSignalView::onPopulatePopup), p_track));
	p_track->signalSizeChanged().connect(sigc::mem_fun(*this, &AudioSignalView::onSizeChanged));
	p_track->signalSelecting().connect(sigc::mem_fun(*this, &AudioSignalView::onSelecting));
	p_track->signalFocusIn().connect(sigc::mem_fun(*this, &AudioSignalView::onFocusIn));
	p_track->signalExpanded().connect(sigc::mem_fun(*this, &AudioSignalView::onExpanded));

	a_mainBox->pack_start(*p_track, false, false, 1);
	a_mainBox->reorder_child(*p_track, a_tracks.size());
	p_track->show();
	p_track->setVideoMode(useVideo) ;

	a_audioTracks.push_back((AudioTrackWidget*)p_track);

	float duration = iodev->m_info()->audio_duration;

	if (duration > a_durationMax)
	{
		a_durationMax = duration;
	}

	/*** single signal ***/
	// if more than one track -> show activate btns, else hide them
	if ( a_audioTracks.size() > 1 )
	{
		for ( unsigned int i = 0; i < a_audioTracks.size(); ++i )
		{
			a_audioTracks[i]->showActivateBtn(true);
		}
	}
	else
	{
		p_track->showActivateBtn(false);
	}
}


// --- AddAudioStreams ---
bool AudioSignalView::addAudioStreams(const vector<string>& in_streams)
{
	bool ok = true ;

	for(int i=0; i<in_streams.size(); i++)
		ok = ok && addAudioStream((char*)in_streams.at(i).c_str(), NULL);

	if (in_streams.size() == 0 )
		return false ;

	// -- Updating Channels Index (Multiple Mono) --
	if (in_streams.size() > 1)
		for(int i=0; i<a_audioTracks.size(); i++)
			a_audioTracks.at(i)->setNbChannels( in_streams.size() );

	// -- Scale Track --
	if (a_scaleToShow)
		addScaleTrack();

	// -- UI updates --
	setSelectedAudioTrack(0);
	updateInfos2();

	// -- Thread Start --
	runPlayThread();

	return ok;
}

// --- AddAudioStream(File, PassPhrase) ---
bool AudioSignalView::addAudioStream(char* inFile, char* passPhrase)
{
	int 			channels;
	const char*		m_format;
	MediumFrame*	f = NULL;

	a_audio_url = inFile;
	a_signalFiles.push_back( Glib::ustring(a_audio_url) );

	streamingMode = (a_audio_url.find("rtsp://") != string::npos);

	// 0 - Streaming Case
	IODevice* iodev = Guesser::open( a_audio_url.c_str() );

	if (iodev == NULL)
		return false ;

	// -- Server / Remote Medium Check --
	if (streamingMode)
	{
		if ( !iodev->m_initialized() )
			return false ;

		// -- First read to determine frame_size --
		iodev->m_play();

		do
			f = iodev->m_read();
		while
			(f == NULL);

		iodev->m_stop();
	}

	// -- Building Streams --
	channels		= iodev->m_info()->audio_channels;

	// -- Saving Shortest Duration --
	if (a_duration == 0.0)
		a_duration = iodev->m_info()->audio_duration;
	else
		if (iodev->m_info()->audio_duration < a_duration)
			a_duration = iodev->m_info()->audio_duration;

	// -- Opening Tracks --
//	m_totalNbThread = channels ;
	m_totalNbThread = 0 ;
	for(int i=0; i<channels; i++)
	{
		/*** single signal ***/
		if (!singleSignal || a_audioTracks.size()==0)
		{
			m_totalNbThread++ ;
			IODevice* devtmp = Guesser::open( a_audio_url.c_str() );
			AudioTrackWidget *track_w = new AudioTrackWidget( devtmp );
			track_w->setPeaksDirectory(a_peaksDirectory);
			track_w->setStreaming(streamingMode, a_audio_url, m_local_file);
			track_w->setNumChannel( a_audioTracks.size() );
			track_w->getAudioWaveformWidget()->signalPeaksReady().connect(sigc::bind<IODevice*,AudioTrackWidget*>( sigc::mem_fun(this, &AudioSignalView::terminateAddAudioStreamThread), devtmp, track_w)) ;
			addAudioTrack(track_w);
			
			Glib::signal_timeout().connect(sigc::mem_fun(track_w->getAudioWaveformWidget(), &AudioWaveformWidget::checkPeaksLoading), 100);
			Glib::Thread::create( sigc::bind<AudioTrackWidget*,bool>( sigc::mem_fun(*this,&AudioSignalView::computePeaksInThread), track_w, false), false) ;
		}
	}

	iodev->m_close() ;
	delete(iodev) ;
	iodev = NULL ;

	if ( a_audioTracks.size()==0 )
		return false ;
	else
		return true ;
}


void AudioSignalView::terminateAddAudioStreamThread(bool ok, IODevice* device, AudioTrackWidget* track_w)
{
	if (device)
	{
		device->m_close();
		delete(device) ;
		device=NULL;
	}

	a_mut_pk.lock() ;
	m_nbThread++;
	Log::out() << "terminateAddAudioStreamThread " << m_nbThread << "/" << m_totalNbThread << std::endl ;
	if (m_nbThread!=m_totalNbThread)
	{
		a_mut_pk.unlock() ;
		Log::out() << "terminateAddAudioStreamThread: still some, let's wait..." << std::endl ;
		return ;
	}
	a_mut_pk.unlock() ;

	if (streamingMode)
		runPlayThread() ;

	a_signalPeaksReady.emit(ok) ;
}

bool AudioSignalView::addEmptyAudioStream(int nbChannels, double length)
{
	if (nbChannels==0)
		return false ;

	IODevice* iodev = Guesser::open(nbChannels, length);

	if (iodev == NULL)
		return false ;

	silentModeNbChannels = nbChannels ;
	silentModeLength = length ;
	silentMode = true ;

	// -- Opening Tracks --
	int channels = iodev->m_info()->audio_channels;

	for(int i=0; i<channels; i++)
	{
		/*** single signal ***/
		if (!singleSignal || a_audioTracks.size()==0)
		{
			IODevice* device = Guesser::open(nbChannels, length);
			AudioTrackWidget *track_w = new AudioTrackWidget( device, true);
			track_w->setPeaksDirectory(a_peaksDirectory);
			track_w->setNumChannel( a_audioTracks.size() );
			track_w->getAudioWaveformWidget()->signalPeaksReady().connect(sigc::bind<IODevice*, double>( sigc::mem_fun(this, &AudioSignalView::terminateAddEmptyAudioStreamThread), device, length)) ;
			addAudioTrack(track_w);
			
			Glib::signal_timeout().connect(sigc::mem_fun(track_w->getAudioWaveformWidget(), &AudioWaveformWidget::checkPeaksLoading), 100);
			Glib::Thread::create( sigc::bind<AudioTrackWidget*,bool>( sigc::mem_fun(*this,&AudioSignalView::computePeaksInThread), track_w, true), false) ;
		}
	}

	iodev->m_close() ;
	delete(iodev) ;
	iodev = NULL ;

	return true ;
}


void AudioSignalView::terminateAddEmptyAudioStreamThread(bool ok, IODevice* device, double length)
{
	device->m_close();
	delete device;

	a_duration = length ;

	// -- Scale Track --
	if (a_scaleToShow)
		addScaleTrack();

	// -- UI updates --
	setSelectedAudioTrack(0);
	updateInfos2();

	// -- Thread Start --
	runPlayThread();

	a_signalPeaksReady.emit(ok) ;
}

void AudioSignalView::computePeaksInThread(AudioTrackWidget* audioTrack, bool silentMode)
{
	if (!audioTrack)
		return ;

	Log::out() << "~~~~~~~~~~ ComputePeaks threading" << std::endl ;
	audioTrack->computePeaks(silentMode) ;
}

// --- UpdateInfos2 ---
bool AudioSignalView::updateInfos2()
{
	//> actualise cursor position
	char str[256];
	string s = "";
	sprintf(str, _("Cur. [%s]"), FormatTime(a_cursor).c_str());
	a_infoCursor->get_layout()->set_markup(MarkupSmall(str).c_str());
	a_infoCursor->queue_draw();
	s = str;

	//> actualise signal lenght
	sprintf(str, _("Total : %s"), FormatTime(signalLength()).c_str());
	a_infoTotal->get_layout()->set_markup(MarkupSmall(str).c_str());
	a_infoTotal->queue_draw();
	s = s+" / "+str;

	//> actualise selection lentgh
	 if ((a_endSelection == -1.0) || (a_beginSelection == -1.0))
	 {
		 a_infoSelection->get_layout()->set_markup(MarkupSmall(_("Sel. [none]")).c_str());
		 a_infoSelection->queue_draw();
		 s = s+" / "+_("Sel. [none]");
	 }
	 else
	 {
		 sprintf(str, _("Sel. [%s]"), FormatTime(a_endSelection - a_beginSelection).c_str());
		 string s2 = "<small>"+string(str)+"</small>";
		 a_infoSelection->get_layout()->set_markup(s2.c_str());
		 a_infoSelection->queue_draw();
		 s = s+" / "+str;
	 }

	s = s+" / "+str;

	a_infos = s;

	if ( (a_endSelection != -1.0) && (a_beginSelection != -1.0) )
	{
		if (a_minSizes.size() == 1)
		{
			float	minSize = a_minSizes[0];
			int		ms		= (int)roundf(minSize*1000);
			char*	color	= (char*)"darkgreen";

			if (minSize > a_endSelection - a_beginSelection)
				color = (char*)"red";

			sprintf(str,
					(char*)"<span foreground=\"%s\"><small>%dms</small></span>",
					color, ms);

			a_infoMinSize1->get_layout()->set_markup(str);
			a_infoMinSize1->queue_draw();
			a_infoMinSize1->show();
		}
		else
		{
			if (a_minSizes.size() == 2)
			{
				float minSize1 = a_minSizes[0];
				float minSize2 = a_minSizes[1];

				if (minSize2 < minSize1)
				{
					float tmp	= minSize1;
					minSize1	= minSize2;
					minSize2	= tmp;
				}

				int ms1 = (int)roundf(minSize1*1000);
				int ms2 = (int)roundf(minSize2*1000);

				char* color1 = (char*)"darkgreen";
				char* color2 = (char*)"darkgreen";

				if (minSize1 > a_endSelection - a_beginSelection)
					color1 = (char*)"red";
				if (minSize2 > a_endSelection - a_beginSelection)
					color2 = (char*)"red";

				sprintf(str,
						"<span foreground=\"%s\"><small>%dms</small></span>",
						color1, ms1);

				a_infoMinSize1->get_layout()->set_markup(str);
				a_infoMinSize1->queue_draw();

				sprintf(str,
						"<span foreground=\"%s\"><small>%dms</small></span>",
						color2, ms2);

				a_infoMinSize2->get_layout()->set_markup(str);
				a_infoMinSize2->queue_draw();
				a_infoMinSize1->show();
				a_infoMinSize2->show();
				a_hBoxInfosFrame3->show();
			}
		}
	}
	else
	{
		a_hBoxInfosFrame3->hide();
	}

	return false;
}


// --- SignalLength ---
float AudioSignalView::signalLength()
{
	return a_duration + a_maxOffset;
}


// --- MoveCursor ---
void AudioSignalView::moveCursor(float p_dist)
{
	double curs = a_lastCursor + a_timer.elapsed() * a_tempo;

	curs += (a_quickmove_delay * p_dist);

	// Normal case
	if (curs < 0)
		curs = 0;
	else
		if (curs > signalLength())
			curs = signalLength();

	// Selection Case
	if (a_endSelection != -1.0)
		if (curs < a_beginSelection)
			curs = a_beginSelection;
		else
			if (curs > a_endSelection)
				curs = a_endSelection;

	// -- Seeking to correct block --
	setCursor(curs, true, true);
}


// --- OnPlayPauseClicked ---
void AudioSignalView::onPlayPauseClicked()
{
	if (a_closure)
	{
		printf("AudioSignalView --> We won't play till streamin' is over :)\n");
		return;
	}

	if (a_speechOnly->get_active() && nextNotSkipable(0) == -1)
		return;

	bool b = !a_play;

	if (b)
	{
		a_lastCursor = a_cursor;
		a_timer.stop();
		a_timer.start();

		if (a_backward_start_delay > 0.0 &&
			a_cursor < a_durationMax &&
			a_beginSelection == -1.0)
		{
			if (a_backward_start_delay < a_lastCursor)
				a_lastCursor -= a_backward_start_delay;
			else
				a_lastCursor = 0.0;

			setCursorAudio(a_lastCursor);
			setCursorTracks(a_lastCursor);

			a_seekVideo.emit(a_beginSelection);
		}

		gettimeofday(&tv, NULL);
		d1 = (double)tv.tv_sec + (((double)tv.tv_usec)/1000000.0);

		if (a_cursor == a_endSelection)
		{
			setCursorAudio(a_beginSelection);
			setCursorTracks(a_beginSelection);

			a_seekVideo.emit(a_beginSelection);
		}
		else
		{
			if (a_cursor == a_durationMax)
			{
				setCursorAudio(0.0);
				setCursorTracks(0.0);
				setScroll(0.0);

				a_seekVideo.emit(0.0);
			}
		}
	}

	setPlay(b);

	if (a_sleep)
		a_interrupt = true;

	a_menuPlay->set_visible(!b);
	a_menuPause->set_visible(b);

	a_signalPlayedPaused.emit(b);
}


// --- OnTempoChanged ---
void AudioSignalView::onTempoChanged(float p_factor)
{
	setTempo(p_factor);
}


// --- OnZoomInClicked ---
bool AudioSignalView::onZoomInClicked(GdkEventButton* ev)
{
	if (a_zoom > 1)
	{
		if ( ev != NULL && ev->state & GDK_CONTROL_MASK )
			setZoom(1);
		else
			setZoom(a_zoom/2);
		a_signalZoomChanged.emit(a_zoom);
	}

	TRACE <<"  AudioSignalView::onZoomInClicked(  a_zoom=" << a_zoom << endl;
	return true;
}


// --- OnZoomOutClicked ---
bool AudioSignalView::onZoomOutClicked(GdkEventButton* ev)
{
	if ( ev != NULL && ev->state & GDK_CONTROL_MASK )
		setZoom(a_zoomMax);
	else
		setZoom( a_zoom*2);
	a_signalZoomChanged.emit(a_zoom);

	return true;
}


// --- OnCursorChanged ---
void AudioSignalView::onCursorChanged(float p_cursor)
{
	if (a_closure)
		return;

	if (a_stopOnClick && a_play)
		setPlay(false);

	//control signal
	a_syncVideo.emit(p_cursor, a_stopOnClick);

	if (p_cursor < 0.0)
		p_cursor = 0.0;

	setCursorAudio(p_cursor);
	setCursorTracks(p_cursor);
	a_signalCursorChanged.emit(p_cursor);
}


// --- OnSelectionChanged ---
void AudioSignalView::onSelectionChanged(float p_begin, float p_end)
{
	setSelection(p_begin, p_end);
	a_signalSelectionChanged.emit(p_begin, p_end);
}


// --- OnSelectAllClicked ---
void AudioSignalView::onSelectAllClicked()
{
	setSelection(0, a_durationMax);
	setCursorAudio(0);
	setCursorTracks(0);
	a_signalSelectionChanged.emit(0, a_durationMax);
}


// --- OnClearSelectionClicked ---
void AudioSignalView::onClearSelectionClicked()
{
	setSelection(-1.0, -1.0);
	a_signalSelectionChanged.emit(-1.0, -1.0);
}


// --- OnShowVideoClicked ---
void AudioSignalView::onShowVideoClicked()
{}


// --- OnTrackScrolled ---
bool AudioSignalView::onTrackScrolled(Gtk::ScrollType p_type, double p_value)
{
	double max_value	= 100.0 - a_adjust->get_page_size();
	double value		= p_value;

	if (value >= max_value)
		value = max_value;
	else
		if (value < 0.0)
			value = 0.0;

	float cursorScroll = a_durationMax * value / 100.0;

	setScroll(cursorScroll);
	a_signalTracksScrolled.emit(cursorScroll);

	return false;
}


// --- OnAudioTrackActivated ---
void AudioSignalView::onAudioTrackActivated(bool p_activated)
{
	for (unsigned int i = 0; i < a_segmentTracks.size(); i++)
		a_segmentTracks[i]->queue_draw();

	a_signalTrackActivated.emit(p_activated) ;
}

// --- OnAudioTrackOffsetUpdated ---
void AudioSignalView::onAudioTrackOffsetUpdated(float p_offset, AudioTrackWidget* p_track)
{
	// -- Restoring standard max duration --
	a_durationMax -= a_maxOffset;

	a_offsets[ p_track->getChannelID() ] = p_offset;

	// -- Zeroing Max Offset --
	bool zero = true;

	for(int i=0; i<a_offsets.size(); i++)
		if (a_offsets[i] != 0.0)
			zero = false;

	if (zero)
		a_maxOffset = 0.0;
	else
	{
		for (int i=0; i<a_offsets.size(); i++)
			if (a_maxOffset < a_offsets[i])
				a_maxOffset = a_offsets[i];

		if (a_maxOffset < p_offset)
			a_maxOffset = p_offset;
	}

	p_track->setOffset(a_cursor, p_offset);
	a_signalDelayChanged.emit(p_track->getChannelID(), p_offset) ;
	Log::out() << "emit delay for track " << p_track->getChannelID() << " : " << p_offset << std::endl ;

 	// -- Applying to max duration --
	a_durationMax += a_maxOffset;

	for (unsigned int i = 0; i < a_tracks.size(); i++)
	{
		TrackWidget* track = a_tracks[i];
		SegmentTrackWidget* segmentTrack = dynamic_cast<SegmentTrackWidget*>(track);

		if (segmentTrack != NULL)
			segmentTrack->remanageSegments();
	}
}


// --- OnAudioTrackSelected ---
void AudioSignalView::onAudioTrackSelected(AudioTrackWidget* p_track)
{
	for (unsigned int i = 0; i < a_audioTracks.size(); i++)
	{
		if (a_audioTracks[i] == p_track)
		{
			setSelectedAudioTrack(i);
			return;
		}
	}
}


// --- OnSegmentClicked ---
void AudioSignalView::onSegmentClicked(AudioTrackWidget* p_track)
{
	onAudioTrackSelected(p_track);

	if (!a_play && a_autoSelectionPlay->get_active())
		onPlayPauseClicked();

	a_traceReadCursor = true;
}


// --- OnPopulatePopup ---
void AudioSignalView::onPopulatePopup(int p_x, int p_y, AudioTrackWidget* p_track)
{
	Gtk::Menu* menu = Gtk::manage(new Gtk::Menu());

	char str[80];
	int naudio = 0;
	int nsegment = 0;

	bool several_track = (a_audioTracks.size()>1) ;

	TrackWidget* scale_track = NULL ;
	int scale_indice = -1 ;

	for (unsigned int i = 0; i < a_tracks.size(); i++)
	{
		TrackWidget* track = a_tracks[i];

		int type = -1 ;
		if (track)
			type = track->getType() ;

		SegmentTrackWidget* segmentTrack = dynamic_cast<SegmentTrackWidget*>(track);

		string label = "";

		if (segmentTrack==NULL)
		{
			// audio
			if (type==0)
			{
				naudio++;
				label = string(_("Audio track"))  ;
				if (several_track) {
					sprintf(str, "%d", naudio);
					label= label + " " + string(str);
				}
			}
			// scale time: let's keep information for displaying at end
			else if (type==2)
			{
				scale_track = track ;
				scale_indice = i ;
			}
		}
		// segment
		else if (type==1)
		{
			nsegment++;
			string s = segmentTrack->getLabel();
			label = s.substr(1).insert(0, 1, (char)(s[0]-32)) ;
			if (several_track) {
				sprintf(str, "%d", (segmentTrack->getNumTrack())+1);
				label = label + string(_(" track ")) + string(str);
			}
		}

		if (type!=2) {
			Gtk::CheckMenuItem* item = Gtk::manage(new Gtk::CheckMenuItem(label));
			item->set_active(track->is_visible());
			item->signal_toggled().connect(sigc::bind<int>(sigc::mem_fun(*this, &AudioSignalView::showTrack), i));
			item->show();
			menu->append(*item);
		}
	}

	//> if needed display scale track
	if (scale_track)
	{
		string label = string(_("Time scale")) ;
		Gtk::CheckMenuItem* item = Gtk::manage(new Gtk::CheckMenuItem(label));
		item->set_active(scale_track->is_visible());
		item->signal_toggled().connect(sigc::bind<int>(sigc::mem_fun(*this, &AudioSignalView::showTrack), scale_indice));
		item->show();
		menu->append(*item);
	}

	if (p_track == NULL)
	{
		a_signalPopulatePopup.emit(-1, p_x, p_y, menu);
	}
	else
	{
		for (unsigned int i = 0; i < a_audioTracks.size(); i++)
		{
			if (a_audioTracks[i] == p_track)
			{
				a_signalPopulatePopup.emit(i, p_x, p_y, menu);
				return;
			}
		}
	}
}


// --- ShowTrack ---
void AudioSignalView::showTrack(int p_num)
{
	if (a_tracks[p_num]->is_visible())
		a_tracks[p_num]->hide();
	else
		a_tracks[p_num]->show();
}


// --- OnLoopClicked ---
void AudioSignalView::onLoopClicked()
{
	bool b = a_loop->get_active();
	a_labelDelay->set_sensitive(b);
	a_entryDelay->set_sensitive(b);
}


// --- OnExpanded ---
void AudioSignalView::onExpanded(bool p_b)
{
	updateInfos2();
}


// --- SetPlay ---
void AudioSignalView::setPlay(bool p_play, bool emit_signal)
{
	a_play = p_play;
	a_playPause->setPlay(a_play);
	setActiveTooltip(!p_play);

	// -- Only active with RTSP device --
	if (device == NULL)
		return;

	if (a_play)
	{
		for(int i=0; i<a_audioTracks.size(); i++)
			a_audioTracks.at(i)->play();
	}
	else
	{
		// -- Interrupting Sleep (delay) --
		if (a_sleep)
			a_interrupt = true;

		TRACE << "AudioSignalView --> Stopping Audio Devices" << std::endl ;
	
		a_closure = true;
		for(int i=0; i<a_audioTracks.size(); i++)
			a_audioTracks.at(i)->stop();
		a_closure = false;

		// -- Actualize last position in audio compoenent
		setCursor(a_cursor, false) ;

		// -- Tell linked GUI we've changed position
		if (emit_signal)
			a_signalCursorChanged.emit(a_cursor);
	}
}


// --- SetTempo ---
void AudioSignalView::setTempo(float p_factor, bool p_upd_widget)
{
	if (p_factor > 4.0)
		p_factor = 4.0;

	if (p_factor < 0.25)
		p_factor = 0.25;

	a_updateTempo = p_factor;
	a_upd_widget = p_upd_widget;
}


// --- SetTempo2 ---
void AudioSignalView::setTempo2(float p_factor, bool p_upd_widget)
{
	if (p_factor < 0)
		p_factor = 0;

	if (p_upd_widget)
		a_atc->setFactor(p_factor);

	a_tempo = p_factor;

	for (unsigned int i = 0; i < a_tracks.size(); i++)
	{
		AudioTrackWidget* audioTrack = dynamic_cast<AudioTrackWidget*>(a_tracks[i]);

		if (audioTrack != NULL)
			audioTrack->setTempo(p_factor);
	}

	a_cursor = roundf(a_cursor*1000)/1000;
	a_lastCursor = a_cursor;
	a_timer.stop();
	a_timer.start();

	gettimeofday(&tv, NULL);
	d1 = (double)tv.tv_sec + (((double)tv.tv_usec)/1000000.0);
}


// --- SetZoom ---
bool AudioSignalView::setZoom(int p_zoom, bool init)
{
	if (p_zoom >= a_zoomMax)
		a_zout->set_sensitive(false);
	else
		a_zout->set_sensitive(true);

	if (p_zoom == 1)
		a_zin->set_sensitive(false);
	else
		a_zin->set_sensitive(true);

	if ( a_zoom == p_zoom && !init)
		return false;

	bool b = true;

	// -- Tracks Not Ready ? --> Re-arm Timeout --
	for (unsigned int i = 0; i < a_audioTracks.size() && b; i++)
		b = a_audioTracks[i]->isReady();

	if (!b)
	{
		if (a_zoomRequest == -1)
		{
			a_zoomRequest = p_zoom;
			Glib::signal_timeout().connect(sigc::bind<int,bool>(sigc::mem_fun(this, &AudioSignalView::setZoom), p_zoom, false), 200);
		}
		return true;
	}

	a_zoomRequest = -1;
	a_zoom = p_zoom;

	if (a_zoom >= a_zoomMax)
		a_zoom = a_zoomMax;

	// -- Scrollbar update --
	a_adjust->set_page_size( (100.0 * a_zoom) / a_zoomMax );

	// -- Zoom in Tracks --
	for (unsigned int i = 0; i < a_tracks.size(); i++)
		a_tracks[i]->setZoom(a_zoom);

	// -- New Scroll Algorithm --

	// -- Getting Width --
	int view_w = 1000;

	if (a_audioTracks.size() > 0)
	{
		view_w = 100000;

		for (unsigned int i = 0; i < a_audioTracks.size(); i++)
		{
			int w2 = a_audioTracks[i]->getWidth();

			if (w2 < view_w)
				view_w = w2;
		}
	}

	float visible		= (float)view_w * (float)a_zoom * AudioWidget::AUDIO_ZOOM_MAX;
	float cursorScroll	= a_cursor - visible / 4.0;

	// -- Scrollbar steps --
	a_scrollBar->set_increments( a_zoom / signalLength(), 1.0 );

	// -- Boundaries Check --
	if (cursorScroll >= a_durationMax - visible)
		cursorScroll = a_durationMax - visible;

	// -- Back To Start --
	if (cursorScroll < 0.0 || a_zoom == a_zoomMax)
		cursorScroll = 0.0;

	setScroll(cursorScroll);
	a_signalTracksScrolled.emit(cursorScroll);

	updateInfos2();
	return false;
}

/**
 * Updates cursor position, and forwards it to every subtrack
 * @param p	New cursor position
 */
void	AudioSignalView::setCursor(float p, bool emit_signal, bool sync_video)
{
	if (p > signalLength() || p < 0.0)
		return;

	setCursorAudio(p);
	setCursorTracks(p);
	if ( sync_video ) {
		a_syncVideo.emit(p, a_stopOnClick);
	}
	if (emit_signal) {
		a_signalCursorChanged.emit(p);
	}

}

// --- SetCursorAudio ---
void AudioSignalView::setCursorAudio(float p_cursor)
{
	bool locked = false;

	while (!locked)
	{
		locked = g_mutex_trylock(seek_mutex);
		USLEEP(10*1000);
	}

	seekRequest = true;
	seekPosition = p_cursor;

	g_mutex_unlock(seek_mutex);
}


// --- SetCursorTracks ---
void AudioSignalView::setCursorTracks(float p_cursor)
{
	a_cursor = p_cursor;
	a_lastCursor = a_cursor;

	// - Réarmement du timer -
	a_timer.stop();
	a_timer.start();

	gettimeofday(&tv, NULL);
	d1 = (double)tv.tv_sec + (((double)tv.tv_usec)/1000000.0);

	a_cursorChanged = TRUE;
	setCursorTracks2(p_cursor);
}

// --- SetCursorTracks2 ---
void AudioSignalView::setCursorTracks2(float p_cursor)
{
	a_cursor = p_cursor;

	if (a_cursor < 0.0)
		a_cursor = 0.0;
	else if (a_cursor > signalLength())
		a_cursor = signalLength();

	for (unsigned int i = 0; i < a_tracks.size(); i++) {
		a_tracks[i]->setCursor(a_cursor);
	}

	int w = 1000;

	if (a_audioTracks.size() > 0)
	{
		w = 100000;

		for (unsigned int i = 0; i < a_audioTracks.size(); i++)
		{
			int w2 = a_audioTracks[i]->getWidth();

			if (w2 < w)
				w = w2;
		}
	}

	float visible		= ((float)w * (float)a_zoom * AudioWidget::AUDIO_ZOOM_MAX);
	float margeBegin	= (60.0 * (float)a_zoom * AudioWidget::AUDIO_ZOOM_MAX);
	float margeEnd		= (60.0 * (float)a_zoom * AudioWidget::AUDIO_ZOOM_MAX);

	// ---------------
	// -- SCROLLING --
	// ---------------

	if (a_selecting)
	{
		// -- Backward Scroll --
		if (p_cursor < a_cursorScroll + margeBegin)
		{
			float cursorScroll = p_cursor - margeBegin;

			if (cursorScroll < 0)
				cursorScroll = 0;

			setScroll(cursorScroll);
			a_signalTracksScrolled.emit(cursorScroll);
		}
		else
		// -- Forward Scroll --
		{
			if (p_cursor > a_cursorScroll + visible - margeEnd)
			{
				float cursorScroll = a_cursorScroll + p_cursor - (a_cursorScroll + visible - margeEnd);

				if (cursorScroll > a_durationMax)
					cursorScroll = a_durationMax;

				if (a_cursorScroll + visible < a_durationMax)
				{
					setScroll(cursorScroll);
					a_signalTracksScrolled.emit(cursorScroll);
				}
			}
		}
	}
	else
	{
		if ( (p_cursor < a_cursorScroll + margeBegin) || (p_cursor > a_cursorScroll + visible - margeEnd) )
        {
			float cursorScroll = p_cursor - margeBegin;

			if (cursorScroll < 0)
				cursorScroll = 0;

			if (a_cursorScroll + visible >= a_durationMax)
			{
				if (p_cursor >= a_cursorScroll + visible)
					cursorScroll = a_cursorScroll;
			}

			setScroll(cursorScroll);
			a_signalTracksScrolled.emit(cursorScroll);
		}
	}

	a_updateInfos = TRUE;
}


// --- SetSelection ---
void AudioSignalView::setSelection(float p_begin, float p_end)
{
	int w = 1000;

	if (a_audioTracks.size() > 0)
	{
		w = 100000;

		for (unsigned int i = 0; i < a_audioTracks.size(); i++)
		{
			int w2 = a_audioTracks[i]->getWidth();

			if (w2 < w)
				w = w2;
		}
	}

	float visible		= ((float)w * (float)a_zoom * AudioWidget::AUDIO_ZOOM_MAX);
	float margeBegin	= (60.0 * (float)a_zoom * AudioWidget::AUDIO_ZOOM_MAX);
	float margeEnd		= (60.0 * (float)a_zoom * AudioWidget::AUDIO_ZOOM_MAX);

	a_beginSelection = p_begin;
	a_endSelection = p_end;

	for (unsigned int i = 0; i < a_tracks.size(); i++)
		a_tracks[i]->setSelection(a_beginSelection, a_endSelection);

	if (a_selecting)
	{
		if (a_beginSelection != -1 && a_endSelection != -1 && !a_leftMoving)
		{
			if (a_endSelection < a_cursorScroll + margeBegin)
			{
				float cursorScroll = a_endSelection - margeBegin;

				if (cursorScroll < 0)
					cursorScroll = 0;

				if (a_cursorScroll + visible < a_durationMax)
				{
					setScroll(cursorScroll);
					a_signalTracksScrolled.emit(cursorScroll);
				}
			}
			else
			{
				if (a_endSelection > a_cursorScroll + visible - margeEnd)
				{
					float cursorScroll = a_cursorScroll + a_endSelection - (a_cursorScroll + visible - margeEnd);

					if (cursorScroll > a_durationMax)
						cursorScroll = a_durationMax;

					if (a_cursorScroll + visible < a_durationMax)
					{
						setScroll(cursorScroll);
						a_signalTracksScrolled.emit(cursorScroll);
					}
				}
			}
		}
	}

	//control signal
	a_videoSelection.emit(a_beginSelection, a_endSelection);

	updateInfos2();
}


// --- SetScroll ---
void AudioSignalView::setScroll(float p_cursorScroll)
{
	if (p_cursorScroll > a_duration)
	{
		Log::err() << "ASW::setScroll -> invalid value. Aborted." << std::endl ;
		return ;
	}

	a_cursorScroll = p_cursorScroll;

	for (unsigned int i = 0; i < a_tracks.size(); i++)
		a_tracks[i]->setScroll(a_cursorScroll);

	double value = p_cursorScroll * 100.0 / a_durationMax;
	a_scrollBar->set_value(value);
	a_updateInfos = TRUE;
}


// --- SetZoomEntire ---
bool AudioSignalView::setZoomEntire()
{
	calcZoomMax();
	setZoom(a_zoomMax);

	// Tracks Not Ready ? --> Re-arm Timeout
	for (unsigned int i = 0; i < a_audioTracks.size()  ; i++)
		if ( !a_audioTracks[i]->isReady() )
			return true;

	return false;
}


// --- CalcZoomMax ---
void AudioSignalView::calcZoomMax(int width)
{
	int w = width;

	if (w <= 0 && a_audioTracks.size() > 0)
	{
		w = 100000;

		for (unsigned int i = 0; i < a_audioTracks.size(); i++)
		{
			int w2 = a_audioTracks[i]->getWidth();

			if (w2 < w)
				w = w2;
		}
	}

	if (w < 100)
		w = 1000;

	a_zoomMax = (int)ceil(a_durationMax / ((float)w * 0.99) / AudioWidget::AUDIO_ZOOM_MAX);

	for (unsigned int i = 0; i < a_tracks.size(); i++)
		a_tracks[i]->setZoomMax(a_zoomMax);
}


// --- SetSelectedAudioTrack ---
void AudioSignalView::setSelectedAudioTrack(int p_selectedAudioTrack)
{
	if (a_selectedAudioTrack == p_selectedAudioTrack)
		return;

	AudioTrackWidget* track= NULL;

	a_selectedAudioTrack = p_selectedAudioTrack;

	for (unsigned int i = 0; i < a_audioTracks.size(); i++)
	{
		if (i == a_selectedAudioTrack)
		{
			track = a_audioTracks[i];
			track->setSelected(true);
		}
		else
		{
			a_audioTracks[i]->setSelected(false);
		}
	}

	for (unsigned int i = 0; i < a_segmentTracks.size(); i++)
		a_segmentTracks[i]->setSelectedAudioTrack(track);

	a_signalAudioTrackSelected.emit(p_selectedAudioTrack);
}


// --- GetAudioTrackDuration ---
float AudioSignalView::getAudioTrackDuration(int p_audioTrackIndex)
{
	return a_duration;
}


// --- GetAudioTrackFormat ---
const char* AudioSignalView::getAudioTrackFormat(int p_audioTrackIndex)
{
	IODevice* t_device = Guesser::open( a_audio_url.c_str() );

	if (!t_device)
		return _("Unknown");

	string t_format = t_device->m_info()->audio_codec;

	t_device->m_close();

	delete t_device;

	return t_format.c_str();
}


// --- GetAudioTrackEncoding ---
const char* AudioSignalView::getAudioTrackEncoding(int p_audioTrackIndex)
{
	IODevice* t_device = Guesser::open( a_audio_url.c_str() );

	if (!t_device)
		return _("Unknown");

	string t_encoding = t_device->m_info()->audio_encoding;

	t_device->m_close();

	delete t_device;

	return t_encoding.c_str();
}


// --- NextNotSkipable ---
float AudioSignalView::nextNotSkipable(float p_secs)
{
	float min = -1;

	for (unsigned int i = 0; i < a_segmentTracks.size(); i++)
	{
		if (a_audioTracks[a_segmentTracks[i]->getNumTrack()]->isActivated())
		{
			float f = a_segmentTracks[i]->nextNotSkipable(p_secs);

			if (f != -1)
				if ((min == -1) || (f < min))
					min = f;
		}
	}

	return min;
}


// --- EndSegment ---
void AudioSignalView::endSegment(float p_secs, float p_size, float& p_delay,
		float& p_endSeg)
{
	float max_delay = 0.0;
	float endSegMax = -1.0;

	for (unsigned int i = 0; i < a_segmentTracks.size(); i++)
	{
		float track_delay	= 0.0;	// normal mode
		float endSeg		= -1.0;	// eof

		a_segmentTracks[i]->endSegment(p_secs, p_size, track_delay, endSeg);

		// -- First Track --
		if (endSegMax == -1.0)
		{
			max_delay = track_delay;
			endSegMax = endSeg;
			continue;
		}

		// -- Stop --
		if (track_delay == -1.0)
		{
			if (endSeg <= endSegMax)
			{
				max_delay = track_delay;
				endSegMax = endSeg;
			}
		}

		// -- Delay --
		if (track_delay > 0.0)
		{
			// -- Skipping, because stop is prior to delay --
			if ( (endSeg == endSegMax) && (max_delay == -1.0) )
				continue;

			if ( (endSeg == endSegMax) && (track_delay >= max_delay) )
			{
				max_delay	= track_delay;
				endSegMax	= endSeg;
			}
			else if ( (endSeg < endSegMax) )
			{
				max_delay = track_delay;
				endSegMax = endSeg;
			}
		}
	}

	p_delay		= max_delay;
	p_endSeg	= endSegMax;
}


// --- OnSegmentModified ---
void AudioSignalView::onSegmentModified(string p_id, float p_start,
		float p_end, SegmentTrackWidget* p_track)
{
	a_signalSegmentModified.emit(p_id, p_start, p_end, p_track);
}


// --- SetActiveToolTip ---
void AudioSignalView::setActiveTooltip(bool p_activeTooltip)
{
	for (unsigned int i = 0; i < a_segmentTracks.size(); i++)
		a_segmentTracks[i]->setActiveTooltip(p_activeTooltip);
}


// --- SetMinSizes ---
void AudioSignalView::setMinSizes(vector<float> p_minSizes)
{
	a_minSizes = p_minSizes;
}


// --- OnClickedForPopup ---
bool AudioSignalView::onClickedForPopup(GdkEventButton* p_event)
{
	if (p_event->button == 3)
	{
		GdkWindow* window = p_event->window;
		int x = -1;
		int y = -1;
		gdk_window_get_origin(window, &x, &y);
		x += (int)p_event->x;
		y += (int)p_event->y;
		onPopulatePopup(x, y, NULL);
		return true ;
	}
	return FALSE;
}


// --- SetCursorSize ---
void AudioSignalView::setCursorSize(int p_pix)
{
	a_cursorSize = p_pix;

	for (unsigned int i = 0; i < a_tracks.size(); i++)
		a_tracks[i]->setCursorSize(p_pix);
}


// --- SetCursorColor ---
void AudioSignalView::setCursorColor(string p_color)
{
	a_cursorColor = p_color;

	for (unsigned int i = 0; i < a_tracks.size(); i++)
		a_tracks[i]->setCursorColor(p_color);
}


// --- OnSizeChanged ---
void AudioSignalView::onSizeChanged(int width)
{
	if (a_zoom == a_zoomMax)
		setZoomEntire();
	else
		calcZoomMax(width);

	updateInfos2();
}


// --- OnSelecting ---
void AudioSignalView::onSelecting(bool p_select, bool p_left)
{
	a_selecting = p_select;
	a_leftMoving = p_left;

	if (!p_select && a_beginSelection != -1 && a_endSelection != -1)
		if (!a_play && a_autoSelectionPlay->get_active())
			onPlayPauseClicked();
}


// --- On_size_allocate ---
void AudioSignalView::on_size_allocate(Gtk::Allocation& p_allocation)
{
	Gtk::Frame::on_size_allocate(p_allocation);

	if (a_windowConnected)
		return;

	a_windowConnected = TRUE;
	signal_button_press_event().connect(sigc::mem_fun(*this, &AudioSignalView::onClickedForPopup));
	add_events(Gdk::BUTTON_PRESS_MASK);
}


// --- GetCursor ---
float AudioSignalView::getCursor()
{
	return a_cursor;
}


// --- GetActualCursor ---
float AudioSignalView::getActualCursor()
{
    return (a_lastCursor + a_timer.elapsed() * a_tempo);
}


// --- SetPeaksDirectory ---
void AudioSignalView::setPeaksDirectory(const char* p_path)
{
	a_peaksDirectory = p_path;

	for (unsigned int i = 0; i < a_audioTracks.size(); i++)
		a_audioTracks[i]->setPeaksDirectory(p_path);
}


// --- OnExpandClicked ---
bool AudioSignalView::onExpandClicked(GdkEventButton* event)
{
	if (a_expanded)
	{
		a_labelDelay->hide();
		a_entryDelay->hide();
		a_speechOnly->hide();
		a_expandButton.set_image(1, ICO_EXPAND_RIGHT, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(2, ICO_EXPAND_RIGHT_OVER, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(3, ICO_EXPAND_RIGHT_PRESS, SMALL_TOGGLE_BUTTON_SIZE) ;
	}
	else
	{
		a_labelDelay->show();
		a_entryDelay->show();
		a_speechOnly->show();
		a_expandButton.set_image(1, ICO_EXPAND_LEFT, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(2, ICO_EXPAND_LEFT_OVER, SMALL_TOGGLE_BUTTON_SIZE) ;
		a_expandButton.set_image(3, ICO_EXPAND_LEFT_PRESS, SMALL_TOGGLE_BUTTON_SIZE) ;
	}

	a_expanded = !a_expanded;
	Glib::signal_idle().connect(sigc::mem_fun(this, &AudioSignalView::updateInfos2));
	return false ;
}


/**
 * store current audio settings as string
 * @return setting string
 */
string AudioSignalView::getCurrentAudioSettings()
{
	// global settings : current zoom level
	ostringstream os;
	long int hzoom = getZoom();
	int nzoom = 0;

	while (hzoom < a_zoomMax)
	{
		if (hzoom > 0 )
			hzoom *= 2;
		else
		{
			Log::err() << "(?) hzoom error. reset to zoom max..." << std::endl ;
			hzoom = a_zoomMax ;
		}
		++nzoom;
	}
	os << "hzoom=" << nzoom << ";";

	// for each audio track : volume, offset and vzoom
	vector<AudioTrackWidget*>::iterator it;
	for (it=a_audioTracks.begin(); it != a_audioTracks.end(); ++it)
	{
		int num = (*it)->getNumChannel();

		os << "vol" << num << "=" << (*it)->getVolume() << ";";
		os << "vzoom" << num << "=" << (*it)->getVZoom() << ";";
		os << "pitch" << num << "=" << (*it)->getPitch() << ";";
		os << "offset" << num << "=" << (*it)->getOffset() << ";";
	}
	return os.str();
}


/**
 * set audio settings from string definition
 * @param settings audio settings definition, using dscv-like syntax, as produced by getCurrentAudioSettings
 */
void AudioSignalView::setAudioSettings(const string& settings)
{
	std::map<std::string, std::string> items;
	std::map<std::string, std::string>::iterator it;

	calcZoomMax();

	StringOps(settings).getDCSVItems(items);

	for (it=items.begin(); it != items.end(); ++it)
	{
		if (it->first == "hzoom")
		{
			int nzoom = atoi(it->second.c_str());
			int hzoom = a_zoomMax;
			while (nzoom > 0 && hzoom > 0 )
			{
				hzoom /= 2;
				--nzoom;
			}
			if ( hzoom == 0 )
				hzoom = 1;

			setZoom(hzoom, true);
		}
		else
		{
			string	key;
			int		notrack;

			notrack	= atoi( it->first.substr(it->first.length() - 1).c_str() );
			key		= it->first.substr(0, it->first.length() - 1);

			if ( notrack < 0 || notrack > (a_audioTracks.size()-1) )
			{
					MSGOUT << "setAudioSettings : invalid item " << it->first << endl;
					continue;
			}

			if ( key == "vol" )
				a_audioTracks[notrack]->setVolume(atof(it->second.c_str()));
			if ( key == "vzoom" )
				a_audioTracks[notrack]->setVZoom(atof(it->second.c_str()));
			if ( key == "pitch" )
			{
				float pitch = atof(it->second.c_str());

				// -- NaN Check --
				if (pitch != pitch)
				{
					TRACE << "AudioSignalView::setAudioSettings -> Pitch : NaN found -> default fallback" << std::endl ;
					pitch = 1.0;
				}

				if (pitch < 0.4 || pitch > 2.5)
					pitch = 1.0;

				a_audioTracks[notrack]->setPitch(pitch);
			}

			if ( key == "offset" )
				a_audioTracks[notrack]->setOffset(0.0, atof(it->second.c_str()));
		}
	}
}


// --- RefreshTracks ---
void AudioSignalView::refreshTracks()
{
	if (a_audioTracks.size() > 1)
		for (int i=0; i<a_segmentTracks.size(); i++)
			a_segmentTracks[i]->resetActivateButtonSize(a_audioTracks[0]->getActivateBtnSize() );
}


// --- FreeAudioResources ---
void AudioSignalView::freeAudioResources()
{
	// -- MediaComponent Shutdown --
	if (pa != NULL)
	{
		pa->terminate();
		pa = NULL;
		pa_initialized = false;
	}
}


// --- Get_signal_color ---
Glib::ustring AudioSignalView::get_signal_color(Glib::ustring mode)
{
	if (a_audioTracks.empty())
		return "" ;

	if (!a_audioTracks[0])
		return "" ;

	return a_audioTracks[0]->get_color(mode) ;
}


// --- Show_scale ---
void AudioSignalView::show_scale(bool show)
{
    if (scale_indice<a_tracks.size() )
	{
		if (show)
			a_tracks[scale_indice]->show() ;
		else
			a_tracks[scale_indice]->hide() ;
    }
}



void AudioSignalView::set_signal_color(Glib::ustring mode, Glib::ustring color)
{
    vector<AudioTrackWidget*>::iterator it ;

	for (it=a_audioTracks.begin(); it!=a_audioTracks.end(); it++)
	{
        if (*it)
            (*it)->set_color(mode, color) ;
    }
}


// --- SaveSelection ---
bool AudioSignalView::saveSelection(float ts_start, float ts_end, char *output_file, bool warnings)
{
	bool result = false ;
	string warn = "" ;

	// -- Inits -> Output Format : PCM/S16 samples --
	float			ts_loop		= ts_start;
	float			bar_pulse	= (ts_end - ts_start) / 100.0;
	float			ts_watch	= 0.0;
	float			fraction	= 0.0;
	float			over_len	= 0.0;
	IODevice*		sf_in;
	SndFileHandler*	sf_out;

	// -- UI : ProgressBar --
	Gtk::Window*		bar_win		= new Gtk::Window;
	Gtk::VBox*			vbox		= Gtk::manage(new Gtk::VBox);
	Gtk::Label*			bar_label	= Gtk::manage(new Gtk::Label);
	Gtk::ProgressBar*	bar			= Gtk::manage(new Gtk::ProgressBar);

	bar_label->set_label( _("Extracting signal...") );
	bar->set_fraction(0.0);

	vbox->pack_start(*bar_label);
	vbox->pack_start(*bar);
	vbox->show();
	vbox->show_all_children();

	bar_win->add(*vbox);
	bar_win->resize(300, 50);
	bar_win->show();
	bar_win->set_position(Gtk::WIN_POS_CENTER_ALWAYS);

	// -- Extraction Loop --
	sf_in	= Guesser::open( a_audio_url.c_str() );
	sf_out	= new SndFileHandler();

	sf_out->m_set_sfinfo(device->m_info()->audio_channels,
						 device->m_info()->audio_sample_rate,
						 SF_FORMAT_WAV | SF_FORMAT_PCM_16);

	if ( sf_out->m_open(output_file, SFM_WRITE) && sf_in != NULL )
	{
		sf_in->m_seek(ts_loop);

		while (ts_loop < ts_end)
		{
			MediumFrame *f = sf_in->m_read();

			if (!f)
				break;

			ts_watch	+= f->ts - ts_loop;
			ts_loop		= f->ts;

			// -- Last Buffer Adjustment --
			if (ts_loop > ts_end)
			{
				over_len = (ts_loop - ts_end) * (float)sf_in->m_info()->audio_sample_rate * (float)sf_in->m_info()->audio_channels;

				if ( (over_len - floor(over_len) > 0.5) )
					f->len -= ceil(over_len);
				else
					f->len -= floor(over_len);
			}

			sf_out->m_write(f);

			// -- ProgressBar Update --
			if (ts_watch >= bar_pulse)
			{
				ts_watch = 0.0;
				fraction = (ts_loop - ts_start) / (ts_end - ts_start);

				if (fraction > 1.0)
					bar->set_fraction(1.0);
				else
					bar->set_fraction(fraction);

				GtUtil::flushGUI(false, false) ;
			}
		}

		sf_in->m_close();
		sf_out->m_close();

		delete sf_in;
		delete sf_out;

		warn =  _("The signal has been extracted successfully") ;
		result = true ;
	}
	else
	{
		// -- UI : Error Dialog --
		warn = _("The signal extraction has failed") ;
		result = false ;
	}

	// -- UI : Finish Dialog --
	if (bar)
		bar->hide();
	if (bar_win)
	{
		bar_win->hide();
		delete bar_win;
	}

	// -- UI : Display info if needed
	if (warnings && !warn.empty())
	{
		#ifdef __APPLE_
		dlg::msg( warn, toplevel );
		#else
		dlg::msg( warn);
		#endif
	}

	Log::out() << "signal extraction : res=" << result << " for file " << output_file << std::endl ;
	return result ;
}


// --- SetEditable ---
void AudioSignalView::setEditable(bool editable)
{
	vector<SegmentTrackWidget*>::iterator it ;

	for (it=a_segmentTracks.begin(); it!=a_segmentTracks.end(); it++)
	{
		SegmentTrackWidget* tw = *it ;
		if (!tw)
			continue ;
		tw->setSelectable(editable) ;
	}
}

} // namespace
