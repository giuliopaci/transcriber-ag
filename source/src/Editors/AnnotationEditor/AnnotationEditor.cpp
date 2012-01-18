/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @file AnnotationEditor.cpp
 * @brief Annotation input widget implementation
 *
 *  Handles interactions between text widget, signal widget
 *  and englobing applicationS
 */
#include "AnnotationEditor.h"

#include <iostream>
#include <sstream>
#include <map>
#include <iterator>
#include <algorithm>
#include <gtkmm/accelmap.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <sys/time.h>
#include <gtk/gtkimcontext.h>

#include "Editors/AnnotationEditor/menus/SignalPopupMenu.h"
#include "Editors/AnnotationEditor/dialogs/DialogSelectConventions.h"
#include "Editors/AnnotationEditor/dialogs/SaveAudioDialog.h"
#include "Editors/AnnotationEditor/dialogs/GoToDialog.h"
#include "Editors/AnnotationEditor/dialogs/FileDialogs.h"
#include "Editors/AnnotationEditor/dialogs/CheckerDialog.h"

#include "AudioWidget/AudioSignalView.h"

#include "Common/ColorsCfg.h"
#include "Common/Explorer_filter.h"
#include "Common/globals.h"
#include "Common/Formats.h"
#include "Common/VersionInfo.h"
#include "Common/util/FileHelper.h"
#include "Common/externals/sendpraat.h"
#include "Common/util/Utils.h"
#include "Common/util/FileHelper.h"
#include "Common/widgets/ToolLauncher.h"
#include "Common/widgets/GtUtil.h"

#include "DataModel/signals/SignalConfiguration.h"
#include "DataModel/conventions/ModelChecker.h"

#include "MediaComponent/base/Guesser.h"

#include "VideoComponent/VideoPlayer/VideoWidget.h"
#include "VideoComponent/FrameBrowser/FrameBrowser.h"
#include "VideoComponent/VideoManager.h"

#define NB_SEG_COLORS 4

#define LANGUAGE_ARABIC "ara"

static const char *StdAudioExt[] =
{ ".wav", ".WAV", ".mp3", ".MP3", ".sph", ".SPH", ".aif", ".AIF", ".aiff",
        ".AIFF", ".au", ".AU", ".ogg", ".OGG", NULL };

namespace tag {

#define THREADS_ENTER if ( m_threads ) gdk_threads_enter();
#define THREADS_LEAVE if ( m_threads ) { gdk_flush(); gdk_threads_leave(); }

/*
 *	constructor
 *   creates annotations text view and signal view widgets
 * connects all callbacks
 *
 */
AnnotationEditor::AnnotationEditor(Gtk::Window* top) : AGEditor::AGEditor(top)
{
	pathHintActualized = false ;
	loadingPeaksReady = false ;
	loadingTracksReady = false ;
	m_loadingView = 0 ;
	m_nbLoadingTracks = 0 ;
	m_totalLoadingTracks = 0;

	firstFocus = false ;

	resultSet = NULL;
	m_statusBar = NULL;

	progressWatcher = NULL;

	videoPlayer = NULL;
	videoFrameBrowser = NULL;
	videoManager = NULL;

	m_activeViewMode = -1;
	m_defaultViewMode = -1;

	m_inhibateSynchro = false;
	synchroLock = false;
	m_syncSource = NULL;

	/*** mode ***/
	consultationMode = false;
	reportLevel = 1;

	m_lastTrack = -1;
	m_activeTrack = -1;
	m_modeStereo = false;
	m_selectTrack_wt_cursor = false;

	m_hiddenTagsMode = -1;
	m_lastModeWhenHidden = 1;
	m_lastEditMode = "";
	m_tagHidden_lastInterlineAbove = -1;
	m_tagHidden_lastInterlineBelow = -1;

	// ICI param conf multitexte Vert/horiz
	m_textbox = Gtk::manage(new class Gtk::HBox());
	m_box->add(*m_textbox);
	m_textbox->show();

	// start with a single text view
	m_activeView = addTextView(-1);

	// default language
	set_input_language(InputLanguageHandler::get_input_language_by_shortcut(
	        DEFAULT_LANGUAGE));

	// creates user activity tracking object
	createActionGroups();

	mapTrackDelays[0] = 0 ;
	mapTrackDelays[1] = 0 ;
}

/*
 * add new text view to editor
 */
AnnotationView* AnnotationEditor::addTextView(int notrack)
{
	Gtk::ScrolledWindow* sw = Gtk::manage(new class Gtk::ScrolledWindow());
	sw->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
	m_textbox->pack_end(*sw);

	AnnotationView* view = Gtk::manage(new class AnnotationView(*this));
	m_textView.insert(m_textView.begin(), view);
	sw->add(*view);
	sw->hide_all();

	//> -- Set some options
	view->setViewTrack(notrack);

	//> -- Signals connection
	if (isDebugMode())
		view->signalSetCursor().connect(sigc::mem_fun(*this, &AnnotationEditor::cursorChanged)) ;

	// connect synchro related events
	view->signalSetCursor().connect(sigc::bind<AnnotationView*>(sigc::mem_fun(*this, &AnnotationEditor::synchroSignalToText), view));
	// connect file update related events
	view->getBuffer()->signal_modified_changed().connect(sigc::mem_fun(*this, &AnnotationEditor::onFileModified));
	m_dataModel.signalModelUpdated().connect(sigc::mem_fun(*this, &AnnotationEditor::onModelUpdated));
	// connect focus to update active view when focus changes
	view->signalHasFocus().connect(sigc::bind<AnnotationView*>(sigc::mem_fun(*this, &AnnotationEditor::setActiveView), view));
	// connect DataModel updates related events
	view->signalElementModified().connect(sigc::mem_fun(*this, &AnnotationEditor::updateTrack));
	// connect DataModel updates related events
	view->signalEditSpeaker().connect(sigc::mem_fun(*this, &AnnotationEditor::editSpeaker));
	//connect change view input language
	view->signalLanguageChange().connect(sigc::mem_fun(*this, &AnnotationEditor::onChangeInputLanguage));
	//connect drag And drop reception
	view->signalGtkDragTarget().connect(sigc::mem_fun(*this, &AnnotationEditor::onViewDragAndDropReceived));

	return view;
}

/*
 * set current active view
 */
void AnnotationEditor::setActiveView(AnnotationView* view)
{
	if (m_activeView != view)
	{
		m_activeView = view;
		view->setFocus(true) ;
		Glib::RefPtr<Gtk::ActionGroup> oldgroup = m_actionGroups["edit"];
		m_actionGroups["edit"] = m_activeView->getActionGroup("edit");
		if (m_signalView)
			m_signalView->setToFocus(view);
		signalUpdateUI().emit("edit", oldgroup);
		signalChangeActiveView().emit();
	}
}

void AnnotationEditor::createActionGroups()
{
	m_actionGroups["file"] = Gtk::ActionGroup::create("FileAction");
	//				_("_Save"), _("Save current annotation file")),
	m_actionGroups["file"]->add(Gtk::Action::create("file_save",
	        Gtk::Stock::SAVE), Gtk::AccelKey("<control>s"), sigc::bind<string>(
	        sigc::mem_fun(*this, &AnnotationEditor::onFileAction), "save"));
	m_actionGroups["file"]->get_action("file_save")->set_tooltip(_("Save file"));

	//				_("Save _as"), _("Save annotation file as new file")),
	m_actionGroups["file"]->add(Gtk::Action::create("file_saveas",
	        Gtk::Stock::SAVE_AS), sigc::bind<string>(sigc::mem_fun(*this,
	        &AnnotationEditor::onFileAction), "saveas"));
	m_actionGroups["file"]->get_action("file_saveas")->set_tooltip(
	        _("Save file as"));

	//				_("_Close"), _("Close current annotation file")),
	m_actionGroups["file"]->add(Gtk::Action::create("file_close",
	        Gtk::Stock::CLOSE), sigc::bind<string>(sigc::mem_fun(*this,
	        &AnnotationEditor::onFileAction), "close"));
	m_actionGroups["file"]->get_action("file_close")->set_tooltip(
	        _("Close file"));

	m_actionGroups["file"]->add(Gtk::Action::create("file_refresh",
	        Gtk::Stock::REFRESH), Gtk::AccelKey("<control>l"), sigc::bind<
	        string>(sigc::mem_fun(*this, &AnnotationEditor::onFileAction),
	        "refresh"));
	m_actionGroups["file"]->get_action("file_refresh")->set_tooltip(
	        _("Refresh file"));

	m_actionGroups["file"]->add(Gtk::Action::create("file_export",
	        _("_Export file")), sigc::bind<string>(sigc::mem_fun(*this,
	        &AnnotationEditor::onFileAction), "export"));
	m_actionGroups["file"]->get_action("file_export")->set_tooltip(
	        _("Export file"));

	m_actionGroups["file"]->add(Gtk::Action::create("file_revert_from_file",
	        Gtk::Stock::REVERT_TO_SAVED, _("Revert to saved file"),
	        _("Revert to saved file")), sigc::bind<string>(sigc::mem_fun(
	        *this, &AnnotationEditor::onFileAction), "revert_from_file"));
	m_actionGroups["file"]->get_action("file_revert_from_file")->set_tooltip(
	        _("Revert from file"));

	m_actionGroups["file"]->add(
	        Gtk::Action::create("file_revert_from_autosave",
	                Gtk::Stock::REVERT_TO_SAVED, _("Revert to autosaved file"),
	                _("Revert to autosaved file")), sigc::bind<string>(
	                sigc::mem_fun(*this, &AnnotationEditor::onFileAction),
	                "revert_from_autosave"));
	m_actionGroups["file"]->get_action("file_revert_from_autosave")->set_tooltip(
	        _("Revert from autosave"));

	ToolLauncher* toolLauncher = ToolLauncher::getInstance();
	if (toolLauncher && toolLauncher->hasFileScopeTools())
	{
		std::vector<ToolLauncher::Tool*> tools = toolLauncher->getTools();
		std::vector<ToolLauncher::Tool*>::const_iterator it;
		for (it = tools.begin(); it != tools.end(); it++)
		{
			// treat only global tool
			ToolLauncher::Tool* ctool = *it;
			if (ctool->isFileScope())
			{
				string name = ctool->getIdentifiant();
				string display = ctool->getDisplay();
				// action
				m_actionGroups["file"]->add(Gtk::Action::create(name, display,
				        display), sigc::bind<ToolLauncher::Tool*>(
				        sigc::mem_fun(*this,
				                &AnnotationEditor::onExternalAction), ctool));
			}
		}
	}

	m_actionGroups["annotate"] = Gtk::ActionGroup::create("AnnotateAction");

	// TODO -> recuperer les labels menus depuis les renderer pour generiquer la creation de ces menus
	m_actionGroups["annotate"]->add(Gtk::Action::create("annotate_new_section",
	        _("New se_ction"), _("Insert new section at current time")),
	        Gtk::AccelKey("<control>r"), sigc::bind<string, bool>(
	                sigc::mem_fun(*this, &AnnotationEditor::onMenuAction),
	                "new_section", false, false, ""));

	m_actionGroups["annotate"]->add(Gtk::Action::create("annotate_new_turn",
	        _("New _turn"), _("Insert new turn at current time")),
	        Gtk::AccelKey("<control>t"), sigc::bind<string, bool>(
	                sigc::mem_fun(*this, &AnnotationEditor::onMenuAction),
	                "new_turn", false, false, ""));

	m_actionGroups["annotate"]->add(Gtk::Action::create("annotate_new_segment",
	        _("New _segment"), _("Insert new segment at current time")),
	        Gtk::AccelKey("Return"), sigc::bind<string, bool>(sigc::mem_fun(
	                *this, &AnnotationEditor::onMenuAction), "new_segment",
	                false, false, ""));

	m_actionGroups["annotate"]->add(Gtk::Action::create("annotate_new_timestamp",
	        _("Add a _time stamp"), _("Insert new time anchor")),
	        Gtk::AccelKey("<control>Return"), sigc::bind<string, bool>(sigc::mem_fun(
	                *this, &AnnotationEditor::onMenuAction), "new_timestamp",
	                false, false, ""));

	m_actionGroups["annotate"]->add(Gtk::Action::create("annotate_new_event",
	        _("New _event"),
	        _("Insert new event at current cursor position")),
	        Gtk::AccelKey("<control>d"), sigc::bind<string, bool>(
	                sigc::mem_fun(*this, &AnnotationEditor::onMenuAction),
	                "new_event", false, true, ""));

	m_actionGroups["annotate"]->add(Gtk::Action::create("annotate_new_entity",
	        _("New named en_tity"),
	        _("Insert new named entity at current cursor position")),
	        Gtk::AccelKey("<control>e"), sigc::bind<string, bool>(
	                sigc::mem_fun(*this, &AnnotationEditor::onMenuAction),
	                "new_qualifier_entity", false, true, ""));

	m_actionGroups["annotate"]->add(Gtk::Action::create("annotate_new_background",
			_("New b_ackground segment"), _("Insert new background segment at current time")),
	        Gtk::AccelKey("<control>b"), sigc::bind<string, bool>(
	                sigc::mem_fun(*this, &AnnotationEditor::onMenuAction), "new_background", true, false, ""));

	m_actionGroups["annotate"]->add(Gtk::Action::create("edit_background",
	                _("Edit background segment"), _("Edit background properties")),
	                sigc::bind<string,bool>(sigc::mem_fun(*this, &AnnotationEditor::onMenuAction), "edit_background",
	                true, false, ""));

	m_actionGroups["annotate"]->add(Gtk::Action::create("delete_background",
	        _("Delete background segment"),
	        _("Delete background segment")), sigc::bind<string, bool>(
	        sigc::mem_fun(*this, &AnnotationEditor::onMenuAction),
	        "delete_background", true, false, ""));

	m_actionGroups["edit"] = m_activeView->getActionGroup("edit");

	m_actionGroups["file"]->add(Gtk::Action::create("filemode",
	        _("Edit / Read only mode"), _("Edit / Read only mode")),
	        Gtk::AccelKey("F6"), sigc::bind<string, bool>(sigc::mem_fun(*this,
	                &AnnotationEditor::onMenuAction), "filemode", false, false, ""));

	m_actionGroups["display"] = Gtk::ActionGroup::create("display");
	m_actionGroups["display"]->add(Gtk::Action::create("hideTags",
	        _("Show / _Hide tags"), _("Show / Hide tags")),
	        Gtk::AccelKey("F7"), sigc::bind<string, bool>(sigc::mem_fun(*this,
	                &AnnotationEditor::onMenuAction), "hideTags", false, false, ""));
	m_actionGroups["display"]->add(
	        Gtk::Action::create("highlight", _("Enable / Disable hi_ghlight"),
	                _("Enable / Disable highlight")),
	        Gtk::AccelKey("F11"),
	        sigc::bind<string, bool>(sigc::mem_fun(*this,
	                &AnnotationEditor::onMenuAction), "highlight", false, false, ""));

	m_actionGroups["display"]->add(Gtk::Action::create("display",
	        _("Enable / Disable _dual display"),
	        _("Enable / Disable dual display")), Gtk::AccelKey("F12"),
	        sigc::bind<string, bool>(sigc::mem_fun(*this,
	                &AnnotationEditor::onMenuAction), "display", false, false, ""));

	if (m_nbTracks<=1)
		m_actionGroups["display"]->get_action("display")->set_sensitive(false) ;

	/*  define UI info for signal & annotate action groups */
	configureAnnotationUI();
}

//**LG3
void AnnotationEditor::set_input_language(InputLanguage *il)
{
	if (il == NULL)
	{
		il = InputLanguageHandler::get_input_language_by_shortcut(
		        DEFAULT_LANGUAGE);
		TRACE_D << "!!! Editor:> set language:> NULL" << endl;
	}
	m_ilang = il;
	//m_ilm.set_current_input_language(il);
	m_signalLanguageChange.emit(il);
}

void AnnotationEditor::onChangeInputLanguage(int offset)
{
	m_ilang = InputLanguageHandler::change_current_language(m_ilang, offset);
	m_signalLanguageChange.emit(m_ilang);
}

/**
* Reconfigure UI when model conventions have changed
*/

void AnnotationEditor::onUpdateConventions()
{
//	m_synchroTypes = m_dataModel.getMainstreamTypes();
	configureSynchrotypes() ;

	// adjust some layout parameters that may be forced in annotation conventions
	guint i;
	for (i = 0; i < m_synchroTypes.size(); ++i)
	{
		string key = m_synchroTypes[i] + ",label,show";
		const string& val = m_dataModel.conventions().getConfiguration("layout," + key);
		if (!val.empty())
			m_configuration["AnnotationLayout," + key] = val;
	}

	configureAnnotationUI();
}

void AnnotationEditor::configureSynchrotypes()
{
	m_synchroTypes.clear() ;

	StringOps stypes(m_configuration["Signal,synchrotypes"]) ;
	vector<string> vstypes ;
	stypes.split(vstypes, ";", true) ;

	if (vstypes.empty())
	{
		m_synchroTypes = m_dataModel.getMainstreamTypes();
		return ;
	}

	vector<string> main = m_dataModel.getMainstreamTypes();
	vector<string>::iterator it_main ;
	vector<string>::iterator it_sync ;
	for ( it_main=main.begin(); it_main!=main.end(); it_main++ )
	{
		if ( find(vstypes.begin(), vstypes.end(), *it_main) != vstypes.end() )
			m_synchroTypes.push_back(*it_main) ;
	}
}

/**
 * configure annotation menus vs current annotation conventions
 */
// TODO -> gerer le cas ou on n'affiche pas l'un des types (eg. section) -> pas de menu !!
void AnnotationEditor::configureAnnotationUI()
{
	string info = "<ui>"
		"  <menubar name='MenuBar'>"
		"    <menu action='AnnotateMenu'>";

	vector<string> v;
	vector<string>::iterator itv;

	const vector<string>& types = m_dataModel.getMainstreamTypes();
	vector<string>::const_reverse_iterator it;
	for (it = types.rbegin(); it != types.rend(); ++it)
	{
		if ( *it != m_dataModel.mainstreamBaseType() )
			info += "      <menuitem action='annotate_new_" + *it + "'/>" ;
	}

	//TODO use configuration or find another menu to this option
	info += "      <menuitem action='annotate_new_timestamp'/>" ;

	if (!m_dataModel.conventions().getConfiguration(
	        "transcription_graph,qualifier_event").empty())
	{
		info += "      <separator/>";
		info += "      <menuitem action='annotate_new_event'/>";
	}

	if (!m_dataModel.conventions().getConfiguration(
	        "transcription_graph,qualifier_entity").empty())
	{
		info += "      <separator/>";
		info += "      <menuitem action='annotate_new_entity'/>";
	}

	if (!m_dataModel.conventions().getConfiguration("transcription_graph,other").empty())
	{
		vector<string> v;
		vector<string>::iterator it2;
		StringOps(m_dataModel.conventions().getConfiguration(
		        "transcription_graph,other")).split(v, ",;");

		info += "      <separator/>";
		for (it2 = v.begin(); it2 != v.end(); ++it2)
			info += "      <menuitem action='annotate_new_" + *it2 + "'/>";
	}

	if (!m_dataModel.hasAnnotationGraph("background"))
	{
		info += "      <separator/>";
		info += "      <menuitem action='annotate_new_background'/>";
	}

	info += "    </menu>"
		"  </menubar>"
		"</ui>";

	m_UIInfo["annotate"] = info;
	Glib::RefPtr<Gtk::ActionGroup> nulgroup =
	        (Glib::RefPtr<Gtk::ActionGroup>) 0;
	signalUpdateUI().emit("annotate", nulgroup);
}

/*
 * configure editor
 */
void AnnotationEditor::setOptions(Parameters& options, bool reload)
{
	globalOptions = options;

#ifdef __APPLE__
	if (reload)
	fontInitialized = true;
#endif

	m_configuration = options.getParametersMap("AnnotationEditor");
	m_modelCfg = options.getParametersMap("DataModel");
	m_colorsCfg = options.getParametersMap("Display");

	// Get user workdir
	string workdir = options.getParameterValue("General", "start,workdir");

	if (!workdir.empty())
	{
		string dir = FileInfo(workdir).dirname();
		dir = FileInfo(dir).realpath();
		if (dir != "")
		{
			string name = FileInfo(workdir).Basename();
			workdir = FileInfo(dir).join(name);
			FileInfo info(workdir);
			if (info.exists())
			{
				if (info.canWrite())
					m_workdir = workdir;
			}
			else
			{
				if (g_mkdir(workdir.c_str(), 0755) == 0)
					m_workdir = workdir;
				else
					MSGOUT << "Could not create user workdir " << workdir
					        << endl;
			}
		}
	}

	m_threads = (options.getParameterValue("AnnotationEditor", "threads,protect") == "true");

	/* configure idle time delay for activity watcher (180 secs by default) */
	int delay = atoi(options.getParameterValue("AnnotationEditor", "Activity,delay").c_str());
	if (delay <= 0)
		delay = 180;
	m_activityWatcher->setIdleTimeDelay(delay);
	TRACE << "Inactivity delay set to " << m_activityWatcher->getIdleTimeDelay() << endl;

	m_configuration["scribe"] = (options.getParameterValue("General", "User,acronym"));
	m_configuration["control"] = (options.getParameterValue("General", "Transcription,control"));
	m_modelCfg["config_dir"] = options.getParameterValue("General", "start,config");
	m_configuration["audio_dir"] = options.getParameterValue("General", "start,audiodir");

	// Initialize conventions, trancription languages, file state only FIRST calling only
	Log::err() << " CONFIG DIR = " << m_modelCfg["config_dir"] << endl << flush;
	Log::err() << " Default conv = " << m_modelCfg["Default,convention"] << endl << flush;

	if (!reload)
	{
		DataModel::initEnviron(m_modelCfg["config_dir"]);
		m_configuration["lang"] = m_modelCfg["Default,lang"];
		m_lang = m_configuration["lang"];
		setMode("EditMode");
		m_dataModel.setConventions(m_modelCfg["Default,convention"], m_lang);
	}

	// Some map parameters can have been modified, reset it each time options are set
	onUpdateConventions();

	// Connect autosave mecanism
	if (!consultationMode && isEditMode())
	{
		m_autosave.disconnect();
		int autosave_period = atoi(m_configuration["Autosave,period"].c_str());
		if (autosave_period == 0)
			autosave_period = 180;
		//  m_autosave = Glib::signal_timeout().connect(sigc::mem_fun (*this, &AnnotationEditor::autosaveFile), autosave_period*1000);
		m_autosave = Glib::signal_timeout().connect(sigc::bind<bool>(sigc::mem_fun(*this, &AnnotationEditor::autosaveFile), true), autosave_period * 1000);
	}

	// Deals with track colors at FIRST calling only
	if (!reload)
	{
		vector<string> v2;
		vector<string>::iterator it_col;
		StringOps(m_configuration["Signal,trackcolor"]).split(v2, " ,;");

		for (it_col = v2.begin(); it_col != v2.end(); ++it_col)
		{
			unsigned long pos = it_col->find("=");
			if (pos != string::npos)
			{
				string type = it_col->substr(0, pos);
				m_trackColor[type] = it_col->substr(pos + 1);
			}
		}
	}
}

void AnnotationEditor::setMode(const string& mode, bool force)
{
	string editmode = "BrowseMode";
	if (strncasecmp(mode.c_str(), "edit", 4) == 0)
		editmode = "EditMode";

	// lock when applying same mode
	// TODO clean similar call
	if (m_configuration["mode"] == editmode && !force)
		return;

	m_configuration["mode"] = editmode;

	std::map<std::string, std::string>::iterator itp;
	string prefix = editmode + ",";
	for (itp = m_configuration.begin(); itp != m_configuration.end(); ++itp ) {
		if (itp->first.compare(0, prefix.length(), prefix) == 0 ) {
			string key=itp->first.substr(prefix.length());
			m_configuration[key] = m_configuration[itp->first];
		}
	}

	m_synchroInterval = StringOps(m_configuration[editmode
	        + ",synchro_force_interval"]).toFloat();
	if (m_synchroInterval <= 0)
		m_synchroInterval = 0.300;

	m_highlightMode = getConfigurationHighlightMode();


	TRACE << "Default View mode = " << m_configuration["stereo,viewmode"]    << endl;
	m_defaultViewMode = -1; // default = merged
	if (m_configuration["stereo,viewmode"] == "dual")
		m_defaultViewMode = -2;
	else if (m_configuration["stereo,viewmode"] == "track1")
		m_defaultViewMode = 0;
	else if (m_configuration["stereo,viewmode"] == "track2")
		m_defaultViewMode = 1;

	for (guint i = 0; i < m_textView.size(); ++i)
	{
		m_textView[i]->configureDisplay(editmode, m_modeStereo);
	}

	if (m_signalView)
		m_signalView->setEditable(editmode == "EditMode");
}


void AnnotationEditor::setLocked(bool lock)
{
	if (lock) {
		setSensitiveViews(false);
		signalEditModeChanged().emit("BrowseMode");
	} else {
		setSensitiveViews(true);
		signalEditModeChanged().emit(m_configuration["mode"]);
	}

}

#ifdef TODEL
void AnnotationEditor::lockEditionMode(bool lock, bool hide_cursor)
{
	if (lock) {
		setSensitiveViews(false);
		signalEditModeChanged().emit("BrowseMode");
	} else {
		setSensitiveViews(m_configuration["mode"]=="EditMode"));
		signalEditModeChanged().emit(m_configuration["mode"]);
	}
	/*
	// lock edition
	if (lock)
	{
		if (hide_cursor)
			setSensitiveViews(false);
		m_lastEditMode = getEditMode();
		if (m_lastEditMode != "BrowseMode")
		{
			setMode("BrowseMode");
			signalEditModeChanged().emit("BrowseMode");
		}
	}
	// restore old edition mode
	else if (!m_lastEditMode.empty() && m_lastEditMode != getEditMode())
	{
		bool can_write = FileInfo(m_agFilename).canWrite();
		string mode;
		if (m_lastEditMode == "EditMode" && can_write)
			mode = m_lastEditMode ;
		else
			mode = "BrowseMode";

			setSensitiveViews(true) ;

		setMode(mode, true);
		signalEditModeChanged().emit(mode);
	}
	else if (m_lastEditMode.empty())
	{
		bool can_write = FileInfo(m_agFilename).canWrite();
		string mode;
		if (can_write)
		{
			mode = "EditMode";
			setSensitiveViews(true);
		}
		else
		{
			mode = "BrowseMode";
			if (hide_cursor)
				setSensitiveViews(false);
		}
		setMode(mode);
		signalEditModeChanged().emit(mode);
	}
	*/
}

