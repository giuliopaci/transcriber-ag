/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "TopicsDialog.h"
#include "Common/globals.h"
#include "Common/Dialogs.h"
#include "Common/icons/Icons.h"
#include "Common/VersionInfo.h"

namespace tag {

//******************************************************************************
//***************************** Topic TREEVIEW *********************************
//******************************************************************************

TopicsTreeView::TopicsTreeView()
{
}

void TopicsTreeView::on_cursor_changed()
{
	int cpt=0 ;
	std::vector<Gtk::TreePath> paths = get_selection()->get_selected_rows() ;
	std::vector<Gtk::TreeIter> iters ;
	for(guint i = 0; i<paths.size(); i++){
		Gtk::TreeIter iter = get_model()->get_iter(paths[i]) ;
		iters.insert(iters.end(), iter) ;
		cpt++ ;
	}
	m_signalSelection.emit(iters);
}


void TopicsTreeView::get_selected(Gtk::TreeIter& result)
{
	result = get_selection()->get_selected() ;
}


//******************************************************************************
//***************************** Topic MODEL COLUMNS*****************************
//******************************************************************************

void TopicsModelColumns::fill_row(Gtk::TreeModel::Row* row, int type, const Glib::ustring& id, const Glib::ustring& label)
{
	TopicsModelColumns m ;

	(*row)[m.a_type] = type ;
	(*row)[m.a_id] = id ;
	(*row)[m.a_label] = label ;
	if ( ( type==1 ) || ( id == TAG_TOPIC_NULL_LABEL ) )
		(*row)[m.a_displayWeight] = 600 ;
	else
		(*row)[m.a_displayWeight] = 400 ;
}

//******************************************************************************
//***************************** DETAILED DIALOG***** ***************************
//******************************************************************************

TopicsDetailDialog::TopicsDetailDialog(Topic* topic, Gtk::Window* win)
{
	if (!topic)
		return ;

	set_modal(true) ;

	if (win)
		set_transient_for(*win) ;

	dlgVbox = get_vbox() ;

	Glib::ustring title = _("Topic details: ") ;
	Icons::set_window_icon(this, ICO_TRANSCRIBER, 12) ;
	set_title(TRANSAG_DISPLAY_NAME) ;

	dlgVbox->pack_start(align_title, false, false, 3) ;
		align_title.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0) ;
		align_title.add(label_title) ;
			label_title.set_label(title + (topic->getLabel()).uppercase()) ;
			label_title.set_name("bold_label") ;
	dlgVbox->pack_start(scrolled, true, true, 3) ;
		scrolled.add(vbox) ;

	scrolled.set_name("topics_details_scrolled") ;
	scrolled.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC) ;

	button_close = add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE) ;

	show_all_children() ;

	addDetail(_("Category"), topic->getGroupLabel()) ;
	addDetail(_("Context"), topic->getContext()) ;

	std::vector<TopicDetails> all_details = topic->getDetails() ;
	std::vector<TopicDetails>::reverse_iterator it ;
	for (it=all_details.rbegin(); it!=all_details.rend(); it++) {
		addDetail((*it).getLabel(), (*it).getValue()) ;
	}

	set_size_request(500, 600) ;
}

TopicsDetailDialog::~TopicsDetailDialog()
{
	std::vector<Details*>::iterator it ;
	for (it = details.begin(); it!= details.end(); it++ ) {
		if (*it)
			delete(*it) ;
	}
}

void TopicsDetailDialog::addDetail(Glib::ustring title, Glib::ustring value)
{
	Details* d = new Details(title, value) ;
	details.insert(details.end(), d) ;
	vbox.pack_start(*d, false, false, 2) ;
	show_all_children() ;
}

//******************************************************************************
//***************************** TopicSDIALOG MEMBERS ***************************
//******************************************************************************


