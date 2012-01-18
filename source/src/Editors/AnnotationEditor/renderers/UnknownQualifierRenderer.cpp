/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */

/**
 * @file UnknownQualifierRenderer.cpp
 *
 *  turn annotation renderer implementation
 */

#include "Editors/AnnotationEditor/renderers/UnknownQualifierRenderer.h"
#include "Editors/AnnotationEditor/dialogs/QualifierPropertiesDialog.h"

using namespace tag;

/** constructor */
UnknownQualifierRenderer::UnknownQualifierRenderer (AnnotationView* view)
: AnnotationRenderer(view, "qualifier_unknown")
{
	m_tagFlags = IS_ACTIVE_TAG | IS_TEXT_EMBEDDED | CAN_BE_HIDDEN_TAG ;
	configure();
	configureMenu() ;
}

/** configure renderer -> justification */
void
UnknownQualifierRenderer::configure ()
{
	AnnotationRenderer::configure ("qualifier_unknown", "qualifier_unknown", true );
	AnnotationRenderer::configure ("qualifier_unknown", "qualifier_unknown", true, true);
}

void UnknownQualifierRenderer::configureMenu()
{
	bool display_error = false ;
	if ( m_view->getViewTrack() == -1)
		display_error = true ;
	contextMenu =  new QualifiersMenu("transcription_graph", "qualifier_unknown", m_dataModel.conventions().getConfiguration(), m_dataModel.conventions().getLayout("Menu"), display_error);
	contextMenu->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (*m_view, &AnnotationView::editAnnotationProperties), "qualifier_unknown"));
	contextMenu->signalDeleteAnnotation().connect(sigc::bind<string>(sigc::mem_fun (*m_view, &AnnotationView::deleteAnnotation), "qualifier_unknown"));
}


AnnotationPropertiesDialog* UnknownQualifierRenderer::newPropertiesDialog(const string& id)
{
	QualifierPropertiesDialog* dlg = new QualifierPropertiesDialog(m_view->getTopWindow(), m_dataModel, id, m_view->isEditable());

	dlg->configureMenuLabels(((QualifiersMenu*)contextMenu)->getLabels());

	return dlg;
}

/**
 *  render section annotation at current text cursor position
 *  @param id annotation id
 */
void UnknownQualifierRenderer::render (const string & id, bool r2l, bool with_contents)
{
	const string& type = m_dataModel.getElementType(id) ;
	const string& desc = m_dataModel.getElementProperty(id, "desc");
	Glib::ustring label;

	bool instantaneous = m_dataModel.isInstantaneous(id);

	label = type ;
	if (!desc.empty())
		label += "=" + desc ;

	formatLabelStart(label, instantaneous) ;

	const string& start_id = m_dataModel.getMainstreamStartElement(id);
	//force empty type to enable unknown tag
	getBuffer()->insertTaggedElement(id, start_id, "", label, true, r2l);
}


/**
 *  render section annotation at current text cursor position
 *  @param id annotation id
 */
void UnknownQualifierRenderer::render_end(const string & id, bool r2l)
{
	// will not insert event if instantaneous && end
	if ( m_dataModel.isInstantaneous(id) )
		return;

	const string& type = m_dataModel.getElementType(id) ;
	const string& desc = m_dataModel.getElementProperty(id, "desc");

	Glib::ustring label = type ;
	if (!desc.empty())
		label += "=" + desc ;

	formatLabelEnd(label) ;

	const string& next_id = m_dataModel.getMainstreamNextElement(id);
	//force empty type to enable unknown tag
	getBuffer()->insertTaggedElement(id, next_id, "", label, false, r2l);
}


/*
 * delete background annotation  from text buffer
 */
void UnknownQualifierRenderer::deleteElement(const string & id)
{
	m_view->getBuffer()->deleteTaggedElement(id);
}

/*
 * update annotation in text buffer
 */
void UnknownQualifierRenderer::updateElement(const string & id, bool r2l)
{
	deleteElement(id);
	render(id, r2l) ;
	render_end(id, r2l) ;
}


void UnknownQualifierRenderer::formatLabelStart(Glib::ustring& label, bool is_instantaneous)
{
	//TODO get all in configuration

	label.insert(0, " [");
	if (!is_instantaneous)
		label.append("-") ;
	label.append("] ") ;
}

void UnknownQualifierRenderer::formatLabelEnd(Glib::ustring& label)
{
	//TODO get all in configuration

	label.insert(0, " [-");
	label.append("] ") ;
}

