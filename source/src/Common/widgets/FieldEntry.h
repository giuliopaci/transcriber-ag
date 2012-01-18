/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file	FieldEntry.h
 */

#ifndef FIELDENTRY_H_
#define FIELDENTRY_H_

#include <string>
#include <gtkmm.h>

namespace tag {

/**
* @class 		FieldEntry
* @ingroup		Common
*
* Basic widget providing entry's features with separators between formatted elements.\n
*/
class FieldEntry : public Gtk::HBox
{
	public:
		/**
		 * Constructor
		 * @param nbElements		Elements number inside the entry
		 * @param separator			String to use as separator (not editable)
		 * @param elementSize		Max size of each elements
		 */
		FieldEntry(int nbElements, Glib::ustring separator, guint elementSize);
		virtual ~FieldEntry();

		/**
		 * Accessor to the Nth element
		 * @param indice	Element indice
		 * @return			The <em>indice</em>NTH element
		 */
		Glib::ustring get_element(guint indice) ;

		/**
		 * Sets the Nth element
		 * @param indice	Element indice
		 * @param value		Element value
		 * @return			True if successful, False if <em>indice</em> doesn't match elements number
		 */
		bool set_element(guint indice, Glib::ustring value) ;

		/**
		 * Set the entry editability
		 * @param editable		True for sets the entry editable, False otherwise
		 */
		void set_editable(bool editable) ;

		/**
		 * Set the entry editability
		 * @param sensitive		True for sets the entry sensitive, False otherwise
		 */
		void set_sensitive(bool sensitive) ;

		/**
		 * Changes the size request of the entry to be about the right size for <em>nb</em> characters.
		 * @param nb		Characters number for which setting the size
		 * @todo			Not yet implemented
		 */
		void set_width_chars(int nb) ;

	private :
		std::vector<Gtk::Entry*> entries ;
		std::vector<Gtk::Entry*> label_separators ;
		void on_separator_grab_focus(guint i) ;

};

} //namespace

#endif /*FieldEntry_H_*/
