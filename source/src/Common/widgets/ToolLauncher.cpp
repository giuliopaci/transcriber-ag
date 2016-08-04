/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */
#include <stdio.h>
#include <stdlib.h>

#include "ToolLauncher.h"
#include "Common/globals.h"
#include "Common/util/Log.h"
#include "Common/util/StringOps.h"
#include "Common/util/Utils.h"
#include "Common/widgets/GtUtil.h"

namespace tag {


//------------------------------------------------------------------------------
//									 STATIC BUSINESS
//------------------------------------------------------------------------------

//
// the Languages instance
ToolLauncher* ToolLauncher::m_instance = NULL;

ToolLauncher* ToolLauncher::getInstance()
{
	return m_instance;
}

void ToolLauncher::configure(Parameters* p_parameters, Gtk::Window* parent)
{
	if ( m_instance != NULL ) delete m_instance;
	m_instance = new ToolLauncher(p_parameters, parent);
}

void ToolLauncher::kill()
{
	if ( m_instance != NULL ) {
		delete m_instance;
		m_instance = NULL ;
	}
}

//------------------------------------------------------------------------------
//  							SUBCLASS BUSINESS : Tool
//------------------------------------------------------------------------------

void ToolLauncher::Tool::setObject(Glib::ustring value) 					{ object = value ;}
void ToolLauncher::Tool::setOptions(Glib::ustring value) 				{ options = value ; if (personalizedOptions.empty()) personalizedOptions = value; }
void ToolLauncher::Tool::setPersonalizedOptions(Glib::ustring value) 	{ personalizedOptions = value ;}
bool ToolLauncher::Tool::isFileScope() 									{ return scope.compare("file")==0 ;}
Glib::ustring ToolLauncher::Tool::getObject() 							{ return object ;}
Glib::ustring ToolLauncher::Tool::getIdentifiant() 						{ return identifiant ;}
Glib::ustring ToolLauncher::Tool::getDisplay()							{ return display ;}
Glib::ustring ToolLauncher::Tool::getOptions()							{ return options ;}
Glib::ustring ToolLauncher::Tool::getType()								{ return type ;}
Glib::ustring ToolLauncher::Tool::getCommand() 							{ return (object + " " + getOptions()) ; }

ToolLauncher::Tool::Tool(Glib::ustring identifiant, Glib::ustring display, Glib::ustring scope, Glib::ustring type)
{
	this->identifiant = identifiant ;
	this->display = display ;
	this->scope = scope ;
	this->type = type ;
}

void ToolLauncher::Tool::configureOptions(Glib::ustring file_path)
{
	string value = options ;
	StringOps ops(value) ;
	string perso = ops.replace("%f", file_path , true) ;
	setPersonalizedOptions(perso) ;
}

std::vector<std::string> ToolLauncher::Tool::getOptionsArgv()
{
	std::vector<std::string> argv ;
	mini_parser(' ', personalizedOptions, &argv) ;
	return argv ;
}

//------------------------------------------------------------------------------
//  							SUBCLASS BUSINESS : ToolDialog
//------------------------------------------------------------------------------

ToolLauncher::ToolDialog::ToolDialog(Glib::ustring command)
{
	// sexy
	Icons::set_window_icon(this, ICO_TRANSCRIBER, 12) ;
	set_title("Tool Launcher") ;

	// Close button
	close_button = add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE) ;
	close_button->signal_released().connect(sigc::mem_fun(*this, &ToolLauncher::ToolDialog::onClose)) ;

	// Prepare view
	stout_view = Gtk::manage(new Gtk::TextView()) ;
	sterr_view = Gtk::manage(new Gtk::TextView()) ;

	Gtk::Label* blank = Gtk::manage(new Gtk::Label(" ")) ;
	Gtk::Label* blank2 = Gtk::manage(new Gtk::Label(" ")) ;

