/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 	EventRenderer.h
 */

#ifndef _HAVE_EVENT_RENDERER_H
#define _HAVE_EVENT_RENDERER_H

#include "Editors/AnnotationEditor/renderers/AnnotationRenderer.h"
namespace tag {

/**
* @class 		EventRenderer
* @ingroup 		AnnotationEditor
*
*  Creates event annotation representation in text editor.
*/
class	EventRenderer : public AnnotationRenderer
{
	public:
		/**
		 * Constructor
		 * @param view		Pointer on the AnnotationView parent
		 */
		EventRenderer (AnnotationView* view);

		/**
		 * Destructor
		 */
		virtual ~EventRenderer() {}

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
		virtual AnnotationMenu* duplicateMenu() ;

		AnnotationMenu* getContextualMenu(const string& id, bool edition) {
			return (edition ? contextMenu : contextMenu_lock) ;
		}
		/**
		 * allocates and returns a new annotation properties edition dialog (that should be afterward deleted)
		 * @param id 	id / "" for new id
		 * @return 		annotation properties edition dialog
		 */
		virtual AnnotationPropertiesDialog* newPropertiesDialog(const string& id) ;

		/**
		 * Return tag name used by renderer
		 * @param id		Annotation id
		 * @param type		Annotation type
		 * @return			Tag name or "" if error
		 */
		const string& getTagName(const string& id, const string& type) {
			return m_name ;
		}

	private:
		bool checkInstantaneous(const string& id) ;
		virtual void formatLabelStart(Glib::ustring& label, bool isInstantaneous) ;
		virtual void formatLabelEnd(Glib::ustring& label) ;
		void configureMenu() ;
};

} /* namespace transcriber */

#endif /* _HAVE_EVENT_RENDERER_H */
