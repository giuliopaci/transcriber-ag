/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */

/**
 *  @file 	AnnotationRenderer.h
 */

#ifndef _HAVE_ANNOTATION_RENDERER
#define _HAVE_ANNOTATION_RENDERER

#include "Editors/AnnotationEditor/AnnotationView.h"
#include "Editors/AnnotationEditor/menus/annotation_menus.h"

#include "Editors/AnnotationEditor/dialogs/AnnotationPropertiesDialog.h"

namespace tag {

/**
* @class 		AnnotationRenderer
* @ingroup		AnnotationEditor
*
*   Base class for all annotation renderers.\n
*   An annotation renderer defines the text presentation of an annotation type
*   (colors, fonts, behavior properties).
*/
class AnnotationRenderer
{
	public:
		/**
		 * Constructor
		 * @param view		Pointer on the view where the annotation will be displayed
		 * @param name		Renderer name
		 */
		AnnotationRenderer(AnnotationView* view, const string& name);
		virtual ~AnnotationRenderer() ;

		/**
		 * @return renderer name
		 */
		const string& getName() const { return m_name; }

		/**
		 * Displays the start presentation element corresponding to the given id
		 * @param id		Annotation id
		 * @param r2l		in r2l mode
		 * @param with_contents	if true, render annotation contents (default), else only annotation "header"
		 */
		virtual void render(const string& id, bool r2l, bool with_contents=true) = 0;

		/**
		 * Displays the end presentation element corresponding to the given id
		 * @param id
		 * @param r2l			True for right-to-left languages, false otherwise
		 */
		virtual void render_end(const string& id, bool r2l) = 0;

		/**
		 * Gets the text associated to given segment
		 * @param id		Annotation id
		 * @return			The corresponding text
		 */
		virtual Glib::ustring getSegmentText(const string & id) ;

		/**
		 * Enables tooltip display on renderer
		 * @param on		True for enabling, False otherwise
		 */
		void setUseTooltip(bool on) { m_useTooltip = on ;}

		/**
		 * Accessor to the tooltip mode
		 * @return			The tooltip mode status
		 */
		bool getUseTooltip() { return m_useTooltip ;}

		/**
		 * Accessor to the tooltip content.\n
		 * Embeds all widgets that has to be displayed in the tooltip
		 * @param id			Annotation id
		 * @param is_start		True for starting element tooltip
		 * @return				The computed tooltip content
		 */
		virtual Gtk::Box* getTooltipBox(const string& id, bool is_start) { return NULL ; }

		/**
		 * update the given rendered element (both start and end)
		 * @param id			Annotation id
		 * @param r2l			True for right-to-left languages, false otherwise
		 */
		virtual void updateElement(const string& id, bool r2l) = 0;

		/**
		 * Delete the given rendered element  (both start and end)
		 * @param id			Annotation id
		 */
		virtual void deleteElement(const string& id) = 0;

		/**
		 * Returns the tag associated to the given id
		 * @param id		Annotation id
		 * @param type		Annotation type
		 * @return			Corresponding tagname
		 */
		virtual const string& getTagName(const string& id, const string& type) = 0 ;

		/**
		 * Accessor to the contextual menu
		 * @param id			Annotation on which the id has been launched (empty if unused)
		 * @param edit_mode		True for edition mode menu, false for read-only menu
		 * @return				The corresponding menu (can be NULL if was not configured)
		 */
		virtual AnnotationMenu* getContextualMenu(const string& id, bool edit_mode) = 0 ;

		/**
		 * Checks if the given annotation can be rendered
		 * @param id		Annotation id
		 * @return			True if render is allowed, False otherwise
		 */
		virtual bool canRender(const string& id) { return true; }

		/**
		 * return true if last rendered annotation properties were empty
		 */
		 bool emptyProperties() { return m_emptyProperties; }

		/**
		 * Hide the renderer associated tags if the hidden mode is enabled.
		 * @return		False if the hidden tag isn't allowed, True when performed.
		 */
		virtual bool hideTag(bool hide=true);

		/**
		 * rendering may depend on if current element is first child of its parent or not
		 * @param on		True if is first child, False otherwise
		 */
		void setFirstChild(bool on) { m_isFirstChild = on ;}

		/**
		 * Returns a newly created menu similar to the contextual menu.
		 * @return				Newly created menu, or NULL.
		 * @remarks				The signals are not connected, will have to be done by the caller
		 * @note				This method is mainly usefull for adding the same option than
		 * 						contextual menu in another menu (Gtk allows only 1 parent)
		 */
		virtual AnnotationMenu* duplicateMenu() { return NULL  ; }

		/**
		 *	Allocates and returns a new annotation properties edition dialog (that should be afterward deleted)
		 * @param id	Annotation id
		 * @ return		Pointer on new annotation dialog or NULL if error
		 */
		virtual AnnotationPropertiesDialog* newPropertiesDialog(const string& id) { return NULL; }

