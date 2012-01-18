/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */

/**
 * @file EntityRenderer.cpp
 *
 *  turn annotation renderer implementation
 */

#include "Editors/AnnotationEditor/renderers/EntityRenderer.h"
#include "Editors/AnnotationEditor/dialogs/QualifierPropertiesDialog.h"

using namespace tag;

/** constructor */
EntityRenderer::EntityRenderer (AnnotationView* view)
: AnnotationRenderer(view, "qualifier_entity")
{
	m_tagFlags = IS_ACTIVE_TAG | IS_TEXT_EMBEDDED | CAN_BE_HIDDEN_TAG ;
	configure();
	configureMenu() ;
	initLayoutRules("qualifier_entity");
}

/** configure renderer -> justification */
void
EntityRenderer::configure ()
{
	//> DEFAULT ONES
	bool use_bg = (m_view->getConfigurationOption("AnnotationLayout,qualifiers,entity_bg") == "true" );

	AnnotationRenderer::configure ("qualifier_entity", "", true, false, "", "", "", "", use_bg);
	AnnotationRenderer::configure ("qualifier_entity", "", true, true, "", "", "", "", use_bg);

	//> CONVENTION ONES

	//> get all entities main types from convention
	const std::vector<string>& entities = m_dataModel.conventions().getQualifierTypes("entity") ;

	//> even for convention entities, style and weight are same than
	// default ones
	Glib::ustring style = getLook("qualifier_entity", "style") ;
	Glib::ustring weight = getLook("qualifier_entity", "weight") ;
	Glib::ustring color_fg = ""  ;
	Glib::ustring color_bg = "" ;
	Glib::ustring name = "" ;

	//> for all entities, check in layout
	std::vector<string>::const_iterator it ;
	for (it=entities.begin(); it!=entities.end(); it++)
	{
		string item = "qualifier_entity,"+*it;
		color_fg = m_dataModel.conventions().getLayoutItem("Colors", item, "fg") ;
		color_bg = m_dataModel.conventions().getLayoutItem("Colors", item, "bg") ;
		name = "qualifier_entity_" + *it  ;
		AnnotationRenderer::configure (name, "", true, false, color_fg, color_bg , style, weight, use_bg);
		AnnotationRenderer::configure (name, "", true, true, color_fg, color_bg , style, weight, use_bg);
	}
}

void EntityRenderer::configureMenu()
{
	if ( m_dataModel.conventions().getConfiguration("transcription_graph,qualifier_entity").empty() )
		return ;

	bool display_error = false ;
	if (m_view->getViewTrack() == -1)
		display_error = true ;

	QualifiersMenu* qmenu =  new QualifiersMenu("transcription_graph", "qualifier_entity", m_dataModel.conventions().getConfiguration(), m_dataModel.conventions().getLayout("Menu"), display_error);
	contextMenu = qmenu ;
	qmenu->signalSetQualifier().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::setQualifier), "qualifier_entity"));
	qmenu->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::editAnnotationProperties), "qualifier_entity"));
	qmenu->signalDeleteAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::deleteAnnotation), "qualifier_entity"));
	qmenu->signalUnanchorAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::removeTimestamp), "qualifier_entity")) ;

	// qualifier entities properties not editable
	contextMenu_lock =  new AnnotationMenu("qualifier_entity", true, false);
	contextMenu_lock->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::editAnnotationProperties), "qualifier_entity"));
}

AnnotationMenu* EntityRenderer::duplicateMenu()
{
	if ( m_dataModel.conventions().getConfiguration("transcription_graph,qualifier_entity").empty() )
		return NULL ;

	QualifiersMenu* qmenu =  new QualifiersMenu("transcription_graph", "qualifier_entity", m_dataModel.conventions().getConfiguration(), m_dataModel.conventions().getLayout("Menu"), false);
	return qmenu ;
}

AnnotationPropertiesDialog* EntityRenderer::newPropertiesDialog(const string& id)
{
	QualifierPropertiesDialog* dlg = new QualifierPropertiesDialog(m_view->getTopWindow(), m_dataModel, id, m_view->isEditable());
	dlg->configureMenuLabels(((QualifiersMenu*)contextMenu)->getLabels());
	return dlg;
}

/**
 *  render section annotation at current text cursor position
 *  @param id annotation id
 */
void EntityRenderer::render (const string & id, bool r2l, bool with_contents)
{
	Glib::ustring label;
	layoutItem(id, m_dataModel.getElementType(id), label, true);
	formatLabelStart(label, false) ;

	const string& start_id = m_dataModel.getMainstreamStartElement(id);
	const string& type = m_dataModel.getElementType(id);
	getBuffer()->insertTaggedElement(id, start_id, type, label, true, r2l) ;
}


/**
 *  render section annotation at current text cursor position
 *  @param id annotation id
 */
