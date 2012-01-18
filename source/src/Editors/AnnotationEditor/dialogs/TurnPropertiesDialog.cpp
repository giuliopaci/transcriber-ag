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

#include "Editors/AnnotationEditor/dialogs/TurnPropertiesDialog.h"
#include "Editors/AnnotationEditor/handlers/SAXAnnotationsHandler.h"

#include "Common/Dialogs.h"
#include "Common/iso639.h"
#include "Common/Languages.h"
#include "Common/icons/Icons.h"

using namespace std;

namespace tag {

TurnPropertiesDialog::TurnPropertiesDialog(Gtk::Window& p_win, DataModel& model, const std::string& id, bool m_editable)
: AnnotationPropertiesDialog(p_win, model, id, m_editable)
{
	mainstreamBaseType = false ;
	segmentBaseType = false ;

	Icons::set_window_icon(this, ICO_TRANSCRIBER, 11) ;

	// Modif checktrace argument to -1, for avoiding problem when track is track1
	bool ok = m_dataModel.getSegment(m_elementId, a_signalSegment, -1, false, true, -1);
	if (ok)
	{
		this->id = m_elementId ;
		string type = m_dataModel.getElementType(m_elementId) ;
		mainstreamBaseType = (m_dataModel.mainstreamBaseType("transcription_graph") == type ) ;
		segmentBaseType = (m_dataModel.segmentationBaseType("transcription_graph") == type ) ;
		prepare_data(type) ;
		prepare_gui(type) ;
	}
	else
		display_error() ;
}


void TurnPropertiesDialog::prepare_data(string type)
{
	start = a_signalSegment.getStartOffset() ;
	end = a_signalSegment.getEndOffset() ;
	track = a_signalSegment.getTrack() ;
	isSpeech = m_dataModel.isSpeechSegment(a_signalSegment.getId()) ;

	if (type!=m_dataModel.mainstreamBaseType("transcription_graph"))
	{
		a_speakerDictionary = &(m_dataModel.getSpeakerDictionary());
		a_speakerDictionary->signalSpeakerUpdated().connect(sigc::mem_fun(*this, &TurnPropertiesDialog::on_combos_changed) );
		a_speakerDictionary->signalSpeakerDeleted().connect(sigc::mem_fun(*this, &TurnPropertiesDialog::on_combos_changed) );

		// get speaker
		string speakerID = m_dataModel.getElementProperty(m_elementId, "speaker");
		Speaker speaker;
		try {
			speaker = a_speakerDictionary->getSpeaker(speakerID);
		} catch (...) {
			speaker = Speaker::noSpeaker();
		}
		const char* speakerFullName = speaker.getFullName().c_str();


		const Speaker::Language& usual_lang = speaker.getUsualLanguage() ;
		string lang = usual_lang.getCode();
		string dialect;
		if ( lang.empty() )
		{
			lang = m_dataModel.getTranscriptionLanguage();
			dialect = "";
		} else dialect = usual_lang.getDialect();

		lang = m_dataModel.getElementProperty(m_elementId, "lang", lang);
		dialect = m_dataModel.getElementProperty(m_elementId, "dialect", dialect);
	}
}

void TurnPropertiesDialog::prepare_gui(string type)
{
	int startH = (int)(start/3600.0);
	int startM = ((int)(start/60.0))%60;
	int startS1 = ((int)start)%60;
	int startS2 = ((int)roundf(start*1000.0))%1000;
	int endH = (int)(end/3600.0);
	int endM = ((int)(end/60.0))%60;
	int endS1 = ((int)end)%60;
	int endS2 = ((int)roundf(end*1000.0))%1000;
	char str[80];

	Gtk::HBox* videA = new Gtk::HBox();
	Gtk::HBox* mainHBox = new Gtk::HBox();
	Gtk::HBox* videB = new Gtk::HBox();
	get_vbox()->pack_start(*videA, false, false, 3);
	get_vbox()->pack_start(*mainHBox, false, false, 3);
	get_vbox()->pack_start(*videB, false, false, 3);
	videA->show();
	mainHBox->show();
	videB->show();

	Gtk::HBox* videC = new Gtk::HBox();
	Gtk::VBox* mainVBox = new Gtk::VBox();
	Gtk::HBox* videD = new Gtk::HBox();
	mainHBox->pack_start(*videC, false, false, 3);
	mainHBox->pack_start(*mainVBox, false, false, 3);
	mainHBox->pack_start(*videD, false, false, 3);
	videC->show();
	mainVBox->show();
	videD->show();

	Gtk::HBox* firstLine = Gtk::manage(new Gtk::HBox());
	Gtk::Label* trackLabel = Gtk::manage(new Gtk::Label("Track :"));
	Gtk::Label* startTimeLabel = Gtk::manage(new Gtk::Label(_("Start time :")));
	Gtk::Label* endTimeLabel = Gtk::manage(new Gtk::Label(_("End time :")));
	Gtk::HBox* startEmpty = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* endEmpty = Gtk::manage(new Gtk::HBox());

	int tracksCount = m_dataModel.getNbTracks();
	a_trackEntry = Gtk::manage(new Gtk::SpinButton(*Gtk::manage(new Gtk::Adjustment(track, 1, tracksCount, 1, 1, 0)), 0, 0));
	a_startTimeEntry = new FieldEntry(3, ":", 2) ;
	sprintf(str, "%.2d", startH);
	a_startTimeEntry->set_element(0, string(str));
	sprintf(str, "%.2d", startM);
	a_startTimeEntry->set_element(1, string(str));
	sprintf(str, "%.2d", startS1);
	a_startTimeEntry->set_element(2, string(str));
	a_startSecondsEntry = Gtk::manage(new Gtk::Entry());
	sprintf(str, "%.3d", startS2);
	a_startSecondsEntry->set_text(string(str));
	a_startSecondsEntry->set_width_chars(3) ;
	a_startSecondsEntry->set_has_frame(false) ;
	a_startSecondsEntry->set_max_length(3) ;

	a_endTimeEntry = new FieldEntry(3, ":", 2);
	sprintf(str, "%.2d", endH);
	a_endTimeEntry->set_element(0, string(str));
	sprintf(str, "%.2d", endM);
	a_endTimeEntry->set_element(1, string(str));
	sprintf(str, "%.2d", endS1);
	a_endTimeEntry->set_element(2, string(str));
	a_endSecondsEntry = Gtk::manage(new Gtk::Entry());
	sprintf(str, "%.3d", endS2);
	a_endSecondsEntry->set_text(string(str));
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

		Gtk::RadioButtonGroup group;
		a_speech = Gtk::manage(new Gtk::RadioButton(group, string(_("Speech"))+"   "));
		a_speech->signal_toggled().connect(sigc::mem_fun(*this, &TurnPropertiesDialog::updateGUI));

		Gtk::HBox* vide1 = Gtk::manage(new Gtk::HBox());
		a_lspeaker = Gtk::manage(new Gtk::Label(string(_("Speaker"))+" :"));
		a_lspeaker->set_text(a_lspeaker->get_text()+"                             ");
		a_speakerCombo = Gtk::manage(new Gtk::ComboBoxText());
		prepare_speakers_combo() ;

		Gtk::HBox* vide2 = Gtk::manage(new Gtk::HBox());
		a_llanguage = Gtk::manage(new Gtk::Label(string(_("Language"))+" :"));
		a_llanguage->set_text(a_llanguage->get_text()+"                             ");
		a_languageCombo = Gtk::manage(new Gtk::ComboBoxText());
		prepare_languages_combo() ;

		a_ldialect = Gtk::manage(new Gtk::Label(string(_("Dialect"))+" :"));
		a_dialectCombo = Gtk::manage(new Gtk::ComboBoxText());
		a_dialectCombo->append_text("");

		a_noSpeech = Gtk::manage(new Gtk::RadioButton(group, _("No speech")));
		a_noSpeech->signal_toggled().connect(sigc::mem_fun(*this, &TurnPropertiesDialog::updateGUI));

		if (isSpeech) a_speech->set_active(true);
		else a_noSpeech->set_active(true);

		vide1->set_size_request(30, -1);
		vide2->set_size_request(30, -1);

		a_lspeaker->set_size_request(85, -1);
		a_llanguage->set_size_request(85, -1);

		a_languageCombo->signal_changed().connect(sigc::mem_fun(*this, &TurnPropertiesDialog::on_comboLanguage_changed)) ;

		transcriptionFrameHBox1->pack_start(*a_speech, false, false, 3);
		transcriptionFrameHBox2->pack_start(*vide1, false, false, 3);
		transcriptionFrameHBox2->pack_start(*a_lspeaker, false, false, 3);
		transcriptionFrameHBox2->pack_start(*a_speakerCombo, true, true, 3);
		transcriptionFrameHBox3->pack_start(*vide2, false, false, 3);
		transcriptionFrameHBox3->pack_start(*a_llanguage, false, false, 3);
		transcriptionFrameHBox3->pack_start(*a_languageCombo, true, true, 3);

		for (int i = 0; i < ind; i++) {
			Gtk::HBox* box = Gtk::manage(new Gtk::HBox());
			transcriptionFrameVBox->pack_start(*box, false, false, 3);
			box->show();
			Gtk::HBox* vide = Gtk::manage(new Gtk::HBox());
			vide->set_size_request(30, -1);
			box->pack_start(*vide, false, false, 3);
			box->pack_start(*a_labels[i], false, false, 3);
			box->pack_start(*a_entries[i], true, true, 3);
			vide->show();
			a_labels[i]->show();
			a_entries[i]->show();
		}

		Gtk::HBox* transcriptionFrameHBox5 = Gtk::manage(new Gtk::HBox());
		transcriptionFrameVBox->pack_start(*transcriptionFrameHBox5, false, false, 3);
		transcriptionFrameHBox5->show();
		transcriptionFrameHBox5->pack_start(*a_noSpeech, false, false, 3);

		a_speech->show();
		vide1->show();
		a_lspeaker->show();
		a_speakerCombo->show();
		vide2->show();
		a_llanguage->show();
		a_languageCombo->show();
		a_ldialect->show();
		a_dialectCombo->show();
		a_noSpeech->show();

		transcriptionFrameVBox->set_sensitive(m_editable);
	}

