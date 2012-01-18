/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef TREEMODEL_COLUMNS_H_
#define TREEMODEL_COLUMNS_H_

#include <gtkmm.h>

namespace tag {


#define M_COL_FILE_NAME "Name"
#define M_COL_FILE_DISPLAY "Display"
#define M_COL_FILE_DESC "Description"
#define M_COL_FILE_ICO "Icon"
#define M_COL_FILE_SYSTYPE "Systype"
#define M_COL_FILE_ROOT "Root"
#define M_COL_FILE_ROOTTYPE "RootType"
#define M_COL_FILE_NBFILES "NBFilteredFiles"
#define M_COL_FILE_DISPLAYWNUMBER "DisplayWNumber"

#define M_COL_FILE_NAME_INDICE 0
#define M_COL_FILE_DISPLAY_INDICE 1
#define M_COL_FILE_DESC_INDICE 2
#define M_COL_FILE_ICO_INDICE 3
#define M_COL_FILE_SYSTYPE_INDICE 4
#define M_COL_FILE_ROOT_INDICE 5
#define M_COL_FILE_ROOTTYPE_INDICE 6
#define M_COL_FILE_NBFILES_INDICE 7
#define M_COL_FILE_DISPLAYWNUMBER_INDICE 8

/**
 * @class 		TreeModel_Columns
 * @ingroup		GUI
 *
 * Definition of the Gtk::TreeModel::ColumnRecord used for the file tree explorer
 *
 * Each row represents an explorer file entry
 *  - icons\n
 *  - display name\n
 *  - number of transcription file available (only for directory) \n
 *  - other information used but not displayed
 *
 */
class TreeModel_Columns : public Gtk::TreeModel::ColumnRecord
{
	public :
		/**
		 * Constructor
		 */
		TreeModel_Columns()
		{
			add(m_file_name) ;
			add(m_file_display) ;
			add(m_file_desc) ;
			add(m_file_ico) ;
			add(m_file_sysType) ;
			add(m_file_root) ;
			add(m_file_rootType) ;
			add(m_file_nbFilteredFiles) ;
			add(m_file_displayWeight) ;
			add(m_file_ico2) ;
			add(m_file_isExpander) ;
			add(m_file_display_wNumber) ;
		}

		/**
		 * @var m_file_display_wNumber
		 * Specific member to fix display packing problem for number of filtered files.\n
		 * @attention	 Only m_file_display_wNumber is displayed, addition of
		 * 				 TreeModel_Columns::m_file_display & TreeModel_Columns::"m_file_nbFilteredFiles".\n
		 * 				 THEREFORE TreeModel_Columns::m_file_display_wNumber needs to be actualized
		 * 				 each time one of these fields are changed.
		 * @note 		 Should get a better fix.
		 */
		Gtk::TreeModelColumn<Glib::ustring> m_file_display_wNumber ;

		Gtk::TreeModelColumn<Glib::ustring> m_file_display ;			/**< Name displayed */
		Gtk::TreeModelColumn<Glib::ustring> m_file_nbFilteredFiles ;	/**< Number of available transcription files (only for directories) */
		Gtk::TreeModelColumn<Glib::ustring> m_file_name ;				/**< File base name */

		/**
		 * @var m_file_desc
		 * Information file in TAG info format
		 * @deprecated 	Useless
		 */
		Gtk::TreeModelColumn<Glib::ustring> m_file_desc ;

		/**
		 * @var m_file_sysType
		 * System type of the file.\n
		 * -1: virtual row (used for filling folder)\n
		 *  0: file\n
		 *  1: directory\n
		 *  2: system root\n
		 *  3: project root\n
		 *  6: shortcut root\n
		 */
		Gtk::TreeModelColumn<int> m_file_sysType ;

		/**
		 * @var m_file_ico
		 * Closed icon name
		 * @see Icons class
		 */
		Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > m_file_ico ;

		/**
		 * @var m_file_ico2
		 * Opened icon name
		 * @see Icons class
		 */
		Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > m_file_ico2 ;

		/**
		 * @var m_file_root
		 * 	Indice of the tree the row belongs to
		 * @see Explorer_tree::get_rootNumber()
		 */
		Gtk::TreeModelColumn<int> m_file_root ;

		/**
		 * @var m_file_rootType
		 * Indicates from with type of tree the row belongs to.\n
		 * 2: filesystem folder
		 * 3: project tree folder
		 * 6: shortcut folder
		 */
		Gtk::TreeModelColumn<int> m_file_rootType ;
		Gtk::TreeModelColumn<int> m_file_displayWeight ;	/**< Weight display of the name */
		Gtk::TreeModelColumn<bool> m_file_isExpander ;		/**< Used for allowing closed icon and opened icon */

		/**
		 * Static wrapper for filling a row using the current model column
		 * @param row			Pointer on the row to be filled
		 * @param name			Display file name
		 * @param path			File path
		 * @param desc			Information file in TAG info format - deprecated
		 * @param ico			Defined ico for closed presentation
		 * @param ico2			Defined ico for opened presentation (only for directories)
		 * @param sysType		System type of the file
		 * @param root			Number of the tree the row belongs too
		 * @param rootType		Type of the tree the row belongs too
		 */
		static void fill_row(Gtk::TreeModel::Row* row, const Glib::ustring& name,
								const Glib::ustring& path, const Glib::ustring& desc,
								const Glib::ustring& ico, const Glib::ustring& ico2, int sysType,
								int root, int rootType) ;
} ;

} //namespace


#endif
