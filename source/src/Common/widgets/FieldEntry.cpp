/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "FieldEntry.h"

namespace tag {

FieldEntry::FieldEntry(int nbElements, Glib::ustring separator, guint elementSize)
{
	for (guint i = 0; i<nbElements; i++) {
		Gtk::Entry* entry = new Gtk::Entry() ;
		entries.insert(entries.end(), entry) ;
		pack_start(*entry, false, false) ;
		entry->set_width_chars(elementSize) ;
		entry->set_has_frame(false) ;
		entry->set_max_length(elementSize) ;
		if ( i!=(nbElements-1) ) { 
			Gtk::Entry* sep = new Gtk::Entry() ;
			sep->set_editable(false) ;
			//separator = " " + separator + " " ;
			sep->set_text(separator) ;
			sep->set_width_chars(separator.size()) ;
			label_separators.insert(label_separators.end(), sep) ;
			pack_start(*sep, false, false) ;
			sep->set_has_frame(false) ;
			sep->signal_grab_focus().connect(sigc::bind<int>(sigc::mem_fun(*this, &FieldEntry::on_separator_grab_focus), i)) ;
		}
	}
	show_all_children() ; 
}

FieldEntry::~FieldEntry()
{
	for (guint i=0; i < entries.size(); i++) 	{
		if (entries[i])
			delete(entries[i]) ;
	}
	for (guint i=0; i < label_separators.size(); i++) {
		if (label_separators[i])
			delete(label_separators[i]) ;
	}
}

Glib::ustring FieldEntry::get_element(guint indice)
{
	if ( indice>=entries.size() )  {
		return "" ;
	}
	else {
		return entries[indice]->get_text() ;
	}
}

bool FieldEntry::set_element(guint indice, Glib::ustring value)
{
	if ( indice>=entries.size() )  {
		return false ;
	}
	else if ( value.size()>entries[indice]->get_max_length() ) {
		gdk_beep() ;
	}
	else {
		entries[indice]->set_text(value) ;
		return true ;
	}
}

void FieldEntry::on_separator_grab_focus(guint i)
{
	if ( entries[i+1] ) {
		entries[i+1]->grab_focus() ;
	}
}

void FieldEntry::set_editable(bool editable)
{
	for (guint i=0; i<entries.size(); i++) {
		entries[i]->set_editable(editable) ;
	}
}

void FieldEntry::set_sensitive(bool sensitive)
{
	for (guint i=0; i<entries.size(); i++) {
		entries[i]->set_sensitive(sensitive) ;
	}
}

void FieldEntry::set_width_chars(int size) 
{
	//TODO
}

} // namespace
