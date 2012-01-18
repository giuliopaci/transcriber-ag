/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "VideoWidget.h"
#include "ToolBox.h"
#include <glib/gthread.h>
#include "AudioWidget/AudioSignalView.h"
#include "Common/icons/Icons.h"
#include "Common/util/FormatTime.h"
#include "Common/globals.h"
#include "Common/util/Log.h"
#include <iostream>

namespace tag {

// --- VideoWidget ---
VideoWidget::VideoWidget(bool with_toolbar)
{
	// -- Defaults --
	firstResizeDone		= false;
	first_sync			= false;
	last_seek_time		= 0.0;
	seek_time			= 0.0;
	video_offset		= 0.0;
	a_beginSelection	= a_endSelection = -1.0;

	toplevel = NULL ;
	toolbox = NULL ;
	m_signalView = NULL ;
	inData = NULL;
	currentFrame = NULL;

	toolbarMode = with_toolbar ;

	initWidgets();
	initMenus();

	if (toolbarMode)
		initToolbars();

	set_skip_pager_hint(true) ;
	set_skip_taskbar_hint(true) ;

	buildUI();
	initSignals();
}


// --- ~VideoWidget ---
VideoWidget::~VideoWidget()
{
	if (toolbox)
		delete(toolbox);
}


// --- SetSignalView ---
void VideoWidget::setSignalView(AudioSignalView* view)
{
	m_signalView = view;

	toplevel = view->getToplevel();

	if (toplevel)
		set_transient_for(*toplevel);
}


// --- InitWidgets ---
void VideoWidget::initWidgets()
{
	// -- Variables --
	v_device			= NULL;
	playback			= false;
	last_audio_offset	= 0.0;
	video_ts			= 0.0;
}


// --- InitMenus ---
void VideoWidget::initMenus()
{}


// --- InitToolbars ---
void VideoWidget::initToolbars()
{
	toolbox	= new ToolBox();
}


// --- InitSignals ---
void VideoWidget::initSignals()
{
	if (toolbarMode)
	{
		toolbox->b_playVideo.signal_clicked().connect(sigc::mem_fun(this, &VideoWidget::playClicked));
		toolbox->b_NseekMinus.signal_clicked().connect(sigc::bind<bool, bool>(sigc::mem_fun(this, &VideoWidget::onSeek), true, true));
		toolbox->b_NseekPlus.signal_clicked().connect(sigc::bind<bool, bool>(sigc::mem_fun(this, &VideoWidget::onSeek), true, false));
		toolbox->b_seekMinus.signal_clicked().connect(sigc::bind<bool, bool>(sigc::mem_fun(this, &VideoWidget::onSeek), false, true));
		toolbox->b_seekPlus.signal_clicked().connect(sigc::bind<bool, bool>(sigc::mem_fun(this, &VideoWidget::onSeek), false, false));

		toolbox->b_keyframe.signal_clicked().connect(sigc::mem_fun(this, &VideoWidget::onKeyFrame));

		toolbox->b_scale->signal_value_changed().connect(sigc::mem_fun(this, &VideoWidget::onScaleUpdated));
		toolbox->b_scale->signal_format_value().connect(sigc::mem_fun(this, &VideoWidget::convertToTime));
		toolbox->b_scale->signal_change_value().connect(sigc::mem_fun(this, &VideoWidget::onSeekValueChanged));
	}

	// -- Expose Events --
	sigc_rgb = v_buffer->signal_expose_event().connect(sigc::mem_fun(this, &VideoWidget::onVideoResized));
	signal_size_request().connect(sigc::mem_fun(this, &VideoWidget::onSizeRequest));
}


// --- BuildUI ---
void VideoWidget::buildUI()
{
	Gtk::VBox* vbox = get_vbox();


	v_buffer = new VideoBuffer();
	v_buffer->set_size_request(40, 30);
	vbox->pack_start(eventBufferBox,		Gtk::PACK_EXPAND_WIDGET);
	eventBufferBox.add(*v_buffer) ;
	eventBufferBox.set_name("widget_black_bg") ;

	if (toolbarMode)
		vbox->pack_start(*toolbox,	Gtk::PACK_SHRINK,1);

	vbox->show_all_children(true);
	show_all_children();

	//> Prepare menu and accel
	if (toolbarMode)
	{
		createActionGroup();
		createUIInfo();
	}

    Icons::set_window_icon(this, ICO_TRANSCRIBER, 12) ;
}


// --- OnFocusInEvent ---
bool VideoWidget::on_focus_in_event(GdkEventFocus* e)
{
	if (toplevel)
		toplevel->present();

	// -- Focus Out --
	Glib::RefPtr<Gdk::Window> win = get_window();

	if (win)
	{
		win->set_focus_on_map(false);
		win->set_accept_focus(false);
	}

	return false;
}


// --- OnFocusOutEvent ---
bool VideoWidget::on_focus_out_event(GdkEventFocus*)
{
	return false;
}


// --- OnVideoResized (RGB) ---
bool VideoWidget::onVideoResized(GdkEventExpose* e)
{
	if (firstResizeDone)
	{
		sigc_rgb.disconnect();
		return false;
	}

	return resizeBuffer();
}


// --- OnSizeRequest (RGB) ---
void VideoWidget::onSizeRequest(Gtk::Requisition* req)
{
	resizeBuffer();
}


// --- ResizeBuffer ---
bool VideoWidget::resizeBuffer()
{
	double	buffer_ratio, base_ratio, aspect_ratio;

	if (buffer_width == v_buffer->get_width() &&
		buffer_height == v_buffer->get_height())
	{
		return false;
	}

	if (v_device != NULL)
	{
		buffer_width	= v_buffer->get_width();
		buffer_height	= v_buffer->get_height();

		// -- Size check --
		if ( (buffer_width < BUFFER_MIN_W) || (buffer_height < BUFFER_MIN_H) )
			return false;

		buffer_ratio	= (double)buffer_width / (double)buffer_height;
		base_ratio		= (double)v_device->m_info()->video_width / (double)v_device->m_info()->video_height;
		aspect_ratio	= v_device->m_info()->video_aspect_ratio;

		if (aspect_ratio == 0.0)
			aspect_ratio = 1.0;

		// -- New algorithm (assuming video ratio > 1) --
		new_video_width		= buffer_width;
		new_video_height	= (int)(new_video_width / base_ratio / aspect_ratio);

		if (buffer_ratio > 1.0)
		{
			if (new_video_height >= buffer_height)
			{
				new_video_height	= buffer_height;
				new_video_width		= (int)(new_video_height * base_ratio * aspect_ratio);
			}
		}

		v_device->m_set_dimensions(new_video_width, new_video_height);

		// -- Displaying first frame --
		if (!firstResizeDone)
		{
			TRACE << "VideoWidget --> Displaying First Frame" << std::endl ;

			MediumFrame* f = NULL;

			while(!f)
				f = v_device->m_next_frame();

			updateVideoBuffer(f);
			
			v_device->m_back();

			first_sync = false;

			if (v_pixbuf)
			{
				Glib::RefPtr<Gdk::Pixbuf> new_pixbuf = v_pixbuf->scale_simple(new_video_width, new_video_height, Gdk::INTERP_BILINEAR);
				v_buffer->set(new_pixbuf);
			}
		}


		// -- Pixbuf scaling (no playback) --
		if (!playback)
		{
			if (v_pixbuf)
			{
				Glib::RefPtr<Gdk::Pixbuf> new_pixbuf = v_pixbuf->scale_simple(new_video_width, new_video_height, Gdk::INTERP_BILINEAR);
				v_buffer->set(new_pixbuf);
			}
		}

		firstResizeDone = true;

		return true;
	}

	return false;
}


// --- OnSeekValueChanged ---
bool VideoWidget::onSeekValueChanged(Gtk::ScrollType, double in_seek)
{
	seek_time = in_seek;
}


// --- OnScaleUpdated ---
void VideoWidget::onScaleUpdated()
{
	if (last_seek_time == seek_time)
		return;

	if (seek_time > v_device->m_info()->video_duration)
		seek_time = v_device->m_info()->video_duration;

	last_seek_time = seek_time;

	double ts = v_device->m_get_nearest_ts( seek_time );

	seek(ts);

	// -- Signal for AudioSignalView --
	seekInVideo.emit(ts);
	onTimeChange.emit(ts);

	// -- Frame Refresh --
	updateVideoBuffer(v_device->m_current_frame());
}


// ----------------
// --- HANDLERS ---
// ----------------
bool VideoWidget::openFile(string path)
{
	TRACE << "VideoWidget --> Open File : " << path.c_str() << std::endl ;

	//dirtyHack: show before opening (bug otherwise)
	loadGeoAndDisplay();

	// -- Initializing New IODevice --
	v_device = Guesser::open(path.c_str(), VideoCtx);

	if (v_device == NULL) {
		TRACE << "VideoWidget --> Warning : Guesser failed to instantiate device" << std::endl ;
		saveGeoAndHide() ;
		return false ;
	}
	// -- Updating Scale Range --
	else
	{
		string name = Glib::path_get_basename(path);
		set_title(name);

		if (toolbarMode)
			toolbox->setScaleRange(0.0, v_device->m_info()->video_duration);

		// -- Video offset --
		MediumFrame* f = v_device->m_next_frame();

		video_offset = f->ts;

		v_device->m_back();

		return true;
	}
}


// --- SetPlayback ---
void VideoWidget::setPlayback(bool in_pb)
{
	if (playback == in_pb)
		return;

	playback = in_pb;

	// launching video
	if (playback)
	{
		// -- Refresh Timer --
		int refresh_ms = (int)(1000 / v_device->m_info()->video_fps);
		frame_duration = 1.0 / v_device->m_info()->video_fps;
	
		if (toolbarMode)
			toolbox->setPlay(true) ;
		sigc_conn = Glib::signal_timeout().connect(sigc::mem_fun(this, &VideoWidget::nextFrame), refresh_ms);
	}
	else
	{
		videoPlayPaused().emit(false);
		if ( !sigc_conn.empty() && sigc_conn.connected() )
		sigc_conn.disconnect();

		if (toolbarMode)	
			toolbox->setPlay(false) ;
		
		// -- Pixbuf update (native res) --
		loadNativePixbuf();
	}
}


// --- PlayClicked ---
void VideoWidget::playClicked()
{
	setPlayback(!playback);
	videoPlayPause.emit(playback);
}


// --- OnSeek ---
void VideoWidget::onSeek(bool Nmode, bool rewind)
{
	int n = 1;

	if (Nmode && toolbarMode)
	{
		string text = toolbox->nbframe_entry.get_text();
		n = atoi(text.c_str());

		if (n==0)
			n=1;
	}

	if (rewind)
		seekMinus(n);
	else
		seekPlus(n);
}


// --- SeekMinus ---
void VideoWidget::seekMinus(int nframe)
{
	// -- Selection --
	if ( (a_beginSelection != -1.0) &&
		 ( (video_ts - (frame_duration * nframe)) <= a_beginSelection ) )
	{
		TRACE << "VideoWidget --> Selection : Begin Reached" << std::endl ;
		return;
	}

	if (nframe > 1)
		updateVideoBuffer( v_device->m_previous_frame(nframe) );
	else
		updateVideoBuffer( v_device->m_previous_frame() );

	onTimeChange.emit(video_ts);
}


// --- SeekPlus ---
void VideoWidget::seekPlus(int nframe)
{
	// -- Selection --
	if ( (a_endSelection != -1.0) &&
		 ( (video_ts + (frame_duration * nframe)) >= a_endSelection ) )
	{
		TRACE << "VideoWidget --> Selection : End Reached" << std::endl ;
		return;
	}

	if (nframe > 1)
		updateVideoBuffer( v_device->m_next_frame(nframe) );
	else
		updateVideoBuffer( v_device->m_next_frame() );

	// -- EOF --
	if (video_ts >= v_device->m_info()->video_duration)
	{
		TRACE << "VideoWidget --> Stopping Playback" << std::endl ;
		setPlayback(false);
	}
	onTimeChange.emit(video_ts);
}


// --- SeekTo ---
void VideoWidget::seek(double offset)
{
	if (v_device != NULL)
	{
		bool seek_ok = v_device->m_seek(offset);

		if (seek_ok)
			video_ts = offset;

		if (!first_sync)
			first_sync = true;

		if (video_ts < video_offset)
			first_sync = false;
	}
	else
		TRACE << "VideoWidget --> Unable to seek : Device is NULL" << std::endl ;
}


// --- Pause ---
void VideoWidget::pause()
{}


// --- Stop ---
void VideoWidget::stop()
{
	if (playback)
	{
		if (toolbarMode)
			toolbox->setPlay(false) ;
		
		playback = false;
	}
}


// --- NextFrame ---
bool VideoWidget::nextFrame()
{
	if (playback)
	{
		float audio_ts = m_signalView->getActualCursor();

		// -- Selection Check --
		if ( (a_endSelection != -1.0) && (video_ts >= (a_endSelection - frame_duration)) )
		{
			stop();
			return false;
		}

		if (currentFrame)
			currentFrame = v_device->m_current_frame();
		else
			currentFrame = v_device->m_next_frame();
	
		if (!currentFrame)
			return playback;
			
		video_ts = currentFrame->ts;

		// ********************
		// *** SYNCHRO ZONE ***
		// ********************

		// -- Video > Audio : waiting... --
		if (video_ts - audio_ts > frame_duration)
			return true;

		// -- Audio > Video --
		while (audio_ts > video_ts)
		{
			currentFrame = v_device->m_next_frame();

			if (!currentFrame)
				return playback;

			video_ts = currentFrame->ts;
		}

		// -- Display --
		updateVideoBuffer(currentFrame);

		if (video_ts >= v_device->m_info()->video_duration)
		{
			setPlayback(false);
			v_device->m_back();
		}

		onTimeChange.emit(video_ts);
	}

	return playback;
}


// --- OnKeyFrame ---
void VideoWidget::onKeyFrame()
{
	m_signalKeyframe.emit(video_ts) ;
}


// ---------------
// --- DRAWING ---
// ---------------

// --- UpdateVideoBuffer ---
bool VideoWidget::updateVideoBuffer(MediumFrame *in_frame)
{
	if (in_frame == NULL)
	{
		TRACE << "VideoWidget : NULL frame -> We should stop...\n" << std::endl ;
		setPlayback(false);
		Glib::ustring msg = _("Error while decoding frame... Aborted.") ;
		signalError().emit(msg) ;
		return false ;
	}

	// -- Pixbuf (RGB Mode) --
	bool    has_alpha       = false;
	int     row_stride      = in_frame->v_linesize[0];
	int     bits_per_sample = 8;

	// -- Memory Copy --
	if (inData)
		delete[] inData;
		
	inData = new uint8_t[in_frame->vlen];

	memcpy(inData, in_frame->v_samples[0], in_frame->vlen);

	v_pixbuf    = Gdk::Pixbuf::create_from_data(reinterpret_cast<guint8*> (inData),
												Gdk::COLORSPACE_RGB,
												has_alpha,
												bits_per_sample,
												in_frame->v_width,
												in_frame->v_height,
												row_stride);

	v_buffer->set(v_pixbuf);


	// -- Updating Scale --
	video_ts = in_frame->ts;

	if (toolbarMode)
	{
		toolbox->b_scale->set_value(in_frame->ts);
		toolbox->b_scale->queue_draw();
	}

	return true;
}


// --- LoadNativePixbuf ---
void VideoWidget::loadNativePixbuf()
{
	if (!v_device)
		return;

	double v_width	= v_device->m_info()->video_width;
	double v_height	= v_device->m_info()->video_height;

	v_device->m_set_dimensions(v_width, (int)(v_height / v_device->m_info()->video_aspect_ratio));

	MediumFrame* fs = v_device->m_get_rgb_frame(video_ts);

	updateVideoBuffer(fs);

	v_device->m_set_dimensions(new_video_width, new_video_height);

	if (v_pixbuf)
	{
		Glib::RefPtr<Gdk::Pixbuf> new_pixbuf = v_pixbuf->scale_simple(new_video_width, new_video_height, Gdk::INTERP_BILINEAR);
		v_buffer->set(new_pixbuf);
	}
}


// --- Synchronize ---
bool VideoWidget::synchronize(double audio_pts)
{
	v_device->m_sync(audio_pts);

	return true;
}


// --- SetScaleOffset ---
void VideoWidget::setScaleOffset(double ts, bool stop)
{
	if (stop)
		setPlayback(false);

	double new_ts = v_device->m_get_nearest_ts(ts);

	onTimeChange.emit(new_ts);

	if (toolbarMode)
		toolbox->b_scale->set_value(new_ts);

	seek(new_ts);

	updateVideoBuffer(v_device->m_current_frame());
}


// --- ConvertToTime ---
Glib::ustring VideoWidget::formatDisplayTime(double ts)
{
	return fromAGTIMEtoTXML(ts, false, v_device) + " (" + FormatTime(ts) + ")";
}

// --- ConvertToTime ---
Glib::ustring VideoWidget::convertToTime(double ts)
{
	return formatDisplayTime(ts) ;
}

// --- FromAGTIMEtoTXML ---
string VideoWidget::fromAGTIMEtoTXML(float AGTime, bool force_previous, IODevice* device)
{
	int relative_frame, relative_time ;

	if (device)
	{
		double fps = device->m_info()->video_fps ;

		if (force_previous)
			AGTime = AGTime - 1/fps ;

		device->m_get_relative_time(AGTime, relative_time, relative_frame) ;
		int absolute = device->m_get_frame_number(AGTime) ;

		return FormatTime::getTXMLtime(relative_time, relative_frame, fps, absolute) ;
	}
	else
		return "" ;
}


//------------------------------------------------------------------------------
//---------------------------Menu & Shortcuts methods --------------------------
//------------------------------------------------------------------------------

void VideoWidget::createActionGroup()
{
	ui_actions	= Gtk::ActionGroup::create("video_signal");
	completeActionGroup(ui_actions) ;
}

void VideoWidget::completeActionGroup(Glib::RefPtr<Gtk::ActionGroup> action_group)
{
	//> PLAY
	action_group->add(Gtk::Action::create("video_signal_play", Gtk::Stock::MEDIA_PLAY),
					Gtk::AccelKey("<mod2>Escape"),
					sigc::mem_fun(*this, &VideoWidget::playClicked)) ;

	action_group->add(Gtk::Action::create("video_signal_pause", Gtk::Stock::MEDIA_PAUSE),
				Gtk::AccelKey("<mod2>Escape"),
				sigc::mem_fun(*this, &VideoWidget::playClicked)) ;

	//> MOVE
	action_group->add( Gtk::Action::create("video_signal_forward", Gtk::Stock::MEDIA_FORWARD),
					Gtk::AccelKey("<control>Right"),
					sigc::bind<bool,bool>(sigc::mem_fun(*this, &VideoWidget::onSeek), false, false)) ;

	action_group->add( Gtk::Action::create("video_signal_rewind", Gtk::Stock::MEDIA_REWIND),
					Gtk::AccelKey("<control>Left"),
					sigc::bind<bool,bool>(sigc::mem_fun(*this, &VideoWidget::onSeek), false, true)) ;

	action_group->add( Gtk::Action::create("video_signal_forward_n", _("forward by N frames"), ""),
					Gtk::AccelKey("<control>Up"),
					sigc::bind<bool,bool>(sigc::mem_fun(*this, &VideoWidget::onSeek), true, false)) ;

	action_group->add( Gtk::Action::create("video_signal_rewind_n", _("rewind by N frames"), ""),
					Gtk::AccelKey("<control>Down"),
					sigc::bind<bool,bool>(sigc::mem_fun(*this, &VideoWidget::onSeek), true, true)) ;
}

void VideoWidget::createUIInfo()
{
	ui_info =
	"<ui>"
	"  <menubar name='MenuBar'>"
	"    <menu action='video_signal'>"
	"      <menuitem action='video_signal_play'/>"
	"      <menuitem action='video_signal_pause'/>"
	"      <menuitem action='video_signal_forward'/>"
	"      <menuitem action='video_signal_rewind'/>"
	"      <menuitem action='video_signal_forward_n'/>"
	"      <menuitem action='video_signal_rewind_n'/>"
	"      <separator/>"
	"    </menu>"
	"  </menubar>"
	"</ui>";
}

std::vector<Glib::ustring> VideoWidget::getUIMenuItems()
{
	std::vector<Glib::ustring> v ;
	v.push_back("      <menuitem action='video_signal_play'/>") ;
	v.push_back("      <menuitem action='video_signal_pause'/>") ;
	v.push_back("      <menuitem action='video_signal_forward'/>") ;
	v.push_back("      <menuitem action='video_signal_rewind'/>") ;
	v.push_back("      <menuitem action='video_signal_forward_n'/>") ;
	v.push_back("      <menuitem action='video_signal_rewind_n'/>") ;
	return v ;
}

// --------------
// --- EVENTS ---
// --------------

bool VideoWidget::on_delete_event(GdkEventAny* event)
{
	m_signalCloseButton.emit() ;
	return true ;
}

//**************************************************************************************
//										GEOMETRY INTERFACE
//**************************************************************************************

void VideoWidget::getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	get_position(pos_x, pos_y) ;
	panel = -1 ;
	get_size(size_xx, size_yy) ;
}

void VideoWidget::setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel)
{
	if (pos_x>0 && pos_y>0)
		move(pos_x, pos_y) ;
	if (size_xx>0 && size_yy>0)
		resize(size_xx, size_yy) ;
}

Glib::ustring VideoWidget::getWindowTagType()
{
	return SETTINGS_VIDEO_PLAYER ;
}

int VideoWidget::loadGeoAndDisplay(bool rundlg)
{
	loadPos() ;
	show() ;
	return 1 ;
}

void VideoWidget::saveGeoAndHide()
{
	if (is_visible())
		savePos() ;
	hide() ;
}

void VideoWidget::getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	size_xx = 500 ;
	size_yy = 400 ;
	pos_x = 300 ;
	pos_y = 200 ;
}

} //namespace

