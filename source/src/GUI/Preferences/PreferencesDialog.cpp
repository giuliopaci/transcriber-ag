/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "PreferencesDialog.h"
#include "globdef.h"
#include "TreeModelColumnsPreferences.h"
#include "Common/icons/Icons.h"
#include "Common/globals.h"
#include "Common/util/Utils.h"
#include "Common/Dialogs.h"
#include "Common/VersionInfo.h"
#include "Explorer_utils.h"


namespace tag {

PreferencesDialog::PreferencesDialog(Configuration* _config, int p_lastOption)
{
	lastFrameCode = p_lastOption ;
	gen_frame = NULL ;
	dmodel_frame = NULL ;
	teditor_frame = NULL ;
/* SPELL */
//	speller_frame = NULL ;
	speakers_frame = NULL ;
	audio_frame = NULL ;
	video_frame = NULL ;

    parent = NULL;
	lookNfeelColors_frame = NULL ;
	lookNfeelFonts_frame = NULL ;

	set_default_size(800,600);
	string title = string(TRANSAG_DISPLAY_NAME) + " " + _("Preferences") ;
	set_title(title) ;
	Icons::set_window_icon(this, ICO_PREFERENCES, 10) ;

	config = _config ;

	refModel = Gtk::TreeStore::create(m_columns) ;
	prepare_view() ;
	fill_tree() ;
	prepare_gui() ;

//	change_frame(lastFrameCode) ;
}

PreferencesDialog::~PreferencesDialog()
{
	std::vector<PreferencesFrame*>::iterator it ;
	for (it=frames.begin(); it!=frames.end(); it++) {
		if (*it)
			delete(*it) ;
	}
}

void PreferencesDialog::prepare_view()
{
	tview.set_model(refModel) ;

	Gtk::TreeViewColumn* pColumn = Gtk::manage(new Gtk::TreeViewColumn("Icons")) ;
	Gtk::CellRendererPixbuf* cellRend_ico = Gtk::manage(new Gtk::CellRendererPixbuf()) ;
	Gtk::CellRendererText* cellRend_name = Gtk::manage(new Gtk::CellRendererText()) ;
	int number = tview.append_column(*pColumn) ;
	if (pColumn) {
		pColumn->pack_start(*cellRend_ico,false);
		pColumn->pack_start(*cellRend_name,false);
		pColumn->add_attribute(cellRend_ico->property_pixbuf(), m_columns.m_config_ico) ;
		pColumn->add_attribute(cellRend_name->property_text(), m_columns.m_config_name) ;
		pColumn->add_attribute(cellRend_name->property_weight(), m_columns.m_config_weight) ;
	}

	tview.set_rules_hint(false) ;
	tview.set_headers_clickable(false) ;
	tview.set_headers_visible(false) ;

	tview.signalSelection().connect(sigc::mem_fun(*this, &PreferencesDialog::on_selection_change)) ;
}

void PreferencesDialog::prepare_gui()
{
	Gtk::VBox* vbox = get_vbox() ;

	vbox->pack_start(paned, true, true) ;
	paned.pack1(scroll, false, false) ;
    scroll.add(tview) ;
	scroll.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);

	Gtk::HButtonBox* button_box = (Gtk::HButtonBox*) get_action_area() ;
	Gtk::Button* close = add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE) ;

	apply.set_icon(ICO_PREFERENCES_APPLY, _("_Apply"), 15, "") ;
	cancel.set_icon(ICO_PREFERENCES_CANCEL, _("_Cancel"), 15, "") ;
	apply.set_use_underline(true) ;
	cancel.set_use_underline(true) ;
	apply.signal_clicked().connect( sigc::mem_fun(*this, &PreferencesDialog::on_apply));
	cancel.signal_clicked().connect( sigc::mem_fun(*this, &PreferencesDialog::on_cancel));

	button_box->add(apply) ;
	button_box->add(cancel) ;

	vbox->show_all_children() ;

	apply.set_sensitive(false) ;
	cancel.set_sensitive(false) ;

	// definir position par défaut
	int w = get_screen()->get_width() ;
	int h = get_screen()->get_height() ;
	if (w > 900 && h > 702) {
		resize(900, 702) ;
	    paned.set_position(200) ;
	}
	else
		paned.set_position(202) ;

	// select lastly activated row in tree
	Gtk::TreeIter iter = refModel->get_iter(lastFramePath) ;
	if (refModel->iter_is_valid(iter))
	{
		// for children row, we need to expand father row first
		if (need_expand)
			tview.expand_to_path(lastParentFramePath) ;
		tview.set_cursor(lastFramePath) ;
	}
}