	// Construct
	Gtk::VBox* vbox = get_vbox() ;
	vbox->pack_start(title_box, false, false) ;
		title_box.pack_start(title_image, false, false, 3) ;
		title_box.pack_start(title_label, false, false, 3) ;
		title_box.pack_start(*blank, true, true, 3) ;
		title_box.pack_start(title_waiting_image, false, false, 3) ;
	vbox->pack_start(scroll, true, true) ;
		scroll.add(v_box) ;
			v_box.pack_start(command_frame, false, false, 7) ;
				command_frame.set_label(_("Command")) ;
				command_frame.set_label_align(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER) ;
				command_frame.add(command_hbox) ;
					command_hbox.pack_start(command_text, false, false) ;
					command_hbox.pack_start(*blank2, true, true) ;
			v_box.pack_start(stout_frame, false, false, 7) ;
				stout_frame.add(*stout_view) ;
				stout_frame.set_label(_("Standard output")) ;
				stout_frame.set_label_align(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER) ;
			v_box.pack_start(sterr_frame, false, false, 7) ;
				sterr_frame.add(*sterr_view) ;
				sterr_frame.set_label(_("Error output")) ;
				sterr_frame.set_label_align(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER) ;

	scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC) ;
	scroll.set_size_request(300, 300) ;

	// Initialize
	command_frame.get_label_widget()->set_name("bold_label") ;
	sterr_frame.get_label_widget()->set_name("bold_label") ;
	stout_frame.get_label_widget()->set_name("bold_label") ;
	title_label.set_label(_("EXTERNAL TOOLS")) ;
	title_label.set_name("bold_label") ;
	stout_view->set_editable(false) ;
	sterr_view->set_editable(false) ;
	stout_view->set_cursor_visible(false) ;
	sterr_view->set_cursor_visible(false) ;
	command_text.set_label(command) ;

	title_image.set_image(ICO_TOOL_LAUNCHER, 35) ;
	title_image.set_tip(_("Inactive")) ;

	title_waiting_image.set_image(ICO_TOOL_LAUNCHER_OFF, 22) ;

	vbox->show_all_children(true) ;
}

bool ToolLauncher::ToolDialog::on_delete_event(GdkEventAny* event)
{
	onClose() ;
	return true ;
}

void ToolLauncher::ToolDialog::setEndStatus(std::string stout, std::string sterr, int error)
{
	// Display returns
	stout_view->get_buffer()->set_text(stout) ;
	sterr_view->get_buffer()->set_text(sterr) ;

	if (error == -1)
	{
		title_waiting_image.set_image(ICO_TOOL_LAUNCHER_KO, 22) ;
		title_waiting_image.set_tip(_("Terminated with errors")) ;
	}
	else
	{
		title_waiting_image.set_image(ICO_TOOL_LAUNCHER_OFF, 22) ;
		title_waiting_image.set_tip(_("Successfully terminated")) ;
	}

	stout_view->set_sensitive(true) ;
	sterr_view->set_sensitive(true) ;
	close_button->set_sensitive(true) ;

	// Unlock all & display
	in_progress = false ;
}

void ToolLauncher::ToolDialog::setStartStatus()
{
	in_progress = true ;

	// Lock all
	title_waiting_image.set_image(ICO_TOOL_LAUNCHER_ON, 22) ;
	title_waiting_image.set_tip(_("Active")) ;

	stout_view->set_sensitive(false) ;
	sterr_view->set_sensitive(false) ;
	close_button->set_sensitive(false) ;
	stout_view->get_buffer()->set_text(_("... Processing ...")) ;
}

void ToolLauncher::ToolDialog::onClose()
{
	if (in_progress)
		return ;

	saveGeoAndHide() ;
	signalClosed().emit() ;
}

//------------------------------- GEOMETRY INTERFACE
void ToolLauncher::ToolDialog::getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	get_position(pos_x, pos_y) ;
	panel = -1 ;
	get_size(size_xx, size_yy) ;
}

