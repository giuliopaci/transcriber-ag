/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */


#include "AGEditor.h"
#include "AudioWidget/AudioSignalView.h"
#include "Common/Explorer_filter.h"

#define LANGUAGE_ARABIC "ara"

namespace tag {

AGEditor::AGEditor(Gtk::Window* top)
: m_top(top), m_activityWatcher(NULL),
m_agFilename(""), m_fileFormat("TransAG"),
m_autosavePath(""), m_lastsavePath(""),
m_isAutosaved(false), m_conventions(""),
m_signalView(NULL), m_nbTracks(0), m_newFile(false)
{
	// INITIALISATION
	m_command_line_offset_segid = "" ;
	m_command_line_offset = -1 ;
	m_isLocked = false;
	m_statusBar = NULL;

	m_threads	= false;
	m_rtsp		= false;
	m_workdir	= g_get_home_dir();
	m_loaded = false ;
	m_loading_cancelled = false ;
	m_nbThread=0;

	// creates user activity tracking object
	m_activityWatcher = new ActivityWatcher(this);

	m_box = Gtk::manage(new class Gtk::VBox());
	add(*m_box);
	m_box->show();
	m_segThread = NULL ;

	m_signalView = NULL ;
}

AGEditor::~AGEditor()
{
	if ( m_activityWatcher != NULL )
		delete(m_activityWatcher) ;

	if ( m_signalView != NULL )
		delete(m_signalView) ;
}


/******************************************************************************/
/******************************** Convention **********************************/
/******************************************************************************/

const std::string& AGEditor::getFileName()
{
	return (m_agFilename.empty() ? m_defaultFilename : m_agFilename);
}

/******************************************************************************/
/******************************** Signal  **********************************/
/******************************************************************************/

void AGEditor::setSignalOffset(float offset)
{
	//-1: see if command line had been used
	float of = offset ;
	if (offset==-1 && m_command_line_offset!=-1)
	{
		of = m_command_line_offset ;
	}
	if (m_signalView && of !=-1)
		m_signalView->setCursor(of) ;
}


Gtk::Window* AGEditor::getTopWindow()
{
	if ( m_top != NULL ) return m_top;
	Gtk::Container* top = this->get_toplevel();
	if ( top )
		if ( GTK_WIDGET_TOPLEVEL(top->gobj()) ) return (Gtk::Window*)top;
		else
			TRACE_D << " TOP IS NOT A WINDOW" << endl;
	return NULL ;
}

} //namespace
