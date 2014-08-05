/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
//
// tagstat : get some statistics on TAG files
//

#include <iostream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <sys/time.h>
#include <getopt.h>

#include "DataModel/DataModel_StatHelper.h"
#include "Common/FileInfo.h"
#include "Common/util/StringOps.h"
using namespace tag;

void printStatistics(ostream& out, DataModel_StatHelper& data, int notrack, bool details, bool with_bg, string with_annots);
void printStatus(ostream& out, DataModel& data);

void USAGE(const char* progname)
{
		Log::err() << "Usage : " << FileInfo(progname).Basename() << " [-o <outfile>] [-d] [-bg] [-c <annot_list>] [-h] <tagfile>" << endl;
		exit(1);
}

int main(int argc, char* const argv[])
{
	const char* progname = argv[0];
	int c;
  	ostream* out = &cout;
	ofstream fout;
	bool with_details=false;
	bool with_bg=false;
	string with_annots("");


	while ((c = getopt(argc , argv, "dho:b:c:")) != -1) {
		switch (c) {
		case 'o': /* redirect output */
			fout.open(optarg);
			if ( ! fout.good() ) { perror(optarg); exit(1); }
			out = &fout;
			break;
		case 'd': with_details = true; break;
		case 'b': with_bg = true; break;
		case 'c': with_annots=optarg; break;

		case 'h':
		case '?': USAGE(progname);
		}
	}

	if ( optind == argc ) USAGE(progname);

	const char* path = argv[optind];

	DataModel::initEnviron(progname);
	DataModel data("TransAG");
	string format;

	format = data.guessFileFormat(path);
	if ( format == "" ) {
			 Log::err() << "Unknown file format" << endl;
				return 1;
	}

	data.loadFromFile(path,format);

	*out << "File: " << path << endl ;
	printStatus(*out, data);

	DataModel_StatHelper helper(data);

	for ( int i=0; i < data.getNbTracks(); ++i ) {
		printStatistics(*out, helper, i, with_details, with_bg, with_annots);
	}

	if ( out == &fout ) fout.close();
	return 0;
}

void printStatus(ostream& out, DataModel& data)
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

void printStatistics(ostream& out, DataModel_StatHelper& helper, int notrack, bool details, bool with_bg, string with_annots)
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


	vector<SignalSegment> v1;
	vector<SignalSegment>::iterator it1;
	helper.getSpeechSegments(v1, notrack);
	sort(v1.begin(), v1.end());
	string turnId;
	string spkId;
	vector<string> v2;

	cout << "  track " << notrack << " :" << endl;

	for ( it1 = v1.begin(); it1 != v1.end(); ++it1 ) {
		if ( helper.getData().isSpeechSegment(it1->getId()) ) {
			float segtime = (it1->getEndOffset() - it1->getStartOffset());
			int i;


			if ( it1->getOrder() > 0 )
				overspeech += segtime;
			else
				speechtime += segtime;
			nbwords += atoi(it1->getProperty("nbwords").c_str()) ;
			gender = 0;
			turnId = helper.getData().getParentElement(it1->getId());
			if ( turnId != "" ) {
				spkId = helper.getData().getElementProperty(turnId, "speaker");
				if ( spkId  != "" ) {
					try {
						const Speaker& speaker = helper.getData().getSpeakerDictionary().getSpeaker(spkId);
						gender = speaker.getGender();
					} catch (...) {}
				}
			}
			speech_by_gender[gender] += segtime;


			if ( with_bg && it1->getProperty("overlapping_noise") == "1" )
				overnoise += segtime;

			for ( i=0; i < types.size(); ++i ) { // check for selected annot types
				v2.clear();
				string id1=it1->getId();
				helper.getData().getLinkedElements(it1->getId(), v2, types[i]);
				if ( v2.size() > 0 )
					annot_time[i] += segtime;
			}
		}

		if ( details ) {
		out << "seg " << it1->getId() << " " << it1->getStartOffset()
			<< " - " << it1->getEndOffset()
			<< "\t overnoise= " << it1->getProperty("overlapping_noise")
			<< "\t pron= " << it1->getProperty("pronounce")
			<< "\t nbwords= " << it1->getProperty("nbwords")
			<< endl;
		}
	}

	float len = helper.getData().getSignalDuration() ;

	out << "\t total speech time  : " << speechtime << " s \t(" << (speechtime * 100 / len) << " %)" << endl;
	out << "\t nb words           : " << nbwords << endl;
	if ( speechtime == 0 ) speechtime = 1; // to avoid divide by 0 !
	out << "\t speech over speech : " << overspeech << " s \t(" << (overspeech * 100 / speechtime) << " %)" << endl;
	if ( with_bg )
		out << "\t noise over speech  : " << overnoise << " s \t(" << (overnoise * 100 / speechtime) << " %)" << endl;
	out << "\t male speakers      : " << speech_by_gender[1] << " s \t(" << (speech_by_gender[1] * 100 / speechtime) << " %)" << endl;
	out << "\t female speakers    : " << speech_by_gender[2] << " s \t(" << (speech_by_gender[2] * 100 / speechtime) << " %)" << endl;
	out << "\t undef speakers     : " << speech_by_gender[0] << " s \t(" << (speech_by_gender[0] * 100 / speechtime) << " %)" << endl;

}
