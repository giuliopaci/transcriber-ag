/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 	EntityRenderer.h
 */

#ifndef _HAVE_ENTITY_RENDERER_H
#define _HAVE_ENTITY_RENDERER_H

#include "Editors/AnnotationEditor/renderers/AnnotationRenderer.h"
namespace tag {

/**
* @class 		EntityRenderer
* @ingroup 		AnnotationEditor
*
*  Creates entity annotation representation in text editor.
*/
class	EntityRenderer : public AnnotationRenderer
{
	public:
		/**
		 * Constructor
		 * @param view		Pointer on parent AnnotationView
		 */
		EntityRenderer (AnnotationView* view);

		/**
		 * Destructor
		 */
		virtual ~EntityRenderer() {}

		/**
		 * Configures all mechanisms and display values specific to
		 * Entity representation.
		 */
		void configure() ;

		/* Inherited method */
		virtual void render(const string& id, bool r2l, bool with_contents=true) ;
		virtual void render_end(const string& id, bool r2l) ;
		virtual void deleteElement(const string& id);
		virtual void updateElement(const string& id, bool r2l);
		virtual const string& getTagName(const string& id, const string& type) ;
		virtual AnnotationMenu* duplicateMenu() ;

		AnnotationMenu* getContextualMenu(const string& id, bool edition) {
			return (edition ? contextMenu : contextMenu_lock) ;
		}

		/**
		 * Allocates and returns a new annotation properties edition dialog (that should be afterward deleted)
		 * @param id 		id / "" for new id
		 * @return 			Annotation properties edition dialog
		 */
		virtual AnnotationPropertiesDialog* newPropertiesDialog(const string& id) ;

		/**
		 * Defines whether qualifiers label corresponding to named entities are displayed with a
		 * background color.
		 * @param use_bg		True for displaying tag background, False otherwise
		 */
		void set_use_bg(bool use_bg) ;

	private:
		bool checkInstantaneous(const string& id) ;
		virtual void formatLabelStart(Glib::ustring& type, bool isInstantaneous) ;
		virtual void formatLabelEnd(Glib::ustring& type) ;
		void configureMenu() ;

		string m_tmpname;
};

} /* namespace tag */

#endif /* _HAVE_ENTITY_RENDERER_H */