#endif

void AnnotationEditor::setOption(const string& option, const string& value)
{
	m_configuration[option] = value;
	if (option == "highlight_current" && value == "none")
	{
		// clear highlight in text views
		for (guint i = 0; i < m_textView.size(); ++i)
			m_textView[i]->clearHighlight();
	}
}

/*
 *  get exported action groups
 */
const Glib::RefPtr<Gtk::ActionGroup>& AnnotationEditor::getActionGroup(
		const string& groupname)
{
	if (m_actionGroups.find(groupname) != m_actionGroups.end())
	{
		if (groupname == "video_signal" && !videoManager)
			return emptyGroup;
		else
			return m_actionGroups[groupname];
	}
	else
		MSGOUT << " : No action group " << groupname << endl;

	return emptyGroup;
}

/* destructor */
AnnotationEditor::~AnnotationEditor()
{
	// inhibate synchro for avoiding signal reception by zombie
	inhibateSynchro(true) ;

	if (videoPlayer)
		delete (videoPlayer);
	if (videoFrameBrowser)
		delete (videoFrameBrowser);
	if (videoManager)
		delete (videoManager);
	if (progressWatcher)
		delete (progressWatcher);
}

/*
 *-----------------------------------------------------------------------------
 *
 *  edited file handling
 *
 *-----------------------------------------------------------------------------
 */

void AnnotationEditor::setFileName(const std::string& filename)
{
	if (m_agFilename.empty())
		m_defaultFilename = filename;
	else
		m_agFilename = filename;
}

void AnnotationEditor::showStatus(const string& st)
{
	if (m_statusBar != NULL)
		if (st.empty())
			m_statusBar->pop();
		else
			m_statusBar->push(st);
}

/*==============================================================================*
 *										       									*
 *									LOADING										*
 *   																			*
 *==============================================================================*/

bool AnnotationEditor::loadFile(string filepath,
        std::vector<string>& signalFiles, bool streaming,
        bool p_lockForced, int p_reportLevel)
{
	logLoadingDone = false ;

	m_elapsed.reset();
	m_elapsed.start();

	bool newTrans;
	bool multiAudio;

	consultationMode = p_lockForced;

	// by default, set a total report level
	if (p_reportLevel == -1)
		reportLevel = 1;
	else
		reportLevel = p_reportLevel;

	if (!filepath.empty())
		newTrans = false;
	else
		newTrans = true;

	m_newFile = newTrans;

	if (signalFiles.size() > 1)
		multiAudio = true;
	else
		multiAudio = false;

	bool success = loadFile(filepath, signalFiles, newTrans, multiAudio,
	        streaming);
	if (!success)
	{
		TRACE << "(!) Loading failure... prepare display" << std::endl;
		logLoading(false, false);
		signalStatusBar().emit(TRANSAG_VERSION_NO);
	}
	return success;
}

bool AnnotationEditor::loadFile(string filename,
        std::vector<string>& signalFiles, bool newTranscription,
        bool multiAudio, bool streaming)
{
	m_signalStatusBar.emit(std::string(_("Loading file...")));

	// Lock edition for correct view update. Will be release by
	// terminateDisplayLoading method
	setLocked(true);

	TRACE << "loadFile:> " << filename << " | audio-size="
	        << signalFiles.size() << " | new(" << newTranscription
	        << ") - multiAudio(" << multiAudio << ") - streaming(" << streaming
	        << ")" << std::endl;

	bool selectSignalDialog = false;
	bool recovered = false;

	//> CONSTRAINSTS

	m_dataModel.setInhibUndoRedo(true);

	// If existing transcription but file doesn't exist
	if (!newTranscription && !FileInfo(filename).exists())
	{
		closeFile(true, false);
		return false;
	}

	// new trans and no audio
	if (newTranscription && !FileHelper::existFiles(signalFiles))
	{
		closeFile(true, false);
		return false;
	}

	SignalConfiguration& signalCfg = m_dataModel.getSignalCfg();

	m_agFilename = filename;
	TRACE << "\tm_agFilename= " << m_agFilename << " is_new= " << newTranscription << endl;

	TRACE << "\n-------------------------------------((o)) CHECKING FILE NAME" << endl;
	m_signalStatusBar.emit(std::string(_("Checking file name...")));

	//> Compute default filename of new transcription
	if (signalFiles.empty())
		makeDefaultFilename(m_agFilename, "");
	else
		makeDefaultFilename(m_agFilename, signalFiles[0]);

	TRACE << "\t(o) DefaultFileName [" << m_agFilename << "]" << endl;

	//> Prepare Autosave

	TRACE << "\n-------------------------------------((o)) CHECKING AUTOSAVE"  << endl;
	m_signalStatusBar.emit(std::string(_("Checking autosave...")));

	//> SEE FOR RECOVERING NEEDS
	recovered = checkAutosave();
	// keep real file name in case we're about to recover from autosave
	string default_fileName = m_agFilename;
	if (recovered)
		m_agFilename = m_autosavePath;
	TRACE << "\t(o) Recovering from autoSave [" << recovered << "] -> " << m_agFilename << endl;

	//> OPENING EXISTING TRANSCRIPTION FILE
	//  will update conventions, default name, and signal map
	if (!newTranscription && !m_agFilename.empty())
	{
		TRACE << "\n----------------------------------((o)) OPENING DATA MODEL" << endl;
		m_signalStatusBar.emit(std::string(_("Loading model...")));

		bool ok = openDataModel(m_agFilename);
		if (!ok)
		{
			TRACE << "\t(o) Model load [0] : aborting." << endl;
			return false;
		}
	}

	TRACE << "\t(o) Model load [1] : found "
	        << signalCfg.getAudioIdPaths().size() << " signals" << endl;

	//> FOR NEW TRANS or IF NO LANGUAGE, DISPLAY CONVENTIONS DIALOG
	bool updateConventionLang = (m_lang.empty() && !newTranscription) ;
	if (newTranscription || updateConventionLang)
	{
		TRACE << "\n-------------------------------------((o)) UPDATING CONVENTION" << endl;
		m_signalStatusBar.emit(std::string(_("Checking conventions...")));

		string forced_convention = "";
		string forced_convention_name = "";

		// format import
		if (!newTranscription && !dataModel().isTAGformat(m_fileFormat))
		{
			string convention_path = m_dataModel.getConversionConventionFile(m_fileFormat);
			if (!convention_path.empty())
			{
				forced_convention = convention_path;
				forced_convention_name = Glib::path_get_basename(convention_path);
			}
		}

		THREADS_ENTER
		bool ok = langAndConventionsDlg(newTranscription, m_configuration["lang"], m_configuration["convention_id"], forced_convention_name);
		THREADS_LEAVE

		if (!ok)
		{
			m_loading_cancelled = true;
			closeFile(true, false);
			TRACE << "\t(o) Loading canceled by user." << endl;
			return false;
		}

		m_lang = m_configuration["lang"];

		if (!forced_convention.empty())
			m_configuration["convention_id"] = forced_convention_name ;

		if (updateConventionLang)
		{
			m_dataModel.setConventions(m_configuration["convention_id"], m_lang) ;
			onUpdateConventions() ;
		}

		TRACE << "\t(o) Update convention [1]" << endl;
	}

	//> FOR EXISTING TRANSCRIPTION FILE
	if (!newTranscription && !m_agFilename.empty())
	{
		TRACE   << "\n-------------------------------------((o)) LOADING EXISTING TRANSCRIPTION" << endl;
		m_signalStatusBar.emit(std::string(_("Loading existing transcription...")));

		signalCfg.print();

		//> If no signal file, display warning
		if (signalCfg.getAudioIdPaths().empty() && !m_rtsp)
		{
			string msg = string(_("No signal file for ")) + m_agFilename;
			THREADS_ENTER
			dlg::warning(msg);
			THREADS_LEAVE
		}
		else if (!m_rtsp && !signalCfg.hasVideo())
		{
			TRACE << "\t(o) Checking signal " << endl;

			bool ok = checkSignalFiles("audio");
			if (!ok)
			{
				TRACE << "\t\t signals [0] :> selection..." << endl;
				bool continue_ = selectSignalFiles();
				if (!continue_)
				{
					m_loading_cancelled = true;
					TRACE  << "\t\t signals selection: canceled by user - no signal" << endl;
					closeFile(true, false);
					return false;
				}
				else
				{
					TRACE << "\t\t signals selection [1]" << endl;
					selectSignalDialog = true;
				}
			}
			else
				TRACE << "\t(o) signals checked [1]" << endl;
		}
		else if (!m_rtsp)
		{
			TRACE << "\t(o) Checking video signal " << endl;
			bool ok = checkSignalFiles("video");
			if (!ok)
			{
				TRACE << "\t\t signals video [0] :> selection..." << endl;
				bool continue_ = selectVideoFile();
				if (!continue_)
				{
					m_loading_cancelled = true;
					TRACE << "\t\t signals video selection: canceled by user" << endl;
					closeFile(true, false);
					return false;
				}
				else
					TRACE << "\t\t signals video selection [1]" << endl;
			}
			else
				TRACE << "\t(o) signals video checked [1]" << endl;

			TRACE << "\t(o) Checking audio signal " << endl;
			ok = checkSignalFiles("audio");
			if (!ok)
				signalCfg.setVideoStandAlone(true);
			TRACE << "\t(o) signals audio checked => video only [" << signalCfg.isVideoStandAlone() << "]" << endl;
		}

		//> get a list of audio files
		signalCfg.print();
		signalFiles = fromSignalsToFiles();
	}

	TRACE << "\n-------------------------------------((o)) INITIALISING SIGNAL WIDGET" << endl;
	m_signalStatusBar.emit(std::string(_("Initializing signal view...")));

	//> Prepare AudioWidget for found files
	bool ok = addSignalView(signalFiles);
	if (!ok)
	{
		TRACE << "\t\t widget adding [0] : aborting." << endl;
		closeFile(true, false);
		return false;
	}

	//> NEW TRANSCRIPTION: prepare model and audio
	if (newTranscription)
	{
		TRACE  << "\n-------------------------------------((o)) SETTING NEW TRANSCRIPTION" << endl;
		m_signalStatusBar.emit(std::string(_("Setting new transcription sheet...")));
		prepareNewTranscription(signalFiles);
	}

	//> for new file or import, let's set corpus information
	if (newTranscription || m_fileFormat != "TransAG")
		actualizeCorpusInformation();

	TRACE << "\n-------------------------------------((o)) SETTING VIEW"
	        << endl;
	m_signalStatusBar.emit(std::string(_("Initializing editor interface...")));

	// restore real name in case we're loading from recovered file
	m_agFilename = default_fileName;

	//> If existing tag transcription and audio file(s) newly associated,
	//  we'll remember it and save file at loading end
	//  (don't do it if user choosed no signal)
	if (!newTranscription && selectSignalDialog
			&& dataModel().isTAGformat(m_fileFormat)
			&& FileInfo(filename).canWrite()
		    && (signalCfg.getEmptySignalTracks() == 0) )
		pathHintActualized = true ;

	//> Prepare View Widget
	initializeView(newTranscription, recovered);

	TRACE << "\n-------------------------------------((o)) Done.\n\n" << endl;

	m_dataModel.setInhibUndoRedo(false);

	m_signalStatusBar.emit(TRANSAG_VERSION_NO);
	return true;
}

void AnnotationEditor::initializeView(bool newTranscription, bool recovered)
{
	//> stereo
	if (m_dataModel.getNbTracks() > 1)
	{
		//view for track 2
		addTextView(1);
		//view for merged tracks
		addTextView(0);
	}
	//> simple channel
	else
		m_defaultViewMode = -1; // mono -> merged view !!

	string speech_type = m_dataModel.getAGSetProperty("speech_type");
	StringOps(speech_type).toLower();
	bool cts_mode = (speech_type == "cts" || speech_type == "h5");

	if ( m_modeStereo || cts_mode )
	{
		// apply Layout for stereo mode
		// TODO -> faire plutot section AnnotationLayout,stereo à part
		string prefix="AnnotationLayout,";
		std::map<std::string, std::string>::iterator itc;
		for (itc=m_configuration.begin(); itc != m_configuration.end(); ++itc )
		{
			if (itc->first.length() > prefix.length()
					&& (itc->first.compare(0, prefix.length(), prefix) == 0 )
					&& (itc->first.compare(itc->first.length()-6, 6, "stereo") == 0) ) {
				string key = itc->first.substr(0, itc->first.length()-7);
				m_configuration[key] = itc->second;
			}
		}
		m_configuration["Signal,tracks"] = m_configuration["Signal,tracks,stereo"] ;
	}

	// -- load track display option
	m_trackTypes.clear() ;
	m_trackTypes = m_dataModel.getAnnotationTypes(true) ;
	StringOps(m_configuration["Signal,tracks"]).split(m_visibleTracks, " ,;") ;

	//! new file : in transcription input mode -> disable signal-text synchro/
	if (newTranscription || FileInfo(m_agFilename).canWrite())
		setMode("EditMode");
	else
		setMode("BrowseMode");

	// get prev. activity time for file
	string wid = m_dataModel.getTranscriptionProperty("wid");
	if (!wid.empty())
	{
		time_t t = (time_t) strtol(wid.c_str(), NULL, 16);
		m_activityWatcher->setActivityTime(t);
	}

	//> Configure display of editor for each view
	string speller_enabled = m_configuration["Speller,enabled"];
	string speller_directory;
	if (speller_enabled == "true")
		speller_directory = m_configuration["Speller,directory"];
	else
		speller_directory = "nospell";

	for (guint i = 0; i < m_textView.size(); ++i)
	{
		m_textView[i]->configure(speller_directory, m_lang, m_configuration["mode"], m_modeStereo);
		m_textView[i]->set_modified((newTranscription || recovered));
	}

	// force display of tracks (but not elements inside tracks)
	updateTrack("", "", 1, false, -1, false);

	m_activeViewMode = -3; // to force update view

	// set focus on new annotation view
	Glib::signal_idle().connect(sigc::mem_fun(*this, &AnnotationEditor::setFocusWhenIdle));

	// switch interline height regarding transcription language
	changeInterline();

	//<!>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> EDITOR IS READY TO DISPLAY DATAS
	/** If signal view exists, signal ready will be emit when tracks are displayed,
	 * 	then m_loaded attribute will be changed to TRUE
	 *	If no signal view, signal ready will be emit after first setDefaultViewMode
	 *	then m_loaded attribute will be changed to TRUE
	 **/
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<!>

	//> Actualize actions regarding conventions
	actualizeActions();

	// TODO conf ?
	if (m_fileFormat == "SGML")
		setMode("BrowseMode");

	//> -- Actualize the confidence display mode
	bool confidence = (m_configuration["Confidence,display"] == "true") ;
	setWithConfidence(confidence);

	//> -- Actualize the sensitive property
	setSensitiveViews(false);
	setSensitiveSignalView(false);
}

void AnnotationEditor::prepareNewTranscription(const std::vector<string>& signalFiles)
{
	if (!m_signalView)
		return;

	string default_generalCorpus = m_modelCfg["Default,corpus"];
	//> No genreal value set (classic behaviour)
	//   => means use corpus value defined in convention file
	//  <CAUTION>: in both case all information will be set in
	//			 "actualizeCorpusInformation" method
	if (default_generalCorpus.empty())
		m_dataModel.initAGSet();
	//> otherwise, force using general default value for corpus
	//  (set in transcriberAG.rc)
	else
		m_dataModel.initAGSet(default_generalCorpus);

	//> Set conventions data
	m_dataModel.setConventions(m_configuration["convention_id"], m_lang) ;
	onUpdateConventions() ;

	// prepare to get signal data
	m_dataModel.getSignalCfg().clear();

	//> for each FILES
	std::vector<string>::const_iterator it;
	for (it = signalFiles.begin(); it != signalFiles.end(); it++)
	{
		Explorer_filter* filter = Explorer_filter::getInstance();
		bool video = filter->is_video_file(*it);

		int nbTrack;
		if (m_rtsp)
			nbTrack = m_signalView->getNbSignalTracks();
		else
		{
			IODevice* device = Guesser::open((*it).c_str());
			if (device)
			{
				nbTrack = device->m_info()->audio_channels;
				device->m_close();
				delete (device);
			}
		}

		TRACE << "********** " << nbTrack << endl;

		//> 1) ADD AUDIO SIGNALS
		std::vector<string> sigid = m_dataModel.addSignal(*it, "audio",
		        m_signalView->getAudioTrackFormat(0),
		        m_signalView->getAudioTrackEncoding(0), nbTrack);

		std::vector<string>::iterator it_sig;
		SignalConfiguration& signalCfg = dataModel().getSignalCfg();

		for (it_sig = sigid.begin(); it_sig != sigid.end(); it_sig++)
		{
			TRACE << "\t\t\t signal= " << *it_sig << " - path=" << *it
			        << " - nbtracks=" << nbTrack << endl;
			FileInfo info(*it);
			string suffix = m_modelCfg["metadata,meta_suffix"];
			if (suffix.empty())
				suffix = ".info";
			info.setTail(suffix);
			if (info.exists())
			{
				m_dataModel.loadSignalInfo(*it_sig, info.path(),
				        m_modelCfg["metadata,items"]);
				TRACE << "\t\t\t\t info-file= " << info.path() << endl;
			}
		}

		//> 2) eventually ADD VIDEO SIGNAL for special video basic mode
		//  remarks: in current version video signal won't be used by annotations
		if (video)
			m_dataModel.addSignal(*it, "video", m_signalView->getAudioTrackFormat(0), m_signalView->getAudioTrackEncoding(0), 1) ;
	}

	//TODO do it for each chanel ?
	float slen = m_signalView->signalLength();
	m_dataModel.setSignalDuration(slen);

	// initialize transcription graph
	m_dataModel.initAnnotationGraphs("", m_lang, m_configuration["scribe"]);
}

void AnnotationEditor::actualizeCorpusInformation()
{
	std::string default_conventionCorpusName =
	        m_dataModel.conventions().getCorpusName();
	std::string default_conventionCorpusVersion =
	        m_dataModel.conventions().getCorpusVersion();

	std::string default_generalCorpusName = m_modelCfg["Default,corpus"];
	std::string default_generalCorpusVersion =
	        m_modelCfg["Default,corpus_version"];

	//> Use convention information
	if (default_generalCorpusName.empty())
	{
		if (!default_conventionCorpusName.empty())
			m_dataModel.setCorpusName(default_conventionCorpusName);
		else
			m_dataModel.setCorpusName(m_fileFormat);

		if (!default_conventionCorpusVersion.empty())
			m_dataModel.setCorpusVersion(default_conventionCorpusVersion);
		else
			m_dataModel.setCorpusVersion("1.0");
	}
	//> otherwise, keep the general corpus name and add version data
	else
	{
		m_dataModel.setCorpusName(default_generalCorpusName);
		m_dataModel.setCorpusVersion(default_generalCorpusVersion);
	}
}

bool AnnotationEditor::selectSignalFiles()
{
	//TODO use CreateNewTranscriptionDialog for better look & better behaviour

	int cpt = 1;
	bool close = false;
	bool without = false;
	string stereo_path = "";

	SignalConfiguration& signalCfg = m_dataModel.getSignalCfg();
	std::map<string, string> signalIds = signalCfg.getAudioIdPaths();
	std::map<string, string>::iterator it;

	bool force_mono = (signalCfg.getNbSignals("") == 1);
	int max_channel = signalCfg.getNbSignals("") ;
	int current_nb_channel = 0;

	for (it = signalIds.begin(); it != signalIds.end() && !close && !without; it++)
	{
		int available = max_channel - current_nb_channel;
		force_mono = (available == 1);

		string current_id = it->first;
		Log::trace() << "signal id: " << it->first << std::endl;
		Log::trace() << "signal path: " << it->second << std::endl;

		//> If previous was stereo, means we're not in multi-audio mode
		//  current signal must be the second of stereo file
		if (!stereo_path.empty())
		{
			//TODO check remaining tracks
			signalCfg.changePath(current_id, stereo_path);
			signalCfg.changeChannel(current_id, 2);
		}
		//> Otherwise, let's search the new path
		else
		{
			string msg = string(_("Select signal file") + number_to_string(cpt)
			        + _(" for ")) + m_agFilename;
			THREADS_ENTER
			string name = it->second;
			name = dlg::selectAudioFile(close, name, msg, getTopWindow(), force_mono, m_configuration["audio_dir"]);
			Log::trace() << "signal selected: " << name << std::endl;
			signalCfg.changePath(current_id, name);
			if (name == "")
			{
				Log::trace() << "\t\t\t\t without signal." << std::endl;
				without = true;
				signalCfg.setEmptySignalTracks(signalIds.size());
			}
			else
			{
				IODevice* device = Guesser::open(name.c_str());
				if (device)
				{
					int current_channels = device->m_info()->audio_channels ;
					signalCfg.changeChannel(current_id, current_channels);

					if (force_mono || current_channels == 1)
						Log::trace() << "mono detected for signal " << name << std::endl;
					else if (current_channels == 2)
					{
						Log::trace() << "stereo detected for signal " << name << std::endl;
						stereo_path = name;
					}
					current_nb_channel = current_nb_channel + current_channels ;

					device->m_close();
					delete (device);
				}
				else
					Log::trace() << "unable to open device for " << name << std::endl;
			}
		}

		THREADS_LEAVE
		cpt++;
	}

	signalCfg.print();

	return !close;
}

