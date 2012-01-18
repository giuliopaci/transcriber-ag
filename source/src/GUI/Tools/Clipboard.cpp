/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Clipboard.h"
#include "Explorer_utils.h"
#include "Explorer_dialog.h"
#include "Common/icons/Icons.h"
#include "Common/util/Utils.h"

namespace tag {

Clipboard::Clipboard(Configuration* _config, Gtk::Window* win)
{
	set_title("Clipboard AG") ;
	set_modal(false) ;
	set_skip_pager_hint(true) ;
	set_skip_taskbar_hint(true) ;
	Icons::set_window_icon(this, ICO_CLIPBOARD, 17) ;

	first_tour = true ;
	reverse_sort = false ;

	table_button = NULL ;

	config = _config ;
	window = win ;

	Gtk::Image* up = Gtk::manage(new Gtk::Image(Gtk::Stock::GO_UP, Gtk::ICON_SIZE_SMALL_TOOLBAR)) ;
	Gtk::Image* down = Gtk::manage(new Gtk::Image(Gtk::Stock::GO_DOWN, Gtk::ICON_SIZE_SMALL_TOOLBAR)) ;
	button_up = new Gtk::Button() ;
	button_down = new Gtk::Button() ;
	button_up->set_image(*up) ;
	button_down->set_image(*down) ;

	button_export = new IcoPackButton() ;
	button_import = new IcoPackButton() ;
	button_clear = new Gtk::Button(_("Delete All")) ;
	button_suppress = new Gtk::Button(_("Delete")) ;
	button_sort_Asc = new IcoPackButton() ;
	button_sort_Desc = new IcoPackButton() ;

	button_export->set_icon(ICO_CLIPBOARD_OUT, "", 22, "") ;
	button_import->set_icon(ICO_CLIPBOARD_IN, "", 22, "") ;
	button_sort_Asc->set_icon(ICO_CLIPBOARD_SORT_ASC, "", 22, "") ;
	button_sort_Desc->set_icon(ICO_CLIPBOARD_SORT_DESC, "", 22, "") ;

	Glib::ustring tmp = _("Up selection") ;
	Glib::ustring tmp2 = _("Alt + Up") ;
	tooltip.set_tip(*button_up, tmp + "\n" + tmp2) ;

	tmp = _("Down selection") ;
	tmp2 = _("Alt + Down") ;
	tooltip.set_tip(*button_down, tmp + "\n" + tmp2) ;

	tmp = _("Paste selection in editor") ;
	tmp2 = _("Alt + Left") ;
	tooltip.set_tip(*button_export, tmp + "\n" + tmp2) ;

	tmp = _("Keep editor selection in clipboard") ;
	tmp2 = _("Alt + Right") ;
	tooltip.set_tip(*button_import, tmp + "\n" + tmp2) ;

	tmp = _("Suppress all clipboard") ;
	tmp2 = _("Alt + Shift + Space") ;
	tooltip.set_tip(*button_clear, tmp + "\n" + tmp2 ) ;

	tmp = _("Suppress clipboard selection") ;
	tmp2 = _("Alt + Shift + Suppress") ;
	tooltip.set_tip(*button_suppress, tmp + "\n" + tmp2) ;

	tmp = _("A -> Z") ;
	//tmp2 = _("Alt + Shift + Space") ;
	tooltip.set_tip(*button_sort_Asc, tmp) ;

	tmp = _("Z -> A") ;
	//tmp2 = _("Alt + Shift + Suppress") ;
	tooltip.set_tip(*button_sort_Desc, tmp) ;


	tview = new Gtk::TextView(Gtk::TextBuffer::create()) ;

	tview->set_editable(false) ;
 	buffer = tview->get_buffer() ;

	Gtk::VBox* vbox = get_vbox() ;

	table_button = new Gtk::Table(2,3,false) ;
	table_button->set_col_spacings(10) ;
	table_button->set_row_spacings(5) ;

	align_button.add(*table_button) ;

	table_button->attach(*button_up, 0, 1, 0, 1, Gtk::FILL, Gtk::EXPAND , 0, 0) ;
	table_button->attach(*button_down, 0, 1, 1, 2, Gtk::FILL, Gtk::EXPAND , 0, 0) ;
	table_button->attach(*button_export, 1, 2, 0, 1, Gtk::FILL, Gtk::EXPAND , 0, 0) ;
	table_button->attach(*button_import, 1, 2, 1, 2, Gtk::FILL, Gtk::EXPAND , 0, 0) ;
	table_button->attach(*button_sort_Asc, 2, 3, 0, 1, Gtk::FILL, Gtk::EXPAND , 0, 0) ;
	table_button->attach(*button_sort_Desc, 2, 3, 1, 2, Gtk::FILL, Gtk::EXPAND , 0, 0) ;
	table_button->attach(*button_clear, 3, 4, 0, 1, Gtk::FILL, Gtk::EXPAND , 0, 0) ;
	table_button->attach(*button_suppress, 3, 4, 1, 2, Gtk::FILL, Gtk::EXPAND , 0, 0) ;

	vbox->pack_start(align_button, false, false) ;
	vbox->pack_start(sep, false, false) ;
	vbox->pack_start(scroll, true, true) ;
	scroll.add(*tview) ;
	scroll.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	vbox->show_all_children() ;
	align_button.set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0) ;