/*
 *  ADD all preferences frame you want to be displayed
 */
void PreferencesDialog::fill_tree()
{
	need_expand = false ;

	Gtk::TreeIter new_iter ;
	Gtk::TreeIter new_child ;
	Glib::ustring name ;
	int type ;
	Gtk::TreeRow row ;

	//> CAUTIOUS
	//> define unique type for each frame
	// it will be used to map between frames in tree callback selection

	name = _("General") ;
	type = 0 ;
	new_iter = refModel->append() ;
	row = *new_iter ;
	m_columns.fill_row(&row, ICO_PREFERENCES_GENERAL, type, name, 550) ;
	if (lastFrameCode==type)
		lastFramePath = Gtk::TreePath(new_iter) ;

	name = _("Transcription") ;
	type = 1 ;
	new_iter = refModel->append() ;
	row = *new_iter ;
	m_columns.fill_row(&row, ICO_PREFERENCES_TRANSCRIPT, type, name, 550) ;
	if (lastFrameCode==type)
		lastFramePath = Gtk::TreePath(new_iter) ;

	name = _("Text Editor") ;
	type = 3 ;
	new_iter = refModel->append() ;
	row = *new_iter ;
	m_columns.fill_row(&row, ICO_PREFERENCES_EDITOR, type, name, 550) ;
	if (lastFrameCode==type)
		lastFramePath = Gtk::TreePath(new_iter) ;

	name = _("Audio Panel") ;
	type = 5 ;
	new_iter = refModel->append() ;
	row = *new_iter ;
	m_columns.fill_row(&row, ICO_PREFERENCES_AUDIO, type, name, 550) ;
	if (lastFrameCode==type)
		lastFramePath = Gtk::TreePath(new_iter) ;

	name = _("Video Panel") ;
	type = 8 ;
	new_iter = refModel->append() ;
	row = *new_iter ;
	m_columns.fill_row(&row, ICO_PREFERENCES_VIDEO, type, name, 550) ;
	if (lastFrameCode==type)
		lastFramePath = Gtk::TreePath(new_iter) ;

/* SPELL */
//	name = _("Speller Tool") ;
//	type = 4 ;
//	new_iter = refModel->append() ;
//	row = *new_iter ;
//	m_columns.fill_row(&row, ICO_PREFERENCES_SPELLER, type, name, 550) ;
//	if (lastFrameCode==type)
//		lastFramePath = Gtk::TreePath(new_iter) ;

	name = _("Speakers") ;
	type = 6 ;
	new_iter = refModel->append() ;
	row = *new_iter ;
	m_columns.fill_row(&row, ICO_PREFERENCES_SPEAKERS, type, name, 550) ;
	if (lastFrameCode==type)
		lastFramePath = Gtk::TreePath(new_iter) ;

	name = _("Look'N'Feel") ;
	type = 7 ;
	new_iter = refModel->append() ;
	row = *new_iter ;
	m_columns.fill_row(&row, ICO_PREFERENCES_LOOK, type, name, 550) ;
	if (lastFrameCode==type)
		lastFramePath = Gtk::TreePath(new_iter) ;

		name = _("Colors") ;
		type = 71 ;
		new_child = refModel->append(new_iter->children()) ;
		row = *new_child;
		m_columns.fill_row(&row, ICO_PREFERENCES_LOOK_COLORS, type, name, 400) ;
		if (lastFrameCode==type) {
			lastFramePath = Gtk::TreePath(new_child) ;
			lastParentFramePath = Gtk::TreePath(new_iter) ;
			need_expand = true ;
		}

		name = _("Fonts") ;
		type = 72 ;
		new_child = refModel->append(new_iter->children()) ;
		row = *new_child  ;
		m_columns.fill_row(&row, ICO_PREFERENCES_LOOK_FONTS, type, name, 400) ;
		if (lastFrameCode==type) {
			lastFramePath = Gtk::TreePath(new_child) ;
			lastParentFramePath = Gtk::TreePath(new_iter) ;
			need_expand = true ;
		}

	#ifdef WITH_FTP
		name = _("FTP connection") ;
		type = 2 ;
		new_iter = refModel->append() ;
		row = *new_iter ;
		m_columns.fill_row(&row, ICO_PREFERENCES_CONNECTION, type, name, 550) ;
		if (lastFrameCode==type)
			lastFramePath = Gtk::TreePath(new_iter) ;
	#endif
}


