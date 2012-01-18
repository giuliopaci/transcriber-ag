/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* @file formatTime : time formatter
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sstream>

#include "FormatTime.h"

/* 		2009-12-07T11:14:18Z   */
const char* FormatTime::FMT_ISO8601 = "%FT%H:%M:%SZ";


FormatTime::FormatTime(double p_time, bool with_sec_frac, bool with_hours)
{
	char str[80];
	int S1 = ((int)p_time)%60;
	int M = ((int)(p_time/60.0))%60;
	int H = (int)(p_time/3600.0);
	if ( with_sec_frac )
	{
		int S2 = ((int)roundf(p_time*1000.0))%1000;
		if ( ! with_hours && H == 0)
			sprintf(str, "%02d:%02d,%.3d", M, S1, S2);
		else
			sprintf(str, "%02d:%02d:%02d,%03d", H, M, S1, S2);
	}
	else
	{
		if (! with_hours && H == 0)
			sprintf(str, "%02d:%02d", M, S1);
		else
			sprintf(str, "%02d:%02d:%02d", H, M, S1);
	}
	*((std::string*)this)= str;
}

/* HH:MM:SS -> double */
double FormatTime::revertFormat(std::string time)
{
	if (!time.empty() && time.size()==8) {
		std::string shour = time.substr(0, 2) ;
		std::string smin = time.substr(3, 2) ;
		std::string ssec = time.substr(6, 2) ;
		int hour = atoi(shour.c_str()) ;
		int min = atoi(smin.c_str()) ;
		int sec = atoi(ssec.c_str()) ;
		double res = hour*3600 + min*60 + sec ;
		return res ;
	}
	else
		return -1 ;
}

/* double -> HH:MM:SS */
std::string FormatTime::getTXMLtime(double seconds, int relative_frame, double frameRate_f, int absolute_frame)
{
    int frameRate = floorf(frameRate_f) ;

    std::ostringstream res;
    res << "T" << FormatTime(seconds, false, true) << ":" << relative_frame << "F" << frameRate ;

    if (absolute_frame != -1)
    	res << " - frame #" << absolute_frame  ;

    return res.str() ;
}


/**
 * format system time according to format specification
 * @param IN t : time in seconds since Jan, 1, 1970
 * @param IN fmt :  format specification (defaults to "%F %H:%M")
 * @return formatted date string
 * @note see strftime man page for format specifications
 */
std::string FormatTime::formatDate(time_t t, const char * fmt)
{
  static char buftime[40];
  static struct tm* lc;
  lc = localtime(&t);
  strftime(buftime, sizeof(buftime), fmt, lc);
  return buftime;
}


