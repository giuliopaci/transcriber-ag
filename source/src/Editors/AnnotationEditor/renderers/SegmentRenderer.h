/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		SegmentRenderer.h
 */

#ifndef _HAVE_SEGMENT_RENDERER_H
#define _HAVE_SEGMENT_RENDERER_H

#include "Editors/AnnotationEditor/renderers/AnnotationRenderer.h"

namespace tag {

/**
* @class 		SegmentRenderer
* @ingroup 		AnnotationEditor
*
*  Creates segment annotation representation in text editor.
*/
class	SegmentRenderer : public AnnotationRenderer
{
	public:
		/**
		 * Constructor
		 * @param view		Pointer on the AnnotationView parent
		 */
		SegmentRenderer (AnnotationView* view);

		/**
		 * Configures all mechanisms and display values specific to
		 * Segment representation.
		 */
		void configure() ;

		/* Inherited method */
		virtual void render(const string& id, bool r2l, bool with_contents=true) ;
		void render_end(const string& id, bool r2l) {}
		virtual void deleteElement(const string& id);
		virtual void updateElement(const string& id, bool r2l);

		AnnotationMenu* getContextualMenu(const string& id, bool edition) {
			return (edition ? contextMenu : contextMenu_lock) ;
		}

		virtual AnnotationPropertiesDialog* newPropertiesDialog(const string& id) ;

		const string& getTagName(const string& id, const string& type) {
			return m_name ;
		}

	private:
		std::string m_label;
		std::string m_nolabel;
		bool m_showLabel;  /* show segment label true/false */
		void configureMenu() ;
};

} //namespace

#endif /* _HAVE_SEGMENT_RENDERER_H */
