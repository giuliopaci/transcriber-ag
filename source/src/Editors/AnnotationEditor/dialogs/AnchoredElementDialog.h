/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 *  @file 	DialogTurnProperties.h
 */

#ifndef __HAVE_ANCHOREDELEMENTDIALOG__
#define __HAVE_ANCHOREDELEMENTDIALOG__

#include "Editors/AnnotationEditor/dialogs/AnnotationPropertiesDialog.h"

#include "Common/widgets/FieldEntry.h"


using namespace std;


namespace tag {
/**
 * @class AnchoredElementDialog
 *
 * Dialog used for displaying and editing anchored element properties.
 */
class AnchoredElementDialog : public AnnotationPropertiesDialog
{
	public:
		/**
		 * Constructor
		 * @param p_win			Reference on window parent
		 * @param model			Reference on parent editor model
		 * @param id			Turn id
		 * @param editable		True for editable dialog, false otherwise
		 */
		AnchoredElementDialog(Gtk::Window& p_win, DataModel& model, const std::string& id, bool editable=true);

		/**
		 * Destructor
		 */
		virtual ~AnchoredElementDialog();

	private:
		// ARCHITECTURE
		Gtk::SpinButton* a_trackEntry;

		FieldEntry* a_startTimeEntry;
		Gtk::Entry* a_startSecondsEntry;

		FieldEntry* a_endTimeEntry;
		Gtk::Entry* a_endSecondsEntry;

		Gtk::Label** a_labels;
		Gtk::Widget** a_entries;

		// - Dynamic properties
		list<Property> a_properties;
		std::map<string, list<string> > a_choiceLists;

		bool mainstreamBaseType ;
		bool segmentBaseType ;

		float start ;
		float end ;

		int track ;

		void prepare_gui(string type) ;
		void prepare_data(string type) ;
		void display_error() ;
		void onButtonClicked(int p_id);
		void updateGUI();
};

} //namespace

#endif // __HAVE_ANCHOREDLEMENTDIALOG__