bool AnnotationEditor::selectVideoFile()
{
	int cpt = 1;
	bool close = false;
	bool without = false;
	string stereo_path = "";

	SignalConfiguration& signalCfg = m_dataModel.getSignalCfg();
	std::map<string, string> signalIds = signalCfg.getVideoIdPaths();
	std::map<string, string>::iterator it;

	for (it = signalIds.begin(); it != signalIds.end() && !close && !without; it++)
	{
		string msg = string(_("Select video file for ")) + m_agFilename;
		const string& sigid = it->first;
		string path = it->second;
		path = dlg::selectVideoFile(close, path, msg, getTopWindow());
		if (path == "")
		{
			without = true;
			signalCfg.clear();
		}
		else
			signalCfg.changePath(sigid, path);
		cpt++;
	}
	return !close;
}

bool AnnotationEditor::checkSignalFiles(string mode)
{
	int errors = 0;
	SignalConfiguration& signalCfg = m_dataModel.getSignalCfg();
	std::map<string, string> signalIds = signalCfg.getIdPaths();

	std::map<string, string>::iterator it;
	// -- For each signal found in model
	for (it = signalIds.begin(); it != signalIds.end(); it++)
	{
		TRACE << "\t\t\t checking [" << it->first << " - " << it->second << "]" << endl;

		const string& sigid = it->first;
		string signalFilename = it->second;
		string path = signalFilename;
		string dir = FileInfo(signalFilename).dirname();
		string myType = "";

		// -- check signal type
		if (signalCfg.isAudio(sigid))
			myType = "audio";
		else if (signalCfg.isVideo(sigid))
			myType = "video";

		std::string stereo_path = "" ;
		int channel_count = 0;

		if (myType == mode)
		{
			//> previous was correct & stereo, the second signal is the second track
			if (!stereo_path.empty())
			{
				signalCfg.changePath(it->first, stereo_path);
				signalCfg.changeChannel(it->first, 2);
				stereo_path = "";
			}
			//> otherwise proceed
			else
			{
				//> if signal file name is relative, let's find that signal
				if (m_agFilename != "" && (signalFilename == FileInfo(signalFilename).Basename()))
				{
					// location of signal file not defined in URL -> try to locate it
					dir = m_dataModel.getSignalProperty(sigid, "path_hint");
					path = "";
					// look in default directory
					if (!dir.empty())
						path = Explorer_filter::lookForMediaFile(dir, signalFilename, myType);
					// look for signal file in annot file dir
					if (path.empty())
					{
						dir = FileInfo(m_agFilename).dirname();
						path = Explorer_filter::lookForMediaFile(dir, signalFilename, myType);
					}
					// look in default signal directory if set in configuration
					if (path.empty() && m_configuration["audio_dir"] != "")
						path = Explorer_filter::lookForMediaFile(m_configuration["audio_dir"], signalFilename, myType);
					// let relative path for the moment
					if (path.empty())
						path = signalFilename;
				}

				string final_path = path;

				// -- The path we computed is wrong (no previous cases have match)
				if (!FileInfo(path).exists())
					errors--;
				else
				{
					// resolve real path and symlink
					final_path = real_regular_path(path);
					if (final_path.empty())
					{
						final_path = path;
						errors--;
					}
					// symlink is OK, check the channel number
					else
					{
						IODevice* device = Guesser::open(final_path.c_str()) ;
						if (device)
						{
							int current_channels = device->m_info()->audio_channels ;
							signalCfg.changeChannel(it->first, current_channels);

							if (myType.compare("audio") == 0)
								channel_count = channel_count + current_channels ;

							stereo_path = final_path;
							device->m_close();
							delete (device);
						}
					}
				}

				// -- Actualize signal configuration
				signalCfg.changePath(it->first, final_path);
				TRACE << "\t\t\t\t result [" << it->first << " - " << it->second << "]" << endl;

				/*** single signal ***/
				if ( (channel_count > signalCfg.getNbSignals("")) && !signalCfg.isSingleSignal() )
				{
					dlg::error(_("Number of media channels is greater than the one required by file, please choose manually"));
					return false;
				}
			} // end mono case
		} // end correct mode
	} // end for each signal
	if (errors < 0 || signalIds.empty())
	{
		TRACE << "\t\t\t remaining undefined -> [" << errors << "]" << endl;
		return false;
	}
	else
	{
		TRACE << "\t\t\t all checked successfully." << endl;
		return true;
	}
}

bool AnnotationEditor::openDataModel(string filename)
{
	try
	{
		bool validate = ( m_configuration["Validation,activated"] != "false" ) ;
		m_dataModel.activateModelChecker(validate);

		const string& format = m_dataModel.guessFileFormat(filename);
		SignalConfiguration& cfg = dataModel().getSignalCfg();

		if (format.empty())
		{
			//			THREADS_ENTER
			dlg::warning(_("Unknown file format"));
			//			THREADS_LEAVE
			closeFile(true, false);
			//			showStatus ("");
			return false;
		}

		m_fileFormat = format;
		TRACE << " ...reading " << filename << " with format " << m_fileFormat
		        << endl << endl;

		if (!dataModel().isTAGformat(m_fileFormat) && cfg.getNbSignals("") == 0)
			makeDefaultFilename(filename, "");
		else if (!dataModel().isTAGformat(m_fileFormat))
			makeDefaultFilename(filename, cfg.getFirstFile("audio"));

		TRACE << "##############" << endl;
		TRACE << endl << " LOADING FORMAT " << m_fileFormat << endl << endl;
		TRACE << "##############" << endl;
		bool opened = m_dataModel.loadFromFile(filename, m_fileFormat);
		TRACE << "##############" << endl;
		TRACE << endl << " AFTER LOADING FORMAT " << m_fileFormat << endl
		        << endl;
		TRACE << "##############" << endl;

//		m_synchroTypes = m_dataModel.getMainstreamTypes();
		configureSynchrotypes() ;

		if (!opened)
			return false;
	}
	catch (const char* msg)
	{
		//		THREADS_ENTER
		Glib::ustring mess = "";
		Glib::ustring mmsg = Glib::ustring(msg) ;

		Formats* formats = Formats::getInstance();
		bool importable = formats && (formats->isImport(m_fileFormat));

		if (importable)
			mess = Glib::ustring(_("Error while loading file")) + "\n"
			        + filename + "\n";
		else
			mess = Glib::ustring(_("This format can't be imported")) + "\n"
			        + filename + "\n";

#ifdef __APPLE__
		if ( importable )
		dlg::error(mess, mmsg, NULL);
		else
		dlg::error(mess, NULL);
#else
		if (importable)
			dlg::error(mess, mmsg, m_top);
		else
			dlg::error(mess, m_top);
#endif

		//		THREADS_LEAVE
		closeFile(true, false);
		showStatus("");
		return false;
	}
	catch (...)
	{
		MSGOUT << " caught unk exception for loadFromFile" << endl;
		return false ;
	}

	m_lang = m_dataModel.getAGSetProperty("lang");
	if (!m_lang.empty())
		m_configuration["lang"] = m_lang;

	onUpdateConventions();

	return true;
}

std::map<string, string> AnnotationEditor::fromFilesToSignals(const std::vector<
        string>& signalFiles)
{
	SignalConfiguration& signalCfg = m_dataModel.getSignalCfg();

	std::map<string, string> signalIds;
	std::vector<string>::const_iterator it;
	int cpt = 1;
	for (it = signalFiles.begin(); it != signalFiles.end(); it++)
	{
		TRACE << "\t\t\t filename = [" << *it << "]" << endl;

		if (FileInfo(*it).exists())
		{
			IODevice* device = Guesser::open((*it).c_str());
			if (device)
			{
				string id = "NO-SIGNAL-" + number_to_string(cpt);
				signalCfg.enterAudioSignal(id, *it, -1);
				TRACE << "\t\t\t\t stock signal = " << *it << endl;
				cpt++;
				int channels = device->m_info()->audio_channels;
				if (channels == 2)
				{
					id = "NO-SIGNAL-" + number_to_string(cpt);
					signalCfg.enterAudioSignal(id, *it, -1);
					TRACE << "\t\t\t\t stock signal = " << *it << endl;
					cpt++;
				}
				device->m_close();
				delete (device);
			}
			else
				TRACE << "\t\t\t\t >> can't determine channels" << endl;
		}
		else
			TRACE << "\t\t\t\t >> can't be found " << endl;
	}
	return signalIds;
}

std::vector<string> AnnotationEditor::fromSignalsToFiles()
{
	SignalConfiguration& signalCfg = m_dataModel.getSignalCfg();
	std::map<std::string, std::string> available_ids_path;
	std::vector<string> signalFiles;

	/*** Empty signal: do nothing ***/
	if (signalCfg.getEmptySignalTracks() > 0)
		return signalFiles;
	/*** classic case: only audio ***/
	else if (!signalCfg.hasVideo())
		available_ids_path = signalCfg.getAudioIdPaths();
	/*** only video case ***/
	else if (signalCfg.isVideoStandAlone())
		available_ids_path = signalCfg.getVideoIdPaths();
	/*** both audio and video ***/
	else
		available_ids_path = signalCfg.getIdPaths();

	std::set<string> unique;

	bool existing ;
	std::map<string, string>::iterator it;
	for (it = available_ids_path.begin(); it != available_ids_path.end(); it++)
	{
		TRACE << "\t\t\t sigid = [" << it->first << "] - path= " << it->second << endl;
		// -- We've already find this path but file is mono channel, add it
		existing = unique.find(it->second) != unique.end()  ;
		if (existing && signalCfg.getChannel(it->first) <= 1)
			signalFiles.push_back(it->second);
		// -- First time we find it,
		else if (!existing)
		{
			unique.insert(it->second);
			signalFiles.push_back(it->second);
		}
	}

//	std::set<string>::iterator itu;
//	for (itu = unique.begin(); itu != unique.end(); itu++)
//		signalFiles.push_back(*itu);

	return signalFiles;
}

bool AnnotationEditor::checkAutosave()
{
	if (!FileInfo(m_autosavePath).exists())
		return false;

	bool recovered = false;

	// check if autosaveFile newer than orig file
	if (FileInfo(m_autosavePath).mtime() > FileInfo(m_agFilename).mtime())
	{
		THREADS_ENTER
		bool ok = dlg::confirm(_("Recover from autosaved file?"),
		        getTopWindow());
		if (ok)
		{
			TRACE << "\trecovering from autosave = " << m_autosavePath << endl;
			if (m_agFilename.empty())
				m_agFilename = m_defaultFilename;
			recovered = true;
		}
		else
		{
			MSGOUT << " remove file " << m_autosavePath << endl;
			::g_unlink(m_autosavePath.c_str());
		}
		THREADS_LEAVE
	}
	return recovered;
}

void AnnotationEditor::actualizeActions()
{
	if (!m_dataModel.hasAnnotationGraph("background_graph"))
	{
		Glib::RefPtr<Gtk::Action> action =
		        m_actionGroups["annotate"]->get_action(
		                "annotate_new_background");
		if (action)
			action->set_sensitive(false);
		action = m_actionGroups["annotate"]->get_action("edit_background");
		if (action)
			action->set_sensitive(false);
		action = m_actionGroups["annotate"]->get_action("delete_background");
		if (action)
			action->set_sensitive(false);
	}
}

void AnnotationEditor::changeInterline()
{
	Glib::ustring lang = m_dataModel.getTranscriptionLanguage();
	int interline_above = -1;
	int interline_below = -1;
	if (lang == "ara")
	{
		interline_above
		        = atoi(m_configuration["interline,above_large"].c_str());
		interline_below
		        = atoi(m_configuration["interline,below_large"].c_str());
	}
	else
	{
		interline_above = atoi(
		        m_configuration["interline,above_default"].c_str());
		interline_below = atoi(
		        m_configuration["interline,below_default"].c_str());
	}

	for (guint i = 0; i < m_textView.size(); ++i)
	{
		if (interline_above > 0)
			m_textView[i]->set_pixels_above_lines(interline_above);
		if (interline_below > 0)
			m_textView[i]->set_pixels_below_lines(interline_below);
	}
	InputLanguage* ilang =
	        InputLanguageHandler::get_input_language_by_shortcut(lang);
	if (ilang == NULL)
		ilang = InputLanguageHandler::get_input_language_by_shortcut(
		        DEFAULT_LANGUAGE);
	set_input_language(ilang);
}

void AnnotationEditor::setDefaultViewMode(bool switching, bool inThread)
{
	changeActiveViewMode(m_defaultViewMode, switching);

	// NO signal view and not loaded yet ? Tell GUI we've finished
	//> DEPRECATED ? should never be reached
//	if (!hasSignalView() && !m_loaded)
//		terminateDisplayLoading();
}

void AnnotationEditor::changeActiveViewMode(int mode, bool only_visibility)
{
	if (m_activeViewMode == mode && !only_visibility)
		return;

	m_activeViewMode = mode;

	if (!only_visibility)
		signalDisplayReloading().emit();

	if (m_textView.size() > 1)
	{
		int to_show ;

		if ( mode >= 0 )
			to_show = mode ;
		else if (mode==-1)
			to_show = m_textView.size() - 1 ;
		else
		{
			if (m_activeTrack>=0)
				to_show = m_activeTrack ;
			else
				to_show = 0 ;
		}

		for (guint i = 0; i < m_textView.size(); ++i)
			m_textView[i]->get_parent()->hide_all();

		m_textView[to_show]->get_parent()->show_all();

		if (!only_visibility)
			m_textView[to_show]->updateView();

		// single mode
		if (mode >= 0)
			m_signalView->setSelectedAudioTrack(mode);
		// dual view
		if (mode == -2 && m_textView.size() > 2)
		{
			int to_actualize = (to_show==1 ? 0 : 1) ;
			m_textView[to_actualize]->get_parent()->show_all();
			if (!only_visibility)
				m_textView[to_actualize]->updateView();
		}
		m_textView[to_show]->setFocus(false) ;
	}
	else if (!only_visibility)
		m_textView[0]->updateView();
}

//
// set focus on text view;
bool AnnotationEditor::setFocusWhenIdle()
{
	//MSGOUT << " IN THREAD " << endl;
	gdk_threads_enter();
	//MSGOUT << " IN THREAD 2 " << endl;
	setFocus() ;
	//gdk_flush();
	gdk_threads_leave();
	//MSGOUT << " OUT THREAD " << endl;
	return false;
}

void AnnotationEditor::setFocus()
{
	getActiveView()->setFocus(false);
}


string AnnotationEditor::makeDefaultFilename(const std::string tagfile,
        const std::string signalfile)
{
	// define current work directory and autosave path
	string workdir;
	if (tagfile != "")
		workdir = FileInfo(tagfile).dirname();
	else
		workdir = FileInfo(signalfile).dirname();

	workdir = FileInfo(workdir).realpath();

	if (workdir != "" && FileInfo(workdir).canWrite())
		m_workdir = workdir;

	string name;
	if (tagfile != "")
		name = Glib::path_get_basename(tagfile);
	else
	{
		name = Glib::path_get_basename(signalfile);
		string tail = FileInfo(name).tail();
		name.erase(name.length() - tail.length());
		string suff = "";
		for (int i = 1;; ++i)
		{
			string test = FileInfo(m_workdir).join(name) + suff + ".tag";
			if (!FileInfo(test).exists())
			{
				// check  that no non-writable autosave path  exists
				test += "~";
				if (!FileInfo(test).exists() || FileInfo(test).canWrite())
					break;
			}
			suff = string("_") + StringOps().fromInt(i);
		}
		name += (suff + ".tag");
	}

	m_defaultFilename = Glib::build_filename(m_workdir, name);

	m_autosavePath = m_defaultFilename + "~";
	m_lastsavePath = m_defaultFilename + "#";

	Log::out() << " autosave_path = " << m_autosavePath << endl;

	return m_defaultFilename;
}

/*
 * check if buffer modified in one of text views
 */
bool AnnotationEditor::hasModifiedBuffer()
{
	bool buffer_modified = false;
	for (guint i = 0; !buffer_modified && i < m_textView.size(); ++i)
	{
		if (m_textView[i])
			buffer_modified = m_textView[i]->getBuffer()->get_modified();
		else
			buffer_modified = false;
	}

	return buffer_modified;
}

void AnnotationEditor::setWaitCursor(bool b)
{
	for (guint i = 0; i < m_textView.size(); ++i)
	{
		if (m_textView[i]->is_visible())
		{
			m_textView[i]->setWaitCursor(b);
		}
	}
}

//------------------------------------------------------------------------------
//-------------------------------- FILE ACTIONS --------------------------------
//------------------------------------------------------------------------------


/**
 *  File actions handler
 *  @param action action to be undertaken : save/saveas/close/refresh
 */
void AnnotationEditor::onFileAction(const string& action)
{
	if (m_isLocked)
	{
		MSGOUT << "LOCKED " << endl;
		get_display()->beep();
		return;
	}

	getActiveView()->storePendingTextEdits();

	//> Refresh file (actualization)
	if (action == "refresh")
	{
		setWaitCursor(true);
		// after idle to force cursor update
		Glib::signal_idle().connect(sigc::mem_fun(this, &AnnotationEditor::refreshWhenIdle), 20);
	}
	//> Export file to another format
	else if (action == "export")
	{
		exportFile();
	}
	//> Save file
	else if (action == "save")
	{
		saveFile(m_agFilename, false, false, true);
	}
	//> Save file under new name
	else if (action == "saveas")
	{
		saveFile("", false, false, true);
	}
	//> Close file
	else if (action == "close")
	{
		// if file modified -> ask for confirmation before closing
		if (!m_loaded)
		{
			dlg::msg(_("Please wait for the file to be loaded."));
			m_signalCloseCancelled.emit();
		}
		else if (progressWatcher)
		{
			dlg::msg(_("A file is beeing copied, please wait."));
			m_signalCloseCancelled.emit();
		}
		// Tell user some modifications are unsaved, except for locked consultation
		else if (fileModified() && !consultationMode)
		{
			string name = Glib::path_get_basename(getFileName());
			string msg = string(_("The file")) + " \'" + name;
			msg = msg + "\' "
			        + _("has been modified, save file before closing ?");

			Gtk::Window* top = getTopWindow();
			dlg::Confirmsg3
			        dialog(Glib::ustring(_("Confirm")), *top, msg, true); //true = modal
			switch (dialog.run())
			{
			case Gtk::RESPONSE_YES:
				if (!saveFile(m_agFilename, false, false, true))
					break;
			case Gtk::RESPONSE_NO:
				if (FileInfo(m_autosavePath).exists())
				{
					MSGOUT << " remove file " << m_autosavePath << endl;
					::g_unlink(m_autosavePath.c_str());
				}
				terminateSession();
				break;
			case Gtk::RESPONSE_CANCEL:
				m_signalCloseCancelled.emit();
				break;
			}
		}
		else
			terminateSession();
	}
	//> Revert from old version/autosave
	else if (action == "revert_from_file" || action == "revert_from_autosave")
	{
		if (fileModified())
		{
			string
			        message =
			                _("You will loose all unsaved modifications.\nDo you want to reload file ?");
			if (!dlg::confirm(message, getTopWindow()))
				return;
		}
		reloadFile((action == "revert_from_autosave"));
		MSGOUT << " OUT REVERT ********************************** " << endl;
	}
}

void AnnotationEditor::reloadFile(bool from_autosave)
{
	//> revert from file and file is empty
	if (!from_autosave && m_agFilename.empty())
	{
		THREADS_ENTER
		dlg::error(_("Can't reload new file !"));
		THREADS_LEAVE
		return;
	}

	string path;
	//> check if autosavepath exists
	if (!from_autosave)
		path = m_agFilename;
	else
		path = m_autosavePath;

	//> if file can't be found even after application test, abort
	if (!FileInfo(path).exists())
	{
		Glib::ustring msg;
		if (from_autosave)
			msg = _("Autosave file not found");
		else
			msg = _("File not found");

		THREADS_ENTER
		dlg::error(msg);
		THREADS_LEAVE
		return;
	}

	//> let's go reverting :)
	try
	{
		setWaitCursor(true);
		bool resetAGid = true;
		m_dataModel.loadFromFile(path, m_fileFormat, true, resetAGid);

		Glib::signal_timeout().connect(sigc::mem_fun(this, &AnnotationEditor::refreshWhenIdle), 20); // after idle to force cursor update
		if (!from_autosave)
		{
			m_isAutosaved = false;

			for (guint i = 0; i < m_textView.size(); ++i)
				m_textView[i]->getBuffer()->set_modified(false);
			onFileModified();
		}
	}
	catch (const char* msg)
	{
		THREADS_ENTER
		dlg::error(msg);
		THREADS_LEAVE
		closeFile(true, true);
		return ;
	}
	catch (...)
	{
		MSGOUT << " caught unk exception for loadFromFile" << endl;
	}
}

bool AnnotationEditor::refreshWhenIdle()
{
	refresh();
	setWaitCursor(false);
	return false;
}

void AnnotationEditor::refresh(bool disableThread)
{
	m_elapsed.reset() ;
	m_elapsed.start() ;

	signalDisplayReloading().emit();

	// Lock edition for correct view update. Will be release by
	// terminateDisplayLoading method
	setLocked(true);

	// save cursor position in all view
	changeTrackCursor(-2, false) ;

	m_isLocked = true;
	float signal_offset = 0.0;
	if (m_signalView != NULL)
		signal_offset = m_signalView->getCursor();

	bool buffer_modified = (hasModifiedBuffer() || m_isAutosaved);
	for (guint i = 0; i < m_textView.size(); ++i)
	{
		if (m_textView[i]->is_visible())
		{
			if (disableThread)
				m_textView[i]->disableThread(true);
			m_textView[i]->updateView();
			if (disableThread)
				m_textView[i]->disableThread(false);
			m_textView[i]->getBuffer()->set_modified(buffer_modified);
		}
	}

//  DEPRECATED ?
//	if (!hasSignalView())
//		terminateDisplayLoading();

	m_isLocked = false;
}

/*
 * request file close
 */
void AnnotationEditor::closeFile(bool force, bool emitSignal)
{
	TRACE  << "AnnotationEditor: closing file ..." << endl;
	if (m_signalView != NULL)
	{
		inhibateSynchro(true) ;
		m_signalView->setPlay(false);
		USLEEP(300*1000);
	}
	if (!force)
		onFileAction("close");
	else
		terminateSession(emitSignal);
}

