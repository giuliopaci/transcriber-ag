/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		GoToDialog.h
 */

#ifndef GOTTODIALOG_H_
#define GOTTODIALOG_H_


#include <gtkmm.h>

namespace tag {
/**
* @class 		GoToDialog
* @ingroup 		AnnotationEditor
*
*  Dialog used for choosing the navigation timecode we want the signal to be set at.
*/
class GoToDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 */
		GoToDialog();

		/**
		 * Destructor
		 */
		virtual ~GoToDialog();

		/**
		 * Accessor to the timecode selected by user
		 * @return		Float timecode value.
		 */
		float getPosition() { return result ;}

	private:
		Gtk::HBox hbox ;
		Gtk::Label label ;
		Gtk::Entry entry ;
		float result ;

		void on_response(int p_id) ;
};

}

#endif /* GOTTODIALOG_H_ */
