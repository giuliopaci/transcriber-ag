/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  TransAGVideo				*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "FBModelColumns.h"
#include "Common/util/FormatTime.h"

namespace tag {

FBModelColumns::~FBModelColumns()
{
}

void FBModelColumns::fill_row(Gtk::TreeModel::Row* row, const Glib::RefPtr<Gdk::Pixbuf>& image, float time)
{
	FBModelColumns m ;
	(*row)[m.image] = image ;
	(*row)[m.frameTime] = time ;
	(*row)[m.displayTime] = FormatTime(time, false, true) ;
}

} //namespace
