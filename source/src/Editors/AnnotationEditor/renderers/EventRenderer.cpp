/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */

/**
 * @file EventRenderer.cpp
 *
 *  turn annotation renderer implementation
 */

#include "Editors/AnnotationEditor/renderers/EventRenderer.h"
#include "Editors/AnnotationEditor/dialogs/QualifierPropertiesDialog.h"

using namespace tag;

/** constructor */
EventRenderer::EventRenderer (AnnotationView* view)
: AnnotationRenderer(view, "qualifier_event")
{
	m_tagFlags = IS_ACTIVE_TAG | IS_TEXT_EMBEDDED | CAN_BE_HIDDEN_TAG ;
	configure();
	configureMenu() ;
	initLayoutRules("qualifier_event");
}

/** configure renderer -> justification */
void
EventRenderer::configure ()
{
	AnnotationRenderer::configure ("qualifier_event", "qualifier_event", true);
	AnnotationRenderer::configure ("qualifier_event", "qualifier_event", true, true);
}

void EventRenderer::configureMenu()
{
	// connect event menu related events
	if ( m_dataModel.conventions().getConfiguration("transcription_graph,qualifier_event").empty() )
		return ;

	bool display_error = false ;
	if ( m_view->getViewTrack() == -1 )
		display_error = true ;

	QualifiersMenu* qmenu =  new QualifiersMenu("transcription_graph", "qualifier_event", m_dataModel.conventions().getConfiguration(), m_dataModel.conventions().getLayout("Menu"), display_error) ;
	contextMenu = qmenu ;
	qmenu->signalSetQualifier().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::setQualifier), "qualifier_event")) ;
	qmenu->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::editAnnotationProperties), "qualifier_event")) ;
	qmenu->signalDeleteAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::deleteAnnotation), "qualifier_event")) ;
	qmenu->signalUnanchorAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::removeTimestamp), "qualifier_event")) ;

	//> qualifier events properties not editable
	contextMenu_lock =  new AnnotationMenu("qualifier_event", true, false);
	contextMenu_lock->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::editAnnotationProperties), "qualifier_event")) ;
}

AnnotationMenu* EventRenderer::duplicateMenu()
{
	if ( m_dataModel.conventions().getConfiguration("transcription_graph,qualifier_event").empty() )
		return NULL ;

	QualifiersMenu* qmenu =  new QualifiersMenu("transcription_graph", "qualifier_event", m_dataModel.conventions().getConfiguration(), m_dataModel.conventions().getLayout("Menu"), false) ;
	return qmenu ;
}


AnnotationPropertiesDialog* EventRenderer::newPropertiesDialog(const string& id)
{
	QualifierPropertiesDialog* dlg = new QualifierPropertiesDialog(m_view->getTopWindow(), m_dataModel, id, m_view->isEditable());
	dlg->configureMenuLabels(((QualifiersMenu*)contextMenu)->getLabels());
	return dlg;
}

/**
 *  render section annotation at current text cursor position
 *  @param id annotation id
 */
void EventRenderer::render (const string & id, bool r2l, bool with_contents)
{
	Glib::ustring label;
	bool instantaneous = m_dataModel.isInstantaneous(id);
	layoutItem(id, m_dataModel.getElementType(id), label, !instantaneous);
	formatLabelStart(label, instantaneous) ;

	const string& start_id = m_dataModel.getMainstreamStartElement(id);
	const string& type = m_dataModel.getElementType(id);
	getBuffer()->insertTaggedElement(id, start_id, type, label, true, r2l);
}


/**
 *  render section annotation at current text cursor position
 *  @param id annotation id
 */
void EventRenderer::render_end(const string & id, bool r2l)
{
	if ( m_dataModel.isInstantaneous(id) )
		return ;

	Glib::ustring label;
	layoutItem(id, m_dataModel.getElementType(id), label, false);
	formatLabelEnd(label) ;

//	const string& next_id = m_dataModel.getMainstreamNextElement(id);
//	const string& type = m_dataModel.getElementType(id);
//	getBuffer()->insertTaggedElement(id, next_id, type, label, false, r2l);

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
 * delete annotation  from text buffer
 */
void EventRenderer::deleteElement(const string & id)
{
	getBuffer()->deleteTaggedElement(id);
}


/*
 * update annotation in text buffer
 */
void EventRenderer::updateElement(const string & id, bool r2l)
{
	deleteElement(id);
	render(id, r2l) ;
	render_end(id, r2l) ;
}


void EventRenderer::formatLabelStart(Glib::ustring& label, bool is_instantaneous)
{
	//TODO get all in configuration
	label.insert(0, " [");
	if (!is_instantaneous)
		label.append("-") ;
	label.append("] ") ;
}

void EventRenderer::formatLabelEnd(Glib::ustring& label)
{
	//TODO get all in configuration

	label.insert(0, " [-");
	label.append("] ") ;
}


