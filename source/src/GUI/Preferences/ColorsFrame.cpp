/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "ColorsFrame.h"

namespace tag {

ColorsFrame::ColorsFrame(Configuration* config, Gtk::Window* parent, std::map<int, Glib::ustring>* _dynamic_values, std::vector<IcoPackImage*>* _static_values)
: PreferencesFrame(config, parent, _("Colors"), _dynamic_values, _static_values)
{
	editor_event_table = NULL ;
	editor_entity_table = NULL ;
	editor_text_table = NULL ;
	editor_background_table = NULL ;
	editor_highlight_table = NULL ;
	audio_signal_table = NULL ;


	//********* EDITOR PART

	//> PACKING
	vbox.pack_start(editor_expander, false, false, 10) ;
		editor_expander.set_label(_("Annotation Editor")) ;
		Gtk::Widget* lab = editor_expander.get_label_widget() ;
		if (lab)
			lab->set_name("bold_label") ;
		editor_expander.add(editor_vbox) ;

		//> textS
		editor_vbox.pack_start(editor_text_frame, false, false, 5) ;
			editor_text_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
			editor_text_frame.set_label(_("Text")) ;
			editor_text_frame.add(editor_text_vbox) ;
				editor_text_vbox.pack_start(editor_text_hbox, false, false, 3) ;
					add_color_table(editor_text_hbox, editor_text_table, "editor_text") ;

			//> EVENTS
			editor_vbox.pack_start(editor_event_frame, false, false, 5) ;
				editor_event_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
				editor_event_frame.set_label(_("Events")) ;
				editor_event_frame.add(editor_event_vbox) ;
					editor_event_vbox.pack_start(editor_event_hbox, false, false, 3) ;
						add_color_table(editor_event_hbox, editor_event_table, "editor_event") ;
						editor_event_hbox.pack_start(warning_event, false, false, 3) ;

			//> ENTITIES
			editor_vbox.pack_start(editor_entity_frame, false, false, 5) ;
				editor_entity_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
				editor_entity_frame.set_label(_("Named Entities")) ;
				editor_entity_frame.add(editor_entity_vbox) ;
				editor_entity_vbox.pack_start(editor_entity_hbox, false, false, 3) ;
					add_color_table(editor_entity_hbox, editor_entity_table, "editor_entity") ;
					editor_entity_hbox.pack_start(warning_entity, false, false, 3) ;

			//> BACKGROUND
			editor_vbox.pack_start(editor_background_frame, false, false, 5) ;
				editor_background_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
				editor_background_frame.set_label(_("Background")) ;
				editor_background_frame.add(editor_background_vbox) ;
				editor_background_vbox.pack_start(editor_background_hbox, false, false, 3) ;
					add_color_table(editor_background_hbox, editor_background_table, "editor_background") ;
					editor_background_hbox.pack_start(warning_background, false, false, 3) ;

			//> HIGHLIGHT
			editor_vbox.pack_start(editor_highlight_frame, false, false, 5) ;
				editor_highlight_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
				editor_highlight_frame.set_label(_("Highlight")) ;
				editor_highlight_frame.add(editor_highlight_vbox) ;
				editor_highlight_vbox.pack_start(editor_highlight_hbox, false, false, 3) ;
					add_color_table(editor_highlight_hbox, editor_highlight_table, "editor_highlight") ;
					editor_highlight_hbox.pack_start(warning_highlight, false, false, 3) ;

	vbox.pack_start(audio_expander, false, false, 10) ;
		audio_expander.set_label(_("Audio panel")) ;
		lab = audio_expander.get_label_widget() ;
		if (lab)
			lab->set_name("bold_label") ;
		audio_expander.add(audio_vbox) ;
			audio_vbox.pack_start(audio_signal_frame, false, false, 5) ;
				audio_signal_frame.set_label_align(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
				audio_signal_frame.set_label(_("Signal")) ;
				audio_signal_frame.add(audio_signal_vbox) ;
					add_color_table(audio_signal_vbox, audio_signal_table, "audio") ;

	warning_images.insert(warning_images.begin(), &warning_entity) ;
	warning_images.insert(warning_images.begin(), &warning_event) ;
	warning_images.insert(warning_images.begin(), &warning_highlight) ;
	warning_images.insert(warning_images.begin(), &warning_background) ;

	editor_expander.set_expanded(false) ;
	audio_expander.set_expanded(false) ;

	reload_data() ;
	modified(false) ;

	show_all() ;
}


void ColorsFrame::add_color_table(Gtk::Box& container, DynamicTable* dtable, Glib::ustring type)
{
	dtable = new DynamicTable(1, 3, false) ;
	container.pack_start(*dtable, false, false, 6) ;

	dtable->set_padding_options(15, 6) ;
	dtable->set_attach_options(Gtk::SHRINK, Gtk::SHRINK) ;

	//entete
	Gtk::Label* lab0 = Gtk::manage(new Gtk::Label("") ) ;
	Gtk::Label* lab1 = Gtk::manage(new Gtk::Label(_("Foreground")) ) ;
	Gtk::Label* lab2 = Gtk::manage(new Gtk::Label(_("Background")) ) ;
	std::vector<Gtk::Widget*> labels ;
	labels.push_back(lab0);
	labels.push_back(lab1);
	labels.push_back(lab2);
	dtable->add_row(labels) ;

	//elements
	//TODO: dynamic
	std::vector<ColorsFrame::UnitFgBg*> units = get_colors_units(type) ;
	std::vector<ColorsFrame::UnitFgBg*>::iterator it ;

	std::vector<Gtk::Widget*> row_elem ;
	for (it=units.begin(); it!=units.end(); it++) {
		UnitFgBg* current = *it ;
		if (current) {
			row_elem.clear() ;
			row_elem.push_back(&(current->name)) ;

			if (current->fg) {
				current->color_fg.signal_color_set().connect(sigc::bind<UnitFgBg*,Glib::ustring>(sigc::mem_fun(*this, &ColorsFrame::on_colorButtons_change), current, "fg")) ;
				row_elem.push_back(&(current->color_fg)) ;
			}
			else
				row_elem.push_back(&(current->blank_fg)) ;

			if (current->bg) {
				current->color_bg.signal_color_set().connect(sigc::bind<UnitFgBg*,Glib::ustring>(sigc::mem_fun(*this, &ColorsFrame::on_colorButtons_change), current, "bg")) ;
				row_elem.push_back(&(current->color_bg)) ;
			}
			else
				row_elem.push_back(&(current->blank_bg)) ;

			dtable->add_row(row_elem) ;
		}
	}
}

std::vector<ColorsFrame::UnitFgBg*> ColorsFrame::get_colors_units(Glib::ustring table_type)
{
	//TODO: get all dynamically
	ColorsFrame::UnitFgBg* newUnit = NULL ;
	Glib::ustring element_type ;

	std::vector<ColorsFrame::UnitFgBg*> res ;
	if (table_type=="editor_event") {
		element_type = table_type ;
		newUnit = new ColorsFrame::UnitFgBg(_("Events"), true, true, element_type) ;
		units[element_type] = newUnit ;
		res.push_back(newUnit) ;
	}
	else if (table_type=="editor_text") {
		element_type = table_type ;
		newUnit = new ColorsFrame::UnitFgBg(_("Text"), true, true, element_type) ;
		units[element_type] = newUnit ;
		res.push_back(newUnit) ;
	}
	else if (table_type=="editor_entity") {
		element_type = table_type ;
		newUnit = new ColorsFrame::UnitFgBg(_("Default entities"), true, true, element_type) ;
		units[element_type]= newUnit ;
		res.push_back(newUnit) ;
	}
	else if (table_type=="editor_background") {
		element_type = table_type ;
		newUnit = new ColorsFrame::UnitFgBg(_("Background"), true, true, element_type) ;
		units[element_type]= newUnit ;
		res.push_back(newUnit) ;
	}
	else if (table_type=="editor_highlight") {
		element_type = table_type+"_1" ;
		newUnit = new ColorsFrame::UnitFgBg(_("Track 1 highlight"), false, true, element_type) ;
		units[element_type]= newUnit ;
		res.push_back(newUnit) ;

		element_type = table_type+"_2" ;
		newUnit = new ColorsFrame::UnitFgBg(_("Track 2 highlight "), false, true, element_type) ;
		units[element_type] = newUnit ;
		res.push_back(newUnit) ;
	}
	else if (table_type=="audio") {
		element_type = table_type + "_signal" ;
		newUnit = new ColorsFrame::UnitFgBg(_("Signal"), true, true, element_type) ;
		units[element_type]= newUnit ;
		res.push_back(newUnit) ;

		element_type = table_type + "_activated" ;
		newUnit = new ColorsFrame::UnitFgBg(_("Activated signal"), false, true, element_type) ;
		units[element_type]= newUnit ;
		res.push_back(newUnit) ;

		element_type = table_type + "_disabled" ;
		newUnit = new ColorsFrame::UnitFgBg(_("Disabled signal"), false, true, element_type) ;
		units[element_type]= newUnit ;
		res.push_back(newUnit) ;

		element_type = table_type + "_selection" ;
		newUnit = new ColorsFrame::UnitFgBg(_("Selection"), false, true, element_type) ;
		units[element_type]= newUnit ;
		res.push_back(newUnit) ;

		element_type = table_type + "_tooltip" ;
		newUnit = new ColorsFrame::UnitFgBg(_("Selection tooltip"), true, true, element_type) ;
		units[element_type]= newUnit ;
		res.push_back(newUnit) ;

		element_type = table_type + "_cursor" ;
		newUnit = new ColorsFrame::UnitFgBg(_("Cursor"), true, false, element_type) ;
		units[element_type]= newUnit ;
		res.push_back(newUnit) ;

		element_type = table_type + "_segmentEnd" ;
		newUnit = new ColorsFrame::UnitFgBg(_("Segment end indicator"), true, false, element_type) ;
		units[element_type]= newUnit ;
		res.push_back(newUnit) ;
	}
	return res ;
}


void ColorsFrame::on_colorButtons_change(ColorsFrame::UnitFgBg* unit, Glib::ustring g_type)
{
	if (lock_data)
		return ;

	if (!unit) {
		Log::err() << "<!> ColorsFrame::on_colorButtons_change:> invalid color unit" << std::endl ;
		return ;
	}
	Glib::ustring type = unit->id ;
	Glib::ustring color ;
	int config_type ;
	color = unit->get_color(g_type) ;

	modified(true) ;

	if (type=="editor_event") {
		config->set_COLORS_EDITOR_event(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_EDITOR ;
		set_warnings(&warning_event,1) ;
	}
	if (type=="editor_text") {
		config->set_COLORS_EDITOR_text(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_EDITOR ;
		set_formatted_string_dynamic_value(TAG_PREFERENCES_PARAM_COLORS_EDITOR, "") ;
	}
	if (type=="editor_entity") {
		config->set_COLORS_EDITOR_entity(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_EDITOR ;
		set_warnings(&warning_entity,1) ;
	}
	if (type=="editor_background") {
		config->set_COLORS_EDITOR_background(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_EDITOR ;
		set_warnings(&warning_background,1) ;
	}
	else if (type=="editor_highlight_1"){
		config->set_COLORS_EDITOR_highlight1(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_EDITOR ;
		set_warnings(&warning_highlight, 1) ;
	}
	else if (type=="editor_highlight_2") {
		config->set_COLORS_EDITOR_highlight2(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_EDITOR ;
		set_warnings(&warning_highlight, 1) ;
	}
	else if (type== "audio_signal") {
		config->set_COLORS_AUDIO_signal(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_AUDIO ;
	}
	else if (type== "audio_activated") {
		config->set_COLORS_AUDIO_signalActivated(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_AUDIO ;
	}
	else if (type== "audio_disabled") {
		config->set_COLORS_AUDIO_signalDisabled(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_AUDIO ;
	}
	else if (type== "audio_selection") {
		config->set_COLORS_AUDIO_signalSelection(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_AUDIO ;
	}
	else if (type== "audio_tooltip") {
		config->set_COLORS_AUDIO_signalTooltip(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_AUDIO ;
	}
	else if (type== "audio_cursor") {
		config->set_COLORS_AUDIO_signalCursor(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_AUDIO ;
	}
	else if (type== "audio_segmentEnd") {
		config->set_COLORS_AUDIO_signalSegmentEnd(color, false, g_type) ;
		config_type = TAG_PREFERENCES_PARAM_COLORS_AUDIO ;
	}

	set_formatted_string_dynamic_value(config_type, color) ;
}

void ColorsFrame::reload_data()
{
	lock_data = true ;

	//>> EDITOR
	ColorsFrame::UnitFgBg* unit ;
	Glib::ustring color ;
	//event
	unit = get_unit("editor_event") ;
	if (unit) {
		color = config->get_COLORS_EDITOR_event("fg") ;
		unit->set_color("fg", color) ;
		color = config->get_COLORS_EDITOR_event("bg") ;
		unit->set_color("bg", color) ;
	}
	//text
	unit = get_unit("editor_text") ;
	if (unit) {
		color = config->get_COLORS_EDITOR_text("fg") ;
		unit->set_color("fg", color) ;
		color = config->get_COLORS_EDITOR_text("bg") ;
		unit->set_color("bg", color) ;
	}
	//entity
	unit = get_unit("editor_entity") ;
	if (unit) {
		color = config->get_COLORS_EDITOR_entity("fg") ;
		unit->set_color("fg", color) ;
		color = config->get_COLORS_EDITOR_entity("bg") ;
		unit->set_color("bg", color) ;
	}
	//background
	unit = get_unit("editor_background") ;
	if (unit) {
		color = config->get_COLORS_EDITOR_background("fg") ;
		unit->set_color("fg", color) ;
		color = config->get_COLORS_EDITOR_background("bg") ;
		unit->set_color("bg", color) ;
	}
	//highlight
	unit = get_unit("editor_highlight_1") ;
	if (unit) {
		color = config->get_COLORS_EDITOR_highlight1("bg") ;
		unit->set_color("bg", color) ;
	}
	unit = get_unit("editor_highlight_2") ;
	if (unit) {
		color = config->get_COLORS_EDITOR_highlight2("bg") ;
		unit->set_color("bg", color) ;
	}

	//AUDIO
	unit = get_unit("audio_signal") ;
	if (unit) {
		color = config->get_COLORS_AUDIO_signal("fg") ;
		unit->set_color("fg", color) ;
		color = config->get_COLORS_AUDIO_signal("bg") ;
		unit->set_color("bg", color) ;
	}
	unit = get_unit("audio_tooltip") ;
	if (unit) {
		color = config->get_COLORS_AUDIO_signalTooltip("fg") ;
		unit->set_color("fg", color) ;
		color = config->get_COLORS_AUDIO_signalTooltip("bg") ;
		unit->set_color("bg", color) ;
	}
	unit = get_unit("audio_activated") ;
	if (unit) {
		color = config->get_COLORS_AUDIO_signalActivated("bg") ;
		unit->set_color("bg", color) ;
	}
	unit = get_unit("audio_disabled") ;
	if (unit) {
		color = config->get_COLORS_AUDIO_signalDisabled("bg") ;
		unit->set_color("bg", color) ;
	}
	unit = get_unit("audio_selection") ;
	if (unit) {
		color = config->get_COLORS_AUDIO_signalSelection("bg") ;
		unit->set_color("bg", color) ;
	}
	unit = get_unit("audio_segmentEnd") ;
	if (unit) {
		color = config->get_COLORS_AUDIO_signalSegmentEnd("fg") ;
		unit->set_color("fg", color) ;
	}
	unit = get_unit("audio_cursor") ;
	if (unit) {
		color = config->get_COLORS_AUDIO_signalCursor("fg") ;
		unit->set_color("fg", color) ;
	}

	lock_data = false ;
}

ColorsFrame::~ColorsFrame()
{
	if (editor_event_table)
		delete(editor_event_table) ;
	if (editor_entity_table)
		delete(editor_entity_table) ;
	if (editor_text_table)
		delete(editor_text_table) ;
	if (editor_background_table)
		delete(editor_background_table) ;
	if (editor_highlight_table)
		delete(editor_highlight_table) ;
	if (audio_signal_table)
		delete(audio_signal_table) ;
	std::map<Glib::ustring, ColorsFrame::UnitFgBg*>::iterator it ;
	for (it=units.begin(); it!= units.end(); it++) {
		if (it->second)
			delete(it->second) ;
	}
}

ColorsFrame::UnitFgBg* ColorsFrame::get_unit(Glib::ustring element_type)
{
	std::map<Glib::ustring, ColorsFrame::UnitFgBg*>::iterator it ;
	it = units.find(element_type) ;
	if (it!=units.end())
		return it->second ;
	else
		return NULL ;
}

} //namespace