void ToolLauncher::ToolDialog::setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel)
{
	if (pos_x>0 && pos_y>0)
		move(pos_x, pos_y) ;
	if (size_xx>0 && size_yy>0)
		resize(size_xx, size_yy) ;
}

Glib::ustring ToolLauncher::ToolDialog::getWindowTagType()
{
	return SETTINGS_TOOL_LAUNCHER ;
}

int ToolLauncher::ToolDialog::loadGeoAndDisplay(bool rundlg)
{
	loadPos() ;
	if (rundlg)
		return run() ;
	else
	{
		show() ;
		return 1 ;
	}
}

void ToolLauncher::ToolDialog::saveGeoAndHide()
{
	if (is_visible())
		savePos() ;
	hide() ;
}

void ToolLauncher::ToolDialog::getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	size_xx = 400 ;
	size_xx = 500 ;
}


//------------------------------------------------------------------------------
//									INSTANCE BUSINESS
//------------------------------------------------------------------------------


ToolLauncher::ToolLauncher(Parameters* p_parameters, Gtk::Window* p_parent)
{
	dialog = NULL ;
	parent = p_parent ;
	parameters = p_parameters ;
	nbGlobal = 0 ;
	nbFile = 0 ;
	loadTools() ;
}

ToolLauncher::~ToolLauncher()
{
	std::vector<ToolLauncher::Tool*>::iterator it = tools.begin() ;
	while (it!=tools.end() )
	{
		Tool* tmp = *it ;
		it = tools.erase(it) ;
		if (tmp)
			delete(tmp) ;
	}

	if (dialog)
	{
		delete(dialog) ;
		dialog = NULL ;
	}
}

ToolLauncher::Tool* ToolLauncher::addTool(Glib::ustring identifiant, Glib::ustring display, Glib::ustring scope, Glib::ustring type)
{
	ToolLauncher::Tool* tool = new ToolLauncher::Tool(identifiant, display, scope, type) ;
	tools.push_back(tool) ;
	if (scope.compare("file")==0)
		nbFile++ ;
	else
		nbGlobal++ ;
	return tool ;
}

void ToolLauncher::loadTools()
{
	Glib::ustring path = parameters->getParameterValue("General", "start", "config") ;
	path = Glib::build_filename(path, "toolsAG.xml") ;

	//TODO test existence of file
	//TODO use locale ?

	try
	{
		// Load
		ToolLauncher_XMLHandler handler(this);
		CommonXMLReader reader(&handler);
		reader.parseFile(path);
		Log::out() << "> loading ToolLauncher [OK]" << std::endl ;

		// Delete empty
		std::vector<ToolLauncher::Tool*>::iterator it = tools.begin() ;
		while (it!=tools.end() )
		{
			Tool* tmp = *it ;
			if (tmp && tmp->getObject().empty() && tmp->getOptions().empty())
			{
				it = tools.erase(it) ;
				delete(tmp) ;
			}
			else
				it ++ ;
		}
	}
	catch(const char *msg)
	{
		Log::out() << "> loading ToolLauncher [0]: " << msg << std::endl ;
	}
}

void ToolLauncher::launch(ToolLauncher::Tool* tool)
{
	if (!tool)
		return  ;

	// get directory of execution
	std::string directory = Glib::path_get_dirname(tool->getObject()) ;
	// get name of child to be launched
	std::string name = Glib::path_get_basename(tool->getObject()) ;
	// get total command
	std::string command = tool->getCommand() ;
	// get tool type ;)
	std::string type = tool->getType() ;

	// get all arguments in argv format
	std::vector<std::string> argv = tool->getOptionsArgv() ;
	argv.insert(argv.begin(), name) ;

	dialog = NULL ;
	if (type=="script")
	{
		dialog = new ToolDialog(command) ;
		dialog->loadGeoAndDisplay(false) ;
		dialog->signalClosed().connect(sigc::mem_fun(*this, &ToolLauncher::onDialogClose)) ;
	}
	Glib::Thread::create( sigc::bind<ToolDialog*,Glib::ustring,std::vector<std::string> >( sigc::mem_fun(*this,&ToolLauncher::process), dialog,directory,argv), false) ;
}


