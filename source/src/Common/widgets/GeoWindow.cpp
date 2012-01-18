/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/


#include "GeoWindow.h"

namespace tag {

GeoWindow::GeoWindow()
{
}

GeoWindow::~GeoWindow()
{
}

void GeoWindow::loadPos()
{
	Settings* settings = Settings::getInstance() ;
	if (!settings)
		return ;

	int w, h, posx, posy, panel ;
	Glib::ustring wincode = getWindowTagType() ;

	// Load pos
	settings->get_datas(wincode, w, h, posx, posy, panel) ;

	// Apply
	// if no values found, use default ones
	if (w==-1 && h==-1 && posx==-1 && posy==-1)
		getDefaultGeo(w, h, posx, posy, panel) ;
	setGeo(w, h, posx, posy, panel) ;
}

void GeoWindow::savePos()
{
	Settings* settings = Settings::getInstance() ;
	if (!settings)
		return ;

	int panel = -1 ;
	int w = -1 ;
	int h = -1 ;
	int posx =-1 ;
	int posy = -1 ;

	// Get data
	Glib::ustring wincode = getWindowTagType() ;
	getGeo(w, h, posx, posy, panel) ;

	// Save data
	settings->set_datas(wincode, w, h, posx, posy, panel) ;
	settings->save() ;
}

} //namespace
