/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */

#include "Editors/AnnotationEditor/renderers/BackgroundRenderer.h"
#include "Common/util/Utils.h"
#include "Common/util/StringOps.h"
#include "Common/util/FormatTime.h"
#include "Common/Dialogs.h"

using namespace tag;

/** constructor */
BackgroundRenderer::BackgroundRenderer (AnnotationView* view)
: AnnotationRenderer(view, "background")
{
	m_tagFlags = IS_ACTIVE_TAG | IS_TEXT_EMBEDDED | CAN_BE_HIDDEN_TAG ;
	configure() ;
	configureMenu() ;
	setUseTooltip(true) ;
}

/** configure renderer -> justification */
void
BackgroundRenderer::configure ()
{
	//> even for convention entities, style and weight are same than
	// default ones
	Glib::ustring style = getLook("background", "style") ;
	Glib::ustring weight = getLook("background", "weight") ;
	Glib::ustring color_fg = getLook("background", "fg") ;
	Glib::ustring color_bg = getLook("background", "bg") ;

	AnnotationRenderer::configure("background", "background", true, false, color_fg, color_bg);
	AnnotationRenderer::configure("background", "background", true, true, color_fg, color_bg);
}

void BackgroundRenderer::configureMenu()
{
	if (m_dataModel.hasAnnotationGraph("background_graph")  )
	{
		bool display_error = false;
		if ( m_view->getViewTrack() == -1)
			display_error = true;
		contextMenu = new BackgroundMenu("background");
		contextMenu->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun(m_view, &AnnotationView::editAnnotationProperties), "background"));
		contextMenu->signalDeleteAnnotation().connect(sigc::bind<string>(sigc::mem_fun(m_view, &AnnotationView::deleteAnnotation), "background"));

		// background properties not editable
		contextMenu_lock = new AnnotationMenu("background", true, false);
		contextMenu_lock->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun(m_view, &AnnotationView::editAnnotationProperties), "background"));
	}
}

/**
 *  render section annotation at current text cursor position
 *  @param id annotation id
 */
