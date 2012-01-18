/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */

/**
 * @file SegmentRenderer.cpp
 *
 *  segment annotation renderer implementation
 */

#include "Editors/AnnotationEditor/renderers/SegmentRenderer.h"
#include "Editors/AnnotationEditor/dialogs/AnchoredElementDialog.h"

namespace tag {

SegmentRenderer::SegmentRenderer (AnnotationView* view)
: AnnotationRenderer(view, "segment")
{
	m_label= " o ";
	m_nolabel= "";
	m_showLabel = true;
	m_tagFlags = IS_ACTIVE_TAG;
	configure();
	configureMenu() ;
}

void
SegmentRenderer::configure ()
{
	AnnotationRenderer::configure();
	m_label = m_view->getConfigurationOption("AnnotationLayout,segment,label");
	m_showLabel = (m_view->getConfigurationOption("AnnotationLayout,segment,label,show") == "true");
}

void SegmentRenderer::configureMenu()
{
	//  connect segment menu related events
	if ( ! m_dataModel.isMainstreamType("segment", "transcription_graph") )
		return ;

	contextMenu =  new AnnotationMenu("segment");
	contextMenu->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::editAnnotationProperties), "segment"));
	contextMenu->signalDeleteAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::deleteAnnotation), "segment"));

	// segment menu properties not editable
	contextMenu_lock =  new AnnotationMenu("segment", true, false);
	contextMenu_lock->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::editAnnotationProperties), "segment"));
}

AnnotationPropertiesDialog* SegmentRenderer::newPropertiesDialog(const string& id)
{
	AnchoredElementDialog* dlg = new AnchoredElementDialog(m_view->getTopWindow(), m_dataModel, id, m_view->isEditable());
	return dlg;
}

void
SegmentRenderer::render (const string & id, bool r2l, bool with_contents)
{
	if (!m_label.empty ())
	{
		int notrack = 0;
		int order = m_dataModel.getOrder (id);
		//                      TRACE_D << "insertSegment id=" <<  segment.getId() << " order = " << order << " notrack =" << notrack << endl;
		if (order > 0)
			notrack = /* order */ 3;
		else if ( m_view->getViewTrack() == -1 )
			notrack = m_dataModel.getElementSignalTrack (id);

		const string& align = m_view->getDataModel().getAlignmentId(id) ;
//		if ( m_view->getDataModel().getElementType(align) != m_view->getDataModel().mainstreamBaseType("transcription_graph") )
//			align = "" ;

		getBuffer()->insertAnchoredLabel ("segment", id, m_label, "segment", notrack, r2l, align);
	}
}


/*
 * delete segment annotation  from text buffer
 */
void SegmentRenderer::deleteElement(const string & id)
{
//	getBuffer()->deleteAnchoredLabel(id, m_dataModel.existsElement(id));
	getBuffer()->deleteAnchoredLabel(id, false);
}


/*
 * update turn annotation in text buffer
 */
void SegmentRenderer::updateElement(const string & id, bool r2l)
{
	guint offset = getBuffer()->getAnchoredIter(id).get_offset();
	deleteElement(id);
	getBuffer()->setCursor(offset);
	render(id, r2l);
}

}