/*
 * Actualize frame displayed when user change selection in tree panel
 */
void PreferencesDialog::on_selection_change(Gtk::TreePath path)
{
	TreeModelColumnsPreferences m ;
	Gtk::TreeIter iter = refModel->get_iter(path) ;

	//> check iter
	if (!refModel->iter_is_valid(iter)) {
		// position restore failed, reset for next time (prevent loop)
		lastFrameCode = 0 ;
		return ;
	}

	//> remove old frame
	Gtk::Widget* w = paned.get_child2() ;
	if (w!=NULL)
		paned.remove(*w) ;

	//> add new frame
	int type = (*(*iter))[m.m_config_type] ;
	change_frame(type) ;
}

/* 0: General
 * 1:  Transcription
 * 2:  Ftp (deprecated)
 * 3:  TextEditor
// * 4:  Speller
 * 5:  Audio
 * 6:  Speaker
 * 71: LookNfeel Fonts
 * 72: LookNfeel Colors
 * 8:  Video
 */
void PreferencesDialog::change_frame(int mode)
{
	PreferencesFrame* frame = NULL ;

	//> GENERAL
	if (mode==0) {
		if (gen_frame == NULL) {
			gen_frame = new GeneralFrame(config, this, &dynamic_values, &static_values) ;
			frames.insert(frames.begin(), gen_frame) ;
		}
		else
			gen_frame->reload_data() ;
		frame = gen_frame ;
	}
	//> TRANSCRIPTION
	else if (mode==1) {
		if (dmodel_frame == NULL) {
			dmodel_frame = new DataModelFrame(config, this, &dynamic_values, &static_values) ;
			frames.insert(frames.begin(), dmodel_frame) ;
		}
		else
			dmodel_frame->reload_data() ;
		frame = dmodel_frame ;
	}
	//> TEXT EDITOR
	else if (mode==3) {
		if (teditor_frame == NULL) {
			teditor_frame = new TextEditorFrame(config, this, &dynamic_values, &static_values) ;
			frames.insert(frames.begin(), teditor_frame) ;
		}
		else
			teditor_frame->reload_data() ;
		frame = teditor_frame ;
	}
/* SPELL */
//	//> SPELLER
//	else if (mode==4) {
//		if (speller_frame == NULL) {
//			speller_frame = new SpellerFrame(config, this, &dynamic_values, &static_values) ;
//			frames.insert(frames.begin(), speller_frame) ;
//		}
//		else
//			speller_frame->reload_data() ;
//		frame = speller_frame ;
//	}
	//> SPEAKER
	else if (mode==6) {
		if (speakers_frame == NULL) {
			speakers_frame = new SpeakersFrame(config, this, &dynamic_values, &static_values) ;
			frames.insert(frames.begin(), speakers_frame) ;
		}
		else
			speakers_frame->reload_data() ;
		frame = speakers_frame ;
	}
	//> AUDIO
	else if (mode==5) {
		if (audio_frame == NULL) {
			audio_frame = new AudioFrame(config, this, &dynamic_values, &static_values) ;
			frames.insert(frames.begin(), audio_frame) ;
		}
		else
			audio_frame->reload_data() ;
		frame = audio_frame ;
	}
	//> COLORS
	else if (mode==71) {
		if (lookNfeelColors_frame == NULL) {
			lookNfeelColors_frame= new ColorsFrame(config, this, &dynamic_values, &static_values) ;
			frames.insert(frames.begin(), lookNfeelColors_frame) ;
		}
		else
			lookNfeelColors_frame->reload_data() ;
		frame = lookNfeelColors_frame ;
	}
	//> FONTS
	else if (mode==72) {
		if (lookNfeelFonts_frame == NULL) {
			lookNfeelFonts_frame= new FontsFrame(config, this, &dynamic_values, &static_values) ;
			frames.insert(frames.begin(), lookNfeelFonts_frame) ;
		}
		else
			lookNfeelFonts_frame->reload_data() ;
		frame = lookNfeelFonts_frame ;
	}
	//> VIDEO
	else if (mode==8) {
		if (video_frame == NULL) {
			video_frame= new VideoFrame(config, this, &dynamic_values, &static_values) ;
			frames.insert(frames.begin(), video_frame) ;
		}
		else
			video_frame->reload_data() ;
		frame = video_frame ;
	}

	if (frame) {
		paned.pack2(*frame, true, false) ;
		paned.show_all_children() ;
		frame->signalIsModified().connect(sigc::mem_fun(*this, &PreferencesDialog::on_modified)) ;
		frame->set_warnings_visible(false) ;
		set_active_warning_visible(true) ;
	}

	lastFrameCode = mode ;
}


