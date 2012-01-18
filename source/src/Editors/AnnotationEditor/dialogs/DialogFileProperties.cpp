#include <gtkmm.h>

#include "DialogFileProperties.h"

#include "Editors/AnnotationEditor/dialogs/FileDialogs.h"

#include "DataModel/speakers/SpeakerDictionary.h"
#include "DataModel/signals/SignalSegment.h"
#include "DataModel/speakers/Speaker.h"
#include "DataModel/versions/Version.h"

#include "Common/FileInfo.h"
#include "Common/globals.h"
#include "Common/icons/Icons.h"

#include "MediaComponent/base/Guesser.h"

//#include "Explorer_fileHelper.h"

namespace tag {

DialogFileProperties::DialogFileProperties(Gtk::Window& p_parent, AnnotationEditor* editor,
											bool p_displayAnnotationTime, Parameters& p_params, bool p_editable)
: Gtk::Dialog(_("File properties"), p_parent, true, true), a_model(editor->getDataModel()),
	        a_versionList(editor->getDataModel().getVersionList()), a_params(p_params),
	        a_editable(p_editable)
{
	Icons::set_window_icon(this, ICO_TRANSCRIBER, 12) ;

	need_reload = false;
	need_save = false;

	SIGNAL_PROPERTIES.push_back("file");
	SIGNAL_PROPERTIES.push_back("format");
	SIGNAL_PROPERTIES.push_back("encoding");
	SIGNAL_PROPERTIES.push_back("channels");
	SIGNAL_PROPERTIES.push_back("length");
	SIGNAL_PROPERTIES.push_back("type");
	SIGNAL_PROPERTIES.push_back("source");
	SIGNAL_PROPERTIES.push_back("date");
	SIGNAL_PROPERTIES.push_back("comment");

	a_parent = &p_parent;

	a_model.getSignalsProperties(a_signals);
	a_displayAnnotationTime = p_displayAnnotationTime;

	string filename = editor->getFileName() ;
	filename = Glib::path_get_basename(filename) ;
	a_values["file"] = filename ;
	a_values["corpus"] = a_model.getCorpusName();
	a_values["corpus_version"] = a_model.getCorpusVersion();
	a_values["comment"] = a_model.getTranscriptionProperty("comment");

	std::map<string, std::map<string, string> >::iterator itA =
	        a_signals.begin();
	a_signal1ID = itA->first;
	a_signal1 = itA->second;
	a_signalsCount = 1;
	if (a_signals.size() > 1)
	{
		itA++;
		a_signal2ID = itA->first;
		a_signal2 = itA->second;
		a_signalsCount = 2;
	}

	string extra_props=a_model.conventions().getConfiguration("extra,properties");
	loadCustomProperties(extra_props);

	string annotations = a_model.getAGSetProperty("annotations");
	bool transcription = (annotations.find("transcription") != string::npos);
	bool namedEntities = (annotations.find("named") != string::npos);
	bool themes = (annotations.find("themes") != string::npos);

	StringOps transcriptionType(a_model.getGraphProperty("transcription_graph", "type"));
	transcriptionType.toLower();

	string language = a_model.getTranscriptionLanguage();
	string dialect = a_model.getTranscriptionDialect();

	Gtk::HBox* videA = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* mainHBox = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* videB = Gtk::manage(new Gtk::HBox());
	get_vbox()->pack_start(*videA, false, false, 3);
	get_vbox()->pack_start(*mainHBox, true, true, 3);
	get_vbox()->pack_start(*videB, false, false, 3);
	videA->show();
	mainHBox->show();
	videB->show();

	Gtk::HBox* videC = Gtk::manage(new Gtk::HBox());
	Gtk::Notebook* notebook = Gtk::manage(new Gtk::Notebook());
	Gtk::HBox* videD = Gtk::manage(new Gtk::HBox());
	mainHBox->pack_start(*videC, false, false, 3);
	mainHBox->pack_start(*notebook, true, true, 3);
	mainHBox->pack_start(*videD, false, false, 3);
	videC->show();
	notebook->show();
	videD->show();

	Gtk::VBox* note1 = Gtk::manage(new Gtk::VBox());
	Gtk::VBox* note3 = Gtk::manage(new Gtk::VBox());
	Gtk::VBox* note2 = Gtk::manage(new Gtk::VBox());
	notebook->append_page(*note1, _("File properties"));
	notebook->append_page(*note3, _("Signals"));
	notebook->append_page(*note2, _("Statistics"));
	note1->show();
	note2->show();
	note3->show();

	Gtk::HBox* videA2 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* mainHBox2 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* videB2 = Gtk::manage(new Gtk::HBox());
	note1->pack_start(*videA2, false, false, 3);
	note1->pack_start(*mainHBox2, true, true, 3);
	note1->pack_start(*videB2, false, false, 3);
	videA2->show();
	mainHBox2->show();
	videB2->show();

	Gtk::HBox* videA9 = Gtk::manage(new Gtk::HBox());
	StatsWidget* stats = new StatsWidget(a_model, a_displayAnnotationTime);
	Gtk::HBox* videB9 = Gtk::manage(new Gtk::HBox());
	note2->pack_start(*videA9, false, false, 3);
	note2->pack_start(*stats, false, false, 3);
	note2->pack_start(*videB9, false, false, 3);
	videA9->show();
	stats->show();
	videB9->show();

	Gtk::HBox* videC2 = Gtk::manage(new Gtk::HBox());
	Gtk::VBox* mainVBox = Gtk::manage(new Gtk::VBox());
	Gtk::HBox* videD2 = Gtk::manage(new Gtk::HBox());
	mainHBox2->pack_start(*videC2, false, false, 3);
	mainHBox2->pack_start(*mainVBox, true, true, 3);
	mainHBox2->pack_start(*videD2, false, false, 3);
	videC2->show();
	mainVBox->show();
	videD2->show();

	Gtk::HBox* mainHBox3 = NULL;
	Gtk::Frame* identificationFrame = Gtk::manage(new Gtk::Frame(
	        _("Identification")));
	identificationFrame->set_shadow_type(Gtk::SHADOW_IN);
	Gtk::VBox* identificationFrameVBox = Gtk::manage(new Gtk::VBox());
	Gtk::HBox* identificationFrameHBox1 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* identificationFrameHBox11 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* identificationFrameHBox2 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* identificationFrameHBox3 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* identificationFrameHBox4 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* identificationFrameHBox5 = Gtk::manage(new Gtk::HBox());
	identificationFrame->add(*identificationFrameVBox);
	identificationFrameVBox->pack_start(*identificationFrameHBox11, false,
	        false, 3);
	identificationFrameVBox->pack_start(*identificationFrameHBox1, false,
	        false, 3);
	identificationFrameVBox->pack_start(*identificationFrameHBox2, false,
	        false, 3);
	identificationFrameVBox->pack_start(*identificationFrameHBox3, false,
	        false, 3);
	identificationFrameVBox->pack_start(*identificationFrameHBox4, false,
	        false, 3);
	identificationFrameVBox->pack_start(*identificationFrameHBox5, false,
	        false, 3);
	if (mainHBox3 != NULL)
		identificationFrameVBox->pack_start(*mainHBox3, false, false, 3);
	identificationFrameVBox->show();
	identificationFrameHBox1->show();
	identificationFrameHBox11->show();
	identificationFrameHBox2->show();
	identificationFrameHBox3->show();
	identificationFrameHBox4->show();
	identificationFrameHBox5->show();

	Gtk::Label* file = Gtk::manage(new Gtk::Label(string(_("File")) + " :"));
	file->set_text(file->get_text() + "                             ");
	Gtk::Entry* fileEntry = Gtk::manage(new Gtk::Entry());
	fileEntry->set_text(a_values["file"]);
	fileEntry->set_sensitive(FALSE);
	fileEntry->set_editable(a_editable);

	Gtk::Label* corpus =
	        Gtk::manage(new Gtk::Label(string(_("Corpus")) + " :"));
	corpusEntry = Gtk::manage(new Gtk::Entry());
	corpusEntry->set_text(a_values["corpus"]);
	corpusEntry->set_sensitive(true);
	corpusEntry->set_editable(a_editable);

	Gtk::Label* corpusVersion = Gtk::manage(new Gtk::Label(string(
	        _("Corpus version")) + " :"));
	corpusVersionEntry = Gtk::manage(new Gtk::Entry());
	corpusVersionEntry->set_text(a_values["corpus_version"]);
	corpusVersionEntry->set_sensitive(true);
	corpusVersionEntry->set_editable(a_editable);

	Gtk::Label* version = Gtk::manage(new Gtk::Label(string(_("Version"))
	        + " :"));
	version->set_text(version->get_text() + "                             ");
	a_versionEntry = Gtk::manage(new Gtk::Entry());
	a_versionEntry->set_text(a_values["version"]);
	Gtk::Button* more = Gtk::manage(new Gtk::Button(_("More")));
	more->set_label("  " + more->get_label() + "  ");
	more->signal_clicked().connect(sigc::mem_fun(*this,
	        &DialogFileProperties::onMore));
	more->set_sensitive(a_versionList.size() > 0);

	Gtk::HBox* dec1 = Gtk::manage(new Gtk::HBox());
	Gtk::Label* creation = Gtk::manage(new Gtk::Label(string(_("Creation"))
	        + " :"));
	creation->set_text(creation->get_text() + "                             ");

	a_creationEntry = new FieldEntry(3, "/", 2);

	Gtk::Label* by = Gtk::manage(new Gtk::Label(string(_("By")) + " :"));
	a_byEntry = Gtk::manage(new Gtk::Entry());
	a_byEntry->set_width_chars(11);
	Gtk::Label* comment = Gtk::manage(new Gtk::Label(string(_("Comment"))
	        + " :"));
	a_tbuffer = Gtk::TextBuffer::create();
	a_commentEntryTV = Gtk::manage(new Gtk::TextView(a_tbuffer));
	Gtk::Frame* commentEntry = Gtk::manage(new Gtk::Frame());
	commentEntry->set_shadow_type(Gtk::SHADOW_IN);
	commentEntry->add(*a_commentEntryTV);
	a_commentEntryTV->show();

	Gtk::HBox* dec2 = Gtk::manage(new Gtk::HBox());
	Gtk::Label* lastMod = Gtk::manage(new Gtk::Label(string(
	        _("Last modification")) + " :"));
	lastMod->set_text(lastMod->get_text() + "                             ");
	a_lastModEntry = new FieldEntry(3, "/", 2);
	Gtk::Label* by2 = Gtk::manage(new Gtk::Label(string(_("By")) + " :"));
	a_byEntry2 = Gtk::manage(new Gtk::Entry());
	a_byEntry2->set_width_chars(11);
	Gtk::Label* comment2 = Gtk::manage(new Gtk::Label(string(_("Comment"))
	        + " :"));
	a_tbuffer2 = Gtk::TextBuffer::create();
	a_commentEntryTV2 = Gtk::manage(new Gtk::TextView(a_tbuffer2));
	Gtk::Frame* commentEntry2 = Gtk::manage(new Gtk::Frame());
	commentEntry2->set_shadow_type(Gtk::SHADOW_IN);
	commentEntry2->add(*a_commentEntryTV2);
	a_commentEntryTV2->show();

	updateVersions();

	Gtk::Label* comment3 = Gtk::manage(new Gtk::Label(string(_("Comment"))
	        + " :"));
	comment3->set_text(comment3->get_text() + "                             ");
	a_tbuffer3 = Gtk::TextBuffer::create();
	a_tbuffer3->set_text(a_values["comment"]);
	a_commentEntryTV3 = Gtk::manage(new Gtk::TextView(a_tbuffer3));
	Gtk::Frame* commentEntry3 = Gtk::manage(new Gtk::Frame());
	commentEntry3->set_shadow_type(Gtk::SHADOW_IN);
	commentEntry3->add(*a_commentEntryTV3);
	a_commentEntryTV3->show();

	identificationFrameHBox11->pack_start(*corpus, false, false, 3);
	identificationFrameHBox11->pack_start(*corpusEntry, false, false, 4);
	identificationFrameHBox11->pack_start(*corpusVersion, false, false, 4);
	identificationFrameHBox11->pack_start(*corpusVersionEntry, false, false, 4);
	identificationFrameHBox1->pack_start(*file, false, false, 3);
	identificationFrameHBox1->pack_start(*fileEntry, true, true, 3);
	identificationFrameHBox2->pack_start(*version, false, false, 3);
	identificationFrameHBox2->pack_start(*a_versionEntry, true, true, 3);
	identificationFrameHBox2->pack_start(*more, false, false, 3);
	identificationFrameHBox3->pack_start(*dec1, false, false, 3);
	identificationFrameHBox3->pack_start(*creation, false, false, 3);
	identificationFrameHBox3->pack_start(*a_creationEntry, false, false, 3);
	identificationFrameHBox3->pack_start(*by, false, false, 3);
	identificationFrameHBox3->pack_start(*a_byEntry, false, false, 3);
	identificationFrameHBox3->pack_start(*comment, false, false, 3);
	identificationFrameHBox3->pack_start(*commentEntry, true, true, 3);
	identificationFrameHBox4->pack_start(*dec2, false, false, 3);
	identificationFrameHBox4->pack_start(*lastMod, false, false, 3);
	identificationFrameHBox4->pack_start(*a_lastModEntry, false, false, 3);
	identificationFrameHBox4->pack_start(*by2, false, false, 3);
	identificationFrameHBox4->pack_start(*a_byEntry2, false, false, 3);
	identificationFrameHBox4->pack_start(*comment2, false, false, 3);
	identificationFrameHBox4->pack_start(*commentEntry2, true, true, 3);
	identificationFrameHBox5->pack_start(*comment3, false, false, 3);
	identificationFrameHBox5->pack_start(*commentEntry3, true, true, 3);

	corpusVersion->show();
	corpusVersionEntry->show();
	file->show();
	fileEntry->show();
	corpus->show();
	corpusEntry->show();
	version->show();
	a_versionEntry->show();
	more->show();
	dec1->show();
	creation->show();
	a_creationEntry->show();
	by->show();
	a_byEntry->show();
	comment->show();
	commentEntry->show();
	dec2->show();
	lastMod->show();
	a_lastModEntry->show();
	by2->show();
	a_byEntry2->show();
	comment2->show();
	commentEntry2->show();
	comment3->show();
	commentEntry3->show();

	file->set_size_request(90, -1);
	version->set_size_request(90, -1);
	dec1->set_size_request(30, -1);
	creation->set_size_request(110, -1);
	dec2->set_size_request(30, -1);
	lastMod->set_size_request(110, -1);
	comment3->set_size_request(90, -1);
	corpusEntry->set_width_chars(15);
	a_versionEntry->set_width_chars(5);
	a_creationEntry->set_width_chars(11);
	a_lastModEntry->set_width_chars(11);

	mainVBox->pack_start(*identificationFrame, true, true, 6);
	identificationFrame->show();

	Gtk::HBox* fprop = showFileProperties();
	if (fprop != NULL)
		mainVBox->pack_start(*fprop);

	a_place = Gtk::manage(new Gtk::HBox());
	note3->pack_start(*a_place, false, false, 6);
	//mainVBox->pack_start(*a_place, false, false, 6);
	a_place->show();
	Gtk::HBox* a_in = drawSignalPage();
	a_place->pack_start(*a_in, true, true, 0);
	a_in->show();

	Gtk::HBox* addSignalBox = Gtk::manage(new Gtk::HBox());
	a_addSignal = Gtk::manage(new Gtk::Button(_("Add signal")));
	a_addSignal->signal_clicked().connect(sigc::mem_fun(*this,
	        &DialogFileProperties::onAddSignal));
	a_addSignal->set_label("  " + a_addSignal->get_label() + "  ");
	//Gtk::HBox* removeSignalBox = Gtk::manage(new Gtk::HBox());
	a_removeSignal = Gtk::manage(new Gtk::Button(_("Remove signal")));
	a_removeSignal->signal_clicked().connect(sigc::mem_fun(*this,
	        &DialogFileProperties::onRemoveSignal));

	// PROVISOIRE
	a_addSignal->set_sensitive(false);
	a_removeSignal->set_sensitive(false);
	// FIN PROVISOIRE

	a_removeSignal->set_label("  " + a_removeSignal->get_label() + "  ");
	Gtk::HBox* vide = Gtk::manage(new Gtk::HBox());
	addSignalBox->pack_start(*vide, true, true, 0);
	addSignalBox->pack_start(*a_removeSignal, false, false, 0);
	addSignalBox->pack_start(*a_addSignal, false, false, 3);
	vide->show();
	if (a_signals.size() > 1)
		a_removeSignal->show();
	a_addSignal->show();
	note3->pack_start(*addSignalBox, false, false, 0);
	//mainVBox->pack_start(*addSignalBox, false, false, 0);
	addSignalBox->show();

	Gtk::Frame* annotationsFrame =
	        Gtk::manage(new Gtk::Frame(_("Annotations")));
	annotationsFrame->set_shadow_type(Gtk::SHADOW_IN);

	Gtk::VBox* annotationsFrameVBox = Gtk::manage(new Gtk::VBox());
	Gtk::HBox* annotationsFrameHBox0 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* annotationsFrameHBox1 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* annotationsFrameHBox2 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* annotationsFrameHBox3 = Gtk::manage(new Gtk::HBox());
	annotationsFrame->add(*annotationsFrameVBox);
	annotationsFrameVBox->pack_start(*annotationsFrameHBox0,
	        Gtk::PACK_EXPAND_WIDGET, 3);
	annotationsFrameVBox->pack_start(*annotationsFrameHBox1,
	        Gtk::PACK_EXPAND_WIDGET, 3);
	annotationsFrameVBox->pack_start(*annotationsFrameHBox2,
	        Gtk::PACK_EXPAND_WIDGET, 3);
	annotationsFrameVBox->pack_start(*annotationsFrameHBox3,
	        Gtk::PACK_EXPAND_WIDGET, 3);
	annotationsFrameVBox->show();
	annotationsFrameHBox0->show();
	annotationsFrameHBox1->show();
	annotationsFrameHBox2->show();
	annotationsFrameHBox3->show();

	showConventions(annotationsFrameHBox0);

	a_transcription = Gtk::manage(new Gtk::CheckButton(string(
	        _("Transcription")) + " :"));
	a_transcription->set_active(transcription);
	a_transcription->signal_toggled().connect(sigc::mem_fun(*this,
	        &DialogFileProperties::updateGUI));
	a_transcriptionCombo = Gtk::manage(new Gtk::ComboBoxText());
	a_transcriptionCombo->append_text("");
	a_transcriptionCombo->append_text(_("Fast"));
	a_transcriptionCombo->append_text(_("Detailed"));
	a_transcriptionCombo->append_text(_("Automatic"));
	if (transcriptionType.contains("fast") )
		a_transcriptionCombo->set_active(1);
	else if (transcriptionType.contains("detailed") )
		a_transcriptionCombo->set_active(2);
	else if (transcriptionType.contains("automatic") )
		a_transcriptionCombo->set_active(3);

	a_language = Gtk::manage(new Gtk::Label(string(_("Language")) + " :"));
	a_languageCombo = Gtk::manage(new Gtk::ComboBoxText());
	a_languageCombo->append_text("");

	a_languages = Languages::getInstance();

	std::vector<Glib::ustring> names = a_languages->get_names();
	std::vector<Glib::ustring> dialects = a_languages->get_dialects("");

	std::vector<Glib::ustring>::iterator itC = names.begin();
	int i1 = 1;
	while (itC != names.end())
	{
		Glib::ustring name = *itC;
		Glib::ustring code = a_languages->get_code(name);
		a_languageCombo->append_text(name);
		if (language == code)
			a_languageCombo->set_active(i1);
		itC++;
		i1++;
	}

	a_dialect = Gtk::manage(new Gtk::Label(string(_("Dialect")) + " :"));
	a_dialectCombo = Gtk::manage(new Gtk::ComboBoxText());
	a_dialectCombo->append_text("");

	std::vector<Glib::ustring>::iterator itB = dialects.begin();
	int i2 = 1;
	while (itB != dialects.end())
	{
		Glib::ustring dial = *itB;
		a_dialectCombo->append_text(dial.c_str());
		if (strcasecmp(dialect.c_str(), dial.c_str()) == 0)
			a_dialectCombo->set_active(i2);
		itB++;
		i2++;
	}

	a_namedEntities = Gtk::manage(new Gtk::CheckButton(_("Named entities")));
	a_namedEntities->set_active(namedEntities);
	a_themes = Gtk::manage(new Gtk::CheckButton(_("Themes")));
	a_themes->set_active(themes);

	annotationsFrameHBox1->pack_start(*a_transcription, false, false, 3);
	annotationsFrameHBox1->pack_start(*a_transcriptionCombo, false, false, 3);
	annotationsFrameHBox1->pack_start(*a_language, false, false, 3);
	annotationsFrameHBox1->pack_start(*a_languageCombo, true, true, 3);
	annotationsFrameHBox1->pack_start(*a_dialect, false, false, 3);
	annotationsFrameHBox1->pack_start(*a_dialectCombo, true, true, 3);
	annotationsFrameHBox2->pack_start(*a_namedEntities, false, false, 3);
	annotationsFrameHBox3->pack_start(*a_themes, false, false, 3);

	a_transcription->show();
	a_transcriptionCombo->show();
	a_language->show();
	a_languageCombo->show();
	a_dialect->show();
	a_dialectCombo->show();
	a_namedEntities->show();
	a_themes->show();

	a_transcription->set_size_request(110, -1);
	a_transcriptionCombo->set_size_request(100, -1);
	a_namedEntities->set_size_request(216, -1);
	a_themes->set_size_request(216, -1);

	mainVBox->pack_start(*annotationsFrame, true, true, 6);
	annotationsFrame->show();

	Gtk::Button* ok = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	Gtk::Button* cancel = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

	updateGUI();

	set_editability(a_editable);
}

DialogFileProperties::~DialogFileProperties()
{
	if (a_lastModEntry)
		delete (a_lastModEntry);
	if (a_creationEntry)
		delete (a_creationEntry);
}

void DialogFileProperties::on_response(int p_id)
{
	int i = 0;
	if (p_id == Gtk::RESPONSE_OK)
	{
		// -- Save corpus data
		string corpusName = corpusEntry->get_text();
		if (corpusName != a_values["corpus"])
			a_model.setCorpusName(corpusName);

		string corpusVersion = corpusVersionEntry->get_text();
		if (corpusVersion != a_values["corpus_version"])
			;
		a_model.setCorpusVersion(corpusVersion);

		// -- Save version data
		if (a_versionList.size() > 0)
		{
			Version& firstVersion = a_versionList.front();
			Version& lastVersion = a_versionList.back();

			firstVersion.setDate(getDate(a_creationEntry));
			firstVersion.setAuthor(a_byEntry->get_text());
			firstVersion.setComment(a_tbuffer->get_text());

			if (a_versionList.size() > 1)
			{
				lastVersion.setDate(getDate(a_lastModEntry));
				lastVersion.setAuthor(a_byEntry2->get_text());
				lastVersion.setComment(a_tbuffer2->get_text());
			}
			lastVersion.setId(a_versionEntry->get_text());
		}
		a_model.setVersionList(a_versionList);
		a_model.setTranscriptionProperty("comment", a_tbuffer3->get_text());

		// -- Transcription property
		list<Property>::iterator it3 = a_properties.begin();
		i = 0;
		while (it3 != a_properties.end())
		{
			Property& p = *it3;
			if (p.type == PROPERTY_TEXT)
			{
				Gtk::Entry* entry = (Gtk::Entry*) (a_entries[i]);
				a_model.setTranscriptionProperty(string("property,") + p.label,
				        entry->get_text());
			}
			else if (p.type == PROPERTY_CHOICELIST)
			{
				Gtk::ComboBoxText* combo = (Gtk::ComboBoxText*) (a_entries[i]);

				const std::map<string, string>& layout =
				        a_model.conventions().getLayout("Labels");
				std::map<string, string>::const_iterator itl;
				string val = combo->get_active_text();
				for (itl = layout.begin(); itl != layout.end(); ++itl)
				{
					if ((itl->first.compare(0, p.choiceList.length(),
					        p.choiceList) == 0) && val == itl->second)
					{
						int pos = itl->first.find(",");
						val = itl->first.substr(pos + 1);
					}
				}
				a_model.setTranscriptionProperty(string("property,") + p.label,
				        val);
			}
			it3++;
			i++;
		}

		// -- Signal data
		a_signals.clear();
		a_signals[a_signal1ID] = a_signal1;
		a_signal1["type"] = a_typeEntrySignal1->get_text();
		a_signal1["source"] = a_sourceEntrySignal1->get_text();
		a_signal1["date"] = a_dateEntrySignal1->get_text();
		a_signal1["comment"] = a_tbufferSignal1->get_text();
		a_model.setSignalProperty(a_signal1ID, "type", a_signal1["type"]);
		a_model.setSignalProperty(a_signal1ID, "source", a_signal1["source"]);
		a_model.setSignalProperty(a_signal1ID, "date", a_signal1["date"]);
		a_model.setSignalProperty(a_signal1ID, "comment", a_signal1["comment"]);

		if (a_signalsCount == 2)
		{
			a_signals[a_signal2ID] = a_signal2;
			a_signal2["type"] = a_typeEntrySignal2->get_text();
			a_signal2["source"] = a_sourceEntrySignal2->get_text();
			a_signal2["date"] = a_dateEntrySignal2->get_text();
			a_signal2["comment"] = a_tbufferSignal2->get_text();
			a_model.setSignalProperty(a_signal2ID, "type", a_signal2["type"]);
			a_model.setSignalProperty(a_signal2ID, "source",
			        a_signal2["source"]);
			a_model.setSignalProperty(a_signal2ID, "date", a_signal2["date"]);
			a_model.setSignalProperty(a_signal2ID, "comment",
			        a_signal2["comment"]);
		}

		// -- Transcription data
		string annotations = "";
		if (a_transcription->get_active())
			annotations = "transcription";
		if (a_namedEntities->get_active())
		{
			if (a_transcription->get_active())
				annotations = annotations + ";";
			annotations = annotations + "named entities";
		}
		if (a_themes->get_active())
		{
			if (a_transcription->get_active() || a_namedEntities->get_active())
				annotations = annotations + ";";
			annotations = annotations + "themes";
		}
		a_model.setAGSetProperty("annotations", annotations);

		// -- Transcription language
		if (strcmp(a_languageCombo->get_active_text().c_str(), "") == 0)
			a_model.setTranscriptionLanguage(a_languageCombo->get_active_text());
		else
		{
			std::string code = a_languages->get_code(
			        a_languageCombo->get_active_text());
			a_model.setTranscriptionLanguage(code);
		}
		a_model.setTranscriptionDialect(a_dialectCombo->get_active_text());

		// -- Single signal display
		if (a_model.getSignalCfg().canSingleSignal())
		{
			// mode must be changed
			if (a_model.getSignalCfg().isSingleSignal()
			        != checkb_singleSignal.get_active())
			{
				a_model.setSingleSignal(checkb_singleSignal.get_active());
				need_reload = true;
			}
		}
		need_save = true;
		a_model.setUpdated(true);
	}
	saveGeoAndHide();
}

void DialogFileProperties::updateGUI()
{

	bool b = a_transcription->get_active();
	a_transcriptionCombo->set_sensitive(b);
	a_language->set_sensitive(b);
	a_languageCombo->set_sensitive(b);
	a_dialect->set_sensitive(b);
	a_dialectCombo->set_sensitive(b);

}

void DialogFileProperties::onMore()
{

	tag::DialogMore* dialog = new tag::DialogMore(*a_parent, &a_versionList,
	        a_displayAnnotationTime, a_editable);
	int r = dialog->run();
	dialog->hide();
	if (r == Gtk::RESPONSE_OK)
		updateVersions();
	delete dialog;
}

void DialogFileProperties::onAddSignal()
{

	if (a_signalsCount == 2)
		return;
	bool close = false;

	Glib::ustring s = dlg::selectAudioFile(close, ".", "Add signal", a_parent);
	if (s == "" || close)
		return;

	a_removeSignal->show();
	a_signal2.clear();

	IODevice* device = Guesser::open(s.c_str());
	if (device)
	{
		a_signal2["encoding"] = device->m_info()->audio_encoding;
		a_signal2["format"] = device->m_info()->audio_codec;

		int channels = device->m_info()->audio_channels;
		if (channels == 1)
			a_signal2["channels"] = "Mono";
		else
			a_signal2["channels"] = "Stereo";

		double secs = device->m_info()->audio_duration;
		int H = (int) (secs / 3600.0);
		int M = ((int) (secs / 60.0)) % 60;
		int S = ((int) secs) % 60;
		char tmp[80];
		sprintf(tmp, "%.2d:%.2d:%.2d", H, M, S);
		a_signal2["length"] = tmp;
		a_signalsCount = 2;

		vector<std::string> vec = a_model.addSignal(s, "audio",
		        a_signal2["format"], a_signal2["encoding"], channels);
		a_signal2ID = vec[0];

		device->m_close();
		delete (device);
	}
	else
	{
		a_signal2["encoding"] = "unspecified";
		a_signal2["channels"] = "unspecified";
		a_signal2["format"] = "unspecified";
	}
	a_signal2["file"] = s;
	a_signal2["type"] = "";
	a_signal2["source"] = "";
	a_signal2["date"] = "";
	a_signal2["comment"] = "";

	a_model.setSignalProperty(a_signal2ID, "length", a_signal2["length"]);

	Glib::ListHandle<Widget*> children = a_place->get_children();
	Glib::ListHandle<Widget*>::const_iterator it = children.begin();
	a_place->remove(**it);

	Gtk::HBox* a_in = drawSignalPage();
	a_place->pack_start(*a_in, true, true, 0);
	a_in->show();

	a_notebook->set_current_page(1);

}

void DialogFileProperties::onRemoveSignal()
{

	a_removeSignal->hide();

	if (a_notebook->get_current_page() == 0)
		a_model.removeSignal(a_signal1ID, true);
	else
		a_model.removeSignal(a_signal2ID, true);

	a_signalsCount = 1;
	if (a_notebook->get_current_page() == 0)
		a_signal1 = a_signal2;

	Glib::ListHandle<Widget*> children = a_place->get_children();
	Glib::ListHandle<Widget*>::const_iterator it = children.begin();
	a_place->remove(**it);

	Gtk::HBox* a_in = drawSignalPage();
	a_place->pack_start(*a_in, true, true, 0);
	a_in->show();

}

Gtk::HBox* DialogFileProperties::drawSignalPage()
{
	if (a_signalsCount == 1)
	{
		Gtk::HBox* box = Gtk::manage(new Gtk::HBox());
		Gtk::VBox* in = Gtk::manage(new Gtk::VBox());

		// generic properties
		Gtk::VBox* propertiesVBox = drawSignalProperties(1);
		Gtk::Frame* propertiesFrame = Gtk::manage(new Gtk::Frame(
		        _("Properties")));
		propertiesFrame->add(*propertiesVBox);
		in->pack_start(*propertiesFrame, true, true, 3);

		// signal presentation
		Gtk::VBox* channelVBox = drawChannelsProperties();
		Gtk::Frame* channelFrame = Gtk::manage(new Gtk::Frame(
		        _("Channels display")));
		channelFrame->add(*channelVBox);
		in->pack_start(*channelFrame, true, true, 3);
		in->show_all_children(true);
		in->show();

		box->pack_start(*in, true, true, 8);
		box->show_all_children(true);
		box->show();

		return box;
	}
	else if (a_signalsCount == 2)
	{
		Gtk::HBox* box = Gtk::manage(new Gtk::HBox());

		Gtk::VBox* in = Gtk::manage(new Gtk::VBox());
		a_notebook = Gtk::manage(new Gtk::Notebook());
		Gtk::VBox* note1 = drawSignalProperties(1);
		Gtk::VBox* note2 = drawSignalProperties(2);
		a_notebook->append_page(*note1, _("Signal 1"));
		a_notebook->append_page(*note2, _("Signal 2"));
		note1->show();
		note2->show();
		in->pack_start(*a_notebook, true, true, 3);
		a_notebook->show();

		box->pack_start(*in, true, true, 8);
		box->show_all_children(true);
		box->show();

		return box;
	}
	return NULL;
}

Gtk::VBox* DialogFileProperties::drawSignalProperties(int p_signal)
{
	std::map<string, string> signal = a_signal1;
	if (p_signal == 2)
		signal = a_signal2;

	Gtk::VBox* signalFrameVBox = Gtk::manage(new Gtk::VBox());
	Gtk::HBox* signalFrameHBox1 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* signalFrameHBox2 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* signalFrameHBox3 = Gtk::manage(new Gtk::HBox());
	Gtk::HBox* signalFrameHBox4 = Gtk::manage(new Gtk::HBox());
	signalFrameVBox->pack_start(*signalFrameHBox1, false, false, 3);
	signalFrameVBox->pack_start(*signalFrameHBox2, false, false, 3);
	signalFrameVBox->pack_start(*signalFrameHBox3, false, false, 3);
	signalFrameVBox->pack_start(*signalFrameHBox4, false, false, 3);
	signalFrameVBox->show();
	signalFrameHBox1->show();
	signalFrameHBox2->show();
	signalFrameHBox3->show();
	signalFrameHBox4->show();

	Gtk::Label* file2 = Gtk::manage(new Gtk::Label(string(_("File")) + " :"));
	file2->set_text(file2->get_text() + "                             ");
	Gtk::Entry* file2Entry = Gtk::manage(new Gtk::Entry());
	file2Entry->set_text(signal["file"]);
	file2Entry->set_sensitive(FALSE);
	Gtk::Label* format =
	        Gtk::manage(new Gtk::Label(string(_("Format")) + " :"));
	format->set_text(format->get_text() + "                             ");
	Gtk::Entry* formatEntry = Gtk::manage(new Gtk::Entry());
	formatEntry->set_text(signal["format"]);
	formatEntry->set_sensitive(FALSE);
	Gtk::Label* encoding = Gtk::manage(new Gtk::Label(string(_("Encoding"))
	        + " :"));
	Gtk::Entry* encodingEntry = Gtk::manage(new Gtk::Entry());
	encodingEntry->set_text(signal["encoding"]);
	encodingEntry->set_sensitive(FALSE);
	Gtk::Label* channels = Gtk::manage(new Gtk::Label(string(_("Channels"))
	        + " :"));
	Gtk::Entry* channelsEntry = Gtk::manage(new Gtk::Entry());
	channelsEntry->set_text(signal["channels"]);
	channelsEntry->set_sensitive(FALSE);

	Gtk::Label* length =
	        Gtk::manage(new Gtk::Label(string(_("Length")) + " :"));
	Gtk::Entry* lengthEntry = Gtk::manage(new Gtk::Entry);
	lengthEntry->set_sensitive(a_editable);
	lengthEntry->set_editable(a_editable);
	string siglen = signal["length"];
	lengthEntry->set_text(siglen);

	//	Gtk::Entry* lengthSecondsEntry = Gtk::manage(new Gtk::Entry());
	//	sprintf(str, "%.3d", S2);
	//	lengthSecondsEntry->set_text(string(str));
	//	lengthSecondsEntry->set_sensitive(FALSE);
	//	lengthSecondsEntry->set_width_chars(4);

	lengthEntry->set_sensitive(FALSE);
	lengthEntry->set_width_chars(11);

	Gtk::Label* type = Gtk::manage(new Gtk::Label(string(_("Type")) + " :"));
	type->set_text(type->get_text() + "                             ");
	Gtk::Entry* typeEntry = Gtk::manage(new Gtk::Entry());
	typeEntry->set_sensitive(a_editable);
	typeEntry->set_editable(a_editable);
	typeEntry->set_text(signal["type"]);
	typeEntry->set_width_chars(7);
	Gtk::Label* source =
	        Gtk::manage(new Gtk::Label(string(_("Source")) + " :"));
	Gtk::Entry* sourceEntry = Gtk::manage(new Gtk::Entry());
	sourceEntry->set_text(signal["source"]);
	sourceEntry->set_width_chars(22);
	Gtk::Label* date = Gtk::manage(new Gtk::Label(string(_("Date")) + " :"));
	sourceEntry->set_sensitive(a_editable);
	sourceEntry->set_editable(a_editable);

	/*
	 view::FieldEntry* dateEntry = Gtk::manage(new view::FieldEntry(3, 2, '/'));
	 str[2] = '\0';
	 string s = signal["date"];
	 str[0] = s[0];
	 str[1] = s[1];
	 dateEntry->SetFieldText(0, str);
	 str[0] = s[3];
	 str[1] = s[4];
	 dateEntry->SetFieldText(1, str);
	 str[0] = s[6];
	 str[1] = s[7];
	 dateEntry->SetFieldText(2, str);
	 */
	Gtk::Entry* dateEntry = Gtk::manage(new Gtk::Entry());
	dateEntry->set_text(signal["date"]);
	dateEntry->set_editable(a_editable);
	dateEntry->set_sensitive(a_editable);

	Gtk::Label* comment4 = Gtk::manage(new Gtk::Label(string(_("Comment"))
	        + " :"));
	comment4->set_text(comment4->get_text() + "                             ");
	Glib::RefPtr<Gtk::TextBuffer> tbuffer = Gtk::TextBuffer::create();
	tbuffer->set_text(signal["comment"]);
	Gtk::TextView* commentEntryTV = Gtk::manage(new Gtk::TextView(tbuffer));
	Gtk::Frame* comment4Entry = Gtk::manage(new Gtk::Frame());
	comment4Entry->set_shadow_type(Gtk::SHADOW_IN);
	comment4Entry->add(*commentEntryTV);
	commentEntryTV->show();
	commentEntryTV->set_sensitive(a_editable);
	commentEntryTV->set_editable(a_editable);

	signalFrameHBox1->pack_start(*file2, false, false, 3);
	signalFrameHBox1->pack_start(*file2Entry, true, true, 3);
	signalFrameHBox2->pack_start(*format, false, false, 3);
	signalFrameHBox2->pack_start(*formatEntry, true, true, 3);
	signalFrameHBox2->pack_start(*encoding, false, false, 3);
	signalFrameHBox2->pack_start(*encodingEntry, true, true, 3);
	signalFrameHBox2->pack_start(*channels, false, false, 3);
	signalFrameHBox2->pack_start(*channelsEntry, true, true, 3);
	signalFrameHBox2->pack_start(*length, false, false, 3);
	signalFrameHBox2->pack_start(*lengthEntry, false, false, 3);
	//	signalFrameHBox2->pack_start(*lengthSecondsEntry, false, false, 3);
	signalFrameHBox3->pack_start(*type, false, false, 3);
	signalFrameHBox3->pack_start(*typeEntry, false, false, 3);
	signalFrameHBox3->pack_start(*source, false, false, 3);
	signalFrameHBox3->pack_start(*sourceEntry, false, false, 3);
	signalFrameHBox3->pack_start(*date, false, false, 3);
	signalFrameHBox3->pack_start(*dateEntry, true, true, 3);
	signalFrameHBox4->pack_start(*comment4, false, false, 3);
	signalFrameHBox4->pack_start(*comment4Entry, true, true, 3);

	std::map<string, string>::iterator itB = signal.begin();
	while (itB != signal.end())
	{
		string s1 = itB->first;
		string s2 = itB->second;
		bool trouve = false;
		list<string>::iterator itC = SIGNAL_PROPERTIES.begin();
		while (itC != SIGNAL_PROPERTIES.end() && !trouve)
		{
			string s3 = *itC;
			if (s1 == s3)
				trouve = true;
			else
				itC++;
		}
		if (!trouve)
		{
			Gtk::HBox* signalFrameHBox5 = Gtk::manage(new Gtk::HBox());
			signalFrameVBox->pack_start(*signalFrameHBox5, false, false, 3);
			signalFrameHBox5->show();
			Gtk::Label* property2 = Gtk::manage(new Gtk::Label(s1 + " :"));
			property2->set_text(property2->get_text()
			        + "                             ");
			Gtk::Entry* property2Entry = Gtk::manage(new Gtk::Entry());
			property2Entry->set_text(s2);
			signalFrameHBox5->pack_start(*property2, false, false, 3);
			signalFrameHBox5->pack_start(*property2Entry, true, true, 3);
			property2->show();
			property2Entry->show();
			property2->set_size_request(85, -1);
			property2Entry->set_sensitive(a_editable);
			property2Entry->set_editable(a_editable);
		}
		itB++;
	}

	file2->show();
	file2Entry->show();
	format->show();
	formatEntry->show();
	encoding->show();
	encodingEntry->show();
	channels->show();
	channelsEntry->show();
	length->show();
	lengthEntry->show();
	//	lengthSecondsEntry->show();
	type->show();
	typeEntry->show();
	source->show();
	sourceEntry->show();
	date->show();
	dateEntry->show();
	comment4->show();
	comment4Entry->show();

	file2->set_size_request(85, -1);
	format->set_size_request(85, -1);
	type->set_size_request(85, -1);
	comment4->set_size_request(85, -1);
	formatEntry->set_width_chars(6);
	encodingEntry->set_width_chars(6);
	channelsEntry->set_width_chars(9);
	dateEntry->set_width_chars(12);

	if (p_signal == 1)
	{
		a_typeEntrySignal1 = typeEntry;
		a_sourceEntrySignal1 = sourceEntry;
		a_dateEntrySignal1 = dateEntry;
		a_tbufferSignal1 = tbuffer;
	}
	else if (p_signal == 2)
	{
		a_typeEntrySignal2 = typeEntry;
		a_sourceEntrySignal2 = sourceEntry;
		a_dateEntrySignal2 = dateEntry;
		a_tbufferSignal2 = tbuffer;
	}

	return signalFrameVBox;
}

Gtk::VBox* DialogFileProperties::drawChannelsProperties()
{
	Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox());