	//internal mecanism
	button_up->signal_clicked().connect(sigc::mem_fun(*this, &Clipboard::up_selection));
	button_down->signal_clicked().connect(sigc::mem_fun(*this, &Clipboard::down_selection));
	button_export->signal_clicked().connect(sigc::mem_fun(*this, &Clipboard::export_text));
	button_import->signal_clicked().connect(sigc::mem_fun(*this, &Clipboard::import_text));
	button_clear->signal_clicked().connect(sigc::mem_fun(*this, &Clipboard::clear));
	button_suppress->signal_clicked().connect(sigc::mem_fun(*this, &Clipboard::suppress));
	button_sort_Asc->signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &Clipboard::set_list_order), false)) ;
	button_sort_Desc->signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &Clipboard::set_list_order), true)) ;

	button_up->set_focus_on_click(false) ;
	button_down->set_focus_on_click(false) ;
	button_export->set_focus_on_click(false) ;
	button_import->set_focus_on_click(false) ;
	button_clear->set_focus_on_click(false) ;
	button_suppress->set_focus_on_click(false) ;
	button_sort_Asc->set_focus_on_click(false) ;
	button_sort_Desc->set_focus_on_click(false) ;
	//> action creation
	actionGr = Gtk::ActionGroup::create("clipboardAG") ;

	actionGr->add( Gtk::Action::create("clipboard_up", _("_Previous entry"), _("Previous clipboard entry")),
		Gtk::AccelKey("<mod1>Up"),
		sigc::mem_fun(*this, &Clipboard::up_selection));
	actionGr->add( Gtk::Action::create("clipboard_down", _("_Next entry"), _("Next clipboard entry")),
		Gtk::AccelKey("<mod1>Down"),
		sigc::mem_fun(*this, &Clipboard::down_selection));
	actionGr->add( Gtk::Action::create("clipboard_export", _("_Export from clipboard"), _("Export from clipboard")),
		Gtk::AccelKey("<mod1>Left"),
		sigc::mem_fun(*this, &Clipboard::export_text));
	actionGr->add( Gtk::Action::create("clipboard_import", _("_Import in clipboard"), _("Import in clipboard")),
		Gtk::AccelKey("<mod1>Right"),
		sigc::mem_fun(*this, &Clipboard::import_text));
	actionGr->add( Gtk::Action::create("clipboard_clear", _("_Clear clipboard"), _("Clear all clipboard")),
		Gtk::AccelKey("<mod1><shift>space"),
		sigc::mem_fun(*this, &Clipboard::clear));
	actionGr->add( Gtk::Action::create("clipboard_suppress", _("_Suppress clipboard selection"), _("Suppress selected line in clipboard")),
		Gtk::AccelKey("<mod1><shift>Delete"),
		sigc::mem_fun(*this, &Clipboard::suppress));
	actionGr->add( Gtk::Action::create("clipboard_sort_asc", _("A -> Z"), _("Alphabetic order")),
		Gtk::AccelKey("<mod1><shift>Home"),
		sigc::bind<bool>(sigc::mem_fun(*this, &Clipboard::set_list_order), true));
	actionGr->add( Gtk::Action::create("clipboard_sort_desc", _("Z -> A"), _("Reverse Alphabetic")),
		Gtk::AccelKey("<mod1><shift>End"),
		sigc::bind<bool>(sigc::mem_fun(*this, &Clipboard::set_list_order), true));
	actionGr->add( Gtk::Action::create("clipboard_display", _("_Display / Hide clipboard"), _("Display / Hide clipboard")),
		Gtk::AccelKey("<mod1><shift>C"), sigc::mem_fun(*this, &Clipboard::display_clipboard));

	activate_clear(false) ;
	activate_suppress(false) ;
	activate_up(false) ;
	activate_down(false) ;
	activate_import(false) ;
	activate_export(false) ;
	activate_sortAsc(false) ;
	activate_sortDesc(false) ;

	activate(false) ;

	//TODO load in config
	can_focus = false ;

	if (window)
		set_transient_for(*window) ;
}

