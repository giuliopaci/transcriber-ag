/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
* @file 	ExpRegul.h
* Regular expression management
*/

#ifndef _HAVE_EXPREGUL_
#define _HAVE_EXPREGUL_

#include <string>
#include <vector>
#include <regex.h>

#define MAX_MATCH 20
#define MAX_COND 10

using namespace std;
/**
 * @class 		ExpRegul
 * @ingroup		Common
 *
 * Regular expression management class.
 *
 */
class ExpRegul
{
	public:
		/**
		 * Constructor
		 * @return
		 */
		ExpRegul() {};

		/**
		 * Constructor
		 * @param expregul		Regular expression
		 * @param nocase
		 * @return
		 */
		ExpRegul(const char* expregul, bool nocase=false) throw (const char*);

		/**
		 * Set the regular expression
		 * @param expregul		Regular expression
		 * @param nocase
		 * @return
		 */
		bool setExpr(const char* expregul, bool nocase=false);

		/**
		 * 	Processes the research and returns the 1st matching offset in the buffer
		 * @param buffer	Buffer into which the match is researched
		 * @param start
		 * @return
		 */
		bool match(const string& buffer, unsigned long& start);

		/**
		 * @return		The error message
		 */
		const char* errmsg() { return _msg.c_str(); }

		/**
		 *
		 * @param buffer
		 * @param noelem
		 * @return
		 */
		string getSubmatch(const string& buffer, int noelem=1);

	private:
		bool compileExpreg(bool nocase=false);
		string _msg;
		string _expregul;
		regex_t _pattern;	// expr. reguliere compilee
		regmatch_t _match[MAX_MATCH];
		int _noelmax;  // nb max de sous-expressions attendues dans postcond

};
#endif // _HAVE_EXPREGUL_
