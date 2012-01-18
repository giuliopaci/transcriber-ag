/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */

/**
 * @file UnitRenderer.cpp
 *
 *  segment annotation renderer implementation
 */

#include "Editors/AnnotationEditor/renderers/UnitRenderer.h"
#include "Editors/AnnotationEditor/dialogs/AnchoredElementDialog.h"

#include "Common/util/FormatTime.h"
#include "Common/ColorsCfg.h"


namespace tag {

static string single_blank("");

UnitRenderer::UnitRenderer (AnnotationView* view)
: AnnotationRenderer(view, "unit")
{
	m_simpleMenu = NULL ;
	m_tagFlags = IS_TEXT_EMBEDDED | CAN_BE_HIDDEN_TAG | IS_ACTIVE_TAG /*| DO_UNDERLINE_TAG*/  ;

	//TODO conventions
	m_threshold = 0.7 ;

	configure();
	configureMenu() ;
	setUseTooltip(true) ;

	// if has subtype, set corresponding feature

	std::vector<string> subtypes ;
	m_dataModel.conventions().mainstreamHasSubtypes("unit", "transcription_graph", subtypes);
	if ( !subtypes.empty() )
		m_textType = subtypes.front();
	else
		m_textType = "" ;

	//  tmp: we use qualifier event layout rules -> a specific "unit_event" layout could be created and used instead
	initLayoutRules("qualifier_event");

}

UnitRenderer::~UnitRenderer()
{
	if (m_simpleMenu)
		delete(m_simpleMenu) ;
}

// TODO -> SET THRESHOLD

void UnitRenderer::configure ()
{
	Glib::ustring style = getLook(m_name, "style") ;
	Glib::ustring weight = getLook(m_name, "weight") ;
	Glib::ustring color_fg = getLook(m_name, "fg") ;
	std::string color_bg = getLook(m_name, "bg") ;

	AnnotationRenderer::configure(m_name, m_name, true, false, color_fg, color_bg, style, weight, true) ;

	m_altname = m_name + "_unknown" ;
	style = getLook(m_altname, "style") ;
	weight = getLook(m_altname, "weight") ;
	color_fg = getLook(m_altname, "fg") ;
	color_bg = getLook(m_altname, "bg") ;
	AnnotationRenderer::configure(m_altname, m_altname, true, false, color_fg, color_bg, style, weight, true) ;
}

void UnitRenderer::configureMenu()
{
	if ( !m_dataModel.isMainstreamType("unit", "transcription_graph") )
		return ;

	bool display_error = false ;
	if ( m_view->getViewTrack() == -1 )
		display_error = true ;

	// menu for predefined unit
	UnitMenu* umenu =  new UnitMenu("transcription_graph", "unit", m_dataModel.conventions().getConfiguration(), m_dataModel.conventions().getLayout("Menu"), display_error);
	contextMenu = umenu ;
	umenu->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::editAnnotationProperties), "unit"));
	umenu->signalDeleteAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::deleteAnnotation), "unit"));
	umenu->signalSetUnit().connect(sigc::mem_fun (m_view, &AnnotationView::setForegroundEvent)) ;
	umenu->signalUnanchorAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::removeTimestamp), "unit")) ;


	// menu for textual unit
	m_simpleMenu =  new UnitMenu("transcription_graph", "unit");
	m_simpleMenu->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::editAnnotationProperties), "unit"));
	m_simpleMenu->signalDeleteAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::deleteAnnotation), "unit"));

	// menu properties not editable
	contextMenu_lock =  new AnnotationMenu("unit", true, false);
	contextMenu_lock->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (m_view, &AnnotationView::editAnnotationProperties), "unit"));
}

AnnotationPropertiesDialog* UnitRenderer::newPropertiesDialog(const string& id)
{
	AnchoredElementDialog* dlg = new AnchoredElementDialog(m_view->getTopWindow(), m_dataModel, id, m_view->isEditable());
	return dlg;
}

void UnitRenderer::render(const string & id, bool r2l, bool with_contents)
{
//	if ( m_dataModel.mainstreamBaseElementIsValid(id, "transcription_graph") )
//	{
//		if ( m_dataModel.mainstreamBaseElementHasText(id) )
	setInvisibleTag(id, m_isFirstChild) ;

	const string& sub = m_dataModel.getElementProperty(id, "subtype");
	if ( sub == m_textType )
		renderUnitText(id, r2l, with_contents) ;
	else if (!sub.empty() )
		renderUnitEvent(id, r2l) ;
	else
		renderUnitUnknown(id, r2l) ;
}