	checkb_singleSignal.set_label(_("Use signal as a mono channel signal"));
	checkb_singleSignal.set_tooltip_text(
	        _("Force editor to display the file as a mono signal file"));
	bool is_single = a_model.getSignalCfg().isSingleSignal();
	checkb_singleSignal.set_active(is_single);
	checkb_singleSignal.signal_toggled().connect(sigc::mem_fun(*this,
	        &DialogFileProperties::onSingleSignalChanged));

	if (!a_model.getSignalCfg().canSingleSignal())
		checkb_singleSignal.set_sensitive(false);

	vbox->pack_start(checkb_singleSignal, false, false);
	return vbox;
}

void DialogFileProperties::onSingleSignalChanged()
{
	bool active = checkb_singleSignal.get_active();
	Glib::ustring msg = "";

	if (active)
	{
		msg = _("Only one track and its annotations will be visible, the others will be hidden.");
		msg.append("\n");
	}
	msg.append(_("File will be reloaded."));
}

void DialogFileProperties::updateVersions()
{
	if (a_versionList.size() > 0)
	{
		Version& firstVersion = a_versionList.front();
		Version& lastVersion = a_versionList.back();

		a_values["version"] = lastVersion.getId();
		a_values["creation_date"] = firstVersion.getDate();
		a_values["creation_by"] = firstVersion.getAuthor();
		a_values["creation_comment"] = firstVersion.getComment();
		if (a_versionList.size() > 1)
		{
			a_values["lastmodification_date"] = lastVersion.getDate();
			a_values["lastmodification_by"] = lastVersion.getAuthor();
			a_values["lastmodification_comment"] = lastVersion.getComment();
			a_versionEntry->set_text(a_values["version"]);
		}
	}

	displayDate(a_values["creation_date"], a_creationEntry);
	a_byEntry->set_text(a_values["creation_by"]);
	a_tbuffer->set_text(a_values["creation_comment"]);

	displayDate(a_values["lastmodification_date"], a_lastModEntry);
	a_byEntry2->set_text(a_values["lastmodification_by"]);
	a_tbuffer2->set_text(a_values["lastmodification_comment"]);

}

