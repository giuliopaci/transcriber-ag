/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef TYPES_H
#define TYPES_H

#define DFT_AUDIO_DECODER	CODEC_ID_PCM_S16LE
#define	AUDIO_BUFFER_SIZE	512
#define AUDIO_SAMPLE_RATE	44100

#include <sys/types.h>
#include <stdint.h>
#include <string>

#include "STTypes.h"

using namespace std;

/**
 * @defgroup MediaComponent MediaComponent
 */

/**
 * @struct	MediumFrame
 * @ingroup	MediaComponent
 * Global structure containing a single medium frame
 */
typedef struct
{
	soundtouch::SAMPLETYPE*	samples;		/**< Audio samples */
	uint8_t**	v_samples;		/**< Video samples */
	float		ts;				/**< Frame timestamp */
	int*		v_linesize;		/**< Video linesizes */
	int			len;			/**< Audio samples count */
	int			vlen;			/**< Video samples count */
	int			v_width;		/**< Video width */
	int			v_height;		/**< Video height */
}
MediumFrame;


/**
 * @struct	MediumInfo
 * @ingroup	MediaComponent
 * Global structure containing all required information for audio/video streams
 */
typedef struct
{
	// -- Audio --
	int		audio_channels;				/**< Audio channels count */
	int		audio_sample_rate;			/**< Audio sample rate */
	int		audio_frame_size;			/**< Audio frame size */
	int		audio_sample_resolution;	/**< Audio sample resolution */
	long	audio_samples;				/**< Audio samples count */
	double	audio_duration;				/**< Audio duration (in seconds) */

	// -- Video --
	int		video_channels;				/**< Video streams count */
	int		video_bit_rate;				/**< Video bit rate */
	int		video_frame_size;			/**< Video frame size */
	int		video_height;				/**< Native video height */
	int		video_width;				/**< Native video width */
	int		video_sample_resolution;	/**< Video sample resolution */
	long	video_samples;				/**< Video samples count */
	double	video_duration;				/**< Video duration (in seconds) */
	double	video_aspect_ratio;			/**< Video aspect ratio (width / height) */
	double	video_fps;					/**< Video frame rate (frames per second) */
	bool	video_key_frame;			/**< True if video contains keyframes */

	// -- General --
	string	audio_codec;				/**< Native audio codec description */
	string	video_codec;				/**< Native video codec description */
	string	audio_encoding;				/**< Audio encoding description */
	string	video_encoding;				/**< Video encoding description */
	string	path;						/**< Medium path */
	string	title;						/**< Medium title (optional) */
	string	author;						/**< Medium author (optional) */
}
MediumInfo;


// -- Modes --
typedef enum handlerctx
{
	NoCtx = 0,
	EncoderCtx,
	DecoderCtx,
	DualCtx
}
HandlerCtx;

typedef enum mediumctx
{
	AudioCtx = 0,
	VideoCtx,
	AudioVideoCtx
}
MediumCtx;

#endif