	Gtk::Button* ok;
	if ( m_editable )
	{
		ok = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
		Gtk::Button* cancel = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		cancel->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &TurnPropertiesDialog::onButtonClicked), Gtk::RESPONSE_CANCEL));
	}
	else
	{
		ok = add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_OK);
		firstLine->set_sensitive(false);
	}

	ok->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &TurnPropertiesDialog::onButtonClicked), Gtk::RESPONSE_OK));


	updateGUI();
	get_vbox()->show_all_children() ;
}

void TurnPropertiesDialog::display_error()
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
	close->signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &TurnPropertiesDialog::onButtonClicked), Gtk::RESPONSE_CLOSE));

	vBox->show_all_children(true) ;
}

TurnPropertiesDialog::~TurnPropertiesDialog()
{
	if (a_startTimeEntry)
		delete(a_startTimeEntry) ;
	if (a_endTimeEntry)
		delete(a_endTimeEntry) ;
}

void TurnPropertiesDialog::onButtonClicked(int p_id)
{

	if (p_id == Gtk::RESPONSE_OK && m_editable)
	{
		string diag;
		bool is_updated = false;

		int startH = atoi(a_startTimeEntry->get_element(0).c_str());
		int startM = atoi(a_startTimeEntry->get_element(1).c_str());
		int startS1 = atoi(a_startTimeEntry->get_element(2).c_str());
		int startS2 = atoi(a_startSecondsEntry->get_text().c_str());
		float start = startH*3600 + startM*60 + startS1 + ((float)startS2 / 1000);
		/*
		 * Warning:
		 * Smaller user increment is 1 ms (depending on the entry : no risk)
		 * but using float can make sometimes the result of fabs smaller than 0.001 (ie 0.00999...96)
		 * dirty hack: compare with 0.0009
		 */
		is_updated = ( fabs(start - a_signalSegment.getStartOffset()) > 0.0009);
		if (!is_updated)
		{
			Log::err() << "anchoredElementDialog ->  invalid new time" << std::endl ;
			gdk_beep() ;
		}

		int endH = atoi(a_endTimeEntry->get_element(0).c_str());
		int endM = atoi(a_endTimeEntry->get_element(1).c_str());
		int endS1 = atoi(a_endTimeEntry->get_element(2).c_str());
		int endS2 = atoi(a_endSecondsEntry->get_text().c_str());
		float end = endH*3600 + endM*60 + endS1 + ((float)endS2 / 1000);

		if ( ! is_updated )
		{
			/*
			* Warning:
			* Smaller user increment is 1 ms (depending on the entry : no risk)
			* but using float can make sometimes the result of fabs smaller than 0.001 (ie 0.00999...96)
			* dirty hack: compare with 0.0009
			*/
			is_updated = ( fabs(start - a_signalSegment.getStartOffset()) > 0.0009 ) ;
			if (!is_updated)
			{
				Log::err() << "anchoredElementDialog ->  invalid new time" << std::endl ;
				gdk_beep() ;
			}
		}

		if ( is_updated  )
		{
			if ( m_dataModel.checkResizeRules(m_elementId, start, end, diag) == false )
			{
				dlg::warning(diag);
				return;
			}
		}

		if ( is_updated )
			m_dataModel.setSegmentOffsets(m_elementId, start, end, true, true);

		if (!mainstreamBaseType && !segmentBaseType)
		{
			Languages* languages = Languages::getInstance();
			string lang = languages->get_code(a_languageCombo->get_active_text());

			//TOTO
			//if differs from speakers language
			if(lang != m_dataModel.getSegmentLanguage(m_elementId, false)) {
				m_dataModel.setElementProperty(m_elementId, "lang", lang, false);
				m_signalTurnLanguageChanged.emit(m_elementId) ;
			}
			m_dataModel.setElementProperty(m_elementId, "dialect", a_dialectCombo->get_active_text(), false);

			const string& prev_spk = m_dataModel.getElementProperty(m_elementId, "speaker", Speaker::NO_SPEECH);
			string spk = Speaker::NO_SPEECH;
			if ( a_speech->get_active() ) {
				SpeakerDictionary::iterator it = a_speakerDictionary->begin();
				for (int i = 0; it != a_speakerDictionary->end()
								&& i < a_speakerCombo->get_active_row_number(); i++) it++;
				if ( it == a_speakerDictionary->end() ) spk = "";
				else spk =  it->second.getId();
				if ( spk != prev_spk ) {
					if ( spk == "" )
						m_dataModel.deleteElementProperty(m_elementId, "speaker");
					else
						m_dataModel.setElementProperty(m_elementId, "speaker", spk, true);
				}
			}
			else {
				if ( spk != prev_spk )
					m_dataModel.deleteElementProperty(m_elementId, "speaker");
			}
		} // end !mainstreamBaseType && !segmentBaseType
	} // end response_ok

	hide();
	response(m_editable ? p_id: Gtk::RESPONSE_CANCEL);
}