//
// load custom properties definition
//
void DialogFileProperties::loadCustomProperties(const string& path)
{
	//		string cfgfic = cfgdir+"/"+path+".rc";
	if (path.empty())
		return;
	if (!FileInfo(path).exists())
	{
		TRACE << "loadCustomProperties: file not found : " << path << endl;
		return;
	}

	TRACE << "loadCustomProperties from " << path << endl;

	try
	{
		XMLPlatformUtils::Initialize();

		std::map<string, std::list<Property> > annotationProperties;
		SAXAnnotationsHandler* handler2 = new SAXAnnotationsHandler(
		        &annotationProperties, &a_properties, &a_choiceLists);

		SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
		parser->setFeature(XMLUni::fgSAX2CoreValidation, false);
		parser->setFeature(XMLUni::fgXercesLoadExternalDTD, false);
		parser->setContentHandler((DefaultHandler*) handler2);
		parser->setErrorHandler((DefaultHandler*) handler2);

		//printf("cfgfic = %s\n", cfgfic.c_str());
		parser->parse(path.c_str());
		delete parser;
		delete handler2;
		XMLPlatformUtils::Terminate();
	}
	catch (const XMLException & toCatch)
	{
		TRACE << "XML Exception when loading " << path << " : ";
		a_properties.clear();
		return;
	}

	const std::map<string, string>& layout = a_model.conventions().getLayout(
	        "Labels");
	std::map<string, string>::const_iterator itl;

	list<Property>::iterator it2 = a_properties.begin();
	while (it2 != a_properties.end())
	{
		Property p = *it2;
		string s = "file_properties," + p.label;
		if ((itl = layout.find(s)) != layout.end())
			s = itl->second;
		else
			s = p.label;
		s += " :  ";
		a_labels.push_back(Gtk::manage(new Gtk::Label(s)));
		a_labels.back()->set_size_request(85, -1);
		string str = "property," + p.label;
		if (p.type == PROPERTY_TEXT)
		{
			a_entries.push_back(Gtk::manage(new Gtk::Entry()));
			((Gtk::Entry*) a_entries.back())->set_text(
			        a_model.getTranscriptionProperty(str));
			((Gtk::Entry*) a_entries.back())->set_sensitive(a_editable);
			((Gtk::Entry*) a_entries.back())->set_editable(a_editable);
		}
		else if (p.type == PROPERTY_CHOICELIST)
		{
			a_entries.push_back(Gtk::manage(new Gtk::ComboBoxText()));
			list<string> choices = a_choiceLists[p.choiceList];
			list<string>::iterator it3 = choices.begin();
			int current = -1;
			int ind2 = 0;

			while (it3 != choices.end())
			{
				string s = p.choiceList + string(",") + *it3;
				if ((itl = layout.find(s)) != layout.end())
					s = itl->second;
				else
					s = *it3;
				if (*it3 == a_model.getTranscriptionProperty(str))
					current = ind2;
				((Gtk::ComboBoxText*) a_entries.back())->append_text(s);
				it3++;
				ind2++;
			}
			((Gtk::ComboBoxText*) a_entries.back())->set_active(current);
		}
		it2++;
	}
}

