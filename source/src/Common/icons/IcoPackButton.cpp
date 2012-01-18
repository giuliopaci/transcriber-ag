/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "IcoPackButton.h"
#include <iostream>
#include "Common/util/Log.h"

namespace tag {

IcoPackButton::IcoPackButton()
{
	set_use_underline() ;
}

IcoPackButton::~IcoPackButton()
{
}

int IcoPackButton::set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& _tooltip)
{
	if (_tooltip != "")
		tooltip.set_tip(*this, _tooltip) ;

	if (icon =="")
	{
		set_label(label) ;
		set_use_stock(false) ;
		return 1 ;
	}
	else
	{
		int res = image.set_image(icon, size);
		if (res>0)
			set_image(image) ;
			if (label!="")
				set_label("  " + label) ;
		return res ;
		}
	}

int IcoPackButton::set_icon(const Gtk::StockID& stock_id, const Glib::ustring& label, int size, const Glib::ustring& _tooltip)
{
	if (_tooltip != "")
		tooltip.set_tip(*this, _tooltip) ;

	int res = image.set_image(stock_id, size) ;
	if (res>0)
		set_image(image) ;
	if (label!="")
		set_label("  " + label) ;
	return res ;
	}

int IcoPackButton::change_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& _tooltip)
{
	if (_tooltip != "")
		tooltip.set_tip(*this, _tooltip) ;

	int res = image.set_image(icon, size) ;
	if (res>0)
		set_image(image) ;
		if (label!="")
			set_label("  " + label) ;
	return res ;
}

void IcoPackButton::set_tip(const Glib::ustring& tip)
{
	tooltip.set_tip(*this, tip) ;
}

} //namespace
