/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "IcoPackMenuToolButton.h"

namespace tag {

IcoPackMenuToolButton::IcoPackMenuToolButton()
{
}

IcoPackMenuToolButton::~IcoPackMenuToolButton()
{
}


int IcoPackMenuToolButton::set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& _tooltip, const Glib::ustring& _arrow_tooltip)
{
	if (_tooltip != "")
		set_tooltip(tooltip, _tooltip, "") ;

	int res = image.set_image(icon, size) ;
	if (res>0)
		set_icon_widget(image) ;
	set_arrow_tooltip(arrow_tooltip, _arrow_tooltip, "") ;

	return res ;
}

int IcoPackMenuToolButton::set_icon(const Gtk::StockID& stock_id, const Glib::ustring& label, int size, const Glib::ustring& _tooltip, const Glib::ustring& _arrow_tooltip)
{
	if (_tooltip != "")
		set_tooltip(tooltip, _tooltip, "") ;

	int res = image.set_image(stock_id, size) ;
	if (res>0)
	set_icon_widget(image) ;
	set_arrow_tooltip(arrow_tooltip, _arrow_tooltip, "") ;

	return res ;
}

int IcoPackMenuToolButton::change_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& _tooltip)
{
	if (_tooltip != "")
		set_tooltip(tooltip, _tooltip, "") ;

	int res = image.set_image(icon, size) ;
	if (res>0)
	set_icon_widget(image) ;
	
	return res ;
}

void IcoPackMenuToolButton::set_tip(const Glib::ustring& tip)
{
	tooltip.set_tip(*this, tip) ;
}

//******************************************************************************
//************************************ Menu behaviouR   ************************
//******************************************************************************

Gtk::RadioMenuItem* IcoPackMenuToolButton::appendItem(std::string name, std::string display)
{
	Gtk::RadioMenuItem* res = NULL ;

	//name empty ? exit
	if (name.empty())
		return res ;

	//already exists ? exit
	std::map<std::string, Gtk::RadioMenuItem*>::iterator it = items.find(name) ;
	if (it != items.end())
		return  res ;

	//create
	items[name] = new Gtk::RadioMenuItem(group, display, false) ;
	res = items[name] ;
	menu.append(*res) ;

	return res ;
}

void IcoPackMenuToolButton::activateItem(std::string name)
{
	std::map<std::string, Gtk::RadioMenuItem*>::iterator it = items.find(name) ;
	if (it!=items.end() && it->second)
		menu.activate_item( *(it->second), false) ;
}

void IcoPackMenuToolButton::selectItem(std::string name)
{
	std::map<std::string, Gtk::RadioMenuItem*>::iterator it = items.find(name) ;
	if (it!=items.end() && it->second)
		menu.select_item( *(it->second) ) ;
}

void IcoPackMenuToolButton::setSensitiveItem(std::string name, bool sensitive)
{
	// global activation
	std::map<std::string, Gtk::RadioMenuItem*>::iterator it ;
	if (name.empty())
	{
		for (it = items.begin(); it!=items.end(); it++) {
			if (it->second)
				it->second->set_sensitive(sensitive) ;
		}
	}
	// specific activation
	else
	{
		it = items.find(name) ;
		if (it!=items.end())
			it->second->set_sensitive(sensitive) ;
	}
}

} //namespace
