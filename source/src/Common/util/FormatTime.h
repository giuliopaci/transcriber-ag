/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
* @file 	FormatTime.h
* Basic date formatter
*/

#ifndef _HAVE_FORMATTIME_
#define _HAVE_FORMATTIME_ 1

#include <string>
#include <time.h>

/**
 * @class 		FormatTime
 * @ingroup		Common
 *
 * Basic date formatter
 *
 */
class FormatTime : public std::string
{
	public:

		/**
		 * Constructor
		 * @param p_time			Time in seconds
		 * @param with_sec_frac		If True, seconds will be displayed too after a coma separator (hh:mm:ss,xx)
		 * @param with_hours		If True, hours will be displayed even if they are null (00:mm::ss)
		 * @return					The formatted date (HH):MM:SS
		 */
		FormatTime(double p_time, bool with_sec_frac=true, bool with_hours=false) ;

		/**
		 * Converts a HH:MM:SS hour format	into seconds
		 * @param time			Formatted hour
		 * @return				Hours in seconds
		 */
		static double revertFormat(std::string time) ;

		/**
		 * Converts time in seconds into specific string representation Thours:minutes:seconds:frame-numberFframe-rate
		 * @param seconds			Start time in seconds
		 * @param relative_frame	Number of the frame starting to the start time
		 * @param frameRate_f		Frame rate
		 * @param absolute_frame	Absolute frame number
		 * @return
		 */
		static std::string getTXMLtime(double seconds, int relative_frame, double frameRate_f, int absolute_frame) ;

		/**
		 * format system time according to format specification
		 * @param t : time in seconds since Jan, 1, 1970
		 * @param fmt :  format specification (defaults to "%H:%M:%S")
		 * @return formatted date string
		 * @note see strftime man page for format specifications
		 */
		static std::string formatDate(time_t t, const char* fmt = "%H:%M:%S") ;

		static const char* FMT_ISO8601;

};

#endif /* _HAVE_FORMATTIME_ 1 */
