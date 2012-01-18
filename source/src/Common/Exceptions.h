/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
*  @file Exceptions.h
*  @brief Exceptions raised by various transcriber components
*/

#ifndef _HAVE_EXCEPTION_H
#define _HAVE_EXCEPTION_H

#include <string>
#include <exception>

namespace tag {

/**
* @class 		Exception
* @ingroup		Common
*
* Exceptions raised by various transcriber components
*/
class Exception : public std::exception
{
	public:
		/**
		 * Constructor
		 * @param msg	Excpetion message
		 */
		Exception(std::string msg = "Exception raised") throw() : _msg(msg) {}
		virtual ~Exception() throw() {}

		/**
		 * @return	The exception message
		 */
		virtual const char* what() throw() { return _msg.c_str(); }

	private:
		std::string _msg;
};


#endif /* _HAVE_EXCEPTION_H */

