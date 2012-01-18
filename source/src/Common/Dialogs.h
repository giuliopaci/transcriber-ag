/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
*  @file 	Dialogs.h
*  @brief 	Basic application dialogs
*/

#ifndef _HAVE_DIALOGS_H
#define _HAVE_DIALOGS_H 1

#include <gtkmm.h>

/**
 * @namespace	dlg
 * Basic user friendly dialogs
 */
namespace dlg {

/**
* @class 	Confirmsg3
* @ingroup 	Common
* Show message dialog with 3 options "yes" "no" "cancel"
*/
class Confirmsg3 : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param title		Dialog title
		 * @param parent	Reference on parent window
		 * @param msg		Message to display
		 * @param modal		True for modal mode, False otherwise
		 */
		Confirmsg3(const Glib::ustring& title, Gtk::Window& parent, const Glib::ustring& msg="", bool modal=false);

		/**
		 * Constructor
		 * @param title		Dialog title
		 * @param msg		Message to display
		 * @param parent	Pointer on parent window
		 * @param modal		True for modal mode, False otherwise
		 */
		Confirmsg3(const Glib::ustring& title, const Glib::ustring& msg="", Gtk::Window* parent=NULL, bool modal=false);

		/**
		 * Constructor
		 * @param msg		Message to display
		 * @param parent	Pointer on parent window
		 */
		Confirmsg3(const Glib::ustring& msg="", Gtk::Window* parent=NULL);

		/**
		 * Set dialog message
		 * @param msg		new message to set
		 */
		void setMessage(const Glib::ustring& msg);

		/**
		 * Set the given label to the button corresponding to the given Gtk::RESPONSE id
		 * @param id		Gtk::RESPONSE id
		 * @param str		Label to set
		 */
		void setActionLabel(int id, const Glib::ustring& str);

	private:
		void buildDialog();
		Gtk::HBox m_hbox;
		Gtk::Label m_label;
		Gtk::Image m_image;
		Gtk::Button *m_yesButton;
		Gtk::Button *m_noButton;
		Gtk::Button *m_cancelButton;
};


/**
 * Standard info message
 * @param a			Message
 * @param parent	Pointer on parent window
 */
void msg(const Glib::ustring& a, Gtk::Window* parent=NULL);

/**
 * Advanced information message. Displays a message as in a classic dialog, and
 * add a text zone for displaying a more detailed message.
 * @param a			Message
 * @param detailed	Detailed message
 * @param parent	Pointer on parent window
 * @param expanded	True for displaying the detailed msg, false for closing the expander
 */
void msg(const Glib::ustring& a, const Glib::ustring& detailed, Gtk::Window* parent, bool expanded=false) ;

/**
 * Standard warning message
 * @param a			message		Message
 * @param parent	Pointer on parent window
 */
void warning(const Glib::ustring& a, Gtk::Window* parent=NULL);

/**
 * Advanced warning message. Displays a message as in a classic dialog, and
 * add a text zone for displaying a more detailed message.
 * @param msg		Message
 * @param detailed	Detailed message
 * @param parent	Pointer on parent window
 * @param expanded	True for displaying the detailed msg, false for closing the expander
 */
void warning(const Glib::ustring& msg, const Glib::ustring& detailed, Gtk::Window* parent, bool expanded=false) ;

/**
 * Standard warning message
 * @param a			message
 * @param parent	Parent
 */
void error(const Glib::ustring& a, Gtk::Window* parent=NULL);

/**
 * Standard confirm message
 * @param a			Message
 * @param parent	Pointer on parent window
 * @return			Gtk::RESPONSE code corresponding to returned value (Gtk::RESPONSE_YES, Gtk::RESPONSE_NO)
 */
bool confirm(const Glib::ustring& a, Gtk::Window* parent=NULL);

/**
 * Two choices button message with cancel option
 * @param a			Message
 * @param choice1	Label for choice1, or Yes if empty
 * @param choice2	Label for choice2, or No if empty
 * @param parent	Pointer on parent window
 * @return			RESPONSE_YES for first button, RESPONSE_NO for second button, RESPONSE_CANCEL for cancel button
 */
int confirmOrCancel(const Glib::ustring& a, const Glib::ustring& choice1, const Glib::ustring& choice2, Gtk::Window* parent) ;

/**
 * Standard confirm message
 * @param button_yes_txt	Yes button text
 * @param button_no_txt		no button text
 * @param a					Message
 * @param parent			Pointer on parent window
 * @return					Gtk::RESPONSE code corresponding to returned value (Gtk::RESPONSE_YES, Gtk::RESPONSE_NO)
 */
bool confirmWithButton(const Glib::ustring& button_yes_txt, const Glib::ustring& button_no_txt, const Glib::ustring& a, Gtk::Window* parent) ;


/**
 * Advanced error message. Displays a message as in a classic dialog, and
 * add a text zone for displaying a more detailed message.
 * @param msg			Message
 * @param detailed		Details message
 * @param parent		Pointer on parent Window
 * @param expanded		True for display detailed message, false for hidden it
 */
void error(const Glib::ustring& msg, const Glib::ustring& detailed, Gtk::Window* parent, bool expanded=false) ;


}

#endif /* _HAVE_DIALOGS_H */
