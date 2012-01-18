/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include <math.h>
#include <vector>
#include <iterator>
#include <glib.h>


#include "Editors/AnnotationEditor/dialogs/AnchoredElementDialog.h"

#include "Common/Dialogs.h"
#include "Common/iso639.h"
#include "Common/Languages.h"
#include "Common/icons/Icons.h"

using namespace std;

namespace tag {

AnchoredElementDialog::AnchoredElementDialog(Gtk::Window& p_win, DataModel& model, const std::string& id, bool p_editable)
: AnnotationPropertiesDialog(p_win, model, id, p_editable)
{
	mainstreamBaseType = false ;
	segmentBaseType = false ;

	Icons::set_window_icon(this, ICO_TRANSCRIBER, 11) ;

	bool ok = m_dataModel.existsElement(id) ;
    if (ok)
	{
		string type = m_dataModel.getElementType(id) ;
		mainstreamBaseType = (m_dataModel.mainstreamBaseType("transcription_graph") == type ) ;
		segmentBaseType = (m_dataModel.segmentationBaseType("transcription_graph") == type ) ;
		prepare_data(type) ;
		prepare_gui(type) ;
	}
	else
		display_error() ;
}


void AnchoredElementDialog::prepare_data(string type)
{
//	start = a_signalSegment.getStartOffset() ;
//	end = a_signalSegment.getEndOffset() ;

	start = m_dataModel.getElementOffset(m_elementId, true) ;
	end = m_dataModel.getElementOffset(m_elementId, false) ;

//	track = a_signalSegment.getTrack() ;
}

void AnchoredElementDialog::prepare_gui(string type)
{
	int startH = -1 ;
	int startM = -1 ;
	int startS1 = -1 ;
	int startS2 = -1 ;
	int endH = -1 ;
	int endM = -1 ;
	int endS1 = -1 ;
	int endS2 = -1 ;

	if (start>=0)
	{
		startH = (int)(start/3600.0);
		startM = ((int)(start/60.0))%60;
		startS1 = ((int)start)%60;
		startS2 = ((int)roundf(start*1000.0))%1000 ;
	}

	if (end>=0)
	{
		endH = (int)(end/3600.0);
		endM = ((int)(end/60.0))%60;
		endS1 = ((int)end)%60;
		endS2 = ((int)roundf(end*1000.0))%1000 ;
	}
	char str[80];

	Gtk::HBox* firstLine = Gtk::manage(new Gtk::HBox());

	Gtk::Label* blank1 = Gtk::manage(new Gtk::Label(" ")) ;
	Gtk::Label* blank2 = Gtk::manage(new Gtk::Label(" ")) ;
	Gtk::HBox* mainHBox = Gtk::manage(new Gtk::HBox()) ;
	get_vbox()->pack_start(*blank1, false, false, 3);
	get_vbox()->pack_start(*mainHBox, false, false, 3);
	get_vbox()->pack_start(*blank2, false, false, 3);
	blank1->show();
	mainHBox->show();
	blank2->show();

	Gtk::Label* blank3 = Gtk::manage(new Gtk::Label(" ")) ;
	Gtk::Label* blank4 = Gtk::manage(new Gtk::Label(" ")) ;
	Gtk::VBox* mainVBox = Gtk::manage(new Gtk::VBox()) ;
	mainHBox->pack_start(*blank3, false, false, 3);
	mainHBox->pack_start(*mainVBox, false, false, 3);
	mainHBox->pack_start(*blank4, false, false, 3);
	blank3->show();
	mainVBox->show();
	blank4->show();

	Gtk::Label* trackLabel = Gtk::manage(new Gtk::Label("Track :"));
	Gtk::Label* startTimeLabel = Gtk::manage(new Gtk::Label(_("Start time :")));
	Gtk::Label* endTimeLabel = Gtk::manage(new Gtk::Label(_("End time :")));
	Gtk::HBox* startEmpty = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* endEmpty = Gtk::manage(new Gtk::HBox());

	// -- Track number
	int tracksCount = m_dataModel.getNbTracks();
	a_trackEntry = Gtk::manage(new Gtk::SpinButton(*Gtk::manage(new Gtk::Adjustment(track, 1, tracksCount, 1, 1, 0)), 0, 0));

	// -- Start time
	a_startTimeEntry = new FieldEntry(3, ":", 2) ;
	a_startSecondsEntry = Gtk::manage(new Gtk::Entry());
	if ( start!=-1 )
	{
		sprintf(str, "%.2d", startH);
		a_startTimeEntry->set_element(0, string(str));
		sprintf(str, "%.2d", startM);
		a_startTimeEntry->set_element(1, string(str));
		sprintf(str, "%.2d", startS1);
		a_startTimeEntry->set_element(2, string(str));
		sprintf(str, "%.3d", startS2);
		a_startSecondsEntry->set_text(string(str));
	}
	a_startSecondsEntry->set_width_chars(3) ;
	a_startSecondsEntry->set_has_frame(false) ;
	a_startSecondsEntry->set_max_length(3) ;

	//-- End time
	a_endTimeEntry = new FieldEntry(3, ":", 2);
	a_endSecondsEntry = Gtk::manage(new Gtk::Entry());
	if (end!=-1)
	{
		sprintf(str, "%.2d", endH);
		a_endTimeEntry->set_element(0, string(str));
		sprintf(str, "%.2d", endM);
		a_endTimeEntry->set_element(1, string(str));
		sprintf(str, "%.2d", endS1);
		a_endTimeEntry->set_element(2, string(str));
		sprintf(str, "%.3d", endS2);
		a_endSecondsEntry->set_text(string(str));
	}
	a_endSecondsEntry->set_width_chars(3) ;
	a_endSecondsEntry->set_has_frame(false) ;
	a_endSecondsEntry->set_max_length(3) ;

	startEmpty->set_size_request(5, -1);
	endEmpty->set_size_request(5, -1);
	a_trackEntry->set_width_chars(2);
	a_startTimeEntry->set_width_chars(11);
	a_endTimeEntry->set_width_chars(11);

	firstLine->pack_start(*trackLabel, false, false, 3);
	firstLine->pack_start(*a_trackEntry, true, true, 3);
	firstLine->pack_start(*startEmpty, true, true, 3);
	firstLine->pack_start(*startTimeLabel, false, false, 3);
	firstLine->pack_start(*a_startTimeEntry, false, false, 3);
	firstLine->pack_start(*a_startSecondsEntry, false, false, 0);
	firstLine->pack_start(*endEmpty, true, true, 3);
	firstLine->pack_start(*endTimeLabel, false, false, 3);
	firstLine->pack_start(*a_endTimeEntry, false, false, 3);
	firstLine->pack_start(*a_endSecondsEntry, false, false, 0);
	mainVBox->pack_start(*firstLine, false, false, 6);

	trackLabel->show();
	a_trackEntry->show();

	startEmpty->show();
	startTimeLabel->show();
	a_startTimeEntry->show();
	a_startSecondsEntry->show();
	endEmpty->show();
	endTimeLabel->show();
	a_endTimeEntry->show();
	a_endSecondsEntry->show();
	firstLine->show();

	string title = type + " " + _("properties") ;
	set_title(title);

	if (!mainstreamBaseType && !segmentBaseType)
	{
		a_labels = new Gtk::Label*[a_properties.size()];
		a_entries = new Gtk::Widget*[a_properties.size()];

		int ind = 0;
		list<Property>::iterator it2 = a_properties.begin();
		while (it2 != a_properties.end())
		{
			Property p = *it2;
			string s = p.label + " :";
			a_labels[ind] = new Gtk::Label(s.c_str());
			a_labels[ind]->set_text(a_labels[ind]->get_text()+"                             ");
			a_labels[ind]->set_size_request(85, -1);
			if (p.type == PROPERTY_TEXT) {
				a_entries[ind] = new Gtk::Entry();
			}
			else if (p.type == PROPERTY_CHOICELIST) {
				a_entries[ind] = new Gtk::ComboBoxText();
				list<string> choices = a_choiceLists[p.choiceList];
				list<string>::iterator it3 = choices.begin();
				while (it3 != choices.end()) {
					const char* c = it3->c_str();
					((Gtk::ComboBoxText*)a_entries[ind])->append_text(c);
					it3++;
				}
			}
			it2++;
			ind++;
		}

		Gtk::Frame* transcriptionFrame = Gtk::manage(new Gtk::Frame(_("Transcription")));
		transcriptionFrame->set_shadow_type(Gtk::SHADOW_IN);
		mainVBox->pack_start(*transcriptionFrame, false, false, 6);
		transcriptionFrame->show();

		Gtk::VBox* transcriptionFrameVBox = Gtk::manage(new Gtk::VBox());
		Gtk::HBox* transcriptionFrameHBox1 = Gtk::manage(new Gtk::HBox());
		Gtk::HBox* transcriptionFrameHBox2 = Gtk::manage(new Gtk::HBox());
		Gtk::HBox* transcriptionFrameHBox3 = Gtk::manage(new Gtk::HBox());
		Gtk::HBox* transcriptionFrameHBox32 = Gtk::manage(new Gtk::HBox());
		Gtk::HBox* transcriptionFrameHBox4 = Gtk::manage(new Gtk::HBox());
		transcriptionFrame->add(*transcriptionFrameVBox);
		transcriptionFrameVBox->pack_start(*transcriptionFrameHBox1, false, false, 3);
		transcriptionFrameVBox->pack_start(*transcriptionFrameHBox2, false, false, 3);
		transcriptionFrameVBox->pack_start(*transcriptionFrameHBox3, false, false, 3);
		transcriptionFrameVBox->pack_start(*transcriptionFrameHBox32, false, false, 3);
		transcriptionFrameVBox->pack_start(*transcriptionFrameHBox4, false, false, 3);
		transcriptionFrameVBox->show();
		transcriptionFrameHBox1->show();
		transcriptionFrameHBox2->show();
		transcriptionFrameHBox3->show();
		transcriptionFrameHBox32->show();
		transcriptionFrameHBox4->show();

		for (int i = 0; i < ind; i++)
		{
			Gtk::HBox* box = Gtk::manage(new Gtk::HBox());
			transcriptionFrameVBox->pack_start(*box, false, false, 3);
			box->show();
			Gtk::HBox* vide = Gtk::manage(new Gtk::HBox()) ;
			vide->set_size_request(30, -1) ;
			box->pack_start(*vide, false, false, 3);
			box->pack_start(*a_labels[i], false, false, 3);
			box->pack_start(*a_entries[i], true, true, 3);
			vide->show();
			a_labels[i]->show();
			a_entries[i]->show();
		}
		transcriptionFrameVBox->set_sensitive(m_editable);
	}

	Gtk::Button* ok;
	if ( m_editable )
	{
		ok = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
		Gtk::Button* cancel = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		cancel->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &AnchoredElementDialog::onButtonClicked), Gtk::RESPONSE_CANCEL));
	}
	else
	{
		ok = add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_OK);
		firstLine->set_sensitive(false);
	}

	ok->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &AnchoredElementDialog::onButtonClicked), Gtk::RESPONSE_OK));

	updateGUI();
	get_vbox()->show_all_children() ;
}

