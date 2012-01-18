/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 	QualifierPropertiesDialog.h
 */

#ifndef __HAVE_QualifierPropertiesDialog__
#define __HAVE_QualifierPropertiesDialog__

#include "Editors/AnnotationEditor/dialogs/AnnotationPropertiesDialog.h"

#include "Common/widgets/ComboEntry_mod.h"
#include "Common/InputLanguage.h"
#include "Common/InputLanguageHandler.h"

using namespace std;

namespace tag {


/**
 * @class QualifierPropertiesDialog
 *
 * Dialogs for displaying and editing qualifier properties
 */
class QualifierPropertiesDialog : public AnnotationPropertiesDialog
{
	public:
		/**
		 * Constructor
		 * @param p_win				Reference on parent window
		 * @param p_dataModel		Reference on AnnotationView parent
		 * @param p_elementId		Element id
		 * @param editable			True for edition mode, False otherwise
		 */
		QualifierPropertiesDialog(Gtk::Window& p_win, DataModel& p_dataModel, const string& p_elementId, bool editable);
		virtual ~QualifierPropertiesDialog();

		/**
		 * COnfigure menus label
		 * @param labels	Label layout map
		 */
		void configureMenuLabels(const std::map<string, string>& labels);

		/**
		 * Accessor to the element type
		 * @return		Element type or empty string if not defined
		 */
		const string& get_type() {return a_type;}

		/**
		 * Accessor to the element description
		 * @return		Element description or empty string if not defined
		 */
		const string& get_desc() {return a_desc;}

		/**
		 * Accessor to the element normalization
		 * @return		Element normalization or empty string if not defined
		 */
		const string& get_norm() {return a_norm;}

		/**
		 * Accessor to the available element types list
		 * @return		A vector with all available element types
		 */
		const std::vector<std::string>& get_typeList() { return a_subtypeList ; }

		/**
		 * Accessor to the available element type labels list
		 * @return		A vector with all available element type labels
		 */
		const std::vector<std::string>& get_typeLabel() { return a_subtypeLabels ; }

		/**
		 * Accessor to the available element description list
		 * @return		A vector with all available element descriptions
		 */
		const std::vector<std::string>& get_descList() { return a_descList ; }

		/**
		 * Accessor to the available element description labels list
		 * @return		A vector with all available element description labels
		 */
		const std::vector<std::string>& get_descLabel() { return a_descLabel ; }

	private:
		/** ARCHITECTURE **/
		Gtk::ComboBoxText* a_typeEntry;
		ComboEntry_mod* a_descEntry;
		Gtk::HBox hbox_entry ;
		Gtk::HBox hbox_language ;
		Gtk::HSeparator sep_language ;
		Gtk::Label label_inputLanguage ;
		Gtk::ComboBoxText combo_language ;
		Gtk::HBox normalization_hbox ;
		Gtk::Entry normalization_entry ;
		Gtk::Label normalization_label ;


		/** DATA RECEIVED **/
		// qualifier can be normalized ?
		bool a_normalize ;
		string a_qualifierType ;
		std::map<string, string> a_menuLabels;
		InputLanguage* a_iLang ;
		std::map<string, string> a_lastDesc;

		/** RESULT **/
		/*** The 4 following parameters are used to keep the latest chosen values
		 *	 in order to make them reachable after dialog validation (with accessors)
		 ***/
		// type feature
		string m_type ;
		// type feature displayed
		string a_type ;

		// desc feature
		string a_desc ;
		// desc norm
		string a_norm ;


		// a_desc will be set to locale value
		// Let's keep the true value of desc before localization
		string a_desc_orig ;

		/*** Dialog modes ***/

		/** Configuration **/
		std::vector<std::string> a_subtypeList ;
		std::vector<std::string> a_subtypeLabels ;
		std::vector<std::string> a_descList ;
		std::vector<std::string> a_descLabel ;

		/** Methods **/
		bool prepare_data() ;
		void prepare_gui() ;
		void display_error() ;
		void prepareResultValues() ;

		void onButtonClicked(int p_id);
		void on_change_language() ;
		void set_combo_language() ;
		void on_type_change() ;
		void del_underscore(string &st);
} ;

}

#endif // __HAVE_DIALOGFILEPROPERTIES__
