/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 	BackgroundRenderer.h
 */

#ifndef _HAVE_BACKGROUND_RENDERER_H
#define _HAVE_BACKGROUND_RENDERER_H


#include "Editors/AnnotationEditor/renderers/AnnotationRenderer.h"
namespace tag {
/**
* @class 		BackgroundRenderer
* @ingroup 		AnnotationEditor
*
*  Creates background annotation representation in text editor.
*/
class	BackgroundRenderer : public AnnotationRenderer
{
	public:
		/**
		 * Constructor
		 * @param view		Pointer on the AnnotationView parent
		 */
		BackgroundRenderer (AnnotationView* view);
		virtual ~BackgroundRenderer() {}

		/**
		 * Configures all mechanisms and display values specific to
		 * Background representation.
		 */
		void configure() ;

		/* Inherited methods */
		virtual void render(const string& id, bool r2l, bool with_contents=true) ;
		virtual void render_end(const string& id, bool r2l) ;
		virtual bool canRender(const string & id) ;
		virtual Glib::ustring getSegmentText(const string & id) ;
		virtual Gtk::Box* getTooltipBox(const string& id, bool is_start) ;
		virtual void deleteElement(const string& id);
		virtual void updateElement(const string& id, bool r2l);

		AnnotationMenu* getContextualMenu(const string& id, bool edition) {
			return (edition ? contextMenu : contextMenu_lock) ;
		}

		const string& getTagName(const string& id, const string& type) {
			return m_name ;
		}

	private:
		virtual Glib::ustring formatLabelStart(Glib::ustring label, bool isInstantaneous) { return label ; }
		virtual Glib::ustring formatLabelEnd(Glib::ustring label)  { return label ; }
		void configureMenu() ;

};


} /* namespace tag */

#endif /* _HAVE_BACKGROUND_RENDERER_H */