void AnchoredElementDialog::display_error()
{
	Gtk::VBox* vBox = get_vbox();
	Gtk::Alignment* align = Gtk::manage(new Gtk::Alignment()) ;
	Glib::ustring msg = _("Unable to find element") ;
	msg = "    " + msg + "    " ;
	Gtk::Label* label = Gtk::manage(new Gtk::Label(msg)) ;
	vBox->pack_start(*align, false, false, 5) ;
	align->add(*label) ;
	align->set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER) ;
	Icons::set_window_icon(this, ICO_ERROR, 11) ;
	Gtk::Button* close = add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
	close->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &AnchoredElementDialog::onButtonClicked), Gtk::RESPONSE_CLOSE));

	vBox->show_all_children(true) ;
}

AnchoredElementDialog::~AnchoredElementDialog()
{
	if (a_startTimeEntry)
		delete(a_startTimeEntry) ;
	if (a_endTimeEntry)
		delete(a_endTimeEntry) ;
}

void AnchoredElementDialog::onButtonClicked(int p_id)
{

	if (p_id == Gtk::RESPONSE_OK && m_editable)
	{
		string diag;
		bool is_updated = false;

		int startH = -1 ;
		int startM = -1 ;
		int startS1 = -1 ;
		int startS2 = -1 ;
		int endH = -1 ;
		int endM = -1 ;
		int endS1 = -1 ;
		int endS2 = -1 ;
		float new_start = -1 ;
		float new_end = -1 ;

		string startElem0, startElem1, startElem2, startSec ;
		startElem0 = a_startTimeEntry->get_element(0).c_str() ;
		startElem1 = a_startTimeEntry->get_element(1).c_str() ;
		startElem2 = a_startTimeEntry->get_element(2).c_str() ;
		startSec = a_startSecondsEntry->get_text().c_str() ;

		// -- Check if we have some values to take from GUI
		if (!startElem0.empty() && !startElem1.empty() && !startElem2.empty() && !startSec.empty())
		{
			int startH = atoi(startElem0.c_str()) ;
			int startM = atoi(startElem1.c_str()) ;
			int startS1 = atoi(startElem2.c_str()) ;
			int startS2 = atoi(startSec.c_str()) ;
			new_start = startH*3600 + startM*60 + startS1 + ( (float)startS2/1000 );
			/*
			 * Warning:
			 * Smaller user increment is 1 ms (depending on the entry : no risk)
			 * but using float can make sometimes the result of fabs smaller than 0.001 (ie 0.00999...96)
			 * dirty hack: compare with 0.0009
			 */
			is_updated = ( fabs(new_start -  m_dataModel.getElementOffset(m_elementId, true) ) > 0.0009 );
			if (!is_updated)
				Log::err() << "anchoredElementDialog ->  invalid new time" << std::endl ;
		}
		// -- No values and initial value was positive ? means we want to unanchor
		else if (start!=-1)
			is_updated = true ;
		// No values and initial value was negative ? means nothing has changed
		else
			new_start = start ;

		string endElem0, endElem1, endElem2, endSec ;
		endElem0 = a_endTimeEntry->get_element(0).c_str() ;
		endElem1 = a_endTimeEntry->get_element(1).c_str() ;
		endElem2 = a_endTimeEntry->get_element(2).c_str() ;
		endSec = a_endSecondsEntry->get_text().c_str() ;

		// -- Check if we have some values to take from GUI
		if (!endElem0.empty() && !endElem1.empty() && !endElem2.empty() && !endSec.empty())
		{
			int endH = atoi(endElem0.c_str());
			int endM = atoi(endElem1.c_str());
			int endS1 = atoi(endElem2.c_str());
			int endS2 = atoi(endSec.c_str());
			new_end = endH*3600 + endM*60 + endS1 + ((float)endS2 / 1000);

			if ( ! is_updated )
			{
				/*
				 * Warning:
				 * Smaller user increment is 1 ms (depending on the entry : no risk)
				 * but using float can make sometimes the result of fabs smaller than 0.001 (ie 0.00999...96)
				 * dirty hack: compare with 0.0009
				 */
				is_updated = ( fabs(new_end - m_dataModel.getElementOffset(m_elementId, false) ) > 0.0009);
				if (!is_updated)
					Log::err() << "anchoredElementDialog ->  invalid new time" << std::endl ;
			}
		}
		// -- No values and initial value was positive ? means we want to unanchor
		else if (end!=-1)
			is_updated = true ;
		// No values and initial value was negative ? means nothing has changed
		else
			new_end = end ;

		// -- Something has changed  ? let's proceed
		if ( is_updated  )
		{
			// -- First check availability
			// RemaRk : allow unanchor offset
			if ( m_dataModel.checkResizeRules(m_elementId, new_start, new_end, diag, true, false, true) == false )
			{
				dlg::warning(diag);
				return;
			}

			// -- Proceed

			if (new_start != -1)
				m_dataModel.setElementOffset(m_elementId, new_start, true, true) ;
			else
				m_dataModel.unsetElementOffset(m_elementId, true, true) ;

			if (new_end != -1)
				m_dataModel.setElementOffset(m_elementId, new_end, false, true) ;
			else
				m_dataModel.unsetElementOffset(m_elementId, false, true) ;
		}
	} // end response_ok

	response(m_editable ? p_id: Gtk::RESPONSE_CANCEL);
	hide();
}
void AnchoredElementDialog::updateGUI()
{
	for (guint i = 0; i < a_properties.size(); i++)
	{
		a_labels[i]->set_sensitive(true);
		a_entries[i]->set_sensitive(true);
	}
}

} //NAMESPACE
