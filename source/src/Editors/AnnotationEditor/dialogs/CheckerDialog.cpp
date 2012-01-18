/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "CheckerDialog.h"
#include "Common/icons/Icons.h"
#include "Common/FileInfo.h"
#include "Common/util/StringOps.h"
#include "Common/widgets/GtUtil.h"

namespace tag {

//------------------------------------------------------------------------------
//								Entry Methods
//------------------------------------------------------------------------------

CheckerDialog::CheckerDialogEntry::CheckerDialogEntry(Glib::ustring p_label, Glib::ustring detailed,
														Glib::ustring p_tip, Glib::ustring checkTip,
														int errorCode, bool canBeFixed)
{
	error_code = errorCode ;

	// header of expander
	Gtk::Label* label = Gtk::manage(new Gtk::Label(p_label)) ;
	Gtk::Label* space = Gtk::manage(new Gtk::Label(" ")) ;

	hbox.pack_start(*label, false, false, 5) ;
	hbox.pack_start(*space, true, true, 5) ;
	hbox.show() ;
	expand.set_label_widget(hbox) ;

	// body of expander if needed
	Gtk::TextView* detailedView = NULL ;
	if (!detailed.empty()) {
		detailedView = Gtk::manage(new Gtk::TextView()) ;
		detailedView->get_buffer()->set_text(detailed) ;
		detailedView->show() ;
		detailedView->set_editable(false) ;
		Gtk::HBox* hbox_presentation = Gtk::manage(new Gtk::HBox()) ;
		Gtk::Label* label_presentation = Gtk::manage(new Gtk::Label("      ")) ;
		hbox_presentation->pack_start(*label_presentation, false, false) ;
		hbox_presentation->pack_start(*detailedView, true, true) ;
		expand.add(*hbox_presentation) ;
		detailedView->modify_text(Gtk::STATE_NORMAL, Gdk::Color("#444444")) ;
		detailedView->modify_base(Gtk::STATE_NORMAL, Gdk::Color("#FBFBFB")) ;
	}
	expand.set_expanded(false) ;

	// entry
	Gtk::Label* checkLabel = Gtk::manage(new Gtk::Label()) ;
	Gtk::Label* space2 = Gtk::manage(new Gtk::Label("    ")) ;
	pack_start(expand, false, false,2) ;
	pack_start(checkHbox, true, true, 2) ;
	checkHbox.pack_start(*space2, false, false,2) ;
	checkHbox.pack_start(checkb, false, false,2) ;
	checkHbox.pack_start(image, false, false,2) ;
	checkHbox.pack_start(*checkLabel, false, false,3) ;

	if (!p_tip.empty())
		tip.set_tip(*this, p_tip) ;

	checkLabel->set_name("chk_dlg_check_label") ;

	if (!checkTip.empty()) {
		tip.set_tip(checkb, checkTip) ;
		checkLabel->set_label(checkTip) ;
	}
	else {
		tip.set_tip(checkb, _("Add in correction list")) ;
		checkLabel->set_label(_("Fix error")) ;
	}

	show_all_children(true) ;

	if (!canBeFixed)
		checkHbox.hide() ;
	image.hide() ;
}

void CheckerDialog::CheckerDialogEntry::actualizeDisplay(int p_error_code, int state)
{
	if (p_error_code!=error_code)
		return ;

	if (state==2) {
 		checkb.hide() ;
 		image.set_icon(ICO_PREFERENCES_APPLY, "", 12, "Successful") ;
 		image.show() ;
 	}
 	else {
 		checkb.hide() ;
 		image.set_icon(ICO_PREFERENCES_CANCEL, "", 12, "Successful") ;
 		image.show() ;
 	}
}

//------------------------------------------------------------------------------
//								 Section Methods
//------------------------------------------------------------------------------

/*
 * Create a section expander (Information - Warning - Errors)
 */
CheckerDialog::CheckerDialogSection::CheckerDialogSection(Glib::ustring p_label, Glib::ustring icon, Glib::ustring mode)
{
	nbElements = 0 ;

	Gtk::Label* label = Gtk::manage(new Gtk::Label(p_label)) ;
	int ico_size = 17 ;

	if (mode=="info") {
		ico_size = 20 ;
		label->set_name("chk_dlg_bold_label_blue") ;
	}
	else if (mode=="warning")
		label->set_name("chk_dlg_bold_label_orange") ;
	else if (mode=="error")
		label->set_name("chk_dlg_bold_label_red") ;

	image.set_image(icon, ico_size) ;

	header.pack_start(image, false, false, 5) ;
	header.pack_start(*label, false, false, 5) ;
	header.pack_start(number, false, false, 5) ;

	Gtk::HBox* hbox_presentation = Gtk::manage(new Gtk::HBox()) ;
	Gtk::Label* label_presentation = Gtk::manage(new Gtk::Label("   ")) ;
	hbox_presentation->pack_start(*label_presentation, false, false) ;
	hbox_presentation->pack_start(vbox, true, true) ;

	add(*hbox_presentation) ;
	set_expanded(true) ;
	set_label_widget(header) ;

	vbox.show() ;

	show_all_children(true) ;
}

/*
 * Add an error entry to the section
 */
void CheckerDialog::CheckerDialogSection::addElement(Gtk::Widget* widget)
{
	nbElements++ ;
	string n_s = number_to_string(nbElements) ;
	n_s = std::string("(") + n_s + std::string(")") ;
	number.set_label(n_s) ;
	vbox.pack_start(*widget, false, false, 2) ;
}


//******************************************************************************
//*********************************** Dialog Methods ***************************
//******************************************************************************

CheckerDialog::CheckerDialog(DataModel* p_model, const string& p_filepath, bool p_fixEnabled)
{
	fixEnabled = p_fixEnabled ;

	Icons::set_window_icon(this, ICO_TRANSCRIBER, 12) ;

	button_cpt =  0 ;
	model = p_model ;
	filepath = p_filepath ;
	if (p_model)
		checker = p_model->getModelChecker() ;

	prepareGUI() ;
	loadData() ;
	adjustDialogSize() ;
	set_focus(button_close) ;
	checker->signalFlushGUI().connect(sigc::mem_fun(this, &CheckerDialog::flushGUI)) ;
}

CheckerDialog::~CheckerDialog()
{
	std::vector<CheckerDialogEntry*>::iterator it = entries.begin() ;
	while (it!=entries.end())
	{
		CheckerDialogEntry* tmp = *it ;
		it++ ;
		if (tmp)
			delete(tmp) ;
	}

	std::map<string, CheckerDialogSection*>::iterator it2 = expands.begin() ;
	while (it2!=expands.end())
	{
		CheckerDialogSection* tmp = it2->second ;
		it2++ ;
		if (tmp)
			delete(tmp) ;
	}
}

void CheckerDialog::prepareGUI()
{
	/** Buttons **/
	button_close.set_use_underline(true) ;
	button_apply.set_use_underline(true) ;

	button_close.set_icon("", _("_Close"), 12, _("Close dialog") ) ;
	button_apply.set_icon(ICO_PREFERENCES_APPLY, _("_Fix selected errors"), 12, _("Apply all actions selected in dialog") ) ;

//	button_close.signal_released().connect(sigc::bind<string>(sigc::mem_fun(this, &CheckerDialog::onButtonReleased), "close")) ;
//	button_apply.signal_released().connect(sigc::bind<string>(sigc::mem_fun(this, &CheckerDialog::onButtonReleased), "apply")) ;

	button_close.signal_clicked().connect(sigc::bind<string>(sigc::mem_fun(this, &CheckerDialog::onButtonReleased), "close")) ;
	button_apply.signal_clicked().connect(sigc::bind<string>(sigc::mem_fun(this, &CheckerDialog::onButtonReleased), "apply")) ;

	button_apply.set_sensitive(false) ;

	Gtk::HButtonBox* bbox =  (Gtk::HButtonBox*) get_action_area() ;
	bbox->pack_start(button_apply, false, false) ;
	bbox->pack_start(button_close, false, false) ;

	scrolledW.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC) ;