void DialogFileProperties::displayDate(const string& s, FieldEntry* entry)
{

	int yy, mm, dd;
	bool ok = (sscanf(s.c_str(), "%d/%d/%d", &yy, &mm, &dd) == 3);
	if (ok)
	{
		char str[4];
		if (yy > 2000)
			yy -= 2000;
		else if (yy > 1900)
			yy -= 1900;
		sprintf(str, "%02d", yy);
		entry->set_element(0, str);
		sprintf(str, "%02d", mm);
		entry->set_element(1, str);
		sprintf(str, "%02d", dd);
		entry->set_element(2, str);
	}
}

string DialogFileProperties::getDate(FieldEntry* entry)
{
	int yy, mm, dd;
	string s = entry->get_element(0);
	if (s.empty())
		return "";
	yy = atoi(s.c_str());
	if (yy < 100)
		if (yy >= 90)
			yy += 1900;
		else
			yy += 2000;
	s = entry->get_element(1);
	mm = atoi(s.c_str());
	if (mm < 1 || mm > 12)
		mm = 1;
	s = entry->get_element(2);
	dd = atoi(s.c_str());
	if (dd < 1 || dd > 31)
		dd = 1;

	char buf[12];
	sprintf(buf, "%04d/%02d/%02d", yy, mm, dd);
	return buf;
}

