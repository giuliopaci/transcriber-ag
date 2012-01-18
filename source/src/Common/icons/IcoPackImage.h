/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file 	IcoPackImage.h
 */

#ifndef ICOPACKIMAGE_H_
#define ICOPACKIMAGE_H_

#include <gtkmm.h>

namespace tag {
/**
 * @class 		IcoPackImage
 * @ingroup		Common
 *
 * Basic wrapper around Gtk::Image for providing an easy set icon method.\n
 * Uses the TranscriberAG Gtk::Theme.
 */
class IcoPackImage : public Gtk::Image
{
	public:
		/**
		 *  Constructor
		 */
		IcoPackImage();
		virtual ~IcoPackImage();

		/**
		 * Sets the image using the TranscriberAG Gtk::Theme
		 * @param ico		Defined name of the icon that is displayed
		 * @param size		Icon size
		 */
		int set_image(const Glib::ustring& ico, int size) ;

		/**
		 * Sets the image using the TransAGVideo Gtk::Theme
		 * @param stock		Stock element
		 * @param size		Icon size
		 */
		int set_image(const Gtk::StockID& stock, int size) ;

		/**
		 * Sets the image using a file
		 * @param path		File path
		 * @param size		Image size
		 * @return			True if successful, False if an error occurred (Gtk::Theme error)
		 */
		int set_image_path(const Glib::ustring& path, int size) ;

		/**
		 * Set image
		 * @param icon		Defined image name
		 * @param label		Label to display
		 * @param size		Size
		 * @param tooltip	Tooltip text
		 * @return			1 for success, -1 for failure
		 */
		int set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;

		/**
		 * Set image
		 * @param stock		Stock icon to use
		 * @param label		Label to display
		 * @param size		Size
		 * @param tooltip	Tooltip text
		 * @return			1 for success, -1 for failure
		 */
		int set_icon(const Gtk::StockID& stock, const Glib::ustring& label, int size, const Glib::ustring& tooltip) ;

		/**
		 * Sets image tooltip
		 * @param tip		Tooltip text
		 */
		void set_tip(const Glib::ustring& tip) ;

	private:
		Gtk::Tooltips tooltip ;
};

} //namespace

#endif /*ICOPACKIMAGE_H_*/
