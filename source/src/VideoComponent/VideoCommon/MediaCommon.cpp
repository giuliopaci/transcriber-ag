/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  TransAGVideo				*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/** @file */

#include "MediaCommon.h"
#include "Common/icons/Icons.h"

namespace tag {

// --------------------------
// --- STATIC UTIL METHOD ---
// --------------------------

// --- GetPixbufFromFrame ---
Glib::RefPtr<Gdk::Pixbuf> MediaCommon::getPixbufFromFrame(float time, IODevice* device, int width, int height)
{
	MediumFrame* medium = device->m_get_rgb_frame(time) ;
	Glib::RefPtr<Gdk::Pixbuf> buf = getPixbufFromFrame(medium) ;
	buf = buf->scale_simple(width, height, Gdk::INTERP_BILINEAR) ;
	return buf ;
}

// --- GetPixbufFromFrame ---
Glib::RefPtr<Gdk::Pixbuf> MediaCommon::getPixbufFromFrame(MediumFrame* medium)
{
	Glib::RefPtr<Gdk::Pixbuf> buf ;

	if (!medium) {
		bool ok ;
		buf = Icons::create_pixbuf(ICO_FRAMEBROWSER_FRAMEERROR, 12, ok) ;
	}
	else {
		bool	has_alpha		= false;
		int		row_stride		= medium->v_linesize[0] ;
		int		bits_per_sample	= 8 ;
		guint8* in_data			= reinterpret_cast<guint8*>(medium->v_samples[0]);

		int base_width = medium->v_width ;
		int base_height = medium->v_height ;

		buf = Gdk::Pixbuf::create_from_data(in_data, Gdk::COLORSPACE_RGB, has_alpha, bits_per_sample, base_width, base_height, row_stride) ;
	}
	return buf ;
}

// --- NormalizeDimensions ---
void MediaCommon::normalizeDimensions(int& v_width, int& v_height, IODevice* v_device)
{
	if (!v_device)
		return ;

	double aspect_ratio = v_device->m_info()->video_aspect_ratio==0 ;

	if (aspect_ratio==0)
		return ;

	if (v_width > v_height)
		v_width	= v_height * aspect_ratio ;
	else
		v_height = v_width / aspect_ratio ;
}


}