Gtk::HBox* DialogFileProperties::showFileProperties()
{
	Gtk::HBox* mainHBox3 = NULL;
	if (a_properties.size() > 0)
	{
		mainHBox3 = Gtk::manage(new Gtk::HBox());
		Gtk::Frame* filePropertiesFrame = Gtk::manage(new Gtk::Frame(
		        _("File properties")));
		filePropertiesFrame->set_shadow_type(Gtk::SHADOW_IN);

		Gtk::VBox* filePropertiesFrameVBox = Gtk::manage(new Gtk::VBox());
		filePropertiesFrame->add(*filePropertiesFrameVBox);
		filePropertiesFrameVBox->show();

		for (guint i = 0; i < a_properties.size(); i++)
		{
			Gtk::HBox* box = Gtk::manage(new Gtk::HBox());
			filePropertiesFrameVBox->pack_start(*box, false, false, 3);
			box->show();
			box->pack_start(*(a_labels[i]), false, false, 3);
			box->pack_start(*(a_entries[i]), true, true, 3);
			(a_labels[i])->show();
			(a_entries[i])->show();
		}

		mainHBox3->pack_start(*filePropertiesFrame, true, true, 3);
		filePropertiesFrame->show();
		mainHBox3->show();
	}
	return mainHBox3;
}

