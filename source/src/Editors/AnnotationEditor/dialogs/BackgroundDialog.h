/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/**
 *  @file 	BackgroundDialog.h
 */

#ifndef BACKGROUNDDIALOG_H_
#define BACKGROUNDDIALOG_H_

#include <gtkmm.h>

#include "DataModel/UndoableDataModel.h"

using namespace std ;

namespace tag {
/**
*  @class 		BackgroundDialog
*  @ingroup 	AnnotationEditor
*
*  Basic dialog used for background creation, edition and deletion.\n
*  Used for displaying information over some tagged elements.\n\n
*/
class BackgroundDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param parent			Pointer on the parent window
		 * @param model				Reference on the model
		 * @param id				Background id
		 * @param can_edit			True if edition is enabled, False otherwise
		 * @param for_new_item		True for creation, false for edition
		 * @param notrack			Impacted view
		 * @param startOffset		Background start offset
		 * @param endOffset			Background end offset
		 */
		BackgroundDialog(Gtk::Window* parent,
				DataModel& model, const std::string& id,
				bool can_edit, bool for_new_item,
				int notrack=1, float startOffset=-1, float endOffset=-1);
		virtual ~BackgroundDialog();

	private:
		Gtk::Window* m_parent ;
		DataModel& m_dataModel;

		Glib::ustring combo_no_value ;
		Gtk::Table* type_table ;
		Gtk::Label type_label ;
		Gtk::VBox type_vbox ;
		Gtk::Label level_label ;
		Gtk::ComboBoxText level_combo ;
		Gtk::Label offset_label ;
		Gtk::Label offset_value ;

		Gtk::Button* deleteButton ;

		std::map<std::string, Gtk::CheckButton*> type_checks ;

		string m_id;

		std::vector<string> m_chosen_types ;
		string m_chosen_level ;

		std::vector<string> m_types ;
		std::vector<string> m_levels ;

		float m_startOffset;
		float m_endOffset;
		int m_notrack;

		bool m_editable  ;

		bool new_item ;

		void onButtonClicked(int response_id) ;
		void delete_background() ;
		bool saveData() ;
};

}

#endif /* BACKGROUNDDIALOG_H_ */
