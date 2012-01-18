/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */

/**
 * @file TurnRenderer.cpp
 *
 *  turn annotation renderer implementation
 */

#include "Editors/AnnotationEditor/renderers/TurnRenderer.h"
#include "Editors/AnnotationEditor/dialogs/TurnPropertiesDialog.h"

using namespace tag;

/** constructor */
TurnRenderer::TurnRenderer (AnnotationView* view)
: AnnotationRenderer(view, "turn")
{
	m_tagFlags = IS_ACTIVE_TAG;
	configure();
	configureMenu() ;
}

/** configure renderer -> justification */
void
TurnRenderer::configure ()
{
	AnnotationRenderer::configure ();
	// also configure for male & female speaker types
	AnnotationRenderer::configure (m_name + "_male");
	AnnotationRenderer::configure (m_name + "_female");
	AnnotationRenderer::configure (m_name + "_nospeech");
}

void TurnRenderer::configureMenu()
{
	//  connect speaker menu related events
	if ( !m_dataModel.isMainstreamType("turn", "transcription_graph") )
		return ;

	bool overlap = (m_dataModel.conventions().canOverlap("turn"));
	SpeakerMenu* smenu = new SpeakerMenu(m_dataModel.getSpeakerDictionary(), m_view->getTranscriptionLanguage(), overlap);
	contextMenu = smenu ;
	smenu->signalSetSpeaker().connect(sigc::mem_fun (*m_view, &AnnotationView::setSpeaker));
	smenu->signalEditSpeaker().connect(sigc::mem_fun (*m_view, &AnnotationView::editSpeakerProperties));
	smenu->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (*m_view, &AnnotationView::editAnnotationProperties), "turn"));
	smenu->signalDeleteAnnotation().connect(sigc::bind<string>(sigc::mem_fun (*m_view, &AnnotationView::deleteAnnotation), "turn"));
	smenu->setDefaultLanguage(m_view->getTranscriptionLanguage());

	SpeakerMenu* slmenu =  new SpeakerMenu(m_dataModel.getSpeakerDictionary(), m_view->getTranscriptionLanguage(), overlap, false);
	contextMenu_lock = slmenu ;
	slmenu->signalEditSpeaker().connect(sigc::mem_fun (*m_view, &AnnotationView::editSpeakerProperties));
	slmenu->signalEditAnnotation().connect(sigc::bind<string>(sigc::mem_fun (*m_view, &AnnotationView::editAnnotationProperties), "turn"));
}

AnnotationPropertiesDialog* TurnRenderer::newPropertiesDialog(const string& id)
{
	TurnPropertiesDialog* dlg = new TurnPropertiesDialog(m_view->getTopWindow(), m_dataModel, id, m_view->isEditable());
	dlg->signalTurnLanguageChanged().connect(sigc::mem_fun(m_view, &AnnotationView::updateAlignment));
	return dlg;
}

/**
 *  render section annotation at current text cursor position
 *  @param id annotation id
 */
void
TurnRenderer::render (const string & id, bool r2l, bool with_contents)
{
	string tagname;
	string label;
	setLabelAndTagname(id, label, tagname);

	int notrack = 0;
	int order = m_dataModel.getOrder (id);

	if (order > 0)
		notrack = /* order */ 3;
	else if ( m_view->getViewTrack() == -1 )
		notrack = m_dataModel.getElementSignalTrack (id);

//    TRACE << "insertTurn id=" << id << " order = " << order << " notrack =" << notrack << endl;

	getBuffer ()->insertAnchoredLabel ("turn", id, label, tagname, notrack, r2l, "");
	const Gtk::TextIter& curpos = getBuffer()->getCursor();
	if ( curpos != getBuffer()->end() )  {
		// we're not inserting "on the fly"
		// eventually adjust alignment tag  for alignment id and move align id to current cursor pos
		const string & align_id = m_dataModel.getAlignmentId (id);
		if ( notrack > 0 ) getBuffer()->setTrackTag (align_id, notrack);
		getBuffer()->moveAnchor(align_id, curpos);
	}
}


/*
 * delete turn annotation  from text buffer
 */
void TurnRenderer::deleteElement(const string & id)
{
//	getBuffer()->deleteAnchoredLabel(id, m_dataModel.existsElement(id));
	getBuffer()->deleteAnchoredLabel(id, false);
}


/*
 * update turn annotation in text buffer
 */
void TurnRenderer::updateElement(const string & id, bool r2l)
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

void TurnRenderer::setLabelAndTagname(const string & id, string& label, string& tagname)
{
	tagname = "turn";
	const string & spkid =
		m_dataModel.getElementProperty (id, "speaker", Speaker::NO_SPEECH);

	label = " ";
	if (! ( spkid.empty() || spkid == Speaker::NO_SPEECH) )
	{
		try
		{
			const Speaker & spk =
				m_dataModel.getSpeakerDictionary ().getSpeaker (spkid);
			label += spk.getFullName ();
			switch (spk.getGender ())
			{
			case Speaker::MALE_GENDER:
				tagname += "_male";
				break;
			case Speaker::FEMALE_GENDER:
				tagname += "_female";
				break;
			default:
				break;
			}
		}
		catch (SpeakerDictionary::NotFoundException & n)
		{
			Log::err() << __FILE__ << ", line " << __LINE__
			<< " : SpeakerDictionary::NotFoundException : "
			<< ",  spk=" << spkid << ", id=" << id << endl;
			label += spkid;
		}
	}
	else
	{
		const string & nospeech_type =
			m_dataModel.getElementProperty (id, "nospeech_type");
		if (nospeech_type == "")
			label += Speaker::noSpeaker ().getFullName ();
		else
			label += string ("(") + nospeech_type + string (")");

	}
	label += "  ";

	if ( spkid.empty() || spkid == Speaker::NO_SPEECH )
			tagname += "_nospeech";
}