Clipboard::~Clipboard()
{
	if(button_up)
		delete(button_up) ;
	if(button_down)
		delete(button_down) ;
	if(button_clear)
		delete(button_clear) ;
	if(button_suppress)
		delete(button_suppress) ;
	if(button_export)
		delete(button_export) ;
	if(button_import)
		delete(button_import) ;
	if(table_button)
		delete(table_button) ;
	if(button_sort_Asc)
		delete(button_sort_Asc) ;
	if(button_sort_Desc)
		delete(button_sort_Desc) ;
}


//**************************************************************************************
//***************************************************************************** BUSINESS
//**************************************************************************************

void Clipboard::activate_sortAsc(bool value)
{
	button_sort_Asc->set_sensitive(value) ;
	actionGr->get_action("clipboard_sort_asc")->set_sensitive(value) ;
}

void Clipboard::activate_sortDesc(bool value)
{
	button_sort_Desc->set_sensitive(value) ;
	actionGr->get_action("clipboard_sort_desc")->set_sensitive(value) ;
}

void Clipboard::activate_up(bool value)
{
	button_up->set_sensitive(value) ;
	actionGr->get_action("clipboard_up")->set_sensitive(value) ;
}

void Clipboard::activate_down(bool value)
{
	button_down->set_sensitive(value) ;
	actionGr->get_action("clipboard_down")->set_sensitive(value) ;
}


void Clipboard::activate_import(bool value)
{
	button_import->set_sensitive(value) ;
	actionGr->get_action("clipboard_import")->set_sensitive(value) ;
}

void Clipboard::activate_export(bool value)
{
	button_export->set_sensitive(value) ;
	actionGr->get_action("clipboard_export")->set_sensitive(value) ;
}

void Clipboard::activate_suppress(bool value)
{
	button_suppress->set_sensitive(value) ;
	actionGr->get_action("clipboard_suppress")->set_sensitive(value) ;
}

void Clipboard::activate_clear(bool value)
{
	button_clear->set_sensitive(value) ;
	actionGr->get_action("clipboard_clear")->set_sensitive(value) ;
}



void Clipboard::activate(bool value)
{
	active=value ;
	activate_clear(value) ;
	activate_suppress(value) ;
	activate_up(value) ;
	activate_down(value) ;
	activate_import(value) ;
	activate_export(value) ;
	activate_sortAsc(value) ;
	activate_sortDesc(value) ;
}

void Clipboard::set_editor(AnnotationEditor* editor)
{
	external_editor = editor ;
	set_buffer(editor->getActiveView()->getBuffer()) ;
}

void Clipboard::set_buffer(Glib::RefPtr<AnnotationBuffer> ext_buf)
{
	//external_buffer = ext_buf ;
	// stock external connection
	if (external_on_change_signal)
		external_on_change_signal.disconnect() ;
	external_on_change_signal = get_external_buffer()->signal_mark_set().connect(sigc::mem_fun(*this, &Clipboard::on_change_cursor));
	buffer->signal_mark_set().connect(sigc::mem_fun(*this, &Clipboard::on_change_cursor));

	//actualize
	show_all_children() ;

	active=true ;
}

void Clipboard::close(bool needhide)
{
	activate(false) ;
	external_on_change_signal.disconnect() ;
	if (needhide)
		saveGeoAndHide() ;
}

