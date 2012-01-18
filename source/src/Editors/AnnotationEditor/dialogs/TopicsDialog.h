/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
*  @file 		TopicsDialog.h
*/

#ifndef TOPICSDIALOG_H_
#define TOPICSDIALOG_H_

#include <gtkmm.h>
#include "DataModel/conventions/Topics.h"

namespace tag {

//******************************************************************************
//***************************** Topic TREEVIEW *********************************
//******************************************************************************
/**
* @class 		TopicsTreeView
* @ingroup		AnnotationEditor
*
* Specific TreeView used for displaying available topics.\n
*
*/
class TopicsTreeView : public Gtk::TreeView
{
	public :
		/**
		 * Constructor
		 */
		TopicsTreeView() ;

		/**
		 * Desctructor
		 */
		~TopicsTreeView() {} ;

		/**
		 * Sets the model tree connected to the view.
		 * @param modelTree		Pointer on associated model
		 */
		void set_modelTree(Glib::RefPtr<Gtk::TreeStore> modelTree) { model = modelTree ; }

		/**
		 * Indicates the parent window.
		 * @param parent	Pointer on the parent window
		 */
		void set_window(Gtk::Window* parent) { m_parent = parent ; }

		/**
		 * Accessor to the last selected iterator.
		 * @param result		Reference on the last selected iterator
		 */
		void get_selected(Gtk::TreeIter& result) ;

		/**
		 * Signal emitted when the tree selection changes
		 * <b>std::vector<Gtk::TreeIter> paramter:</b>		New selected iterator
		 */
		sigc::signal<void, std::vector<Gtk::TreeIter> >& signalSelection() { return m_signalSelection; }

	private :
		Gtk::Window* m_parent ;
		Gtk::TreeIter selection_active_iter ;
		Glib::RefPtr<Gtk::TreeStore> model ;
		virtual void on_cursor_changed() ;
		//signal emitted each time a row is selected (simple click)
		//pass iter
		sigc::signal<void, std::vector<Gtk::TreeIter> > m_signalSelection ;
} ;


//******************************************************************************
//***************************** Topic MODEL COLUMNS*****************************
//******************************************************************************
/**
* @class 		TopicsModelColumns
* @ingroup		AnnotationEditor
*
* Specific TreeModelColumns used for displaying available topics.\n
*
*/
class TopicsModelColumns : public Gtk::TreeModel::ColumnRecord
{
	public:
		/**
		 * Constructor
		 */
		TopicsModelColumns() : Gtk::TreeModel::ColumnRecord()
		{
			add(a_type) ;
			add(a_id) ;
			add(a_label) ;
			add(a_displayWeight) ;
		}

		/**
		 * Destructor
		 */
		virtual ~TopicsModelColumns() {} ;

		Gtk::TreeModelColumn<Glib::ustring> a_id ;		/**< Topic or topic group id */
		Gtk::TreeModelColumn<Glib::ustring> a_label ;	/**< Topic or topic group label */
		/**
		 * @var a_type
		 * Internal type:\n
		 * - 0: topic\n
		 * - 1: group of topics
		 */
		Gtk::TreeModelColumn<int> a_type ;
		Gtk::TreeModelColumn<int> a_displayWeight ;		/**< Label font weight */

		/**
		 * Static method for easily fill a topic row
		 * @param row		Pointer on the row to be filled
		 * @param type		Internal element type
		 * @param id		Topic or topic group id
		 * @param label		Topic or topic group label
		 */
		static void fill_row(Gtk::TreeModel::Row* row, int type, const Glib::ustring& id, const Glib::ustring& label) ;
} ;


//******************************************************************************
//***************************** DETAILS WINDOW *********************************
//******************************************************************************
/**
* @class 		TopicsDetailDialog
* @ingroup		AnnotationEditor
*
* Dialog displaying topic details.\n
* These information are caught from a specific XML file.
*
*/
class TopicsDetailDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor
		 * @param topic		Pointer on the corresponding topic
		 * @param win		Pointer on the parent window
		 */
		TopicsDetailDialog(Topic* topic, Gtk::Window* win) ;

