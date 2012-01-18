/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file	ResultSet.h
 */

#ifndef RESULTSET_H_
#define RESULTSET_H_

#include <gtkmm.h>

namespace tag {
/**
* @class 		ResultSet
* @ingroup		Common
*
* Class managing query result set received in light mode.\n
*/
class ResultSet
{
	public:
		/**
		 * Constructor.\n
		 */
		ResultSet() ;

		/**
		 * Destructor
		 */
		virtual ~ResultSet() ;

		/**
		 * True if the result set loading suceed, false otherwise
		 * @return		True for success, False otherwise
		 */
		bool isLoaded() { return loaded ;}

		/**
		 * Loads highlight result set from path
		 * @param path		File path
		 * @return			True for success, false for failure
		 */
		bool loadResultSetFile(Glib::ustring path) ;

		/**
		 * Loads highlight result set from string
		 * @param resultset		Highlight matching string
		 * @return			True for success, false for failure
		 */
		bool loadResultSetString(Glib::ustring resultset) ;

		/**
		 * Accessor to all matching terms
		 * @return		A vector of all matching terms
		 * @remarks		Available only when resultset has been loaded from string,
		 * 				otherwise return an empty vector
		 */
		const std::vector<Glib::ustring>& getMatchingTerms() { return resultset_vector ; }

	private:
		/** Data file**/
		Glib::ustring filepath ;

		/** Data string **/
		Glib::ustring resultset_string ;
		std::vector<Glib::ustring> resultset_vector ;

		bool loaded ;

		/** Parse the result set file **/
		bool parseResultSetFile() ;

		/** Parse the result set string **/
		bool parseResultSetString() ;
};

} // namespace

#endif /* RESULTSET_H_ */
