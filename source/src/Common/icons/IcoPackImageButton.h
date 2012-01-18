/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef ICOPACKIMAGEBUTTON_H_
#define ICOPACKIMAGEBUTTON_H_

#include "gtkmm.h"
#include "Common/icons/Icons.h"
#include "Common/icons/IcoPackImage.h"

namespace tag {

/**
 * @class 	IcoPackImageButton
 * @ingroup	GUI
 * Button simulation with Gtk::image
 * Specially implemented for notebook page header in order to get a smaller button
 */
class IcoPackImageButton : public Gtk::EventBox
{
	public:
		IcoPackImageButton();
		virtual ~IcoPackImageButton();

		/**
		 * Set the image that will be used for displaying the button
		 * @param mode		Mode in which the image will be used:\n
		 * 					1: image for button normal state
		 * 					2: image when pointer is over
		 * 					3: image when button is pressed
		 * @param ico		icon name to be used\n
		 * 					See Icons class for all defined icon names
		 * @param size		Size for displaying button
		 */
		void set_image(int mode, const Glib::ustring& ico, int size) ;

		/**
		 * Set the image that will be used for displaying the button
		 * @param mode		Mode in which the image will be used:\n
		 * 					1: image for button normal state
		 * 					2: image when pointer is over
		 * 					3: image when button is pressed
		 * @param path		Path of file image\n
		 * @param size		Size for displaying button
		 */
		void set_image_path(int mode, const Glib::ustring& path, int size) ;

		/**
		 * Set tooltip of buttons
		 * @param tip		tip to be displayed
		 */
		void set_tooltip(const Glib::ustring& tip) ;

		/**
		 * Actualizes the button state
		 * @param mode		Mode in which the button will be:\n
		 * 					1: image for button normal state
		 * 					2: image when pointer is over
		 * 					3: image when button is pressed
		 */
		void set_mode(int mode) ;

	private:
		IcoPackImage image1 ;
		IcoPackImage image2 ;
		IcoPackImage image3 ;
		Gtk::HBox hbox ;
		Gtk::Tooltips tooltip ;

		bool on_enter_notify_event(GdkEventCrossing* event) ;
		bool on_leave_notify_event(GdkEventCrossing* event) ;
		bool on_button_press_event(GdkEventButton* event) ;
		bool on_button_release_event(GdkEventButton* event) ;
};

} //namespace

#endif /*ICOPACKIMAGEBUTTON_H_*/