void UnitRenderer::renderUnitText(const string & id, bool r2l, bool with_contents)
{
	const string& text = m_dataModel.getElementProperty (id, "value");

	bool high_confidence = false;
	m_emptyProperties = text.empty();

	// -- with text
	if ( ! m_emptyProperties && m_view->getWithConfidence() )
	{
		const string& score = m_dataModel.getElementProperty(id, "score") ;
		if ( !score.empty() )
		{
			istringstream is(score);
			float f;
			is >> f;
			if ( f > m_threshold )
				high_confidence = true;
		}
		else
		{
			const string& confidence = m_dataModel.getElementProperty(id, "confidence") ;
			high_confidence = (confidence == "high") ;
		}
	}

	string buf = "" ;
	bool anchored ;

	/* <!> CAUTION : don't change buf without changing the canDeleteModelSelection method
	 * 	of AnnotationView because it is related to buf size, and markNeedsLeftGravity method
	 *  of AnnotationBuffer because it is related to the text iter position we search an anchor at.
	 * IMPACT:
	 * - canDeleteModelSelection
	 * - markNeedsLeftGravity
	 * TODO get the size from the renderer
	 */
	if ( configureRendererVisibility(id, anchored) )
		buf = " " ;

	getBuffer()->insertAnchoredTaggedText(getName(), id, (with_contents ?  text : single_blank), high_confidence, anchored, true, buf) ;
}

//
// event-type units are rendered like qualifier events, but using curly braces instead of square braces
void UnitRenderer::renderUnitEvent(const string& id, bool r2l)
{
	Glib::ustring label;
	bool instantaneous = m_dataModel.isInstantaneous(id);
	layoutItem(id, m_dataModel.getElementProperty(id, "value"), label, true);
	formatLabelStart(label, true) ;

	// -- 2. Render
	bool showAnchored = /*!m_isFirstChild && */ m_dataModel.isAnchoredElement(id, 0) ;
	getBuffer()->insertAnchoredTaggedText(getName(), id, "", false, showAnchored, false, label, m_dataModel.getNextElementId(id)) ;
}

void UnitRenderer::renderUnitUnknown(const string& id, bool r2l)
{
	Glib::ustring label ;

	//> -- 1. Configure layout
	const string& event_type = m_dataModel.getElementProperty(id, "value") ;
	const string& desc = m_dataModel.getElementProperty(id, "desc");

	label = event_type ;
	if (!desc.empty())
		label += "=" + desc ;

	formatLabelStart(label, true) ;

	// -- 2. Render
	bool isAnchored = m_dataModel.isAnchoredElement(id, 0) ;
	getBuffer()->insertAnchoredTaggedText(m_altname, id, "", false, isAnchored, false, label) ;
}


bool UnitRenderer::configureRendererVisibility(const string& id, bool& anchored)
{
	//> Start anchor isn't time anchored ? don't display element
	if ( m_dataModel.getElementOffset(id, true) == -1 )
	{
		anchored = false ;
		return false ;
	}
	else
		anchored = true ;

	//> Set invisible by caller ? exit
	if ( getInvisibleTag(id) )
	{
		setInvisibleTag(id, false) ;
		return false ;
	}

	//> Some qualifiers attached to start ? exit
	if ( m_dataModel.hasQualifiers(id) || m_dataModel.isFirstChild(id, ""))
		return false ;

	return true ;
}


/**
 * delete segment text from text buffer
 * @param id annotation id
 *
 * @note
 */
void UnitRenderer::deleteElement(const string& id)
{
	getBuffer()->deleteAnchoredLabel(id, false) ;
	setInvisibleTag(id, false) ;
}

/**
 * update segment text in text buffer
 * @param id annotation id
 *
 * @note
 */
