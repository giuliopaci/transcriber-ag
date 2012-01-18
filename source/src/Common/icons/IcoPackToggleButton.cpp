/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "IcoPackToggleButton.h"
#include <iostream>
#include "Common/util/Log.h"

namespace tag {

IcoPackToggleButton::IcoPackToggleButton()
{
	set_use_stock(true) ;
}

IcoPackToggleButton::~IcoPackToggleButton()
{
}

int IcoPackToggleButton::set_icon(const Glib::ustring& icon, const Glib::ustring& label, int size, const Glib::ustring& _tooltip)
{
	if (_tooltip != "")
		tooltip.set_tip(*this, _tooltip) ;

	int res = image.set_image(icon, size);
		set_image(image) ;
	if (!label.empty())
		set_label(label) ;
		return 1 ;
	}

int IcoPackToggleButton::set_icon(const Gtk::StockID& stock, const Glib::ustring& label, int size, const Glib::ustring& _tooltip)
{
	if (_tooltip != "")
		tooltip.set_tip(*this, _tooltip) ;

	int res = image.set_image(stock, size);
	set_image(image) ;
	if (!label.empty())
		set_label(label) ;
	return 1 ;
}

void IcoPackToggleButton::set_tip(const Glib::ustring& tip)
{
	tooltip.set_tip(*this, tip) ;
}

} //namespace