void
BackgroundRenderer::render (const string & id, bool r2l, bool with_contents)
{
	if (!canRender(id))
		return ;

	Glib::RefPtr<Gtk::IconTheme>theme = Gtk::IconTheme::get_default() ;
	Glib::RefPtr<Gdk::Pixbuf>pixbuf = theme->load_icon("background_enable", 15,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
	Glib::ustring type = "background" ;
	//const string& start_id = m_dataModel.getMainstreamStartElement(id);
	int notrack = m_dataModel.getElementSignalTrack(id) ;
	float start = m_dataModel.getElementOffset(id, true) ;

	bool display_error = false ;

	string parallel_segment_start = m_dataModel.getByOffset(m_dataModel.mainstreamBaseType(), start, notrack) ;

	// AGAPI seems to sometimes give previous segment even if not totally adequate
	// ? maybe only for background case ?
	// Check conditions and re-ajust if needed
//	if (!parallel_segment_start.empty())
//		parallel_segment_start = m_dataModel.checkBackgroundParallelSegmentAtStart(parallel_segment_start, id) ;

	if (!parallel_segment_start.empty()) {
//		m_view->getBuffer()->insertPixElement(id, parallel_segment_start, type, true, pixbuf, "background") ;
		m_view->getBuffer()->insertPixElement(id, parallel_segment_start, type, true, "((o-))", "background") ;
	}
	else {
		Glib::ustring text = _("Display Error") ;
		text.append("\n") ;
		text.append(_("(can't find corresponding base type element)")) ;
		Log::err() << "Display error when rendering (start) " << id << std::endl ;
		Log::err() << text << id << std::endl ;
	}
}


/**
 *  render section annotation at current text cursor position
 *  @param id annotation id
 */
void BackgroundRenderer::render_end(const string & id, bool r2l)
{
	if (!canRender(id))
		return ;

	Glib::RefPtr<Gtk::IconTheme>theme = Gtk::IconTheme::get_default() ;
	Glib::RefPtr<Gdk::Pixbuf>pixbuf = theme->load_icon("background_disable", 15,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
	Glib::ustring type = "background" ;
	//const string& next_id = m_dataModel.getMainstreamNextElement(id);
	int notrack = m_dataModel.getElementSignalTrack(id) ;
	float end = m_dataModel.getElementOffset(id, false) ;

	string parallel_segment_end = m_dataModel.getByOffset(m_dataModel.mainstreamBaseType(), end, notrack) ;

	//UNCOMMENT TO PLACE PIX ELEMENT AT END OF LINE
	//> for displaying at end of line, Astuce de Sioux:
	// go to next segment: the insertPixElement will compute good position
	// by backwarding iterator
	//TODO: faire plus propre :)
	//const string& next = m_dataModel.getNextAnchoredElementId(parallel_segment_end) ;

	if (!parallel_segment_end.empty())
		parallel_segment_end = m_dataModel.checkBackgroundParallelSegmentAtEnd(parallel_segment_end, id) ;

	if (!parallel_segment_end.empty()) {
		//IDEM (modify insertPixElement too)
		m_view->getBuffer()->insertPixElement(id, parallel_segment_end, type, false, "((-o))", "background_end");
		//m_view->getBuffer()->insertPixElement(id, parallel_segment_end, type, false, pixbuf, "background_end");
	}
	else {
		Glib::ustring text = _("Display Error") ;
		text.append("\n") ;
		text.append(_("(can't find corresponding base type element)")) ;
		Log::err() << "Display error when rendering (end) " << id << std::endl ;
		Log::err() << text << id << std::endl ;
	}
}

bool BackgroundRenderer::canRender(const string & id)
{
	const string&  subtype = m_dataModel.getElementProperty(id, "type", "none");
	return (m_dataModel.getElementType(id) == "background" && !subtype.empty() && subtype != "none") ;
}

Glib::ustring BackgroundRenderer::getSegmentText(const string & id)
{
	Glib::ustring res("") ;
	string types_s = m_dataModel.getElementProperty(id, "type", "none") ;
	if ( types_s == "none" ) return res;

	std::vector<string> types;
	std::vector<string>::iterator itv;
	StringOps(types_s).split(types, ";");
	for ( itv=types.begin(); itv != types.end(); ++itv )
		*itv = m_dataModel.conventions().getLocalizedLabel(*itv);

	string level = m_dataModel.getElementProperty(id, "level") ;
	level = m_dataModel.conventions().getLocalizedLabel(level);

	if (level.empty() && types.size()==0)
		return "" ;


	std::vector<string>::iterator it ;
	for (it=types.begin(); it!=types.end(); it++) {
		res.append(*it) ;
		std::vector<string>::iterator tmp = it ;
		tmp++ ;
		if (tmp!=types.end())
			res.append(" - ") ;
	}
	res.append(" (") ;
	res.append(level) ;
	res.append(")") ;

	return res ;
}

/*
 * delete background annotation  from text buffer
 */
void BackgroundRenderer::deleteElement (const string & id)
{
	getBuffer()->deletePixElement(id);
}


/*
 * update turn annotation in text buffer
 */
void BackgroundRenderer::updateElement(const string & id, bool r2l)
{
	guint offset = getBuffer()->getAnchoredIter(id).get_offset();
	deleteElement(id);
	getBuffer()->setCursor(offset);
	render(id, r2l);
}

Gtk::Box* BackgroundRenderer::getTooltipBox(const string& id, bool is_start)
{
	Gtk::VBox* box = Gtk::manage(new Gtk::VBox()) ;

	Glib::ustring typeLevel = getSegmentText(id) ;
	if (!typeLevel.empty()) {
		Gtk::Label* label = Gtk::manage(new Gtk::Label(typeLevel)) ;
		box->pack_start(*label, false,false) ;
	}

	float start = m_dataModel.getElementOffset(id, true);
	float end = m_dataModel.getElementOffset(id, false);
	if (start!=-1 && end!=-1) {
		Glib::ustring start_s = FormatTime(start, true) ;
		Glib::ustring end_s = FormatTime(end, true) ;
		Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox()) ;
		Gtk::Label* label_s = Gtk::manage(new Gtk::Label()) ;
		Gtk::Label* label_e = Gtk::manage(new Gtk::Label()) ;
		Gtk::Label* label_sep = Gtk::manage(new Gtk::Label(" - ")) ;
		hbox->pack_start(*label_s, false, false, 2) ;
		hbox->pack_start(*label_sep, false, false, 2) ;
		hbox->pack_start(*label_e, false, false, 2) ;
		box->pack_start(*hbox, false,false) ;
		if (is_start) {
			label_s->set_markup("<b>"+start_s+"</b>") ;
			label_e->set_label(end_s) ;
		}
		else {
			label_e->set_markup("<b>"+end_s+"</b>") ;
			label_s->set_label(start_s) ;
		}
	}
	box->show_all_children() ;

	return box ;
}