void UnitRenderer::updateElement(const string& p_id, bool r2l)
{
	//only update time status
//	bool anchored = m_dataModel.isAnchoredElement(id, 0) ;
//	getBuffer()->switchTimestampTag(id, anchored) ;
	string id = p_id;

	Anchor* an = getBuffer()->anchors().getAnchor(id) ;

	if (an)
	{
		// keep insert position for future restore
		getBuffer()->backupInsertPosition(2,true) ;

		// keep anchor position
		getBuffer()->setCursor(an->getIter()) ;
		Glib::RefPtr<Gtk::TextMark> mark = getBuffer()->create_mark("tmpUpdEmt", an->getIter(), true);

		// delete element
		deleteElement(id) ;

		// replace position at anchor iter
		Gtk::TextIter iter = getBuffer()->get_iter_at_mark(mark) ;
		getBuffer()->setCursor(iter, false) ;
		getBuffer()->delete_mark(mark) ;

		// render element
		if ( m_dataModel.mainstreamBaseElementIsValid(id, "transcription_graph")  )
		{
			if ( m_dataModel.mainstreamBaseElementHasText(id) ) {
				renderUnitText(id, r2l, false) ;
			}
			else
			{
				renderUnitEvent(id, r2l) ;
				// HACK -> get rid of unwanted non-editable tag at next anchor start
//				Anchor* an = getBuffer()->anchors().getAnchor(id) ;
//				if ( an != NULL ) {
//					an = getBuffer()->anchors().getNextAnchor(*an);
//					if ( an != NULL && an->isTextType() ) {
//						Gtk::TextIter it = an->getMark()->get_iter();
//						if ( ! it.editable() ) {
//							if ( getBuffer()->isLabelOnly(it) ) {
//								getBuffer()->setEditable(it, true);
//							}
//						}
//					}
//				}
			}
		}
		else
			renderUnitUnknown(id, r2l) ;

		// restore cursor at initial insert position
		getBuffer()->restoreInsertPosition(2,true) ;
	}
}

void UnitRenderer::formatLabelStart(Glib::ustring& label, bool instantaneous)
{
	// always treated as "instantaneous"
	label.insert(0, " {");
	label.append("} ") ;
}

Gtk::Box* UnitRenderer::getTooltipBox(const string& id, bool is_start)
{
	bool hasText = m_dataModel.mainstreamBaseElementHasText(id) ;
	string value =  m_dataModel.getElementProperty (id, "value", "") ;
	const string& submain =  m_dataModel.getElementProperty (id, "subtype", "") ;
	float start = m_dataModel.getElementOffset(id, true);
	float stop = m_dataModel.getElementOffset(id, false);

	if (start==-1 && value.empty() || start==-1 && stop==-1)
		return NULL ;

	Gtk::VBox* box = Gtk::manage(new Gtk::VBox()) ;

	Gtk::Label* label_time = Gtk::manage(new Gtk::Label()) ;
	Glib::ustring text_time ;

	// for textual unit, display only start anchor if it is anchored
	if ( start!=-1 )
		text_time.append(FormatTime(start, true)) ;
	// when not anchored, for event, display anyway
	else if (!hasText)
		text_time.append("*") ;

	// for event, display end anchor too
	if (!hasText)
	{
		text_time.append(" - ") ;

		if (stop!=-1 )
			text_time.append(FormatTime(stop, true)) ;
		else
			text_time.append("*") ;
	}

	label_time->set_markup("<b>" + text_time + "</b>") ;
	box->pack_start(*label_time , false, false) ;

	// -- Textual unit: display text
	const string& display = value ;
	// -- Event unit: display type
	if ( !hasText )
	{
		value = _("foreground event") + string(" ") + value ;
		const string& desc =  m_dataModel.getElementProperty(id, "desc", "") ;
		if (!desc.empty())
			value = value + "\n" + desc ;
	}

	Gtk::Label* label = Gtk::manage(new Gtk::Label(value)) ;
	box->pack_start(*label, false,false) ;

	box->show_all_children() ;
	return box ;
}

const string& UnitRenderer::getTagName(const string& id, const string& type)
{
	if ( !m_dataModel.mainstreamBaseElementIsValid(id, "transcription_graph") )
		return m_altname ;
	else
		return m_name ;
}

AnnotationMenu* UnitRenderer::getContextualMenu(const string& id, bool edition)
{
	if ( !edition )
		return contextMenu_lock ;

	if ( id.empty() || !m_dataModel.mainstreamBaseElementHasText(id) )
		return contextMenu ;
	else
		return m_simpleMenu ;
}

AnnotationMenu* UnitRenderer::duplicateMenu()
{
	if ( !m_dataModel.isMainstreamType("unit", "transcription_graph") )
		return NULL ;

	UnitMenu* umenu = new UnitMenu("transcription_graph", "unit", m_dataModel.conventions().getConfiguration(), m_dataModel.conventions().getLayout("Menu"), false);
	return umenu ;
}


} //namespace
