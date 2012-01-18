/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef IODEVICE_H
#define IODEVICE_H

#include "MediaComponent/base/Types.h"

struct AVPicture;
struct AVFrame;

/**
 * @class	IODevice
 * @ingroup	MediaComponent
 *
 * Abstract Base Class for all devices (local file, remote uri, ...)
 */
class IODevice
{
public:

	/**
	 * Virtual destructor (must be implemented in subclasses)
	 */
	virtual ~IODevice() {};

	// ------------------------
	// -- Public Virtual API --
	// ------------------------

	// -- Pure I/O Methods --

	/**
	 * Initializes internals
	 * @return True on succes, false otherwise.
	 */
	virtual bool	m_init()	= 0;

	/**
	 * Opens a new medium
	 * @param uri	Medium URI
	 * @param mode	Optional mode (used by SndFileHandler)
	 * @return True on success, false otherwise.
	 */
	virtual bool	m_open(const char* uri, int mode)	= 0;

	/**
	 * Closes current medium
	 * @return True on success, false otherwise.
	 */
	virtual bool	m_close()	= 0;

	/**
	 * Writes an instantiated frame to file
	 * @param frame Single frame pointer
	 * @return True on success, false otherwise.
	 */
	virtual bool	m_write(MediumFrame* frame)	= 0;

	/**
	 * Reads from current medium
	 * @return A MediumFrame pointer on success, NULL otherwise.
	 */
	virtual MediumFrame*	m_read()	= 0;

	/**
	 * Returns a structure containing medium info
	 * @return An information structure.
	 */
	virtual MediumInfo*		m_info()	= 0;


	// ---------------------
	// -- Stream Controls --
	// ---------------------
	/**
	 * Starts playback (only active with remote RTSP streams)
	 * @return True on success, false otherwise.
	 */
	virtual bool			m_play() 							{return true;}

	/**
	 * Pauses playback (only active with remote RTSP streams)
	 * @return True on success, false otherwise.
	 */
	virtual bool			m_pause()							{return true;}

	/**
	 * Stops playback (only active with remote RTSP streams)
	 * @return True on success, false otherwise.
	 */
	virtual bool			m_stop()							{return true;}

	/**
	 * Seeks into stream
	 * @param timestamp	Target offset (in seconds)
	 * @return True on success, false otherwise.
	 */
	virtual bool			m_seek(double timestamp)			{return true;}

	/**
	 * Provides previous frame
	 * @return A MediumFrame pointer on success, NULL otherwise.
	 */
	virtual MediumFrame*	m_previous_frame()					{return NULL;}

	/**
	 * Provides previous frame (in skip mode)
	 * @param	step	Skip value (in frames)
	 * @return A MediumFrame pointer on success, NULL otherwise.
	 */
	virtual MediumFrame*	m_previous_frame(int step)			{return NULL;}

	/**
	 * Provides next frame
	 * @return A MediumFrame pointer on success, NULL otherwise.
	 */
	virtual MediumFrame*	m_next_frame()						{return NULL;}

	/**
	 * Provides next frame (in skip mode)
	 * @param	step	Skip value (in frames)
	 * @return A MediumFrame pointer on success, NULL otherwise.
	 */
	virtual MediumFrame*	m_next_frame(int step)				{return NULL;}

	/**
	 * Provides current frame
	 * @return A MediumFrame pointer on success, NULL otherwise.
	 */
	virtual MediumFrame*	m_current_frame()					{return NULL;}

	/**
	 * Provides a single frame, converted in RGB format
	 * @param timestamp	Frame position (in seconds)
	 * @return A MediumFrame pointer on success, NULL otherwise.
	 */
	virtual MediumFrame*	m_get_rgb_frame(double timestamp)		{return NULL;}

	/**
	 * Provides a single raw frame, straight from decoding pass
	 * @param timestamp	Frame position (in seconds)
	 * @return A libavformat standard frame structure on success, NULL otherwise.
	 */
	virtual AVFrame*		m_get_raw_frame(double timestamp)		{return NULL;}