void Clipboard::up_selection()
{
	if (active)
	{
		Gtk::TextIter current_iter ;
		Gtk::TextIter start_iter ;
		Gtk::TextIter end_iter ;

		int nb_line = buffer->get_line_count() ;

		//> if text is selected, go to next upper
		if ( buffer->get_selection_bounds(start_iter, end_iter) ) {
			//remonter
			if (end_iter.get_line()>0) {
				start_iter.backward_visible_line() ;
				end_iter.backward_visible_line() ;
				tview->forward_display_line_end(end_iter) ;
				buffer->select_range(start_iter, end_iter);
			}
			if ( buffer->get_slice(start_iter, end_iter) == "\n"
					|| buffer->get_slice(start_iter, end_iter) == "" ) {
				start_iter.backward_visible_line() ;
				end_iter.backward_visible_line() ;
				tview->forward_display_line_end(end_iter) ;
				buffer->select_range(start_iter, end_iter);
			}
		}
		//else select last entry
		else {
			start_iter = buffer->get_iter_at_line(nb_line-1) ;
			end_iter = start_iter ;
			tview->forward_display_line_end(end_iter) ;
			buffer->select_range(start_iter, end_iter);
		}
		//> if no selected text, use current markup
		tview->scroll_to_iter(start_iter, 0.1) ;
	}//end is active
}

void Clipboard::down_selection()
{
	if (active) {
		Gtk::TextIter current_iter ;
		Gtk::TextIter start_iter ;
		Gtk::TextIter end_iter ;

		int nb_line = buffer->get_line_count() ;

		//> if selection, go to next downer
		if ( buffer->get_selection_bounds(start_iter, end_iter) ) {
			if (end_iter.get_line() < nb_line-1 ) {
				start_iter.forward_visible_line() ;
				end_iter.forward_visible_line() ;
				tview->forward_display_line_end(end_iter) ;
				if ( buffer->get_slice(start_iter, end_iter) != "\n"  && buffer->get_slice(start_iter, end_iter) != "")
					buffer->select_range(start_iter, end_iter);
			}
		}
		//else select last entry
		else {
			start_iter = buffer->get_iter_at_line(nb_line-1) ;
			end_iter = start_iter ;
			tview->forward_display_line_end(end_iter) ;
			buffer->select_range(start_iter, end_iter);
		}
		tview->scroll_to_iter(start_iter,0.1) ;
	}//end is active
}

void Clipboard::import_text()
{
	if (active) {

		Gtk::TextIter start ;
		Gtk::TextIter end ;
		Glib::ustring value_head =  get_external_buffer()->getSelectedText(false) ;
		//> if smtg to import
		if ( ! value_head.empty() ) {
			//> do only if imported selection not blank or end of line
			if (value_head!= " " && value_head!= "\n" )
			{
				std::vector<Glib::ustring> v ;
				mini_parser('\n', value_head, &v) ;
				guint size = v.size() ;
				for (guint i =0; i<size; i++)
				{
					Glib::ustring value_tmp = v[i] ;
					bool exist = is_in_buffer(value_tmp) ;
					// just add if not in buffer if not already in
					if ( !exist && value_tmp!=" " && value_tmp!="" )
					{
						Glib::ustring value = "\n" + value_tmp ;
						//> if buffer empty don't add a \n
						if (buffer->get_line_count()==1
								&& buffer->get_slice(buffer->begin(), buffer->end())=="" )
							buffer->insert(buffer->end(), value_tmp) ;
						//>else add a \n before
						else
							buffer->insert(buffer->end(), value) ;
						end= buffer->end() ;
						tview->scroll_to_iter(end, 0.1) ;
						//actualise
						activate_up(true) ;
						activate_down(true) ;
						activate_clear(true) ;
						activate_sortAsc(true) ;
						activate_sortDesc(true) ;
					}
					//> if already in bip !
					else if (exist && size==1) {
							gdk_beep() ;
					}
				} //end for each word
				//> reorder after insertion
				std::vector<Glib::ustring> vec ;
				get_vector_from_buffer(vec) ;
				order_buffer(vec, reverse_sort) ;
				set_buffer_with_vector(vec) ;
				//> save to file system
				save() ;
			} //end value head ok
		} //end value head empty
		else {
			gdk_beep() ;
		}
	} //end is active
}

