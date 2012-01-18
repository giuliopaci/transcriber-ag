/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		UnknownQualifierRenderer.h
 */

#ifndef _HAVE_UNKNOWNQUAL_RENDERER_H
#define _HAVE_UNKNOWNQUAL_RENDERER_H

#include "Editors/AnnotationEditor/renderers/AnnotationRenderer.h"
namespace tag {

/**
* @class 		UnknownQualifierRenderer
* @ingroup 		AnnotationEditor
*
*  Creates unknown qualifier annotation representation in text editor.
*/
class	UnknownQualifierRenderer : public AnnotationRenderer
{
	public:
		/**
		 * Constructor
		 * @param view		Pointer on the AnnotationView parent
		 */
		UnknownQualifierRenderer (AnnotationView* view);

		/**
		 * Destructor
		 */
		virtual ~UnknownQualifierRenderer() {}

		/**
		 * Configures all mechanisms and display values specific to
		 * Event representation.
		 */
		void configure() ;

		/* Inherited method */
		virtual void render(const string& id, bool r2l, bool with_contents=true) ;
		virtual void render_end(const string& id, bool r2l) ;
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
		virtual void formatLabelStart(Glib::ustring& type, bool isInstantaneous) ;
		virtual void formatLabelEnd(Glib::ustring& type) ;
		void configureMenu() ;
};

} /* namespace transcriber */

#endif /* _HAVE_UNKNOWNQUAL_RENDERER_H */
