/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/**
 *  @file 	AnnotationViewTooltip.h
 */

#ifndef ANNOTATIONVIEWTOOLTIP_H_
#define ANNOTATIONVIEWTOOLTIP_H_

#include "Common/widgets/TooltipTT.h"

namespace tag {

class AnnotationView ;
/**
*  @class 		AnnotationViewTooltip
*  @ingroup 	AnnotationEditor
*
*  Specialization of the TooltipTT class.\n
*  Used for displaying information over some tagged elements.\n\n
*
*/
class AnnotationViewTooltip : public TooltipTT
{
	public:
		/**
		 * Constructor
		 * @param parent	Reference on the AnnotationView parent
		 */
		AnnotationViewTooltip(AnnotationView& parent);
		virtual ~AnnotationViewTooltip();

	private:
		AnnotationView& parent ;
		void reset_data() {} ;
		bool display(GdkEventMotion event, Gtk::Widget* win) ;
		bool prepare_tooltip(std::string tagtype, const std::string& id) ;

		/* USED FOR DEBUG OR MONITORING - display all tags of tagged buffer element */
		bool prepare_4_debug(const Gtk::TextIter& iter) ;
};

}

#endif /* ANNOTATIONVIEWTOOLTIP_H_ */
