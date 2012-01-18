/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "ProgressionWatcher.h"
#include "Common/icons/Icons.h"
#include "Explorer_utils.h"

namespace tag {

ProgressionWatcher::ProgressionWatcher(Glib::ustring title, Glib::ustring image)
{
	add(vbox_gen) ;
	b_close = new Gtk::Button(Gtk::Stock::CLOSE) ;
	b_clean = new Gtk::Button(Gtk::Stock::CLEAR) ;

	set_position(Gtk::WIN_POS_CENTER) ;
	resize(400,112);
	Icons::set_window_icon(this, ICO_TRANSCRIBER, 11) ;

	if (title.empty())
		title = TRANSAG_DISPLAY_NAME ;

	set_title(title) ;
	label_title.set_label(title) ;
	label_title.set_name("bold_label") ;

	vbox_gen.pack_start(presentation_box, false, false, 3) ;
		presentation_box.pack_start(align_title, false, false, 7) ;
			align_title.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;
			align_title.add(label_title) ;

	if (!image.empty()) {
		gen_image.set_image(image, 30) ;
		presentation_box.pack_start(align_image, false, false, 5) ;
			align_image.set(Gtk::ALIGN_RIGHT, Gtk::ALIGN_RIGHT, 0.0, 0.0) ;
			align_image.add(gen_image) ;
	}

	vbox_gen.pack_start(scrollW,true,true) ;
		scrollW.add(vbox_data) ;
	vbox_gen.pack_start(align_buttons,false,false, 7) ;
		align_buttons.add(hbox_buttons) ;
		align_buttons.set(Gtk::ALIGN_RIGHT, Gtk::ALIGN_RIGHT, 0.0, 0.0) ;
			hbox_buttons.pack_start(*b_clean,false,false,2) ;
			hbox_buttons.pack_start(*b_close,false,false,2) ;

	scrollW.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);

	b_clean->signal_clicked().connect(sigc::mem_fun(*this, &ProgressionWatcher::clean)) ;
	b_close->signal_clicked().connect(sigc::mem_fun(*this, &Gtk::Widget::hide)) ;

	set_focus(*b_close) ;

	show_all_children(true) ;
}

ProgressionWatcher::~ProgressionWatcher()
{
	for (guint i=0; i<entries.size(); i++){
		delete(entries[i]) ;
	}

	if (b_close)
		delete(b_close) ;
	if (b_clean)
		delete(b_clean) ;
}

ProgressionWatcher::Entry* ProgressionWatcher::add_entry(Glib::ustring label, Glib::ustring file, Glib::ustring legend, bool do_present)
{
	//create entry
	entries.insert(entries.end(), new Entry(label, file, legend)) ;

	Entry* e = entries[entries.size()-1] ;
	vbox_data.pack_end(*(e->get_separator()),false,false);
	vbox_data.pack_end(*e,false,false);

	show_all_children(true) ;

	//raise
	if (do_present)
		present() ;

	return e ;
}

void ProgressionWatcher::clean()
{
	std::vector<ProgressionWatcher::Entry*>::iterator it =  entries.begin() ;
	while(it!=entries.end()){
		if ((*it)->is_finished()) {
			Gtk::Widget* w = (Gtk::Widget*) (*it) ;
			Gtk::Widget* s = (*it)->get_separator() ;
			vbox_data.remove(*s) ;
			vbox_data.remove(*w) ;
			ProgressionWatcher::Entry* tmp = *it ;
			it = entries.erase(it) ;
			delete(tmp) ;
		}
		else
			it++ ;
	}
}

} //namespace

