/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */


#ifndef FBMODELCOLUMNS_H_
#define FBMODELCOLUMNS_H_

#include <gtkmm.h>

namespace tag {
/**
 * @class 		FBModelColumns
 * @ingroup		MediaComponent
 * Model columns for the frames browser model.
 */
class FBModelColumns : public Gtk::TreeModel::ColumnRecord
{
	public:
		/**
		 *	Constructor
		 */
		FBModelColumns() : Gtk::TreeModel::ColumnRecord()
		{
			add(image) ;
			add(frameTime) ;
			add(displayTime) ;
		}
		virtual ~FBModelColumns();

		Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > image ; /**< row icon */
		Gtk::TreeModelColumn<float> frameTime ; /**< row time */
		Gtk::TreeModelColumn<Glib::ustring> displayTime ; /**< row time display */
		/**
		 * Fills row with data
		 * @param row		Row's pointer
		 * @param image		Pixbuf
		 * @param time		Display time
		 */
		static void fill_row(Gtk::TreeModel::Row* row, const Glib::RefPtr<Gdk::Pixbuf>& image, float time) ;

};

} // namespace

#endif /* FBMODELCOLUMNS_H_ */