	/** Global **/
	Gtk::Alignment* align = Gtk::manage(new Gtk::Alignment()) ;
	Gtk::VBox* titlebox = Gtk::manage(new Gtk::VBox()) ;
	align->set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
	Gtk::Label* title = Gtk::manage(new Gtk::Label(_("LOADING REPORT"))) ;
	string name = Glib::path_get_basename(filepath) ;
	Gtk::Label* file = Gtk::manage(new Gtk::Label(name)) ;
	file->set_tooltip_text(filepath) ;

	align->add(*titlebox) ;
	titlebox->pack_start(*title, false, false, 3) ;
	titlebox->pack_start(*file, false, false, 3) ;

	title->set_name("chk_dlg_bold_title") ;

	Gtk::VBox* mainVbox = get_vbox() ;
	mainVbox->pack_start(*align, false, false, 15 ) ;
	mainVbox->pack_start(scrolledW, true, true, 5) ;
	scrolledW.add(expand_box) ;

	show_all_children(true) ;
}

void CheckerDialog::adjustDialogSize()
{
	int nbEntries = entries.size() ;
	int h = 140 + 70*nbEntries ;

	if (h > 600)
		h = 600 ;

	set_size_request(700, h) ;
}


//------------------------------------------------------------------------------
//----------------------------------- Data business ----------------------------
//------------------------------------------------------------------------------


