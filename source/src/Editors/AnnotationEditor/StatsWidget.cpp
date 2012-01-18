/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @class StatsWidget
 *
 * StatsWidget...
 */

/////////////////////////////////////////////////////
//
//	Bertin Technologies 2006
//	Auteur : Marc-Olivier PICOREAU
//	Version 0.1
//
//	Notes :
//
/////////////////////////////////////////////////////

#include <sstream>

#include "StatsWidget.h"
#include "Common/FileInfo.h"
#include "Common/util/StringOps.h"
#include <math.h>

using namespace std;


namespace tag {

void StatsWidget::printStatus(ostream& out, DataModel& data)
{
	SpeakerDictionary::iterator it;
	int nb[3] = { 0, 0, 0};

	for ( it = data.getSpeakerDictionary().begin();
		it != data.getSpeakerDictionary().end(); ++it )
		nb[it->second.getGender()]++;

	out << "\t total duration : " <<  data.getSignalDuration() << " s" << endl;
	out << "\t nb tracks      : " <<  data.getNbTracks() << endl;
	out << "\t nb speakers    : " <<  data.getSpeakerDictionary().size() << endl;
	out << "\t repartition    : " <<  "male: " << nb[1] << "  female: " << nb[2] << "  undef: " << nb[0] << endl;
	out << endl;
}

void StatsWidget::printStatistics(ostream& out, DataModel& data, int notrack, bool details, bool with_bg, string with_annots)
{
	float speechtime = 0.0;
	float overspeech = 0.0;
	float overnoise = 0.0;
	int nbwords = 0;
	float speech_by_gender[3] = { 0.0, 0.0, 0.0 };
	int gender;
	vector<float> annot_time;
	vector<string> types;

	if ( !with_annots.empty() ) {
		vector<string>::iterator ita;
		StringOps(with_annots).split(types, ";,");
		for ( ita = types.begin(); ita != types.end(); ++ita )
			annot_time.push_back(0.0);
	}

//	m_dataModel.getChilds(childs, curtype, "turn", notrack);
//	vector<string>::iterator itc;

	DataModel_StatHelper helper(data);

	vector<SignalSegment> v1;
	vector<SignalSegment>::iterator it1;
	helper.getSpeechSegments(v1, notrack);
	sort(v1.begin(), v1.end());
	string turnId;
	string spkId;
	vector<string> v2;

	TRACE << "  track " << notrack << " :" << endl;

	for ( it1 = v1.begin(); it1 != v1.end(); ++it1 ) {
			float segtime = (it1->getEndOffset() - it1->getStartOffset());
			int i;

			if ( it1->getOrder() > 0 )
				overspeech += segtime;
			else
				speechtime += segtime;
			nbwords += atoi(it1->getProperty("nbwords").c_str()) ;
			gender = 0;
			spkId = data.getElementProperty(it1->getId(), "speaker");
			if ( spkId  != "" ) {
			try {
					const Speaker& speaker = data.getSpeakerDictionary().getSpeaker(spkId);
					gender = speaker.getGender();
				} catch (...) {}
			}
			speech_by_gender[gender] += segtime;


			if ( with_bg && it1->getProperty("overlapping_noise") == "1" )
				overnoise += segtime;

			for ( i=0; i < types.size(); ++i ) { // check for selected annot types
				v2.clear();
				data.getLinkedElements(it1->getId(), v2, types[i]);
				if ( v2.size() > 0 )
					annot_time[i] += segtime;
			}

//		if ( details ) {
//		out << "seg " << it1->getId() << " " << it1->getStartOffset()
//			<< " - " << it1->getEndOffset()
//			<< "\t noise over speech= " << it1->getProperty("overlapping_noise")
//			<< "\t pron= " << it1->getProperty("pronounce")
//			<< "\t nbwords= " << it1->getProperty("nbwords")
//			<< endl;
//		}
	}

	float len = data.getSignalDuration() ;


	out << "\t total speech time  : " << speechtime << " s \t(" << (speechtime * 100 / len) << " %)" << endl;
	out << "\t nb words           : " << nbwords << endl;
	if ( speechtime == 0. ) speechtime = 1.; // to avoid divide by 0;
	out << "\t speech over speech : " << overspeech << " s \t(" << (overspeech * 100 / speechtime) << " %)" << endl;
//	out << "\t noise over speech : " << overnoise << " s \t(" << (overnoise * 100 / speechtime) << " %)" << endl;
	out << "\t male speakers      : " << speech_by_gender[1] << " s \t(" << (speech_by_gender[1] * 100 / speechtime) << " %)" << endl;
	out << "\t female speakers    : " << speech_by_gender[2] << " s \t(" << (speech_by_gender[2] * 100 / speechtime) << " %)" << endl;
	out << "\t undef speakers     : " << speech_by_gender[0] << " s \t(" << (speech_by_gender[0] * 100 / speechtime) << " %)" << endl;

}

StatsWidget::StatsWidget(DataModel& p_model, bool p_displayAnnotationTime)
{
	bool with_details=false;
	bool with_bg=false;
	string with_annots("");


	m_displayAnnotationTime = p_displayAnnotationTime;
//////////////////////////////////////////////////////////////

	Gtk::TextView* tview =  Gtk::manage(new Gtk::TextView());
	tview->show();
	pack_start(*tview, true, true, 3);

	ostringstream out;

	out << endl << endl;

	printStatus(out, p_model);

	for ( int i=0; i < p_model.getNbTracks(); ++i ) {
		printStatistics(out, p_model, i, with_details, with_bg, with_annots);
	}

	tview->get_buffer()->set_text(out.str());
	tview->set_editable(false);

}

StatsWidget::~StatsWidget() {

}

}
