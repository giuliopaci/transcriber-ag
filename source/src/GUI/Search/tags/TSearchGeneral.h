/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef TSEARCHGENERAL_H_
#define TSEARCHGENERAL_H_

#include "AnnotationEditor/AnnotationEditor.h"
#include "AnnotationEditor/AnnotationView.h"
#include "Common/icons/IcoPackButton.h"
#include "Common/icons/IcoPackImage.h"
#include "Common/widgets/ComboEntry_mod.h"
#include <gtkmm.h>
#include <iostream>

#include "SpeakerDico_dialog.h"
#include "../SearchGeneral.h"

#include "Editors/AnnotationEditor/dialogs/TopicsDialog.h"
#include "DataModel/conventions/Topics.h"

namespace tag {

class InputLanguage ;
/**
 * @class 	TSearchGeneral
 * @ingroup	GUI
 *
 * Implementation of the general mechanism for research in tags property.
 * This version enables to search section with given topic or turn of given speaker.
 *
 * The research of named entities / event of given type isn't implemented yet in this component,
 * but is possible with the text search components (SearchReplaceGeneral class).
 *
 * @note 	The tag search toolbar mode isn't implemented yet, only dialog mode is available
 *
 */
class TSearchGeneral : public SearchGeneral
{
	public:
		/**
		 * Constructor
		 * @return
		 */
		TSearchGeneral();
		virtual ~TSearchGeneral();

		/* Abstract method implementation */
		void init(AnnotationEditor* editor) ;
		void close() ;
		void switch_file(AnnotationEditor* editor) ;
		bool is_active() {return active ; }
		void on_search_fw() ;
		void on_search_bk() ;

		/**
		 * Set the focus of the tag search components
		 */
		virtual void mySet_focus()=0 ;

		/**
		 * Hide the tag search components
		 */
		virtual void myHide()=0 ;

		/**
		 * Deprecated
		 */
		void check_initial_selection() ;

		/**
		 * Accessor to the external editor connected to the tag search components
		 * @return		The external AnnotationEditor using the tag search components
		 */
		AnnotationEditor* get_editor() { return external_editor ; }

	protected:

		//------------------------------- WIDGETS

		/*
		 *  Available types for search
		 */
		Gtk::RadioButton rb_section ;	/**< Topic scope radio button */
		Gtk::RadioButton rb_turn ;		/**< Speaker scope radio button */
		Gtk::RadioButton rb_entity ;	/**< Entity scope radio button - DEPRECATED */
		Gtk::RadioButton rb_event ;		/**< Event scope radio button - DEPRECATED */

		Gtk::Label label_value ;
		ComboEntry_mod combo_value ;
		Gtk::Button button_value ;

		IcoPackButton find_bk ;			/**< search backward button */
		IcoPackButton find_fw ;			/**< search forward button */
		IcoPackButton close_button ;		/**< close button */

		Gtk::Frame search_in_progress_frame ;
		Gtk::HBox search_in_progress_hbox ;
		IcoPackImage search_in_progress ;	/**< search_in_progress gif */
		IcoPackImage search_static ;	/**< search_in_progress gif */

		Glib::ustring selected_id ;				/**< last candidate ID (ID from annotation graph) */
		Glib::ustring last_overlap_checked ;	/**< close button */

		//> -------------------------------- INTERNAL

		std::map<Glib::ustring,Topics*> topic_list ;  	/**< Application available topics\n All topic that can be chosen for matching term */
		SpeakerDico_dialog* dico ; 						/**< External speaker dictionary - used to stock Pointer */

		/**
		 * @var m_type
		 * Selected researched type (section or type)
		 */
		std::string m_type ;

		/**
		 * @var 	m_property
		 * @brief 	Annotation graph feature the search compoenent deals with
		 * Depending on search mode, the property the component will compare changes:\n
		 * - searching topic into sections, the property is the feature "topic"
		 * - searching speaker turns, the property is the feature "speaker"
		 * - searching a qualifier, the property is the feature "text"
		 * @note 	The event / named entity research isn't yet implemented in this version\n
		 * but the function is available with the text search component
		 */
		std::string m_property ;

		/**
		 * @var m_value_topic
		 * The topic id currently searched in section mode
		 */
		std::string m_value_topic ;

		/**
		 * @var m_value_speaker
		 * The speaker id currently searched in turn mode
		 */
		std::string m_value_speaker ;

		/**
		 * @var m_value_qualifier
		 * The qualifier type currently searched in qualifier mode
		 */
		std::string m_value_qualifier ;

		/* Stock all internal signals */
		sigc::connection connection_find_fw ;			/**< Connection to find forward button */
		sigc::connection connection_find_bk ;			/**< Connection to find backward button */
		sigc::connection connection_close_button ;		/**< Connection to close button */
		sigc::connection connection_combo_search ;		/**< Connection to search  combo entry */
		sigc::connection connection_rb_section ;		/**< Connection to section scope radio button */
		sigc::connection connection_rb_turn ;			/**< Connection to turn scope radio button */
		sigc::connection connection_rb_event ;			/**< Connection to qualifier event scope radio button */
		sigc::connection connection_rb_entity ;			/**< Connection to qualifier entity scope radio button */
		sigc::connection connection_button_value;		/**< Connection to change searched value button */

	private :

		bool cancel ;
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> METHODS

		void on_button_value_clicked() ;
		void reset_mode(Glib::ustring type) ;
		void on_close_speakerdialog() ;

		void on_type_changed(std::string type) ;
		Glib::ustring get_first_value(Glib::ustring type) ;
		void reset() ;

		bool search_error() ;

		void on_change_language() ;
		void set_combo_language() ;

		void search_end() ;
		void selection_mode(bool value) {} ;
		void on_change_cursor(const Gtk::TextBuffer::iterator& it, const Glib::RefPtr<Gtk::TextBuffer::Mark >& mark) ;
		void on_buffer_changed() ;
		void on_change_view() ;

		void remove_occurence(const string& tagname) ;
		void actualize_occurence(const std::string& id, const string& tagname) ;
		bool my_forward_search(Gtk::TextIter iter) ;
		bool my_backward_search(Gtk::TextIter iter) ;
		Glib::ustring check_conditions(Gtk::TextIter iter) ;
		Glib::ustring check_conditions(const string& id) ;

		std::string getSearchedParentElement(const string& id) ;

		void prepareTag() ;
		void searchingStatus(bool searching) ;

};

} //namespace


#endif /*SEARCHREPLACEGENERAL_H_*/