void CheckerDialog::loadData()
{
	if (!checker)
	{
		TRACE << "No checker activated. No display" << std::endl ;
		return ;
	}

	/*
	 *  DATA
	 *	Load msg by priority order
	 */
	loadInfo() ;
	loadWarningsErrors(1) ;
	loadWarningsErrors(2) ;
}

void CheckerDialog::loadInfo()
{
	//  For formats different than TAG of course graphs are added
	//	so don't display anything
	if (!model->isLoadedTagFormat())
		return ;

	int nbAdded = checker->getNbAddedGraphs() ;
	if (nbAdded==0)
		return ;

	/** add section **/
	CheckerDialogSection *expand =	addSection(_("Information"), ICO_CHK_INFO, "info") ;

	/** get info & display **/
	std::set<std::string> added_types = checker->getAddedGraphs() ;
	string label ;
	if (nbAdded==1)
		label = _("The following graph is defined in convention but not in file, file has been updated: ") ;
	else
		label = _("The following graphs are defined in convention but not in file, file has been updated: ") ;

	std::set<std::string>::iterator it ;
	for (it=added_types.begin(); it!=added_types.end(); it++)
		label = label + "\n - " + *it ;

	addEntry(expand, NULL, label, "", "", "", 1, false) ;
}

void CheckerDialog::loadWarningsErrors(int priority)
{
	if (priority!=1 && priority!=2)
		return ;

	if ( (priority==1 && !checker->hasWarnings() )
			|| (priority==2 && !checker->hasErrors() ) )
		return ;

	/** add section **/
	CheckerDialogSection *expand = NULL ;
	if (priority==1)
		expand = addSection(_("Warning"), ICO_CHK_WARN, "warning") ;
	if (priority==2)
		expand = addSection(_("Errors"), ICO_CHK_ERROR, "error") ;


	/** get info & display **/
	if (priority==1)
	{
		//> 1 : specific convention errors
		string name, version ;
		int convention_error = checker->getConventionLog(name, version) ;
		displayError(expand, NULL, convention_error) ;

		//> 2 : import version
		//		only for import format
		if (!model->isLoadedTagFormat() && checker->hasImportWarnings())
			displayError(expand, NULL, MCHK_ERROR_IMPORT_WARNING) ;
	}

	//> 3 : graph errors
	std::set<std::string> bad_graph = checker->getGraphesByPriority(priority) ;
	std::set<std::string>::iterator it ;
	// -- for each graph
	for (it=bad_graph.begin(); it!=bad_graph.end(); it++)
	{
		ModelChecker::CheckGraphResult* result = checker->getCheckResult(*it) ;
		if (result)
			displayCheckResult(expand, result) ;
	}
}


