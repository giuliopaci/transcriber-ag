/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef LISTMODEL_COLUMNS_H_
#define LISTMODEL_COLUMNS_H_

#include <gtkmm.h>
#include "DataModel/speakers/Speaker.h"
#include "Common/Languages.h"

#define SPEAKER_COL_ID 0
#define SPEAKER_COL_FNAME 1
#define SPEAKER_COL_LNAME 2
#define SPEAKER_COL_GENDER 3
#define SPEAKER_COL_LANGUAGE 4
#define SPEAKER_COL_ACCENT 5
#define SPEAKER_COL_DESC 6

namespace tag {
/**
 * @class 		ListModel_Columns
 * @ingroup		GUI
 *
 * Definition of the Gtk::TreeModel::ColumnRecord used for the speaker list used
 * by SpeakerDico_dialog class\n
 *
 * Represents a speaker entry:\n
 *  - last name\n
 *  - first name\n
 *  - main language\n
 *  - free description\n
 *  - other information used but not displayed
 */
class ListModel_Columns : public Gtk::TreeModel::ColumnRecord
{
	public:
		/**
		 *	Constructor
		 */
		ListModel_Columns() : Gtk::TreeModel::ColumnRecord()
		{
			add(a_id) ;
			add(a_firstName) ;
			add(a_lastName) ;
			add(a_gender) ;
			add(a_lang) ;
			add(a_has_accent) ;
			add(a_desc) ;
		}
		virtual ~ListModel_Columns();

		Gtk::TreeModelColumn<Glib::ustring> a_id;					/**< Speaker id (DataModel id) */
		Gtk::TreeModelColumn<Glib::ustring> a_firstName;			/**< first name display */
		Gtk::TreeModelColumn<Glib::ustring> a_lastName;				/**< last name display */
		Gtk::TreeModelColumn<Speaker::Gender> a_gender ;			/**< gender */
		Gtk::TreeModelColumn<Glib::ustring> a_lang;					/**< first language in its language list */
		Gtk::TreeModelColumn<bool> a_has_accent ;					/**< has accent ? */
		Gtk::TreeModelColumn<Glib::ustring> a_desc ;				/**< free description */

		/**
		 * Static wrapper for filling a row using the current model column
		 * @param row				Pointer on the row we want to fill
		 * @param speaker			Pointer on DataModel::Speaker object
		 * @param languages			Pointer on Speaker::Language object
		 */
		static void fill_row(Gtk::TreeModel::Row* row, Speaker* speaker, Languages* languages) ;
};

} //namespace

#endif /*LISTMODEL_H_*/
