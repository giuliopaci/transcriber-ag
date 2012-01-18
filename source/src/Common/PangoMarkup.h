/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
*  @file  PangoMarkup.h
*  @brief helper classes for Pango markup strings
*/

#ifndef _HAVE_PANGOMARKUP_
#define _HAVE_PANGOMARKUP_ 1

#include <sstream>
#include <string>

/**
* @class 		Markup
* @ingroup		Common
* Markup...
*/
class Markup
{
	public:
		/**
		 * Constructor
		 * @param markup	Markup characteristic (specificity tag)
		 * @return
		 */
		Markup(const char* markup) : _markup(markup) {}

		/**
		 * Get the markup formatted string
		 * @param label		String to format
		 * @return			The given string with markup format
		 */
		const char* get(const char* label)
		{
			_os << "<" << _markup << ">" << label << "</" << _markup << ">";
			return _os.str().c_str();
		}

	private:
		std::string _markup;
		std::ostringstream _os;
};


/**
* @class 		MarkupSmall
* @ingroup		Common
* Small markup...
*/
class MarkupSmall : public Markup
{
	public:
		/**
		 * Constructor
		 * @param label		String to format
		 * @return
		 */
		MarkupSmall(const char* label) : Markup("small"), _label(label) {}
		/**
		 * Accessor
		 * @return	The formatted string
		 */
		const char* c_str() { return get(_label); }
	private:
		const char* _label;
};


#endif  /* _HAVE_PANGOMARKUP_ */
