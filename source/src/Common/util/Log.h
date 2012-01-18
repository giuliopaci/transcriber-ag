/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* LECUYER Paule 	*/
/* 	         																	*/
/********************************************************************************/

/**
* @file 	Log.h
* Basic TranscriberAG logger
*/

#ifndef _HAVE_LOG_H
#define _HAVE_LOG_H

#include <iostream>

using namespace std;

/**
 * @class 		Log
 * @ingroup		Common
 *
 * Basic TranscriberAG logger
 *
 */
class Log
{
	public:
		/**
		 * @typedef 	Level
		 * Log level
		 */
		typedef enum { FINE, MEDIUM, MAJOR, OFF } Level;

		static ostream& timeStamp(ostream&);

		/*! @returns "standard" log stream (defaults to stdout) */
		static ostream& out() { return timeStamp(*_out); }
		/*! @returns "error" log stream (defaults to stderr) */
		static ostream& err() { return timeStamp(*_err); }

		/*! @returns "trace" log stream (defaults to stdout) */
		static ostream& trace(Level level=MEDIUM) { return ( level>=_traceLevel ? *_out : getNull() ) ; }

		/**
		* Redirects logging to given streams
		* @param out 	Destination stream for standard messages
		* @param err 	Destination stream for error messages
		* @note 		<em>out</em> and <em>err</em> streams must have been opened in write mode
		*/
		static void redirect(ostream& out, ostream& err)
		{ _out = &out; _err=&err; }

		/**
		* Redirects logging to log files with given prefix path/name.
		* @param log_prefix 	Log file path prefix
		* @param distinct_err 	if true, will redirect standard messages to "<prefix>.log"
		* 		 and  error messages to "<prefix>.trace" files, else redirect both to "<prefix>.log" file
		* @note		 			If prefix="/dev/null" -> redirect logging to null device \n
		*
		*/
		static void redirect(string log_prefix, bool distinct_err = false)  throw (const char*);

		/**
		* close logging redirection and reset it to stdout and stderr.
		*/
		static void close();

		/**
		* set trace level
		*/
		static void setTraceLevel(Level level) { _prevLevel = _traceLevel; _traceLevel = level; }

		/**
		 * reset trace level
		 */
		static void resetTraceLevel() { _traceLevel = _prevLevel; }

		/**
		 * Sets whether the logger prints timestamp
		 * @param b		True for yes, false for no
		 */
		static void printTimestamp(bool b) { _printTimestamp = b; }

	private:
		static ostream& getNull();

		static ostream* _out;
		static ostream* _err;
		static ostream* _null;
		static Level _traceLevel;
		static Level _prevLevel;
		static bool _firstTry;
		static bool _printTimestamp;
};


/**
 * @def	MSGOUT
 * helper define
 */
#define MSGOUT  Log::err() << __FILE__ << ":" << __LINE__ << " : "
//#define MSGOUT  Log::err()

/**
 * @def	TRACE
 * helper define for trace
 */
#define TRACE Log::trace()

/**
 * @def	TRACE_D
 * helper define for debug trace
 */
#define TRACE_D Log::trace(Log::FINE)


#endif /* _HAVE_LOG_H */
