/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Common/globals.h"
#include "Common/InputLanguageHandler.h"
#include "ComboEntry_mod.h"
#include "gtk/gtkimcontext.h"
#include "gtk/gtk.h"
#include "GtUtil.h"

namespace tag {

ComboEntry_mod::ComboEntry_mod()
{
	iLang = InputLanguageHandler::get_input_language_by_name(DEFAULT_LANGUAGE) ;
	arrow.signal_clicked().connect(sigc::mem_fun(*this, &ComboEntry_mod::myPopulate)) ;
	arrow.set_focus_on_click(false) ;
	arrow.set_sensitive(false) ;
	//set_name("arabic_combo") ;

	//> ICON for ARROW
	try {
		Glib::RefPtr<Gtk::IconTheme>theme = Gtk::IconTheme::get_default() ;
		Glib::RefPtr<Gdk::Pixbuf>pixbufPlay = theme->load_icon(COMBO_ENTRY_MOD_ICON, COMBO_ENTRY_MOD_ICON_SIZE,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
		image.set(pixbufPlay);
		arrow.set_image(image) ;
	}
	catch (Gtk::IconThemeError e) {
		Log::err() << "ComboEntry_mod:> " <<  e.what()  << std::endl ;
	}
}

ComboEntry_mod::~ComboEntry_mod()
{
}

void ComboEntry_mod::onPopupMenuPosition(int& x, int& y, bool& push_in)
{
	x=m_x ;
	y=m_y ;
	push_in = TRUE;
}

void ComboEntry_mod::myPopulate()
{
	int root_x,root_y ;
	get_window()->get_root_origin(root_x,root_y) ;

	int entry_x, entry_y ;
	get_window()->get_position(entry_x,entry_y) ;

	m_x = entry_x + root_x + 5;
	m_y = entry_y + root_y  + 45 ;

	if(menu.items().size() > 0)
		menu.popup(sigc::mem_fun(*this, &ComboEntry_mod::onPopupMenuPosition), 1, gtk_get_current_event_time());
}

void ComboEntry_mod::add_in_list(Glib::ustring word)
{
	menu.items().push_back( Gtk::Menu_Helpers::MenuElem(word,
		 sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &ComboEntry_mod::on_list), word ) ) );
	list[word] = 1 ;

	if (menu.items().size() > 1)
		arrow.set_sensitive(true) ;
}

bool ComboEntry_mod::on_key_press_event(GdkEventKey* event)
{
	//Do not process Shift and Ctrl Keys for their uses with arrows
	//in order to enable words selection or navigation
	bool shift = event->state & GDK_SHIFT_MASK ;
	bool ctrl = event->state & GDK_CONTROL_MASK ;
	bool shiftORctrl = ( shift || ctrl ) ;
	bool shiftANDctrl = (shift && ctrl ) ;

	bool is_fun_key = GtUtil::isFunctionKey(event->keyval) ;
	bool is_accel_key = GtUtil::isAccelKeyEvent(event) ;

	//> let all FUN keys
	if ( event->keyval == GDK_Shift_L
				|| event->keyval == GDK_Shift_R
				|| event->keyval == GDK_Control_L
				|| event->keyval == GDK_Control_R
				|| ( shiftORctrl && event->keyval == GDK_Left)
				|| ( shiftORctrl && event->keyval == GDK_Up)
				|| ( shiftORctrl && event->keyval == GDK_Right)
				|| ( shiftORctrl && event->keyval == GDK_Down)
				|| ( shiftORctrl && event->keyval == GDK_End)
				|| ( shiftORctrl && event->keyval == GDK_Home)
				|| ( (shiftORctrl||shiftANDctrl) && event->keyval == GDK_space)
				|| shift
				|| ctrl
				|| is_fun_key
				|| is_accel_key )
	{
		return Gtk::Entry::on_key_press_event(event);
	}

	if (iLang == NULL || !iLang->modifyMapping())  {
		return Gtk::Entry::on_key_press_event(event);
	}
	else {
		//> check out if selection
		int s,e ;
		bool has_sel = get_selection_bounds(s,e) ;

		//> compute map result character(s)
		Glib::ustring res ;
		if ( ! iLang->hasKeyMap(event, res) ) {
			int ret = Gtk::Entry::on_key_press_event(event);
			return ret;
		}
		else {
			if (has_sel)
				delete_selection() ;
			my_insert_text(res) ;
		}
		return false ;
	}
}

bool ComboEntry_mod::on_button_press_event(GdkEventButton* event)
{
	bool res = Gtk::Entry::on_button_press_event(event) ;
	if (event->button==1)
	{
		force_cursor() ;
	}
	return res ;
}

void ComboEntry_mod::force_cursor()
{
	//> dirty h@ck
	// if no text, force cursor apparition
	// bug ?? no cursor visible
	if (get_text().compare("")==0)
	{
		int useless ;
		insert_text(" ", 1, useless) ;
		delete_text(0,1) ;
	}
}

bool ComboEntry_mod::is_in_list(Glib::ustring word)
{
	if (list[word]==1)
		return true ;
	else
		return false ;
}

void ComboEntry_mod::on_list(Glib::ustring word)
{
	set_text(word) ;
}

void ComboEntry_mod::my_insert_text(const Glib::ustring& text)
{
	int current_pos = get_position() ;

	if (iLang->check_insertion_rules_str(get_text(), current_pos, text) ) {
		insert_text(text, text.bytes(), current_pos) ;
		current_pos = get_position() ;
		//> adjust cursor position
		int nb_char = text.size() ;
		set_position(current_pos+nb_char) ;
	}
	else
		gdk_beep() ;
}

void ComboEntry_mod::externalIMEcontrol(bool activate)
{
	Glib::signal_idle().connect(sigc::bind<bool>(sigc::mem_fun(*this, &ComboEntry_mod::externalIMEcontrol_afterIdle), activate)) ;
}

bool ComboEntry_mod::externalIMEcontrol_afterIdle(bool activate)
{
	grab_focus();
	//> grab_focus does selection on text, uncomment the 2 following lines
	//> for desactivating text auto-selection after focus
	//int p = get_position() ;
	//select_region(p,p) ;
	GtkWidget* widg = (GtkWidget*)(this->gobj()) ;
	GtkIMContext* context = (GTK_ENTRY(widg))->im_context ;
	GdkWindow* window =  gtk_widget_get_parent_window((GtkWidget*)this->gobj()) ;
	InputLanguageHandler::activate_external_IME(context, window, activate) ;
	return false ;
}

bool ComboEntry_mod::on_focus_in_event(GdkEventFocus* event)
{
	externalIMEcontrol(IME_on) ;
	return false ;
}


void ComboEntry_mod::clearMenu()
{
	Gtk::Menu::MenuList& menulist = menu.items() ;
	menulist.clear() ;
	arrow.set_sensitive(false) ;
}

}//NAMESPACE