void TurnPropertiesDialog::updateGUI()
{
	if (!mainstreamBaseType && !segmentBaseType)
	{
		if (a_speech == NULL) return;
		bool b = a_speech->get_active();
		a_speakerCombo->set_sensitive(b);
		a_languageCombo->set_sensitive(b);
		a_dialectCombo->set_sensitive(b);
		a_lspeaker->set_sensitive(b);
		a_llanguage->set_sensitive(b);
		a_ldialect->set_sensitive(b);
	}
	for (guint i = 0; i < a_properties.size(); i++) {
		a_labels[i]->set_sensitive(true);
		a_entries[i]->set_sensitive(true);
	}
}

void TurnPropertiesDialog::on_combos_changed(string id)
{
	prepare_speakers_combo() ;
	prepare_languages_combo() ;
}

void TurnPropertiesDialog::prepare_speakers_combo()
{
	a_speakerCombo->clear() ;

	string speakerID = m_dataModel.getElementProperty(m_elementId, "speaker");

	Speaker speaker;
	try {
		speaker = a_speakerDictionary->getSpeaker(speakerID);
	} catch (...) {
		speaker = Speaker::noSpeaker();
	}

	const char* speakerFullName = speaker.getFullName().c_str();

	SpeakerDictionary::iterator it = a_speakerDictionary->begin();
	int ind1 = 0;
	int ind2 = -1;
	while (it != a_speakerDictionary->end()) {
		const string& a = it->second.getFullName();
		a_speakerCombo->append_text(a);
		if (a == speakerFullName)
			ind2 = ind1;
		it++;
		ind1++;
	}
	a_speakerCombo->append_text(Speaker::noSpeaker().getFullName());
	if ( ind2 == -1 ) ind2=ind1;
	a_speakerCombo->set_active(ind2);
}

void TurnPropertiesDialog::prepare_languages_combo()
{
	a_languageCombo->clear() ;

	string speakerID = m_dataModel.getElementProperty(m_elementId, "speaker");
	string lang = m_dataModel.getSegmentLanguage(m_elementId, false);
	string labelText="";

	// turn has no language, use speaker one
	if(lang == "")	{
		// CONSIDER TRANSCRIPTION FILE LANGUAGE AS DEFAULT LANGUAGE
		lang = m_dataModel.getTranscriptionLanguage();
		labelText = _("File language");
	}

	Languages* languages = Languages::getInstance();
	vector<Glib::ustring>::const_iterator itl;
	const vector<Glib::ustring>& lcodes = languages->get_codes();
	int i;
	for ( itl = lcodes.begin(), i=0; itl != lcodes.end(); ++itl , ++i) {
		a_languageCombo->append_text(languages->get_name(*itl));
		if (lang == *itl )
			a_languageCombo->set_active(i);
	}
	defaultSpeaker_label.set_label(labelText);
}

void TurnPropertiesDialog::on_comboLanguage_changed()
{
	defaultSpeaker_label.set_label(_("Turn language"));
}

} //NAMESPACE
