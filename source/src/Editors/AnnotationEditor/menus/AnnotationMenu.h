/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/**
 *  @file 	AnnotationMenu.h
 */

#ifndef _HAVE_ANNOTATION_MENU
#define _HAVE_ANNOTATION_MENU


#include <map>
#include <queue>
#include <string>
#include <gtkmm.h>
#include <gtkmm/menu.h>
#include <gtkmm/textiter.h>
#include <gdk/gdk.h>
#include <gtkmm/image.h>
#include <gtkmm/menu_elems.h>
#include "Common/globals.h"


namespace tag {

/**
 * @class 		AnnotationMenu
 * @ingroup		AnnotationEditor
 *
 * Interface class for annotation type-specific popup menus
 *
 */
class AnnotationMenu : public Gtk::Menu
{
	public:
		/**
		 * Constructor
		 * @param type				Annotation type
		 * @param add_delete		True for adding delete entry
		 * @param editable			True is menu is editable, false otherwise
		 */
		AnnotationMenu(std::string type = _("annotation"), bool add_delete=true, bool editable=true);

		/**
		 * Presents the popup menu.
		 * @param textIter			Text position
		 * @param x					x position
		 * @param y					y position
		 * @param event_time		Event time
		 * @param can_create		True for allowing annotation creation
		 * @param can_edit			True for edition mode, False otherwise
		 * @param can_delete		True for allowing annotation deletion
		 * @param can_unanchor		True for allowing timecoded element deletion, false otherwise
		 * @param start				Signal selection start if exists (-1 otherwise)
		 * @param end				Signal selection end if exists (-1 otherwise)
		 */
		void popup(const Gtk::TextIter& textIter, int x, int y, guint32 event_time, bool can_create=true, bool can_edit=true, bool can_delete=true, bool can_unanchor=false, float start=-1, float end=-1);

		/**
		 * Signal emitted when an edition entry is asked (edition or creation)
		 * <b>const Gtk::TextIter& paramter:</b>		TextIterator position
		 */
		sigc::signal<void, const Gtk::TextIter&>&  signalEditAnnotation() { return m_signalEditAnnotation; }

		/**
		 * Signal emitted when an edition entry is asked (edition or creation)
		 * <b>const Gtk::TextIter& paramter:</b>		TextIterator position
		 */
		sigc::signal<void, const Gtk::TextIter&>&  signalDeleteAnnotation() { return m_signalDeleteAnnotation; }

		/**
		 * Signal emitted when an edition entry is asked (unanchor)
		 * <b>const Gtk::TextIter& paramter:</b>		TextIterator position
		 */
		sigc::signal<void, const Gtk::TextIter&>&  signalUnanchorAnnotation() { return m_signalUnanchorAnnotation; }

		/**
		 * Accessor to the annotation type used
		 * @return			Annotation type
		 */
		const std::string& getType() const { return m_type; }

		/**
		 * Accessor to the last iterator position the last popup has been launched from.
		 * @return		Text position
		 */
		const Gtk::TextIter& getIter()  { return getTextIter(); }

		/**
		 * Sets the text position
		 * @param iter		Text position
		 */
		void setIter(const Gtk::TextIter& iter);

		/**
		 * Updates menu properties. Called each time the popup is called.
		 * @param can_create		True for allowing annotation creation
		 * @param can_edit			True for edition mode, False otherwise
		 * @param can_delete		True for allowing annotation deletion
		 * @param can_unanchor		True for allowing timecode deletion, false otherwise
		 */
		virtual void updateMenu(bool can_create=true, bool can_edit=true, bool can_delete=true, bool can_unanchor=false);

		/**
		 * set default menu option hint - does nothing by default, ie. active option is 1st menu option
		 * @param hint option hint
		 */
		virtual void setHint(const std::string& hint) {};

		/**
		 * Indicates whether "delete annotation" option is to be added at menu end
		 * @param b					True for adding "delete" option, False otherwise
		 */
		void setWithStdOptions(bool b) { m_addStdOptions =b; }

	protected:
		/**
		 * Accessor to the current text position
		 * @return		Text position
		 */
		const Gtk::TextIter& getTextIter();

		/**
		 * Edit annotation action callback
		 */
		void onEditAnnotation();

		/**
		 * Delete annotation action callback
		 */
		void onDeleteAnnotation();

		/**
		 * Unanchor annotation callback
		 */
		void onUnanchorAnnotation() ;

		int m_x ; 		/**< x position for menu popup */
		int m_y;  		/**< y position for menu popup */

		/**
		 * Calculates popup menu position
		 * @param[in] 	x			X computed position
		 * @param[in]	y			Y computed position
		 * @param 		push_in		Always returns true
		 * @remarks					Internal method
		 */
		void onPopupMenuPosition(int& x, int& y, bool& push_in);

	protected:
		float m_selectionStart ;
		float m_selectionEnd ;

	private:
		Gtk::TextIter m_textIter; /**< current insert position in parent AnnotationEditor. */

		Glib::RefPtr<Gtk::TextMark> m_popupMark;

		std::string m_type; /**< annotation menu type */
		bool m_addStdOptions; /**< states if "delete annotation" option to be added at menu end or not */
		bool m_editable;
		Gtk::Image m_editImage;
		Gtk::Image m_deleteImage;
		Gtk::Menu_Helpers::ImageMenuElem* m_editItem ;
		Gtk::Menu_Helpers::ImageMenuElem* m_deleteItem ;
		Gtk::Menu_Helpers::MenuElem* m_unanchoredItem ;

		sigc::signal<void, const Gtk::TextIter&>  m_signalEditAnnotation;
		sigc::signal<void, const Gtk::TextIter&>  m_signalDeleteAnnotation;
		sigc::signal<void, const Gtk::TextIter&>  m_signalUnanchorAnnotation;
	};

}
#endif  /* _HAVE_ANNOTATION_MENU */