void CheckerDialog::displayCheckResult(CheckerDialogSection *expand, ModelChecker::CheckGraphResult* checkResult)
{
	std::map<int,int> errors = checkResult->get_errorCodes() ;
	std::map<int,int>::iterator it ;
	// -- For each type of error of the graph result, display a line
	for (it=errors.begin(); it!=errors.end(); it++)
		displayError(expand, checkResult, it->first) ;
}

void CheckerDialog::displayError(CheckerDialogSection *expand, ModelChecker::CheckGraphResult* checkResult, int errorCode)
{
	string graphtype = "" ;
	string graphid = "" ;
	string msg = "" ;
	string detailed = "" ;
	string checkTip = "" ;
	string start_line = "==> " ;
	string eol = "\n" ;
	string space = " " ;
	string cote = "\"" ;

	if (checkResult)
	{
		graphtype = checkResult->get_graphType() ;
		graphid = checkResult->get_graphId() ;
		msg = graphtype +  " " ;
		detailed = _("Graph") + space + cote + graphid + cote + space
					+  _("of type") + space + cote + graphtype + cote + space + "\n" ;
	}

	bool can_fix = false ;


	switch (errorCode)
	{
		case MCHK_ERROR_GRAPH_INVALIDUNIT :
		{
				msg.append(_("has base type element not defined in conventions")) ;
				detailed.append(_("The following base type annotation types are not defined in conventions: ")) ;
				checkTip = _("Delete base type elements of unknown types") ;
				set<string> invalidSubmain  =  checkResult->get_invalidSubmain() ;
				set<string>::iterator it ;
				for (it=invalidSubmain.begin(); it!=invalidSubmain.end(); it++)
					detailed.append("\n - " + *it ) ;
				//TODO check
				can_fix = false ;
				break ;
		}
		case MCHK_ERROR_GRAPH_INVALIDUNIT_VALUE :
		{
				msg.append(_("has foreground element whose type is undefined in conventions")) ;
				detailed.append(_("The following type are not defined in conventions: ")) ;
				checkTip = _("Delete foreground elements of unknown types") ;
				std::map<string, set<string> > invalidSubmain  =  checkResult->get_invalidSubmainValues() ;
				std::map<string, set<string> >::iterator it ;
				for (it=invalidSubmain.begin(); it!=invalidSubmain.end(); it++)
				{
					string baseType = it->first ;
					set<string> invalid_values = it->second ;
					set<string>::iterator it_val ;
					for (it_val=invalid_values.begin(); it_val!=invalid_values.end(); it_val++)
						detailed.append("\n - " + *it_val + " (for base element " + baseType +")" ) ;
				}
				//TODO check
				can_fix = false ;
				break ;
		}
		case MCHK_ERROR_GRAPH_INVALIDUNIT_DESC :
		{
				msg.append(_("has foreground element whose subtype is undefined in conventions")) ;
				detailed.append(_("The following subtype are not defined in conventions: ")) ;
				checkTip = _("Delete foreground elements of unknown subtypes") ;
				std::map<string, set<string> > invalidSubmain  =  checkResult->get_invalidSubmainDesc() ;
				std::map<string, set<string> >::iterator it ;
				for (it=invalidSubmain.begin(); it!=invalidSubmain.end(); it++)
				{
					string baseType = it->first ;
					set<string> invalid_desc = it->second ;
					set<string>::iterator it_val ;
					for (it_val=invalid_desc.begin(); it_val!=invalid_desc.end(); it_val++)
					{
						StringOps tmp(*it_val) ;
						vector<string> cut ;
						tmp.split(cut, ";", true) ;
						if ( cut.size() == 2 )
							detailed.append("\n - " + cut[1] + " (for base element " + baseType +")" ) ;
						else
							detailed.append("\n - #unable to retrieve subtype# (for base element " + baseType +")" ) ;
					}
				}
				//TODO check
				can_fix = false ;
				break ;
		}
		case MCHK_ERROR_GRAPH_INVALIDTYPES :
		{
				msg.append(_("has annotation types not defined in conventions")) ;
				detailed.append(_("The following annotation types are not defined in conventions: ")) ;
				checkTip = _("Delete elements of unknown types") ;
 				set<string> invalidTypes  =  checkResult->get_invalidTypes() ;
				set<string>::iterator it ;
				for (it=invalidTypes.begin(); it!=invalidTypes.end(); it++)
					detailed.append("\n - " + *it ) ;
				can_fix = true ;
				break ;
		}
		case MCHK_ERROR_GRAPH_INVALIDSUBTYPES :
		{
				msg.append(_("has annotation sub-types not defined in conventions")) ;
				checkTip = _("Delete types with unknown sub-types") ;
 				std::map<string, std::set<std::string> > invalidSubtypes =  checkResult->get_invalidSubtypes();
 				std::map<string, std::set<std::string> >::iterator it ;
				std::set<string> subtypes ;
				std::set<string>::iterator it_sub ;
				string type ;
 				string sub_s = _("Type") ;
  				string tmp = _("has unknown subtypes") ;
				checkTip = _("Delete elements of unknown sub-types") ;
				for (it=invalidSubtypes.begin(); it!=invalidSubtypes.end(); it++)
				{
					type = it->first ;
					detailed.append( eol + sub_s + space + cote + type + cote + space + tmp + " : " ) ;
					subtypes = it->second ;
					for (it_sub=subtypes.begin(); it_sub!=subtypes.end(); it_sub++)
						detailed.append(eol + " - " + *it_sub ) ;
				}
				can_fix = true ;
				break ;
		}
		case MCHK_ERROR_GRAPH_NOBASESEG :
		{
				msg = msg + _("has no base type elements") ;
				string baseType = model->mainstreamBaseType() ;
				detailed.append(_("Base element type defined in conventions: ") + baseType + eol) ;
				detailed.append(start_line + _("No elements of this type exists in file") + eol) ;
				detailed.append(start_line + _("Graph has been skipped")) ;
				break ;
		}
		case MCHK_ERROR_GRAPH_NOCONTINUOUS :
		{
				msg = msg + _("is not continuous") ;
				detailed.append(_("The graph is defined as continuous in conventions") + eol)  ;
				detailed.append(start_line + _("Continuity check failed")) ;
				break ;
		}
		case MCHK_ERROR_GRAPH_NOTINCONV :
		{
				msg = msg + _("is not defined in conventions") ;
				detailed.append(_("This type of graph is not defined in conventions") + eol) ;
				detailed.append(start_line + _("Graph has been skipped")) ;
				break ;
		}
		case MCHK_ERROR_CONV_FILE :
		{
				string name, version ;
				checker->getConventionLog(name, version) ;
				name = Glib::path_get_basename(name) ;
				string convention_directory = model->conventions().getDirectory() ;
				string current_conventions =  model->conventions().name() ;
				msg = _("Unable to find conventions file") ;
				detailed = _("The file requires the following conventions:") + eol ;
				detailed.append("- " + name + "\n") ;
				detailed.append(start_line + _("Not found in convention directory ") + convention_directory + eol) ;
				detailed.append(start_line + _("Default convention applied: ") + current_conventions)  ;
				break;
		}
		case MCHK_ERROR_CONV_VERSION :
		{
				string name, version ;
				checker->getConventionLog(name, version) ;
				string current_version = model->conventions().version() ;
				string current_conventions =  model->conventions().name() ;
				msg = _("Conventions version doesn't match") ;
				detailed = _("The file is using the convention") + space + current_conventions + space  ;
				detailed.append(_("of version") + space + current_version + eol) ;
				detailed.append(_("The file requires version") + space + version) ;
				break;
		}
		case MCHK_ERROR_IMPORT_WARNING :
		{
				msg = std::string(_("Error while importing format")) +  std::string(" ") +  model->getLoadedFileFormat() ;
				detailed = _("Importing file") + space  + model->getPath() + eol ;
				detailed = detailed + "Format: " + model->getLoadedFileFormat() ;
				std::vector<std::string> warns = checker->getImportWarnings() ;
 				std::vector<std::string>::iterator it ;
				for (it=warns.begin(); it!=warns.end(); it++)
					detailed = detailed + eol + *it ;
				break ;
		}
		default :
				return ;
	}

	// fix only if it is allowed in global option
	bool fix = (fixEnabled ? can_fix : false) ;

	addEntry(expand, checkResult, msg, detailed, detailed, checkTip, errorCode, fix) ;
}