void PreferencesDialog::on_response(int response)
{
	if (response== Gtk::RESPONSE_CLOSE || response==-4)
	{
		bool close = true ;
		if (apply.is_sensitive()) {
			Gtk::MessageDialog diag(*this, _("Parameters has been modified, do you want to save ?"), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_YES_NO, true) ;
			switch(diag.run()) {
				case Gtk::RESPONSE_YES : on_apply() ;  break ;
				case Gtk::RESPONSE_NO : on_cancel() ; break ;
			}
		}
		if (close) {
			bool ok = config->reload_user_config() ;
			if (ok)
				reload_data() ;
			saveGeoAndHide() ;
			set_active_warning_visible(false) ;
			static_values.clear() ;
		}
	}
}

bool PreferencesDialog::on_delete_event(GdkEventAny* event)
{
	on_response(-4) ;
	return true ;
}


void PreferencesDialog::on_modified(bool modified)
{
	if (modified) {
		apply.set_sensitive(true) ;
		cancel.set_sensitive(true) ;
	}
}

void PreferencesDialog::on_apply()
{
	bool ok = config->save_user_config() ;
	if (ok) {
		apply.set_sensitive(false) ;
		cancel.set_sensitive(false) ;
		//> send notification because a modification applied need file reloading
		int static_changes = -1 ;
		if (static_values.size()>0)
			static_changes = 1 ;
		//> dynamic behavior
		m_signalReloadModifications.emit(dynamic_values, static_changes) ;
		dynamic_values.clear() ;
	}
	else {
		Glib::ustring msg = _("An error occurred when saving preferences.") ;
		Glib::ustring detailed = _("Check if the configuration files in the personal folder still exist and are not corrupted.") ;
		dlg::error(msg, detailed, this) ;
	}
}

void PreferencesDialog::on_cancel()
{
	//> reload previous data
	bool ok = config->reload_user_config() ;

	if (ok) {
		reload_data() ;

		//> acutalize buttons
		apply.set_sensitive(false) ;
		cancel.set_sensitive(false) ;

		//> remove dynamic changes
		dynamic_values.clear() ;

		//> remove static changes
		set_active_warning_visible(false) ;
		static_values.clear() ;

		std::vector<PreferencesFrame*>::iterator it ;
		for (it=frames.begin(); it!=frames.end(); it++) {
			if (*it)
				(*it)->set_warnings_visible(false) ;
		}
	}
	else {
		Glib::ustring msg = _("An error occurred when reloading preferences.") ;
		Glib::ustring detailed = _("Check if the configuration files in the personal folder still exist and are not corrupted.") ;
		dlg::error(msg, detailed, this) ;
	}
}

void PreferencesDialog::reload_data()
{
	std::vector<PreferencesFrame*>::iterator it ;
	for (it=frames.begin(); it!=frames.end(); it++) {
		if (*it)
			(*it)->reload_data() ;
	}
}

bool PreferencesDialog::has_static_modifications()
{
	if (static_values.size()>0)
		return true ;
	else
		return false ;
}

void PreferencesDialog::set_active_warning_visible(bool value)
{
	std::vector<IcoPackImage*>::iterator it ;
	for (it=static_values.begin(); it!=static_values.end(); it++)
	{
		if (value)
			(*it)->show() ;
		else
			(*it)->hide() ;
	}
}


//**************************************************************************************
//										GEOMETRY INTERFACE
//**************************************************************************************

void PreferencesDialog::getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	get_position(pos_x, pos_y) ;
	panel = paned.get_position() ;
	get_size(size_xx, size_yy) ;
}

void PreferencesDialog::setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel)
{
	if (pos_x>0 && pos_y>0)
		move(pos_x, pos_y) ;
	paned.set_position(panel) ;
	if (size_xx>0 && size_yy>0)
		resize(size_xx, size_yy) ;
}

Glib::ustring PreferencesDialog::getWindowTagType()
{
	return SETTINGS_PREFERENCES_NAME ;
}

int PreferencesDialog::loadGeoAndDisplay(bool rundlg)
{
	loadPos() ;

	if (rundlg)
		return run() ;
	else
		show() ;

	return 1 ;
}

void PreferencesDialog::saveGeoAndHide()
{
	if (is_visible())
		savePos() ;
	hide() ;
}

void PreferencesDialog::getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) {}

} // namespace
