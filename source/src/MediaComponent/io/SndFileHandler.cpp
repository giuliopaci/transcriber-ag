/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "SndFileHandler.h"

#ifdef MEDIACOMPONENT_STANDALONE
	#define _(a) a
#else
	#include "globals.h"
#endif

// --- SndFileHandler ---
SndFileHandler::SndFileHandler()
{
	info_retrieved	= false;
	current_ts		= 0.0;
	step			= 0.0;
}


// --- SndFile Info ---
SF_INFO* SndFileHandler::m_sfinfo()
{
	return &file_info;
}


// --------------------
// --- IODevice API ---
// --------------------

// --- Init ---
bool SndFileHandler::m_init()
{
	return true;
}


// --- Open ---
bool SndFileHandler::m_open(const char *in_medium, int mode)
{
	medium = (char*)in_medium;
	s_file = sf_open(medium, mode, &file_info);

	if (s_file != NULL)
	{
		// -- Static Frame Allocation --
		f = new MediumFrame;
		f->samples = new int16_t[AUDIO_BUFFER_SIZE * file_info.channels];

		m_info();
	}

	return (s_file != NULL);
}


// --- Close ---
bool SndFileHandler::m_close()
{
	return !sf_close(s_file);
}


// --- Write ---
bool SndFileHandler::m_write(MediumFrame *f)
{
	if (f == NULL)
		return false;

	sf_count_t written = sf_write_short(s_file, f->samples, f->len);

	return (written > 0);
}


// --- Read ---
MediumFrame* SndFileHandler::m_read()
{
	current_ts += step;

	f->len	= sf_read_short(s_file, f->samples, AUDIO_BUFFER_SIZE * file_info.channels);
	f->ts	= current_ts;

	if (f->len == 0)
		return NULL;

	return f;
}


// --- Seek ---
bool SndFileHandler::m_seek(double ts)
{
	// Time -> Frame Number
	sf_count_t f_nbr = (ts * file_info.frames) / s_info.audio_duration;

	current_ts = (float)(f_nbr * s_info.audio_duration) / (float)s_info.audio_samples;

	int idx = sf_seek(s_file, f_nbr, SEEK_SET);

	return (idx >= 0);
}


// --- Back (1 frame) ---
bool SndFileHandler::m_back()
{
	return true;
}


// --- GetMediumInfo ---
MediumInfo* SndFileHandler::m_info()
{
	if (!info_retrieved)
	{
		m_formatDescription(file_info.format);

		// -- Filling --
		s_info.audio_channels		= file_info.channels;
		s_info.audio_frame_size		= AUDIO_BUFFER_SIZE;
		s_info.audio_sample_rate	= file_info.samplerate;
		s_info.audio_duration		= (double)file_info.frames / (double)file_info.samplerate;
		s_info.audio_samples		= file_info.frames;

		step = (float)s_info.audio_frame_size / (float)s_info.audio_sample_rate;
		info_retrieved = true;
	}

	return &s_info;
}


// --- SetSFInfo ---
void SndFileHandler::m_set_sfinfo(int channels, int samplerate, int format)
{
	file_info.channels		= channels;
	file_info.samplerate	= samplerate;
	file_info.format		= format;
}