		/**
		 * Destructor
		 */
		virtual ~TopicsDetailDialog() ;

	private:
		Topic* m_topic ;
		Gtk::VBox* dlgVbox ;
			Gtk::Alignment align_title ;
				Gtk::Label label_title ;
			Gtk::ScrolledWindow scrolled ;
			Gtk::VBox vbox ;
			Gtk::Button* button_close ;

		class Details : public Gtk::Frame
		{
			public:
				Details(Glib::ustring title, Glib::ustring value)
				{
					add(vbox) ;
					vbox.pack_start(align_title, false, false, 1) ;
						align_title.add(label_title) ;
					vbox.pack_start(view, false, false, 1) ;
					align_title.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER,0, 0) ;
					label_title.set_label(title) ;
					label_title.set_name("bold_label") ;
					view.get_buffer()->set_text(value) ;
					view.set_name("topics_details_view") ;
					vbox.set_name("topics_details_box") ;
					view.set_editable(false) ;
					show_all_children() ;
				}
			protected:
				Gtk::Alignment align_title ;
					Gtk::Label label_title ;
				Gtk::TextView view ;
				Gtk::VBox vbox ;
		} ;
		std::vector<Details*> details ;
		void addDetail(Glib::ustring title, Glib::ustring value) ;
} ;

//******************************************************************************
//***************************** TopicsDialog CLASS *****************************
//******************************************************************************
/**
* @class 		TopicsDialog
* @ingroup		AnnotationEditor
*
* Dialog displaying all available topics; enables user to
* display topic specification and to choose one of the listed topics.
*/
class TopicsDialog : public Gtk::Dialog
{
	public:
		/**
		 * Constructor.
		 * @param topics	Topics list ( map <topic group id - topic group object> )
		 * @param win		Reference on parent window
		 */
		TopicsDialog(std::map<Glib::ustring, Topics*>& topics, Gtk::Window* win) ;
		virtual ~TopicsDialog() ;

		/**
		 * Accessor to chosen information
		 * @param[out] id		Chosen id, or empty if none was selected
		 * @param[out] label	Chosen label, or empty if none was selected
		 * @return				True if a topic has been chosen, False otherwise
		 */
		bool get_chosen(Glib::ustring& id, Glib::ustring& label) ;

		/**
		 * Activate the light mode.\n
		 * The light mode only enables to choose a given topic (no details displayed)
		 */
		void set_light_mode() ;

	private:
		bool m_light_mode ;

		Glib::ustring m_id_result ;
		Glib::ustring m_label_result ;
		Gtk::TreeIter selected ;

		//> WIDGETS
		Gtk::VBox* dlgVbox ;
			Gtk::Alignment align_title ;
				Gtk::Label label_title ;
			Gtk::VSeparator sep_title ;
			Gtk::ScrolledWindow scrolledw ;
				TopicsTreeView m_view ;
			Gtk::Button* button_ok ;
			Gtk::Button* button_cancel ;
			Gtk::Button* button_details ;

		//> list model
		Glib::RefPtr<Gtk::TreeStore> m_refModel ;
		Glib::RefPtr<Gtk::TreeModelSort> m_refSortedModel ;

		//> modelcolumns
		TopicsModelColumns m_modelColumns ;

		/* topic group id - topics */
		std::map<Glib::ustring, Topics*>& m_topics ;
		bool on_key_press_event(GdkEventKey* event) ;
		void prepare_tree() ;
		void fill_model() ;
		int sort_tree(const Gtk::TreeModel::iterator& it1, const Gtk::TreeModel::iterator& it2) ;
		void on_selection_changed(std::vector<Gtk::TreeIter> paths) ;

		void on_selection(std::vector<Gtk::TreeIter> paths) ;
		void on_row_activated(const Gtk::TreeModel::Path&, Gtk::TreeViewColumn* c) ;

		int get_selected(Glib::ustring& result_id, Glib::ustring& result_label) ;
		void on_details_clicked() ;
};


}

#endif /*TOPICSDIALOG_H_*/
