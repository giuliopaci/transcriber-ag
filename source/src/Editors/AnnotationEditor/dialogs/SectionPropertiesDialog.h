/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 	SectionPropertiesDialog.h
 */

#ifndef __HAVE_SECTIONPROPERTIESDIALOG__
#define __HAVE_SECTIONPROPERTIESDIALOG__

#include "Editors/AnnotationEditor/dialogs/AnnotationPropertiesDialog.h"

#include "Common/widgets/ComboEntry_mod.h"
#include "Common/InputLanguage.h"
#include "Common/InputLanguageHandler.h"

using namespace std;

namespace tag {

/**
 * @class SectionPropertiesDialog
 *
 * Dialogs for displaying and editing qualifier properties
 */
class SectionPropertiesDialog : public AnnotationPropertiesDialog
{
	public:
		/**
		 * Constructor
		 * @param p_win				Reference on parent window
		 * @param p_dataModel		Reference on datamodel
		 * @param p_elementId		Element id
		 * @param editable			True for edition mode, False otherwise
		 */
		SectionPropertiesDialog(Gtk::Window& p_win, DataModel& p_dataModel, const string& p_elementId, bool editable);
		virtual ~SectionPropertiesDialog();


		/**
		 * Accessor to the element type
		 * @return		Element type or empty string if not defined
		 */
		const string& get_type() {return m_type;}

		/**
		 * Accessor to the element description
		 * @return		Element description or empty string if not defined
		 */
		const string& get_desc() {return a_desc;}


		/**
		 * Accessor to the element topic
		 * @return		Element topic or empty string if not defined
		 */
		const string& get_topic() {return a_topic;}

		/**
		 * Accessor to the available element types list
		 * @return		A vector with all available element types
		 */
		const std::vector<std::string>& get_typeList() { return subtypes_list ; }

		/**
		 * Accessor to the available element type labels list
		 * @return		A vector with all available element type labels
		 */
		const std::vector<std::string>& get_typeLabel() { return subtypes_label ; }

		/**
		 * Accessor to the available element description list
		 * @return		A vector with all available element descriptions
		 */
		const std::vector<std::string>& get_descList() { return desc_list ; }

		/**
		 * Accessor to the available element description labels list
		 * @return		A vector with all available element description labels
		 */
		const std::vector<std::string>& get_descLabel() { return desc_label ; }

	private:
		/** ARCHITECTURE **/
		Gtk::ComboBoxText* a_typeEntry;
		ComboEntry_mod* a_descEntry;
		Gtk::HBox hbox_entry ;
		Gtk::HBox hbox_language ;
		Gtk::HSeparator sep_language ;
		Gtk::Label label_inputLanguage ;
		Gtk::ComboBoxText combo_language ;
		Gtk::HBox topic_hbox ;
		Gtk::Entry topic_entry ;
		Gtk::Label topic_label ;
		Gtk::Button topic_button ;
		Gtk::Button topic_no_button ;

		InputLanguage* m_iLang ;

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
		// desc topic
		string a_topic ;

		// a_desc will be set to locale value
		// Let's keep the true value of desc before localization
		string a_desc_orig ;

		string title ;

		/*** Dialog modes ***/
		// topic feature available
		bool with_topic ;
		// is type feature editable ?
		bool editable_type ;

		/** Configuration **/
		std::map<Glib::ustring,Topics*> topic_list ;
		std::vector<std::string> subtypes_list ;
		std::vector<std::string> subtypes_label ;
		std::vector<std::string> desc_list ;
		std::vector<std::string> desc_label ;

		/** Methods **/
		bool prepare_data() ;
		void prepare_gui() ;
		void display_error() ;
		void prepareResultValues() ;

		void on_topic_change(Glib::ustring mode) ;
		void onButtonClicked(int p_id);
		void on_change_language() ;
		void set_combo_language() ;
		void on_type_change() ;
} ;

}

#endif // __HAVE_SECTIONPROPERTIESDIALOG__
