/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 		TextRenderer.h
 */

#ifndef _HAVE_UNIT_TEXT_RENDERER_H
#define _HAVE_UNIT_TEXT_RENDERER_H

#include "Editors/AnnotationEditor/renderers/AnnotationRenderer.h"

namespace tag {

/**
* @class 		UnitRenderer
* @ingroup 		AnnotationEditor
*
*  Creates segment annotation representation in text editor.
*/
class	UnitRenderer : public AnnotationRenderer
{
	public:
		/**
		 * Constructor
		 * @param view		Pointer on the AnnotationView parent
		 */
		UnitRenderer (AnnotationView* view);

		~UnitRenderer() ;

		/**
		 * Configures all mechanisms and display values specific to
		 * Segment representation.
		 */
		void configure() ;

		/* Inherited & overloaded method */
		virtual void render(const string& id, bool r2l, bool with_contents=true) ;
		void render_end(const string& id, bool r2l) {};

		virtual void deleteElement(const string& id);
		virtual void updateElement(const string& id, bool r2l);
		virtual void formatLabelStart(Glib::ustring& label, bool instantaneous) ;
		Gtk::Box* getTooltipBox(const string& id, bool is_start) ;
		virtual const string&  getTagName(const string& id, const string& type) ;
		virtual AnnotationMenu* getContextualMenu(const string& id, bool edition) ;
		virtual AnnotationPropertiesDialog* newPropertiesDialog(const string& id) ;
		virtual AnnotationMenu* duplicateMenu() ;

	private:

		UnitMenu* m_simpleMenu ;

		std::string m_altname ;

		std::string m_label;
		std::string m_nolabel;
		std::string m_textType;
;
		bool m_showLabel;  /* show segment label true/false */
		float m_threshold;	// high confidence threshold

		bool configureRendererVisibility(const string& id, bool& anchored) ;

		void renderUnitText(const string & id, bool r2l, bool with_contents) ;
		void renderUnitEvent(const string& id, bool r2l) ;
		void renderUnitUnknown(const string& id, bool r2l) ;
		void configureMenu() ;
};

} //namespace

#endif /* _HAVE_UNIT_TEXT_RENDERER_H */