		/**
		 * Formats the given label for start renderer
		 * @param[in,out] 	label				Label to format
		 * @param 			instantaneous		True if the label will be applied on an instantaneous element
		 */
		virtual void formatLabelStart(Glib::ustring& label, bool instantaneous) { }

	protected:
		/**
		* Gets the rendering configuration item corresponding to the given tag
		* @param tagname 		Tag name
		* @param item  			Configuration item (bg/fg/font/weight)
		* @return item value	The corresponding value
		*/
		string getLook(const string& tagname, const string& item) ;

		/**
		 * Configures the renderer displayed tag.
		 * @param tagname		Tag name
		 * @param tagclass		Tag class
		 * @param undoable		True if the renderer can be impacted by undo/redo mechanisms
		 * @param is_end		True for getting the end tag, False otherwise
		 * @param fg			Foreground color
		 * @param bg			Background color
		 * @param style			Font style
		 * @param weight		Font weight
		 * @param use_bg		False to inhibate background style.\n
		 * 						Mostly true except for annotation type whose background display
		 * 						depends on configuration values.
		 * @return
		 */
		Glib::RefPtr<Gtk::TextTag> configure(string tagname="", string tagclass="",
												bool undoable=true, bool is_end=false,
												Glib::ustring  fg="", Glib::ustring  bg="",
												Glib::ustring style="", Glib::ustring  weight="",
												bool use_bg=true) ;

		/**
		 * Configures the renderer displayed tag.
		 * @param tagname		Tag name
		 * @param tagclass		Tag class
		 * @param undoable		True if the renderer can be impacted by undo/redo mechanisms
		 * @param is_end		True for getting the end tag, False otherwise
		 * @param fg			Foreground color
		 * @param bg			Background color
		 * @param style			Font style
		 * @param weight		Font weight
		 * @param use_bg		False to inhibate background style.\n
		 * 						Mostly true except for annotation type whose background display
		 * 						depends on configuration values.
		 * @param tagFlags		Specify the tag flags for rendering the element
		 * @return
		 */
		Glib::RefPtr<Gtk::TextTag> configure(string tagname, string tagclass, bool undoable, bool is_end,
											Glib::ustring fg, Glib::ustring bg,
											Glib::ustring style, Glib::ustring weight,
											bool use_bg, unsigned long& tagFlags) ;

		/**
		 * Formats the given label for end renderer
		 * @param[in,out] 	label		Label to format
		 */
		virtual void formatLabelEnd(Glib::ustring& label) { }

		/**
		 * Specific update action
		 * @param id			Annotation id
		 */
		virtual void specificUpdate(const string& id) { }

		/**
		 * Accessor to the renderer visibility
		 * @param id		Element id
		 * @return			True or False
		 */
		bool getInvisibleTag(const string& id) ;

		/**
		 * Specifies whether a tag should be used for rendering
		 * @param id			Element id
		 * @param invisible		True for rendering element without tagged label, false otherwise
		 */
		void setInvisibleTag(const string& id, bool invisible) ;

		/**
		 * init layout for given layout class
		 * @param layoutClass	applicable layout class
		 */
		void initLayoutRules(const string& layoutClass);

		/**
		 * layout (qualifier= item according to layout rules defined in conventions
		 * @param		id			Item id (qualifier or event-type unit
		 * @param 		type		Item type
		 * @param[out] 	label 		Buffer in which to store layout results
		 * @param 		at_start	If true, layout start label, else layout end label
		 */
		void layoutItem(const string& id, const string& type, Glib::ustring& label, bool at_start=true);


		/**
		 * Accessor to the buffer in use
		 * @return		Pointer on the AnnotationBuffer embedded in the AnnotationView in use
		 */
		Glib::RefPtr<AnnotationBuffer> getBuffer() { return m_view->getBuffer(); }

		AnnotationView* m_view;							/**< Pointer on the related view */
		DataModel& m_dataModel;		 					/**< Reference on the related model */
		string m_name;				 					/**< Renderer name */
		unsigned long m_tagFlags;						/**< Flags used by corresponding tags */

		bool m_emptyProperties;							/**< true if empty properties for rendered annotation */
		bool m_useTooltip ;			 				/**< Tooltip mode value */
		bool m_isFirstChild;							/**< current element is first child of its parent or not */
		vector< Glib::RefPtr<Gtk::TextTag> > m_tags ;	/**< Vector with all associated tags */

		/**< Keeps whether an id renderer is labelled (tagged label) or not **/

		std::map<std::string, std::string> m_layout;	/**< layout rules (qualifiers */
		std::set<std::string> 	m_invisibleTagSet ;
		AnnotationMenu*	contextMenu ;
		AnnotationMenu*	contextMenu_lock ;
	};

} /* namespace tag */

#endif /* _HAVE_ANNOTATION_RENDERER  */