void DialogFileProperties::showConventions(Gtk::HBox* hbox)
{
	// display annotation conventions references

	Gtk::Label* lconv1 = Gtk::manage(new Gtk::Label(string(_("Convention"))
	        + " :"));
	lconv1->show();
	Gtk::Entry* convid = Gtk::manage(new Gtk::Entry());
	convid->set_text(a_model.getAGSetProperty("convention_id"));
	convid->show();
	convid->set_sensitive(false);
	Gtk::Label* lconv2 = Gtk::manage(
	        new Gtk::Label(string(_("Version")) + " :"));
	lconv2->show();
	Gtk::Entry* convvers = Gtk::manage(new Gtk::Entry());
	convvers->set_text(a_model.getAGSetProperty("convention_version"));
	convvers->set_width_chars(6);
	convvers->show();
	convvers->set_sensitive(false);
	Gtk::Label* lconv3 = Gtk::manage(new Gtk::Label(string(_("Reference"))
	        + " :"));
	lconv3->show();
	Gtk::Entry* convdoc = Gtk::manage(new Gtk::Entry());
	convdoc->set_text(a_model.getAGSetProperty("convention_doc"));
	convdoc->show();
	convdoc->set_sensitive(false);
	hbox->pack_start(*lconv1, Gtk::PACK_SHRINK, 3);
	hbox->pack_start(*convid, Gtk::PACK_SHRINK, 3);
	hbox->pack_start(*lconv2, Gtk::PACK_SHRINK, 3);
	hbox->pack_start(*convvers, Gtk::PACK_SHRINK, 3);
	hbox->pack_start(*lconv3, Gtk::PACK_SHRINK, 3);
	hbox->pack_start(*convdoc, Gtk::PACK_EXPAND_WIDGET, 3);
}

