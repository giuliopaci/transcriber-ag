/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/*
 * BackgroundDialog.cpp
 *
 *  Created on: 9 juil. 2008
 *      Author: montilla
 */

#include "BackgroundDialog.h"
#include "Common/globals.h"
#include "Common/Dialogs.h"
#include "Common/util/Utils.h"
#include "Common/util/StringOps.h"
#include "Common/util/FormatTime.h"
#include "Common/icons/Icons.h"

namespace tag {

BackgroundDialog::BackgroundDialog(Gtk::Window* parent,
										DataModel& model, const std::string& id,
										bool can_edit, bool for_new_item,
										int notrack, float startOffset, float endOffset)
: m_parent(parent), m_dataModel(model), m_id(id), m_editable(can_edit)
{
	new_item = for_new_item ;

	Icons::set_window_icon(this, ICO_TRANSCRIBER, 11) ;

	combo_no_value = "---" ;

#ifdef __APPLE__
    if (parent)
        set_transient_for(*parent) ;
#endif
	set_modal(true) ;

	set_title(_("Background properties")) ;

	if ( id != "" && ! for_new_item ) {
		notrack = m_dataModel.getElementSignalTrack(id);
		startOffset = m_dataModel.getElementOffset(id, true);
		endOffset = m_dataModel.getElementOffset(id, false);
		m_chosen_level = m_dataModel.getElementProperty(id, "level");
		StringOps(m_dataModel.getElementProperty(id, "type")).split(m_chosen_types, ";,");
	}
	m_notrack=notrack;
	m_startOffset=startOffset;
	m_endOffset=endOffset;

	//PREPARE TABLE
	type_table = Gtk::manage(new Gtk::Table(2, 2, false)) ;
	type_table->attach(type_label, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 15, 15) ;
	type_table->attach(type_vbox, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 15, 15) ;
	type_table->attach(level_label, 0, 1, 1, 2, Gtk::SHRINK, Gtk::SHRINK, 15, 15) ;
	type_table->attach(level_combo, 1, 2, 1, 2, Gtk::SHRINK, Gtk::SHRINK, 15 ,15) ;

	if (startOffset>0 && endOffset>0) {
		type_table->resize(3,2) ;
		type_table->attach(offset_label, 0, 1, 2, 3, Gtk::SHRINK, Gtk::SHRINK, 15, 15) ;
		type_table->attach(offset_value, 1, 2, 2, 3, Gtk::SHRINK, Gtk::SHRINK, 15 ,15) ;

		offset_label.set_label(_("Time")) ;
		offset_label.set_name("bold_label") ;
		Glib::ustring time = FormatTime(startOffset, true) ;
		time.append(" - ") ;
		time.append(FormatTime(endOffset, true)) ;
		offset_value.set_label(time) ;
	}

	//PREPARE TYPES TABLE
	m_dataModel.conventions().getAnnotationItems(m_types, "background", "subtypes");
	m_dataModel.conventions().getAnnotationItems(m_levels, "background", "levels");

	std::vector<string>::iterator it_types ;
	for (it_types=m_types.begin(); it_types!=m_types.end(); it_types++) {
		if ( (*it_types) == "none") continue;  // skip "none" entry
		type_checks[*it_types] = Gtk::manage(new Gtk::CheckButton(m_dataModel.conventions().getLocalizedLabel(*it_types))) ;
		type_checks[*it_types]->set_active(false) ;
		type_vbox.pack_start( *(type_checks[*it_types]), false, false, 3) ;
		std::vector<string>::iterator it ;
		//> loop inside loop, not optimal, but chosen_types is to be a tiny list
		for (it=m_chosen_types.begin(); it!=m_chosen_types.end(); it ++) {
			if (*it == *it_types)
				type_checks[*it_types]->set_active(true) ;
			type_checks[*it_types]->set_sensitive(m_editable) ;
		}
	}

	//LABEL
	type_label.set_label(_("Type")) ;
	type_label.set_name("bold_label") ;
	level_label.set_label(_("Level")) ;
	level_label.set_name("bold_label") ;

	//PREPARE COMBO
	//level_combo.append_text(combo_no_value) ;
	std::vector<string>::iterator it_levels ;
	for (it_levels=m_levels.begin(); it_levels!=m_levels.end(); it_levels++) {
		const string& label = m_dataModel.conventions().getLocalizedLabel(*it_levels);
		level_combo.append_text(label) ;
		if (*it_levels == m_chosen_level)
			level_combo.set_active_text(label) ;
	}
	if (m_chosen_level.empty())
		level_combo.set_active(0) ;

	level_combo.set_sensitive(m_editable) ;


	Gtk::VBox* vbox = get_vbox() ;

	vbox->pack_start(*type_table, false, false, 5) ;
	
	Gtk::Button* ok;

	if (m_editable)
	{
		ok = add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY) ;
		if (!for_new_item) {
			deleteButton = Gtk::manage(new Gtk::Button(Gtk::Stock::DELETE)) ;
			get_action_area()->pack_start(*deleteButton, false, false) ;
			deleteButton->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &BackgroundDialog::onButtonClicked), Gtk::RESPONSE_DELETE_EVENT)) ;
		}
		Gtk::Button* cancel = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
		cancel->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &BackgroundDialog::onButtonClicked), Gtk::RESPONSE_CANCEL));
	}
	else
		ok = add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_APPLY) ;

	ok->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &BackgroundDialog::onButtonClicked), Gtk::RESPONSE_APPLY));
	set_default_response(Gtk::RESPONSE_APPLY);

	vbox->show_all_children() ;
}