//------------------------------------------------------------------------------
//------------------------------------ Add business ----------------------------
//------------------------------------------------------------------------------

CheckerDialog::CheckerDialogEntry* CheckerDialog::addEntry(CheckerDialogSection* expander, ModelChecker::CheckGraphResult* checkResult,
											Glib::ustring p_label, Glib::ustring detailed,
											Glib::ustring tip, Glib::ustring checkTip,
											int errorCode, bool canBeFixed)
{
	if (!expander)
		return NULL ;

	CheckerDialogEntry* entry = new CheckerDialogEntry(p_label, detailed, tip, checkTip, errorCode, canBeFixed) ;

	// set connections
	if (canBeFixed) {
		entry->getChekButton()->signal_toggled().connect(sigc::bind<CheckerDialogEntry*,ModelChecker::CheckGraphResult*>(sigc::mem_fun(this, &CheckerDialog::onCheckboxChanged), entry, checkResult)) ;
		if (checkResult)
			checkResult->signalDisplayState().connect(sigc::mem_fun(*entry, &CheckerDialog::CheckerDialogEntry::actualizeDisplay)) ;
	}

	entries.push_back(entry) ;
	expander->addElement(entry) ;
	entry->show() ;
	return entry ;
}

CheckerDialog::CheckerDialogSection* CheckerDialog::addSection(Glib::ustring label, Glib::ustring icon, Glib::ustring mode)
{
	CheckerDialogSection* expand = new CheckerDialogSection(label, icon, mode) ;
	expands[mode] = expand ;
	expand_box.pack_start(*expand, false, false, 6) ;
	expand->show() ;
	return expand ;
}