void DialogFileProperties::set_editability(bool value)
{
	a_editable = value;

	if (a_language)
		a_language->set_sensitive(value);
	if (a_dialect)
		a_dialect->set_sensitive(value);
	if (a_transcriptionCombo)
		a_transcriptionCombo->set_sensitive(value);
	if (a_transcription)
		a_transcription->set_sensitive(value);
	if (a_languageCombo)
		a_languageCombo->set_sensitive(value);
	if (a_dialectCombo)
		a_dialectCombo->set_sensitive(value);
	if (a_namedEntities)
		a_namedEntities->set_sensitive(value);
	if (a_themes)
		a_themes->set_sensitive(value);
	if (a_versionEntry)
	{
		a_versionEntry->set_editable(value);
		a_versionEntry->set_sensitive(value);
	}
	if (a_creationEntry)
	{
		a_creationEntry->set_editable(value);
		a_creationEntry->set_sensitive(value);
	}
	if (a_byEntry)
	{
		a_byEntry->set_editable(value);
		a_byEntry->set_sensitive(value);
	}
	if (a_commentEntryTV)
	{
		a_commentEntryTV->set_sensitive(value);
		a_commentEntryTV->set_editable(value);
	}
	if (a_lastModEntry)
	{
		a_lastModEntry->set_editable(value);
		a_lastModEntry->set_sensitive(value);
	}
	if (a_byEntry2)
	{
		a_byEntry2->set_editable(value);
		a_byEntry2->set_sensitive(value);
	}
	if (a_commentEntryTV2)
	{
		a_commentEntryTV2->set_sensitive(value);
		a_commentEntryTV2->set_editable(value);
	}
	if (a_commentEntryTV3)
	{
		a_commentEntryTV3->set_sensitive(value);
		a_commentEntryTV3->set_editable(value);
	}
	if (a_typeEntrySignal1)
	{
		a_typeEntrySignal1->set_editable(value);
		a_typeEntrySignal1->set_sensitive(value);
	}
	if (a_sourceEntrySignal1)
	{
		a_sourceEntrySignal1->set_editable(value);
		a_sourceEntrySignal1->set_sensitive(value);
	}
	if (a_dateEntrySignal1)
	{
		a_dateEntrySignal1->set_editable(value);
		a_dateEntrySignal1->set_sensitive(value);
	}
	if (a_signalsCount == 2)
	{
		if (a_sourceEntrySignal2)
		{
			a_sourceEntrySignal2->set_editable(value);
			a_sourceEntrySignal2->set_sensitive(value);
		}
		if (a_typeEntrySignal2)
		{
			a_typeEntrySignal2->set_editable(value);
			a_typeEntrySignal2->set_sensitive(value);
		}
		if (a_dateEntrySignal2)
		{
			a_dateEntrySignal2->set_editable(value);
			a_dateEntrySignal2->set_sensitive(value);
		}
	}

	/*	for  (int i=0; i<a_entries.size(); i++) {
	 if (a_entries[i]) {
	 ((Gtk::Entry*)a_entries[i])->set_editable(value) ;
	 ((Gtk::Entry*)a_entries[i])->set_sensitive(value) ;
	 }
	 }*/
}

//**************************************************************************************
//										GEOMETRY INTERFACE
//**************************************************************************************

void DialogFileProperties::getGeo(int& size_xx, int& size_yy, int& pos_x,
        int& pos_y, int& panel)
{
	get_size(size_xx, size_yy);
	get_position(pos_x, pos_y);
	panel = -1;
}

void DialogFileProperties::setGeo(int size_xx, int size_yy, int pos_x,
        int pos_y, int panel)
{
	if (size_xx > 0 && size_yy > 0)
		resize(size_xx, size_yy);
	if (pos_x > 0 && pos_y > 0)
		move(pos_x, pos_y);
}

Glib::ustring DialogFileProperties::getWindowTagType()
{
	return SETTINGS_FPROPERTIES_NAME;
}

int DialogFileProperties::loadGeoAndDisplay(bool rundlg)
{
	loadPos();

	if (rundlg)
		return run();
	else
		show();

	return 1;
}

void DialogFileProperties::saveGeoAndHide()
{
	if (is_visible())
		savePos();
	hide();
}

void DialogFileProperties::getDefaultGeo(int& size_xx, int& size_yy,
        int& pos_x, int& pos_y, int& panel)
{
}

} /* namespace tag */
