/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		SectionRenderer.h
 */

#ifndef _HAVE_SECTION_RENDERER_H
#define _HAVE_SECTION_RENDERER_H

#include "Editors/AnnotationEditor/renderers/AnnotationRenderer.h"

namespace tag {

/**
* @class 		SectionRenderer
* @ingroup 		AnnotationEditor
*
*  Creates section annotation representation in text editor.
*/
class	SectionRenderer : public AnnotationRenderer
{
	public:
		/**
		 * Constructor
		 * @param view		Pointer on the AnnotationView parent
		 */
		SectionRenderer (AnnotationView* view);
		virtual ~SectionRenderer() {}

		/**
		 * Configures all mechanisms and display values specific to
		 * Event representation.
		 */
		void configure() ;

		/* Inherited methods */
		virtual void render(const string& id, bool r2l, bool with_contents=true) ;
		virtual void render_end(const string& id, bool r2l) {} ;

		virtual void deleteElement(const string& id);
		virtual void updateElement(const string& id, bool r2l);

		AnnotationMenu* getContextualMenu(const string& id, bool edition) {
			return (edition ? contextMenu : contextMenu_lock) ;
		}

		virtual AnnotationPropertiesDialog* newPropertiesDialog(const string& id) ;
		virtual const string& getTagName(const string& id, const string& type) {
			return m_name ;
		}

	private:
		void setLabelAndTagname(const string & id, string& label, string& tagname);
	
		/**
		 * Configure the menu associated to this renderer
		 */
		void configureMenu() ;
	};
} /* namespace transcriber */

#endif /* _HAVE_SECTION_RENDERER_H */