//
//  detach speller from all views coz if done in View destructor,
// a segfault may happen on some configurations
bool AnnotationEditor::terminateSession(bool emit_signal)
{
	Log::out() << " IN terminateSession ********************************** " << endl;

	// inhibate synchro or audio signals will be sent to zombie audio component :)
	inhibateSynchro(true) ;

	if (videoManager)
		videoManager->hideVideo();

/* SPELL */
//	for (guint i = 0; i < m_textView.size(); ++i)
//	{
//		if (m_textView[i])
//			m_textView[i]->detachSpeller();
//	}
//	Log::out() << " >>> speller detached" << endl;

	m_dataModel.clear(); // free mem allocated for dataModel
	Log::out() << " >>> model cleaned" << endl;

	if (emit_signal)
		m_signalCanClose.emit();
	Log::out() << " >>> can close emitted" << endl;

	Log::out() << " OUT terminateSession ********************************** " << endl;
	return false;
}

/*==============================================================================*
 *										       									*
 *									SAVING										*
 *   																			*
 *==============================================================================*/
bool AnnotationEditor::saveFile(string filename, bool evenNotModified, bool skipPath, bool checkSpk)
{
	//> -- Block for unauthorized actions
	// TODO: do it before calling method ?
	if (m_fileFormat == "SGML")
	{
		dlg::msg(_("SGML import can't be saved."));
		return false;
	}

	//> -- Saving existing file and no modification ? nothing to do, exit
	bool modified = fileModified() || evenNotModified;
	if (!filename.empty() && !modified)
		return true;

	//> -- Save last pending modification in model
	for (guint i = 0; i < m_textView.size(); ++i)
		m_textView[i]->storePendingTextEdits();

	//> -- Save some data in file
	// User activity
	char wid[10];
	sprintf(wid, "%x", (guint) (m_activityWatcher->getActivityTime()));
	m_dataModel.updateVersionInfo(m_configuration["scribe"], wid);

	if (!skipPath)
	{
		//> -- For all signals: save path hint && audio settings
		// (current zoom level, sound volume, track VZoom, track offset)
		SignalConfiguration& signalCfg = dataModel().getSignalCfg();
		std::map<string, string> sids = signalCfg.getIdPaths();
		std::map<string, string>::iterator it;
		for (it = sids.begin(); it != sids.end(); it++)
		{
			// save path hint
			m_dataModel.setSignalProperty(it->first, "path_hint", string(
			        FileInfo(it->second).dirname()));
			// save settings
			if (hasSignalView())
				m_dataModel.setSignalProperty(it->first, "audio_settings",
				        m_signalView->getCurrentAudioSettings());
		}
	}

	//> -- Ask for cleaning file
	bool del_spk = false;
	if ( m_dataModel.hasUnusedSpeakers() && checkSpk )
		del_spk = dlg::confirm( _("Delete unused speakers from file dictionary ?"), getTopWindow() ) ;

	//> -- No file name given, select one
	bool overwritten = false;
	bool converted = false;
	int copyMediaFile = -1;

	//> -- Don't allow media file when empty signal
	if (m_dataModel.getSignalCfg().getEmptySignalTracks() == 0)
		copyMediaFile = 0;

	if (!filename.empty())
		filename = prepareSavingExistingFile(filename, overwritten, converted);
	else
		filename = prepareSavingAsFile(copyMediaFile, overwritten, converted);

	bool saved = processFileSaving(filename, del_spk, copyMediaFile, overwritten, converted);
	return saved;
}

std::string AnnotationEditor::prepareSavingExistingFile(string filename,
        bool& overwritten, bool& converted)
{
	//> -- Not native format : mention the conversion
	if (m_fileFormat != "TransAG")
	{
		if (m_fileFormat != "TransAG_compat")
		{
			dlg::msg(_("File will be converted to TransAG format")) ;
			converted = true ;
		}
		else
		{
			Log::out() << "Format: auto-import..." << std::endl ;
			converted = false ;
		}
		string tail = FileInfo(filename).tail();
		StringOps(tail).toLower();
		if (!(tail == ".tag" || tail == ".xml"))
		{
			filename.erase(filename.length() - tail.length());
			filename += ".tag";
		}
		m_fileFormat = "TransAG";
		// for conversion, rename notebook page
	}

	//> -- Actualize current file
	m_agFilename = filename;

	return filename;
}

std::string AnnotationEditor::prepareSavingAsFile(int& copyMediaFile,
        bool& overwritten, bool& converted)
{
	// -- Get proposition from user
	string filename = dlg::selectTAGFile(m_defaultFilename, false,
	        copyMediaFile, getTopWindow());

	// -- Force TAG extension
	if (!filename.empty() && Explorer_filter::get_extension(filename) != ".tag"
	        && Explorer_filter::get_extension(filename) != ".TAG")
		filename = filename + ".tag";

	return filename;
}

bool AnnotationEditor::processFileSaving(string filename, bool clean_speaker,
        int copyMediaFile, bool overwritten, bool converted)
{
	string dir = Glib::path_get_dirname(filename);

	bool file_ok = (!FileInfo(filename).exists() || filename
	        == m_defaultFilename);
	bool dir_ok = FileInfo(dir).exists() && FileInfo(dir).canWrite();

	//> -- Check save path (and enable to change it if invalid)
	while (!(file_ok && dir_ok))
	{
		bool chooser;

		//> -- Current dir is no more available ? change
		if (!dir_ok)
		{
			dlg::msg(
			        _("Destination directory doesn't exist or you don't have permission to modify it\n Please choose another directory"));
			chooser = true;
		}
		//> -- File already exists and writable ? overwrite ?
		else if (!file_ok)
		{
			// -- File with same path exist, overwrite ?
			if (FileInfo(filename).canWrite())
			{
				Gtk::Window* top = getTopWindow();
				Glib::ustring
				        txt =
				                Glib::ustring(
				                        _("An annotation file with same name already exists."));
				txt = txt + "\n( " + Glib::path_get_basename(filename) + " )\n"
				        + _("Overwrite existing file ?");
				Glib::ustring title = Glib::ustring(_("Confirm"));
				dlg::Confirmsg3 dialog(title, *top, txt, true);
				int rep = dialog.run();
				if (rep == Gtk::RESPONSE_YES)
					chooser = false;
				else if (rep == Gtk::RESPONSE_NO)
					chooser = true;
				else if (rep == Gtk::RESPONSE_CANCEL)
					return "";
				// -- if overwritting, rename current page and disable expand
				overwritten = true;
			}
			else
			{
				dlg::msg(_("Name already in use, please choose another name"));
				chooser = true;
			}
		}

		if (chooser)
		{
			// -- Get user proposition
			filename = dlg::selectTAGFile(filename, false, copyMediaFile,
			        getTopWindow());
			// -- Force TAG extension
			if (!filename.empty() && Explorer_filter::get_extension(filename)
			        != ".tag" && Explorer_filter::get_extension(filename)
			        != ".TAG")
				filename = filename + ".tag";
			// -- Check new values
			dir = Glib::path_get_dirname(filename);
			file_ok = !(FileInfo(filename).exists() && filename
			        != m_defaultFilename);
			dir_ok = FileInfo(dir).exists() && FileInfo(dir).canWrite();
		}
		else
		{
			// -- Allow loop exit
			file_ok = true;
			dir_ok = true;
		}
	}

	if (filename.empty())
		return false;

	bool is_new_file = !FileInfo(filename).exists();

	if (m_activityWatcher != NULL)
	{
		// store overall activity time in graph
		char buf[12];
		sprintf(buf, "%x", (int) m_activityWatcher->getActivityTime());
		m_dataModel.setTranscriptionProperty("wid", buf);
	}

	//> -- Save in model
	bool saved = false;
	string newMediaDir = "";
	try
	{
		m_dataModel.setAGSetProperty("convention_id",
		        m_dataModel.conventions().name());

		// save file
		m_dataModel.saveToFile(filename, "TransAG", clean_speaker);
		saved = true;

		// copy media file if needed
		// do it only when saving from an existing file
		if (copyMediaFile > 0)
		{
			string newdir = Glib::path_get_dirname(filename);
			newMediaDir = copyMediaFiles(newdir);
		}

		m_agFilename = filename;
	}
	catch (const char* detailed)
	{
		Glib::ustring det = Glib::ustring(detailed) ;
		Log::err() << "Saving file: ERROR (" << detailed << ")" << endl;
		string msg = _("An error occurred while saving file.");
		dlg::error(msg, det, NULL);
		return false;
	}

	//> -- Actualize state
	for (guint i = 0; i < m_textView.size(); ++i)
	{
		m_textView[i]->set_editable(true);
		m_textView[i]->getBuffer()->set_modified(false);
	}
	m_isAutosaved = false;

	//> -- Media file has changed ? actualize path
	if (!newMediaDir.empty())
		actualizeMediaPathHint(newMediaDir);

	//> -- Remove autosave file
	if (FileInfo(m_autosavePath).exists())
	{
		MSGOUT << " remove file " << m_autosavePath << endl;
		::g_unlink(m_autosavePath.c_str());
	}

	//> -- Adjust autosavePath in case file name has changed
	SignalConfiguration& signalCfg = m_dataModel.getSignalCfg();
	std::vector<string> signalFiles = signalCfg.getPaths("");
	if (signalFiles.size() > 0)
		makeDefaultFilename(m_agFilename, signalFiles[0]);
	else
		makeDefaultFilename(m_agFilename, "");

	//> -- File has been save, let's make a backup of that last saved version
	if (saved)
	{
		try
		{
			// save file
			m_dataModel.saveToFile(m_lastsavePath, "TransAG", clean_speaker);
		}
		catch (const char* msg)
		{
			Log::err() << "Saving backup ERROR (" << msg << ")" << endl;

#ifdef __APPLE__
			dlg::error(msg, m_top);
#else
			dlg::error(msg);
#endif
			return false;
		}
	}

	bool rename_notebook = false;
	bool expand_notebook = true;

	if (converted)
		rename_notebook = true;
	if (overwritten)
	{
		rename_notebook = true;
		expand_notebook = false;
	}

	Log::out() << "File saved : " << m_agFilename << endl;
	m_signalFileSaved.emit(m_agFilename, is_new_file, rename_notebook, expand_notebook);

	return true;
}

std::string AnnotationEditor::copyMediaFiles(string newAudioDir)
{
	string newMediaDir = "";
	string audioName, newAudioPath, oldAudioPath;

	std::vector<std::string> paths = m_dataModel.getSignalCfg().getPaths("");
	std::vector<std::string>::iterator it;
	progressWatcher = new ProgressionWatcher(
	        _("Copying media files... PLEASE WAIT..."), "property_audio",
	        true, getTopWindow());
	for (it = paths.begin(); it != paths.end(); it++)
	{
		bool copy = true;
		audioName = Glib::path_get_basename(*it);
		newAudioPath = Glib::build_filename(newAudioDir, audioName);

		if (*it == "")
			copy = false;
		else if (FileInfo(newAudioPath).exists())
		{
			Gtk::Window* top = getTopWindow();
			Glib::ustring txt = Glib::ustring(
			        _("A media file with same name already exists."));
			txt = txt + "\n( " + audioName + " )\n"
			        +_("Overwrite existing file ?");
			Glib::ustring title = Glib::ustring(_("Confirm"));
			dlg::Confirmsg3 dialog(title, *top, txt, true);
			int rep = dialog.run();
			dialog.hide();
			if (rep != Gtk::RESPONSE_YES)
				copy = false;
		}

		if (copy)
		{
			GtUtil::copy(*it, newAudioDir, progressWatcher, getTopWindow());
			newMediaDir = newAudioDir;
		}
	}
	progressWatcher->hide();
	delete (progressWatcher);
	progressWatcher = NULL;

	return newMediaDir;
}

void AnnotationEditor::actualizeMediaPathHint(string newdir)
{
	SignalConfiguration& signalCfg = dataModel().getSignalCfg();
	std::map<string, string> sids = signalCfg.getIdPaths();
	std::map<string, string>::iterator it;

	string tmpdir = newdir;
	for (it = sids.begin(); it != sids.end(); it++)
	{
		//just save: use the SignalConfiguration path value
		if (newdir.empty())
			tmpdir = string(FileInfo(it->second).dirname());
		//changing dir: modify SignalConfiguration
		else
		{
			const string& base = Glib::path_get_basename(it->second);
			const string& newpath = Glib::build_filename(tmpdir, base);
			signalCfg.changePath(it->first, newpath);
		}

		// set in model
		m_dataModel.setSignalProperty(it->first, "path_hint", tmpdir);
	}

	// save file
	m_dataModel.saveToFile(getFileName(), "TransAG", false);

	Log::out() << "Path hint actualized." << std::endl;
}

void AnnotationEditor::cleanPathHint()
{
	SignalConfiguration& signalCfg = dataModel().getSignalCfg();
	std::map<string, string> sids = signalCfg.getIdPaths();
	std::map<string, string>::iterator it;

	for (it = sids.begin(); it != sids.end(); it++)
	{
		// set in model
		m_dataModel.setSignalProperty(it->first, "path_hint", "");
	}
	Log::out() << "Path hint cleaned in file." << std::endl;
}

/*
 * autosave annotation file if updated
 */
bool AnnotationEditor::autosaveFile(bool inThread)
{
	if (m_autosavePath == "")
		return true;
	if (hasModifiedBuffer() == false)
		return true;

	//TODO allow undo while autosaving ?
	//(filter while undoing, no while autosaving ?)
	setUndoableActions(false);

	/*
	 for ( guint i=0; i < m_textView.size(); ++i )
	 m_textView[i]->storePendingTextEdits();
	 */
	signalStatusBar().emit(_("Autosaving...")) ;

	SignalConfiguration& signalCfg = dataModel().getSignalCfg();
	std::map<string, string> sids = signalCfg.getAudioIdPaths();
	std::map<string, string>::iterator it;
	// FOR ALL SIGNALS
	for (it = sids.begin(); it != sids.end(); it++)
	{
		// save path hint
		m_dataModel.setSignalProperty(it->first, "path_hint", string(FileInfo(
		        it->second).dirname()));
	}

	//> check if directory exists, else
	Glib::ustring tmp_name = m_autosavePath;
	Glib::ustring dir = Glib::path_get_dirname(tmp_name);

	if (!FileInfo(dir).exists() || !FileInfo(dir).canWrite())
	{
		if (inThread)
		{
			gdk_threads_enter();
#ifdef __APPLE__
			dlg::msg(_("Autosave failure, can't write on current file's directory"), m_top);
#else
			dlg::msg(
			        _("Autosave failure, can't write on current file's directory"));
#endif
			gdk_threads_leave();
		}
		else
		{
			THREADS_ENTER
#ifdef __APPLE__
			dlg::msg(_("Autosave failure, can't write on current file's directory"), m_top);
#else
			dlg::msg(
			        _("Autosave failure, can't write on current file's directory"));
#endif
			THREADS_LEAVE
		}
	}
	else
	{
		try
		{
			Log::out() << "Autosaving " << m_autosavePath << "..." << endl;
			m_dataModel.setAGSetProperty("lang", m_lang);
			m_dataModel.setAGSetProperty("convention_id", m_dataModel.conventions().name());
			m_dataModel.saveToFile(m_autosavePath, "TransAG", false);
			m_isAutosaved = true;
			for (guint i = 0; i < m_textView.size(); ++i)
				m_textView[i]->getBuffer()->set_modified(false);
		}
		catch (const char* msg)
		{
			Log::out() << "Autosaving failed..." << endl;
			if (inThread)
				gdk_threads_enter();
			else
				THREADS_ENTER
			dlg::error(msg);
			if (inThread)
				gdk_threads_leave();
			else
				THREADS_LEAVE
		}
	}

	//TODO remove if allowing undo while autosaving
	setUndoableActions(true);
	signalStatusBar().emit("") ;

	return true;
}

/*
 * 	Export action : export to other format
 */
bool AnnotationEditor::exportFile()
{
	bool successful = false;

	// TODO enable it ?
	if (m_fileFormat == "SGML")
	{
#ifdef __APPLE__
		dlg::msg(_("SGML import cannot be exported."), m_top);
#else
		dlg::msg(_("SGML import cannot be exported."));
#endif
		return successful;
	}

	bool ok = false;
	string nameToBe;
	string name_to_display = m_agFilename;
	Glib::ustring format_to_be;

	// choose a name not already existing !
	while (!ok)
	{
		nameToBe = dlg::selectExportFile(name_to_display, getTopWindow(),
		        format_to_be);

		if (FileInfo(nameToBe).exists())
		{
			ok = false;
			if (FileInfo(nameToBe).canWrite())
			{
				Gtk::Window* top = getTopWindow();
				dlg::Confirmsg3 dialog(_("Confirm"), *top,
				        _("Overwrite existing file ?"), true);
				int rep = dialog.run();
				if (rep == Gtk::RESPONSE_YES)
					ok = true;
				else if (rep == Gtk::RESPONSE_NO)
					ok = false;
				else if (rep == Gtk::RESPONSE_CANCEL)
					return false;
			}
			else
#ifdef __APPLE__
				dlg::msg(_("Name already in use, please choose another name"), m_top);
#else
				dlg::msg(_("Name already in use, please choose another name"));
#endif
		}
		else
			ok = true;
	}

	//> OK; let's launch
	if (ok && !nameToBe.empty())
	{
		string msg = "";

		std::map<string, string> export_options;
		//TODO get export options from file

		try
		{
			m_dataModel.saveToFileWithOptions(export_options, nameToBe,
			        format_to_be, true);

			bool check = true;
			if (isStereo() && format_to_be == "CHAT")
			{
				Glib::ustring ext = Explorer_filter::get_extension(nameToBe);
				Glib::ustring nameToBe_0 = Explorer_filter::cut_extension(
				        nameToBe) + "_0" + ext;
				Glib::ustring nameToBe_1 = Explorer_filter::cut_extension(
				        nameToBe) + "_1" + ext;
				check = Glib::file_test(nameToBe_0, Glib::FILE_TEST_EXISTS);
				check = check && Glib::file_test(nameToBe_1,
				        Glib::FILE_TEST_EXISTS);
			}
			else
				check = Glib::file_test(nameToBe, Glib::FILE_TEST_EXISTS);

			if (check)
				msg = _("The file has been successfully exported.");
			else
				msg = _("An error occurred while exporting file.");
#ifdef __APPLE__
			dlg::msg(msg, m_top);
#else
			dlg::msg(msg, NULL);
#endif
		}
		catch (const char* detailed)
		{
			TRACE << "EXPORT ERROR =(" << detailed << ")" << endl;
			msg = _("An error occurred while exporting file.");
			Glib::ustring det = Glib::ustring(detailed) ;
#ifdef __APPLE__
			dlg::error(msg, det, m_top);
#else
			dlg::error(msg, det, NULL);
#endif
			return false;
		}
	}

	return ok && !nameToBe.empty();
}

/*
 * editor menu/toolbar actions handler.
 */
void AnnotationEditor::onMenuAction(const std::string& action, bool onlySelection, bool ignoreTime, string hint)
{
	float cursor = -1.0 ;
	float end_cursor = 0.0 ;
	int notrack = 0 ;

	if (m_signalView != NULL)
		cursor = m_signalView->getCursor() ;

//		TRACE << "onMenuAction " << action << " onlySelection=" << onlySelection << endl;

	bool focus = has_focus() || getActiveView()->has_focus();

	if (!onlySelection && !focus)
	{
		TRACE_D << "NO focus !" << endl;
		return ;
	}

	//	autosaveFile();  // always autosave before modifying graph

	if (m_signalView != NULL)
	{
		// ---------------
		// -- Externals --
		// ---------------
		if (action == "export_to_audacity")
		{
			// -- Audacity --
			exportSelectionTo("audacity");
			return;
		}
		else if (action == "export_to_wavesurfer")
		{
			// -- Wavesurfer --
			exportSelectionTo("wavesurfer");
			return;
		}
		else if (action == "export_to_praat")
		{
			// -- Praat --
			exportSelectionTo("praat");
			return;
		}
		// --------------------
		// -- Editor options --
		// --------------------
		else if (action == "hideTags")
		{
			//> hide tags
			toggleHideTags(true);
			return;
		}
		else if (action == "highlight")
		{
			//> switch on/off highlight
			toggleHighlightMode(true);
			return;
		}
		else if (action == "filemode")
		{
			//> change filemode
			toggleFileMode(true);
			return;
		}
		else if (action == "display")
		{
			//> change dual view
			toggleDisplay(true);
			return;
		}
		else if (action == "synchro_signal_to_text")
		{
			//> switch on/off synchro swt
			toggleSynchro(action, true);
			return;
		}
		else if (action == "synchro_text_to_signal")
		{
			//> switch on/off synchro tw
			toggleSynchro(action, true);
			return;
		}
		else
		{
			notrack = m_signalView->getSelectedAudioTrack();
//			if ( onlySelection && action != "new_background"
//							 && action != "hideTags"
//							 && action != "highlight"
//							 && action != "filemode"
//							 && action != "display"
//							 && action != "synchro_signal_to_text"
//							 && action != "synchro_text_to_signal" )
			if ( onlySelection )
			{
				float selcursor;
				// -- Check is selection really exists
				if ( !m_signalView->getSelection(selcursor, end_cursor) )
				{
					if ( action == "new_background" ||  action == "new_unit_event" )
						onlySelection = false;
					else
					{
						dlg::warning(_("No signal portion selected"));
						return;
					}
				}


				// -- When adding annotations from signal popup menu -> force text synchro
				if ( action != "new_background" &&  action != "new_unit_event" )
				{
					getView(notrack)->setFocus(false) ;
					synchroTextToSignal(selcursor, end_cursor, true);
				}
				cursor = selcursor ;
			}
		}
	}

	//> save signal selection
	if (action == "save_selected_signal")
		saveSelectionSignal();
	//> go to offset
	else if (action == "go_to")
		goToPosition();
	//> launch annotationView action
	//  (only accept action if current view correspond to current signal)
	else if (m_activeView == getView(notrack))
		getView(notrack)->onAnnotateAction(action, notrack, cursor, end_cursor, onlySelection, ignoreTime, hint);
	//> otherwise display error msg
	else
		dlg::error(_("Active editor doesn't correspond to selected audio signal"), m_top);
}

/*
 * if modified state of buffer changes to true -> emit fileModified signal
 */
void AnnotationEditor::onFileModified()
{
	m_signalFileModified.emit(fileModified());
}

bool AnnotationEditor::fileModified()
{
	bool autosaved = m_isAutosaved;
	bool bufferModified = hasModifiedBuffer();
	bool modelUpdated = m_dataModel.isUpdated();
	bool total = (autosaved || bufferModified || modelUpdated);
	return total;
}

/*========================================================================
 *
 *  SignalView tracks management
 *
 ========================================================================*/

