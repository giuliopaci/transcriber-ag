/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef VIDEOWRITER_H
#define VIDEOWRITER_H

#include <string>
#include "base/Guesser.h"

#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT PIX_FMT_RGB24 /* default pix_fmt */


using namespace std;

class VideoResult;

/**
*  @class 		VideoWriter
*  @ingroup		MediaComponent
*
*  Class used for writting a video stream in file.\n
*/
class VideoWriter
{
	public:
		/**
		 * Constructor
		 * @param inPath	File path
		 * @param inWidth	Video width
		 * @param inHeight	Video height
		 * @param inStep	Extraction step
		 * @return
		 */
		VideoWriter(string inPath, int inWidth, int inHeight, int inStep);

		/**
		 * Generates video and returns video data
		 * @return		VideoResult pointer
		 */
		VideoResult*	generateVideo();

		/**
		 *
		 * @param oc
		 * @param codecID
		 * @return
		 */
		AVStream*		addVideoStream(AVFormatContext* oc, AVCodecID codecID);

		/**
		 *
		 * @param oc
		 * @param st
		 */
		void			openVideo(AVFormatContext *oc, AVStream *st);

		/**
		 *
		 * @param oc
		 * @param st
		 */
		void			closeVideo(AVFormatContext *oc, AVStream *st);

		/**
		 *
		 * @param oc
		 * @param st
		 * @param index
		 */
		void			writeVideoFrame(AVFormatContext *oc, AVStream *st, int index);

		/**
		 *
		 * @param pix_fmt
		 * @param width
		 * @param height
		 * @return
		 */
		AVFrame*		alloc_picture(int pix_fmt, int width, int height);

	private:
		string	sourcePath;
		int		outWidth;
		int		outHeight;
		int		frameStep;

		IODevice* device;

		// -- Video --
		AVOutputFormat*		outFormat;
		AVFormatContext*	formatCtx;
		AVStream*			video_st;
		double				video_pts;

		AVFrame *picture, *tmp_picture;
		uint8_t *video_outbuf;
		int frame_count, video_outbuf_size;
};

#endif