BackgroundDialog::~BackgroundDialog()
{
	// TODO Auto-generated destructor stub
}

void BackgroundDialog::onButtonClicked(int response_id)
{
	bool ok = true;

	switch ( response_id )
	{
		case Gtk::RESPONSE_APPLY:
			if ( m_editable )
				ok = saveData();
			else
				response_id = Gtk::RESPONSE_CANCEL;
			break;
		case Gtk::RESPONSE_DELETE_EVENT:
			ok = dlg::confirm(_("Delete background segment ? "), this) ;
			if ( ok )
			{
				m_dataModel.deleteElement(m_id, true);
				response_id = Gtk::RESPONSE_APPLY;
			}
			break;
		default:
			break;
	}
	if ( ok )
		response(response_id);
}

bool BackgroundDialog::saveData ()
{
	string level = level_combo.get_active_text() ;

	// GET DATA AND THEIRS LABELS
	if (!level.empty() && level!=combo_no_value)
	{
		std::vector<string>::const_iterator it ;
		for (it=m_levels.begin(); it!=m_levels.end(); it++)
		{
			if ( level == m_dataModel.conventions().getLocalizedLabel(*it) )
				m_chosen_level = (*it) ;
		}
	}

	m_chosen_types.clear() ;
	std::map<string, Gtk::CheckButton*>::iterator it2 ;
	for (it2=type_checks.begin(); it2!=type_checks.end(); it2++)
	{
		if (it2->second)
		{
			if (it2->second->get_active())
				m_chosen_types.push_back(it2->first) ;
		}
	}

	// If we're editing an existing background and we want
	// to unset all types, it means we'll delete it
	if ( m_chosen_types.size() == 0 && !new_item) {
		onButtonClicked(Gtk::RESPONSE_DELETE_EVENT) ;
		return false ;
	}
	else
	{
		string types;
		StringOps(types).concat(m_chosen_types, ";");

		// CREATE BACKGROUND SEGMENT
		if ( m_id.empty() )
		{
			string id = m_dataModel.addBackgroundSegment(m_notrack, m_startOffset, m_endOffset, types,
					m_chosen_level, true);
		}
		else {

			if (types != m_dataModel.getElementProperty(m_id, "type", "") )
			{
				if (types.empty() )
					m_dataModel.deleteElementProperty(m_id, "type");
				else
					m_dataModel.setElementProperty(m_id, "type", types);
			}

			if (level != m_dataModel.getElementProperty(m_id, "level", "") )
			{
				if (level.empty() )
					m_dataModel.deleteElementProperty(m_id, "level");
				else
					m_dataModel.setElementProperty(m_id, "level", level);
			}
		}
		return true;
	}
}

} //namespace