bool AnnotationEditor::addSignalView(std::vector<string>& signalFiles)
{
	//>>>>>>>>>>>>>>>>>>>>>>>>>> PRE-CREATION OPTIONS
	// Set horizontal & vertical scale widget size
	string size_a = m_configuration["Signal,vertical_scale_size"];
	if (size_a.compare("") != 0)
	{
		int size = atoi(size_a.c_str());
		if (size > 0)
			AudioSignalView::setVerticalScaleSize(size);
	}

	// Set horizontal & vertical scale widget size
	size_a = m_configuration["Signal,horizontal_scale_size"];
	if (size_a.compare("") != 0)
	{
		int size = atoi(size_a.c_str());
		if (size > 0)
			AudioSignalView::setHorizontalScaleSize(size);
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>> CREATION
	Explorer_filter *e_filter = Explorer_filter::getInstance();
	bool signal_video = m_dataModel.getSignalCfg().hasVideo();

	if (!signalFiles.empty() && (e_filter->is_video_file(signalFiles[0])
	        || signal_video))
	{
		string filepath = signalFiles[0];
		createMediaComponents(filepath, true);
	}
	else
		createMediaComponents("", false);

	//>>>>>>>>>>>>>>>>>>>>>> POST CREATION OPTIONS
	// Set use of relative or absolute norm for computing peaks
	string norm = m_configuration["Signal,peaks_norm"];
	if (norm.compare("") != 0 && norm.compare("absolute") == 0)
		AudioSignalView::setUseAbsolutePeaksNorm(true);
	else if (norm.compare("") != 0 && norm.compare("relative") == 0)
		AudioSignalView::setUseAbsolutePeaksNorm(false);

	// Set max resolution for waveform display
	if (m_configuration["Signal,resolution"] != "")
	{
		float resmax = atof(m_configuration["Signal,resolution"].c_str());
		if (resmax > 0.0)
		{
			TRACE << "Set waveform max resolution to " << resmax << endl;
			AudioSignalView::setZoomMax(resmax);
		}
	}

	m_signalView->setRewindAtEnd(m_configuration["Signal,rewind_at_end"] == "true") ;
	m_signalView->setRewindAtSelectionEnd(m_configuration["Signal,rewind_at_selection_end"] == "true") ;
	bool stopOnClick = (m_configuration["Signal,stop_on_click"] == "true") ;
	m_signalView->setStopOnClick(stopOnClick) ;

	// Set save peaks
	if (m_configuration["Signal,save_peaks"] == "true")
	{
		AudioSignalView::setSavePeaks(true);
		if (m_configuration["Signal,peaks_dir"] != "")
		{
			FileInfo fi(m_configuration["Signal,peaks_dir"]);
			if (fi.exists() && fi.canWrite())
				m_signalView->setPeaksDirectory(fi.realpath());
			else
				MSGOUT << "non existent or not writable peaks dir "
				        << fi.realpath() << endl;
		}
	}
	else
		AudioSignalView::setSavePeaks(false);

	// set cursor size
	if (m_configuration["Signal,cursor,size"] != "")
	{
		int cursor_sz = atoi(m_configuration["Signal,cursor,size"].c_str());
		if (cursor_sz == 0)
			cursor_sz = 1;
		m_signalView->setCursorSize(cursor_sz);
	}

	//> hack : force time scale packing in addAudioStreamMethod
	AudioSignalView::setShowScale(true);

	//> Restart delay options
	if (m_configuration.find("Signal,restart_delay") != m_configuration.end())
		m_signalView->setBackwardStartDelay(atof(m_configuration["Signal,restart_delay"].c_str()));

	//> Skip delay options
	if (m_configuration.find("Signal,skip_delay") != m_configuration.end())
		m_signalView->setQuickmoveDelay(atof(m_configuration["Signal,skip_delay"].c_str()));

	//>>>>>>>>>>>>>>>>>>>>>> TRACKS CONFIGURATION
	bool ok = openSignals(signalFiles);
	if (!ok)
		return false;

	//>>>>>>>>>>>>>>>>>>>>>> COLORS CONFIGURATION
	setAudioColors();

	//>>>>>>>>>>>>>>>>>>>>>> DISPLAY
	if (m_configuration["Signal,show"] != "false")
	{
		m_box->pack_start(*m_signalView, Gtk::PACK_SHRINK, 0);
		m_signalView->show();
	}

	//>>>>>>>>>>>>>>>>>>>>>> MENU ACTIONS
	configureMediaActions();

	//>>>>>>>>>>>>>>>>>>>>>> SIGNALS CONNECTIONS
	configureMediaSignals();

	//>>>>>>>>>>>>>>>>>>>>>> END
	signalSignalViewAdded().emit(__LINE__);
	//TRACE_D << "++++++   signalView added in " << tim.elapsed() << " secs" << endl;

	return true;
}

void AnnotationEditor::createMediaComponents(const string& filepath,
        bool videoMode)
{
	if (videoMode)
	{
		videoPlayer = new VideoWidget();
		m_signalView = new AudioSignalView(videoPlayer);
		m_signalView->setToplevel(getTopWindow());
		m_signalView->setToFocus(getActiveView());
		videoPlayer->setSignalView(m_signalView);

		std::string configdir = globalOptions.getParameterValue("General",
		        "start", "config");
		videoFrameBrowser = new FrameBrowser(getTopWindow(), configdir, false);
		videoFrameBrowser->loadGeoAndDisplay();

		videoManager = new VideoManager(videoPlayer, videoFrameBrowser);
	}
	else
		m_signalView = new AudioSignalView();

	m_signalView->signalPeaksReady().connect(sigc::mem_fun(this, &AnnotationEditor::onPeaksReady)) ;
}

void AnnotationEditor::deleteVideoComponents()
{
	if (videoPlayer)
	{
		delete (videoPlayer);
		videoPlayer = NULL;
	}
	if (videoFrameBrowser)
	{
		delete (videoFrameBrowser);
		videoFrameBrowser = NULL;
	}
	if (videoManager)
	{
		delete (videoManager);
		videoManager = NULL;
	}
}

bool AnnotationEditor::openSignals(std::vector<string>& signalFiles)
{
	bool retry = true;
	const char* passphrase = NULL;
	bool openVideoFile = false ;
	string videoFile ;

	while (retry)
	{
		try
		{
			// -- Streaming Case ? --
			if (m_rtsp)
			{
				m_rtsp_path = m_dataModel.getSignalProperty(0, "url_hint");
				TRACE << "\t\t\t adding signal for STREAMING MODE : [ " << m_rtsp_path << " ]" << std::endl;
				m_signalView->setRtspChannels(m_channels);
				m_signalView->setStreamFile(m_agFilename);
				bool ok = m_signalView->addAudioStream( (char*) m_rtsp_path.c_str(), (char*) passphrase);
				if (!ok)
				{
					m_signalStreamingError.emit();
					return false;
				}
			}
			else
			{
				SignalConfiguration& signalCfg = m_dataModel.getSignalCfg();
				std::vector<string>::iterator it;
				Explorer_filter* filter = Explorer_filter::getInstance();

				for (it = signalFiles.begin(); it != signalFiles.end();)
				{
					TRACE << "\t\t\t prepare to add signal(s) for LOCAL MODE : [ "<< *it << " ]" << std::endl;
					// special behavior for video file stream creation
					if (filter && filter->is_video_file(*it))
					{
						videoFile = *it;
						// if VideoFile uses Audio file for the sound, don't create audiostream from Video file
						if (!signalCfg.isVideoStandAlone() && signalFiles.size() > 1)
							it = signalFiles.erase(it);
						else
							++it;
					}
					else
						++it;
				}

				// configure signalview for single signal mode
				if (signalCfg.isSingleSignal())
					m_signalView->setSingleSignal(true);

				// once streams has been created, let's open media file if some exists
				if (!videoFile.empty() && videoPlayer)
				{
					TRACE << "\t\t\t dealing with video stream for : [ " << videoFile << " ]" << std::endl;
					bool ok = videoPlayer->openFile(videoFile.c_str());
					if (!ok)
					{
						dlg::error(_("The video file could not be opened, aborted."), NULL);
						deleteVideoComponents();
					}
					else
						openVideoFile = true ;
				}

				bool ok;
				if (!signalFiles.empty())
					ok = m_signalView->addAudioStreams(signalFiles);
				else
				{
					int channel = signalCfg.getNbSignals("audio");
					double length = m_dataModel.getSignalDuration(false);
					ok = m_signalView->addEmptyAudioStream(channel, length);
				}

				TRACE << "\t\t\t added [" << ok << "]" << std::endl;

				if (!ok)
				{
					TRACE << "\t\t\t error when adding stream: aborting ... " << std::endl;
					dlg::error(_("An error occurred when opening media, closing file...")) ;
					return false;
				}
				else if (openVideoFile)
				{
					const std::string& width = globalOptions.getParameterValue("Video", "frameBrowser", "frame-w");
					const std::string& height = globalOptions.getParameterValue("Video", "frameBrowser", "frame-h");
					const std::string& step = globalOptions.getParameterValue("Video", "frameBrowser", "resolution");
					videoFrameBrowser->fill(videoFile, width, height, step);
				}
			}

			/*** single signal ***/
			SignalConfiguration& signalCfg = m_dataModel.getSignalCfg();
			// -- Use channel signal computation for classic mode
			if ( !signalCfg.isSingleSignal()
					&& signalCfg.getEmptySignalTracks()==0 )
				m_nbTracks = m_signalView->getNbSignalTracks();
			else
				m_nbTracks = signalCfg.getNbSignals("audio");

			TRACE << "\t\t\t\t total tracks [" << m_nbTracks << "]" << std::endl;

			/*** multi tracks case ***/
			if (m_nbTracks == 2 )
			{
				TRACE << "\t\t\t\t\t audio mode : STEREO" << endl;
				m_modeStereo = true;
				m_signalView->setCurrent(true);
				m_highlightMode = getConfigurationHighlightMode();
			}
			else
			{
				TRACE << "\t\t\t\t\t audio mode : MONO" << endl;
				m_modeStereo = false;
				m_highlightMode = getConfigurationHighlightMode();
				if (m_highlightMode != -1)
					m_highlightMode = 3;
			}

			m_activeTrack = 0;
			m_lastTrack = -1;
			retry = false;

			//> once time scale has been added by addAudioStream method, actualize visibility
			bool show_scale = (m_configuration["Signal,scale,show"] != "false");
			m_signalView->show_scale(show_scale);

		}
		catch (const char* msg)
		{
			if (passphrase != NULL || m_configuration["control"].empty())
				retry = false;
			else
				passphrase = m_configuration["control"].c_str();
			if (!retry)
			{
				delete m_signalView;
				m_signalView = NULL;
				THREADS_ENTER
#ifdef __APPLE__
				dlg::error(msg, m_top);
#else
				dlg::error(msg);
#endif
				THREADS_LEAVE
				return false;
			}
		}
	}
	return true;
}

void AnnotationEditor::configureMediaActions()
{
	// AUDIO COMPONENT

	m_actionGroups["signal"] = m_signalView->getActionGroup();
	m_UIInfo["signal"] = "<ui>"
		"  <menubar name='MenuBar'>"
		"    <menu action='SignalMenu'>"
		"      <menuitem action='signal_play'/>"
		"      <menuitem action='signal_pause'/>"
		"      <separator/>"
/*		"      <menuitem action='signal_decr_tempo'/>"
		"      <menuitem action='signal_incr_tempo'/>"
		"      <separator/>"
*/		"      <menuitem action='signal_forward'/>"
		"      <menuitem action='signal_rewind'/>";

	if (m_nbTracks > 1)
	{
		m_UIInfo["signal"] += "      <separator/>";

		for (int i = 1; i <= m_nbTracks; ++i)
		{
			string item = "signal_track";
			item += i;
			string label = _("Active track");
			label += i;
			string accel = "<control>";
			accel += i;
			m_UIInfo["signal"] += "      <menuitem action='" + item + "'/>";

			m_toggleActions[item] = Gtk::ToggleAction::create(item, label,
			        _("Set track as current track for annotations"));
			m_toggleActions[item]->set_active(i == 1);
			m_actionGroups["signal"]->add(m_toggleActions[item], Gtk::AccelKey(
			        accel), sigc::bind<int, string>(sigc::mem_fun(*this,
			        &AnnotationEditor::selectSignalTrack), (i - 1), item));

		}
		m_signalView->signalAudioTrackSelected().connect(sigc::mem_fun(*this,
		        &AnnotationEditor::onSelectSignalTrack));
	}

	m_actionGroups["signal"]->add(Gtk::Action::create("synchro_text_to_signal",
	        _("Enable / Disable _text to signal synchronization"),
	        _("Toggle text to signal synchronization option")),
	        Gtk::AccelKey("F8"), sigc::bind<string, bool>(sigc::mem_fun(*this,
	                &AnnotationEditor::onMenuAction),
	                "synchro_text_to_signal", false, false, ""));

	m_actionGroups["signal"]->add(Gtk::Action::create("synchro_signal_to_text",
	        _("Enable / Disable _signal to text synchronization"),
	        _("Toggle signal to text synchronization option")),
	        Gtk::AccelKey("F9"), sigc::bind<string, bool>(sigc::mem_fun(*this,
	                &AnnotationEditor::onMenuAction),
	                "synchro_signal_to_text", false, false, ""));

	m_UIInfo["signal"] += "      <separator/>"
		"      <menuitem action='synchro_text_to_signal'/>"
		"      <menuitem action='synchro_signal_to_text'/>";

	m_actionGroups["signal"]->add(Gtk::Action::create("save_selected_signal",
	        _("Save signa_l selection"),
	        _("save selected portion signal")), sigc::bind<string, bool>(
	        sigc::mem_fun(*this, &AnnotationEditor::onMenuAction),
	        "save_selected_signal", false, false, ""));

	// -- Audacity / WaveSurfer --
	m_actionGroups["signal"]->add(Gtk::Action::create("export_to_audacity",
	        _("Export signal/selection to Audacity"),
	        _("Export selected portion or file to Audacity")),
	        sigc::bind<string, bool>(sigc::mem_fun(*this,
	                &AnnotationEditor::onMenuAction), "export_to_audacity",
	                false, false, ""));
	m_actionGroups["signal"]->add(Gtk::Action::create("export_to_wavesurfer",
	        _("Export signal/selection to Wavesur_fer"),
	        _("Export selected portion or file to Wavesurfer")),
	        sigc::bind<string, bool>(sigc::mem_fun(*this,
	                &AnnotationEditor::onMenuAction),
	                "export_to_wavesurfer", false, false, ""));
	m_actionGroups["signal"]->add(Gtk::Action::create("export_to_praat",
	        _("Export signal/selection to Praa_t"),
	        _("Export selected portion or file to Praat")), sigc::bind<
	        string, bool>(sigc::mem_fun(*this,
	        &AnnotationEditor::onMenuAction), "export_to_praat", false, false, ""));

	m_UIInfo["signal"] += "      <separator/>"
		"      <menuitem action='save_selected_signal'/>";

	m_actionGroups["signal"]->add(Gtk::Action::create("go_to",
	        _("Go to signal position"),
	        _("Go to signal position (in seconds)")), sigc::bind<string,
	        bool>(sigc::mem_fun(*this, &AnnotationEditor::onMenuAction),
	        "go_to", false, false, ""));
	m_UIInfo["signal"] += "      <separator/>"
		"      <menuitem action='go_to'/>";

	m_UIInfo["signal"] += "    </menu>"
		"  </menubar>"
		"</ui>";

	if (videoManager)
	{
		m_UIInfo["video_signal"] = videoManager->getUIInfo();
		m_actionGroups["video_signal"] = videoManager->getActionGroup();
	}
}

void AnnotationEditor::configureMediaSignals()
{
	// Audio signals for transcription actions
	m_signalView->signalCursorChanged().connect(sigc::bind<float, bool>(sigc::mem_fun(*this, &AnnotationEditor::synchroTextToSignalIdle), -1.0, false));
	m_signalView->signalDelayChanged().connect(sigc::mem_fun(*this, &AnnotationEditor::onSignalDelayChanged)) ;

	m_signalView->signalSelectionChanged().connect(sigc::bind<bool>(sigc::mem_fun(*this, &AnnotationEditor::synchroTextToSignalIdle), false));
	m_signalView->signalTrackActivated().connect(sigc::mem_fun(*this, &AnnotationEditor::onActivateSignalTrack));
	m_signalView->signalPopulatePopup().connect(sigc::mem_fun(*this, &AnnotationEditor::onSignalPopulatePopup));
	m_signalView->signalSegmentModified().connect(sigc::mem_fun(*this, &AnnotationEditor::onModifySegment));

	// Audio signals for media action
	if (videoPlayer)
	{
		m_signalView->signalPlayedPaused().connect(sigc::mem_fun(*this, &AnnotationEditor::onAudioPlayPauseReceived));
		m_signalView->syncSelection().connect(sigc::mem_fun(*this, &AnnotationEditor::onSyncSelection));
		m_signalView->stopVideo().connect(sigc::mem_fun(*this, &AnnotationEditor::onVideoStopReceived));
		m_signalView->syncVideo().connect(sigc::mem_fun(*this, &AnnotationEditor::onAudioSeekReceived));
		m_signalView->seekVideo().connect(sigc::mem_fun(videoPlayer, &VideoWidget::seek));
}

	//> VIDEO PLAYER
	//TODO actually fully piloted by the signal view, and inside it.
	//Should be external and connected here

	//> VIDEO BRWOSER
	if (videoFrameBrowser)
	{
		m_signalView->signalCursorChanged().connect(sigc::bind<float>(sigc::mem_fun(*this, &AnnotationEditor::synchroVBrowserWithSignal), -1.0));
		m_signalView->signalSelectionChanged().connect(sigc::mem_fun(*this, &AnnotationEditor::synchroVBrowserWithSignal));
		videoFrameBrowser->signalFrameChanged().connect(sigc::mem_fun(this, &AnnotationEditor::onFrameBrowserChanged));
	}
}

/*==============================================================*
 *   															*
 * 				 VIDEO-SIGNAL SYNCHRONISATION					*
 *   															*
 *==============================================================*/

void AnnotationEditor::onSignalDelayChanged(int notrack, float delay)
{
	mapTrackDelays[notrack] = delay ;
	Log::out() << "I have the delay for track=" << notrack << " - delay=" << delay << std::endl ;
}

void AnnotationEditor::synchroVBrowserWithSignal(float time, float unusedEndTime)
{
	if (synchroLock || !m_loaded)
		return;

	synchroLock = true;
	videoFrameBrowser->setToTime(time);
	synchroLock = false;
}

void AnnotationEditor::onFrameBrowserChanged(float time)
{
	if (synchroLock)
		return;

	synchroLock = true;
	m_signalView->setCursor(time);
	bool stopOnClick = (m_configuration["Signal,stop_on_click"] == "true");
	if (stopOnClick)
		m_signalView->setPlay(false);
	m_signalView->setCursor(time);
	videoPlayer->setScaleOffset(time, stopOnClick);
	synchroLock = false;
}

// --- OnVideoSeekReceived ---
// Each time user change audio cursor (not called in playback)
void AnnotationEditor::onAudioSeekReceived(double ts, bool stop_on_click)
{
	videoPlayer->setScaleOffset(ts, stop_on_click);
}

// --- OnAudioPlayPauseReceived ---
void AnnotationEditor::onAudioPlayPauseReceived(bool val)
{
	videoPlayer->setPlayback(val);
}

// --- OnVideoStopReceived ---
void AnnotationEditor::onVideoStopReceived()
{
	videoPlayer->stop();
}

// --- OnSyncSelection ---
void AnnotationEditor::onSyncSelection(float s_begin, float s_end)
{
	videoPlayer->setSelection(s_begin, s_end);
}

/*========================================================================
 *
 *  Options
 *
 ========================================================================*/

int AnnotationEditor::getConfigurationHighlightMode()
{
	string value;

	if (m_configuration["mode"] == "BrowseMode")
		value = m_configuration["BrowseMode,highlight_current"];
	else
		value = m_configuration["EditMode,highlight_current"];

	int res = 3;

	if (value == "selected")
		res = 3;
	else if (value == "both")
		res = 2;
	else if (value == "track1")
		res = 0;
	else if (value == "track2")
		res = 1;
	else if (value == "none")
		res = -1;

	return res;
}

/* update menu state on signal track selection */

void AnnotationEditor::selectSignalTrack(int notrack, string item)
{
	if (m_signalView != NULL && (notrack
	        != m_signalView->getSelectedAudioTrack())
	        && m_toggleActions[item]->get_active())
	{
		//set to TRUE for replacing cursor at the last position it was
		//when track was switched
		m_selectTrack_wt_cursor = false;
		m_signalView->setSelectedAudioTrack(notrack);
	}
}

void AnnotationEditor::selectSegmentTrack(int notrack, string type)
{
	if (m_segmentTrack.size() == 0 || m_segmentTrack[0].size() == 0
	        || m_segmentTrack[1].size() == 0)
		return;

	if (notrack == 1)
	{
		if (m_segmentTrack[0][type])
			m_segmentTrack[0][type]->set_current(false);
		if (m_segmentTrack[1][type])
			m_segmentTrack[1][type]->set_current(true);
	}
	else
	{
		if (m_segmentTrack[0][type])
			m_segmentTrack[0][type]->set_current(true);
		if (m_segmentTrack[1][type])
			m_segmentTrack[1][type]->set_current(false);
	}
}

void AnnotationEditor::onActivateSignalTrack(bool activated)
{
	//TRACE_D <<   " *******onActivateSignalTrack"  << endl ;

	if (!m_signalView->getAudioTrack(0)->isActivated())
	{
		getView().clearHighlight("", 0);
	}
	if (!m_signalView->getAudioTrack(1)->isActivated())
	{
		getView().clearHighlight("", 1);
	}
}

/* update menu state on signal track selection */
void AnnotationEditor::onSelectSignalTrack(int notrack)
{
	//	TRACE_D <<   " IN onSelectSignalTrack   " << notrack << "mode" << m_modeStereo << endl << flush;

	for (int i = 1; i <= m_nbTracks; ++i)
	{
		string item = "signal_track";
		item += i;
		m_toggleActions[item]->set_active(i == (notrack + 1));
	}

	//JM
	m_lastTrack = m_activeTrack;
	m_activeTrack = notrack;

	//replace cursor
	if (m_modeStereo)
	{
		if (m_activeViewMode == 0 && notrack == 1)
			m_signalView->setSelectedAudioTrack(0);
		else if (m_activeViewMode == 1 && notrack == 0)
			m_signalView->setSelectedAudioTrack(1);
		else
		{
			//set current track
			getView().setCurrentTrack(notrack);
			//set current segment
			selectSegmentTrack(notrack, "segment");
			//actualize current cursor
//			getView().actualizeTrackCursor(notrack, m_selectTrack_wt_cursor);
			changeTrackCursor(notrack, m_selectTrack_wt_cursor) ;

			if (m_selectTrack_wt_cursor)
				m_selectTrack_wt_cursor = false;
		}
		if (m_activeViewMode == -2)
			m_textView[notrack]->setFocus(false);
	}
}

void AnnotationEditor::changeTrackCursor(int notrack, bool move)
{
	Log::out() << "\nChange track cursor for track " << notrack << " - move = " << move << std::endl ;

	if (notrack==0 || notrack == -2)
	{
		if (m_textView.size()>1)
		{
			AnnotationView* view1 = getView(1) ;
			if (view1)
				view1->getBuffer()->saveCursor() ;
		}

		AnnotationView* view0 = getView(0) ;
		if (view0 && move)
		{
			float offset = view0->getBuffer()->restoreCursor() ;
			view0->setFocus(false) ;
		}
	}

	if ( (notrack==1 || notrack ==-2 ) && (m_textView.size()>1) )
	{
		AnnotationView* view0 = getView(0) ;
		if (view0)
			view0->getBuffer()->saveCursor() ;

		AnnotationView* view1 = getView(1) ;
		if (view1 && move)
		{
			float offset = view1->getBuffer()->restoreCursor() ;
			view1->setFocus(false) ;
		}
	}
}

void AnnotationEditor::onForceSynchroSTT()
{
	synchroSignalToText(getActiveView()->getBuffer()->getCursor(), true,
	        getActiveView());
}

/**
 * handler for segments anchors graphical modification through signal view widget
 * @param id modified segment id
 * @param start segment new start offset
 * @param stop segment new stop offset
 * @param ptrack Segment track widget that has emitted signal
 *
 * segment anchors will be updated in annotation graph, adjacent segments will eventually be automatically resized.
 */
void AnnotationEditor::onModifySegment(const string& id, float start,
        float stop, SegmentTrackWidget* ptrack)
{
	string type = "";
	int notrack;

	//	TRACE_D << "AnnotationEditor::onModifySegment  id = " << id << " start=" << start << " stop=" << stop << endl;

	std::map<std::string, SegmentTrackWidget*>::iterator it;
	for (notrack = 0; notrack < m_nbTracks; ++notrack)
	{
		for (it = m_segmentTrack[notrack].begin(); it
		        != m_segmentTrack[notrack].end(); ++it)
			if (it->second == ptrack)
			{
				type = it->first;
				break;
			}
		if (!type.empty())
			break;
	}

	if (type == "")
	{
		MSGOUT << ": track not found" << endl;
		return;
	}

	if (!m_dataModel.existsElement(id))
	{
		TRACE << "WARNING : onModifySegment " << id
		        << " : element not found in graph !!" << endl;
		return;
	}

	string diag;
	bool checkResizeRules = m_dataModel.checkResizeRules(id, start, stop, diag);

	if (!checkResizeRules)
	{
#ifdef __APPLE__
		dlg::warning(diag, m_top);
#else
		dlg::warning(diag);
#endif
		updateTrack(type, id, 1, false, notrack, true);
	}
	else
	{
		getView().getBuffer()->begin_user_action() ;

		m_dataModel.setSegmentOffsets(id, start, stop);

		string startAnchor = m_dataModel.getStartAnchor(id) ;
		string endAnchor = m_dataModel.getEndAnchor(id) ;

		m_dataModel.anchorLinks().setLinksOffset(startAnchor, start) ;
		m_dataModel.anchorLinks().setLinksOffset(endAnchor, stop) ;

		getView().getBuffer()->end_user_action() ;

		m_currentSegment[notrack][type].setId("");
		synchroTextToSignalWithType(type, m_signalView->getCursor(), -1.0, notrack, false);
	}
}

void AnnotationEditor::showSegmentTracks(const string& type, int p_notrack, bool afterLoading)
{
	Glib::Timer tim;
	tim.start();

	signalStatusBar().emit(std::string(_("Initializing signal tracks...")));

	TRACE << "IN showSegmentTracks type=" << type << " - notrack=" << p_notrack << " - fillTrack=" << afterLoading << endl;

	int notrack;
	int start_track, end_track;
	if (p_notrack == -1)
	{
		start_track = 0;
		end_track = m_segmentTrack.size();
	}
	else
	{
		start_track = p_notrack;
		end_track = p_notrack + 1;
	}

	// empty previous segments storage
	if (p_notrack < (int) m_segmentTrack.size())
	{
		for (notrack = start_track; notrack < end_track; ++notrack)
		{
			//		m_currentSegment[notrack].clear();
			m_indexes[notrack].clear();
			std::map<std::string, AnnotationIndex >::iterator itm;
			for (itm = m_indexes[notrack].begin(); itm != m_indexes[notrack].end(); ++itm)
				itm->second.clear();

			std::map<std::string, SegmentTrackWidget*>::iterator its;
			for (its = m_segmentTrack[notrack].begin(); its!= m_segmentTrack[notrack].end(); ++its)
			{
				if (its->second)
				{
					if (!its->second->is_visible())
						m_toHideTmp.push_back(its->second) ;
 					its->second->removeSegments(m_signalView->getAudioTrack(notrack));
				}
			}
		}
	}

	// allocate enough space for all tracks
	if (m_segmentTrack.size() != (guint) m_nbTracks)
	{
		m_segmentTrack.resize(m_nbTracks);
		m_indexes.resize(m_nbTracks);
		m_currentSegment.resize(m_nbTracks);
	}

	vector<string> v ;
	vector<string>::iterator it_type;

	//> if no segment type passed as argument, search all type existing
	// in configuration and display all
	if (type.empty())
	{
		m_modelTypes.clear() ;
		m_trackTypes.clear() ;
		m_modelTypes = m_dataModel.getAnnotationTypes(true) ;
		v = m_modelTypes ;
		m_trackTypes = m_modelTypes ;
	}
	else
		v.push_back(type) ;

	//> If we have no segment to display emit signal ready for indicating
	//  GUI that file loading is ended.
	if (v.size() == 0)
	{
		loadingTracksReady = true ;
		onViewReady(-1) ;
		return;
	}

	if (p_notrack == -1)
	{
		start_track = 0;
		end_track = m_segmentTrack.size();
	}
	else
	{
		start_track = p_notrack;
		end_track = p_notrack + 1;
	}

	for (notrack = start_track; notrack < end_track && notrack < m_segmentTrack.size() ; ++notrack)
	{
		AudioTrackWidget* audioTrack = m_signalView->getAudioTrack(notrack);
		//	audioTrack->showLastBegin(false);

		for (it_type = v.begin(); it_type != v.end(); ++it_type)
		{
			bool isAnchoredType = m_dataModel.conventions().isAnchoredType(*it_type);
			bool isBaseUnit = (m_dataModel.mainstreamBaseType() == *it_type) ;
			
			//TODO do it dynamically
			bool special_case = (m_fileFormat == "SGML" && *it_type == "background");
			
			//- exists in convention ?
			bool exist = ( find(m_trackTypes.begin(), m_trackTypes.end(), *it_type)!=m_trackTypes.end() ) ;
			//- user wants them ?
			bool visible = ( find(m_visibleTracks.begin(), m_visibleTracks.end(), *it_type)!=m_visibleTracks.end() ) ;
			bool canDisplay = exist && visible ;

			//-- Only base unit can be unanchored, otherwise skip
			// 	 Special case can be skipped
			//   Display only what should be displayed
			if ( (!isAnchoredType&&!isBaseUnit)
					|| special_case)
			{
				continue ;
			}

			//-- Proceed all tracks
			if (m_segmentTrack[notrack].find(*it_type) == m_segmentTrack[notrack].end())
			{
				m_segmentTrack[notrack][*it_type] = m_signalView->addSegmentTrack(_(it_type->c_str()), notrack, isBaseUnit);
				m_segmentTrack[notrack][*it_type]->signalGetText().connect(sigc::bind<int, string>(sigc::mem_fun(*this, &AnnotationEditor::getSegmentText), notrack, *it_type));
				m_segmentTrack[notrack][*it_type]->setSelectable(isEditMode());
				if (!canDisplay)
					m_toHideTmp.push_back(m_segmentTrack[notrack][*it_type]) ;

				//> before Loading ? => add temporary segments while loading is proceeded
				if (!afterLoading)
				{
					string label = _("Loading...");
					string color = getSegmentColor("", *it_type);
					m_segmentTrack[notrack][*it_type]->addSegment("AG-load", 0, m_dataModel.getSignalDuration(false),
																		label.c_str(), color.c_str(), true, 0, false,
																		false, m_signalView->getAudioTrack(notrack));
				}
				 
				//-- Hide unwanteds tracks 
				if (!canDisplay)
					m_segmentTrack[notrack][*it_type]->hide() ;
			}
			else if (m_segmentTrack[notrack][*it_type])
				m_segmentTrack[notrack][*it_type]->removeSegments(audioTrack);

			m_currentSegment[notrack][*it_type].setId("");
		}

		//> If processing loading, launch actions
		if (afterLoading)
		{
			m_nbThread++;
			Glib::signal_idle().connect(sigc::bind<int>(sigc::mem_fun(*this, &AnnotationEditor::showLabelTracksAfterIdle), notrack));
		}
	}

	Log::out() << "((( showSegmentTrack << type=" << type << " notrack=" << p_notrack << " in " << tim.elapsed() << " secs" << endl;
}

bool AnnotationEditor::showLabelTracksAfterIdle(int notrack)
{
	gdk_threads_enter();
	getTrackSegmentation(notrack);
	showLabelTracks(notrack);
	--m_nbThread;
	gdk_threads_leave();
	if (m_nbThread == 0)
		onViewReady(-12) ;

	return false;
}


void AnnotationEditor::getTrackSegmentation(int notrack)
{
	//TRACE_D << "IN getSegmentTrackData for track" << notrack  << endl;
	Glib::Timer tim;

	signalStatusBar().emit(std::string(_("Initializing signal segments...")));

	vector<string> v;
	vector<string>::iterator it_type;
//	StringOps(m_configuration["Signal,tracks"]).split(v, " ,;");
	AudioTrackWidget* audioTrack = m_signalView->getAudioTrack(notrack);
	audioTrack->showLastBegin(false);

	std::vector<string> v_singleLine;
	std::vector<string> v_disable_highlight_types;
	StringOps(m_configuration["Signal,force_single_line"]).split(v_singleLine, ";");
	StringOps(m_configuration["Signal,disable_highlight"]).split(v_disable_highlight_types, ";");

	for (it_type = m_modelTypes.begin(); it_type != m_modelTypes.end(); ++it_type)
	{
		const string& graphtype = m_dataModel.conventions().getGraphtypeFromType(*it_type);

		if (graphtype.empty()) // unknown in current conventions -> skip type
			continue;

		// show only types valid for current annotation conventions definition
		bool isAnchoredType = m_dataModel.conventions().isAnchoredType(*it_type, graphtype)  ;
		bool isBaseUnit = ( *it_type== m_dataModel.mainstreamBaseType(graphtype)) ;

		// not anchored or not autoinsert ? don't add it to segment mechanism
		// (except for baseUnit, always in it)
		if (  !isAnchoredType && !isBaseUnit)
			continue;

		bool hidden_order = false;
		if ( find(v_singleLine.begin(), v_singleLine.end(), *it_type) != v_singleLine.end() )
			hidden_order = true;

		vector<SignalSegment> segments;
		vector<SignalSegment>::iterator it;
		bool is_speech_type = (*it_type == m_dataModel.conventions().getSpeakerType(graphtype));
		if (m_segmentTrack[notrack][*it_type])
		{
			if (m_dataModel.getSegments(*it_type, segments, 0.0, 0.0, notrack, true, true))
			{
				for (it = segments.begin(); it != segments.end(); ++it)
				{
					//const string& label = getView(notrack)->getSegmentText(*it_type, it->getId(), it->getEndId());
					int order = it->getOrder();
					string color = getSegmentColor(it->getId(), *it_type);
					bool skip_highlight = is_in_svect(v_disable_highlight_types,
							*it_type);
					bool can_skip = (!is_speech_type || !m_dataModel.isSpeechSegment(it->getId())) ;
					m_segmentTrack[notrack][*it_type]->addSegmentToCache(
							it->getId(), it->getStartOffset(),
							it->getEndOffset(),
							/*label.c_str() */NULL, color.c_str(),
							can_skip, order, hidden_order,
							skip_highlight, audioTrack);
					m_indexes[notrack][*it_type].add(it->getId(),
							it->getStartOffset(), order);
				}
			}
		}
	}

	TRACE_D << "++++++   OUT getSegmentTrackData for " << notrack << " in "
	        << tim.elapsed() << " secs" << endl;
}

string AnnotationEditor::getSegmentColor(const string& id, const string& type)
{
	if (type.empty() && id.empty())
		return "Grey";

	// TODO en fichier de conf
	if (type == "alignmentREF" || type == "alignmentHYP")
	{
		string align_type = dataModel().getElementProperty(id, "type", "");
		if (align_type == "D" || align_type == "I")
			return "Red";
		else if (align_type == "S")
			return "Grey";
		else if (align_type == "C")
			return "White";
		else
			return "Black";
	}
	// TODO inactif en couleur différente
	//	else if (type=="background" && !dataModel().isActiveBackground(id) ) {
	//			return "PaleGrey" ;
	//	}
	// TODO fichier de conf configurable !
	else
	{
		std::map<string, string>::iterator it = m_trackColor.find(type);
		if (it != m_trackColor.end())
			return m_trackColor[type];
		else
			return "Green";
	}
}


void AnnotationEditor::onPeaksReady(bool success)
{
	Log::out() << "Editor loader ----------------------> Peaks   [OK]" << std::endl ;
	loadingPeaksReady = true ;
}

void AnnotationEditor::onTracksReady()
{
	loadingTracksMutex.lock() ;
	Log::out() << "TrackReady received ... " << std::endl ;

	//> only process for last view when two activated views
	if (isStereo() && m_activeViewMode == -2)
	{
		m_nbLoadingTracks++ ;
		if ( hasSignalView() && (m_nbLoadingTracks % m_nbTracks != 0) )
		{
			Log::out() << "TrackReady : still some, let's wait..." << std::endl ;
			loadingTracksMutex.unlock() ;
			return ;
		}
	}

	loadingTracksReady = true ;

	loadingTracksMutex.unlock() ;
	Log::out() << "Editor loader ----------------------> Tracks   [OK]" << std::endl ;
}

void AnnotationEditor::onViewReady(int notrack)
{
	//> only process for last view when two activated views
	if ( isStereo() && m_activeViewMode == -2 )
	{
		m_loadingView++ ;
		// -- For merged or dual view, wait for the last view
		// -- For single view, only 1 call so no pb
		if ( hasSignalView() && (m_loadingView % m_nbTracks != 0) )
		{
			Log::out() << "........... onViewReady BLOCKED for " << notrack << std::endl ;
			return ;
		}
	}

	//> Opening file : Restore audio settings only when loading file
	//(NOT WHEN REFRESHING DISPLAY!)
	if (!m_loaded && hasSignalView())
		setWaitCursor(false);

	//> Keep old state
	bool was_loaded = m_loaded;

	//> Opening file : check special loading
	if (!was_loaded)
		logLoading(true, true);

	Log::out() << "Editor loader ----------------------> View   [OK]" << std::endl ;
	terminateDisplayLoading() ;
}

void AnnotationEditor::terminateDisplayLoading()
{
	while (/*!loadingTracksReady ||*/ !loadingPeaksReady)
	{
		GtUtil::flushGUI(false, true) ;
		USLEEP(500) ;
	}

	Log::out() << "Editor loader ------------------------> Ending" << std::endl ;

	//> <!> WARNING: set audio preferences BEFORE all saving action in order
	//      to enable correct setting recuperation while saving file.
	//TODO  improve this behaviour
	if (!m_loaded && hasSignalView())
	{
		setWaitCursor(false);

		const string& settings = m_dataModel.getSignalProperty(0, "audio_settings");
		if (!settings.empty())
			m_signalView->setAudioSettings(settings);
		else
			m_signalView->setZoomEntire();
	}

	//> If existing tag transcription and audio file(s) newly associated, try to save
	//  (don't do it if user choosed no signal)
	if (!m_loaded && pathHintActualized)
	{
		m_signalStatusBar.emit(std::string(_("Updating signal path...")));
		bool saved = saveFile(getFileName(), true, false, false);
		TRACE << "\n-------------------------------------((o)) UPDATING audio path [" << saved << "]" << endl;
		pathHintActualized = false ;
	}

	//> If opening create a backup
	if (!m_loaded && !m_newFile && !m_signalView->isStreaming() && !consultationMode)
	{
		m_signalStatusBar.emit(std::string(_("Creating backup...")));
		int res = FileHelper::copy_and_rename(getFileName(), m_lastsavePath, true, NULL, true);
		TRACE << "\n-------------------------------------((o)) CREATING BACKUP [" << res << "]" << endl;
	}

	//> Actualize state
	//  (For plugin consultation mode, always lock edition)
	// <!> WARNING: clicking on view before complete loading seems to block
	// 				main loop 1.time/20
	// TODO improve this behaviour and unlock as soon as view part
	// is done
	setSensitiveSignalView(true);
	setLocked(false);

	//> Actualize the current track
	if (hasSignalView())
	{
		if (m_nbTracks > 1)
		{
			int track ;
			if (m_activeTrack>0)
				track = m_activeTrack ;
			else
				track = 0 ;
			onSelectSignalTrack(track);
		}
		else
			m_signalView->setSelectedAudioTrack(0);

		hideInvalidSegmentTracks() ;
	}

	m_loaded = true ;
	loadingTracksReady = false ;
	m_totalLoadingTracks = 0 ;
	m_nbLoadingTracks = 0 ;
	m_loadingView = 0 ;

//	disableThreadProtection = false ;

	//> Tells GUI we've finished loading or refreshing
	Log::out() << "\nEditor loader ======================  READY in " << m_elapsed.elapsed() << endl;
	signalStatusBar().emit(TRANSAG_VERSION_NO) ;
	signalReady().emit() ;

	if (isDebugMode())
	{
		for (guint i = 0; i < m_textView.size(); i++)
		{
			if (m_textView[i])
				m_textView[i]->signalSetCursor().connect(sigc::mem_fun(*this, &AnnotationEditor::cursorChanged));
		}
	}
	Log::out() << "**Exit display loading**" << endl;
}

void AnnotationEditor::hideInvalidSegmentTracks()
{
	std::vector<SegmentTrackWidget*>::iterator it ;
	for (it=m_toHideTmp.begin(); it!=m_toHideTmp.end(); it++)
	{
		if ( *it && (*it)->is_visible() )
			(*it)->hide() ;
	}
	m_toHideTmp.clear() ;
}

void AnnotationEditor::showLabelTracks(int notrack)
{
	Glib::Timer tim;
	tim.start();

	signalStatusBar().emit(std::string(_("Displaying signal segments...")));

	if (m_segThread != NULL)
	{
		m_segThread = NULL;
	}

	Log::out() << "++++++ IN showLabelTracks -> notrack = " << notrack << endl;

	vector<string> v;
	vector<string>::iterator it_type;

	string show_begin = m_configuration["Signal,tracks,show_begin"];

	AudioTrackWidget* audioTrack = m_signalView->getAudioTrack(notrack);
	audioTrack->showLastBegin(false);

	for (it_type = m_trackTypes.begin(); it_type != m_trackTypes.end(); it_type++)
	{
		const string& graphtype = m_dataModel.conventions().getGraphtypeFromType(*it_type);

		if (graphtype.empty()) // unknown in current conventions -> skip type
			continue;

		bool isBaseUnit = (m_dataModel.mainstreamBaseType(graphtype) == *it_type ) ;
		bool canDisplay = ( find(m_visibleTracks.begin(), m_visibleTracks.end(), *it_type) != m_visibleTracks.end() ) ;

		// show only types valid for current annotation conventions definition
		if ( !m_dataModel.conventions().isAnchoredType(*it_type, graphtype) && !isBaseUnit)
			continue;

		if (m_segmentTrack[notrack][*it_type])
		{
			m_segmentTrack[notrack][*it_type]->manageCachedSegments();
			if (canDisplay)
				m_segmentTrack[notrack][*it_type]->show();
		}
	}

	if ( it_type!=m_trackTypes.end() && (show_begin == *it_type) )
	{
		audioTrack->showLastBegin(true);
		audioTrack->setLastBeginSegmentTrack(m_segmentTrack[notrack][*it_type]);
	}

	Log::out() << "++++++ OUT  showLabelTracks for " << notrack << " in " << tim.elapsed() << " secs" << endl;
}

/**
 * return annotation text view associated to given track no
 * @param notrack signal track no (starting at 0)
 * @return annotation view associated to track
 */
AnnotationView* AnnotationEditor::getView(int notrack)
{
	// --  activeViewMode set to -1 means we are in mono view or stereo merged view
	// ==> in both cases, last view is the one :)
	// ==> otherwise returns corresponding view
	//TRACE_D << "AnnotationEditor::getViewForTrack number " << notrack << endl;
	if (m_activeViewMode!=-1 && notrack == 0 && m_textView[notrack] && m_textView[0]->is_visible())
	{
		//TRACE_D << "AnnotationEditor::getViewForTrack: 0" << endl;
		return m_textView[0];
	}
	else if (m_activeViewMode!=-1 && notrack == 1 && m_textView.size()>1 && m_textView[notrack] && m_textView[1]->is_visible())
	{
		//TRACE_D << "AnnotationEditor::getViewForTrack: 1" << endl;
		return m_textView[1];
	}
	else
	{
		//TRACE_D << "AnnotationEditor::getViewForTrack: else" << endl;
		return m_textView.back();
	}
}

/**
 * retrieve segment text from AnnotationBuffer and load it into return buffer
 * @param id annotation id
 * @param res (OUT) returned buffer pointer
 * @param notrack current track
 * @param type annotation type
 *
 * @note
 *   returned string is allocated and will be freed by caller
 */
void AnnotationEditor::getSegmentText(const string& id, char*& res, int notrack, const string& type)
{
	const string& label = getView(notrack)->getSegmentText(type, id, "end");
	res = strdup(label.c_str());
}

/**
 * Updates segment track display.
 * @param type 				Track type
 * @param mid 				Updated annotation id / "" to update all track
 * @param action 			Update action : 0=delete, 1=update, 2=insert 	8=freeze_update 9=commit_update
 * @param text_only 		If true only update segment text
 * @param notrack 			Corresponding signal track no
 * @param fill_tracks 		If set to no, only display tracks without filling elements
 * @return 					Always false (allow to use method with signal_timout or signal_connect)
 */
void AnnotationEditor::updateTrack(string type, string mid, int action, bool text_only, int notrack, bool fill_tracks)
{
	Glib::Timer tim;
	tim.start();

//	TRACE << "\n\n\n********************************* UpdateTrack: type= " << type << "  id = " << mid << " action ="  << action <<" text_only=" << text_only << " NOTRACK=" << notrack << endl;

	//>> GLOBAL UPDATE
	if (mid.empty() && type.empty())
	{
		showSegmentTracks(type, notrack, fill_tracks) ;
		return;
	}
	else if ( type!=m_dataModel.getElementType(mid) && m_dataModel.existsElement(mid) )
		Log::out() << "~~~~~~~~~ Update Track: Element ID and Element TYPE don't match " << std::endl ;

	string id("");
	string parent("") ;

	//>> For all action except delete
	if (action != 0)
	{
		//> If we want to update mainstream base type, search for the first anchored
		//  (because some can be not anchored)
		if (type == m_dataModel.mainstreamBaseType())
			id = m_dataModel.getAnchoredBaseTypeStartId(mid);
		//> Otherwise we update given element if it exists
		else if (m_dataModel.existsElement(mid))
			id = mid;
	}

	string old_id = (id.empty() ? mid : id);
	//	TRACE_D << "\t old_id= " << old_id << endl;

	if (notrack >= m_nbTracks)
	{
		MSGOUT << " invalid notrack = " << notrack << endl;
		return;
	}

	//> for merged view
	std::vector<SegIndex>::iterator its;
	if (notrack == -1)
	{
		//> if not a segment creation find the old one
		if (action != 2)
		{
			bool ok = false;
			for (notrack = 0; notrack < m_nbTracks && !ok ; ++notrack)
			{
				if ( (ok=m_indexes[notrack][type].contains(old_id)) ) break;
			}
			if (!ok)
			{
				TRACE_D << "IN updateTrack : old_id not found " << old_id << endl;
				return;
			}
		}
		//> else get the track number where segment is created
		else
			notrack = m_dataModel.getElementSignalTrack(mid);
	}

	//>> LOCK UPDATE
	if (action == 8 && m_segmentTrack[notrack][type])
	{
		// only freeze segment update
		m_segmentTrack[notrack][type]->startUpdate();
		return;
	}

	//>> COMMIT UPDATE
	if (action == 9 && m_segmentTrack[notrack][type])
	{
		// only apply all segment updates
		m_segmentTrack[notrack][type]->commitUpdate();
		return;
	}

	//>> other case : do updates
	//	 TRACE_D << " UPDATE TRACK old_id = " << old_id << endl;

	//> Restore current segment information in case it has changed...
	if (notrack != -1 && m_currentSegment[notrack][type].getId() == old_id)
		m_dataModel.getSegment(old_id, m_currentSegment[notrack][type],
		        notrack, true, true);

	//> Search for segment track to be updated
	if (m_segmentTrack[notrack].find(type) == m_segmentTrack[notrack].end())
		return;

	if (!m_segmentTrack[notrack][type])
		return;

	//> Specify audio widget we're starting an update
	m_segmentTrack[notrack][type]->startUpdate();

	bool is_visible = m_segmentTrack[notrack][type]->is_visible() ;

	//> Searching for segment to be updated
	if ( m_indexes[notrack][type].contains(old_id) )
	{
		//> If total update, let's delete segment from segment list and segment cache
		if (!text_only)
		{
//				TRACE_D << "\t DELETING Data for " << old_id << std::endl ;
			m_segmentTrack[notrack][type]->removeSegment(old_id);
			// Delete segment from cache index, will add it later
			m_indexes[notrack][type].remove(old_id);
			old_id="";
		}
	}
	else
		old_id="";

	SignalSegment s;

	if (!id.empty())
	{
		//-- Possible to get there wt error because some unit are not anchored
		if (!m_dataModel.getSegment(id, s, notrack, true, true))
		{
			TRACE_D << "\tupdate track ID no longer exists: " << id << std::endl;
			id = ""; // segment doesn't exist anymore
		}
	}

	if (!id.empty())
	{
		const string& label = getView(notrack)->getSegmentText(type, id, s.getEndId());
		const string& type = m_dataModel.getElementType(id) ;
		const string& graphtype = m_dataModel.getGraphType(id) ;

		//> Total segment update
		if (!text_only)
		{
			//			TRACE_D << "\t TOTAL UPDATE for : " << id << std::endl ;
			//			TRACE_D << "\t\t : offset= " <<  s.getStartOffset() << std::endl ;

			// add new cache index element
			m_indexes[notrack][type].add(id, s.getStartOffset(), s.getOrder());

			//> display highlight ?
			string disable_highlight =
			        m_configuration["Signal,disable_highlight"];
			std::vector<string> v_disable_highlight_types;
			mini_parser(';', disable_highlight, &v_disable_highlight_types);
			bool skip_highlight = is_in_svect(v_disable_highlight_types, type);

			//> force single line ?
			string forced_single_line =
			        m_configuration["Signal,force_single_line"];
			std::vector<string> v_singleLine;
			mini_parser(';', forced_single_line, &v_singleLine);
			bool hidden_order = is_in_svect(v_singleLine, type);

			// add segment track element
			string color = getSegmentColor(s.getId(), type);
			bool is_speech_type = (type == m_dataModel.conventions().getSpeakerType(graphtype));
			bool can_skip = (!is_speech_type || !m_dataModel.isSpeechSegment(s.getId())) ;

			m_segmentTrack[notrack][type]->addSegment(s.getId(), s.getStartOffset(), s.getEndOffset(), label.c_str(),
															color.c_str(), can_skip, m_dataModel.getOrder(s.getId()),
															hidden_order, skip_highlight, m_signalView->getAudioTrack(s.getTrack()));
		}
		//> Only text segment update
		else
		{
			//			TRACE_D << "\t teXt UPDATE for : " << id << std::endl ;

			if ( old_id.empty() )
			{
				/*	m_indexesIds[notrack][type][no].id != id  */
				MSGOUT << "update track segment text : non-existing segment " << id << endl;
			}
			else if (m_segmentTrack[notrack][type])
				m_segmentTrack[notrack][type]->updateSegmentText(id, label);
		}
	}

	//> Call for remanaging segment
	if (m_segmentTrack[notrack][type])
	{
		m_segmentTrack[notrack][type]->commitUpdate();
		bool exists = ( find(m_trackTypes.begin(), m_trackTypes.end(), type) != m_trackTypes.end() ) ;
		bool visible = ( find(m_visibleTracks.begin(), m_visibleTracks.end(), type) != m_visibleTracks.end() ) ;
		if (is_visible && exists && visible)
			m_segmentTrack[notrack][type]->show();
	}

//	TRACE << "AnnotationEditor::updateTrack: ********************** END\n\n\n" << std::endl ;
	//	TRACE << "    ADD SEGMENT= " << s.getId() <<"="<< s.getStartOffset() << " - " << s.getEndId() <<"="<<  s.getEndOffset()  <<  " - " << label << endl;

	return;
}

void AnnotationEditor::onSignalPopulatePopup(int notrack, int x, int y,
        Gtk::Menu* trackmenu)
{
	SignalPopupMenu* menu = Gtk::manage(new SignalPopupMenu(this, trackmenu));

	Gtk::TextIter iter = getView(notrack)->getBuffer()->getCursor();

	menu->popup(iter, x - 5, y - 5, 0);
}

/*==============================================================*
 *   															*
 * 				 TEXT-SIGNAL SYNCHRONISATION					*
 *   															*
 *==============================================================*/

//> connected to m_signalView->signalCursorChanged signal
//> called after m_signalView->signalSelection changed
void AnnotationEditor::synchroTextToSignalIdle(float startTime, float endTime,
        bool force)
{
	a_lastActiveTime = time(0);
	if (m_signalView == NULL || (m_inhibateSynchro == true && m_syncSource==NULL) || startTime == -1 || !m_loaded)
		return;
	synchroTextToSignal(startTime, endTime, force, false);
}

/**
 * synchroTextToSignal callback
 *   connected to signal view signalCursorChanged() and signalSelectionChanged() events
 *   if text-signal synchronisation activated, scrolls text view to make corresponding
 *   text position visible and highlights corresponding text element
 *   - current turn
 *   - current segment
 *   - current word if corresponding timecodes available.
 *
 *  @param startTime current cursor position / selection start
 *  @param endTime selection end / -1 if signalCursorChanged
 */
bool AnnotationEditor::synchroTextToSignal(float startTime, float endTime,
        bool force, bool inthread)
{
	if (m_signalView == NULL || (m_inhibateSynchro == true && m_syncSource == NULL) )
		return false;

	double elapsed = m_TTSTimer.elapsed();
	m_TTSTimer.start();

	if (!(force || (elapsed > 0.2 && elapsed < m_synchroInterval
	        && !m_signalView->is_playing()))
	        && (m_configuration["synchro_text_to_signal"] == "false"))
		return false;

	vector<string>::iterator it;

	int notrack, i;
	for (i = 0; i < m_currentSegment.size(); i++)
	{
		notrack = i;
		if (notrack != m_activeTrack && getView(notrack) != m_syncSource )
		{
			for (it = m_synchroTypes.begin(); it != m_synchroTypes.end(); ++it)
			{
				if (m_indexes[notrack].find(*it) != m_indexes[notrack].end())
					synchroTextToSignalWithType(*it, startTime, endTime, notrack, inthread);
			}
		}
	}
	if ( getView(m_activeTrack) != m_syncSource )
	{
		for (it = m_synchroTypes.begin(); it != m_synchroTypes.end(); ++it)
		{
			if (m_indexes[m_activeTrack].find(*it) != m_indexes[m_activeTrack].end())
				synchroTextToSignalWithType(*it, startTime, endTime, m_activeTrack, inthread);
		}
	}
	m_lastTrack = m_activeTrack;

//	float f4 = m_TTSTimer.elapsed();
//	cout << "@@@@@@@@@@   DOne  AnnotationEditor::synchroTextToSignal in " << f4 <<  endl;

	return false;
}

string AnnotationEditor::synchroTextToSignalWithType(const string& type,
        float startTime, float endTime, int notrack, bool inthread)
{
	float epsilon = 0.001 ;
	float startsel, endsel ;

	if ( notrack >=0 && mapTrackDelays[notrack]!=0 )
	{
		if ( startTime!=-1 )
			startTime = startTime - mapTrackDelays[notrack] ;
		if ( endTime!=-1 )
			endTime = endTime - mapTrackDelays[notrack] ;
	}

	if (m_indexes.size() == 0 || m_indexes[notrack].size() == 0)
		return "";

	bool hasSelection = m_signalView->getSelection(startsel, endsel);

	//if ( m_signalView->hasSelection() ) epsilon = 0.001;
	if (hasSelection)
	{
		if (startTime == startsel)
			startTime += epsilon;
		if (startTime == endsel)
			startTime -= epsilon;
	}
//	bool btype= (type==m_dataModel.mainstreamBaseType());
//cout << "@@@ synchroText type=" << type << " current=" << m_currentSegment[notrack][type].getId() << endl;
	if (m_currentSegment[notrack][type].getId() != "")
	{
		//> if audio cursor still in same segment
		//>   - if still playing audio -> don't have to move, so do nothing
		//>   - else if text cursor not well set -> move it to appropriate position.

		if (m_currentSegment[notrack][type].getStartOffset() <= startTime
		        && m_currentSegment[notrack][type].getEndOffset() > startTime
		        && (m_activeTrack == m_lastTrack))
		{
			const string& id = m_currentSegment[notrack][type].getId();
			if (! m_signalView->is_playing() && type == m_dataModel.mainstreamBaseType() )
			{
				getView(notrack)->setCursor(id, false, true);
			}
			return id;
		}
	}

	//> Find segment corresponding to current time
	string id = m_indexes[notrack][type].getIdAtTime(startTime);
	if ( id.empty() )
	{
		id = m_dataModel.getByOffset(type, startTime, notrack);
//		gdk_beep();
	}

	if (id != "" && id != m_currentSegment[notrack][type].getId())
		m_dataModel.getSegment(id, m_currentSegment[notrack][type], notrack, true, true);

	if (getView(notrack)->is_visible())
	{
		bool doHighlight = false;
		if (m_highlightMode == 3)
			doHighlight = (notrack == m_activeTrack);
		else
			doHighlight = (m_highlightMode == 2 || m_highlightMode == notrack);
		id = synchroTextToSignalWithTypeAndTrack(type, id, notrack, doHighlight, inthread);
	}
//	float f4 = e.elapsed();
//	cout << "@@@@@@@@@@   DOne " << type << " synchro in " << f4 << " f1=" << f1 << " f3=" << f3 << endl;
	return m_currentSegment[notrack][type].getId();
}

string AnnotationEditor::synchroTextToSignalWithTypeAndTrack(const string& type, const string& id, int notrack, bool do_highlight, bool inthread)
{
	//> process highlight

	bool is_basetype = (type == m_dataModel.mainstreamBaseType());

	bool scrolled = false;
	if (inthread)
		gdk_threads_enter();
	// PROCESS HGHLIGHT
	if (do_highlight && m_highlightMode != -1)
	{
		for (guint i = 0; i < m_textView.size(); ++i)
		{
			if (m_textView[i]->is_visible() && (i == notrack || i == (m_textView.size() - 1)))
				m_textView[i]->setHighlight(type, id, is_basetype, notrack);
				scrolled = is_basetype;
		}
	}

	if (is_basetype)
	{
		//		if (notrack==m_activeTrack)
		if (! m_signalView->is_playing() )
		{
			getView(notrack)->setCursor(id);
		}

		if ( ! scrolled ) {
//			cout << "  .... scroll view"  << endl;
			getView(notrack)->scrollViewToId(id);
		}
		//		if ( inthread ) gdk_flush();
	}
	if (inthread)
		gdk_threads_leave();
	//}
	return id;
}

/**
 * setTextCursor callback
 *   connected to signal view signalPlayedPaused() event
 *   if text-signal synchronisation activated, and signal paused,
 *   set text cursor position to start of text element corresponding to signal cursor
 *   - current turn
 *   - current segment
 *   - current word if corresponding timecodes available.
 *
 *  @param play	true if played, false if paused
 */
void AnnotationEditor::setTextCursor(bool play)
{
	if (play || m_configuration["synchro_text_to_signal"] == "false")
		return;

	string id = "";
	vector<string>::reverse_iterator it;
	for (it = m_synchroTypes.rbegin(); id == "" && it != m_synchroTypes.rend(); ++it)
	{
		if (m_indexes[0].find(*it) != m_indexes[0].end())
			id = synchroTextToSignalWithType(*it, m_signalView->getCursor(),
			        -1.0);
	}

}

/**
 * synchroSignalToText callback
 *   connected to text view signalSetCursor() event and signalSelectionChanged event
 *   if text-signal synchronisation activated, set signal cursor position to
 *   start time of current text element (turn / word)
 *  @param view current view
 *  @param pos current text position in view
 */

void AnnotationEditor::synchroSignalToText(const Gtk::TextIter& pos,
        bool force, AnnotationView* view)
{
//	TRACE_D << "IN synchroSignalToText pos=" << pos << " doit=" << m_configuration["synchro_signal_to_text"] << " inhib=" << m_inhibateSynchro << endl;
	if (m_signalView == NULL
			|| (m_inhibateSynchro == true && view != m_syncSource)
			|| (!force && m_configuration["synchro_signal_to_text"] == "false"))
		return;

	if (!m_signalView->is_mapped())
	{
		return;
	}
	// get text mark at or near pos
	string id("");

	id = view->getTextCursorElement() ;

	if (id == "")
	{
		MSGOUT << "NO MARK ID FOUND AT POS " << pos.get_offset() << endl;
		return;
	}

	// TODO use getParentElement when modifying the track mechanism (hidden track words)
//	const string& baseid = m_dataModel.getParentElement(id, m_dataModel.segmentationBaseType());
	const string& baseid = m_dataModel.getAnchoredBaseTypeStartId(id);
	int notrack = m_dataModel.getElementSignalTrack(baseid);

	//condition <notrack!=m_activeTrack> added to enable track selection switching when passing form a track to the other
	if (m_currentSegment.size() > 0 && notrack < m_currentSegment.size() && m_currentSegment[notrack].size() > 0)
	{
//		TRACE_D << "@@@@@@@@ DO synchroSignalToText pos=" << pos << " baseid=" << baseid << endl;
		if ((m_currentSegment[notrack][m_dataModel.mainstreamBaseType()].getId() != baseid)
				|| notrack != m_activeTrack)
		{
			SignalSegment& s = m_currentSegment[notrack][m_dataModel.mainstreamBaseType()];
			m_dataModel.getSegment(baseid, s, view->getViewTrack(), true, true);
			float offset = s.getStartOffset();
			float cursor = m_signalView->getCursor();

//			TRACE_D << "       ... synchroSignalToText baseid=" << baseid << " ->  offset=" << offset << endl;

			// 	if cursor not in current segment, change it
			if ((offset < cursor || s.getEndOffset() >= cursor))
			{
				// 	if cursor already in current segment, do not change it
				inhibateSynchro();
				m_syncSource = view;
				m_signalView->setSelection(-1.0, -1.0);
				m_signalView->setSelectedAudioTrack(notrack);
				m_signalView->setCursor(offset, true, true);
				inhibateSynchro(false);
				m_syncSource = NULL;
			}
			// and also highlight current id
			// HERE : if more than one view -> will have to synchronize other view !!
			view->clearHighlight();
			vector<string>::iterator it;
			if (m_highlightMode != -1)
			{
				for (it = m_synchroTypes.begin(); it != m_synchroTypes.end(); ++it)
				{
					id = view->getBuffer()->anchors().getPreviousAnchorId(const_cast<Gtk::TextIter&> (pos), *it);
					if (!id.empty())
					{
						//view->setHighlight(*it, id, true,view->getViewTrack());
						bool is_basetype = (*it == m_dataModel.mainstreamBaseType()) ;
						view->setHighlight(*it, id, is_basetype, notrack);
					}
				}
			}
		}
	}
	//	TRACE_D << "OUT synchroSignalToText " << pos.get_line() << "." << pos.get_line_offset() << endl;
}

/*
 * 0: highlight track 1 (indice 0)
 * 1: highlight track 2 (indice 1)
 * 2: highlight both track
 * 3: highlight selected track
 */
void AnnotationEditor::setHighlightMode(int mode)
{
	m_highlightMode = mode;
	int toClear = -1;

	if (mode == 0 || (mode == 3 && m_activeTrack == 0))
		toClear = 1;
	else if (mode == 1 || (mode == 3 && m_activeTrack == 1))
		toClear = 0;
	else if (mode == -1)
		toClear = 2;
	if (toClear != -1)
	{
		for (guint i = 0; i < m_textView.size(); ++i)
			m_textView[i]->clearHighlight("", toClear);
	}
}

/*
 * 1: hide all qualifiers
 * 2: hide events
 * 3: hide named entities
 * 4: hide unknown/invalid qualifiers
 * 5: hide words
 */
void AnnotationEditor::setHiddenTagsMode(int mode)
{
	// save last hidden mode
	if (m_hiddenTagsMode != -1)
		m_lastModeWhenHidden = m_hiddenTagsMode;

	// actualize mode
	m_hiddenTagsMode = mode;

	// apply changements
	for (guint i = 0; i < m_textView.size(); ++i)
		m_textView[i]->hideTags(m_hiddenTagsMode);
}

//
// activate / inhibate undo manager
//
void AnnotationEditor::setUndoableActions(bool b)
{
	for (guint i = 0; i < m_textView.size(); ++i)
		m_textView[i]->setUndoableActions(b);

	m_dataModel.setInhibUndoRedo(!b);
}

/**
 * user activity tracking
 * returns the time of last user action (in seconds).
 */

time_t AnnotationEditor::lastActiveTime() const
{
	time_t t1 = a_lastActiveTime;
	for (guint i = 0; i < m_textView.size(); ++i)
		if (t1 < m_textView[i]->lastEditTime())
			t1 = m_textView[i]->lastEditTime();
	return t1;
}

void AnnotationEditor::goToPosition()
{
	//> no audio, out !
	if (!m_signalView)
		return;

	GoToDialog dialog;
	int rep = dialog.run();
	if (rep == Gtk::RESPONSE_APPLY)
	{
		float seconds = dialog.getPosition();
		m_signalView->setSelection(-1.0, -1.0);
		m_signalView->setCursor(seconds);
	}
}

int AnnotationEditor::getSignalSelection(float& start, float& stop)
{
	start = -1 ;
	stop = -1 ;

	if (!m_signalView)
		return -1 ;

	m_signalView->getSelection(start, stop) ;

	if (start==-1 || stop ==-1)
		return 0 ;
	else
		return 1 ;
}

void AnnotationEditor::saveSelectionSignal()
{
	//> no audio, out !
	if (!m_signalView)
		return;
	//> no selection, out
	if (!m_signalView->hasSelection())
		return;
	//> several audio, out !
	//TODO allo this
	SignalConfiguration& signalCfg = dataModel().getSignalCfg();
	std::vector<string> names = signalCfg.getPaths("audio");
	if (names.size() > 1)
	{
		//> 2 files but not same path: we're in multiaudio, block !
		if ((names.size() == 2 && (names[0] != names[1])) || names.size() > 2)
		{
			dlg::error(_("Can't save audio selection with multi file audio transcription mode"));
			return;
		}
	}
	//> streaming mode, out !
	else if (m_signalView->isStreaming())
	{
#ifdef __APPLE__
		dlg::error(_("Can't save audio selection with streaming mode"), m_top);
#else
		dlg::error(_("Can't save audio selection with streaming mode"));
#endif
		return;
	}

	float start, end;
	m_signalView->getSelection(start, end);
	string name = signalCfg.getFirstFile("audio");

	SaveAudioDialog dialog(name, start, end);
	int rep = Gtk::RESPONSE_HELP;
	while (rep != Gtk::RESPONSE_APPLY && rep != Gtk::RESPONSE_CANCEL && rep
	        != Gtk::RESPONSE_CLOSE && rep != -4)
	{
		rep = dialog.run();
	}
	if (rep == Gtk::RESPONSE_APPLY)
	{
		string new_path = dialog.getSelectionPath();

		// -- Renaming Extension -> Wav --
		int ext_pos = new_path.find_last_of(".");

		if (ext_pos == string::npos)
			new_path += ".wav";
		else
			new_path = new_path.substr(0, ext_pos) + ".wav";

		TRACE_D << "SAVE AUDIO SELECTION:> " << new_path << " (" << start
		        << " - " << end << ")" << endl;

		bool saved = m_signalView->saveSelection(start, end, (char*) new_path.c_str(), true);
		if (!saved)
		{
			string msg = _("An error occurred when saving signal...") ;
			dlg::error(msg, m_top);
			::g_unlink(new_path.c_str());
			return ;
		}
	}

	TRACE_D << "********/*******/*********/********/******/" << std::endl;
}

// --- ExportSelectionTo ---
void AnnotationEditor::exportSelectionTo(string program)
{
	// -- Local variables --
	float start, end;
	int fh;
	string command = "";

	// -- Temp dir
	string tmpFile = Glib::get_tmp_dir() ;

	// -- Obtaining temporary unique ID --
	tmpFile = Glib::build_filename(tmpFile, "wavfile_XXXXXX.wav") ;
	fh = Glib::mkstemp(tmpFile);

	// -- Save unformatted file name for praat command
	string tmpFile_unformatted = tmpFile ;

#ifdef _WIN32
	closesocket(fh);
#else
	close(fh);
#endif

	// -- Signal Extraction --
	if (!m_signalView)
		return;

	m_signalView->getSelection(start, end);

	// -- No selection -> Extractin' whole medium --
	if (start == -1.0 || end == -1.0)
	{
		Log::trace() << "AnnotationEditor --> Extracting Whole Signal" << std::endl;
		start = 0.0;
		end = m_signalView->signalLength();
	}

	bool saved = m_signalView->saveSelection(start, end, (char*) tmpFile.c_str(), false);
	if (!saved)
	{
		string msg = _("An error occurred when saving signal...") ;
		dlg::error(msg, m_top);
		::g_unlink(tmpFile.c_str());
		return ;
	}

#ifdef _WIN32
	tmpFile = string("\"") + tmpFile + "\"" ;
#endif

	// -- Lookin' up for external path --
	string externalPath = globalOptions.getParameterValue("General", "External," + program);
	Log::out() << "~~~ looking for external tool : " << externalPath << std::endl ;

	if (externalPath == "" || !Glib::file_test(externalPath, Glib::FILE_TEST_EXISTS) )
	{
		// -- Searching Executable --
		command = Glib::find_program_in_path(program);

		if (command != "")
		{
			globalOptions.setParameterValue("General", "External", program, command, true);
			globalOptions.save();

			if (program != "praat")
				command += " " + tmpFile;
		}
		else
		{
			Glib::ustring errorMessage = Glib::ustring(_("Unable to find external : ") + program + "\n"+ _("Please install it or check binary path."));

			Glib::ustring dir = Glib::get_home_dir() ;
			dir = Glib::build_filename(dir, ".TransAG") ;
			dir = Glib::build_filename(dir, "userAG.rc") ;
			Glib::ustring msg =	Glib::ustring(_("Unable to find external : ")) + externalPath ;
			msg= msg + "\n" + Glib::ustring(_("Binary path can be changed in the file")) ;
			msg = msg + " " + dir ;
			msg = msg + "\n" + _("in the section External") ;
			dlg::warning(errorMessage, msg, getTopWindow(), true) ;
		}
	}
	else
	{
#ifdef _WIN32
		/* Protect with cote */
		externalPath = string("\"") + externalPath + "\"" ;
#else
		/* White space are bad */
		//TODO Fix replace_in_string and make it beautiful
		externalPath = replace_in_string(externalPath, " ", "||") ;
		externalPath = replace_in_string(externalPath, "||", "\\ ") ;
#endif

		// -- Command in conf --
		if (program == "praat")
			command = externalPath;
		else
		{
			command += externalPath + " ";
			command += tmpFile;
		}
	}

	// -- Run Command --
	if (command != "")
	{
		try
		{
			Log::out() << "~~~ invoking external tool : " << command << std::endl ;
			Glib::spawn_command_line_async(command);
		}
		catch (Glib::SpawnError e)
		{
			Glib::ustring errorMessage = _("Error while launching external tool...");
			Glib::ustring msg = e.what() ;
			Log::trace() << "~~~ invoking external tool --> Glibmm SpawnError : " << e.what() << std::endl;

			dlg::error(errorMessage, msg, getTopWindow(), false) ;
			return;
		}
	}
	else
	{
		// -- Temporary File Removal --
		::g_unlink(tmpFile.c_str());
		return;
	}

	// -- Praat Case - Commands sent with sendpraat subroutine --
	if (program == "praat")
	{
#ifdef _WIN32
		//DIRTY : wait for being sure praat had time enough to be launched
		USLEEP(3*1000*1000) ;
#else
		sleep(3) ;
#endif
		char* praatError = NULL;
		int tries = 0;
		string praatCommand;

		praatCommand = string("Read from file... ") + tmpFile_unformatted ;
		praatCommand += "\nEdit";
		Log::trace() << "Praat Command --> " << praatCommand << std::endl;

		// It can happen that praat returns error message but manages to load audio
		// => we think the proccess failed but 2 or more windows are opened, and then we open the error message : that's ugly
		// Wait for him with the previous sleep, and nothing more
		praatError = sendpraat(NULL, "praat", 2, praatCommand.c_str());

/*		// -- Waiting for Praat Program --
		while (tries < 5)
		{
			praatError = sendpraat(NULL, "praat", 2, praatCommand.c_str());
			
			if (!praatError)
			{
				Log::trace() << "Praat Command successfully launched " << std::endl;
				return;
			}

			tries++;
			Log::trace() << "Praat Command Failed -> " << string(praatError) << std::endl;

			// -- Sleeping 500 ms --
			USLEEP(500 * 1000);
		}
*/
		if (praatError)
		{
			Glib::ustring errorMessage = _("Error while launching external tool...");
			// See below why we don't want to display error msg anymore
//			dlg::error(errorMessage, getTopWindow());
			Log::err() << errorMessage << std::endl ;
		}
	}
}

void AnnotationEditor::onExternalAction(ToolLauncher::Tool* tool)
{
	if (!tool)
		return;

	tool->configureOptions(getFileName());
	ToolLauncher* toolLauncher = ToolLauncher::getInstance();
	toolLauncher->launch(tool);
}

/*==============================================================*
 *   															*
 *
 *   															*
 *==============================================================*/

bool AnnotationEditor::langAndConventionsDlg(bool is_new_file, string& lang,
        string& conventions, string& forced_convention)
{
	bool ok = false;

	Gtk::Window* top = getTopWindow();
	if (top == NULL)
	{
		MSGOUT << "got NULL top window pointer" << endl;
		return false;
	}

	lang = m_modelCfg["Default,lang"];
	vector<string> p_lang;
	vector<string> l_lang;
	conventions = m_modelCfg["Default,convention"];
	vector<string> p_conv;
	vector<string> l_conv;
	guint i;
	std::map<string, string>::iterator it;

	StringOps(m_modelCfg["Configuration,lang"]).split(p_lang, ";,");
	string loc = ISO639::getLocale();

	for (i = 0; i < p_lang.size(); ++i)
		l_lang.push_back(
		        ISO639::getLanguageName(p_lang[i].c_str(), loc.c_str()));
	lang = ISO639::getLanguageName(lang.c_str(), loc.c_str());

	StringOps(m_modelCfg["Configuration,conventions"]).split(p_conv, ";,");
	for (i = 0; i < p_conv.size(); ++i)
	{
		it = m_modelCfg.find("Configuration," + p_conv[i]);
		if (it == m_modelCfg.end())
			l_conv.push_back(p_conv[i]);
		else
			l_conv.push_back(it->second);
		if (conventions == p_conv[i])
			conventions = l_conv[i];
	}

	string title;
	if (is_new_file)
		title = _("New annotation file");
	else
		title = _("Select main transcription language");

	DialogSelectConventions* dlg = new DialogSelectConventions(*top, title,
	        lang, conventions, l_lang, l_conv, forced_convention);

	if (dlg->run() == Gtk::RESPONSE_OK)
	{
		for (i = 0; i < l_lang.size() && l_lang[i] != lang; ++i)
			;
		if (i < l_lang.size())
			lang = p_lang[i];
		for (i = 0; i < l_conv.size() && l_conv[i] != conventions; ++i)
			;
		if (i < l_conv.size())
			conventions = p_conv[i];
		ok = true;
	}
	dlg->hide();
	delete dlg;
	return ok;
}

/*
 * forward edit speaker message to caller
 */
void AnnotationEditor::editSpeaker(string id, bool modal)
{
	m_signalEditSpeaker.emit(id, modal);
}

void AnnotationEditor::externalIMEcontrol(bool activate)
{
	if (m_loaded)
	{
		//> prepare for next focus
		for (guint i = 0; i < m_textView.size(); i++)
		{
			m_textView[i]->setIMEstatus(activate);
		}
		//> do it for current view
		AnnotationView* view = getActiveView();
		view->externalIMEcontrol(activate);
	}
}

/** RESET **/
void AnnotationEditor::reset_autosave(int autosave_period)
{
	if (consultationMode)
		return;

	m_autosave.disconnect();
	m_autosave = Glib::signal_timeout().connect(sigc::bind<bool>(sigc::mem_fun(
	        *this, &AnnotationEditor::autosaveFile), true), autosave_period
	        * 1000);
}

void AnnotationEditor::reset_activity(int activity_period)
{
	m_activityWatcher->setIdleTimeDelay(activity_period);
}

void AnnotationEditor::reset_autosetLanguage(bool autoset)
{
	std::vector<AnnotationView*>::iterator it;
	for (it = m_textView.begin(); it != m_textView.end(); it++)
	{
		if (*it)
			(*it)->setAutosetLanguage(autoset);
	}
}

void AnnotationEditor::reset_stopOnClick(bool autoset)
{
	if (m_signalView)
		m_signalView->setStopOnClick(autoset);
}

void AnnotationEditor::reset_timeScale(bool show)
{
	if (m_signalView)
		m_signalView->show_scale(show);
}

/* SPELL */
//void AnnotationEditor::reset_speller()
//{
//	std::vector<AnnotationView*>::iterator it;
//
//	//> Configure display of editor for each view
//	string speller_enabled = m_configuration["Speller,enabled"];
//	string speller_directory;
//	if (speller_enabled == "true")
//		speller_directory = m_configuration["Speller,directory"];
//	else
//		speller_directory = "nospell";
//
//	for (it = m_textView.begin(); it != m_textView.end(); it++)
//	{
//		if (*it)
//		{
//			(*it)->detachSpeller();
//			m_lang = m_dataModel.getAGSetProperty("lang");
//			(*it)->configureSpeller(speller_directory, m_lang);
//			(*it)->speller_recheck_all();
//		}
//	}
//}

void AnnotationEditor::reset_entityTags_bg(bool use_bg)
{
	std::vector<AnnotationView*>::iterator it;
	for (it = m_textView.begin(); it != m_textView.end(); it++)
	{
		if (*it)
			(*it)->set_entityTag_bg(use_bg);
	}
}

void AnnotationEditor::setFontStyle(const string& font, const string& mode)
{
	for (guint i = 0; i < m_textView.size(); i++)
	{
		m_textView[i]->setFontStyle(font, mode);
	}
}

void AnnotationEditor::setViewStyle(Glib::ustring rc)
{
	for (guint i = 0; i < m_textView.size(); i++)
	{
		m_textView[i]->set_name(rc);
	}
}

bool AnnotationEditor::viewHasFocus()
{
	if (m_activeView)
		return m_activeView->has_focus();
	else
		return false;
}

void AnnotationEditor::setAudioColors()
{
	if (!m_signalView)
		return;

	//> adjust user colors
	string element, key;

	element = TAG_COLORS_AUDIO_WAVE_ACTIVE_BACKGROUND;
	key = "Colors-audio," + element;
	m_signalView->set_signal_color(element, m_colorsCfg[key]);

	element = TAG_COLORS_AUDIO_WAVE_BACKGROUND;
	key = "Colors-audio," + element;
	m_signalView->set_signal_color(element, m_colorsCfg[key]);

	element = TAG_COLORS_AUDIO_WAVE_FOREGROUND;
	key = "Colors-audio," + element;
	m_signalView->set_signal_color(element, m_colorsCfg[key]);

	element = TAG_COLORS_AUDIO_WAVE_SELECTION;
	key = "Colors-audio," + element;
	m_signalView->set_signal_color(element, m_colorsCfg[key]);

	element = TAG_COLORS_AUDIO_WAVE_DISABLED;
	key = "Colors-audio," + element;
	m_signalView->set_signal_color(element, m_colorsCfg[key]);

	element = TAG_COLORS_AUDIO_TIP_FOREGROUND;
	key = "Colors-audio," + element;
	m_signalView->set_signal_color(element, m_colorsCfg[key]);

	element = TAG_COLORS_AUDIO_TIP_BACKGROUND;
	key = "Colors-audio," + element;
	m_signalView->set_signal_color(element, m_colorsCfg[key]);

	element = TAG_COLORS_AUDIO_WAVE_SEGMENTEND;
	key = "Colors-audio," + element;
	m_signalView->set_signal_color(element, m_colorsCfg[key]);

	element = TAG_COLORS_AUDIO_WAVE_CURSOR;
	key = "Colors-audio," + element;
	m_signalView->setCursorColor(m_colorsCfg[key]);
}

void AnnotationEditor::setEditorColors()
{
	for (guint i = 0; i < m_textView.size(); i++)
	{
		m_textView[i]->resetColors();
	}
}

void AnnotationEditor::setUseTooltip(bool value)
{
	for (guint i = 0; i < m_textView.size(); i++)
	{
		m_textView[i]->setUseTooltip(value);
	}
}

bool AnnotationEditor::isEditMode()
{
	if (m_configuration["mode"] == "EditMode" && !consultationMode)
		return true;
	else
		return false;
}

string AnnotationEditor::getEditMode()
{
	std::map<string, string>::iterator it = m_configuration.find("mode");
	if (it != m_configuration.end())
		return it->second;
	else
		return "";
}

/**
 * Displays message corresponding to loading.\n
 * Called from loadFile if loading fails, or from terminateDisplayLoading
 * if loading succeeds.
 * @param success		True or False
 * @param inThread		True if calling from thread or signal idle process.
 */
void AnnotationEditor::logLoading(bool success, bool inThread)
{
	if (logLoadingDone)
		return ;
	else
		logLoadingDone = true ;

	Log::out() << "Reporting mode [" << reportLevel << "]" << std::endl;

	// user has canceled loading: don't need to see log
	if (m_loading_cancelled)
		return;

	ModelChecker* log = m_dataModel.getModelChecker();

	//>
	// 1- no log: skip
	// 2- report level set to 0: skip
	// 3- nothing to display: skip
	// 4- report level set to 2 and no errors : skip
	if (!log || reportLevel == 0 || log->isFullyCorrect() || (!log->hasErrors()
	        && reportLevel == 2))
	{
		// Actualize file state <!> except for view mode <!>
		if (success && fileModified() && !consultationMode)
			m_signalFileModified.emit(true);
		return;
	}

	/*** Actualize status ***/
	if (success)
	{
		// If we have added graph (tag files) or if file is in modified state,
		// actualize tab state
		if ((log->getNbAddedGraphs() > 0 && m_dataModel.isLoadedTagFormat())
		        || fileModified())
			m_signalFileModified.emit(true);
	}

	/*** Display log ***/
	CheckerDialog dial(getDataModelPtr(), getFileName(), isEditMode());
	if (success)
		dial.signalModelModified().connect(sigc::bind<bool>(sigc::mem_fun(this, &AnnotationEditor::refresh), true));
	if (!hasSignalView() && success)
		inThread = true;
	if (inThread)
		gdk_threads_enter() ;
	dial.run();
	if (inThread)
		gdk_threads_leave() ;
}

void AnnotationEditor::onModelUpdated(bool b)
{
	onFileModified();
}

// --- SetPlay ---
void AnnotationEditor::setPlay(bool b, bool inhibate_synchro)
{
	if ( !m_signalView )
		return ;

	if (inhibate_synchro)
		inhibateSynchro(true) ;
	m_signalView->setPlay(b);
	if (inhibate_synchro)
		inhibateSynchro(false) ;
}

/*==============================================================================*
 *										*
 *								TOGGLE OPTIONS	*
 *   										*
 *==============================================================================*/

int AnnotationEditor::toggleHideTags(bool emit_signal)
{
	// current is hidden, save the mode
	if (m_hiddenTagsMode != -1)
	{
		m_lastModeWhenHidden = m_hiddenTagsMode;
		m_hiddenTagsMode = -1;
	}
	// current is not hidden, restore the last hidden mode
	else
	{
		m_hiddenTagsMode = m_lastModeWhenHidden;
	}

	//> HIDDING TAGS
	if (m_hiddenTagsMode != -1)
	{
		m_configuration["sav_mode"] =m_configuration["mode"];
		setMode("BrowseMode");
		// tags are hidden, can't edit

		Glib::RefPtr<Gtk::Action> action = m_actionGroups["file"]->get_action(
		        "filemode");
		if (action)
			action->set_sensitive(false);
		if (emit_signal)
			signalTagHiddenChanged().emit(m_hiddenTagsMode);

		// TODO: BUG GDK with HiddenTags & set_pixels_below_lines
		getViewInterline(m_tagHidden_lastInterlineAbove,
		        m_tagHidden_lastInterlineBelow);
		//> Dirty solution:
		//  change below interline to 0 for hidden mode
		//  and set double value for above value for compensating display appearance
		setViewInterline(m_tagHidden_lastInterlineAbove * 2, 0);
	}
	//> SHOWING TAGS
	else
	{
		//> actualize action group
		Glib::RefPtr<Gtk::Action> action = m_actionGroups["file"]->get_action(
		        "filemode");
		if (action)
			action->set_sensitive(true);

		// set old value
		setMode(m_configuration["sav_mode"]);

		//> set previous interline value
		if (m_tagHidden_lastInterlineAbove != -1
		        && m_tagHidden_lastInterlineBelow != -1)
			setViewInterline(m_tagHidden_lastInterlineAbove,
			        m_tagHidden_lastInterlineBelow);
		else
			setViewInterline(4, 4);

		//> tell GUI
		if (emit_signal)
			signalTagHiddenChanged().emit(m_hiddenTagsMode);
	}

	for (guint i = 0; i < m_textView.size(); i++)
	{
		if (m_textView[i])
			m_textView[i]->hideTags(m_hiddenTagsMode);
		else
			TRACE_D << "hideTags: NULL view" << std::endl;
	}

	return m_hiddenTagsMode;
}

string AnnotationEditor::toggleFileMode(bool emit_signal)
{
	string value = getEditMode();
	string new_mode = "";
	if (value == "BrowseMode")
		new_mode = "EditMode";
	else if (value == "EditMode")
		new_mode = "BrowseMode";

	if (!new_mode.empty())
	{
		setMode(new_mode);
		if (emit_signal)
			m_signalEditModeChanged.emit(new_mode);
	}

	return new_mode;
}

int AnnotationEditor::toggleHighlightMode(bool emit_signal)
{
	int mode = getHighlightMode();

	if (!isStereo())
	{
		if (mode == -1)
			mode = 3;
		else
			mode = -1;
	}
	else
	{
		//> if nohighlight mode, we change to highlight mode
		if (mode == -1)
		{
			// first try default mode
			mode = getConfigurationHighlightMode();
			// if default mode is set to nohighlight, set both highlight (default)
			if (mode == -1)
				mode = 2;
		}
		else
			mode = -1;
	}

	setHighlightMode(mode);

	if (emit_signal)
		m_signalHighlightChanged.emit(mode);

	return mode;
}

/*
 * synchro_signal_to_text
 * synchro_text_to_signal
 */
string AnnotationEditor::toggleSynchro(string type, bool emit_signal)
{
	const string& value = getOption(type);
	string new_value = "";
	if (value == "true")
		new_value = "false";
	else if (value == "false")
		new_value = "true";

	if (!new_value.empty())
	{
		setOption(type, new_value);
		m_signalSynchroChanged(type, new_value);
	}

	return new_value;
}

int AnnotationEditor::toggleDisplay(bool emit_signal)
{
	Log::out() << "\n ~~toggle display" << std::endl ;

	int res = -9;
	int mode = getActiveViewMode();

	if (mode == -2)
		res = -1;
	else if (mode == -1 || mode == 1 || mode == 0)
		res = -2;

	if (res != -9)
	{
		// Lock edition for correct view update. Will be release by
		// terminateDisplayLoading method
		setLocked(true);
//		disableThreadProtection = true ;
		disableViewThreads(true);
		changeActiveViewMode(res);
		disableViewThreads(false);
		m_signalDisplayChanged.emit(-2);
	}

	return res;
}

void AnnotationEditor::disableViewThreads(bool disable)
{
	for (guint i = 0; i < m_textView.size(); i++)
	{
		if (m_textView[i])
			m_textView[i]->disableThread(disable);
	}
}

void AnnotationEditor::setViewInterline(int above, int below)
{
	for (guint i = 0; i < m_textView.size(); i++)
	{
		if (m_textView[i])
			m_textView[i]->set_pixels_interline(above, below);
	}
}

void AnnotationEditor::getViewInterline(int& above, int& below)
{
	if (m_textView.size() > 0)
		if (m_textView[0])
			m_textView[0]->get_pixels_interline(above, below);
}

void AnnotationEditor::clearViewTag(std::string tagname)
{
	for (guint i = 0; i < m_textView.size(); i++)
	{
		if (m_textView[i])
			m_textView[i]->clearTag(tagname);
	}
}

void AnnotationEditor::setSensitiveViews(bool sensitive)
{
	for (guint i = 0; i < m_textView.size(); i++)
	{
		if (m_textView[i])
			m_textView[i]->set_sensitive(sensitive);
	}
}

void AnnotationEditor::setWithConfidence(bool value)
{
	for (guint i = 0; i < m_textView.size(); i++)
	{
		if (m_textView[i])
			m_textView[i]->setWithConfidence(value);
	}
}

void AnnotationEditor::setSensitiveSignalView(bool sensitive)
{
	if (m_signalView)
		m_signalView->set_sensitive(sensitive);
}

const string& AnnotationEditor::getOption(const string& option)
{
	return m_configuration[option];
}

int AnnotationEditor::resetDisplayMode()
{
	int mode = getActiveViewMode();
	changeActiveViewMode(mode, true);
	return mode;
}

/*==============================================================*
 *   															*
 *							VIDEO
 *   															*
 *==============================================================*/

void AnnotationEditor::onPageActivated(bool on)
{
	if (!videoManager)
		return;

	if (!on)
		videoManager->hideVideo();
	else
		videoManager->showVideo();
}

bool AnnotationEditor::hasVideo()
{
	return (videoManager != NULL);
}

void AnnotationEditor::setFrameBrowserResolution(int step)
{
	if (!videoFrameBrowser)
		return;

	videoFrameBrowser->setResolutionStep(step);
}

/*==============================================================*
 *   															*
 *							GTK EVENTS
 *   															*
 *==============================================================*/

/*
 * to allow drag&drop in  window
 */
bool AnnotationEditor::on_drag_drop(
        const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time)
{
	m_signalGtkDragTarget.emit(context, this, "");
	return false;
}

void AnnotationEditor::onViewDragAndDropReceived(const Glib::RefPtr<
        Gdk::DragContext>& context, string id)
{
	m_signalGtkDragTarget.emit(context, this, id);
}

void AnnotationEditor::addDragAndDropTarget(Gtk::TargetEntry target)
{
	// for me
	dragDropTargets.push_back(target);
	drag_dest_set(dragDropTargets, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);

	// for all view
	for (guint i = 0; i < m_textView.size(); i++)
	{
		if (m_textView[i])
			m_textView[i]->addDragAndDropTarget(target);
	}
}

/*==============================================================*
 *   															*
 *					Command Line Offset							*
 *   															*
 *==============================================================*/

void AnnotationEditor::setSignalOffset(float offset)
{
	//-1: see if command line had been used
	float of = offset;
	if (offset == -1 && m_command_line_offset != -1)
	{
		of = m_command_line_offset;
		// for consultation mode, keep the id of the given position
		//		if (consultationMode)
		//			m_command_line_offset_segid = m_dataModel.getByOffset(m_dataModel.mainstreamBaseType("transcription_graph"), of, m_activeTrack);
	}
	if (m_signalView && of != -1)
		m_signalView->setCursor(of);
}

/*==============================================================*
 *   															*
 *					HIGHLIGHT RESULT SET
 *   															*
 *==============================================================*/

void AnnotationEditor::setHighlightResultset(Glib::ustring resultset)
{
	if (resultSet)
		delete (resultSet);

	resultSet = new ResultSet();

	resultSet->loadResultSetString(resultset);
	if (!resultSet->isLoaded())
	{
		delete (resultSet);
		resultSet = NULL;
		Log::err() << "(!) Unable to load result set file, aborted" << std::endl;
	}
	else
	{
		print_vector_s(resultSet->getMatchingTerms(), "ResultSet matching terms");
		Log::out() << "(o) Resultset successfully loaded" << std::endl;
	}
	//todo do it with file instead of URL ?
}


//******************************************************************************
//										DEBUG
//******************************************************************************

bool AnnotationEditor::isDebugMode()
{
	return ( globalOptions.getParameterValue("General", "LOG", "mode") == "debug" ) ;
}

void AnnotationEditor::cursorChanged(const Gtk::TextIter& iter, bool unused)
{
	string pos = string(" ") + number_to_string(iter.get_line())
									+ "." + number_to_string(iter.get_line_offset())
									+ " ("+ number_to_string(iter.get_offset()) + ")" ;

	bool canSplit = m_activeView->getBuffer()->canSplitAtIter(iter) ;

	pos = pos + " cansplit=" + number_to_string(canSplit) ;

	signalStatusBar().emit(pos) ;
}

void AnnotationEditor::printIndex()
{
	Log::out() << "\n-------------------------------- PRINT INDEXES G(O.o) !!! ---------------------------" << std::endl ;
	std::map<std::string, AnnotationIndex> index = m_indexes[0] ;
	std::map<std::string, AnnotationIndex>::iterator it_types ;
	for (it_types=index.begin(); it_types!=index.end(); it_types++)
	{
		Log::out() << "** TYPE=" << it_types->first << std::endl ;
		it_types->second.printIndexes() ;
	}
	Log::out() << "\n-------------------------------- D(O.o)ne -------------------------------------------\n" << std::endl ;
}


void AnnotationEditor::reportCheckAnchors()
{
	Log::out() << "\n\n___________________________REPORT CHECK ANCHOR_____________________________" << std::endl ;
	for (guint i = 0; i < m_textView.size(); i++)
	{
		if (m_textView[i])
		{
			Log::out() << "\n************************** VIEW=" << m_textView[i]->getViewTrack() << std::endl ;

			std::map<int, vector<string> > v = m_textView[i]->getBuffer()->anchors().checkAnchors() ;
			std::map<int, vector<string> >::iterator it ;
			vector<string>::iterator vt ;
			for ( it=v.begin(); it!=v.end(); it++ )
			{
				if ( it->second.size() > 1 )
				{
					Log::out() << "========= Offset " << it->first << std::endl ;
					for ( vt=it->second.begin(); vt!=it->second.end(); vt++ )
						Log::out() << "> Id=" << *vt << std::endl ;
				}
			}
		}
	}
	Log::out() << "______________________________________________________________________________\n\n" << std::endl ;
}


} /* namespace tag */
