/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef LISTMODELCOLUMNS_LANGUAGE_H_
#define LISTMODELCOLUMNS_LANGUAGE_H_

#include <gtkmm.h>
#include "DataModel/speakers/Speaker.h"
#include "Common/Languages.h"

#define SPEAKER_LANGUAGE_COL_CODE 0
#define SPEAKER_LANGUAGE_COL_NAME 1
#define SPEAKER_LANGUAGE_DIALECT 2
#define SPEAKER_LANGUAGE_ACCENT 3
#define SPEAKER_LANGUAGE_NATIVE 4
#define SPEAKER_LANGUAGE_USUAL 5

namespace tag {
/**
 * @class 		ListModelColumns_language
 * @ingroup		GUI
 *
 * Definition of the Gtk::TreeModel::ColumnRecord used for the languages list used
 * by SpeakerData class\n
 *
 * Represents a language entry:\n
 *  - language\n
 *  - dialect\n
 *  - accent\n
 *  - isUsual\n
 *  - isNative\n
 */
class ListModelColumns_language : public Gtk::TreeModel::ColumnRecord
{
	public:
		/**
		 * 	Constructor
		 */
		ListModelColumns_language() : Gtk::TreeModel::ColumnRecord()
		{
			add(a_code) ;
			add(a_name) ;
			add(a_dialect) ;
			add(a_accent) ;
			add(a_isNative) ;
			add(a_isUsual) ;
			add(a_activatable_combo) ;
			add(a_activatable_toggle) ;
		}
		virtual ~ListModelColumns_language(){};


		Gtk::TreeModelColumn<Glib::ustring> a_code; 						/**< Iso639-3 code of the languages */
		Gtk::TreeModelColumn<Glib::ustring> a_name;							/**< display of the languages */
		Gtk::TreeModelColumn<Glib::ustring> a_dialect;						/**< display of the dialect */
		Gtk::TreeModelColumn<Glib::ustring> a_accent;						/**< display of the accent */
		Gtk::TreeModelColumn<bool> a_isNative ;								/**< is the language native ? */
		Gtk::TreeModelColumn<bool> a_isUsual;								/**< is the language usual ? */
		Gtk::TreeModelColumn<Gtk::CellRendererMode> a_activatable_combo;	/**< property for enabling ComboBoxText in cells */
		Gtk::TreeModelColumn<bool> a_activatable_toggle;					/**< property for enabling CheckBox in cells */

		/**
		 * Static wrapper for filling a row using the current model column
		 * @param row				Pointer on the row we want to fill
		 * @param language			Pointer on Speaker::Language object
		 * @param editable			Editability of the row
		 * @param language_list		Pointer to class Languages
		 */
		static void fill_row(Gtk::TreeRow* row, Speaker::Language* language, bool editable, Languages* language_list) ;
};

} //namespace

#endif /*LISTMODELCOLUMNS_LANGUAGE_H_*/
