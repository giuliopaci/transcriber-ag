/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */

/**
* @file SectionRenderer.cpp
*
*  section annotation renderer implementation
*/

#include "Editors/AnnotationEditor/renderers/SectionRenderer.h"
#include "Editors/AnnotationEditor/dialogs/SectionPropertiesDialog.h"

using namespace std;
using namespace tag;

/** constructor */
SectionRenderer::SectionRenderer (AnnotationView* view)
: AnnotationRenderer(view, "section")
{
	m_tagFlags = IS_ACTIVE_TAG;
	configure();
	configureMenu() ;
}

/** configure renderer -> justification */
void SectionRenderer::configure()
{
	const Glib::RefPtr<Gtk::TextTag>& tag= AnnotationRenderer::configure();
	if ( tag == 0 ) TRACE_D << "NULL TAG !!" << endl;
	tag->property_justification().set_value(Gtk::JUSTIFY_CENTER);
}

void SectionRenderer::configureMenu()
{
	//  connect section menu related events
	if ( ! m_dataModel.isMainstreamType("section", "transcription_graph") )
		return ;

	string subtypes = m_dataModel.conventions().getConfiguration("section,subtypes") ;
	SectionMenu* smenu =  new SectionMenu(subtypes, m_dataModel.conventions().getLayout("Menu"));
	contextMenu = smenu ;
	smenu->signalSetSection().connect(sigc::mem_fun (m_view, &AnnotationView::setSection));
	smenu->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::editAnnotationProperties), "section"));
	smenu->signalDeleteAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::deleteAnnotation), "section"));

	// section menu not editable
	contextMenu_lock =  new AnnotationMenu("section", true, false);
	contextMenu_lock->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::editAnnotationProperties), "section"));
}


AnnotationPropertiesDialog* SectionRenderer::newPropertiesDialog(const string& id)
{
	SectionPropertiesDialog* dlg = new SectionPropertiesDialog(m_view->getTopWindow(), m_dataModel, id, m_view->isEditable());
	return dlg;
}

/**
*  render section annotation at current text cursor position
*  @param id annotation id
*/
void SectionRenderer::render(const string& id, bool r2l, bool with_contents)
{
	string label;
	string tagname;

	setLabelAndTagname(id, label, tagname);

	getBuffer()->insertAnchoredLabel("section", id, label, tagname, 0, r2l, "");
}

/*
 * delete background annotation  from text buffer
 */
void SectionRenderer::deleteElement(const string & id)
{
	getBuffer()->deleteAnchoredLabel(id, m_dataModel.existsElement(id));
}
/*
 * update section annotation in text buffer
 */
void SectionRenderer::updateElement(const string & id, bool r2l)
{
//	guint offset = getBuffer()->getAnchoredIter(id).get_offset();
//	deleteElement(id);
//	getBuffer()->setCursor(offset);
//	render(id, r2l);
	string tagname;
	string label;
	setLabelAndTagname(id, label, tagname);
	getBuffer()->setAnchoredLabel(id, label, tagname, r2l);
}

void SectionRenderer::setLabelAndTagname(const string & id, string& label, string& tagname)
{
	tagname = "section";

	const string& section_type = m_dataModel.getElementProperty(id, "type");
	const string& desc = m_dataModel.getElementProperty(id, "desc");
	Glib::ustring topic = m_dataModel.getElementProperty(id, "topic");
	Glib::ustring topic_label = "" ;
	if (!topic.empty())
		topic_label = Topics::getTopicLabel(topic, m_dataModel.conventions().getTopics()) ;

	label = "   ";
	label += section_type;
	if ( ! desc.empty() ) {
		label += string(" - ") ;
		label += desc;
	}
	if ( !topic.empty() ) {
		label += string("   {") ;
		label += topic_label ;
		label += string("}") ;
	}
	label += "   ";
}