TopicsDialog::TopicsDialog(std::map<Glib::ustring, Topics*>& topics, Gtk::Window* parent)
: m_topics(topics)
{
	m_light_mode = false ;

	if (parent)
		set_transient_for(*parent) ;

	set_modal(true) ;
	set_size_request(400, 300) ;

	Icons::set_window_icon(this, ICO_TRANSCRIBER, 12) ;
	set_title(TRANSAG_DISPLAY_NAME) ;

	m_id_result = TAG_TOPIC_NULL_LABEL ;
	m_label_result = TAG_TOPIC_NULL_LABEL ;

	set_has_separator(true) ;

	dlgVbox = get_vbox() ;

	dlgVbox->pack_start(align_title, false, false, 3) ;
		align_title.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0) ;
		align_title.add(label_title) ;
			label_title.set_label(_("Annotation Topics")) ;
			label_title.set_name("bold_label") ;
	dlgVbox->pack_start(scrolledw, true, true, 3) ;
		scrolledw.add(m_view) ;
		scrolledw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC) ;

	button_details = Gtk::manage(new Gtk::Button(Gtk::Stock::INFO)) ;
	button_details->set_use_underline(true) ;

	button_details->signal_clicked().connect(sigc::mem_fun(this, &TopicsDialog::on_details_clicked)) ;
	Gtk::HButtonBox* box = (Gtk::HButtonBox*) get_action_area() ;
	box->pack_start(*button_details, false, false) ;

	button_ok = add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY) ;
	button_cancel = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
	button_ok->set_sensitive(false) ;

	//>prepare tree
	prepare_tree() ;

	//>fill model
	fill_model() ;

	button_details->set_sensitive(false) ;

	m_view.expand_all() ;

	show_all_children() ;
}

TopicsDialog::~TopicsDialog()
{
}


bool TopicsDialog::on_key_press_event(GdkEventKey* event)
{
	if (event->keyval==GDK_Return) {
		Gtk::TreeIter i ;
		m_view.get_selected(i) ;

		if (!m_refSortedModel->iter_is_valid(i))
			return true ;

		int type = (*i)[m_modelColumns.a_type] ;

		if (type==0)
			response(Gtk::RESPONSE_APPLY) ;

		return true ;
	}
	else
		return Gtk::Dialog::on_key_press_event(event) ;
}

void TopicsDialog::prepare_tree()
{
	m_refModel = Gtk::TreeStore::create(m_modelColumns) ;
	m_refSortedModel = Gtk::TreeModelSort::create(m_refModel) ;

	m_view.set_model(m_refSortedModel) ;
	m_view.set_window(this) ;

	m_refSortedModel->set_default_sort_func(sigc::mem_fun(*this, &TopicsDialog::sort_tree)) ;

	//> prepare view
	m_view.set_enable_search(true) ;
	m_view.set_search_column(m_modelColumns.a_label) ;
	m_view.set_headers_visible(false) ;
	m_view.set_rules_hint(false) ;
	m_view.get_selection()->set_mode(Gtk::SELECTION_BROWSE) ;

	//> Choose the displayed columns
	Gtk::TreeViewColumn* pColumn = Gtk::manage(new Gtk::TreeViewColumn("label")) ;
	Gtk::CellRendererText* cellRend_name = Gtk::manage(new Gtk::CellRendererText()) ;
	int number = m_view.append_column(*pColumn) ;
	if (pColumn) {
		pColumn->pack_start(*cellRend_name,false);
		pColumn->add_attribute(cellRend_name->property_text(), m_modelColumns.a_label) ;
		pColumn->add_attribute(cellRend_name->property_weight(), m_modelColumns.a_displayWeight) ;
	}
	m_view.signalSelection().connect(sigc::mem_fun(*this, &TopicsDialog::on_selection_changed)) ;
	m_view.signal_row_activated().connect(sigc::mem_fun(*this, &TopicsDialog::on_row_activated)) ;
}

int TopicsDialog::sort_tree(const Gtk::TreeModel::iterator& it1, const Gtk::TreeModel::iterator& it2)
{
	int res_return, res_compare ;
	Glib::ustring tmp1, tmp2 ;

	//get labels
	(*(it1)).get_value(2 , tmp1) ;
	(*(it2)).get_value(2, tmp2) ;

	//compare
	res_compare = tmp1.compare(tmp2) ;

	if (res_compare < 0)
		res_return = -1 ;
	else if (res_compare==0)
		res_return =  0 ;
	else
		res_return = 1 ;
	return res_return ;
}

