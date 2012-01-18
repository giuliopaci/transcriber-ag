/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 	DialogSelectConventions.h
 */

#ifndef __HAVE_DIALOGSELECTCONVENTIONS__
#define __HAVE_DIALOGSELECTCONVENTIONS__

#include <gtkmm.h>
#include <sstream>
#include <map>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

namespace tag {
/**
 * @class DialogSelectConventions
 *
 * Dialog used for displaying and selecting transcription language and
 * transcription conventions.
 */
class DialogSelectConventions : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param p_parent				Reference on parent window
		 * @param p_title				Dialog title
		 * @param p_lang				Language label to be displayed by default
		 * @param p_conventions			Convention name to be displayed by default
		 * @param p_lang_list			Available language label list
		 * @param p_conv_list			Available conventions name list
		 * @param forced_convention		If set, the convention displayed is this one and
		 * 								can't be changed by dialog
		 */
		DialogSelectConventions(Gtk::Window& p_parent, string& p_title,
									string& p_lang, string& p_conventions,
									vector<string>& p_lang_list, vector<string>& p_conv_list,
									string& forced_convention);
		virtual ~DialogSelectConventions();

	private:
		Gtk::ComboBoxText* a_langEntry;
		Gtk::ComboBoxText* a_convEntry;
		string& a_lang;
		string& a_conventions;
		string& a_forced_convention ;

		void onButtonClicked(int p_id);

};

}

#endif // __HAVE_DIALOGFILEPROPERTIES__
