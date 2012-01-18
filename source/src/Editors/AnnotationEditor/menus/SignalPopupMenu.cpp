/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @file SectSignalPopup.cpp
 *   @brief Qualfiers popup menu implementation
 */

#include <iostream>
#include <map>
#include <iterator>

#include "SignalPopupMenu.h"

#include "Editors/AnnotationEditor/AnnotationEditor.h"

#include "Common/globals.h"
#include "Common/util/Utils.h"
#include "Common/util/StringOps.h"
#include "Common/Dialogs.h"
#include "Common/Parameters.h"
#include "Common/FileInfo.h"

#include "AudioWidget/AudioSignalView.h"

using namespace Gtk::Menu_Helpers;
using namespace std;



namespace tag {


SignalPopupMenu::SignalPopupMenu(AnnotationEditor* parent, Gtk::Menu* trackmenu)
: AnnotationMenu(_("signal"), false)
{
		// segmentation items
	bool can_edit = parent->isEditMode() && parent->getTextview().get_editable() ;
	Glib::RefPtr<Gtk::Action> action;
	Glib::RefPtr<Gtk::ActionGroup> ag = parent->getActionGroup("annotate");

	if ( can_edit ) {

		// -- Menu turn
		Gtk::Menu* sub_turn = Gtk::manage(new Gtk::Menu()) ;

		action = ag->get_action("annotate_new_turn");
		sub_turn->items().push_back(MenuElem(action->property_label().get_value(),
					sigc::bind<string, bool>(sigc::mem_fun(*parent, &AnnotationEditor::onMenuAction),
						"new_turn", true, false, "")));

		//action = ag->get_action("annotate_new_turn");
		sub_turn->items().push_back(MenuElem(_("New no speech turn"),
					sigc::bind<string, bool>(sigc::mem_fun(*parent, &AnnotationEditor::onMenuAction),
						"new_turn", true, false, Speaker::NO_SPEECH)));

		items().push_back(MenuElem(_("Turn"), *sub_turn)) ;

//		// -- Foreground event
//		Gtk::Menu* sub_fe = Gtk::manage(new Gtk::Menu()) ;
//		action = ag->get_action("annotate_new_unit_event");
//		sub_fe->items().push_back(MenuElem(action->property_label().get_value(),
//					sigc::bind<string, bool>(sigc::mem_fun(*parent, &AnnotationEditor::onMenuAction),
//						"new_unit_event", true, false, "")));
//		items().push_back(MenuElem(_("Foreground event"), *sub_fe)) ;

		// -- Menu Background
		if (parent->getDataModel().hasAnnotationGraph("background_graph"))
		{
			Gtk::Menu* sub_bg = Gtk::manage(new Gtk::Menu()) ;

			action = ag->get_action("annotate_new_background");
			sub_bg->items().push_back(MenuElem(action->property_label().get_value(),
						sigc::bind<string, bool>(sigc::mem_fun(*parent, &AnnotationEditor::onMenuAction),
							"new_background", true, false, "")));
			action = ag->get_action("edit_background");
			sub_bg->items().push_back(MenuElem(action->property_label().get_value(),
						sigc::bind<string, bool>(sigc::mem_fun(*parent, &AnnotationEditor::onMenuAction),
							"edit_background", true, false, "")));
			action = ag->get_action("delete_background");
			sub_bg->items().push_back(MenuElem(action->property_label().get_value(),
						sigc::bind<string, bool>(sigc::mem_fun(*parent, &AnnotationEditor::onMenuAction),
							"delete_background", true, false, "")));

			items().push_back(MenuElem(_("Background"), *sub_bg)) ;
		}
	}

	items().push_back(SeparatorElem());

	//> Save selection
	if (!parent->getSignalView()->isStreaming())
	{
		ag = parent->getActionGroup("signal");
		action = ag->get_action("save_selected_signal");
		items().push_back(MenuElem(action->property_label().get_value(),
							sigc::bind<string, bool>(sigc::mem_fun(*parent, &AnnotationEditor::onMenuAction),
							"save_selected_signal", true, false, "")));

		items().push_back(SeparatorElem());
	}

	// -- Exports to Externals (Audacity / WaveSurfer) --
	ag		= parent->getActionGroup("signal");
	action	= ag->get_action("export_to_audacity");

	items().push_back(MenuElem(action->property_label().get_value(),
						sigc::bind<string, bool>(sigc::mem_fun(*parent, &AnnotationEditor::onMenuAction),
						"export_to_audacity", true, false, "")));

	action	= ag->get_action("export_to_wavesurfer");

	items().push_back(MenuElem(action->property_label().get_value(),
						sigc::bind<string, bool>(sigc::mem_fun(*parent, &AnnotationEditor::onMenuAction),
						"export_to_wavesurfer", true, false, "")));

	action	= ag->get_action("export_to_praat");

	items().push_back(MenuElem(action->property_label().get_value(),
						sigc::bind<string, bool>(sigc::mem_fun(*parent, &AnnotationEditor::onMenuAction),
						"export_to_praat", true, false, "")));

	items().push_back( SeparatorElem() );
	// Signal management items
	items().push_back(MenuElem(_("Show _tracks"), *trackmenu));
}


/* destructor */
SignalPopupMenu::~SignalPopupMenu()
{
	TRACE << "Death of SignalPopupMenu" << endl;
}

void SignalPopupMenu::popup(const Gtk::TextIter& iter, int x, int y, guint32 event_time)
{
	m_x=x;  m_y=y;
	setIter(iter);

	//> add a separator on top of menu because of persistance bug
	if ( items().size() > 0 )
	{
		Gtk::Image* image = Gtk::manage(new Gtk::Image()) ;
		try {
			Glib::RefPtr<Gtk::IconTheme>theme = Gtk::IconTheme::get_default() ;
			Glib::RefPtr<Gdk::Pixbuf>pixbufPlay = theme->load_icon("preferences_audio", 12,Gtk::ICON_LOOKUP_NO_SVG | Gtk::ICON_LOOKUP_USE_BUILTIN);
			image->set(pixbufPlay);
			Gtk::MenuItem* item = Gtk::manage(new Gtk::ImageMenuItem( *image, _("SIGNAL ACTIONS"),true)) ;
			items().push_front(MenuElem(*item));
		}
		catch (Gtk::IconThemeError e) {
			Log::err() << "Signal menu:> while searching icon:> " <<  e.what()  << std::endl ;
			Gtk::MenuItem* item = Gtk::manage(new Gtk::ImageMenuItem(_("SIGNAL ACTIONS"),true)) ;
			items().push_front(MenuElem(*item));
		}
	}
	((Gtk::Menu*)(this))->popup(sigc::mem_fun(*this, &SignalPopupMenu::onPopupMenuPosition), 1, event_time);
}


} /* namespace tag */
