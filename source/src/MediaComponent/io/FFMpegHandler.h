/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef FFMPEGHANDLER_H
#define FFMPEGHANDLER_H

#define __STDC_CONSTANT_MACROS

#include "IODevice.h"
#include <vector>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}

#include <glib.h>
#include <glib/gstdio.h>

/**
 * @class	FFMpegHandler
 * @ingroup	MediaComponent
 *
 * IODevice based on FFmpeg libraries (libavcodec / libavformat)
 */
class FFMpegHandler : public IODevice
{
public:
	/**
	 * Constructor
	 */
	FFMpegHandler();

	/**
	 * Destructor
	 */
	~FFMpegHandler();


	// -- IODevice API --
	bool			m_init();
	bool			m_open(const char *url, int mode = 0);
	bool			m_close();
	bool			m_write(MediumFrame*);
	MediumFrame*	m_read();
	MediumFrame*	m_previous_frame();
	MediumFrame*	m_current_frame();
	MediumFrame*	m_previous_frame(int);
	MediumFrame*	m_next_frame();
	MediumFrame*	m_next_frame(int);
	MediumFrame*	m_get_rgb_frame(double);
	AVFrame*		m_get_raw_frame(double);
	bool			m_seek(double);
	bool			m_back();
	bool			m_set_dimensions(int, int);
	bool			m_sync(double pts)	{ audio_pts = pts; return true;}
	MediumInfo*		m_info();
	double			m_get_absolute_time(int time_s, int frame_shift);
	void			m_get_relative_time(double time_f, int& time_s, int& frame_shift);
	int				m_get_frame_shift(double time_f);
	int				m_get_frame_number(double time_f);
	double			m_get_nearest_ts(double time_f);
	double 			m_get_frame_ts(int f_nbr);

	/**
	 * Initializes RGB / YUV scalers (video context only)
	 * @return True on success, false otherwise.
	 */
	bool			initScalers();

	/**
	 * Converts a frame to RGB format
	 * @return A converted RGB frame (pointer)
	 */
	MediumFrame*	convertToRGB();


	// -- FFMpeg Inits --
	/**
	 * Defines HandlerContext
	 * @param in_ctx Handler context
	 */
	void			setHandlerContext(HandlerCtx in_ctx);

	/**
	 * Initializes an encoder for specified codec ID
	 * @param codec_id Codec ID
	 * @return True on success, false otherwise.
	 */
	bool			initEncoder(CodecID codec_id);

	/**
	 * Initializes a decoder for specified codec ID
	 * @param codec_id 		Codec ID
	 * @param allocate		If true, allocates a new context
	 * @return True on success, false otherwise.
	 */
	bool			initDecoder(CodecID codec_id, bool allocate=false);

	/**
	 * Sets decoder extra parameters (channels, samplerate)
	 * @param channels		Codec ID
	 * @param sample_rate	Samplerate
	 */
	void			setDecoderParams(int channels, int sample_rate);

	/**
	 * Initializes default frame structure
	 */
	void			initFrame();

	// -------------------
	// -- Base Commands --
	// -------------------

	/**
	 * Encode input buffer
	 * @param buffer		buffer
	 * @param len			buffer length
	 */
	MediumFrame*	encode(uint8_t* buffer, int len);

	/**
	 * Decode input buffer
	 * @param buffer		buffer
	 * @param len			buffer length
	 */
	MediumFrame*	decode(uint8_t* buffer, int len);

	vector<MediumFrame*> decodeMulti(uint8_t* buffer, int len);

	// -----------
	// -- Tools --
	// -----------
	/**
	 * Formats codec information in a fancy way
	 */
	void			m_formatDescription();


	// ---------------
	// -- Accessors --
	// ---------------
	/**
	 * Accessor for decoderCtx
	 * @return Decoder context
	 */
	AVCodecContext*	getDecoderCtx()	{ return decoderCtx; }

	/**
	 * Accessor for encoderCtx
	 * @return Encoder context
	 */
	AVCodecContext*	getEncoderCtx()	{ return encoderCtx; }

	/**
	 * Accessor for decoder
	 * @return Decoder instance
	 */
	AVCodec*		getDecoder()	{ return decoder; }

	/**
	 * Accessor for encoder
	 * @return Encoder instance
	 */
	AVCodec*		getEncoder()	{ return encoder; }


private:
	// -- FFmpeg --
	AVFormatContext	*formatCtx;
	AVCodecContext	*encoderCtx,	*decoderCtx;
	AVCodec			*encoder,		*decoder;
	AVFrame			*o_frame,		*i_frame, *s_frame;
	uint8_t			encBuf[FF_MIN_BUFFER_SIZE];
	uint8_t			*o_data;
	int				o_size;
	int				base_frame_size;
	int				v_width, v_height;
	double			time_base;
	double			frame_rate, frame_duration, current_ts;
	double			audio_time_base, video_time_base;

	// -- Seek --
	double			current_pts, current_dts;
	int64_t			pkt_duration, frame_step;
	double			key_frame_counter, key_frame_interval;
	double			audio_pts, video_pts;

	// -- Vars --
	bool			info_retrieved, stream_seek;
	double			step;
	std::string		s_format;
	MediumFrame*	frame;
	MediumInfo		s_info;
	GMutex*			mutex;

	// -- A/V Streams --
	vector<int>		a_streams;
	vector<int>		v_streams;

	// -- Scaling Context --
	SwsContext		*scaler_rgb;

};

#endif