//------------------------------------------------------------------------------
//---------------------------------- Callback ----------------------------------
//------------------------------------------------------------------------------

void CheckerDialog::onCheckboxChanged(CheckerDialogEntry* entry, ModelChecker::CheckGraphResult* checkResult)
{
	if (!entry)
		return ;

	bool activated = entry->getChekButton()->get_active() ;

	if (activated)
		button_cpt++ ;
	else
		button_cpt-- ;

	setCleanCandidate(activated, checkResult, entry->getErrorCode()) ;

	if (button_cpt==0)
		button_apply.set_sensitive(false) ;
	else
		button_apply.set_sensitive(true) ;
}

void CheckerDialog::onButtonReleased(const string& mode)
{
	if (mode.compare("apply") ==0)
	{
		flushGUI() ;
		bool mod = CleanModel() ;
		if (mod)
			m_signalModelModified.emit() ;
		button_apply.set_sensitive(false) ;
	}
	else if (mode.compare("close") ==0)
		response(Gtk::RESPONSE_CLOSE) ;
}

//------------------------------------------------------------------------------
//---------------------------------Clean business ------------------------------
//------------------------------------------------------------------------------

void CheckerDialog::setCleanCandidate(bool add, ModelChecker::CheckGraphResult* checkResult, int errorCode)
{
	//TODO remove this test for enabling correction on general errors
	// (general in contrast with specific to a graph)
	if (!checkResult)
		return ;

	if (add)
		checkResult->setErrorCodeCleanCandidate(errorCode, true) ;
	else
		checkResult->setErrorCodeCleanCandidate(errorCode, false) ;
}

bool CheckerDialog::CleanModel()
{
	int result =  checker->applyCleanActions() ;
	return (result>0) ;
}


void CheckerDialog::flushGUI()
{
	// flush
	GtUtil::flushGUI(false, false) ;
}


} // namespace