void TopicsDialog::fill_model()
{
	std::map<Glib::ustring, Topics*>::iterator it ;

	Gtk::TreeRow tmp ;
	for (it=m_topics.begin(); it!=m_topics.end(); it++)
	{
		if (it->second) {
			Gtk::TreeIter iter = m_refModel->append() ;
			Gtk::TreeRow row = *iter ;
			m_modelColumns.fill_row(&row, 1, it->second->getId(), it->second->getLabel()) ;
			const std::map<Glib::ustring, Topic*>& all_topics = it->second->getAllTopics() ;
			std::map<Glib::ustring, Topic*>::const_iterator it2 ;
			for (it2=all_topics.begin(); it2!=all_topics.end(); it2++) {
				if (it2->second) {
					Gtk::TreeRow row_child = *(m_refModel->append(iter->children())) ;
					m_modelColumns.fill_row(&row_child, 0, it2->second->getId(), it2->second->getLabel()) ;
				}
			}
		}
	}
	Gtk::TreeIter iter = m_refModel->append() ;
	Gtk::TreeRow row = *iter ;
	m_modelColumns.fill_row(&row, 0, TAG_TOPIC_NULL_LABEL, _("Without theme")) ;
}

void TopicsDialog::on_selection_changed(std::vector<Gtk::TreeIter> paths)
{
	//> unique selection, so only one vector in paths
	Gtk::TreeIter iter = paths[0] ;

	if (paths.empty() || !m_refSortedModel->iter_is_valid(iter))
		return ;

	int type = (*iter)[m_modelColumns.a_type] ;
	if ( type==0 ) {
		get_selected(m_id_result, m_label_result) ;
		button_ok->set_sensitive(true) ;
		button_details->set_sensitive(true) ;
	}
	else {
		button_ok->set_sensitive(false) ;
		button_details->set_sensitive(false) ;
	}
}

void TopicsDialog::on_row_activated(const Gtk::TreeModel::Path& p, Gtk::TreeViewColumn* c)
{
	Gtk::TreeIter iter = m_refSortedModel->get_iter(p) ;
	int type = (*iter)[m_modelColumns.a_type] ;
	//> display details for topic
	if (type==0)
	{
		// for m_light_mode don't display details
		if (!m_light_mode)
		{
			Glib::ustring id = (*iter)[m_modelColumns.a_id] ;
			Topic* topic =  Topics::getTopicFromAll(id, m_topics) ;
			if (topic) {
				TopicsDetailDialog dialog(topic, this) ;
				dialog.run() ;
			}
		}
	}
	//> expand or close
	else if (type==1)
	{
		if (m_view.row_expanded(p))
			m_view.collapse_row(p) ;
		else
			m_view.expand_row(p, false) ;
	}
}

void TopicsDialog::on_details_clicked()
{
	Gtk::TreeIter i ;
	m_view.get_selected(i) ;
	if (m_refSortedModel->iter_is_valid(i)) {
		Gtk::TreeViewColumn* unused = NULL ;
		Gtk::TreePath p = m_refSortedModel->get_path(i);
		on_row_activated(p, unused) ;
	}
}

/*
 * -1: no selection
 * 0 : selection is a group !
 * 1 : selection is OK
 */
int TopicsDialog::get_selected(Glib::ustring& result_id, Glib::ustring& result_label)
{
	Gtk::TreeIter iter ;
	m_view.get_selected(iter) ;
	if (m_refSortedModel->iter_is_valid(iter)) {
		int type = (*iter)[m_modelColumns.a_type] ;
		if (type==1)
			return 0 ;
		else {
			result_id = (*iter)[m_modelColumns.a_id] ;
			result_label = (*iter)[m_modelColumns.a_label] ;
			return 1 ;
		}
	}
	else
		return -1 ;
}

bool TopicsDialog::get_chosen(Glib::ustring& result_id, Glib::ustring& result_label)
{
	result_id = m_id_result ;
	result_label = m_label_result ;

	return true ;
}


void TopicsDialog::set_light_mode()
{
	if (m_light_mode)
		return ;

	m_light_mode = true ;

	button_details->hide() ;
}

} // end namespace tag
