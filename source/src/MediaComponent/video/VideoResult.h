/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef VIDEORESULT_H
#define VIDEORESULT_H

#include <string>

/**
*  @class 		VideoResult
*  @ingroup		MediaComponent
*
*  Class used keepong some useful data of a video file.\n
*/
class VideoResult
{
	public:
		/**< file path */
		std::string	filePath;
		/**< extraction state */
		int			processState;
		/**< width */
		int			videoWidth;
		/**< height */
		int			videoHeight;
		/**< resolution */
		double		fps;
};

#endif

