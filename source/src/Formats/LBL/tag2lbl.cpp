/********************************************************************************/
/******************** (c) Bertin Technologies 2006 - 2008  **********************/
/*			  TranscriberAG version 1.0				*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include <iostream>
#include <iterator>
#include <algorithm>
#include <set>
#include <vector>
#include <list>
#include <getopt.h>
#include <ag/AGAPI.h>
#include <libgen.h>
#include <glib.h>

#include "DataModel/speakers/SpeakerDictionary.h"
#include "DataModel/DataModel.h"
#include "Common/util/ExpRegul.h"
#include "Common/util/StringOps.h"
#include "Common/VersionInfo.h"
#include "Common/FileInfo.h"
#include "Common/iso639.h"

using namespace tag;

int printData(DataModel& data, const char* fname);
void printSpeechSegments(DataModel& data);


string header =
";; CATEGORY \"0\" \"\" \"\" \n"
";; LABEL \"O\" \"Overall\" \"Overall\" \n"
";; \n"
";; CATEGORY \"1\" \"Hub4 Focus Conditions\" \"\" \n"
";; LABEL \"F0\" \"Baseline//Broadcast//Speech\" \"\" \n"
";; LABEL \"F1\" \"Spontaneous//Broadcast//Speech\" \"\" \n"
";; LABEL \"F2\" \"Speech Over//Telephone//Channels\" \"\" \n"
";; LABEL \"F3\" \"Speech in the//Presence of//Background Music\" \"\" \n"
";; LABEL \"F4\" \"Speech Under//Degraded//Acoustic Conditions\" \"\" \n"
";; LABEL \"F5\" \"Speech from//Non-Native//Speakers\" \"\" \n"
";; LABEL \"F6\" \"Overlapping Speech\" \"\" \n"
";; LABEL \"FX\" \"All other speech\" \"\" \n"
";; CATEGORY \"2\" \"Speaker Sex\" \"\" \n"
";; LABEL \"female\" \"Female\" \"\" \n"
";; LABEL \"male\"   \"Male\" \"\" \n"
";; LABEL \"unknown\"   \"Unknown\" \"\" \n";


void USAGE(const char* progname)
{
	cerr << "USAGE: " << progname << " [-v] <filename>" << endl;
	cerr << "\toptions :" << endl;
	cerr << "\t\v-v : print program version " << endl;

	exit(1);
}


int main(int argc,  char* const argv[])
{
	string format("TransAG");
	string signalFilename ("");
	map<string, string> options;
	map<string, string>::iterator it;

	const char* progname = argv[0];

	#ifdef WIN32
	DataModel::initEnviron(progname);
	#else
	DataModel::initEnviron("");
	#endif

	int c;

	while ((c = getopt(argc , argv, "vh")) != -1) {
		switch (c) {
		case 'v':
			cout << progname << " version " << TRANSAG_VERSION_NO << endl;
			return 0;
		default: USAGE(progname);
		}
	}


	if ( optind == argc ) USAGE(progname);
	const char* filename = argv[optind];


	try {

		DataModel data("TransAG");
		string guessed = data.guessFileFormat(filename) ;
		if ( format != guessed ) {
			cerr << "Input file is not in " << format << " format : guessed=" << guessed << endl;
			return 0;
		}
		data.loadFromFile(filename,format);
		FileInfo info(filename);
		info.setTail("lbl");
		data.saveToFile(info.path(), "LBL");
		cerr << "Saved file " << info.path() << flush<< endl;

	} catch (const char* msg ) {
		cerr << " Caught exception : " << msg << endl;
	}

	return 0;
}



int checkOverlappingSegments(DataModel& data, SignalSegment& s)
{
        set<string> v;
        set<string>::iterator it;
        data.getOverlappingSegmentsIds(s, v, "background");

        for(it = v.begin(); it != v.end() ; it++)
        {
                if(*it != s.getId())
                {
                        return 0;
                }
        }
        return 1;
}



int printData(DataModel& data, const char* fname)
{
        vector<SignalSegment> v0;
        vector<SignalSegment> v1;
        vector<SignalSegment> v2;
        vector<string> v3;
        vector<SignalSegment>::iterator it0;
        vector<SignalSegment>::iterator it1;
        vector<SignalSegment>::iterator it2;
        vector<string>::iterator it3;
        bool overlap = false;
        bool already_done = false;
//         double it1_startoffset_backup = -1;
        char* track = (char*)malloc(3);
        (void)memset(track, 0, 3);
				string defname = SpeakerDictionary::defaultFormat;
				unsigned long pos = defname.find("%");
				if ( pos != string::npos ) defname.erase(pos);

        char* tmp = NULL;;
        char* name = (char*)malloc(1+strlen(fname));
        if(strncpy(name, fname, strlen(fname)) == NULL)
        {
                perror("strncpy()");
                return EXIT_FAILURE;
        }
        if((tmp = strrchr(name, '.')) == NULL)
        {
                (void)fprintf(stderr, "'%s' must be a tag file", fname);
                return EXIT_FAILURE;
        }
        else
        {
                *tmp = 0;
        }

        name = basename(name);

        //   data.getSegments("section", v0, 0.0, 0.0);
        data.getSegments("turn", v1, 0.0, 0.0);

        cout << header;

        for(it1 = v1.begin(); it1 != v1.end(); ++it1)
        {
                double start_backup = -1;
                string final_text = "";
                string labels = "";
                string spkgender;

                (void)snprintf(track, 2, "%d", 1 + it1->getTrack());

				if ( it1->getOrder() > 0 ) continue; // overlapping speech

//                 Si aucun speaker identifie, il faut passer tout le tour de parole en exluded
                if(data.getElementProperty(it1->getId(), "speaker") == "")
                {
                        cout << (string)name << " " << (string)track << " excluded_region  ";
                        cout << it1->getStartOffset() << " " << it1->getEndOffset();
                        cout << " <o,,unknown> ignore_time_segment_in_scoring" << endl;
                        continue;
                }

								const vector<string>& over = data.getElementsWithSameStart(it1->getId(), "turn") ;
                if ( over.size() > 0 && !already_done)
                        // alors il y a de la parole superposee
                {
                        cout << (string)name << " " << (string)track << " excluded_region  ";
                        cout << it1->getStartOffset() << " " << it1->getEndOffset();
                        cout << " <o,f6,unknown> ignore_time_segment_in_scoring" << endl;
                        overlap = true;
                        already_done = true;
//                         it1_startoffset_backup = it1->getStartOffset();
                        continue;
                }
                else
                {
                        overlap = false;
                        already_done = false;
                }

                v2.clear();
                string spkid = data.getElementProperty(it1->getId(), "speaker");
								string spkname=spkid;
                try
                                {
																		const Speaker& spk=data.getSpeakerDictionary().getSpeaker(spkid) ;
                                        spkgender = spk.getGenderStr();
                                        spkname = spk.getFullName();
																				if ( spkname.compare(0, defname.length(), defname) == 0 )
																					spkname = spkid;
																				else
																					for ( int i=0; i<spkname[i]; ++i)
																						if ( spkname[i] == ' ' ) spkname[i] = '_';
                                }
                                catch(...)
                                {
                                        spkgender = "unknown";
                                }
                string segment1 = "";
                data.getChildSegments("segment", v2, *it1);
                for(it2 = v2.begin(); it2 != v2.end(); ++it2)
                {
                        string partial_text = "";
                        string text = "";
                        string lab = "";


                        if(it2->getStartOffset() >= 0)
                        {
                                segment1 = (string)name + " " + (string)track + " " + (string)name + "_" + spkname + " ";
                        }

                        data.getQualifiers(it2->getId(), v3, "", true);
                        text = data.getElementProperty(it2->getId(), "value");
                        for(it3 = v3.begin(); it3 != v3.end(); ++it3)
                        {
                                partial_text += "[" + data.getElementType(*it3);
                                const string& desc = data.getElementProperty(*it3, "desc");
								// check if instantaneous or start of event
								const string& start_id = data.getMainstreamStartElement(*it3);
								bool instantaneous = data.isInstantaneous(*it3);
							//	if ( text.empty() ) instantaneous = true;
								if (!instantaneous ) {
									const string& end_id = data.getMainstreamEndElement(*it3);
									if ( end_id.empty() || (start_id == end_id) ) {
										string text =  data.getElementProperty(start_id, "value");
										instantaneous = text.empty() ;
									}
								}

                                if(!desc.empty())
                                {
                                        partial_text +=  "=" + desc;
                                }
								if ( ! instantaneous ) partial_text += "-] ";
								else  partial_text += "] ";
                        }

                        partial_text += text;

                        data.getQualifiers(it2->getId(), v3, "", false);
                        for(it3 = v3.begin(); it3 != v3.end(); ++it3)
                        {
								// check if instantaneous or start of event
									const string& start_id = data.getMainstreamStartElement(*it3);
									const string& end_id = data.getMainstreamEndElement(*it3);
										bool instantaneous = false;
									if ( end_id.empty() || (start_id == end_id) ) {
										string text =  data.getElementProperty(start_id, ""value") ;
										instantaneous = text.empty() ;
									}
								   if ( !instantaneous ) {
								     partial_text += " [-" + data.getElementType(*it3);
                                const string& desc = data.getElementProperty(*it3, "desc");
                                if(!desc.empty())
                                {
                                        partial_text += "=" + desc;
                                }
                                partial_text += "] ";
                        }
										}

                        final_text += partial_text;

                        if(it2->getStartOffset() >= 0)
                        {
                                if(it2->getEndOffset() >= 0)
                                {
					                       if ( final_text == "" ) {
//                                 spkgender = "";
		                                segment1 = (string)name + " " + (string)track + " inter_segment_gap ";
   		                     }
                                		cout << segment1 << " ";
                                        SignalSegment s(it2->getId(), it2->getStartOffset(), it2->getEndOffset(), 0);
                                        if(checkOverlappingSegments(data, s) == 0)
                                        {
                                                lab = "f3";
                                        }
                                        else
                                        {
                                                lab = "f0";
                                        }
                                        labels = "<o," + lab + "," + spkgender + "> ";
                                        cout << it2->getStartOffset() << " " << it2->getEndOffset() << " " << labels << final_text << endl;
                                        final_text = "";
                                        labels = "";
                                        start_backup = -1;
                                }
                                else
                                {
                                        start_backup = it2->getStartOffset();
                                        final_text += " ";
                                }
                        }
                        else
                        {
                                if(it2->getEndOffset() >= 0)
                                {
					                       if ( final_text == "" ) {
//                                 spkgender = "";
		                                segment1 = (string)name + " " + (string)track + " inter_segment_gap ";
   		                     }
                                		cout << segment1 << " ";
                                        SignalSegment s(it2->getId(), it2->getStartOffset(), it2->getEndOffset(), 0);
                                        if(checkOverlappingSegments(data, s) == 0)
                                        {
                                                lab = "f3";
                                        }
                                        else
                                        {
                                                lab = "f0";
                                        }
                                        labels = "<o," + lab + "," + spkgender + "> ";
                                        cout << start_backup << " " << it2->getEndOffset() << " " << labels << final_text << endl;
                                        start_backup = -1;
                                        labels = "";
                                        final_text = "";
                                }
                                else
                                {
                                        final_text += " ";
                                }
                        }
                }
        }
        return EXIT_SUCCESS;
}
