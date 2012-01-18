/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class AudioWaveformWidget
 *
 * AudioWaveformWidget...
 */

#include "AudioWidget.h"

#include <gdkmm/window.h>
#include <gdk/gdk.h>
#include <sys/time.h>
#include <sstream>
#include <valarray>

#include "Common/util/FormatTime.h"
#include "Common/widgets/GtUtil.h"
#include "Common/FileInfo.h"

#include "MediaComponent/base/Guesser.h"

namespace tag {

bool AudioWaveformWidget::SAVE_PEAKS = false;
bool AudioWaveformWidget::ABSOLUTE_PEAKS_NORM = true;


// --- AudioWaveformWidget ---
AudioWaveformWidget::AudioWaveformWidget(IODevice* in_device, float* p_peaks) :
	Gtk::DrawingArea(), a_currentPeaks(NULL)
{
	stereo		= false;
	drawnOnce	= false;

	// -- Inits --
	device					= in_device;
	a_lastBeginSegmentTrack = NULL;
	a_showLastBegin			= false;
	a_loading				= true;
	a_enabled				= true;
	a_abortLoading			= false;
	a_cursorIn				= false;
	a_peaksDirectory		= NULL;
	a_numChannel			= 1;
	a_streaming				= false;
	a_backextent			= false;
	a_trackingMode			= false;
	a_emit_on_release		= false;

	if (device != NULL)
	{
		a_samplesCount		= device->m_info()->audio_samples;
		a_samplingRate		= device->m_info()->audio_sample_rate;
		a_duration			= device->m_info()->audio_duration;
	}

	a_cursorScroll			= 0.0;
	a_currentSample			= 0.0;
	a_peaksLength			= 0.0;
	a_selection1			= -1.0;
	a_selection2			= -1.0;
	a_beginSelection		= -1.0;
	a_endSelection			= -1.0;

	a_w						= (int)(a_duration / AudioWidget::AUDIO_ZOOM_MAX);
	a_h						= 0;
	a_beginVisible			= 0;
	a_endVisible			= 0;
	a_selectable			= true;
	a_offset				= 0.0;
	p_offset				= 0;
	a_lastPressedButton 	= -1;
	a_cursorSize			= 1;
	a_cursorColor			= "yellow";
	lastSegment				= -1.0;
	min_pixels_sel			= 4;

	//DEFAULT COLOR VALUES
	color_wave_bg_active.insert(color_wave_bg_active.end(), 0.80) ;
	color_wave_bg_active.insert(color_wave_bg_active.end(), 0.65) ;
	color_wave_bg_active.insert(color_wave_bg_active.end(), 0.65) ;

	color_wave_bg.insert(color_wave_bg.end(), 0.80) ;
	color_wave_bg.insert(color_wave_bg.end(), 0.80) ;
	color_wave_bg.insert(color_wave_bg.end(), 0.80) ;

	color_wave_fg.insert(color_wave_fg.end(), 0.00) ;
	color_wave_fg.insert(color_wave_fg.end(), 0.00) ;
	color_wave_fg.insert(color_wave_fg.end(), 1.00) ;

	color_selection.insert(color_selection.end(), 0.35) ;
	color_selection.insert(color_selection.end(), 0.35) ;
	color_selection.insert(color_selection.end(), 0.35) ;

	color_disabled.insert(color_disabled.end(), 0.80) ;
	color_disabled.insert(color_disabled.end(), 0.80) ;
	color_disabled.insert(color_disabled.end(), 0.80) ;

	color_tip_bg.insert(color_tip_bg.end(), 1.00) ;
	color_tip_bg.insert(color_tip_bg.end(), 1.00) ;
	color_tip_bg.insert(color_tip_bg.end(), 1.00) ;

	color_tip_fg.insert(color_tip_fg.end(), 0.00) ;
	color_tip_fg.insert(color_tip_fg.end(), 0.00) ;
	color_tip_fg.insert(color_tip_fg.end(), 0.00) ;

	color_cursor.insert(color_cursor.end(), 0.90) ;
	color_cursor.insert(color_cursor.end(), 1.00) ;
	color_cursor.insert(color_cursor.end(), 0.10) ;

	color_segmentEnd.insert(color_segmentEnd.end(), 1.00) ;
	color_segmentEnd.insert(color_segmentEnd.end(), 0.20) ;
	color_segmentEnd.insert(color_segmentEnd.end(), 0.00) ;

	// -- Global Variables --
	w = 0;
	h = 0;

	// -- Cairo Font Settings --
	font_options.set_hint_style(Cairo::HINT_STYLE_NONE);
	font_options.set_hint_metrics(Cairo::HINT_METRICS_OFF);

	// Signals
	if (p_peaks == NULL && device != NULL)
	{
		int samplingRate	= device->m_info()->audio_sample_rate;
		float duration		= device->m_info()->audio_duration;
		int ws				= (int)(duration / AudioWidget::AUDIO_ZOOM_MAX);

        a_arraySize         = ws + 1;
        a_maxPeaks			= CALLOC(a_arraySize, float);
	}
	else
	{
		a_maxPeaks = p_peaks;
	}

	a_currentPeaks = a_maxPeaks;

	a_currentZoom = 1;
	a_currentVZoom = 1.0;
	a_zoomMax = 1;

	add_events(Gdk::BUTTON_PRESS_MASK);
	add_events(Gdk::BUTTON_MOTION_MASK);
	add_events(Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::ENTER_NOTIFY_MASK);
	add_events(Gdk::LEAVE_NOTIFY_MASK);
	queue_draw();
}


// --- ~AudioWaveformWidget ---
AudioWaveformWidget::~AudioWaveformWidget()
{
	if (a_loading)
	{
		a_abortLoading = true;

		while (a_loading)
			USLEEP((int)(10*1000));
	}

	if ( a_currentPeaks != a_maxPeaks )
		FREE(a_currentPeaks);

	FREE(a_maxPeaks);
}



// --- RetrieveStreamPeaks ---
float* AudioWaveformWidget::retrieveStreamPeaks(IODevice *iodev, int chan_ID, bool silentMode)
{
	// -- Streaming Case --
	if (a_streaming)
	{
		// -- Extracting Peak File --
		ostringstream	oss;
		string			peak_file = a_path.substr(0, a_path.find_last_of("."));

		oss << peak_file << "_" << chan_ID << ".pks";

		// -- Checking Size --
		FileInfo info( oss.str() );

		int size	= (int)(iodev->m_info()->audio_duration / AudioWidget::AUDIO_ZOOM_MAX);
		int c_size	= info.size() / sizeof(float);

		if (size != c_size)
		{
			Log::out() << "WARNING !!! Peaks Size Check Failed --> " << size << " / " << c_size << endl;
			size			= c_size;
			a_peaksLength	= c_size;
		}
		else
		{
			a_peaksLength	= size;
		}

		// -- Loading Peak File --
		FILE* fd = fopen(oss.str().c_str(), "rb");

		if (fd != NULL)
		{
			Log::out() << "Streaming Mode --> Loading Existing Peak File : " << oss.str() << endl;

			float* tab	= new float[size];
			int cur		= 0;
			int n		= 0;
			while (cur != size && !feof(fd))
			{
				n = fread(tab + cur, sizeof(float), BUFFER_SIZE, fd);
				cur += n;
			}

			fclose(fd);
			TRACE_D << "Streaming Mode -->Loading peaks done in " << tim.elapsed() << endl;

			return tab;
		}
	}

	if (!SAVE_PEAKS || silentMode)
	{
		return computeStreamPeaks(iodev, chan_ID, silentMode);
	}

	// -- Stockage des fichiers de pics --
	ostringstream oss;

	MediumInfo* s_info	= iodev->m_info();
	string		pks		= iodev->m_medium();

	oss << pks.substr(0, pks.find_last_of("."));
	oss << "_" << chan_ID << ".pks";

	// -- Loading Peaks --
	FILE* fd = fopen(oss.str().c_str(), "rb");

	if (fd != NULL)
	{
		// -- Checking Size --
		FileInfo info( oss.str() );

		int size	= (int)(s_info->audio_duration  / AudioWidget::AUDIO_ZOOM_MAX);
		int c_size	= info.size() / sizeof(float);

		if (size != c_size)
		{
			TRACE << "WARNING !!! Peaks Size Check Failed --> " << size << " / " << c_size << endl;
			size			= c_size;
			a_peaksLength	= c_size;
		}
		else
		{
			a_peaksLength	= size;
		}

		TRACE << "Loading Existing Peak File : " << oss.str() << endl;

		float* tab	= new float[size];
		int cur		= 0;

		while (cur != size && !feof(fd))
		{
			int n = fread(tab + cur, sizeof(float), BUFFER_SIZE, fd);
			cur += n;
		}
		fclose(fd);

		TRACE_D << "Loading peaks done in " << tim.elapsed() << endl;
		return tab;
	}
	else
	{
		TRACE << "Saving peaks into file" << std::endl ;

		float* tab	= computeStreamPeaks(iodev, chan_ID, silentMode);
		int size	= (int)(s_info->audio_duration  / AudioWidget::AUDIO_ZOOM_MAX);

		FILE* fd2 = fopen(oss.str().c_str(), "wb");

		if (fd2 != NULL)
		{
			int cur = 0;

			while (cur < size)
			{
				int toRead = BUFFER_SIZE;

				if (size-cur < BUFFER_SIZE)
					toRead = size-cur;
				int n = fwrite(tab + cur, sizeof(float), toRead, fd2);
				cur += n;
			}
			fflush(fd);
		}
		return tab;
	}
}

// --- ComputeStreamPeaks ---
float* AudioWaveformWidget::computeStreamPeaks(IODevice *iodev, int chan_ID, bool silentMode)
{
	chan_ID = a_numChannel;

	// -- Streaming Case : no device yet --
	if (iodev == NULL)
		return NULL;

	// -- Gathering Stream Info --
	MediumFrame*	frame;
	MediumInfo*		s_info;

	// -- First Frame <> Init --
	iodev->m_seek(0.0);
	frame	= iodev->m_read();
	s_info	= iodev->m_info();

	long n_samples		= s_info->audio_samples;
	int sample_rate		= s_info->audio_sample_rate;
	int channels		= s_info->audio_channels;
	int frame_size		= s_info->audio_frame_size;
	float duration		= s_info->audio_duration;

	// -- Inits --
	int ws = (int)(duration / AudioWidget::AUDIO_ZOOM_MAX);
	int ns = (int)(roundf(AudioWidget::AUDIO_ZOOM_MAX * sample_rate));
	int is = 0;

	float* maxPeaks = (float*)calloc(ws+1, sizeof(float));
	float* buffer;

	a_peaksLength = ws + 1;

	// -- Buffering Process --
	int indice		= 0;
	int stop		= 0;
	bool finTroncon = false;
	int16_t* tmpBuf;

	TRACE << "Computing Waveform for Track no. " << chan_ID << std::endl ;

	buffer = new float[frame_size];

	// -- Computing Loop --
	while (frame != NULL)
	{
		// -- Frame Buffer --
		for (int idx=0; idx<frame_size; idx++)
			buffer[idx] = (float)frame->samples[channels * idx + chan_ID] ;

		int n = frame_size;
		int nbEchPerPeak = ns;
		int debut = 0;
		int fin = nbEchPerPeak - (indice % nbEchPerPeak) - 1;

		if ( n < (frame_size * channels) )
			stop = 1;

		if (fin > n-1)
			fin = n-1;
		else
			finTroncon = true;

		while (true)
		{
			for (int j = debut; j < fin; j++)
			{
				float sample = fabs(buffer[j]);

				if (is > ws)
					break;

				if (sample > maxPeaks[is])
				{
					maxPeaks[is] = sample ;
				}
			}

			if (finTroncon)
			{
				is++;
				finTroncon = false;
			}

			if (fin == n-1)
				break;

			debut = fin+1;

			fin = debut + nbEchPerPeak - 1;

			if (fin > n-1)
				fin = n-1;
			else
				finTroncon = true;
		}

		indice += n;
		frame = iodev->m_read();
	}

	return maxPeaks;
}


// --- NormalizePeaks ---
void AudioWaveformWidget::normalizePeaks(int ws)
{
	Glib::Timer tim ;
	tim.start() ;

	float max = MAX_PEAK;

	// -- Peaks Check --
	if ( (ws > a_peaksLength) && (a_peaksLength > 0) )
		ws = a_peaksLength;

	if (!ABSOLUTE_PEAKS_NORM)
	{
		max = 0 ;
		for (int j = 0; j < ws; j++)
		{
			if (a_maxPeaks[j] > max)
				max = a_maxPeaks[j];
		}
	}

	// -- Size Check --
	for (int j = 0; j < ws; j++)
		a_maxPeaks[j] = a_maxPeaks[j]/1.10/max;
}


// --- ComputePeaks ---
void AudioWaveformWidget::computePeaks(bool silentMode)
{
	Glib::Timer tim;

	Log::out() << "Computing peaks..." << endl;

	if ( a_currentPeaks != NULL && a_currentPeaks != a_maxPeaks )
		FREE(a_currentPeaks) ;

	FREE(a_maxPeaks) ;
	a_loading = true ;

	Log::out() << "Retrieving peaks..." << std::endl ;
	a_maxPeaks = retrieveStreamPeaks(device, a_numChannel, silentMode) ;

	if (device)
	{
		MediumInfo*	s_info = device->m_info() ;
		double duration = s_info->audio_duration ;
		int ws = (int)(duration / AudioWidget::AUDIO_ZOOM_MAX) ;

		Log::out() << "Normalizing peaks..." << std::endl ;
        normalizePeaks(a_peaksLength);
	}
	else
		Log::err()  << "No device !! can't normalize, even partl" << endl ;

	a_mut1.lock();
	a_currentPeaks = a_maxPeaks;
	a_mut1.unlock();
	a_loading = false;

	Log::out() << "Peaks computed in " << tim.elapsed() << endl;
}


/** Peaks Loading check */
bool AudioWaveformWidget::checkPeaksLoading()
{
	bool loadStatus = true;

	a_mut1.lock();
	loadStatus = a_loading;
	a_mut1.unlock();
	
	if(loadStatus)
	{
		a_mut1.lock();
		loadStatus = a_loading;
		a_mut1.unlock();
		
		return true;
	}

	Log::out() << "Peaks computed... Emitting peaksReady signal!" << endl;
	
	if (a_maxPeaks != NULL)
		peaksReady(true);
	else
		peaksReady(false);
		
	return false;
}


// --- PeaksReady ---
bool AudioWaveformWidget::peaksReady(bool ok)
{
	signalPeaksReady().emit(ok) ;
	return false;
}


// --- On_button_press_event ---
bool AudioWaveformWidget::on_button_press_event(GdkEventButton* p_event)
{
	a_lastPressedButton = p_event->button;

	if (a_lastPressedButton == 3)
	{
		GdkWindow* window = ((GdkEventKey*)p_event)->window;
		int x = -1;
		int y = -1;
		gdk_window_get_origin(window, &x, &y);
		x += (int)((GdkEventButton*)p_event)->x;
		y += (int)((GdkEventButton*)p_event)->y;
		a_signalPopulatePopup.emit(x, y);
		return true;
	}

	int x = (int)roundf(p_event->x);
	x++;
	float secs = a_cursorScroll + ((float)x * AudioWidget::AUDIO_ZOOM_MAX * (float)a_currentZoom);

	if (secs > a_duration + a_offset) return false;

	// -- Tracking Mode --
	if (p_event->state & GDK_CONTROL_MASK)
		a_trackingMode = true;

	if (a_selectable)
	{
		a_signalSelecting.emit(true, true);

		// -- Existing Selection --
		if ((a_beginSelection != -1.0) || (a_endSelection != -1.0))
		{
			// -- Selection Extension (with Shift) --
			if ((p_event->state & GDK_SHIFT_MASK))
			{
				if (a_backextent)
				{
					if (secs >= a_endSelection)
					{
						a_backextent		= false;
						a_beginSelection	= a_endSelection;
						a_endSelection		= secs;
					}
					else
					{
						a_beginSelection = secs;
					}
				}
				else
				{
					if (secs < a_beginSelection)
					{
						a_backextent		= true;
						a_endSelection		= a_beginSelection;
						a_beginSelection	= secs;
					}
					else
					{
						a_endSelection = secs;
					}
				}
			}
			else
			{
				a_endSelection		= -1.0;
				a_beginSelection	= -1.0;
				a_backextent		= false;
			}

			a_signalSelectionChanged.emit(a_beginSelection, a_endSelection);
		}
		else
		{
			// -- Normal State (with SHIFT) --
			if (p_event->state & GDK_SHIFT_MASK)
			{
				if (a_currentSample >= secs)
				{
					a_beginSelection 	= secs;
					a_endSelection		= a_currentSample;
				}
				else
				{
					a_beginSelection	= a_currentSample;
					a_endSelection		= secs;
				}

				a_signalSelectionChanged.emit(a_beginSelection, a_endSelection);
			}
		}

		a_selection1 = secs;
		a_selection2 = secs;
	}

	a_currentSample = secs;
	a_signalAudioTrackSelected.emit() ;
	a_signalCursorChanged.emit(secs) ;
	queue_draw();

	return false;
}


// --- On_motion_notify_event ---
bool AudioWaveformWidget::on_motion_notify_event(GdkEventMotion* p_event)
{
	if (a_lastPressedButton == 3)
		return false;

	int x = (int)roundf(p_event->x);
	x++;
	if (x < 0)
		x = 0;

	float secs = a_cursorScroll + ((float)x  * AudioWidget::AUDIO_ZOOM_MAX * (float)a_currentZoom);

	if (a_trackingMode)
	{
		if ( (secs + a_offset) <= a_duration )
		{
			a_signalCursorChanged.emit(secs);
			a_currentSample = secs;
		}

		return false;
	}

	if (a_selectable)
	{
		a_signalSelecting.emit(true, a_selection2 < a_selection1);

		if (secs > a_duration + a_offset)
			secs = a_duration + a_offset;

		a_selection2 = secs;

		if (a_selection1 < a_selection2)
		{
			a_beginSelection = a_selection1;
			a_endSelection = a_selection2;

			if (a_currentSample < a_beginSelection)
			{
				a_currentSample = a_beginSelection;
				a_signalCursorChanged.emit(a_beginSelection);
			}
		}
		else
		{
			a_beginSelection	= a_selection2;
			a_endSelection		= a_selection1;
			a_currentSample		= secs;

			if (video_mode)
				a_emit_on_release = true;
			else
				a_signalCursorChanged.emit(secs);
		}
		a_signalSelectionChanged.emit(a_beginSelection, a_endSelection);
	}
	else
	{
		if (secs > a_duration + a_offset)
			return false;

		a_currentSample = secs;
		a_signalCursorChanged.emit(secs);
	}

	return false;
}


// --- On_button_release_event ---
bool AudioWaveformWidget::on_button_release_event(GdkEventButton* p_event)
{
	// -- Tracking Mode --
	if (a_trackingMode)
		a_trackingMode = false;

	// -- Checking Selection Size --
	if ( (a_beginSelection != -1.0) || (a_endSelection != -1.0) )
		if (endSelection - beginSelection < min_pixels_sel)
		{
			a_beginSelection = -1.0;
			a_endSelection = -1.0;
			a_signalSelectionChanged.emit(-1.0, -1.0);
		}

	if (a_emit_on_release)
	{
		a_signalCursorChanged.emit(a_currentSample);
		a_emit_on_release = false;
	}

	a_signalSelecting.emit(false, false);
	return false;
}


// --- On_enter_notify_event ---
bool AudioWaveformWidget::on_enter_notify_event(GdkEventCrossing* p_event)
{
	a_cursorIn = true;
	queue_draw();
	return false;

}


// --- On_leave_notify_event ---
bool AudioWaveformWidget::on_leave_notify_event(GdkEventCrossing* p_event)
{
	a_cursorIn = false;
	queue_draw();

	return false;
}


// --- On_size_allocate ---
void AudioWaveformWidget::on_size_allocate(Gtk::Allocation& p_allocation)
{
	Gtk::DrawingArea::on_size_allocate(p_allocation);
	a_signalSizeChanged.emit(p_allocation.get_width());
}


// --- On_expose_event (Cairo Version) ---
bool AudioWaveformWidget::on_expose_event(GdkEventExpose* e)
{
	if (e == NULL)
		return false;

	// -- Variables --
	window = get_window();
	window->get_size(w, h);

	h2				= h/2;
	w2				= (int)(a_duration / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	begin			= (int)((a_cursorScroll - a_offset) / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	beginSelection	= (int)((a_beginSelection - a_offset) / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	endSelection	= (int)((a_endSelection - a_offset) / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	cursor			= (int)((a_currentSample - a_offset) / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	end				= begin + w;

	// -- Initializing Cairo --
	cr = window->create_cairo_context();
	cr->set_antialias(Cairo::ANTIALIAS_NONE);
	cr->set_line_width(1.0);
	cr->rectangle(e->area.x, e->area.y, e->area.width, e->area.height);
	cr->clip();

	// -- Fonts --
	cr->set_font_size(12);
	cr->set_font_options(font_options);

	// -- Waveform Loading --
	if (a_loading)
	{
		cr->save();
		cr->set_source_rgb(0.0, 0.0, 0.0);
		cr->move_to(15, h/2);
		cr->show_text("Loading Waveform...");
		cr->restore();
		return false;
	}

	// -- Grey Background --
	if (a_selected)
		cr->set_source_rgb(color_wave_bg_active[0], color_wave_bg_active[1], color_wave_bg_active[2]);
	else
		cr->set_source_rgb(color_wave_bg[0], color_wave_bg[1], color_wave_bg[2]);
	cr->paint();

	// -- Waveform --
	cr->set_source_rgb(color_wave_fg[0], color_wave_fg[1], color_wave_fg[2]);
	for (int i = begin + e->area.x; i <= begin + e->area.x + e->area.width; i++)
		drawPoint(i);
	cr->stroke();

	// -- Selection Rectangle --
	if ((a_beginSelection != -1.0) || (a_endSelection != -1.0))
	{
		cr->set_source_rgba(color_selection[0], color_selection[1], color_selection[2], 0.5);

		int bs = beginSelection - begin;
		int es = endSelection - begin;

		// -- Checking bounds --
		if (bs < e->area.x)
			bs = e->area.x;

		if (es > (e->area.x + e->area.width))
			es = e->area.x + e->area.width;

		cr->rectangle(bs, 0, es - bs, h);
		cr->fill();
		cr->stroke();
	}

	// -- Cursor --
	cr->set_source_rgb(color_cursor[0], color_cursor[1], color_cursor[2]);

	#ifndef WIN32
		cursor++;
	#endif

	cr->move_to(cursor - begin, 0);
	cr->line_to(cursor - begin, h);
	cr->stroke();


	// -- Last Segment --
	/*
	if (a_showLastBegin && a_lastBeginSegmentTrack != NULL)
	{
		float lastBegin = a_lastBeginSegmentTrack->getLastBegin(a_currentSample);

		if (lastBegin != -1)
		{

			int lastB = (int)((lastBegin - a_offset) / AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);

			if (lastB-begin > -32000 && lastB-begin < 32000)
			{
				valarray<double> dashes;

				dashes << 1.0 << 0.0 << 1.0 << 0.0 << 1.0 << 0.0;
				dashes << 1.0 << 0.0 << 1.0 << 0.0 << 1.0;

				cr->save();
				cr->set_source_rgb(color_segmentEnd[0], color_segmentEnd[1], color_segmentEnd[2]);
				cr->set_dash(dashes, 0.0);
				cr->move_to(lastB - begin, 0);
				cr->line_to(lastB - begin, h);
				cr->restore();

				// Erasing previous segment line
				if (lastSegment >= 0.0 && lastSegment != lastB)
				{
					queue_draw_area(lastSegment - begin, 0, lastSegment - begin + 1, h);
				}

				lastSegment = lastB;
			}
		}
	}
	*/

	// -- Custom Tooltip --
	selectionTip();

	// -- Enabled / Disabled --
	if (!a_enabled)
	{
		cr->set_source_rgba(color_disabled[0], color_disabled[1], color_disabled[2], 0.9);
		cr->rectangle(e->area.x, e->area.y, e->area.width, e->area.height);
		cr->fill();
	}

	return true;
}


// --- SelectionTooltip ---
void AudioWaveformWidget::selectionTip()
{
	float	sel = a_endSelection - a_beginSelection;
	int		bs = (int) (beginSelection - begin);
	int		es = (int) (endSelection - begin);
	int		rect_x, rect_y;
	int		x_off, y_off;
	ostringstream ostr;

	// -- Computing String --
	if (sel != 0 && a_cursorIn)
	{
		if (sel < 1)
		{
			int ms = ((int)roundf(sel * 1000))%1000;
			ostr << ms << " ms";
		}
		else
		{
			ostr << FormatTime(sel).c_str();
		}

		// -- Display (with Cairo) --
		string str = ostr.str();

		cr->get_text_extents(str, extents);
		rect_y = h2 / 2;
		x_off = (int)(extents.width / 4);
		y_off = (int)(extents.height / 4);


		// -- Computing Visible Rect --
		if (bs < 0)
			bs = 0;

		if (es > w)
			es = w;

		// -- Drawing Rectangle & Text --
		if (extents.width > es - bs)
			rect_x = bs;
		else
			rect_x = (int)( bs + ((es - bs) - extents.width) / 2 );

		cr->save();
		cr->set_source_rgb(color_tip_bg[0], color_tip_bg[1], color_tip_bg[2]);
		cr->rectangle(rect_x - x_off, rect_y - y_off,
						extents.width + 2*x_off, extents.height + 2*y_off);
		cr->fill();
		cr->set_source_rgb(color_tip_fg[0], color_tip_fg[1], color_tip_fg[2]);
		cr->move_to(rect_x, rect_y + extents.height / 2 + 2*y_off);
		cr->show_text(str);
		cr->restore();
	}
}


// --- DrawPoint ---
void AudioWaveformWidget::drawPoint(int i)
{
	float p;

    if (i < a_w && i > 0)
		p = a_currentPeaks[i] * (float)h2*a_currentVZoom;
	else
		p = 0.0;

	if (p > h2)
		p = h2;

	// 1 - Tracé du point de la waveform
	cr->move_to( i - begin, h2-(int)roundf(p) );

	if (p < 0.5)
		cr->line_to( i - begin, h2 + 1 );
	else
		cr->line_to( i - begin, h2 + (int)roundf(p) );
}


// --- SetCursor ---
void AudioWaveformWidget::setCursor(float p_cursor)
{
	float lastCursor = a_currentSample;
	a_currentSample = p_cursor;

	int c1		= (int)((lastCursor - a_offset)		/ AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	int c2		= (int)((p_cursor - a_offset)		/ AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	int begin	= (int)((a_cursorScroll - a_offset)	/ AudioWidget::AUDIO_ZOOM_MAX / (float)a_currentZoom);
	c1 -= begin;
	c2 -= begin;

	queue_draw_area(c1 - 1, 0, c1 + 1, h);
	queue_draw_area(c2 - 1, 0, c2 + 1, h);
}


// --- GetWidth ---
int AudioWaveformWidget::getWidth()
{
	int w = -1;
	int h = -1;

	if ( get_window() )
		get_window()->get_size(w, h);

	return w;
}


// --- SetSelection ---
void AudioWaveformWidget::setSelection(float p_beginSelection, float p_endSelection)
{
	a_beginSelection = p_beginSelection;
	a_endSelection = p_endSelection;
	queue_draw();
}


// --- SetZoom ---
void AudioWaveformWidget::setZoom(int p_factor)
{
	if ( a_loading )
		return ;

	h2 = h/2;
	a_w = (int)(a_duration / AudioWidget::AUDIO_ZOOM_MAX / (float)p_factor);

	if ( a_currentPeaks != NULL && a_currentPeaks != a_maxPeaks )
			FREE(a_currentPeaks);

    bool sizeIntegrity = true;

	a_currentZoom = p_factor;
	a_mut1.lock();

	if (a_currentZoom > 1)
	{
        a_currentPeaks = CALLOC(a_w, float);
		int indice2 = 0;

        for (int j = 0; j < a_w; j++)
		{
			float max = 0;

			for (int k = 0; k < p_factor; k++)
			{
                if (indice2 < a_peaksLength)
                {
                    if (a_maxPeaks[indice2] > max)
                    {
                        max = a_maxPeaks[indice2];
                    }
                }
                else
                {
                    sizeIntegrity = false;
                }
                indice2++;
			}

			if (a_currentPeaks == NULL)
			{
                a_mut1.unlock();
				return;
			}

			a_currentPeaks[j] = max;
		}
	}
	else
	{
		a_currentPeaks = a_maxPeaks;
	}

    /** Integrity check */
    if (!sizeIntegrity)
    {
        printf("Invalid peak size : emitting signal !\n");
        a_signalInvalidPeakSize.emit(true);
    }

	a_mut1.unlock();

	queue_draw();
}

// --- SetScroll ---
void AudioWaveformWidget::setScroll(float p_cursorScroll)
{
	if (p_cursorScroll != a_cursorScroll)
	{
		a_cursorScroll = p_cursorScroll;
		queue_draw();
	}
}

// --- SetSelected ---
void AudioWaveformWidget::setSelected(bool p_selected)
{
	a_selected = p_selected;
	queue_draw();
}

void AudioWaveformWidget::set_color(Glib::ustring mode, Glib::ustring color)
{
	if (mode.compare(TAG_COLORS_AUDIO_WAVE_ACTIVE_BACKGROUND)==0)
		ColorsCfg::color_from_str_to_rgb(color, color_wave_bg_active);
	else if (mode.compare(TAG_COLORS_AUDIO_WAVE_BACKGROUND)==0)
		ColorsCfg::color_from_str_to_rgb(color, color_wave_bg);
	else if (mode.compare(TAG_COLORS_AUDIO_WAVE_FOREGROUND)==0)
		ColorsCfg::color_from_str_to_rgb(color, color_wave_fg);
	else if (mode.compare(TAG_COLORS_AUDIO_WAVE_SELECTION)==0)
		ColorsCfg::color_from_str_to_rgb(color, color_selection);
	else if (mode.compare(TAG_COLORS_AUDIO_WAVE_DISABLED)==0)
		ColorsCfg::color_from_str_to_rgb(color, color_disabled);
	else if (mode.compare(TAG_COLORS_AUDIO_TIP_FOREGROUND)==0)
		ColorsCfg::color_from_str_to_rgb(color, color_tip_fg);
	else if (mode.compare(TAG_COLORS_AUDIO_TIP_BACKGROUND)==0)
		ColorsCfg::color_from_str_to_rgb(color, color_tip_bg);
	else if (mode.compare(TAG_COLORS_AUDIO_WAVE_SEGMENTEND)==0)
		ColorsCfg::color_from_str_to_rgb(color, color_segmentEnd);
	else
		Log::err()<<"audio color mapping error"<<std::endl ;
}

Glib::ustring AudioWaveformWidget::get_color(Glib::ustring mode)
{
	if (mode.compare(TAG_COLORS_AUDIO_WAVE_ACTIVE_BACKGROUND)==0)
		return ColorsCfg::color_from_rgb_to_str(color_wave_bg_active) ;
	else if (mode.compare(TAG_COLORS_AUDIO_WAVE_BACKGROUND)==0)
		return ColorsCfg::color_from_rgb_to_str(color_wave_bg) ;
	else if (mode.compare(TAG_COLORS_AUDIO_WAVE_FOREGROUND)==0)
		return ColorsCfg::color_from_rgb_to_str(color_wave_fg) ;
	else if (mode.compare(TAG_COLORS_AUDIO_WAVE_SELECTION)==0)
		return ColorsCfg::color_from_rgb_to_str(color_selection) ;
	else if (mode.compare(TAG_COLORS_AUDIO_WAVE_DISABLED)==0)
		return ColorsCfg::color_from_rgb_to_str(color_disabled) ;
	else if (mode.compare(TAG_COLORS_AUDIO_TIP_FOREGROUND)==0)
		return ColorsCfg::color_from_rgb_to_str(color_tip_fg) ;
	else if (mode.compare(TAG_COLORS_AUDIO_TIP_BACKGROUND)==0)
		return ColorsCfg::color_from_rgb_to_str(color_tip_bg) ;
	else if (mode.compare(TAG_COLORS_AUDIO_WAVE_SEGMENTEND)==0)
		return ColorsCfg::color_from_rgb_to_str(color_segmentEnd) ;
	else
		return "" ;
}

} // namespace
