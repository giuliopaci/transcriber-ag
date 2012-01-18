/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef TREEMODELCOLUMNSPREFERENCES_H_
#define TREEMODELCOLUMNSPREFERENCES_H_

#include <gtkmm.h>
#include <TreeViewPreferences.h>

namespace tag {
/**
 * @class 	TreeModelColumnsPreferences
 * @ingroup	GUI
 *
 * Gtk::TreeModel::ColumnRecord implementation for the tree packed into the PreferencesDialog
 */
class TreeModelColumnsPreferences : public Gtk::TreeModel::ColumnRecord
{
	public:
		/**
		 *	Constructor
		 */
		TreeModelColumnsPreferences()
		{
			add(m_config_type) ;
			add(m_config_name) ;
			add(m_config_ico) ;
			add(m_config_weight) ;
		}
		virtual ~TreeModelColumnsPreferences();

		/**
		 * Static wrapper for filling a row using the current model column
		 * @param row		Pointer on the row we want to fill
		 * @param ico_name	Defined icon name (see Icons class)
		 * @param type		Stamp value defining which frame we are filling\n
		 * 					0:  general frame\n
		 * 					1:  <em>deprecated</em>\n
		 * 					2:  <em>deprecated</em>\n
		 * 					3:  text editor frame\n
		 * 					4:  speller frame\n
		 * 					5:  audio frame\n
		 * 					6:  speaker frame\n
		 * 					7:  look 'n feel frame\n
		 * 					71: colors sub frame\n
		 * 					72: font sub frame
		 * @param name		Name to display
		 * @param weight	Display weight
		 */
		static void fill_row(Gtk::TreeModel::Row* row, const Glib::ustring& ico_name, int type, const Glib::ustring& name, int weight) ;

		/**
		 * @var m_config_ico
		 * Gtk::TreeModelColumn for icon name
		 */
		Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > m_config_ico ;

		/**
		 * @var m_config_type
		 * Gtk::TreeModelColumn for frame type
		 * @see fill_row(Gtk::TreeModel::Row* row, const Glib::ustring& ico_name, int type, const Glib::ustring& name, int weight)
		 */
		Gtk::TreeModelColumn<int> m_config_type ;

		/**
		 * @var m_config_name
		 * Gtk::TreeModelColumn for display name
		 */
		Gtk::TreeModelColumn<Glib::ustring> m_config_name ;

		/**
		 * @var m_config_weight
		 * Gtk::TreeModelColumn for the weight of the display name
		 */
		Gtk::TreeModelColumn<int> m_config_weight ;
};

} //namespace

#endif /*TREEMODELCOLUMNSPREFERENCES_H_*/