void Clipboard::export_text()
{
	if (active)
	{
		Gtk::TextIter start ;
		Gtk::TextIter end ;
		//> if selection exist
		if (buffer->get_selection_bounds(start, end))
		{
			Glib::ustring value =  buffer->get_slice(start,end) ;
			if (value!="\n" && value != "" && value!= " ")
				get_external_view()->onSelectionPasteEvent(value, false) ;
		}
		//> else emit bip
		else
			gdk_beep() ;
	} //end is active
}


void Clipboard::on_change_cursor(const Gtk::TextBuffer::iterator& it, const Glib::RefPtr<Gtk::TextBuffer::Mark >& mark)
{
	if (active)
		actualise_state() ;
}

void Clipboard::switch_file(AnnotationEditor* editor, bool switching)
{
	if (switching)
		close(false) ;
	set_editor(editor) ;
	active=true ;
	actualise_state() ;
}

bool Clipboard::save()
{
	if (!config || !active) {
		TRACE << "> saving clipboard not activated... " << std::endl ;
		return false ;
	}

	Glib::ustring savePath = config->get_CLIPBOARD_path() ;

	if (savePath.compare("")!=0  && savePath.compare("none")) {
		std::vector<Glib::ustring> vect ;
		get_vector_from_buffer(vect) ;
		int res = Explorer_utils::write_lines(savePath, vect, "w");
		if (res<0) {
			TRACE << "> saving clipboard KO... " << std::endl ;
			return false ;
		}
		else {
			TRACE << "> clipboard saved. " << std::endl ;
			return true ;
		}
	}
	else {
		TRACE << "> saving clipboard KO... " << std::endl ;
		return false;
	}
}

/*
bool Clipboard::autosave()
{
	if (!config || !active)
		return false ;

	Glib::ustring savePath = config->get_CLIPBOARD_path() ;
	if (savePath.compare("")!=0  && savePath.compare("none")) {
		Log::err() << "> saving clipboard... " << std::endl ;
		save(savePath) ;
		return true ;
	}
	else {
		Log::err() << "> saving clipboard KO... " << std::endl ;
		return false;
	}
}
*/

void Clipboard::load(Glib::ustring path)
{
	std::vector<Glib::ustring> vect ;
	int res = Explorer_utils::read_lines(path, &vect) ;
	if (res==1) {
		order_buffer(vect, reverse_sort) ;
		set_buffer_with_vector(vect) ;
		actualise_state() ;
	}
	else {
		Explorer_utils::print_trace("Clipboard::load :> error", 0) ;
	}
}

bool Clipboard::is_in_buffer(Glib::ustring txt)
{
	std::vector<Glib::ustring> vect ;

	Glib::ustring text = buffer->get_text(true) ;
	mini_parser('\n', text, &vect) ;
	bool found = false ;

	if (vect.size()==1 ) {
		if (text == txt)
			found=true ;
	}
	else {
		std::vector<Glib::ustring>::iterator it = vect.begin() ;
		while(it!=vect.end() && !found) {
			if (*it == txt)
				found=true ;
			it ++ ;
		}
	}
	return found ;
}

bool Clipboard::on_focus_in_event(GdkEventFocus* event)
{
	#ifndef __APPLE__
	if (!can_focus)
	{
		//> give away focus for first mapping
		if (window) {
			window->present() ;
		}
		//> disable focus
		Glib::RefPtr<Gdk::Window> win = get_window() ;
		if (win) {
			win->set_focus_on_map(false) ;
			win->set_accept_focus(false) ;
		}
	}
	return false;
	#endif
	
	return true;
}

bool Clipboard::on_focus_out_event(GdkEventFocus* event)
{
	/*
	external_buffer->delete_mark_by_name("Cursor") ;
	*/
	#ifndef __APPLE__
	return false ;
	#else
	return true;
	#endif
}

void Clipboard::clear()
{
	int close = Explorer_dialog::msg_dialog_question(_("Clear all clipboard ?"), this, true, "") ;

	if (close==Gtk::RESPONSE_YES) {
		buffer->set_text("") ;
		activate_up(false) ;
		activate_down(false) ;
		activate_suppress(false) ;
		activate_sortAsc(false) ;
		activate_sortDesc(false) ;
		activate_clear(false) ;
		//> save clipboard to filesystem
		save() ;
	}
}

