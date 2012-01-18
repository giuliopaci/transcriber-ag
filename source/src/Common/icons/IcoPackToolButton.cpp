#include "IcoPackToolButton.h"
#include <iostream>
#include "Common/util/Log.h"

namespace tag {

IcoPackToolButton::IcoPackToolButton() 
{
}

IcoPackToolButton::~IcoPackToolButton()
{
}

int IcoPackToolButton::set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& _tooltip)
{
	if (_tooltip != "")
		tooltip.set_tip(*this, _tooltip) ;

	if (icon =="")
	{
		set_label(label) ;
		return 1 ;
	}
	else
	{
		int res = image.set_image(icon,size);
		if (res>0)
			set_icon_widget(image) ;
			if (label!="")
				set_label("  " + label) ;
			return res ;
		}
		}

int IcoPackToolButton::set_icon(const Gtk::StockID& stock, const Glib::ustring& label, int size, const Glib::ustring& _tooltip)
{
	if (_tooltip != "")
		tooltip.set_tip(*this, _tooltip) ;

	int res = image.set_image(stock,size);
	if (res>0)
		set_icon_widget(image) ;
	if (label!="")
		set_label("  " + label) ;
	return res ;
	}

int IcoPackToolButton::change_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& _tooltip)
{
	if (_tooltip != "")
		tooltip.set_tip(*this, _tooltip) ;

	int res = image.set_image(icon,size);
	if (res>0)
		set_icon_widget(image) ;
	if (label!="")
		set_label("  " + label) ;
	return res ;
}

int IcoPackToolButton::change_icon(const Gtk::StockID& stock, const Glib::ustring& label, int size, const Glib::ustring& _tooltip)
{
	if (_tooltip != "")
		tooltip.set_tip(*this, _tooltip) ;

	int res = image.set_image(stock,size);
	if (res>0)
		set_icon_widget(image) ;
	if (label!="")
		set_label("  " + label) ;
	return res ;
}

void IcoPackToolButton::set_tip(const Glib::ustring& tip)
{
	tooltip.set_tip(*this, tip) ;
}

} //namespace