	/**
	 * Rewinds stream
	 * @return True on success, false otherwise.
	 */
	virtual bool			m_back()							{return true;}

	/**
	 * Synchronizes Audio/Video stream
	 * @param timestamp	Reference timestamp
	 * @return True on success, false otherwise.
	 */
	virtual	bool			m_sync(double timestamp)			{return true;}

	/**
	 * Sets frame output dimensions (only active in video context)
	 * @param w	Frame width
	 * @param h	Frame height
	 * @return True on success, false otherwise.
	 */
	virtual bool			m_set_dimensions(int w, int h)		{return true;}


	// ---------------------
	// -- Utility Methods --
	// ---------------------
	/**
	 * Returns current stream position
	 * @return A timestamp (in seconds)
	 */
	virtual	double			m_get_current_time()					{return 0.0;}

	/**
	 * Converts from absolute time to relative time (reference timestamp + frame shift)
	 * @param		timestamp	Absolute time
	 * @param[out]	frame_ref	Relative reference frame
	 * @param[out]	frame_shift	Frame shift (0..FPS)
	 */
	virtual void			m_get_relative_time(double timestamp, int& frame_ref, int& frame_shift)	{return;}

	/**
	 * Converts from relative time to absolute time
	 * @param frame_ref		Relative reference frame (in seconds)
	 * @param frame_shift	Frame shift (0..FPS)
	 * @return Absolute timestamp (in seconds)
	 */
	virtual double			m_get_absolute_time(int frame_ref, int frame_shift)	{return 0.0;}

	/**
	 * Extracts frame shift from absolute time
	 * @param timestamp	Absolute time (in seconds)
	 * @return Frame shift (0..FPS)
	 */
	virtual int				m_get_frame_shift(double timestamp)		{return 0; }

	/**
	 * Extracts reference frame time from absolute time
	 * @param timestamp	Absolute time (in seconds)
	 * @return Reference Frame (in seconds)
	 */
	virtual int				m_get_frame_number(double timestamp)	{return 0; }

	/**
	 * Computes nearest and exact timestamp from approximate time
	 * @param timestamp Approximate time (in seconds)
	 * @return Correct timestamp (in seconds)
	 */
	virtual double			m_get_nearest_ts(double timestamp)		{return 0.0; }

	// -------------------
	// -- State Control --
	// -------------------

	/**
	 * Returns playback status
	 * @return True if playback is stopped, false otherwise.
	 */
	virtual bool			m_stopped()							{return true;}

	/**
	 * Returns initialization status
	 * @return True if device is initialized, false otherwise.
	 */
	virtual bool			m_initialized()						{return true;}


	// -------------------------
	// -- Non-Virtual methods --
	// -------------------------
	/**
	 * Return current medium uri
	 * @return Medium URI
	 */
	char*					m_medium()							{return medium;}

	/**
	 * Medium URI setter
	 * @param path	Medium path
	 */
	void					set_medium(char* path)				{medium = path;}

	/**
	 * Medium context accessor
	 * @return Medium Context
	 */
	MediumCtx				m_av_ctx()							{return av_ctx;}

	/**
	 * DeviceHandler context accessor
	 * @return HandlerContext
	 */
	HandlerCtx				m_handler_ctx()						{return handler_ctx;}

	/**
	 * MediumContext context setter
	 * @param in_ctx New HandlerContext
	 */
	void					set_av_ctx(MediumCtx in_ctx) 		{av_ctx			= in_ctx;}

	/**
	 * HandlerContext setter
	 * @param in_ctx New MediumContext
	 */
	void					set_handler_ctx(HandlerCtx in_ctx)	{handler_ctx	= in_ctx; }

protected:
	/**
	 * @var handler_ctx
	 * Current HandlerContext
	 */
	HandlerCtx		handler_ctx;

	/**
	 * @var av_ctx
	 * Current MediumContext
	 */
	MediumCtx		av_ctx;

	/**
	 * @var medium
	 * Medium uri
	 */
	char*			medium;
};

#endif