// --- FormatDescription ---
void SndFileHandler::m_formatDescription(int format_id)
{
	// -- Main Format --
	switch (format_id & SF_FORMAT_TYPEMASK)
	{
		case SF_FORMAT_WAV		: s_info.audio_codec = _("WAV (Little Endian)");break;
		case SF_FORMAT_AIFF		: s_info.audio_codec = _("Apple/SGI AIFF (Big Endian)");break;
		case SF_FORMAT_AU		: s_info.audio_codec = _("Sun/NeXT AU (Big Endian)");break;
		case SF_FORMAT_RAW		: s_info.audio_codec = _("RAW PCM Data");break;
		case SF_FORMAT_PAF		: s_info.audio_codec = _("Ensoniq PARIS Format");break;
		case SF_FORMAT_SVX		: s_info.audio_codec = _("Amiga IFF / SVX8 / SV16");break;
		case SF_FORMAT_NIST		: s_info.audio_codec = _("Sphere NIST");break;
		case SF_FORMAT_VOC		: s_info.audio_codec = _("VOC");break;
		case SF_FORMAT_IRCAM	: s_info.audio_codec = _("Berkeley/IRCAM/CARL");break;
		case SF_FORMAT_W64		: s_info.audio_codec = _("Sonic Foundry's 64 bit RIFF/WAV");break;
		case SF_FORMAT_MAT4		: s_info.audio_codec = _("Matlab (TM) v4.2 / GNU Octave v2.0");break;
		case SF_FORMAT_MAT5		: s_info.audio_codec = _("Matlab (TM) v5.0 / GNU Octave v2.1");break;
		case SF_FORMAT_PVF		: s_info.audio_codec = _("Portable Voice Format");break;
		case SF_FORMAT_XI		: s_info.audio_codec = _("Fasttracker 2 Extended Instrument");break;
		case SF_FORMAT_HTK		: s_info.audio_codec = _("HMM Tool Kit Format");break;
		case SF_FORMAT_SDS		: s_info.audio_codec = _("Midi Sample Dump Standard");break;
		case SF_FORMAT_AVR		: s_info.audio_codec = _("Audio Visual Research");break;
		case SF_FORMAT_WAVEX	: s_info.audio_codec = _("MS WAVE With WAVEFORMATEX");break;
		case SF_FORMAT_SD2		: s_info.audio_codec = _("Sound Designer 2");break;
		case SF_FORMAT_FLAC		: s_info.audio_codec = _("FLAC Lossless Format");break;
		case SF_FORMAT_CAF		: s_info.audio_codec = _("Core Audio File");break;
		default					: s_info.audio_codec = _("Unrecognized"); break;
	};

	// -- Sample Resolution & Encodings --
	switch (format_id & SF_FORMAT_SUBMASK)
	{
		// -- Resolutions --
		case SF_FORMAT_PCM_S8		: s_info.audio_sample_resolution = 8;	break;
		case SF_FORMAT_PCM_16		: s_info.audio_sample_resolution = 16;	break;
		case SF_FORMAT_PCM_24		: s_info.audio_sample_resolution = 24;	break;
		case SF_FORMAT_PCM_32		: s_info.audio_sample_resolution = 32;	break;
		case SF_FORMAT_PCM_U8		: s_info.audio_sample_resolution = 8;	break;
		case SF_FORMAT_FLOAT		: s_info.audio_sample_resolution = 32;	break;
		case SF_FORMAT_DOUBLE		: s_info.audio_sample_resolution = 64;	break;
		case SF_FORMAT_DWVW_12		: s_info.audio_sample_resolution = 12;	break;
		case SF_FORMAT_DWVW_16		: s_info.audio_sample_resolution = 16;	break;
		case SF_FORMAT_DWVW_24		: s_info.audio_sample_resolution = 24;	break;
		case SF_FORMAT_DPCM_8		: s_info.audio_sample_resolution = 8;	break;
		case SF_FORMAT_DPCM_16		: s_info.audio_sample_resolution = 16;	break;
		default						: s_info.audio_sample_resolution = -1;
	};

	switch (format_id & SF_FORMAT_SUBMASK)
	{
		// -- Encodings --
		case SF_FORMAT_ULAW			: s_info.audio_encoding = _("U-Law"); break;
		case SF_FORMAT_ALAW			: s_info.audio_encoding = _("A-Law"); break;
		case SF_FORMAT_IMA_ADPCM	: s_info.audio_encoding = _("IMA ADPCM"); break;
		case SF_FORMAT_MS_ADPCM		: s_info.audio_encoding = _("Microsoft ADPCM"); break;
		case SF_FORMAT_GSM610       : s_info.audio_encoding = _("GSM 6.10 Encoding"); break;
		case SF_FORMAT_VOX_ADPCM	: s_info.audio_encoding = _("Oki Dialogic ADPCM Encoding"); break;
		case SF_FORMAT_G721_32		: s_info.audio_encoding = _("32 Kbs G721 ADPCM Encoding"); break;
		case SF_FORMAT_G723_24		: s_info.audio_encoding = _("24 Kbs G723 ADPCM Encoding"); break;
		case SF_FORMAT_G723_40		: s_info.audio_encoding = _("40 Kbs G723 ADPCM Encoding"); break;
		default						: break;
	}
}

