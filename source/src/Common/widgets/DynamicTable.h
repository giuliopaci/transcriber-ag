/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
 * @file	DynamicTable.h
 */

#ifndef DYNAMICTABLE_H_
#define DYNAMICTABLE_H_

#include <gtkmm.h>

namespace tag {

/**
* @class 		DynamicTable
* @ingroup		Common
*
* Basic widget providing a dynamic table behaviour.\n
* (wrapper around Gtk::Table class)
*/
class DynamicTable : public Gtk::Table
{
	public:
		/**
		 * Constructor
		 * @param nbRows		Default rows number
		 * @param nbColumns		Default columns number
		 * @param homogenous	True for homogeneous presentation
		 */
		DynamicTable(guint nbRows, guint nbColumns, bool homogenous) ;
		virtual ~DynamicTable();

		/**
		 * @return		The current rows number
		 */
		guint get_current_nb_rows() {return current_nb_rows ;}

		/**
		 * Adds a new row into the table
		 * @param widgets		Vector of all widgets to add
		 * @return				True if successful addition, False otherwise.\n
		 * 						(compares widgets number and columns number)
		 */
		bool add_row(const std::vector<Gtk::Widget*>& widgets) ;

		/**
		 * Sets Gtk::Table attach option
		 * @param xopt		---
		 * @param yopt		---
		 */
		void set_attach_options(Gtk::AttachOptions xopt, Gtk::AttachOptions yopt) ;

		/**
		 * Sets Gtk::Table padding option
		 * @param xpadding		---
		 * @param ypadding		---
		 */
		void set_padding_options(guint xpadding, guint ypadding) ;


	private:
		guint current_nb_rows ;
		guint columns ;
		guint rows ;
		guint xpadding ;
		guint ypadding ;
		Gtk::AttachOptions xattach ;
		Gtk::AttachOptions yattach ;
};

} //namespace tag

#endif /* DYNAMICTABLE_H_ */