void ToolLauncher::process(ToolLauncher::ToolDialog* dialog, Glib::ustring childDirectory, std::vector<std::string> argv)
{
	if (dialog)
	{
#ifndef WIN32
		gdk_threads_enter() ;
		dialog->setStartStatus() ;
		gdk_threads_leave() ;
#endif
	}

	int code_exit ;
	std::string stout, sterr ;

	try
	{
		Glib::spawn_sync(childDirectory, argv, Glib::SPAWN_SEARCH_PATH, sigc::mem_fun(*this, &ToolLauncher::emptyslot), &stout, &sterr, &code_exit) ;
	}
	catch (const Glib::Error& e)
	{
		code_exit = -1 ;
		sterr = e.what() ;
	}

	if (dialog)
	{
#ifndef WIN32
		gdk_threads_enter() ;
		GtUtil::flushGUI(false,false) ;
#endif
		dialog->setEndStatus(stout, sterr, code_exit) ;
#ifndef WIN32
		gdk_threads_leave() ;
#endif
	}
}

void ToolLauncher::onDialogClose()
{
	delete(dialog) ;
	dialog=NULL;
}

//------------------------------------------------------------------------------
//								EXTRERNAL CLASS BUSINESS
//------------------------------------------------------------------------------

ToolLauncher_XMLHandler::ToolLauncher_XMLHandler(ToolLauncher* launcher)
{
	tLauncher = launcher ;
	m_object = NULL ;
	m_options = NULL ;
	m_tool = NULL ;
	m_tools = NULL ;
	m_current_tool = NULL ;
	in_object = false ;
	in_options = false ;
}

ToolLauncher_XMLHandler::~ToolLauncher_XMLHandler()
{
	if ( m_object != NULL )
		XMLString::release (&m_object);
	if ( m_options != NULL )
		XMLString::release (&m_options);
	if ( m_tool != NULL )
		XMLString::release (&m_tool);
	if ( m_tools != NULL )
		XMLString::release (&m_tools);
}

/* start XML element */
void ToolLauncher_XMLHandler::startElement (const XMLCh * const uri,
			  const XMLCh * const localname,
			  const XMLCh * const qname, const Attributes & attrs)
{
	map < string, string > attmap;
	map < string, string >::iterator it;
	getAttributes (attrs, attmap);

	if ( m_tools == NULL )
	{
		m_object = XMLString::transcode("object");
		m_options = XMLString::transcode("options");
		m_tool = XMLString::transcode("tool");
		m_tools = XMLString::transcode("tools");
	}

	if (XMLString::compareIString(localname, m_tool) == 0)
	{
		m_current_tool = tLauncher->addTool(attmap["id"], attmap["display"], attmap["scope"], attmap["type"]) ;
	}
	else if (XMLString::compareIString(localname, m_object) == 0)
	{
		if (m_current_tool) {
			in_object = true ;
			current_object = "" ;
		}
	}
	else if (XMLString::compareIString(localname, m_options) == 0)
	{
		if (m_current_tool) {
			in_options = true ;
			current_options = "" ;
		}
	}
}

void ToolLauncher_XMLHandler::endElement (const XMLCh * const uri,
			const XMLCh * const localname,
			const XMLCh * const qname)
{
	if (m_current_tool && in_options)
	{
		in_options = false ;
		if (!current_options.empty())
			m_current_tool->setOptions(current_options) ;
	}
	else if (m_current_tool && in_object)
	{
		in_object = false ;
		if (!current_object.empty())
			m_current_tool->setObject(current_object) ;
	}
}

void ToolLauncher_XMLHandler::characters (const XMLCh * const chars, const XMLSize_t length)
{
	if (length != 0 && in_object )
	  	current_object += getString(chars);
	else if (length != 0 && in_options )
		current_options += getString(chars);
}


} // namespace
