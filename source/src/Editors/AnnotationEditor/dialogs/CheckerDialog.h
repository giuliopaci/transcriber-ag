/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/**
 *  @file 	CheckerDialog.h
 */

#ifndef CHECKERDIALOG_H_
#define CHECKERDIALOG_H_

#include <gtkmm.h>

#include "Common/icons/IcoPackImage.h"
#include "Common/icons/IcoPackButton.h"
#include "Common/util/Utils.h"
#include "DataModel/DataModel.h"
#include "DataModel/conventions/ModelChecker.h"

namespace tag {
/**
*  @class 		CheckerDialog
*  @ingroup 	AnnotationEditor
*
*  Dialog used for displaying loading results.\n
*  Based on CheckerModel results.\n\n
*/
class CheckerDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param data			Reference on the model used by editor
		 * @param filepath		Path of the file beeing loaded
		 * @param fixEnabled	True for allowing user to fix errors, False otherwise
		 */
		CheckerDialog(DataModel* data, const string& filepath, bool fixEnabled);

		/**
		 * Destructor
		 */
		virtual ~CheckerDialog();

		/**
		 * Signal emitting when modifications has been applied to the model
		 */
		sigc::signal<void>& signalModelModified() { return m_signalModelModified ; }

	private:

		/** allow errors correction **/
		bool fixEnabled ;

		/** structure **/
		int button_cpt ;
		IcoPackButton button_close ;
		IcoPackButton button_apply ;
		Gtk::ScrolledWindow scrolledW ;
		Gtk::VBox expand_box ;
		Gtk::HBox hbox_title ;

		/** models **/
		DataModel* model ;
		ModelChecker* checker ;
		string filepath ;

		/** Widget for header line **/
		class CheckerDialogSection : public Gtk::Expander
		{
			public :
				/**
				 * Constructor
				 * @param p_label	Label to be displayed
				 * @param icon		Icon
				 * @param mode	 	"info", "warning", "error"
				 */
				CheckerDialogSection(Glib::ustring p_label, Glib::ustring icon, Glib::ustring mode) ;

				/**
				 * Adds a new element inside the expander
				 * @param widget
				 */
				void addElement(Gtk::Widget* widget) ;

			private :

				int nbElements ;		/**< nb entry **/

				/** structure **/
				Gtk::Expander expand ;
				Gtk::VBox vbox ;
				IcoPackImage image ;
				Gtk::HBox header ;
				Gtk::Label number ;
		} ;

		/** Widget line for an error **/
		class CheckerDialogEntry : public Gtk::VBox
		{
			public:
				/**
				 * Constructor
				 * @param p_label		String to display
				 * @param detailed		Detailed messaged displayed in a text zone
				 * @param p_tip			Tooltip
				 * @param checkTip		Check button tip
				 * @param errorCode		ErrorCode
				 * @param canBeFixed	If true, a checkbox is displayed
				 */
				CheckerDialogEntry(Glib::ustring p_label, Glib::ustring detailed,
										Glib::ustring p_tip, Glib::ustring checkTip,
										int errorCode, bool canBeFixed) ;

				/**
				 * Accessor to the checkbutton
				 * @return		Pointer on entry checkbutton
				 */
				Gtk::CheckButton* getChekButton() { return &checkb ;}

				/**
				 * Gets the error associated to the entry
				 * @return		Error code
				 * @see 		ModelChecker class
				 */
				int getErrorCode() { return error_code ;}

				/**
				 * Displays the result of the apply action
				 */
				void actualizeDisplay(int error_code, int state) ;

			private :
				Gtk::Expander expand ;
				int error_code ;
				Gtk::HBox hbox ;
				Gtk::HBox checkHbox ;
				Gtk::CheckButton checkb ;
				Gtk::Tooltips tip ;
				IcoPackImage image ;
		} ;

		std::vector<CheckerDialogEntry*> entries ;
		std::map<string,CheckerDialogSection*> expands ;

		/*** Data business ***/
		CheckerDialogEntry* addEntry(CheckerDialogSection* expander, ModelChecker::CheckGraphResult* checkResult, Glib::ustring p_label, Glib::ustring detailed, Glib::ustring tip, Glib::ustring checkTip, int errorCode, bool canBeFixed) ;
		CheckerDialogSection* addSection(Glib::ustring label, Glib::ustring icon, Glib::ustring mode) ;
		void loadData() ;
		void loadInfo() ;
		void loadWarningsErrors(int priority) ;

		/*** gui business ***/
		void prepareGUI() ;
		void adjustDialogSize() ;
		void displayCheckResult(CheckerDialogSection* expander, ModelChecker::CheckGraphResult* checkResult) ;
		void displayError(CheckerDialogSection* expander, ModelChecker::CheckGraphResult* checkResult, int errorCode) ;
		void onCheckboxChanged(CheckerDialogEntry* entry, ModelChecker::CheckGraphResult* checkResult) ;
		void onButtonReleased(const string& mode) ;

		/*** Clean business ***/
		void setCleanCandidate(bool add, ModelChecker::CheckGraphResult* checkResult, int errorCode) ;
		bool CleanModel() ;

		void flushGUI() ;

		sigc::signal<void> m_signalModelModified ;

};

}

#endif /* CHECKERDIALOG_H_ */