void Clipboard::suppress()
{
	Gtk::TextIter start ;
	Gtk::TextIter end ;
	buffer->get_selection_bounds(start,end) ;
	if (end == buffer->end() && start != buffer->begin() ) {
		start.backward_line() ;
		start.forward_to_line_end() ;
	}
	end.forward_line() ;
	buffer->erase(start,end) ;

	Glib::ustring text = buffer->get_text(true) ;
	int size = text.size() ;
	if ( (text.c_str())[size-1] == '\n' )
		text = text.substr(0, size-2) ;

	actualise_state();

	//> save clipboard to filesystem
	save() ;
}

void Clipboard::actualise_state()
{
	Gtk::TextIter start ;
	Gtk::TextIter end ;

	//> check clipboard status
	Glib::ustring clip = buffer->get_text(true) ;
	if (clip != "") {
		activate_up(true) ;
		activate_down(true) ;
		activate_clear(true) ;
		activate_sortAsc(true) ;
		activate_sortDesc(true) ;
		if (buffer->get_selection_bounds(start,end)) {
			activate_export(true) ;
			activate_suppress(true) ;
		}
		else {
			activate_export(false) ;
			activate_suppress(false) ;
		}
	}
	else {
		activate_up(false) ;
		activate_down(false) ;
		activate_clear(false) ;
		activate_suppress(false) ;
		activate_export(false) ;
		activate_sortAsc(false) ;
		activate_sortDesc(false) ;
	}

	//> check external buffer status (is any view linked ?)
	if (active) {
		if (get_external_buffer()->get_selection_bounds(start,end)) {
			activate_import(true) ;
		}
		else {
			activate_import(false) ;
		}
	}
}

void Clipboard::order_buffer(std::vector<Glib::ustring>& vector, bool reverse)
{
	if (!reverse)
		std::sort(vector.begin(), vector.end(), Clipboard::compare_asc) ;
	else
		std::sort(vector.begin(), vector.end(), Clipboard::compare_desc) ;
}

bool Clipboard::compare_asc(Glib::ustring s1, Glib::ustring s2) {
	return s1 < s2 ;
}

bool Clipboard::compare_desc(Glib::ustring s1, Glib::ustring s2) {
	return s1 > s2 ;
}

void Clipboard::get_vector_from_buffer(std::vector<Glib::ustring>& res)
{
	res.clear() ;
	Glib::ustring text = buffer->get_text(true) ;
	mini_parser('\n', text, &res) ;
	if (text!="" && res.size()==1)  {
		res[0] = text ;
	}
}

void Clipboard::set_buffer_with_vector(std::vector<Glib::ustring> vect)
{
	Glib::ustring tmp = "" ;
	if (vect.size()!=0) {
		std::vector<Glib::ustring>::iterator it = vect.begin() ;
		std::vector<Glib::ustring>::iterator next ;
		while ( it!=vect.end()) {
			tmp.append(*it) ;
			it++ ;
			if (it != vect.end())
				tmp.append("\n") ;
		}
	}
	buffer->set_text(tmp) ;
	if (tmp != "") {
		activate_up(true) ;
		activate_down(true) ;
		activate_clear(true) ;
		activate_sortAsc(true) ;
		activate_sortDesc(true) ;
	}
}

void Clipboard::set_list_order(bool reverse)
{
	reverse_sort = reverse ;
	std::vector<Glib::ustring> v ;
	get_vector_from_buffer(v) ;
	order_buffer(v, reverse_sort) ;
	set_buffer_with_vector(v) ;
}

void Clipboard::display_clipboard()
{
	m_signalDisplayClipboard.emit() ;
}


//**************************************************************************************
//										GEOMETRY INTERFACE
//**************************************************************************************

void Clipboard::getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel)
{
	get_position(pos_x, pos_y) ;
	panel = -1 ;
	get_size(size_xx, size_yy) ;
}

void Clipboard::setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel)
{
	if (pos_x>0 && pos_y>0)
		move(pos_x, pos_y) ;
	if (size_xx>0 && size_yy>0)
		resize(size_xx, size_yy) ;
}

Glib::ustring Clipboard::getWindowTagType()
{
	return SETTINGS_CLIPBOARD_NAME ;
}

int Clipboard::loadGeoAndDisplay(bool rundlg)
{
	loadPos() ;
	show() ;
	return 1 ;
}

void Clipboard::saveGeoAndHide()
{
	if (is_visible())
		savePos() ;
	hide() ;
}

void Clipboard::getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) {}

} //namespace

