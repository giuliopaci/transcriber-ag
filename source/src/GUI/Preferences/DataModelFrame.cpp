/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "DataModelFrame.h"
#include "Explorer_utils.h"
#include "Common/globals.h"
#include "Common/util/Utils.h"

namespace tag {

DataModelFrame::DataModelFrame(Configuration* _config, Gtk::Window* _parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values)
	: PreferencesFrame(_config, _parent, _("Data Model"), _dynamic_values, _static_values)
{
	lock_combo = false ;
	convention_frame.set_label(_("Change default annotation conventions")) ;
	convention_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;

	//> CONVENTIONS and LANGUAGE
	vbox.pack_start(convention_frame, false, false, TAG_PREFERENCESFRAME_SPACE) ;
		convention_frame.add(convention_vbox) ;
			convention_vbox.pack_start(conv_hbox, false, false, 7) ;
				conv_hbox.pack_start(conv_label, false, false, 5) ;
				conv_hbox.pack_start(conv_combo, false, false, 5) ;
			convention_vbox.pack_start(lang_hbox, false, false, 7) ;
				lang_hbox.pack_start(lang_label, false, false, 5) ;
				lang_hbox.pack_start(lang_combo, false, false, 5) ;

	Glib::ustring space = "  " ;
	Glib::ustring lab =  space + _("Convention") ;
	conv_label.set_label(lab) ;

	lab =  space + _("Language") ;
	lang_label.set_label(lab) ;

	conv_combo.signal_changed().connect( sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &DataModelFrame::on_convention_combo_changed), "conv")) ;
	lang_combo.signal_changed().connect( sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &DataModelFrame::on_convention_combo_changed), "lang")) ;

	//> PREPARE STATE
	reload_data() ;
	modified(false) ;
}

DataModelFrame::~DataModelFrame()
{

}

void DataModelFrame::reload_data()
{
	lock_data = true ;
	prepare_combo() ;
	lock_data = false ;
}

void DataModelFrame::prepare_combo()
{
	lang_combo.clear() ;
	conv_combo.clear() ;

	Glib::ustring conventions = config->get_DATAMODEL_conventions() ;
	Glib::ustring conv_default = config->get_DATAMODEL_defaultConvention() ;

	std::vector<Glib::ustring> v ;
	int nb = mini_parser(';', conventions, &v) ;

	for(guint i=0; i<v.size(); i++) {
		conv_combo.append_text(v[i]) ;
		if (v[i]==conv_default)
			conv_combo.set_active_text(v[i]) ;
	}

	Glib::ustring languages = config->get_DATAMODEL_languages() ;
	Glib::ustring lang_default = config->get_DATAMODEL_defaultLanguage() ;

	v.clear() ;
	nb = mini_parser(';', languages, &v) ;

	for(guint i=0; i<v.size(); i++) {
		lang_combo.append_text(v[i]) ;
		if (v[i]==lang_default)
			lang_combo.set_active_text(v[i]) ;
	}
}

void DataModelFrame::on_convention_combo_changed(Glib::ustring type)
{
	if (!lock_data) {
		//for convention
		if (type=="conv") {
			Glib::ustring txt = conv_combo.get_active_text() ;
			if (txt!="" && txt!=" ") {
				config->set_DATAMODEL_defaultConvention(txt,false) ;
				modified(true) ;
			}
		}
		//for language
		else if (type=="lang") {
			Glib::ustring txt = lang_combo.get_active_text() ;
			if (txt!="" && txt!=" ") {
				config->set_DATAMODEL_defaultLanguage(txt, false) ;
				modified(true) ;
			}
		}
	}
}

} //namespace