void EntityRenderer::render_end(const string & id, bool r2l)
{
	Glib::ustring label;
	layoutItem(id, m_dataModel.getElementType(id), label, false);
	formatLabelEnd(label) ;

	string next_id = m_dataModel.getMainstreamNextElement(id);
	const string& type = m_dataModel.getElementType(id);
	int order = m_dataModel.getOrder(id) ;

	if (order==0)
	{
		string parentNext ;
		const string& parent = m_dataModel.getParentElement(id, "turn", false, false) ;

		if  (!next_id.empty())
			parentNext = m_dataModel.getParentElement(next_id, "turn", false, false) ;

		if ( parent!=parentNext || next_id.empty())
		{
			const string& over = m_dataModel.getElementStartingAtAnchor(m_dataModel.getAnchor(parent,true), "turn", 1) ;
			if (!over.empty())
				next_id = over ;
		}
	}
	getBuffer()->insertTaggedElement(id, next_id, type, label, false, r2l);
}

/*
 * update annotation in text buffer
 */
void EntityRenderer::updateElement(const string & id, bool r2l)
{
	deleteElement(id);
	render(id, r2l) ;
	render_end(id, r2l) ;
}

/*
 * delete qualifier annotation  from text buffer
 */
void EntityRenderer::deleteElement(const string & id)
{
	getBuffer()->deleteTaggedElement(id);
}


void EntityRenderer::formatLabelStart(Glib::ustring& label, bool is_instantaneous)
{
	label.insert(0, " [");
	if (!is_instantaneous)
		label.append("-") ;
	label.append("] ") ;
}

void EntityRenderer::formatLabelEnd(Glib::ustring& label)
{
	label.insert(0, " [-");
	label.append("] ") ;
}

bool EntityRenderer::checkInstantaneous(const string& id)
{
	const string& start_id = m_dataModel.getMainstreamStartElement(id);
	bool instantaneous = m_dataModel.isInstantaneous(id);
	const string& type = m_dataModel.getElementProperty(id, "desc", "") ;

	/* Temporary disable: can we allow spontaneous qualifier ? */
	// if ! instantaneous but no text between start & stop -> set instantaneous to true
	if ( !instantaneous && type.compare("comment")==0)
	{
		const string& end_id = m_dataModel.getMainstreamEndElement(id);

		//-- Check if the qualifier doesn't span over more than 1 mainstream element
		if ( start_id == end_id )
		{
			const string& text =  m_dataModel.getElementProperty(start_id, "value");
			if ( text.empty() )
				instantaneous = true;
		}
	}
	return instantaneous ;
}

const string& EntityRenderer::getTagName(const string& id, const string& type)
{
	m_tmpname= m_name + "_" + type ;
	return m_tmpname;
}


// TODO a deplacer dans le renderer d'entites !!!
void EntityRenderer::set_use_bg(bool use_bg)
{
	if (!m_view)
		return ;

	string color ;

	//> if not bg needed, get view bg color for apply to tag
	Glib::RefPtr<Gtk::Style> style = m_view->get_style() ;
	if (!use_bg && m_view)
		color = m_view->getBaseViewColor() ;
	else
		color = "#ffffff" ;


	//> for all m_entities, check in layout
	const vector<string>& entities = m_dataModel.conventions().getQualifierTypes("entity");
	std::vector<string>::const_iterator it ;
	for (it=entities.begin(); it!=entities.end(); it++)
	{
		bool exist_in_layout = true ;
		if (exist_in_layout)
		{
			//TODO
			if (use_bg) {
				string key = "qualifier_entity,"+*it;
				color = m_dataModel.conventions().getLayoutItem("Colors", key, "bg") ;
			}
			string name = "qualifier_entity_" + *it  ;
			const Glib::RefPtr<Gtk::TextTag>& tag = getBuffer()->get_tag_table()->lookup(name) ;
			if (tag && !color.empty())
				tag->property_background().set_value(color);
			string name_end = "qualifier_entity_" + *it + "_end" ;
			const Glib::RefPtr<Gtk::TextTag>& tag2 = getBuffer()->get_tag_table()->lookup(name_end) ;
			if (tag2 && !color.empty())
				tag2->property_background().set_value(color);
		}
	}

	// DO IT FOR DEFAULT ONE
	if (use_bg)
		color = getBuffer()->getLabelLook("entity", "bg") ;
	string name = "qualifier_entity" ;
	const Glib::RefPtr<Gtk::TextTag>& tag3 = getBuffer()->get_tag_table()->lookup(name) ;
	if (tag3 && !color.empty())
		tag3->property_background().set_value(color);
	string name_end = "qualifier_entity_end" ;
	const Glib::RefPtr<Gtk::TextTag>& tag4 = getBuffer()->get_tag_table()->lookup(name_end) ;
	if (tag4 && !color.empty())
		tag4->property_background().set_value(color);
}

